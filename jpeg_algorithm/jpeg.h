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

#include "huff.h"
//#include "tables.h"

#include "ac_fixed.h"
#include "ac_channel.h"

// ----------------------------------------------------------------------------------------------
//  Data types
// ----------------------------------------------------------------------------------------------

// Huffman Codes
// DC code is index 0, AC codes 1..63
struct codes_t {
  uint6   size;
  uint32  code;
  
  codes_t() {
    size=0;
    code=0; 
  };
  
  codes_t(uint6 s, uint32 c) {
    size=s;
    code=c;
  }
};

struct rgb_t {
  uint8 r;
  uint8 g;
  uint8 b;
};

struct block64 {
   short mem[64];
};

// ----------------------------------------------------------------------------------------------
//  Forward definitions (local functions)
// ----------------------------------------------------------------------------------------------

void pixelpipe(
      ac_channel<uint2>   &blocktype,           // input  block type, Y, Cr, Cb
      ac_channel<rgb_t>   &rgb,                 // input  block
      ac_channel<codes_t> &codes );             // output huffman codes

