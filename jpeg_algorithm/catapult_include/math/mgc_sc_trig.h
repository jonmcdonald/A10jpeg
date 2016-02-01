/*
// -*-mode:c++-*- 
//
////////////////////////////////////////////////////////////////////////////////
// Catapult Synthesis
// 
// Copyright (c) 2003-2011 Calypto Design Systems, Inc.
//       All Rights Reserved
// 
// This document contains information that is proprietary to Calypto Design
// Systems, Inc. The original recipient of this document may duplicate this  
// document in whole or in part for internal business purposes only, provided  
// that this entire notice appears in all copies. In duplicating any part of  
// this document, the recipient agrees to make every reasonable effort to  
// prevent the unauthorized use and distribution of the proprietary information.
//
////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//  Source:         mgc_sc_trig.h
//  Description:    synthesizable sin/cos functions for SystemC
//                  fixed point datatypes
/////////////////////////////////////////////////////////////////////////// */

#ifndef __MGC_SC_TRIG_H
#define __MGC_SC_TRIG_H

#ifndef __cplusplus
#error C++ is required to include this header file
#endif

#include <systemc.h>

#ifdef __SYNTHESIS__
// for generating the tables
#undef MGC_SC_TRIG_GEN_TABLES
// for generating tables or for validating tables (simulation) 
#undef MGC_SC_TRIG_USE_MATH_LIB
#endif

// The computation of the K table using double arithmetic
//  limits what practical TE could be chosen. 

#define TE 70 
#ifdef MGC_SC_TRIG_GEN_TABLES
#define MGC_SC_TRIG_USE_MATH_LIB 
#endif

#ifdef MGC_SC_TRIG_USE_MATH_LIB
#include <math.h>
#else
#ifndef M_PI
#define M_PI            3.14159265358979323846
#endif
#endif

typedef sc_fixed<TE+2,1,SC_RND,SC_WRAP> table_t;

static table_t atan_pow2_table[] = {
#ifndef MGC_SC_TRIG_USE_MATH_LIB
#include "mgc_sc_atan_pow2.tab"
#endif
};

// Scaled atan
static table_t atan_pi_pow2_table[] = {
#ifndef MGC_SC_TRIG_USE_MATH_LIB
#include "mgc_sc_atan2_pow2.tab"
#endif
};

static table_t K_table[] = {
#ifndef MGC_SC_TRIG_USE_MATH_LIB
#include "mgc_sc_K_cordic.tab"
#endif
};

static table_t atan_2mi(int i) {
   if (i >= TE)
     return 0;
#ifndef MGC_SC_TRIG_USE_MATH_LIB 
   return atan_pow2_table[i];
#else
   return atan(pow(2,-i));
#endif
}

static table_t atan_pi_2mi(int i) {
   if (i >= TE)
     return 0;
#ifndef MGC_SC_TRIG_USE_MATH_LIB 
   return atan_pi_pow2_table[i];
#else
   return atan(pow(2,-i))/M_PI;
#endif
}

static table_t K(int n) {
   if (n >= TE)
     return 0;
#ifndef MGC_SC_TRIG_USE_MATH_LIB 
   return K_table[n];
#else
   double a = 1;
   for(int i=0; i < n; i++)
     a *= 1 + pow(2, -2*i);
   return 1/sqrt(a); 
#endif
}

// Function:  C*sin(a*PI), C*cos(a*PI)
// Inputs: 
//   - angle scaled by PI,
//   - scaling factor C
// Outputs:
//   - sin
//   - cos
//
// Overview: angle is expected as radians scaled by 1/PI
// This assumption makes it easy to determine what quadrant
//   the angle is in. Also it saves a multiplication in
//   a typical call.  For instance instead of 2*PI*i/n,
//   we call it with 2*i/n.

template< int AW, int AI, sc_q_mode AQ, sc_o_mode AO, 
          int OW, int OI, sc_q_mode OQ, sc_o_mode OO >
