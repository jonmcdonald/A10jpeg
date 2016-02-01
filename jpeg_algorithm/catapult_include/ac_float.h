/* -*-mode:c++-*- *********************************************************
 *                                                                        *
 *  Algorithmic C (tm) Datatypes                                          *
 *                                                                        *
 *  Software Version: 3.4                                                 *
 *                                                                        *
 *  Release Date    : Wed Jul 22 21:51:58 PDT 2015                        *
 *  Release Type    : Production Release                                  *
 *  Release Build   : 3.4.0                                               *
 *                                                                        *
 *  Copyright 2013, Calypto Design Systems, Inc.,                     *
 *                                                                        *
 *  All Rights Reserved.                                                  *
 *                                                                        *
 **************************************************************************
 *                                                                        *
 *  The most recent version of this package can be downloaded from:       *
 *     http://www.calypto.com/ac_datatypes.php                            *
 *                                                                        *
 **************************************************************************
 *                                                                        *
 *  IMPORTANT - THIS SOFTWARE IS COPYRIGHTED AND SUBJECT TO LICENSE       *
 *  RESTRICTIONS                                                          *
 *                                                                        *
 *  THE LICENSE THAT CONTROLS YOUR USE OF THE SOFTWARE IS:                *
 *     ALGORITHMIC C DATATYPES END-USER LICENSE AGREEMENT                 *
 *                                                                        *
 *  THESE COMMENTS ARE NOT THE LICENSE.  PLEASE CONSULT THE FULL LICENSE  *
 *  FOR THE ACTUAL TERMS AND CONDITIONS WHICH IS LOCATED AT THE BOTTOM    *
 *  OF THIS FILE.                                                         *
 *                                                                        *  
 *  CAREFULLY READ THE LICENSE AGREEMENT BEFORE USING THE SOFTWARE.       *
 *                                                                        *  
 *       *** MODIFICATION OF THE SOFTWARE IS NOT AUTHORIZED ***           *
 *                                                                        *
 **************************************************************************
 *                                                                        *
 *  YOUR USE OF THE SOFTWARE INDICATES YOUR COMPLETE AND UNCONDITIONAL    *
 *  ACCEPTANCE OF THE TERMS AND CONDITIONS SET FORTH IN THE LICENSE. IF   *
 *  YOU DO NOT  AGREE TO THE LICENSE TERMS AND CONDITIONS, DO NOT USE THE *
 *  SOFTWARE, REMOVE IT FROM YOUR SYSTEM, AND DESTROY ALL COPIES.         *
 *                                                                        *
 *************************************************************************/

//  Source:         ac_float.h
//  Description:    class for floating point operation handling in C++ 
//  Author:         Andres Takach, Ph.D.

#ifndef __AC_FLOAT_H
#define __AC_FLOAT_H

#include <ac_fixed.h>

#ifndef __SYNTHESIS__
#include <cmath>
#endif

#if (defined(__GNUC__) && __GNUC__ < 3 && !defined(__EDG__))
#error GCC version 3 or greater is required to include this header file
#endif

#if (defined(_MSC_VER) && _MSC_VER < 1400 && !defined(__EDG__))
#error Microsoft Visual Studio 8 or newer is required to include this header file
#endif

#if (defined(_MSC_VER) && !defined(__EDG__))
#pragma warning( push )
#pragma warning( disable: 4003 4127 4308 4365 4514 4800 )
#endif

// for safety
#if (defined(E) || defined(WF) || defined(IF) || defined(SF))
#error One or more of the following is defined: E, WF, IF, SF. Definition conflicts with their usage as template parameters. 
#error DO NOT use defines before including third party header files.
#endif


#define __AC_FLOAT_NO_UNSIGNED

#ifdef __AC_FLOAT_NO_UNSIGNED
#define AC_FL(v) ac_float<W##v,I##v,E##v,Q##v>
#define AC_FL0(v) ac_float<W##v,I##v,E##v>
#define AC_FL_T(v) int W##v, int I##v, int E##v, ac_q_mode Q##v
#define AC_FL_TV(v) W##v, I##v, E##v, Q##v
#define AC_FL_T0(v) int W##v, int I##v, int E##v
#define AC_FL_TV0(v) W##v, I##v, E##v
#else
#define AC_FL(v) ac_float<W##v,I##v,E##v,S##v,Q##v>
#define AC_FL0(v) ac_float<W##v,I##v,E##v,S##v>
#define AC_FL_T(v) int W##v, int I##v, int E##v, bool S##v, ac_q_mode Q##v
#define AC_FL_TV(v) W##v, I##v, E##v, S##v, Q##v
#define AC_FL_T0(v) int W##v, int I##v, int E##v, bool S##v
#define AC_FL_TV0(v) W##v, I##v, E##v, S##v
#endif

#ifdef __AC_NAMESPACE
namespace __AC_NAMESPACE {
#endif

#ifdef __AC_FLOAT_NO_UNSIGNED
template<int W, int I, int E, ac_q_mode Q=AC_TRN> class ac_float;
#else
template<int W, int I, int E, bool S=true, ac_q_mode Q=AC_TRN> class ac_float;
#endif

namespace ac {
  template<typename T>
  struct rt_ac_float_T {
    template< AC_FL_T0() >
    struct op1 {
      typedef AC_FL0() fl_t;
      typedef typename T::template rt_T<fl_t>::mult mult;
      typedef typename T::template rt_T<fl_t>::plus plus;
      typedef typename T::template rt_T<fl_t>::minus2 minus;
      typedef typename T::template rt_T<fl_t>::minus minus2;
      typedef typename T::template rt_T<fl_t>::logic logic;
      typedef typename T::template rt_T<fl_t>::div2 div;
      typedef typename T::template rt_T<fl_t>::div div2;
    };
  };
  // specializations after definition of ac_float

  typedef ac_float<54,1,11> ac_float_cdouble_t;
  typedef ac_float<25,1,8> ac_float_cfloat_t;
}

namespace ac_private {
  inline ac::ac_float_cdouble_t double_to_ac_float(double d);
  inline ac::ac_float_cfloat_t float_to_ac_float(float f);
};

//////////////////////////////////////////////////////////////////////////////
//  ac_float
//////////////////////////////////////////////////////////////////////////////

template< AC_FL_T() >
class ac_float {
#ifdef __AC_FLOAT_NO_UNSIGNED
  enum { NO_UN = true, S = true, S2 = true, SR = true };
#else
  enum { NO_UN = true, S = true };
#endif
public:
  typedef ac_fixed<W,I,S> mant_t; 
  typedef ac_int<E,true> exp_t;
  mant_t m;
  exp_t e;

  void set_mantissa(const ac_fixed<W,I,S> &man) { m = man; }
  void set_exp(const ac_int<E,true> &exp) { if(E) e = exp; }

private:
  inline bool is_neg() const { return m < 0; }   // is_neg would be more efficient

  enum {NZ_E = !!E, MIN_EXP = -NZ_E << (E-NZ_E), MAX_EXP = (1 << (E-NZ_E))-1};

public:
  static const int width = W;
  static const int i_width = I;
  static const int e_width = E;
  static const bool sign = S;
  static const ac_q_mode q_mode = Q;


  template< AC_FL_T0(2) >
  struct rt {
    enum {
      // need to validate 
      F=W-I,
      F2=W2-I2,
      mult_w = W+W2,
      mult_i = I+I2,
      mult_e = AC_MAX(E,E2)+1,
      mult_s = S||S2,
      plus_w = AC_MAX(I+(S2&&!S),I2+(S&&!S2))+1+AC_MAX(F,F2),
      plus_i = AC_MAX(I+(S2&&!S),I2+(S&&!S2))+1,
      plus_e = AC_MAX(E,E2),
      plus_s = S||S2,
      minus_w = AC_MAX(I+(S2&&!S),I2+(S&&!S2))+1+AC_MAX(F,F2),
      minus_i = AC_MAX(I+(S2&&!S),I2+(S&&!S2))+1,
      minus_e = AC_MAX(E,E2),
      minus_s = true,
      div_w = W+AC_MAX(W2-I2,0)+S2,
      div_i = I+(W2-I2)+S2,
      div_e = AC_MAX(E,E2)+1,
      div_s = S||S2,
      logic_w = AC_MAX(I+(S2&&!S),I2+(S&&!S2))+AC_MAX(F,F2),
      logic_i = AC_MAX(I+(S2&&!S),I2+(S&&!S2)),
      logic_s = S||S2,
      logic_e = AC_MAX(E,E2)
    };
#ifdef __AC_FLOAT_NO_UNSIGNED
    typedef ac_float<mult_w, mult_i, mult_e> mult;
    typedef ac_float<plus_w, plus_i, plus_e> plus;
    typedef ac_float<minus_w, minus_i, minus_e> minus;
    typedef ac_float<logic_w, logic_i, logic_e> logic;
    typedef ac_float<div_w, div_i, div_e> div;
#else
    typedef ac_float<mult_w, mult_i, mult_e, mult_s> mult;
    typedef ac_float<plus_w, plus_i, plus_e, plus_s> plus;
    typedef ac_float<minus_w, minus_i, minus_e, minus_s> minus;
    typedef ac_float<logic_w, logic_i, logic_e, logic_s> logic;
    typedef ac_float<div_w, div_i, div_e, div_s> div;
#endif
    typedef ac_float arg1;

  };

