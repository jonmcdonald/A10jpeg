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

#include "jpeg.h"
#include "dct.h"
#include <math/mgc_ac_math.h>
#include <mc_scverify.h>
// ----------------------------------------------------------------------------------------------

// ---------------------------------------------------
// Convert a 8x8 RGB block to a 8x8 YCbCr block
// ---------------------------------------------------
void convert( ac_channel<uint2>  &blocktype, 
              ac_channel<uint2>  &blocktype_out, 
              ac_channel<rgb_t>  &rgb,
              ac_channel<ac_int<8> >   &ycbcr )
{
  ac_fixed<16,1,true> coeffs[3][3] = {
    { 0.299   ,  0.587   ,  0.114    },
    {-0.168736, -0.331264,  0.5      },
    { 0.5     , -0.418688, -0.081312 } };

  ac_fixed<8,8,false> tmp;
  uint2 k = blocktype.read();
  blocktype_out.write(k);
  // normalize values by subtracting 128 and convert to YCbCr
  unsigned char r, g, b;
  convert2ycbcr:
  for (unsigned int i=0; i<8; i++) {
    for (unsigned int j=0; j<8; j++) {
      rgb_t datai = rgb.read();
      r = datai.r;
      g = datai.g;
      b = datai.b;
      tmp = coeffs[k][0]*r + coeffs[k][1]*g + coeffs[k][2]*b + ((k)?128:0);
      tmp -= 128;
      ycbcr.write( tmp.to_uint() );
    }
  }
}



void quantize(
              ac_channel<uint2>   &blocktype,                // input  block type, Y, Cr, Cb
              ac_channel<uint2>   &blocktype_out,
              ac_channel<memStruct> &blocki,               // input   block - dct output, vectorize input
              ac_channel<short>   &datao,                // output  block - quantize output, huffman input
              ac_channel<uint6>   &last_non_zero_index ) // output  index of the last non-zero value in the output block

{
  memStruct datai    = blocki.read();
  uint2   type     = blocktype.read();
  blocktype_out.write(type);
  uint1   LC       = (type==LUMA)?LUMA:CHROMA;
  uint6   last_nzi = 0;
  uint6   addr;
  const unsigned zigzagpath_vector[64] = {
    0,  1,  8, 16,  9,  2,  3, 10,
    17, 24, 32, 25, 18, 11,  4,  5,
    12, 19, 26, 33, 40, 48, 41, 34,
    27, 20, 13,  6,  7, 14, 21, 28,
    35, 42, 49, 56, 57, 50, 43, 36,
    29, 22, 15, 23, 30, 37, 44, 51,
    58, 59, 52, 45, 38, 31, 39, 46,
    53, 60, 61, 54, 47, 55, 62, 63
  };

  for (unsigned short i=0; i<64; i++) 
    {  
      // Zig-Zag encoding
      addr = zigzagpath_vector[i];
      int16 vectorized = datai.data[addr.slc<3>(3)][addr.slc<3>(0)];
    
      // Quantization
      //int16 quantized  = (short)(vectorized / qvector[LC][i]);
      int16 quantized;
      div(vectorized,(int9)qvector[LC][i],quantized);//ac_int cordic divider
      // Find last non-zero value
      if (quantized) {
        last_nzi = i;
      }
    
      // Write stream of quantized values
      datao.write(quantized);
    }
    
  // Write out index of last non-zero value
  last_non_zero_index.write(last_nzi);
}

// ---------------------------------------------------
// Huffman encode
void encode(
            ac_channel<uint2>   &blocktype,                   // input  block type, Y, Cr, Cb
            ac_channel<short>   &datai,                   // input  block - quantize output, huffman input
            ac_channel<uint6>   &last_non_zero_index,     // input  index of the last non-zero value in the input block
            ac_channel<codes_t> &codes )                  // output huffman codes
{
  
  // previous dc coefficient values for the 3 picture elements from last block
  static int dc[3] = {0, 0, 0};

  uint6  num_zeros = 0;
  bool   new_code;
  uint6  huf_size;
  uint32 huf_code;
  uint6  huf_lnzi = last_non_zero_index.read();
  uint2  type = blocktype.read();
  
  // Luma/Chroma block
  uint1 LC = (type==LUMA)?LUMA:CHROMA;


  // ---------------------------------------------
  // Huffman encoding
  huffmanization:
  for (unsigned short i=0; i<64; i++) {
    int value = datai.read();
    if (i==0) {
      // ---------------------------------------------
      // Huffman encode the DC Coefficient
      int dcval         = value;
      int diff          = dcval - dc[type];
      dc[type]   = dcval;

      huf_code = huffencode(huffcodes[LC][DC],diff);
      huf_size = huffencode(huffsizes[LC][DC],diff);
    } else {
      // ---------------------------------------------
      // Huffman encode the AC Coefficient
      if (value) {
        // if data is not 0...
        // ...insert the (num_zeros,block[i]) code
        new_code   = true;
      } else {
        // if data is 0...
        if (num_zeros==15 && i<huf_lnzi) {
          // if we found 16 consecutive zeros
          // and if we are not yet in the last block of zero's
          // ...insert the special (15,0) code
          new_code   = true;
        } else if (i==63) {
          // if we reached the end of the last block of zero's
          // ...insert the special (0,0) code
          new_code   = true;
        } else {
          // else keep counting zero's !
          new_code   = false;
        }
      }
      huf_code  = (new_code) ? huffencode(huffcodes[LC][AC],num_zeros,value) : 0;
      huf_size  = (new_code) ? huffencode(huffsizes[LC][AC],num_zeros,value) : 0;
      num_zeros = (new_code || (i>=huf_lnzi)) ? 0 : (int)num_zeros+1;
    }
    
    //  huf_code : the code        (the bits)
    //  huf_size : the code length (the number of bits)
    codes.write(codes_t(huf_size, huf_code));
  }
}

void pixelpipe(
               ac_channel<uint2>   &blocktype,           // input  block type, Y, Cr, Cb
               ac_channel<rgb_t>   &rgb,                 // input  block
               ac_channel<codes_t> &codes )              // output huffman codes
{
  static ac_channel<ac_int<8> >    converted;
  static ac_channel<memStruct> transformed;
  static ac_channel<short>   quantized;
  static ac_channel<uint6>   last_non_zero_index;
  static ac_channel<uint2>   blocktype1;
  static ac_channel<uint2>   blocktype2;
  
  // convert 8x8 RGB block to 8x8 YCbCr
  convert(blocktype, blocktype1, rgb, converted);
          
  // run 2D 8x8 DCT on the block
  dct(converted, transformed);

  // zig-zag and quantize the results
  quantize(blocktype1, blocktype2, transformed, quantized, last_non_zero_index);

  // run-length and Huffman encode
  encode(blocktype2, quantized, last_non_zero_index, codes);
}

