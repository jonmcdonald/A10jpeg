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
 *  Copyright 2004-2014, Calypto Design Systems, Inc.,                     *
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

#ifndef __AC_SC_H
#define __AC_SC_H

#ifndef __cplusplus
#error C++ is required to include this header file
#endif

#if !defined(IEEE_1666_SYSTEMC) && !defined(SYSTEMC_VERSION) && !defined(SC_API_VERSION_STRING)
#error SystemC header file needs to be included before the ac_sc is included
#endif

#include <ac_float.h>

#ifdef __AC_NAMESPACE
namespace __AC_NAMESPACE {
#endif

// Explicit conversion functions from ac to sc and viceversa 
template <int W>
ac_int<W, true> to_ac(const sc_dt::sc_bigint<W> &val){
  enum {N = (W+31)/32 };
  sc_dt::sc_bigint<N*32> v = val;
  ac_int<N*32, true> r = 0;
#ifdef __SYNTHESIS__
#pragma UNROLL y
#endif
  for(int i = 0; i < N; i++) {
    r.set_slc(i*32, ac_int<32,true>(v.to_int())); 
    v >>= 32;
  }
  return ac_int<W,true>(r); 
}

template <int W>
ac_int<W, false> to_ac(const sc_dt::sc_biguint<W> &val){
  enum {N = (W+31)/32 };
  sc_dt::sc_biguint<N*32> v = val;
  ac_int<N*32, true> r = 0;
#ifdef __SYNTHESIS__
#pragma UNROLL y
#endif
  for(int i = 0; i < N; i++) {
    r.set_slc(i*32, ac_int<32,true>(v.to_int())); 
    v >>= 32;
  }
  return ac_int<W,false>(r);
}

template <int W>
sc_dt::sc_bigint<W> to_sc(const ac_int<W,true> &val) {
  enum {N = (W+31)/32 };
  ac_int<N*32, true> v = val;
  sc_dt::sc_bigint<N*32> r;
#ifdef __SYNTHESIS__
#pragma UNROLL y
#endif
  for(int i = N-1; i >= 0; i--) {
    r <<= 32;
    r.range(31, 0) = (v.template slc<32>(i*32)).to_int();
  }
  return sc_dt::sc_bigint<W>(r);
}
  
template <int W>
sc_dt::sc_biguint<W> to_sc(const ac_int<W,false> &val) {
  enum {N = (W+31)/32 };
  ac_int<N*32, true> v = val;
  sc_dt::sc_biguint<N*32> r;
#ifdef __SYNTHESIS__
#pragma UNROLL y
#endif
  for(int i = N-1; i >= 0; i--) {
    r <<= 32;
    r.range(31, 0) = (v.template slc<32>(i*32)).to_int();
  }
  return sc_dt::sc_biguint<W>(r);
}

#ifdef SC_INCLUDE_FX
template <int W, int I, sc_dt::sc_q_mode Q, sc_dt::sc_o_mode O, int nbits>
ac_fixed<W,I, true> to_ac(const sc_dt::sc_fixed<W,I,Q,O,nbits> &val){
  ac_fixed<W,I,true> r = 0;
  sc_dt::sc_fixed<W,W> fv;
  fv.range(W-1,0) = val.range(W-1,0);
  sc_dt::sc_bigint<W> v(fv);
  r.set_slc(0, to_ac(v)); 
  return r; 
}

template <int W, int I, sc_dt::sc_q_mode Q, sc_dt::sc_o_mode O, int nbits>
ac_fixed<W,I, false> to_ac(const sc_dt::sc_ufixed<W,I,Q,O,nbits> &val){
  ac_fixed<W,I,false> r = 0;
  sc_dt::sc_ufixed<W,W> fv;
  fv.range(W-1,0) = val.range(W-1,0);
  sc_dt::sc_biguint<W> v(fv);
  r.set_slc(0, to_ac(v)); 
  return r; 
}

template <int W, int I, ac_q_mode Q, ac_o_mode O>
sc_dt::sc_fixed<W,I> to_sc(const ac_fixed<W,I,true,Q,O> &val) {
  ac_int<W,true> v = val.template slc<W>(0);
  sc_dt::sc_bigint<W> i = to_sc(v);
  sc_dt::sc_fixed<W,W> f(i);
  sc_dt::sc_fixed<W,I> r;
  r.range(W-1,0) = f.range(W-1,0);
  return r; 
}
  
template <int W, int I, ac_q_mode Q, ac_o_mode O>
sc_dt::sc_ufixed<W,I> to_sc(const ac_fixed<W,I,false,Q,O> &val) {
  ac_int<W,false> v = val.template slc<W>(0);
  sc_dt::sc_biguint<W> i = to_sc(v);
  sc_dt::sc_ufixed<W,W> f(i);
  sc_dt::sc_ufixed<W,I> r;
  r.range(W-1,0) = f.range(W-1,0);
  return r; 
}
#endif

// Utility global functions for initialization 

template<ac_special_val V, int W>
inline sc_dt::sc_int<W> value(sc_dt::sc_int<W>) {
  sc_dt::sc_int<W> r;
  if(V == AC_VAL_DC) {
    int t;
    r = t;
  } else if(V == AC_VAL_0 || V == AC_VAL_MIN || V == AC_VAL_QUANTUM) {
    r = 0;
    if(V == AC_VAL_MIN)
      r[W-1] = 1;
    else if(V == AC_VAL_QUANTUM)
      r[0] = 1;
  } else if(AC_VAL_MAX) {
    r = -1;
    r[W-1] = 0;
  }
  return r;
}
                                                                                           
template<ac_special_val V, int W>
inline sc_dt::sc_uint<W> value(sc_dt::sc_uint<W>) {
  sc_dt::sc_uint<W> r;
  if(V == AC_VAL_DC) {
    int t;
    r = t;
  } else if(V == AC_VAL_0 || V == AC_VAL_MIN || V == AC_VAL_QUANTUM) {
    r = 0;
    if(V == AC_VAL_QUANTUM)
      r[0] = 1;
  } else if(AC_VAL_MAX)
    r = -1;
  return r;
}
                                                                                           
template<ac_special_val V, int W>
inline sc_dt::sc_bigint<W> value(sc_dt::sc_bigint<W>) {
  sc_dt::sc_bigint<W> r;
  if(V == AC_VAL_DC) {
    int t;
    r = t;
  } else if(V == AC_VAL_0 || V == AC_VAL_MIN || V == AC_VAL_QUANTUM) {
    r = 0;
    if(V == AC_VAL_MIN)
      r[W-1] = 1;
    else if(V == AC_VAL_QUANTUM)
      r[0] = 1;
  } else if(AC_VAL_MAX) {
    r = -1;
    r[W-1] = 0;
  }
  return r;
}
                                                                                           
template<ac_special_val V, int W>
inline sc_dt::sc_biguint<W> value(sc_dt::sc_biguint<W>) {
  sc_dt::sc_biguint<W> r;
  if(V == AC_VAL_DC) {
    int t;
    r = t;
  } else if(V == AC_VAL_0 || V == AC_VAL_MIN || V == AC_VAL_QUANTUM) {
    r = 0;
    if(V == AC_VAL_QUANTUM)
      r[0] = 1;
  } else if(AC_VAL_MAX)
    r = -1;
  return r;
}

#ifdef SC_INCLUDE_FX
template<ac_special_val V, int W, int I, sc_dt::sc_q_mode Q, sc_dt::sc_o_mode O, int nbits>
inline sc_dt::sc_fixed<W,I,Q,O,nbits> value(sc_dt::sc_fixed<W,I,Q,O,nbits>) {
  sc_dt::sc_fixed<W,I> r;
  if(V == AC_VAL_DC) {
    int t;
    r = t;
  } else if(V == AC_VAL_0 || V == AC_VAL_MIN || V == AC_VAL_QUANTUM) {
    r = 0;
    if(V == AC_VAL_MIN)
      r[W-1] = 1;
    else if(V == AC_VAL_QUANTUM)
      r[0] = 1;
  } else if(AC_VAL_MAX) {
    r = ~ (sc_dt::sc_fixed<W,I>) 0;
    r[W-1] = 0;
  }
  return r;
}
                                                                                           
template<ac_special_val V, int W, int I, sc_dt::sc_q_mode Q, sc_dt::sc_o_mode O, int nbits>
inline sc_dt::sc_ufixed<W,I,Q,O,nbits> value(sc_dt::sc_ufixed<W,I,Q,O,nbits>) {
  sc_dt::sc_ufixed<W,I> r;
  if(V == AC_VAL_DC) {
    int t;
    r = t;
  } else if(V == AC_VAL_0 || V == AC_VAL_MIN || V == AC_VAL_QUANTUM) {
    r = 0;
    if(V == AC_VAL_QUANTUM)
      r[0] = 1;
  } else if(AC_VAL_MAX)
    r = ~ (sc_dt::sc_ufixed<W,I>) 0;
  return r;
}
#endif


namespace ac {
// PUBLIC FUNCTIONS
// function to initialize (or uninitialize) arrays
  template<ac_special_val V, int W>
  inline bool init_array(sc_dt::sc_int<W> *a, int n) {
    sc_dt::sc_int<W> t = value<V>(*a);
    for(int i=0; i < n; i++)
      a[i] = t;
    return true;
  }
  template<ac_special_val V, int W>
  inline bool init_array(sc_dt::sc_uint<W> *a, int n) {
    sc_dt::sc_uint<W> t = value<V>(*a);
    for(int i=0; i < n; i++)
      a[i] = t;
    return true;
  }
  template<ac_special_val V, int W>
  inline bool init_array(sc_dt::sc_bigint<W> *a, int n) {
    sc_dt::sc_bigint<W> t = value<V>(*a);
    for(int i=0; i < n; i++)
      a[i] = t;
    return true;
  }
  template<ac_special_val V, int W>
  inline bool init_array(sc_dt::sc_biguint<W> *a, int n) {
    sc_dt::sc_biguint<W> t = value<V>(*a);
    for(int i=0; i < n; i++)
      a[i] = t;
    return true;
  }
#ifdef SC_INCLUDE_FX
  template<ac_special_val V, int W, int I, sc_dt::sc_q_mode Q, sc_dt::sc_o_mode O, int nbits>
  inline bool init_array(sc_dt::sc_fixed<W,I,Q,O,nbits> *a, int n) {
    sc_dt::sc_fixed<W,I> t = value<V>(*a);
    for(int i=0; i < n; i++)
      a[i] = t;
    return true;
  }
  template<ac_special_val V, int W, int I, sc_dt::sc_q_mode Q, sc_dt::sc_o_mode O, int nbits>
  inline bool init_array(sc_dt::sc_ufixed<W,I,Q,O,nbits> *a, int n) {
    sc_dt::sc_ufixed<W,I> t = value<V>(*a);
    for(int i=0; i < n; i++)
      a[i] = t;
    return true;
  }
#endif
}

#ifdef __AC_NAMESPACE
}
#endif


