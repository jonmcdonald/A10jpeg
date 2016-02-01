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

#ifndef __CCS_P2P_VALID_H
#define __CCS_P2P_VALID_H
#include <ccs_types.h>

//----------------------------------------------------------------------------
// Interconnect: p2p_valid
//   Three signals used to directly connect two blocks using the same protocol as a FIFO.
//     <name>_vld:  Data is valid, active high, driven active by the source when data is valid.
//     <name>_dat:  Data to be transmitted across interconnect, 
//   Data is transmitted in any cycle where "vld" is active.
//   Underlying datatypes are bool for one-bit and sc_lv for vector types to allow clean
//   mixed-language simulation.
//   Back pressure is not supported by this channel.
//
//   The interconnect has a latency of one and supports a throughput of one.
//   Supported Methods:
//     out:
//         void nb_write(T data):  Attempts to write data into channel.
//                                 No status is returned, data is assumed to be transmitted.
//      in:
//         T read():  Reads data from channel, blocks process until completion
//         bool nb_read(T& data):  Attempts to read data from channel and set data to read value.  
//                                  Returns false if data can not be read.
//   Most common declaration examples, which result in transaction level simulation:
//     p2p_valid<>::chan<data_T> my_channel;
//     p2p_valid<>::in<data_T> my_input_port;
//     p2p_valid<>::out<data_T> my_output_port;
//   To declare pin-accurate interconnects use SYN template parameter:
//     p2p_valid<SYN>::chan<data_T> my_channel;
//     p2p_valid<SYN>::in<data_T> my_input_port;
//     p2p_valid<SYN>::out<data_T> my_output_port;

// Container Template
template <abstraction_t source_abstraction = AUTO>
class p2p_valid {

public:
// Base Template Definition, override defaults for synthesis
// Do not overried setting on impl_abstraction template parameter!
// Always use SYN view for synthesis and formal
#if defined(__SYNTHESIS__) || defined (CALYPTO_SYSC)
  template <class T, abstraction_t impl_abstraction = SYN> class chan {};
  template <class T, abstraction_t impl_abstraction = SYN> class assign {};
  template <class T, abstraction_t impl_abstraction = SYN> class in {};
  template <class T, abstraction_t impl_abstraction = SYN> class out {};
// Default to SYN view in SCVerify.  Channels connecting testbench blocks should be hard-coded to TLM.
#elif defined (CCS_DUT_CYCLE) || defined (CCS_DUT_RTL)
  template <class T, abstraction_t impl_abstraction = (source_abstraction==AUTO)? SYN : source_abstraction > class chan {};
  template <class T, abstraction_t impl_abstraction = (source_abstraction==AUTO)? SYN : source_abstraction > class assign {};
  template <class T, abstraction_t impl_abstraction = (source_abstraction==AUTO)? SYN : source_abstraction > class in {};
  template <class T, abstraction_t impl_abstraction = (source_abstraction==AUTO)? SYN : source_abstraction > class out {};
#else
  template <class T, abstraction_t impl_abstraction = (source_abstraction==AUTO)? P2P_DEFAULT_VIEW : source_abstraction > class chan {};
  template <class T, abstraction_t impl_abstraction = (source_abstraction==AUTO)? P2P_DEFAULT_VIEW : source_abstraction > class assign {};
  template <class T, abstraction_t impl_abstraction = (source_abstraction==AUTO)? P2P_DEFAULT_VIEW : source_abstraction > class in {};
  template <class T, abstraction_t impl_abstraction = (source_abstraction==AUTO)? P2P_DEFAULT_VIEW : source_abstraction > class out {};
#endif

#ifndef CALYPTO_SYSC
  // TLM channel definition
  // NOTE:  Minimum FIFO depth is one or SystemC simulation will hang!
  template <class T>
  class chan <T,TLM> {
    p2p_checker rd_chk, wr_chk;
  public:
    tlm::tlm_fifo<T> fifo;

    chan(const char *name) 
      : rd_chk(name, "call reset_read()", "read from this channel")
      , wr_chk(name, "call reset_write()", "write to this channel")
      , fifo(ccs_concat(name, "fifo"), 1)
      {}

