#include <AppMain.h>
#include <filesystem>
#include <memory>
#include <system_error>

#include "CHIPProjectAppConfig.h"
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

int bootstrap()
{
    std::filesystem::path chip_dir = CHIP_DIR;
    if (std::filesystem::exists(chip_dir))
    {
        return 0;
    }

    std::error_code err;
    if (!std::filesystem::create_directories(chip_dir, err))
    {
        ERROR("Could not create %s: %s", chip_dir.c_str(), err.message().c_str());
        return -1;
    }

    DEBUG("Created CHIP_DIR: %s", chip_dir.c_str());
    return 0;
}

int main(int argc, char * argv[])
{
    if (bootstrap() != 0)
    {
        return -1;
    }

    if (ChipLinuxAppInit(argc, argv) != 0)
    {
        return -1;
    }

    ChipLinuxAppMainLoop();
    return 0;
}
