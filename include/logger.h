#pragma once

#include <cstring>

#include "lib/support/logging/TextOnlyLogging.h"

#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#define TRACE                                                                                                          \
    ChipLogProgress(DeviceLayer, "\033[30m[%s:%d]\033[36m Entering %s\033[0m", __FILENAME__, __LINE__, __func__)
#define DEBUG(a, args...)                                                                                              \
    ChipLogProgress(DeviceLayer, "\033[30m[%s:%d] \033[32m" a "\033[0m", __FILENAME__, __LINE__, ##args)
#define WARN(a, args...)                                                                                               \
    ChipLogProgress(DeviceLayer, "\033[30m[%s:%d] \033[33m" a "\033[0m", __FILENAME__, __LINE__, ##args)
#define ERROR(a, args...)                                                                                              \
    ChipLogProgress(DeviceLayer, "\033[30m[%s:%d] \033[1;31m" a "\033[0m", __FILENAME__, __LINE__, ##args)
