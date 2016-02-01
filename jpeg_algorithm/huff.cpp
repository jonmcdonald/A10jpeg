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
#include "huff.h"
#include "ac_int.h"

// Figure F.1 DC Difference magnitude categories
unsigned char SSSS(int diff) {
   unsigned char r = 0;
  diff = (diff>=0)?diff:-diff;
  #pragma unroll yes
   for (int i=11; i>=0; i--)
      if ((diff>>i)==0) r=i;
   return r;
}  

// returns RRRRSSSS byte, R is run length up to 15, S is category of coefficient
// part of AC encoding
unsigned char RS(unsigned char r, int coefficient) {
#ifndef __SYNTHESIS__
   assert(r <= 15);
#endif
   return (r << 4) | SSSS(coefficient);
}

// Concatinate bits and return them (with result msbs 0 if unfilled)
unsigned int concat_bits(unsigned int msbs, int lsbs, unsigned char nlsbs) {
   lsbs &= (1<<(nlsbs))-1;
   return (msbs << nlsbs) | lsbs;
}


unsigned char huffencode(huff_sizes_t t, int diff) {
   unsigned char magnitude = SSSS(diff);
   unsigned char huffsize  = t[magnitude];
   
   return magnitude + huffsize;
}


unsigned int huffencode(huff_codes_t t, int diff) {
   unsigned char magnitude = SSSS(diff);
   unsigned int  huffcode  = t[magnitude];

   if (diff < 0) diff = diff - 1;

   return concat_bits(huffcode, diff, magnitude);
}

unsigned int huffencode(huff_codes_t t, unsigned char r, int coeff) {
   unsigned char magnitude = RS(r, coeff);
   unsigned int  huffcode  = t[magnitude];
   
   return concat_bits(huffcode, (coeff < 0) ? coeff - 1 : coeff, SSSS(coeff));
}


unsigned char huffencode(huff_sizes_t t, unsigned char r, int coeff) {
   unsigned char magnitude = RS(r, coeff);
   unsigned char huffsize  = t[magnitude];
   
   return huffsize + SSSS(coeff);
}
