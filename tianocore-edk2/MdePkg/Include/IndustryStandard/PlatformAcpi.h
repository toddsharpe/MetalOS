/** @file
  This file contains the latest ACPI definitions that are
  consumed by drivers that do not care about ACPI versions.

  Copyright (c) 2006 - 2017, Intel Corporation. All rights reserved.<BR>
  Copyright (c) 2019, ARM Ltd. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef _ACPI_H_
#define _ACPI_H_

//
// Define macros to build data structure signatures from characters.
//
#define SIGNATURE_16(A, B)        ((A) | (B << 8))
#define SIGNATURE_32(A, B, C, D)  (SIGNATURE_16 (A, B) | (SIGNATURE_16 (C, D) << 16))
#define SIGNATURE_64(A, B, C, D, E, F, G, H) \
    (SIGNATURE_32 (A, B, C, D) | ((UINT64) (SIGNATURE_32 (E, F, G, H)) << 32))
#include <IndustryStandard/Acpi63.h>

#endif
