#include <arpa/inet.h>
#include <cstring>
#include <errno.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <unistd.h>

#include <platform/CHIPDeviceLayer.h>
#include <system/SystemLayerImpl.h>
#include <system/SystemLayer.h>

#include "logger.h"
#include "socket.h"
#include "system/SocketEvents.h"

using namespace chip;
using namespace chip::System;

static void SocketCallback2(chip::System::SocketEvents events, intptr_t data)
{
    TRACE;
    auto _this = reinterpret_cast<Socket*>(data);
    _this->SocketCallback(events);
}

CHIP_ERROR Socket::Init()
{
    TRACE;

    mSockFd = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (mSockFd == -1)
    {
        ERROR("Failed to create socket: %s", strerror(errno));
        return CHIP_ERROR_NOT_CONNECTED; 
    }

    int opt = 1;
    if (::setsockopt(mSockFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        ERROR("Failed to set reuseaddr: %s", strerror(errno));
        return CHIP_ERROR_NOT_CONNECTED; 
    }

    struct sockaddr_in server_addr = {};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(1884);

    if (::bind(mSockFd, (struct sockaddr *)&server_addr, sizeof(server_addr)))
    {
        ERROR("Failed to bind to address: %s", strerror(errno));
        return CHIP_ERROR_NOT_CONNECTED;
    }

    if (::listen(mSockFd, 0))
    {
        ERROR("Failed to start listening: %s", strerror(errno));
        return CHIP_ERROR_NOT_CONNECTED;
    }

    int flags = ::fcntl(mSockFd, F_GETFL, 0);
    if (::fcntl(mSockFd, F_SETFL, flags | O_NONBLOCK))
    {
        ERROR("Failed to set non-blocking socket: %s", strerror(errno));
        return CHIP_ERROR_NOT_CONNECTED;
    }

    return CHIP_NO_ERROR;
}

void Socket::Close()
{
    if (mSockFd == -1)
    {
        return;
    }

    CHIP_ERROR result = DeviceLayer::SystemLayerSockets().StopWatchingSocket(&mSocketWatchToken);
    if (result != CHIP_NO_ERROR)
    {
        WARN("Failed to stop watching socket");
    }
    mSocketWatchToken = 0;

    if (::close(mSockFd))
    {
        WARN("Failed to close socket");
    }
    mSockFd = -1;
}

Socket::~Socket()
{
    Close();
}

Socket::Socket(Socket && other)
{
    if (&other != this)
    {
        std::swap(mSockFd, other.mSockFd);
        std::swap(mSocketWatchToken, other.mSocketWatchToken);
        mReadCallback.swap(other.mReadCallback);
        mWriteCallback.swap(other.mWriteCallback);
    }
}

Socket & Socket::operator=(Socket && other)
{
    if (&other != this)
    {
        std::swap(mSockFd, other.mSockFd);
        std::swap(mSocketWatchToken, other.mSocketWatchToken);
        mReadCallback.swap(other.mReadCallback);
        mWriteCallback.swap(other.mWriteCallback);
    }
    return *this;
}


ssize_t Socket::Recv(void * buffer, size_t len, int flags)
{
    return ::recv(mSockFd, buffer, len, flags);
}

ssize_t Socket::Send(const void * buffer, size_t len, int flags)
{
    return ::send(mSockFd, buffer, len, flags);
}

Socket::Socket(int fd)
    : mSockFd(fd)
{
}


Socket Socket::Accept()
{
    struct sockaddr_in address{};
    int addrlen = sizeof(address);
    int fd;

    if ((fd = ::accept(mSockFd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) == -1)
    {
        ERROR("Failed to accept: %s", strerror(errno));
        chipDie();
    }

    return Socket(fd);
}


void Socket::SocketCallback(chip::System::SocketEvents events)
{
    TRACE;
    if (events.Has(SocketEventFlags::kRead))
    {
        if (mReadCallback == nullptr)
        {
            ERROR("No read callback set");
            chipDie();
        }
        mReadCallback();
    }
    if (events.Has(SocketEventFlags::kWrite))
    {
        if (mWriteCallback == nullptr)
        {
            ERROR("No write callback set");
            chipDie();
        }
        mWriteCallback();
    }
    if (events.Has(SocketEventFlags::kExcept))
    {
        ERROR("Socket exception");
    }
    if (events.Has(SocketEventFlags::kError))
    {
        ERROR("Socket error");
    }
}

CHIP_ERROR Socket::SetReadCallback(std::function<void(void)> readCallback)
{
    TRACE;

    if (mSocketWatchToken == 0)
    {
        CHIP_ERROR result = DeviceLayer::SystemLayerSockets().StartWatchingSocket(mSockFd, &mSocketWatchToken);
        if (result != CHIP_NO_ERROR)
        {
            ERROR("Could not start watching socket: %s", result.AsString());
            return result;
        }

        result = DeviceLayer::SystemLayerSockets().SetCallback(mSocketWatchToken, SocketCallback2, reinterpret_cast<intptr_t>(this));
        if (result != CHIP_NO_ERROR)
        {
            ERROR("Could not set callback");
            return result;
        }
    }

    mReadCallback = readCallback;
    return DeviceLayer::SystemLayerSockets().RequestCallbackOnPendingRead(mSocketWatchToken);
}

CHIP_ERROR Socket::SetWriteCallback(std::function<void(void)> writeCallback)
{
    TRACE;

    if (mSocketWatchToken == 0)
    {
        CHIP_ERROR result = DeviceLayer::SystemLayerSockets().StartWatchingSocket(mSockFd, &mSocketWatchToken);
        if (result != CHIP_NO_ERROR)
        {
            ERROR("Could not start watching socket: %s", result.AsString());
            return result;
        }

        result = DeviceLayer::SystemLayerSockets().SetCallback(mSocketWatchToken, SocketCallback2, reinterpret_cast<intptr_t>(this));
        if (result != CHIP_NO_ERROR)
        {
            ERROR("Could not set callback");
            return result;
        }
    }

    mWriteCallback = writeCallback;
    return DeviceLayer::SystemLayerSockets().RequestCallbackOnPendingWrite(mSocketWatchToken);
}