    // Empty FIFO on reset
    void reset_read() { 
      rd_chk.ok(); 
      T temp; 
      while (fifo.nb_get(temp)); 
    }

    void reset_write() {wr_chk.ok();}

    T read() { rd_chk.test(); return fifo.get(); }
    bool nb_read(T &data) { rd_chk.test(); return fifo.nb_get(data); }
    void nb_write(T data) { // Assumes target will take data 
      wr_chk.test(); 
      fifo.nb_put( data ); 
    } 
  };

  // TLM input port definition
  template <class T>
  class in <T,TLM> {
    p2p_checker rd_chk;

  public:
    sc_port<tlm::tlm_fifo_get_if<T> > i_fifo;

    in(sc_module_name name) 
      : rd_chk(name, "call reset_read()", "read from this port")
      , i_fifo(ccs_concat(name, "i_fifo"))
      {}

    // Empty FIFO on reset
    void reset_read() { 
      rd_chk.ok();
      T temp; 
      while (i_fifo->nb_get(temp)); 
    }

    T read() { rd_chk.test(); return i_fifo->get(); }
    bool nb_read(T &data) { rd_chk.test(); return i_fifo->nb_get(data); }

    void bind (in<T> &x) { i_fifo(x.i_fifo); }
    template<class C>
    void bind (C &x) { i_fifo(x.fifo); }
    template <class C>
    void operator() (C &x) { bind(x); }
  };

  // TLM output port definition
  template <class T>
  class out <T,TLM> {
    p2p_checker wr_chk;

  public:
    sc_port<tlm::tlm_fifo_put_if<T> > o_fifo;

    out(sc_module_name name) 
      : wr_chk(name, "call reset_write()", "write to this port")
      , o_fifo(ccs_concat(name,"o_fifo")) 
      {}

    void reset_write(){wr_chk.ok();}

    void nb_write(T data) { // Assumes target will take data (and is rate matched)
      wr_chk.test(); 
      o_fifo->nb_put( data ); 
    } 

    void bind (out<T> &x) { o_fifo(x.o_fifo); }
    template<class C>
    void bind (C &x) { o_fifo(x.fifo); }
    template <class C>
    void operator() (C &x) { bind(x); }
  };

  // TLM assignemnt definition
  // Used to connect in->chan, chan->chan, chan->out
  template < class T >
  struct assign <T, TLM> : ::sc_core::sc_module {
    in<T,TLM> bind_in;
    out<T,TLM> bind_out;

    SC_CTOR (assign)
      :  bind_in ("bind_in")
      ,  bind_out ("bind_out")
    {
      SC_THREAD(pass_through);
    }

    void pass_through () {
      bind_out.reset_write();
      bind_in.reset_read();
      while(1) {
        bind_out.nb_write(bind_in.read()); // Assumes target will take data
      }
    }
  };

#endif
  // SYN channel definition
  template <class T>
  class chan <T,SYN> {
    // Get bitwidth and signness of T for conversion to sc_lv
    enum {signedtype = mc_typedef_T_traits<T>::issigned,
          data_width = mc_typedef_T_traits<T>::bitwidth};

    p2p_checker rd_chk, wr_chk;

  public:
    sc_signal <sc_lv<data_width> > dat ;
    sc_signal <bool> vld ;

    chan(const char *name) 
      : rd_chk(name, "call reset_read()", "read from this channel")
      , wr_chk(name, "call reset_write()", "write to this channel")
      , dat( ccs_concat(name,"dat") )
      , vld( ccs_concat(name,"vld") ) 
      {}

    void reset_write() {
      wr_chk.ok();
      dat.write(false);
      vld.write(false);
    }

    void reset_read() {
      rd_chk.ok();
      // Provided only for consistent API with p2p_wire/fifo
    }

    #pragma design modulario
    T read() {
      rd_chk.test();
      do {
        wait() ;
      } while (vld.read() != true );
      T data;
      vector_to_type(dat.read(),(signedtype?true:false),&data);
      return data;
    }

    #pragma design modulario
    bool nb_read(T &data) {
      rd_chk.test();
      wait() ;
      vector_to_type(dat.read(),(signedtype?true:false),&data);
      return vld.read();
    }

