////////////////////////////////////////////////////////////////////////////////
// Catapult Synthesis
//
// Copyright (c) 2003-2015 Calypto Design Systems, Inc.
//       All Rights Reserved
//
// This document contains information that is proprietary to Calypto Design
// Systems, Inc. The original recipient of this document may duplicate this
// document in whole or in part for internal business purposes only, provided
// that this entire notice appears in all copies. In duplicating any part of
// this document, the recipient agrees to make every reasonable effort to
// prevent the unauthorized use and distribution of the proprietary information.
//
// The design information contained in this file is intended to be an example
// of the functionality which the end user may study in preparation for creating
// their own custom interfaces. This design does not present a complete
// implementation of the named protocol or standard.
//
// NO WARRANTY.
// CALYPTO DESIGN SYSTEMS, INC. EXPRESSLY DISCLAIMS ALL WARRANTY
// FOR THE SOFTWARE. TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE
// LAW, THE SOFTWARE AND ANY RELATED DOCUMENTATION IS PROVIDED "AS IS"
// AND WITH ALL FAULTS AND WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, EITHER EXPRESS OR IMPLIED, INCLUDING, WITHOUT LIMITATION, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
// PURPOSE, OR NONINFRINGEMENT. THE ENTIRE RISK ARISING OUT OF USE OR
// DISTRIBUTION OF THE SOFTWARE REMAINS WITH YOU.
//
////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "stdio.h"
#include "assert.h"


typedef unsigned char huff_sizes_t[256];
typedef unsigned int  huff_codes_t[256];

typedef struct {
  huff_sizes_t sizes;
  huff_codes_t codes;
} hufftables_t;

  
// DC encoding interface, diff is difference from last DC coefficient
// si is size, co is code value 
unsigned char huffencode(huff_sizes_t t, int diff);
unsigned int  huffencode(huff_codes_t t, int diff);

// AC encoding interface, r is 0 run length (up to 15), coeff is AC coefficient
// si is size, co is code value
unsigned char huffencode(huff_sizes_t t, unsigned char r, int coeff);
unsigned int  huffencode(huff_codes_t t, unsigned char r, int coeff);


