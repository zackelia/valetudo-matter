#include "rvc.h"

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

    return CHIP_NO_ERROR;
}

void RVC::Shutdown()
{
    // TODO
}
