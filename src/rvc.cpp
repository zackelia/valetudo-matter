#include "lib/core/CHIPError.h"
#include "logger.h"
#include "platform/CHIPDeviceLayer.h"
#include "rvc.h"
#include "system/SocketEvents.h"

using namespace chip::app::Clusters;

CHIP_ERROR RVC::Init()
{
    CHIP_ERROR result;

    if ((result = mCleanModeInstance.Init()) != CHIP_NO_ERROR)
    {
        return result;
    }

    if ((result = mRvcOperationalStateInstance.Init()) != CHIP_NO_ERROR)
    {
        return result;
    }

    if ((result = mRunModeInstance.Init()) != CHIP_NO_ERROR)
    {
        return result;
    }

    // if ((result = mSocket.Init()) != CHIP_NO_ERROR)
    // {
    //     return result;
    // }

    // if ((result = mSocket.SetReadCallback(std::bind(&RVC::SocketRead, this))) != CHIP_NO_ERROR)
    // {
    //     return result;
    // }

    return CHIP_NO_ERROR;
}

void RVC::Shutdown()
{
    // TODO: Shutdown instances
}

// void RVC::SocketRead()
// {
//     TRACE;

//     char buffer[1024] = {};
//     ssize_t recvd = mSocket.Recv(buffer, sizeof(buffer));
//     if (!recvd)
//     {
//         mSocket.Close();
//         WARN("Socket disconnected");
//         return;
//     }

//     mRvcOperationalStateInstance.SetOperationalState(to_underlying(RvcOperationalState::OperationalStateEnum::kCharging));
// }