// TRACE FUNCTIONS

// SystemC Versions - 2.2.0 20070314
//                    2.3.0 20120701
//                    2.3.1 20140417

#if (SYSTEMC_VERSION <= 20140417) && !defined(NCSC)

#if (SYSTEMC_VERSION > 20120701)
#include <sysc/tracing/sc_vcd_trace.h>
#endif

//==============================================================================
// The following block of code is copied from the file sc_vcd_trace.cpp in the
// SystemC 2.2.0 distribution. This code should have been placed in the file
// sc_vcd_trace.h to allow proper C++ derivation. 
namespace sc_core {
class vcd_trace
{
public:

    vcd_trace(const std::string& name_, const std::string& vcd_name_);

    // Needs to be pure virtual as has to be defined by the particular
    // type being traced
    virtual void write(FILE* f) = 0;
    
    virtual void set_width();

    static const char* strip_leading_bits(const char* originalbuf);

    // Comparison function needs to be pure virtual too
    virtual bool changed() = 0;

    // Make this virtual as some derived classes may overwrite
    virtual void print_variable_declaration_line(FILE* f);

    void compose_data_line(char* rawdata, char* compdata);

#if (SYSTEMC_VERSION > 20120701)
    std::string compose_line(const std::string& data);
#else
    std::string compose_line(const std::string data);
#endif

