#pragma once

#include <bitset>

#include "lib/support/Span.h"
#include "socket.h"

namespace MQTT
{

/** This is a limited and partially compliant MQTT broker implementation. It
    allows for one client device to be connected. */
class Broker
{
public:
    Broker() = default;

    CHIP_ERROR Init();

private:
    Socket mServer;
    Socket mClient;
    bool mConnected = false;

    void ServerRead();
    void ClientRead();

    void HandleConnect(chip::Span<char> &);
    void HandleSubscribe(chip::Span<char> &);
    void HandlePingReq(chip::Span<char> &);
    void HandlePublish(chip::Span<char> &, std::bitset<4>);
    void HandleDisconnect(chip::Span<char> &, std::bitset<4>);
};

}