  template<int WI, bool SI>
  struct rt_i {
    enum {
      lshift_w = W,
      lshift_i = I,
      lshift_s = S,
      lshift_e_0 = exp_t::template rt<WI,SI>::plus::width,
      lshift_e = AC_MIN(lshift_e_0, 24), 
      rshift_w = W,
      rshift_i = I,
      rshift_s = S,
      rshift_e_0 = exp_t::template rt<WI,SI>::minus::width,
      rshift_e = AC_MIN(rshift_e_0, 24)
    };
#ifdef __AC_FLOAT_NO_UNSIGNED
    typedef ac_float<lshift_w, lshift_i, lshift_e> lshift;
    typedef ac_float<rshift_w, rshift_i, rshift_e> rshift;
#else
    typedef ac_float<lshift_w, lshift_i, lshift_e, lshift_s> lshift;
    typedef ac_float<rshift_w, rshift_i, rshift_e, rshift_s> rshift;
#endif
  }; 

  template<typename T>
  struct rt_T {
    typedef typename ac::map<T>::t map_T;
    typedef typename ac::rt_ac_float_T<map_T>::template op1< AC_FL_TV0() >::mult mult;
    typedef typename ac::rt_ac_float_T<map_T>::template op1< AC_FL_TV0() >::plus plus;
    typedef typename ac::rt_ac_float_T<map_T>::template op1< AC_FL_TV0() >::minus minus;
    typedef typename ac::rt_ac_float_T<map_T>::template op1< AC_FL_TV0() >::minus2 minus2;
    typedef typename ac::rt_ac_float_T<map_T>::template op1< AC_FL_TV0() >::logic logic;
    typedef typename ac::rt_ac_float_T<map_T>::template op1< AC_FL_TV0() >::div div;
    typedef typename ac::rt_ac_float_T<map_T>::template op1< AC_FL_TV0() >::div2 div2;
    typedef ac_float arg1;
  };

  template<typename T>
  struct rt_T2 {
    typedef typename ac::map<T>::t map_T;
    typedef typename ac::rt_ac_float_T<map_T>::template op1< AC_FL_TV0() >::mult mult;
    typedef typename ac::rt_ac_float_T<map_T>::template op1< AC_FL_TV0() >::plus plus;
    typedef typename ac::rt_ac_float_T<map_T>::template op1< AC_FL_TV0() >::minus2 minus;
    typedef typename ac::rt_ac_float_T<map_T>::template op1< AC_FL_TV0() >::minus minus2;
    typedef typename ac::rt_ac_float_T<map_T>::template op1< AC_FL_TV0() >::logic logic;
    typedef typename ac::rt_ac_float_T<map_T>::template op1< AC_FL_TV0() >::div2 div;
    typedef typename ac::rt_ac_float_T<map_T>::template op1< AC_FL_TV0() >::div div2;
    typedef ac_float arg1;
  };

  struct rt_unary {
    enum {
      neg_w = W+1,
      neg_i = I+1,
      neg_e = E,
      neg_s = true,
      mag_sqr_w = 2*W-S + NO_UN,
      mag_sqr_i = 2*I-S + NO_UN,
      mag_sqr_e = E,
      mag_sqr_s = false | NO_UN,
      mag_w = W+S + NO_UN,
      mag_i = I+S + NO_UN,
      mag_e = E,
      mag_s = false | NO_UN,
      to_fx_i = I + MAX_EXP,
      to_fx_w = W + MAX_EXP - MIN_EXP,
      to_fx_s = S,
      to_i_w = AC_MAX(to_fx_i,1),
      to_i_s = S
    };
#ifdef __AC_FLOAT_NO_UNSIGNED
    typedef ac_float<neg_w, neg_i, neg_e> neg;
    typedef ac_float<mag_sqr_w, mag_sqr_i, mag_sqr_e> mag_sqr;
    typedef ac_float<mag_w, mag_i, mag_e> mag;
    template<unsigned N>
    struct set {
      enum { sum_w = W + ac::log2_ceil<N>::val, sum_i = (sum_w-W) + I, sum_e = E, sum_s = S};
      typedef ac_float<sum_w, sum_i, sum_e> sum;
    };
#else
    typedef ac_float<neg_w, neg_i, neg_e, neg_s> neg;
    typedef ac_float<mag_sqr_w, mag_sqr_i, mag_sqr_e, mag_sqr_s> mag_sqr;
    typedef ac_float<mag_w, mag_i, mag_e, mag_s> mag;
    template<unsigned N>
    struct set {
      enum { sum_w = W + ac::log2_ceil<N>::val, sum_i = (sum_w-W) + I, sum_e = E, sum_s = S};
      typedef ac_float<sum_w, sum_i, sum_e, sum_s> sum;
    };
#endif
    typedef ac_fixed<to_fx_w, to_fx_i, to_fx_s> to_ac_fixed_t;
    typedef ac_int<to_i_w, to_i_s> to_ac_int_t;
  };

  template<AC_FL_T(2)> friend class ac_float;

  ac_float() {
#if defined(AC_DEFAULT_IN_RANGE)
#endif
  }
  ac_float(const ac_float &op) {
    m = op.m;
    e = op.e;
  }

  template<AC_FL_T(2)>
  ac_float(const AC_FL(2) &op, bool assert_on_overflow=false, bool assert_on_rounding=false) {
    typedef AC_FL(2) fl2_t;
    enum {
           ST2 = S|S2, WT2 = W2+(!S2&S), IT2 = I2+(!S2&S),
           RND = Q!=AC_TRN & Q!=AC_TRN_ZERO & WT2 > W,
           MAX_EXP2 = fl2_t::MAX_EXP, MIN_EXP2 = fl2_t::MIN_EXP,
           I_DIFF = IT2-I,
           MAX_EXP_T = MAX_EXP2 + I_DIFF + RND, MIN_EXP_T = MIN_EXP2 + I_DIFF,
           MAX_EXP_T_P = MAX_EXP_T < 0 ? ~ MAX_EXP_T : MAX_EXP_T,
           MIN_EXP_T_P = MIN_EXP_T < 0 ? ~ MIN_EXP_T : MIN_EXP_T,
           WC_EXP_T = AC_MAX(MAX_EXP_T_P, MIN_EXP_T_P),
           ET = ac::template nbits<WC_EXP_T>::val + 1 };

    typedef ac_fixed<WT2,IT2,ST2> fx2_t;
    typedef ac_fixed<WT2,I,S> fx_t;

    fx2_t m2 = op.m;
    fx_t t;
    t.set_slc(0, m2.template slc<fx_t::width>(0));
    ac_fixed<W+RND,I+RND,S,Q> m_1 = t;
    bool rnd_ovfl = false;
    if(RND) {
      rnd_ovfl = !m_1[W] & m_1[W-1];
      m_1[W-1] = m_1[W-1] & !rnd_ovfl;
      m_1[W-2] = m_1[W-2] | rnd_ovfl;
    }
    m.set_slc(0, m_1.template slc<W>(0));
    if(fx_t::width > W && assert_on_rounding)
      AC_ASSERT(m == t, "Loss of precision due to Rounding in ac_float constructor");
    ac_int<ET,true> exp = !m ? ac_int<ET,true> (0) :
      ac_int<ET,true> (op.e + I_DIFF + (RND & rnd_ovfl));
    adjust(exp, false, assert_on_overflow);
  }

