#include <arpa/inet.h>
#include <cstdint>

#include "lib/support/TypeTraits.h"
#include "lib/core/CHIPError.h"
#include "lib/support/CodeUtils.h"
#include "lib/support/Span.h"
#include "logger.h"

#include "mqtt/broker.h"

namespace MQTT
{

enum class PACKET_TYPE : uint8_t
{
    Reserved = 0,
    CONNECT = 1,
    CONNACK = 2,
    PUBLISH = 3,
    PUBACK = 4,
    SUBSCRIBE = 8,
    SUBACK = 9,
    PINGREQ = 12,
    PINGRESP = 13,
    DISCONNECT = 14,
};

CHIP_ERROR Broker::Init(std::function<void(std::string, std::string)> publish_callback)
{
    TRACE;

    CHIP_ERROR result;
    if ((result = mServer.Init()) != CHIP_NO_ERROR)
    {
        return result;
    }

    mServer.SetReadCallback(std::bind(&Broker::ServerRead, this));

    if (publish_callback == nullptr)
    {
        ERROR("PublishCallback is nullptr");
        return CHIP_ERROR_INVALID_ADDRESS;
    }
    mPublishCallback = publish_callback;

    return CHIP_NO_ERROR;
}

void Broker::ServerRead()
{
    if (!mConnected)
    {
        mClient = mServer.Accept();
        CHIP_ERROR result;
        if ((result = mClient.SetReadCallback(std::bind(&Broker::ClientRead, this))) != CHIP_NO_ERROR)
        {
            ERROR("Could not set read callback");
            chipDie();
        }
        return;
    }
    ERROR("Accepting when already connected");
    chipDie();
}

template <typename T>
T consume(chip::Span<char> & buffer)
{
    T a = *reinterpret_cast<T*>(buffer.data());
    buffer = buffer.SubSpan(sizeof(T));
    return a;
}

uint32_t consume_variable_byte_integer(chip::Span<char> & buffer)
{
    constexpr uint8_t CONTINUATION_BIT = 1 << 7;
    uint32_t value = 0;
    uint32_t temp = 0;
    temp = consume<uint8_t>(buffer);
    value |= (temp & ~CONTINUATION_BIT);
    if (temp & CONTINUATION_BIT)
    {
        temp = consume<uint8_t>(buffer);
        value |= ((temp & ~CONTINUATION_BIT) << 7);
        if (temp & CONTINUATION_BIT)
        {
            temp = consume<uint8_t>(buffer);
            value |= ((temp & ~CONTINUATION_BIT) << 15);
            if (temp & CONTINUATION_BIT)
            {
                temp = consume<uint8_t>(buffer);
                value |= ((temp & ~CONTINUATION_BIT) << 23);
            }
        }
    }
    return value;
}

std::string consume_data(chip::Span<char> & buffer, size_t size)
{
    auto sub = buffer.SubSpan(0, size);
    buffer = buffer.SubSpan(size);
    return std::string(sub.begin(), sub.end());
}

std::string consume_utf8(chip::Span<char> & buffer)
{
    auto len = htons(consume<uint16_t>(buffer));
    auto sub = buffer.SubSpan(0, len);
    buffer = buffer.SubSpan(len);
    return std::string(sub.begin(), sub.end());
}

// TODO: Have a global malloc'd buffer that can dynamically grow when needed?
static char gBuffer[16384];

CHIP_ERROR Broker::Publish(const std::string & topic, const std::string & message)
{
    size_t payload_size = sizeof(uint8_t) + sizeof(uint8_t) + sizeof(uint16_t) + topic.length() + message.length();

    if (topic.length() > 0b01111111)
    {
        ERROR("Variable length detected, need %ld", topic.length());
        chipDie();
    }

    DEBUG("Publishing %s, message: %s", topic.c_str(), message.c_str());

    gBuffer[0] = chip::to_underlying(PACKET_TYPE::PUBLISH) << 4;
    gBuffer[1] = payload_size - 2; // remaining length
    gBuffer[2] = topic.length() >> 8;
    gBuffer[3] = topic.length() & 0b11111111;
    topic.copy(reinterpret_cast<char*>(gBuffer) + 4, topic.length());
    message.copy(reinterpret_cast<char*>(gBuffer) + 4 + topic.length(), message.length());

    ssize_t sent_bytes = mClient.Send(gBuffer, payload_size);
    if (sent_bytes < static_cast<ssize_t>(payload_size))
    {
        ERROR("Could not send all data?");
        chipDie();
    }

    return CHIP_NO_ERROR;
}

void Broker::HandleConnect(chip::Span<char> & buffer)
{
    // TODO: Process second connect command as error
    auto len = htons(consume<uint16_t>(buffer));
    auto protocol = consume_data(buffer, len);
    if (strncmp(protocol.data(), "MQTT", protocol.size()) != 0)
    {
        ERROR("Invalid protocol");
        chipDie();
    }

    auto protocol_version = consume<uint8_t>(buffer);

    auto connect_flags = std::bitset<8>(consume<uint8_t>(buffer));

    if (connect_flags.test(0)) // Reserved
    {
        ERROR("Reserved flag is set");
        chipDie();
    }

    if (connect_flags.test(1)) // Clean start
    {
        // TODO: Enforce this is a "clean start"
    }

    // TODO: Will
    [[maybe_unused]] bool has_will = connect_flags.test(2);
    [[maybe_unused]] uint8_t will_qos = (connect_flags.test(4) << 1) | connect_flags.test(3);
    [[maybe_unused]] bool will_retain = connect_flags.test(5);

    if (connect_flags.test(6) || connect_flags.test(7))
    {
        ERROR("Unexpected connect flags: %s", connect_flags.to_string().c_str());
        chipDie();
    }

    auto keep_alive = htons(consume<uint16_t>(buffer));
    auto client_id = consume_utf8(buffer);
    if (client_id.empty())
    {
        client_id = "<None>";
    }

    DEBUG("Connection from: %s (protocol: %d, keep alive: %d)", client_id.data(), protocol_version, keep_alive);

    if (has_will)
    {
        auto will_topic = consume_utf8(buffer);
        auto will_message = consume_utf8(buffer);
        DEBUG("Will: %s, message: %s", will_topic.c_str(), will_message.c_str());
    }

    if (buffer.size())
    {
        ERROR("More unexpected bytes");
        chipDie();
    }

    unsigned char response[4];
    response[0] = chip::to_underlying(PACKET_TYPE::CONNACK) << 4;
    response[1] = 2; // remaining length
    response[2] = 0; // connect ack flags (no session present flag)
    response[3] = 0; // connect reason code (success for clean start)
    
    ssize_t sent_bytes = mClient.Send(response, sizeof(response));
    if (sent_bytes < static_cast<ssize_t>(sizeof(response)))
    {
        ERROR("Could not send all data?");
        chipDie();
    }
}

void Broker::HandleSubscribe(chip::Span<char> & buffer)
{
    auto packet_identifier = htons(consume<uint16_t>(buffer));

    while (!buffer.empty())
    {
        auto topic_filter = consume_utf8(buffer);
        auto subscription_options = std::bitset<8>(consume<uint8_t>(buffer));
        
        if (subscription_options.test(1) || subscription_options.test(2) || subscription_options.test(3) || subscription_options.test(4) || subscription_options.test(5) || subscription_options.test(6) || subscription_options.test(7))
        {
            ERROR("Unexpected subscription options: %s", subscription_options.to_string().c_str());
            chipDie();
        }
        
        DEBUG("Client subscribing to %s (id %d)", topic_filter.data(), packet_identifier);
    }

    unsigned char response[5];
    response[0] = chip::to_underlying(PACKET_TYPE::SUBACK) << 4;
    response[1] = 3; // remaining length
    response[2] = packet_identifier >> 8;
    response[3] = packet_identifier & 0b11111111;
    response[4] = 0; // QoS 0

    ssize_t sent_bytes = mClient.Send(response, sizeof(response));
    if (sent_bytes < static_cast<ssize_t>(sizeof(response)))
    {
        ERROR("Could not send all data?");
        chipDie();
    }
}

void Broker::HandlePingReq(chip::Span<char> &)
{
    unsigned char response[2];
    response[0] = chip::to_underlying(PACKET_TYPE::PINGRESP) << 4;
    response[1] = 0; // remaining length

    ssize_t sent_bytes = mClient.Send(response, sizeof(response));
    if (sent_bytes < static_cast<ssize_t>(sizeof(response)))
    {
        ERROR("Could not send all data?");
        chipDie();
    }
#if 0
    DEBUG("Pong");
#endif
}

void Broker::HandlePublish(chip::Span<char> & buffer, std::bitset<4> flags)
{
    bool dup = flags.test(3);
    uint8_t qos = (flags.test(2) << 1) | flags.test(1);
    bool retain = flags.test(0);

    if (qos == 0 && dup)
    {
        ERROR("Invalid state: dup and qos != 0");
        chipDie();
    }

    if (qos == 0b11)
    {
        ERROR("Invalid QoS");
        chipDie();
    }

    if (dup)
    {
        DEBUG("<re-delivery>");
    }

    if (retain)
    {
        // We default retain behavior
    }

    auto topic_name = consume_utf8(buffer);

    uint16_t packet_identifier = 0;
    if (qos == 1 || qos == 2)
    {
        packet_identifier = htons(consume<uint16_t>(buffer));
    }

    std::string message(buffer.begin(), buffer.end());

    if (qos == 1)
    {
        unsigned char response[6];
        response[0] = chip::to_underlying(PACKET_TYPE::PUBACK) << 4;
        response[1] = 4; // remaining length
        response[2] = packet_identifier >> 8;
        response[3] = packet_identifier & 0b11111111;
        response[4] = 0; // reason code: success
        response[5] = 0;

        ssize_t sent_bytes = mClient.Send(response, sizeof(response));
        if (sent_bytes < static_cast<ssize_t>(sizeof(response)))
        {
            ERROR("Could not send all data?");
            chipDie();
        }
    }

    if (qos == 2)
    {
        ERROR("QoS 2");
        chipDie();
    }

    mPublishCallback(topic_name, message);
}

void Broker::HandleDisconnect(chip::Span<char> & buffer, std::bitset<4> flags)
{
    if (buffer.size())
    {
        auto reason = consume<uint8_t>(buffer);
        if (reason != 0)
        {
            ERROR("Disconnected for reason: %d", reason);
            chipDie();
        }

        if (buffer.size())
        {
            ERROR("More bytes at end of packet");
            chipDie();
        }
    }

    DEBUG("Disconnected");
    mClient.Close();

    // TODO: Send malformed packet when flags != 0
}

void AssertFlags(std::bitset<4> flags, std::bitset<4> expected)
{
    if (flags != expected)
    {
        ERROR("Got unexpected flags");
        chipDie();
    }        
}

void Broker::ClientRead()
{
    ssize_t recvd = 0;

    // Fixed header
    char control_flags = 0;
    recvd = mClient.Recv(&control_flags, 1);
    if (recvd != 1)
    {
        ERROR("Client disconnected unexpectedly");
        mClient.Close();
        return;
    }

    uint8_t packet_type = (control_flags & 0b11110000) >> 4;
    std::bitset<4> flags = control_flags & 0b1111;

    constexpr uint8_t CONTINUATION_BIT = 1 << 7;
    uint32_t remaining_length = 0;
    uint8_t temp = 0;
    mClient.Recv(&temp, 1);
    remaining_length |= (temp & ~CONTINUATION_BIT);
    if (temp & CONTINUATION_BIT)
    {
        mClient.Recv(&temp, 1);
        remaining_length |= ((temp & ~CONTINUATION_BIT) << 7);
        if (temp & CONTINUATION_BIT)
        {
            mClient.Recv(&temp, 1);
            remaining_length |= ((temp & ~CONTINUATION_BIT) << 15);
            if (temp & CONTINUATION_BIT)
            {
                mClient.Recv(&temp, 1);
                remaining_length |= ((temp & ~CONTINUATION_BIT) << 23);
            }
        }
    }

    if (remaining_length > sizeof(gBuffer) - 1)
    {
        ERROR("Remaining length too big: %d", remaining_length);
        chipDie();
    }

    if (remaining_length)
    {
        recvd = 0;
        while (recvd < remaining_length)
        {
            ssize_t rtemp = mClient.Recv(gBuffer + recvd, remaining_length - recvd);
            if (rtemp == 0)
            {
                ERROR("Client disconnected unexpectedly");
                mClient.Close();
                return;
            }
            recvd += rtemp;
        }
        gBuffer[recvd] = 0;
    }

    chip::Span<char> span(gBuffer, remaining_length);

    switch (static_cast<PACKET_TYPE>(packet_type))
    {
        case PACKET_TYPE::CONNECT:
            AssertFlags(flags, 0b0000);
            HandleConnect(span);
            break;
        case PACKET_TYPE::PUBLISH:
            HandlePublish(span, flags);
            break;
        case PACKET_TYPE::SUBSCRIBE:
            AssertFlags(flags, 0b0010);
            HandleSubscribe(span);
            break;
        case PACKET_TYPE::PINGREQ:
            AssertFlags(flags, 0b0000);
            HandlePingReq(span);
            break;
        case PACKET_TYPE::DISCONNECT:
            HandleDisconnect(span, flags);
            break;
        default:
            ERROR("Unhandled packet type: %d", packet_type);
            chipDie();
    }
}

}
