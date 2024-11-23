#pragma once

#include <functional>

#include "system/SocketEvents.h"
#include <lib/core/CHIPError.h>

class Socket
{
public:
    Socket() = default;
    ~Socket();

    // No copying.
    Socket(const Socket &) = delete;
    Socket & operator=(const Socket &) = delete;

    // Moving is allowed.
    Socket(Socket &&);
    Socket & operator=(Socket &&);

    CHIP_ERROR Init();
    void Close();

    // TODO: C++ify?
    ssize_t Recv(void * buffer, size_t len, int flags = 0);
    ssize_t Send(const void * buffer, size_t len, int flags = 0);

    Socket Accept();

    CHIP_ERROR SetReadCallback(std::function<void(void)> callback);
    CHIP_ERROR SetWriteCallback(std::function<void(void)> callback);

    void SocketCallback(chip::System::SocketEvents events);

private:
    explicit Socket(int fd);

    int mSockFd = -1;
    chip::System::SocketWatchToken mSocketWatchToken = 0;

    std::function<void(void)> mReadCallback = nullptr;
    std::function<void(void)> mWriteCallback = nullptr;
};