  ac_float(const ac_fixed<W,I,S> &m2, const ac_int<E,true> &e2, bool normalize=true) {
    m = m2;
    if(E) {
      if(!m)
        e = 0;
      else { 
        e = e2;
        if(normalize)
          m.normalize(e);
      }
    }
  }

  template<int WFX, int IFX, bool SFX, int E2>
  ac_float(const ac_fixed<WFX,IFX,SFX> &m2, const ac_int<E2,true> &e2, bool normalize=true) {
#ifdef __AC_FLOAT_NO_UNSIGNED
    enum { WF2 = WFX+!SFX, IF2 = IFX+!SFX };
    ac_float<WF2,IF2,E2>  f(ac_fixed<WF2,IF2,true>(m2), e2, normalize); 
#else
    ac_float<WFX,IFX,E2,SFX>  f(m2, e2, normalize); 
#endif
    *this = f;
  }

  template<int WFX, int IFX, bool SFX, ac_q_mode QFX, ac_o_mode OFX>
  ac_float(const ac_fixed<WFX,IFX,SFX,QFX,OFX> &op, bool normalize=true) {
   enum { 
#ifdef __AC_FLOAT_NO_UNSIGNED
      U2S = !SFX,
#else
      U2S = 0,
#endif
      WT2 = WFX+U2S, IT2 = IFX+U2S,
      RND = QFX!=AC_TRN & QFX!=AC_TRN_ZERO & WT2 > W,
      I_DIFF = IT2-I,
      MAX_EXP_T = -(I_DIFF + RND), MIN_EXP_T = AC_MAX(MIN_EXP - I_DIFF, -WT2+1),
      MAX_EXP_T_P = MAX_EXP_T < 0 ? ~ MAX_EXP_T : MAX_EXP_T,
      MIN_EXP_T_P = MIN_EXP_T < 0 ? ~ MIN_EXP_T : MIN_EXP_T,
      WC_EXP_T = AC_MAX(MAX_EXP_T_P, MIN_EXP_T_P),
      ET = ac::template nbits<WC_EXP_T>::val + 1
    };
    ac_float<WT2,IT2,ET>  f(op, ac_int<ET,true>(0), normalize);
    *this = f; 
  }

  template<int WI, bool SI>
  ac_float(const ac_int<WI,SI> &op) {
    *this = ac_fixed<WI,WI,SI>(op); 
  }

  inline ac_float( bool b ) { *this = (ac_int<1,false>) b; }
  inline ac_float( char b ) { *this = (ac_int<8,true>) b; }
  inline ac_float( signed char b ) { *this = (ac_int<8,true>) b; }
  inline ac_float( unsigned char b ) { *this = (ac_int<8,false>) b; }
  inline ac_float( signed short b ) { *this = (ac_int<16,true>) b; }
  inline ac_float( unsigned short b ) { *this = (ac_int<16,false>) b; }
  inline ac_float( signed int b ) { *this = (ac_int<32,true>) b; }
  inline ac_float( unsigned int b ) { *this = (ac_int<32,false>) b; }
  inline ac_float( signed long b ) { *this = (ac_int<ac_private::long_w,true>) b; }
  inline ac_float( unsigned long b ) { *this = (ac_int<ac_private::long_w,false>) b; }
  inline ac_float( Slong b ) { *this = (ac_int<64,true>) b; }
  inline ac_float( Ulong b ) { *this = (ac_int<64,false>) b; }

  // Explicit conversion functions to ac_int and ac_fixed
  inline typename rt_unary::to_ac_fixed_t to_ac_fixed() const {
    typename rt_unary::to_ac_fixed_t r = m;
    r <<= e; 
    return r;
  }
  inline typename rt_unary::to_ac_int_t to_ac_int() const {
    return to_ac_fixed().to_ac_int(); 
  }

  // Explicit conversion functions to C built-in types -------------
  inline int to_int() const { return to_ac_int().to_int(); }
  inline unsigned to_uint() const { return to_ac_int().to_uint(); }
  inline long to_long() const { return (signed long) to_ac_int().to_int64(); }
  inline unsigned long to_ulong() const { return (unsigned long) to_ac_int().to_uint64(); }
  inline Slong to_int64() const { return to_ac_int().to_int64(); }
  inline Ulong to_uint64() const { return to_ac_int().to_uint64(); }
  inline float to_float() const { return ldexpf(m.to_double(), exp()); }
  inline double to_double() const { return ldexp(m.to_double(), exp()); }

  const ac_fixed<W,I,S> mantissa() const { return m; }
  const ac_int<E,true> exp() const { return e; }
  bool normalize() {
    bool normalized = operator !() || !S && m[W-1] || S && (m[W-1] ^ m[W-2]);
    if(E && !normalized)
      normalized = m.normalize(e);
    return normalized;
  }

  template <int ET>
  void adjust(ac_int<ET,true> new_e, bool normalize, bool assert_on_overflow) {
    if(E >= ET) {
      e = new_e;
      if(E && normalize)
        m.normalize(e);
    } else {
      ac_int<E,false> offset = 0;
      offset[E-1] = 1;
      ac_int<ET+1,true> e_s = new_e + offset; 
      if(e_s < 0) {
        m >>= (MIN_EXP - new_e);   // can a full barrel-shifter be avoided ???
        e = MIN_EXP;
      } else {
        // break down:  bits( (1<<E) -1 + W-1 ) + 1 is max bit for normalization
        // other bits can be tested separetely
        ac_int<ET,false> e_u = e_s;
        if(ET && normalize)
          m.normalize(e_u);
        e_u -= offset;
        if(e_u[ET-1] | !(e_u >> (E-1)))   // what about E == 0 or ET == 0 ???
          e = e_u;
        else {
          e = MAX_EXP;
          m = m < 0 ? value<AC_VAL_MIN>(m) : value<AC_VAL_MAX>(m);
          if(assert_on_overflow)
            AC_ASSERT(0, "OVERFLOW ON ASSIGNMENT TO AC_FLOAT");
        }
      }
    }
  }

  ac_float( double d, bool assert_on_overflow=false, bool assert_on_rounding=false ) {
    enum { I_EXT = AC_MAX(I,1), W_EXT = ac::ac_float_cdouble_t::width + I_EXT - 1,  };
    ac::ac_float_cdouble_t t = ac_private::double_to_ac_float(d);
    ac_float r(t, assert_on_overflow, assert_on_rounding);
    *this = r;
  }

  ac_float( float f, bool assert_on_overflow=false, bool assert_on_rounding=false ) {
    enum { I_EXT = AC_MAX(I,1), W_EXT = ac::ac_float_cfloat_t::width + I_EXT - 1,  };
    ac::ac_float_cfloat_t t = ac_private::float_to_ac_float(f);
    ac_float r(t, assert_on_overflow, assert_on_rounding);
    *this = r;
  }

  template<AC_FL_T(2)>
  typename rt< AC_FL_TV0(2) >::mult operator *(const AC_FL(2) &op2) const {
    typename rt< AC_FL_TV0(2) >::mult r(m*op2.m, exp()+op2.exp(), false);
    return r;
  }

  template<AC_FL_T(2)>
  bool compare(const AC_FL(2) &op2, bool *gt) const {
    typedef ac_fixed<W2,I,S2> fx2_t;
    typedef typename ac_fixed<W,I,S>::template rt_T< fx2_t >::logic fx_t; 
    typedef ac_fixed<fx_t::width,fx_t::i_width,false> fxu_t;

    fx2_t op2_m_0;
    op2_m_0.set_slc(0, op2.m.template slc<W2>(0));

    fx_t op1_m = m;
    fx_t op2_m = op2_m_0;
    int e_dif = exp() - op2.exp() + I - I2; 
    bool op2_m_neg = op2_m[fx_t::width-1];
    fx_t out_bits = op2_m ^ ((op2_m_neg & e_dif < 0) ? ~fx_t(0) : fx_t(0));
    out_bits &= ~(fxu_t(~fxu_t(0)) << e_dif);
    op2_m >>= e_dif;
    bool overflow = e_dif < 0 & !!out_bits | op2_m_neg ^ op2_m[fx_t::width-1]; 

    *gt = overflow & op2_m_neg | !overflow & op1_m > op2_m;
    bool eq = op1_m == op2_m & !overflow & !out_bits;
    return eq; 
  }