    virtual ~vcd_trace();

    const std::string name;
    const std::string vcd_name;
    const char* vcd_var_typ_name;
    int bit_width; 
};
}
static
void
remove_vcd_name_problems(std::string& name)
{
    using namespace sc_core;
    char message[4000];
    static bool warned = false;

    bool braces_removed = false;
    for (unsigned int i = 0; i< name.length(); i++) {
      if (name[i] == '[') {
        name[i] = '(';
        braces_removed = true;
      }
      else if (name[i] == ']') {
        name[i] = ')';
        braces_removed = true;
      }
    }

    if(braces_removed && !warned){
        std::sprintf(message,
                "Traced objects found with name containing [], which may be\n"
                "interpreted by the waveform viewer in unexpected ways.\n"
                "So the [] is automatically replaced by ().");
#if (SYSTEMC_VERSION <= 20120701)
        put_error_message(message, true);
#endif
        warned = true;
    }
}
//==============================================================================
#endif


//==============================================================================
// TRACING SUPPORT FOR AC_INT
template <int W, bool S>
class vcd_ac_int_trace : public sc_core::vcd_trace
{
public:
  vcd_ac_int_trace(const ac_int<W,S> &object_, const std::string& name_, const std::string& vcd_name_) :
    vcd_trace(name_, vcd_name_), object(object_)
  {
    vcd_var_typ_name = "wire"; // SystemC does not expose vcd_types[] in sc_vcd_trace.h
    bit_width = W; // bit_width defined in base class 'vcd_trace'
  }

