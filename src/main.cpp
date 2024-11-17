#include <AppMain.h>
#include <memory>

#include "logger.h"
#include "rvc.h"

std::unique_ptr<chip::app::Clusters::RVC> rvc;

void ApplicationInit()
{
    rvc = std::make_unique<chip::app::Clusters::RVC>();
    CHIP_ERROR result = rvc->Init();
    if (result != CHIP_NO_ERROR)
    {
        chipDie();
    }
    DEBUG("Application initialized");
}

void ApplicationShutdown()
{
    WARN("Application shutting down");
    rvc->Shutdown();
    rvc.reset();
}

int main(int argc, char * argv[])
{
    if (ChipLinuxAppInit(argc, argv) != 0)
    {
        return -1;
    }

    ChipLinuxAppMainLoop();
    return 0;
}