  template<AC_FL_T(2), AC_FL_T(R)>
  void plus_minus(const AC_FL(2) &op2, AC_FL(R) &r, bool sub=false) const {
    typedef AC_FL(R) r_type;
    enum { IT = AC_MAX(I,I2) };
    typedef ac_fixed<W, IT, S> fx1_t;
    typedef ac_fixed<W2, IT, S2> fx2_t;
    typedef typename fx1_t::template rt_T< ac_fixed<WR,IT,SR> >::logic fx1t_t; 
    typedef typename fx2_t::template rt_T< ac_fixed<WR,IT,SR> >::logic fx2t_t; 
    typedef typename fx1t_t::template rt_T<fx2t_t>::plus mt_t; 
    typedef ac_fixed<mt_t::width+1,mt_t::i_width,SR> t1_t;
    typedef ac_fixed<mt_t::width+2,mt_t::i_width,SR> t2_t;
    typedef ac_fixed<t2_t::width,IR,SR> r1_t;

    enum { MAX_EXP2 = AC_FL(2)::MAX_EXP, MIN_EXP2 = AC_FL(2)::MIN_EXP, 
          I_DIFF = mt_t::i_width - IR,
          MAX_EXP_T = AC_MAX(MAX_EXP, MAX_EXP2) + I_DIFF, 
          MIN_EXP_T = AC_MIN(MIN_EXP, MIN_EXP2) + I_DIFF,
          MAX_EXP_T_P = MAX_EXP_T < 0 ? ~ MAX_EXP_T : MAX_EXP_T,
          MIN_EXP_T_P = MIN_EXP_T < 0 ? ~ MIN_EXP_T : MIN_EXP_T,
          WC_EXP_T = AC_MAX(MAX_EXP_T_P, MIN_EXP_T_P),
          ET = ac::template nbits<WC_EXP_T>::val + 1 };
 
    ac_fixed<mt_t::width, I+1, mt_t::sign> op1_m_0 = m;
    mt_t op1_m = 0;
    op1_m.set_slc(0, op1_m_0.template slc<mt_t::width>(0));
    int op1_e = exp() + I-IT + (SR&!S);

    ac_fixed<mt_t::width, I2+1, mt_t::sign> op2_m_0 = op2.m;
    mt_t op2_m = 0;
    op2_m.set_slc(0, op2_m_0.template slc<mt_t::width>(0));
    if(sub)
      op2_m = -op2_m;
    int op2_e = op2.exp() + I2-IT + (SR&!S2);

    bool op1_zero = operator !();
    bool op2_zero = !op2;
    int e_dif = op1_e - op2_e;
    bool e1_lt_e2 = e_dif < 0;
    e_dif = (op1_zero | op2_zero) ? 0 : e1_lt_e2 ? -e_dif : e_dif;
    mt_t op_sl = e1_lt_e2 ? op1_m : op2_m;
    // Sticky bits are bits shifted out, leaving out the MSB
    mt_t sticky_bits = op_sl;
    sticky_bits &= ~((~t1_t(0)) << e_dif);
    bool sticky_bit = !!sticky_bits;
    bool msb_shifted_out = (bool) t1_t(op_sl).template slc<1>(e_dif);
    op_sl >>= e_dif;
    op1_m = e1_lt_e2 ? op_sl : op1_m; 
    op2_m = e1_lt_e2 ? op2_m : op_sl; 

    t1_t t1 = op1_m;
    t1 += op2_m;
    t1[0] = msb_shifted_out;

    bool shift_r_1 = false; //t1[t1_t::width-1] != t1[t1_t::width-2]; 
    sticky_bit |= shift_r_1 & t1[0];
    t1 >>= shift_r_1;
    t2_t t2 = t1;
    t2[0] = sticky_bit;
    r1_t r1;
    r1.set_slc(0, t2.template slc<t2_t::width>(0));
    r_type r_t; 
    r_t.m = (ac_fixed<WR,IR,SR,QR>) r1; // t2;  This could overflow if !SR&ST
    ac_int<ET,true> r_e = op1_zero ? op2_e : (op2_zero ? op1_e : AC_MAX(op1_e, op2_e));
    r_e = ac_int<1,true>(!op1_zero | !op2_zero) & (r_e + shift_r_1 + I_DIFF); 
    r_t.adjust(r_e, true, false);
    r.m = r_t.m;
    r.e = r_t.e;
  }

  template<AC_FL_T(1), AC_FL_T(2)>
  ac_float add(const AC_FL(1) &op1, const AC_FL(2) &op2) {
    op1.plus_minus(op2, *this);
    return *this;
  }
 
  template<AC_FL_T(1), AC_FL_T(2)>
  ac_float sub(const AC_FL(1) &op1, const AC_FL(2) &op2) {
    op1.plus_minus(op2, *this, true);
    return *this;
  }

  typename rt_unary::neg abs() const {   
    typedef typename rt_unary::neg r_t; 
    r_t r;
    r.m = is_neg() ? -m : r_t::mant_t(m);
    r.e = e;
    return r;
  }
 
#ifdef __AC_FLOAT_ENABLE_ALPHA
  // These will be changed!!! For now only enable to explore integration with ac_complex
  template<AC_FL_T(2)>
  typename rt< AC_FL_TV0(2) >::plus operator +(const AC_FL(2) &op2) const {
    typename rt< AC_FL_TV0(2) >::plus r;
    plus_minus(op2, r); 
    return r;
  }
  template<AC_FL_T(2)>
  typename rt< AC_FL_TV0(2) >::minus operator -(const AC_FL(2) &op2) const {
    typename rt< AC_FL_TV0(2) >::minus r;
    plus_minus(op2, r, true); 
    return r;
  } 
#endif

  template<AC_FL_T(2)>
  typename rt< AC_FL_TV0(2) >::div operator /(const AC_FL(2) &op2) const {
    typename rt< AC_FL_TV0(2) >::div r(m/op2.m, exp()-op2.exp());
    return r;
  } 
  template<AC_FL_T(2)>
  ac_float operator +=(const AC_FL(2) &op2) {
    ac_float r;
    plus_minus(op2, r);
    *this = r; 
  } 
  template<AC_FL_T(2)>
  ac_float operator -=(const AC_FL(2) &op2) {
    ac_float r;
    plus_minus(op2, r, true);
    *this = r; 
  } 
  template<AC_FL_T(2)>
  ac_float operator *=(const AC_FL(2) &op2) {
    *this = *this * op2;
  } 
  template<AC_FL_T(2)>
  ac_float operator /=(const AC_FL(2) &op2) {
    *this = *this / op2;
  } 
  ac_float operator + () const {
    return *this;
  } 
  typename rt_unary::neg operator - () const {
    typename rt_unary::neg r;
    r.m = -m; 
    r.e = e;
    return r; 
  } 
  bool operator ! () const {
    return !m;
  } 

  // Shift --------------------------------------------------------------------
  template<int WI, bool SI>
  typename rt_i<WI,SI>::lshift operator << ( const ac_int<WI,SI> &op2 ) const {
    typename rt_i<WI,SI>::lshift r;
    r.m = m;
    r.e = e + op2;
    return r;
  }
  template<int WI, bool SI>
  typename rt_i<WI,SI>::rshift operator >> ( const ac_int<WI,SI> &op2 ) const {
    typename rt_i<WI,SI>::rshift r;
    r.m = m;
    r.e = e - op2;
    return r;
  }
  // Shift assign -------------------------------------------------------------
  template<int WI, bool SI>
  ac_float &operator <<= ( const ac_int<WI,SI> &op2 ) {
    *this = operator << (op2);
    return *this;
  }
  template<int WI, bool SI>
  ac_float &operator >>= ( const ac_int<WI,SI> &op2 ) {
    *this = operator >> (op2);
    return *this;
  }

  template<AC_FL_T(2)>
  bool operator == (const AC_FL(2) &f) const {
    bool gt;
    return compare(f, &gt);
  } 
  template<AC_FL_T(2)>
  bool operator != (const AC_FL(2) &f) const {
    return !operator == (f); 
  } 
  template<AC_FL_T(2)>
  bool operator < (const AC_FL(2) &f) const {
    bool gt;
    bool eq = compare(f, &gt);
    return !(eq | gt); 
  } 
  template<AC_FL_T(2)>
  bool operator >= (const AC_FL(2) &f) const {
    return !operator < (f);
  } 
  template<AC_FL_T(2)>
  bool operator > (const AC_FL(2) &f) const {
    bool gt;
    compare(f, &gt);
    return gt; 
  } 
  template<AC_FL_T(2)>
  bool operator <= (const AC_FL(2) &f) const {
    return !operator > (f);
  } 
#if 0
  bool operator == ( double d) const {
    return *this == ac_private::double_to_ac_float(d);
  }
  bool operator != ( double d) const {
    return *this != ac_private::double_to_ac_float(d);
  }
  bool operator < ( double d) const {
    return *this < ac_private::double_to_ac_float(d);
  }
  bool operator >= ( double d) const {
    return *this >= ac_private::double_to_ac_float(d);
  }
  bool operator > ( double d) const {
    return *this > ac_private::double_to_ac_float(d);
  }
  bool operator <= ( double d) const {
    return *this <= ac_private::double_to_ac_float(d);
  }
#endif

