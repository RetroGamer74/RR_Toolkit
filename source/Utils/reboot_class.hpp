#pragma once

#include <switch.h>
#include "reboot.h"

namespace Reboot {
    
bool InitalizeRebootFeature(void);
void ExitRebootFeature(bool can_reboot);
void DoReboot();

};
