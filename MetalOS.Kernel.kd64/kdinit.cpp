/*
 * PROJECT:         ReactOS Kernel
 * LICENSE:         GPL - See COPYING in the top level directory
 * FILE:            ntoskrnl/kd64/kdinit.c
 * PURPOSE:         KD64 Initialization Code
 * PROGRAMMERS:     Alex Ionescu (alex.ionescu@reactos.org)
 *                  Stefan Ginsberg (stefan.ginsberg@reactos.org)
 */

 /* INCLUDES ******************************************************************/

#include <sal.h>
#include <cstdint>
#include <windows/types.h>
#include <windows/winnt.h>
#include <reactos/ketypes.h>
#include <reactos/amd64/ketypes.h>
#include <reactos/windbgkd.h>
#include <coreclr/list.h>
#include <kddll.h>
#include <reactos/amd64/ke.h>
#include "kd64.h"

#define DPRINT1 DbgPrint


BOOLEAN
NTAPI
KdInitSystem()
{
	return TRUE;
}