  inline std::string to_string(ac_base_mode base_rep, bool sign_mag = false, bool hw=true) const {
    // TODO: printing decimal with exponent
    if(!hw) {
      ac_fixed<W,0,S> mantissa;
      mantissa.set_slc(0, m.template slc<W>(0)); 
      std::string r = mantissa.to_string(base_rep, sign_mag);
      r += "e2";
      r += (e + I).to_string(base_rep, sign_mag | base_rep == AC_DEC);
      return r; 
    } else {
      std::string r = m.to_string(base_rep, sign_mag);
      if(base_rep != AC_DEC)
        r += "_";
      r += "e2";
      if(base_rep != AC_DEC)
        r += "_";
      if(E)
        r += e.to_string(base_rep, sign_mag | base_rep == AC_DEC);
      else
        r += "0";
      return r; 
    }
  }

  inline static std::string type_name() {
    const char *tf[] = {"false", "true" };
    const char *q[] = {"AC_TRN", "AC_RND", "AC_TRN_ZERO", "AC_RND_ZERO", "AC_RND_INF", "AC_RND_MIN_INF", "AC_RND_CONV" };
    std::string r = "ac_float<";
    r += ac_int<32,true>(W).to_string(AC_DEC) + ',';
    r += ac_int<32,true>(I).to_string(AC_DEC) + ',';
    r += ac_int<32,true>(E).to_string(AC_DEC) + ',';
    r += tf[S];
    r += ',';
    r += q[Q];
    r += '>';
    return r;
  }

  template<ac_special_val V>
  inline ac_float &set_val() {
    m.set_val<V>();
    if(V == AC_VAL_MIN)
      e.set_val<AC_VAL_MAX>();
    else if(V == AC_VAL_QUANTUM)
      e.set_val<AC_VAL_MIN>();
    else
      e.set_val<V>();
    return *this;
  }
};

namespace ac_private {
  template<typename T>
  bool ac_fpclassify(T x, bool &inf) {
    bool nan = !(x==x);
    if(!nan) {
      T d = x - x;
      inf = !(d==d);
    } 
    return nan;
  }
 
  inline ac::ac_float_cdouble_t double_to_ac_float(double d) {
    typedef ac::ac_float_cdouble_t r_t;
#ifndef __SYNTHESIS__
    bool inf;
    bool nan = ac_fpclassify(d, inf);
    if(nan)
      AC_ASSERT(0, "In conversion from double to ac_float: double is NaN");
    else if(inf)
      AC_ASSERT(0, "In conversion from double to ac_float: double is Infinite");
#endif
    r_t::exp_t exp;
    r_t::mant_t mant = ac::frexp_d(d, exp);
    return r_t(mant, exp, false);   
  }

  inline ac::ac_float_cfloat_t float_to_ac_float(float f) {
    typedef ac::ac_float_cfloat_t r_t;
#ifndef __SYNTHESIS__
    bool inf;
    bool nan = ac_fpclassify(f, inf);
    if(nan)
      AC_ASSERT(0, "In conversion from float to ac_float: float is NaN");
    else if(inf)
      AC_ASSERT(0, "In conversion from float to ac_float: float is Infinite");
#endif
    r_t::exp_t exp;
    r_t::mant_t mant = ac::frexp_f(f, exp);
    return r_t(mant, exp, false);   
  }
};

namespace ac {
  // with T == ac_float
  template< AC_FL_T0(2) >
  struct rt_ac_float_T< AC_FL0(2) > {
    typedef AC_FL0(2) fl2_t;
    typedef AC_FL0(2) equiv;
    template< AC_FL_T0() >
    struct op1 {
      typedef AC_FL0() fl_t;
      typedef typename fl_t::template rt< AC_FL_TV0(2) >::mult mult;
      typedef typename fl_t::template rt< AC_FL_TV0(2) >::plus plus;
      typedef typename fl_t::template rt< AC_FL_TV0(2) >::minus minus;
      typedef typename fl2_t::template rt< AC_FL_TV0() >::minus minus2;
      typedef typename fl_t::template rt< AC_FL_TV0(2) >::logic logic;
      typedef typename fl_t::template rt< AC_FL_TV0(2) >::div div;
      typedef typename fl2_t::template rt< AC_FL_TV0() >::div div2;
    };
  };
  // with T == ac_fixed
  template<int WFX, int IFX, bool SFX>
  struct rt_ac_float_T< ac_fixed<WFX,IFX,SFX> > {
    // For now E2 > 0
#ifdef __AC_FLOAT_NO_UNSIGNED
    enum { E2 = 1, S2 = true, W2 = WFX + !SFX, I2 = IFX + !SFX };
#else
    enum { E2 = 1, S2 = SFX, W2 = WFX, I2 = IFX };
#endif
    typedef AC_FL0(2) fl2_t;
    typedef AC_FL0(2) equiv;
    template< AC_FL_T0() >
    struct op1 {
      typedef AC_FL0() fl_t;
      typedef typename fl_t::template rt< AC_FL_TV0(2) >::mult mult;
      typedef typename fl_t::template rt< AC_FL_TV0(2) >::plus plus;
      typedef typename fl_t::template rt< AC_FL_TV0(2) >::minus minus;
      typedef typename fl2_t::template rt< AC_FL_TV0() >::minus minus2;
      typedef typename fl_t::template rt< AC_FL_TV0(2) >::logic logic;
      typedef typename fl_t::template rt< AC_FL_TV0(2) >::div div;
      typedef typename fl2_t::template rt< AC_FL_TV0() >::div div2;
    };
  };
  // with T == ac_int
  template<int WI, bool SI>
  struct rt_ac_float_T< ac_int<WI,SI> > {
    // For now E2 > 0
#ifdef __AC_FLOAT_NO_UNSIGNED
    enum { E2 = 1, S2 = true, I2 = WI + !SI, W2 = I2 };
#else
    enum { E2 = 1, S2 = SI, I2 = WI, W2 = I2 };
#endif
    typedef AC_FL0(2) fl2_t;
    typedef AC_FL0(2) equiv;
    template< AC_FL_T0() >
    struct op1 {
      typedef AC_FL0() fl_t;
      typedef typename fl_t::template rt< AC_FL_TV0(2) >::mult mult;
      typedef typename fl_t::template rt< AC_FL_TV0(2) >::plus plus;
      typedef typename fl_t::template rt< AC_FL_TV0(2) >::minus minus;
      typedef typename fl2_t::template rt< AC_FL_TV0() >::minus minus2;
      typedef typename fl_t::template rt< AC_FL_TV0(2) >::logic logic;
      typedef typename fl_t::template rt< AC_FL_TV0(2) >::div div;
      typedef typename fl2_t::template rt< AC_FL_TV0() >::div div2;
    };
  };

