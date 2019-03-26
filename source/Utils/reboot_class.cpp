#include <stdlib.h>
#include <stdio.h>
#include <switch.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>

extern "C" {
#include "reboot.h"
}
#include "reboot_class.hpp"


bool Reboot::InitalizeRebootFeature()
{
	return init_slp();

}

void Reboot::ExitRebootFeature(bool can_reboot)
{
	exit_spl(can_reboot);
}

void Reboot::DoReboot()
{
	reboot_to_payload();
}