  virtual void print_variable_declaration_line(FILE* f) {
    char buf[2000];
    std::string namecopy = name;
#if !defined(NCSC)
    remove_vcd_name_problems(namecopy);
#endif
    std::sprintf(buf, "$var %s  % 3d  %s  %s [%d:0]  $end\n", vcd_var_typ_name,bit_width,vcd_name.c_str(),namecopy.c_str(),bit_width-1);
    std::fputs(buf, f);
  }

  virtual void write(FILE* f) { 
    // The function to_string(AC_BIN) returns a string with the zero-radix prefix (i.e. "0b").
    // Strip that prefix off because compose_line will add its own.
    std::fprintf(f, "%s", compose_line(((ac_int<W,false>)object).to_string(AC_BIN,true).substr(3)).c_str());
    old_value = object;
  }

  virtual void set_width() { bit_width = W; }

  // Comparison function needs to be pure virtual too
  virtual bool changed() { return !(object == old_value); }

  virtual ~vcd_ac_int_trace() {}
protected:
  const ac_int<W,S> &object;
  ac_int<W,S>        old_value;
};

template <int W, bool S>
inline void sc_trace(sc_core::sc_trace_file *tf, const ac_int<W,S> &a, const std::string &name)
{
  using namespace sc_core;
  if (tf) {
    //--- SystemC 2.2.0 deficiency. The 'initialized' class member of sc_trace_file is
    // declared as 'protected' and does not have a public access 'get_initialized()' method.
    // Therefore, we cannot check for initialized so the following code is commented out.
    //if( tf->initialized ) {
    //    put_error_message(
    //   "No traces can be added once simulation has started.\n"
    //        "To add traces, create a new vcd trace file.", false );
    //}
    vcd_trace *t = (vcd_trace*) new vcd_ac_int_trace<W,S>(a,name,((vcd_trace_file*)tf)->obtain_name());
    ((vcd_trace_file*)tf)->traces.push_back(t);
  }
}
//==============================================================================

#if !defined(__AC_FIXED_MTI_H)
// The ac_fixed.h shipped with ModelSim/QuestaSim has a stub for sc_trace() for ac_fixed so
// this code is not used. The stub should be removed in a future release of the simulator.
#if defined(__AC_FIXED_H) && !defined(SC_TRACE_AC_FIXED)
#define SC_TRACE_AC_FIXED

//==============================================================================
// TRACING SUPPORT FOR AC_FIXED
template<int W, int I, bool S, ac_q_mode Q, ac_o_mode O>
class vcd_ac_fixed_trace : public sc_core::vcd_trace
{
public:
  vcd_ac_fixed_trace(const ac_fixed<W,I,S,Q,O> &object_, const std::string& name_, const std::string& vcd_name_) :
    vcd_trace(name_, vcd_name_), object(object_)
  {
    vcd_var_typ_name = "wire"; // SystemC does not expose vcd_types[] in sc_vcd_trace.h
    bit_width = W; // bit_width defined in base class 'vcd_trace'
  }

  virtual void print_variable_declaration_line(FILE* f) {
    char buf[2000];
    std::string namecopy = name;
#if !defined(NCSC)
    remove_vcd_name_problems(namecopy);
#endif
    std::sprintf(buf, "$var %s  % 3d  %s  %s [%d:0]  $end\n", vcd_var_typ_name,bit_width,vcd_name.c_str(),namecopy.c_str(),bit_width-1);
    std::fputs(buf, f);
  }