  // Multiplication is optimizable, general operator +/- is not yet supported
  template<typename T>
  struct rt_ac_float_T< c_type<T> > {
#ifdef __AC_FLOAT_NO_UNSIGNED
    // For now E2 > 0
    enum { SCT = c_type_params<T>::S, S2 = true, W2 = c_type_params<T>::W + !SCT, I2 = c_type_params<T>::I + !SCT, E2 = AC_MAX(1, c_type_params<T>::E) };
#else
    enum { SCT = c_type_params<T>::S, S2 = SCT, W2 = c_type_params<T>::W, I2 = c_type_params<T>::I, E2 = AC_MAX(1, c_type_params<T>::E) };
#endif
    typedef AC_FL0(2) fl2_t;
    typedef AC_FL0(2) equiv;
    template< AC_FL_T0() >
    struct op1 {
      typedef AC_FL0() fl_t;
      typedef typename fl_t::template rt< AC_FL_TV0(2) >::mult mult;
      typedef typename fl_t::template rt< AC_FL_TV0(2) >::plus plus;
      typedef typename fl_t::template rt< AC_FL_TV0(2) >::minus minus;
      typedef typename fl2_t::template rt< AC_FL_TV0() >::minus minus2;
      typedef typename fl_t::template rt< AC_FL_TV0(2) >::logic logic;
      typedef typename fl_t::template rt< AC_FL_TV0(2) >::div div;
      typedef typename fl2_t::template rt< AC_FL_TV0() >::div div2;
    };
  };
}

// Stream --------------------------------------------------------------------

#ifndef __SYNTHESIS__
template<AC_FL_T()>
inline std::ostream& operator << (std::ostream &os, const AC_FL() &x) {
  os << x.to_string(AC_DEC);
  return os;
}
#endif

#define FL_BIN_OP_WITH_CTYPE(BIN_OP, C_TYPE, RTYPE)  \
  template< AC_FL_T() > \
  inline typename AC_FL()::template rt_T2<C_TYPE>::RTYPE operator BIN_OP ( C_TYPE c_op, const AC_FL() &op) {  \
    typedef typename ac::template rt_ac_float_T< ac::c_type<C_TYPE> >::equiv fl2_t; \
    return fl2_t(c_op).operator BIN_OP (op);  \
  } \
  template< AC_FL_T() > \
  inline typename AC_FL()::template rt_T<C_TYPE>::RTYPE operator BIN_OP ( const AC_FL() &op, C_TYPE c_op) {  \
    typedef typename ac::template rt_ac_float_T< ac::c_type<C_TYPE> >::equiv fl2_t; \
    return op.operator BIN_OP (fl2_t(c_op));  \
  }

#define FL_REL_OP_WITH_CTYPE(REL_OP, C_TYPE)  \
  template< AC_FL_T() > \
  inline bool operator REL_OP ( const AC_FL() &op, C_TYPE op2) {  \
    typedef typename ac::template rt_ac_float_T< ac::c_type<C_TYPE> >::equiv fl2_t; \
    return op.operator REL_OP (fl2_t(op2));  \
  }  \
  template< AC_FL_T() > \
  inline bool operator REL_OP ( C_TYPE op2, const AC_FL() &op) {  \
    typedef typename ac::template rt_ac_float_T< ac::c_type<C_TYPE> >::equiv fl2_t; \
    return fl2_t(op2).operator REL_OP (op);  \
  }

#define FL_ASSIGN_OP_WITH_CTYPE_2(ASSIGN_OP, C_TYPE)  \
  template< AC_FL_T() > \
  inline AC_FL() &operator ASSIGN_OP ( AC_FL() &op, C_TYPE op2) {  \
    typedef typename ac::template rt_ac_float_T< ac::c_type<C_TYPE> >::equiv fl2_t; \
    return op.operator ASSIGN_OP (fl2_t(op2));  \
  }

#define FL_OPS_WITH_CTYPE(C_TYPE) \
  FL_BIN_OP_WITH_CTYPE(*, C_TYPE, mult) \
  FL_BIN_OP_WITH_CTYPE(+, C_TYPE, plus) \
  FL_BIN_OP_WITH_CTYPE(-, C_TYPE, minus) \
  FL_BIN_OP_WITH_CTYPE(/, C_TYPE, div) \
  \
  FL_REL_OP_WITH_CTYPE(==, C_TYPE) \
  FL_REL_OP_WITH_CTYPE(!=, C_TYPE) \
  FL_REL_OP_WITH_CTYPE(>, C_TYPE) \
  FL_REL_OP_WITH_CTYPE(>=, C_TYPE) \
  FL_REL_OP_WITH_CTYPE(<, C_TYPE) \
  FL_REL_OP_WITH_CTYPE(<=, C_TYPE) \
  \
  FL_ASSIGN_OP_WITH_CTYPE_2(+=, C_TYPE) \
  FL_ASSIGN_OP_WITH_CTYPE_2(-=, C_TYPE) \
  FL_ASSIGN_OP_WITH_CTYPE_2(*=, C_TYPE) \
  FL_ASSIGN_OP_WITH_CTYPE_2(/=, C_TYPE)

#define FL_SHIFT_OP_WITH_INT_CTYPE(BIN_OP, C_TYPE, RTYPE)  \
  template< AC_FL_T() > \
  inline typename AC_FL()::template rt_i< ac::c_type_params<C_TYPE>::W, ac::c_type_params<C_TYPE>::S >::RTYPE operator BIN_OP ( const AC_FL() &op, C_TYPE i_op) {  \
    typedef typename ac::template rt_ac_int_T< ac::c_type<C_TYPE> >::equiv i_t; \
    return op.operator BIN_OP (i_t(i_op));  \
  }

#define FL_SHIFT_ASSIGN_OP_WITH_INT_CTYPE(ASSIGN_OP, C_TYPE)  \
  template< AC_FL_T() > \
  inline AC_FL() &operator ASSIGN_OP ( AC_FL() &op, C_TYPE i_op) {  \
    typedef typename ac::template rt_ac_int_T< ac::c_type<C_TYPE> >::equiv i_t; \
    return op.operator ASSIGN_OP (i_t(i_op));  \
  }

#define FL_SHIFT_OPS_WITH_INT_CTYPE(C_TYPE) \
  FL_SHIFT_OP_WITH_INT_CTYPE(>>, C_TYPE, rshift) \
  FL_SHIFT_OP_WITH_INT_CTYPE(<<, C_TYPE, lshift) \
  FL_SHIFT_ASSIGN_OP_WITH_INT_CTYPE(>>=, C_TYPE) \
  FL_SHIFT_ASSIGN_OP_WITH_INT_CTYPE(<<=, C_TYPE)

#define FL_OPS_WITH_INT_CTYPE(C_TYPE) \
  FL_OPS_WITH_CTYPE(C_TYPE) \
  FL_SHIFT_OPS_WITH_INT_CTYPE(C_TYPE)

// --------------------------------------- End of Macros for Binary Operators with C Floats 

    // Binary Operators with C Floats --------------------------------------------
    FL_OPS_WITH_CTYPE(float)
    FL_OPS_WITH_CTYPE(double)
    FL_OPS_WITH_INT_CTYPE(bool)
    FL_OPS_WITH_INT_CTYPE(char)
    FL_OPS_WITH_INT_CTYPE(signed char)
    FL_OPS_WITH_INT_CTYPE(unsigned char)
    FL_OPS_WITH_INT_CTYPE(short)
    FL_OPS_WITH_INT_CTYPE(unsigned short)
    FL_OPS_WITH_INT_CTYPE(int)
    FL_OPS_WITH_INT_CTYPE(unsigned int)
    FL_OPS_WITH_INT_CTYPE(long)
    FL_OPS_WITH_INT_CTYPE(unsigned long)
    FL_OPS_WITH_INT_CTYPE(Slong)
    FL_OPS_WITH_INT_CTYPE(Ulong)
    // -------------------------------------- End of Binary Operators with C Floats 

// Macros for Binary Operators with ac_int --------------------------------------------

#define FL_BIN_OP_WITH_AC_INT_1(BIN_OP, RTYPE)  \
  template< AC_FL_T(), int WI, bool SI> \
  inline typename AC_FL()::template rt_T2< ac_int<WI,SI> >::RTYPE operator BIN_OP ( const ac_int<WI,SI> &i_op, const AC_FL() &op) {  \
    typedef typename ac::template rt_ac_float_T< ac_int<WI,SI> >::equiv fl2_t; \
    return fl2_t(i_op).operator BIN_OP (op);  \
  }

#define FL_BIN_OP_WITH_AC_INT_2(BIN_OP, RTYPE)  \
  template< AC_FL_T(), int WI, bool SI> \
  inline typename AC_FL()::template rt_T2< ac_int<WI,SI> >::RTYPE operator BIN_OP ( const AC_FL() &op, const ac_int<WI,SI> &i_op) {  \
    typedef typename ac::template rt_ac_float_T< ac_int<WI,SI> >::equiv fl2_t; \
    return op.operator BIN_OP (fl2_t(i_op));  \
  }

#define FL_BIN_OP_WITH_AC_INT(BIN_OP, RTYPE)  \
  FL_BIN_OP_WITH_AC_INT_1(BIN_OP, RTYPE) \
  FL_BIN_OP_WITH_AC_INT_2(BIN_OP, RTYPE)

#define FL_REL_OP_WITH_AC_INT(REL_OP)  \
  template< AC_FL_T(), int WI, bool SI> \
  inline bool operator REL_OP ( const AC_FL() &op, const ac_int<WI,SI> &op2) {  \
    typedef typename ac::template rt_ac_float_T< ac_int<WI,SI> >::equiv fl2_t; \
    return op.operator REL_OP (fl2_t(op2));  \
  }  \
  template< AC_FL_T(), int WI, bool SI> \
  inline bool operator REL_OP ( ac_int<WI,SI> &op2, const AC_FL() &op) {  \
    typedef typename ac::template rt_ac_float_T< ac_int<WI,SI> >::equiv fl2_t; \
    return fl2_t(op2).operator REL_OP (op);  \
  }

#define FL_ASSIGN_OP_WITH_AC_INT(ASSIGN_OP)  \
  template< AC_FL_T(), int WI, bool SI> \
  inline AC_FL() &operator ASSIGN_OP ( AC_FL() &op, const ac_int<WI,SI> &op2) {  \
    typedef typename ac::template rt_ac_float_T< ac_int<WI,SI> >::equiv fl2_t; \
    return op.operator ASSIGN_OP (fl2_t(op2));  \
  }  

// -------------------------------------------- End of Macros for Binary Operators with ac_int