void sin_cos(
  sc_fixed<AW,AI,AQ,AO> angle_over_pi,  
  sc_fixed<OW,OI,OQ,OO> C, 
  sc_fixed<OW,OI,OQ,OO> &sin,  
  sc_fixed<OW,OI,OQ,OO> &cos
){
  // Number of iterations depends on output precision OW
  const int N_I  = (OW+1);

  // assume maximal N_I of 127 
  const int LOG_N_I = (N_I < 16) ? 4 : (N_I < 32) ? 5 : (N_I < 64) ? 6 : 7;
  // Precision for internal computation: n + log(n)
  const int ICW = (N_I + LOG_N_I);

  typedef sc_fixed<ICW,OI> fx_xy;

  fx_xy x, y;
  
  // Accumulator may actually swing slightly out of target range
  //  -1 to 1  (-PI to PI). So make the range -2 to 2. Thus 2
  //  bits for integer part
  // Precision for angle accumulator: n + log(n) + 2 + 2 
  sc_fixed<ICW+4,2> acc_a;

  // throw away upper bits to make target_a in correct range: 
  //    -1 <= target_a < 1   (-PI <= angle < PI)
  sc_fixed<AW-(AI-1),1> target_a = angle_over_pi;

  fx_xy k = K(N_I); 
  fx_xy K_x_scale = C * k;

  if(target_a > 0.5) {   // PI/2 <= angle < PI/2
    x = 0;
    y = K_x_scale;
    acc_a = 0.5;
  }
  else if (target_a < -0.5) {   // -PI <= angle < -PI/2 
    x = 0;
    y = -K_x_scale;
    acc_a = -0.5;
  }
  else {   // -PI/2 <= angle <= PI/2
    x = K_x_scale;
    y = 0;
    acc_a = 0;
  }
  
  for(int i = 0; i < N_I; i++) {
    sc_fixed<ICW+1,1> d_a = atan_pi_2mi(i); 
    fx_xy x_2mi = x;
    x_2mi >>= i;  // x * pow(2, -i)
    fx_xy y_2mi = y;
    y_2mi >>= i;  // y * pow(2, -i)
    if(target_a < acc_a) {
      x += y_2mi;
      y -= x_2mi; 
      acc_a -= d_a;
    }
    else {
      x -= y_2mi; 
      y += x_2mi; 
      acc_a += d_a;
    }
  }
  sin = y;
  cos = x;
}

template< int AW, int AI, sc_q_mode AQ, sc_o_mode AO, 
          int OW, int OI, sc_q_mode OQ, sc_o_mode OO >
void sin(
  sc_fixed<AW,AI,AQ,AO> angle_over_pi,  
  sc_fixed<OW,OI,OQ,OO> &sin
){
  sc_fixed<OW,OI,OQ,OO> scale = 1;
  sc_fixed<OW,OI,OQ,OO> cos;
  sin_cos(angle_over_pi, scale, sin, cos); 
}

template< int AW, int AI, sc_q_mode AQ, sc_o_mode AO, 
          int OW, int OI, sc_q_mode OQ, sc_o_mode OO >
void cos(
  sc_fixed<AW,AI,AQ,AO> angle_over_pi,  
  sc_fixed<OW,OI,OQ,OO> &cos
){
  sc_fixed<OW,OI,OQ,OO> scale = 1;
  sc_fixed<OW,OI,OQ,OO> sin;
  sin_cos(angle_over_pi, scale, sin, cos); 
}

template< int YW, int YI, sc_q_mode YQ, sc_o_mode YO,
          int XW, int XI, sc_q_mode XQ, sc_o_mode XO,
          int OW, int OI, sc_q_mode OQ, sc_o_mode OO >
void atan2(
  sc_fixed<YW,YI,YQ,YO> y,
  sc_fixed<XW,XI,XQ,XO> x,
  sc_fixed<OW,OI,OQ,OO> &atan
){
  // Number of iterations depends on output precision OW
  const int N_I = OW-OI+3;

  // assume maximal N_I of 127 
  const int LOG_N_I = (N_I < 16) ? 4 : (N_I < 32) ? 5 : (N_I < 64) ? 6 : 7;
  // Precision for internal computation: n + log(n)
  const int ICW = (N_I + LOG_N_I);

  typedef sc_fixed<ICW+4,3> fx_a;
  fx_a acc_a = 0;
  bool x_neg = x < 0;

  const int XYI = __MGC_MAX(YI, XI) + 2;
  const int XYW = ICW+4+XYI; 
  typedef sc_fixed<XYW,XYI> fx_xy;

  fx_xy x1 = x_neg ? (fx_xy) -x : (fx_xy) x;
  fx_xy y1 = y; 

  for(int i = 0; i < N_I; i++) {
    sc_fixed<ICW+1,1> d_a = atan_2mi(i);
    fx_xy x_2mi = x1;
    x_2mi >>= i;  // x1 * pow(2, -i)
    fx_xy y_2mi = y1;
    y_2mi >>= i;  // y1 * pow(2, -i)
    if(y1 < 0) {
      x1 -= y_2mi;
      y1 += x_2mi;
      acc_a -= d_a;
    }
    else {
      x1 += y_2mi;
      y1 -= x_2mi;
      acc_a += d_a;
    }
  }
  if(y == 0)
    acc_a = x_neg ? fx_a(M_PI) : fx_a(0);
  else if(x == 0)
    acc_a = y < 0 ? fx_a(-M_PI/2) : fx_a(M_PI/2);
  else if(x_neg)
    acc_a = (y < 0 ? fx_a(-M_PI) : fx_a(M_PI)) - acc_a;
  atan = acc_a;
}

//////////////////////////////////////////////////////////////////////////////
#endif // __MGC_SC_TRIG_H