  virtual void write(FILE* f) { 
    // The function to_string(AC_BIN) returns a string with the zero-radix prefix (i.e. "0b").
    // Strip that prefix off because compose_line will add its own.
    std::fprintf(f, "%s", compose_line(((ac_fixed<W,I,false>)object).to_string(AC_BIN,true).substr(3)).c_str());
    old_value = object;
  }

  virtual void set_width() { bit_width = W; }

  // Comparison function needs to be pure virtual too
  virtual bool changed() { return !(object == old_value); }

  virtual ~vcd_ac_fixed_trace() {}
protected:
  const ac_fixed<W,I,S,Q,O> &object;
  ac_fixed<W,I,S,Q,O>        old_value;
};

template<int W, int I, bool S, ac_q_mode Q, ac_o_mode O>
inline void sc_trace(sc_core::sc_trace_file *tf, const ac_fixed<W,I,S,Q,O> &a, const std::string &name)
{
  using namespace sc_core;
  if (tf) {
    //--- SystemC 2.2.0 deficiency. The 'initialized' class member of sc_trace_file is
    // declared as 'protected' and does not have a public access 'get_initialized()' method.
    // Therefore, we cannot check for initialized so the following code is commented out.
    //if( tf->initialized ) {
    //    put_error_message(
    //   "No traces can be added once simulation has started.\n"
    //        "To add traces, create a new vcd trace file.", false );
    //}
    vcd_trace *t = (vcd_trace*) new vcd_ac_fixed_trace<W,I,S,Q,O>(a,name,((vcd_trace_file*)tf)->obtain_name());
    ((vcd_trace_file*)tf)->traces.push_back(t);
  }
}
//==============================================================================
#endif
#endif

#if defined(__AC_FLOAT_H) && !defined(SC_TRACE_AC_FLOAT)
#define SC_TRACE_AC_FLOAT

//==============================================================================
// TRACING SUPPORT FOR AC_FLOAT
template<int W, int I, int E, ac_q_mode Q>
class vcd_ac_float_trace : public sc_core::vcd_trace
{
public:
  vcd_ac_float_trace(const ac_float<W,I,E,Q> &object_, const std::string& name_, const std::string& vcd_name_) :
    vcd_trace(name_, vcd_name_), object(object_)
  {
    vcd_var_typ_name = "wire"; // SystemC does not expose vcd_types[] in sc_vcd_trace.h
    bit_width = W; // bit_width defined in base class 'vcd_trace'
  }

  virtual void print_variable_declaration_line(FILE* f) {
    char buf[2000];
    std::string namecopy = name;
#if !defined(NCSC)
    remove_vcd_name_problems(namecopy);
#endif
    std::sprintf(buf, "$var %s  % 3d  %s  %s [%d:0]  $end\n", vcd_var_typ_name,bit_width,vcd_name.c_str(),namecopy.c_str(),bit_width-1);
    std::fputs(buf, f);
  }

  virtual void write(FILE* f) {
    // The function to_string(AC_BIN) returns a string with the zero-radix prefix (i.e. "0b").
    // Strip that prefix off because compose_line will add its own.
    std::fprintf(f, "%s", compose_line(object.to_string(AC_BIN).substr(2)).c_str());
    old_value = object;
  }

  virtual void set_width() { bit_width = W; }

  // Comparison function needs to be pure virtual too
  virtual bool changed() { return !(object == old_value); }

  virtual ~vcd_ac_float_trace() {}
protected:
  const ac_float<W,I,E,Q> &object;
  ac_float<W,I,E,Q>        old_value;
};

template<int W, int I, int E, ac_q_mode Q>
inline void sc_trace(sc_core::sc_trace_file *tf, const ac_float<W,I,E,Q> &a, const std::string &name)
{
  using namespace sc_core;
  if (tf) {
    //--- SystemC 2.2.0 deficiency. The 'initialized' class member of sc_trace_file is
    // declared as 'protected' and does not have a public access 'get_initialized()' method.
    // Therefore, we cannot check for initialized so the following code is commented out.
    //if( tf->initialized ) {
    //    put_error_message(
    //   "No traces can be added once simulation has started.\n"
    //        "To add traces, create a new vcd trace file.", false );
    //}
    vcd_trace *t = (vcd_trace*) new vcd_ac_float_trace<W,I,E,Q>(a,name,((vcd_trace_file*)tf)->obtain_name());
    ((vcd_trace_file*)tf)->traces.push_back(t);
  }
}
//==============================================================================
#endif


#endif

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