    // Binary Operators with ac_int --------------------------------------------
    FL_BIN_OP_WITH_AC_INT(*, mult)
    FL_BIN_OP_WITH_AC_INT(+, plus)
    FL_BIN_OP_WITH_AC_INT(-, minus)
    FL_BIN_OP_WITH_AC_INT(/, div)

    FL_REL_OP_WITH_AC_INT(==)
    FL_REL_OP_WITH_AC_INT(!=)
    FL_REL_OP_WITH_AC_INT(>)
    FL_REL_OP_WITH_AC_INT(>=)
    FL_REL_OP_WITH_AC_INT(<)
    FL_REL_OP_WITH_AC_INT(<=)

    FL_ASSIGN_OP_WITH_AC_INT(+=)
    FL_ASSIGN_OP_WITH_AC_INT(-=)
    FL_ASSIGN_OP_WITH_AC_INT(*=)
    FL_ASSIGN_OP_WITH_AC_INT(/=)
    FL_ASSIGN_OP_WITH_AC_INT(%=)
    // -------------------------------------- End of Binary Operators with ac_int 

// Macros for Binary Operators with ac_fixed --------------------------------------------

#define FL_BIN_OP_WITH_AC_FIXED_1(BIN_OP, RTYPE)  \
  template< AC_FL_T(), int WF, int IF, bool SF> \
  inline typename AC_FL()::template rt_T2< ac_fixed<WF,IF,SF> >::RTYPE operator BIN_OP ( const ac_fixed<WF,IF,SF> &f_op, const AC_FL() &op) {  \
    typedef typename ac::template rt_ac_float_T<ac_fixed<WF,IF,SF> >::equiv fl2_t; \
    return fl2_t(f_op).operator BIN_OP (op);  \
  }

#define FL_BIN_OP_WITH_AC_FIXED_2(BIN_OP, RTYPE)  \
  template< AC_FL_T(), int WF, int IF, bool SF> \
  inline typename AC_FL()::template rt_T2< ac_fixed<WF,IF,SF> >::RTYPE operator BIN_OP ( const AC_FL() &op, const ac_fixed<WF,IF,SF> &f_op) {  \
    typedef typename ac::template rt_ac_float_T< ac_fixed<WF,IF,SF> >::equiv fl2_t; \
    return op.operator BIN_OP (fl2_t(f_op));  \
  }

#define FL_BIN_OP_WITH_AC_FIXED(BIN_OP, RTYPE)  \
  FL_BIN_OP_WITH_AC_FIXED_1(BIN_OP, RTYPE) \
  FL_BIN_OP_WITH_AC_FIXED_2(BIN_OP, RTYPE)

#define FL_REL_OP_WITH_AC_FIXED(REL_OP)  \
  template< AC_FL_T(), int WF, int IF, bool SF> \
  inline bool operator REL_OP ( const AC_FL() &op, const ac_fixed<WF,IF,SF> &op2) {  \
    typedef typename ac::template rt_ac_float_T< ac_fixed<WF,IF,SF> >::equiv fl2_t; \
    return op.operator REL_OP (fl2_t(op2));  \
  }  \
  template< AC_FL_T(), int WF, int IF, bool SF> \
  inline bool operator REL_OP ( ac_fixed<WF,IF,SF> &op2, const AC_FL() &op) {  \
    typedef typename ac::template rt_ac_float_T< ac_fixed<WF,IF,SF> >::equiv fl2_t; \
    return fl2_t(op2).operator REL_OP (op);  \
  }

#define FL_ASSIGN_OP_WITH_AC_FIXED(ASSIGN_OP)  \
  template< AC_FL_T(), int WF, int IF, bool SF> \
  inline AC_FL() &operator ASSIGN_OP ( AC_FL() &op, const ac_fixed<WF,IF,SF> &op2) {  \
    typedef typename ac::template rt_ac_float_T< ac_fixed<WF,IF,SF> >::equiv fl2_t; \
    return op.operator ASSIGN_OP (fl2_t(op2));  \
  }

// -------------------------------------------- End of Macros for Binary Operators with ac_fixed

    // Binary Operators with ac_fixed --------------------------------------------
    FL_BIN_OP_WITH_AC_FIXED(*, mult)
    FL_BIN_OP_WITH_AC_FIXED(+, plus)
    FL_BIN_OP_WITH_AC_FIXED(-, minus)
    FL_BIN_OP_WITH_AC_FIXED(/, div)

    FL_REL_OP_WITH_AC_FIXED(==)
    FL_REL_OP_WITH_AC_FIXED(!=)
    FL_REL_OP_WITH_AC_FIXED(>)
    FL_REL_OP_WITH_AC_FIXED(>=)
    FL_REL_OP_WITH_AC_FIXED(<)
    FL_REL_OP_WITH_AC_FIXED(<=)

    FL_ASSIGN_OP_WITH_AC_FIXED(+=)
    FL_ASSIGN_OP_WITH_AC_FIXED(-=)
    FL_ASSIGN_OP_WITH_AC_FIXED(*=)
    FL_ASSIGN_OP_WITH_AC_FIXED(/=)
    // -------------------------------------- End of Binary Operators with ac_fixed 

// Global templatized functions for easy initialization to special values
template<ac_special_val V, AC_FL_T()>
inline AC_FL() value( AC_FL() ) {
  AC_FL() r;
  return r.template set_val<V>();
}

namespace ac {
// function to initialize (or uninitialize) arrays
  template<ac_special_val V, AC_FL_T() > 
  inline bool init_array( AC_FL() *a, int n) {
    AC_FL0() t = value<V>(*a);
    for(int i=0; i < n; i++)
      a[i] = t;
    return true;
  }
}

///////////////////////////////////////////////////////////////////////////////

#if (defined(_MSC_VER) && !defined(__EDG__))
#pragma warning( pop )
#endif

#ifdef __AC_NAMESPACE
}
#endif

#endif // __AC_FLOAT_H