    #pragma design modulario
    void nb_write (T data) {
      wr_chk.test();
      sc_lv<data_width> temp;
      type_to_vector(data, (signedtype?true:false), temp);
      dat.write(temp);
      vld.write(true);
      wait();
      // Return outputs to dc to save area
      sc_lv<data_width> dc;
      dat.write(dc);
      vld.write(false);
    }
  };

  // SYN input port definition
  template <class T>
  class in <T,SYN> {
    // Get bitwidth and signness of T for conversion to sc_lv
    enum {signedtype = mc_typedef_T_traits<T>::issigned,
          data_width = mc_typedef_T_traits<T>::bitwidth};
    p2p_checker rd_chk;

  public:
    sc_in <sc_lv<data_width> > i_dat ;
    sc_in <bool> i_vld ;

    in(const char *name) 
      : rd_chk(name, "call reset_read()", "read from this port")
      , i_dat( ccs_concat(name,"i_dat") )
      , i_vld( ccs_concat(name,"i_vld") ) 
      {}

    void reset_read() {
      rd_chk.ok(); 
      // Provided for consistent API with p2p_wire/fifo
    }

    #pragma design modulario
    T read() {
      rd_chk.test();
      do {
        wait() ;
      } while (i_vld.read() != true );
      T data;
      vector_to_type(i_dat.read(),(signedtype?true:false),&data);
      return data;
    }

    #pragma design modulario
    bool nb_read(T &data) {
      rd_chk.test();
      wait() ;
      vector_to_type(i_dat.read(),(signedtype?true:false),&data);
      return i_vld.read();
    }

    void bind (in<T>& c) {
      i_vld(c.i_vld);
      i_dat(c.i_dat);
    }

    template <class C>
    void bind (C& c) {
      i_vld(c.vld);
      i_dat(c.dat);
    }

    template <class C>
    void operator() (C& c) {
      bind(c);
    }

  };

  // SYN output port definition
  template <class T>
  class out <T,SYN> {
    // Get bitwidth and signness of T for conversion to sc_lv
    enum {signedtype = mc_typedef_T_traits<T>::issigned,
          data_width = mc_typedef_T_traits<T>::bitwidth};
    p2p_checker wr_chk;
  public:
    sc_out <sc_lv<data_width> > o_dat ;
    sc_out <bool> o_vld ;

    out(const char *name) 
      : wr_chk(name, "call reset_write()", "write to this port")
      , o_dat( ccs_concat(name,"o_dat") )
      , o_vld( ccs_concat(name,"o_vld") ) 
      {}

    void reset_write() {
      o_dat.write(false);
      o_vld.write(false);
      wr_chk.ok();
    }

    #pragma design modulario
    void nb_write (T data) {
      wr_chk.test();
      sc_lv<data_width> temp;
      type_to_vector(data, (signedtype?true:false), temp);
      o_dat.write(temp);
      o_vld.write(true);
      wait();
      // Return outputs to dc to save area
      sc_lv<data_width> dc;
      o_dat.write(dc);
      o_vld.write(false);
    }

    void bind (out<T>& c) {
      o_vld(c.o_vld);
      o_dat(c.o_dat);
    }

    template <class C>
    void bind (C& c) {
      o_vld(c.vld);
      o_dat(c.dat);
    }

    template <class C>
    void operator() (C& c) {
      bind(c);
    }
  };

  // SYN assignemnt definition
  // Used to connect in->chan, chan->chan, chan->out
  #pragma ungroup
  template < class T >
  struct assign <T, SYN> : ::sc_core::sc_module {
    in<T,SYN> bind_in;
    out<T,SYN> bind_out;

    SC_CTOR (assign)
      :  bind_in ("bind_in")
      ,  bind_out ("bind_out")
    {
      SC_METHOD(assign_dat);
      sensitive << bind_in.i_dat;
      SC_METHOD(assign_vld);
      sensitive << bind_in.i_vld;
    }

    void assign_dat() { bind_out.o_dat.write(bind_in.i_dat.read()); }
    void assign_vld() { bind_out.o_vld.write(bind_in.i_vld.read()); }
  };

};
#endif