/**************************************************************************
 *                                                                        *
 *  ALGORITHMIC C DATATYPES END-USER LICENSE AGREEMENT                    *
 *                                                                        *
 *                                                                        *
 *  IMPORTANT - USE OF SOFTWARE IS SUBJECT TO LICENSE RESTRICTIONS        *
 *  CAREFULLY READ THIS LICENSE AGREEMENT BEFORE USING THE SOFTWARE       *
 *                                                                        *
 *  YOU MAY USE AND DISTRIBUTE UNMODIFIED VERSIONS OF THIS SOFTWARE AS    *
 *  STATED BELOW, YOU MAY NOT MODIFY THE SOFTWARE This license is a       *
 *  legal Agreement between you, the end user, either individually or     *
 *  as an authorized representative of a company acquiring the license,   *
 *  and Calypto Design Systems, Inc. ("Calypto"). YOUR USE OF             *
 *  THE SOFTWARE INDICATES YOUR COMPLETE AND UNCONDITIONAL ACCEPTANCE     *
 *  OF THE TERMS AND CONDITIONS SET FORTH IN THIS AGREEMENT. If you do    *
 *  not agree to these terms and conditions, promptly return or, if       *
 *  received electronically, delete the Software and all accompanying     *
 *  items.                                                                *
 *                                                                        *
 *                                                                        *
 *  1. GRANT OF LICENSE. YOU MAY USE AND DISTRIBUTE THE SOFTWARE, BUT     *
 *  YOU MAY NOT MODIFY THE SOFTWARE. The Software you are installing,     *
 *  downloading, or otherwise acquired, under this Agreement, including   *
 *  source code, binary code, updates, modifications, revisions,          *
 *  copies, or documentation pertaining to Algorithmic C Datatypes        *
 *  (collectively the "Software") is a copyrighted work owned by          *
 *  Calypto. Calypto grants to you, a nontransferable,                    *
 *  nonexclusive, limited copyright license to use and distribute the     *
 *  Software, but you may not modify the Software. Use of the Software    *
 *  consists solely of reproduction, performance, and display.            *
 *                                                                        *
 *  2. RESTRICTIONS; NO MODIFICATION. Modifying the Software is           *
 *  prohibited. Each copy of the Software you create must include all     *
 *  notices and legends embedded in the Software.  Modifying the          *
 *  Software means altering, enhancing, editing, deleting portions or     *
 *  creating derivative works of the Software.  You may append other      *
 *  code to the Software, so long as the Software is not otherwise        *
 *  modified. Calypto retains all rights not expressly granted            *
 *  by this Agreement. The terms of this Agreement, including without     *
 *  limitation, the licensing and assignment provisions, shall be         *
 *  binding upon your successors in interest and assigns.  The            *
 *  provisions of this section 2 shall survive termination or             *
 *  expiration of this Agreement.                                         *
 *                                                                        *
 *  3. USER COMMENT AND SUGGESTIONS.  You are not obligated to provide    *
 *  Calypto with comments or suggestions regarding the                    *
 *  Software.  However, if you do provide to Calypto comments             *
 *  or suggestions for the modification, correction, improvement or       *
 *  enhancement of (a) the Software or (b) Calypto products or            *
 *  processes which may embody the Software ("Comments"), you grant to    *
 *  Calypto a non-exclusive, irrevocable, worldwide, royalty-free         *
 *  license to disclose, display, perform, copy, make, have made, use,    *
 *  sublicense, sell, and otherwise dispose of the Comments, and          *
 *  Calypto's products embodying such Comments, in any manner which       *
 *  Calypto chooses, without reference to the source.                     *
 *                                                                        *
 *  4. NO WARRANTY. CALYPTO EXPRESSLY DISCLAIMS ALL WARRANTY              *
 *  FOR THE SOFTWARE. TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE       *
 *  LAW, THE SOFTWARE AND ANY RELATED DOCUMENTATION IS PROVIDED "AS IS"   *
 *  AND WITH ALL FAULTS AND WITHOUT WARRANTIES OR CONDITIONS OF ANY       *
 *  KIND, EITHER EXPRESS OR IMPLIED, INCLUDING, WITHOUT LIMITATION, THE   *
 *  IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR       *
 *  PURPOSE, OR NONINFRINGEMENT. THE ENTIRE RISK ARISING OUT OF USE OR    *
 *  DISTRIBUTION OF THE SOFTWARE REMAINS WITH YOU.                        *
 *                                                                        *
 *  5. LIMITATION OF LIABILITY. IN NO EVENT WILL CALYPTO OR ITS           *
 *  LICENSORS BE LIABLE FOR INDIRECT, SPECIAL, INCIDENTAL, OR             *
 *  CONSEQUENTIAL DAMAGES (INCLUDING LOST PROFITS OR SAVINGS) WHETHER     *
 *  BASED ON CONTRACT, TORT OR ANY OTHER LEGAL THEORY, EVEN IF CALYPTO    *
 *  OR ITS LICENSORS HAVE BEEN ADVISED OF THE POSSIBILITY OF              *
 *  SUCH DAMAGES.                                                         *
 *                                                                        *
 *  6.  LIFE ENDANGERING APPLICATIONS. NEITHER CALYPTO NOR ITS            *
 *  LICENSORS SHALL BE LIABLE FOR ANY DAMAGES RESULTING FROM OR IN        *
 *  CONNECTION WITH THE USE OR DISTRIBUTION OF SOFTWARE IN ANY            *
 *  APPLICATION WHERE THE FAILURE OR INACCURACY OF THE SOFTWARE MIGHT     *
 *  RESULT IN DEATH OR PERSONAL INJURY.  THE PROVISIONS OF THIS SECTION 6 *
 *  SHALL SURVIVE TERMINATION OR EXPIRATION OF THIS AGREEMENT.            *
 *                                                                        *
 *  7.  INDEMNIFICATION.  YOU AGREE TO INDEMNIFY AND HOLD HARMLESS        *
 *  CALYPTO AND ITS LICENSORS FROM ANY CLAIMS, LOSS, COST,                *
 *  DAMAGE, EXPENSE, OR LIABILITY, INCLUDING ATTORNEYS' FEES, ARISING     *
 *  OUT OF OR IN CONNECTION WITH YOUR USE OR DISTRIBUTION OF SOFTWARE.    *
 *                                                                        *
 *  8. TERM AND TERMINATION. This Agreement terminates immediately if     *
 *  you exceed the scope of the license granted or fail to comply with    *
 *  the provisions of this License Agreement.  If you institute patent    *
 *  litigation against Calypto (including a cross-claim or                *
 *  counterclaim in a lawsuit) alleging that the Software constitutes     *
 *  direct or contributory patent infringement, then any patent           *
 *  licenses granted to you under this License for that Software shall    *
 *  terminate as of the date such litigation is filed. Upon termination   *
 *  or expiration, you agree to cease all use of the Software and         *
 *  delete all copies of the Software.                                    *
 *                                                                        *
 *  9. EXPORT. Software may be subject to regulation by local laws and    *
 *  United States government agencies, which prohibit export or           *
 *  diversion of certain products, information about the products, and    *
 *  direct products of the products to certain countries and certain      *
 *  persons. You agree that you will not export any Software or direct    *
 *  product of Software in any manner without first obtaining all         *
 *  necessary approval from appropriate local and United States           *
 *  government agencies.                                                  *
 *                                                                        *
 *  10. U.S. GOVERNMENT LICENSE RIGHTS. Software was developed entirely   *
 *  at private expense. All software is commercial computer software      *
 *  within the meaning of the applicable acquisition regulations.         *
 *  Accordingly, pursuant to US FAR 48 CFR 12.212 and DFAR 48 CFR         *
 *  227.7202, use, duplication and disclosure of the Software by or for   *
 *  the U.S. Government or a U.S. Government subcontractor is subject     *
 *  solely to the terms and conditions set forth in this Agreement,       *
 *  except for provisions which are contrary to applicable mandatory      *
 *  federal laws.                                                         *
 *                                                                        *
 *  11. CONTROLLING LAW AND JURISDICTION. THIS AGREEMENT SHALL BE         *
 *  GOVERNED BY AND CONSTRUED UNDER THE LAWS OF THE STATE OF CALIFORNIA,  *
 *  USA. All disputes arising out of or in relation to this Agreement     *
 *  shall be submitted to the exclusive jurisdiction of Santa Clara       *
 *  County, California. This section shall not restrict Calypto's         *
 *  right to bring an action against you in the jurisdiction where your   *
 *  place of business is located.  The United Nations Convention on       *
 *  Contracts for the International Sale of Goods does not apply to       *
 *  this Agreement.                                                       *
 *                                                                        *
 *  12. SEVERABILITY. If any provision of this Agreement is held by a     *
 *  court of competent jurisdiction to be void, invalid, unenforceable    *
 *  or illegal, such provision shall be severed from this Agreement and   *
 *  the remaining provisions will remain in full force and effect.        *
 *                                                                        *
 *  13. MISCELLANEOUS.  This Agreement contains the parties' entire       *
 *  understanding relating to its subject matter and supersedes all       *
 *  prior or contemporaneous agreements. This Agreement may only be       *
 *  modified in writing by authorized representatives of the parties.     *
 *  Waiver of terms or excuse of breach must be in writing and shall      *
 *  not constitute subsequent consent, waiver or excuse. The prevailing   *
 *  party in any legal action regarding the subject matter of this        *
 *  Agreement shall be entitled to recover, in addition to other          *
 *  relief, reasonable attorneys' fees and expenses.                      *
 *                                                                        *
 *  Algorithmic C Datatypes EULA (Rev. 110928)                            *
 **************************************************************************/
