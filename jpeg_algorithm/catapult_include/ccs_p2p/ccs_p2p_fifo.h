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

#ifndef __CCS_P2P_FIFO_H
#define __CCS_P2P_FIFO_H
#include <ccs_types.h>

//----------------------------------------------------------------------------
// Interconnect: p2p_fifo
//   Three signals used to directly connect two blocks.
//     <name>_vld:  Data is valid, active high, driven active by the source when data is valid.
//     <name>_dat:  Data to be transmitted across interconnect, 
//     <name>_rdy:  Target block is ready to take the data; this signal is high in any cycle
//   Data is transmitted in any cycle where "vld" and "rdy" are active.
//   Underlying datatypes are bool for one-bit and sc_lv for vector types to allow clean
//   mixed-language simulation.
//   The interconnect has a latency of one and supports a throughput of one.
//   Supported Methods:
//     out:
//         write(T data):  Writes data into channel, blocks process until completion
//         bool nb_write(T data):  Attempts to write data into channel.
//                                  Returns false if data can not be written.
//     in:
//        T read():  Reads data from channel, blocks process until completion
//        bool nb_read(T& data):  Attempts to read data from channel and set data to read value.  
//                                  Returns false if data can not be read.
//   Most common declaration examples, which result in transaction level simulation:
//   NOTE: Second template argument to channel is the depth of the FIFO.  Min depth is 1.
//     p2p_fifo::chan<data_T, 10> my_channel;
//     p2p_fifo::in<data_T> my_input_port;
//     p2p_fifo::out<data_T> my_output_port;
//   To declare pin-accurate interconnects use SYN template parameter:
//     p2p_fifo<SYN>::chan<data_T, 10> my_channel;
//     p2p_fifo<SYN>::in<data_T> my_input_port;
//     p2p_fifo<SYN>::out<data_T> my_output_port;

template <class T, 
          unsigned int fifo_depth,
          bool clk_neg = false,
          bool rst_neg = true>
SC_MODULE(p2p_fifo_impl) {
  // Get bitwidth and signness of T for conversion to sc_lv
  enum {signedtype = mc_typedef_T_traits<T>::issigned,
        data_width = mc_typedef_T_traits<T>::bitwidth,
        index_width = nbits<fifo_depth>::val};

public:
  // Input with handshake
  sc_in <sc_lv<data_width> > i_dat ;
  sc_in <bool> i_vld ;
  sc_out <bool> o_rdy ;

  // Output with handhshake
  sc_out <sc_lv<data_width> > o_dat ;
  sc_out <bool> o_vld ;
  sc_in <bool> i_rdy ;

  // clock and reset
  sc_in <bool> clk ;
  sc_in <bool> rst ;

  // Keep storage as type T for easier debugging
  T storage[fifo_depth] ;
  sc_signal< sc_uint<index_width> > write_index ;
  sc_signal< bool > full ;
  sc_signal< bool > empty ;

  bool index_is_zero ; // used to indicate empty
  bool index_is_zero_plus_one ;
  bool index_is_max ;
  bool index_is_max_plus_one ;


  SC_CTOR( p2p_fifo_impl )
    : i_dat("i_dat")
    , i_vld("i_vld")
    , o_rdy("o_rdy")
    , o_dat("o_dat")
    , o_vld("o_vld")
    , i_rdy("i_rdy")
    , clk("clk")
    , rst("rst")
    , write_index("write_index")
    , full("full")
    , empty("empty")
  {
    SC_METHOD(rtl);
    sensitive << (clk_neg?clk.neg():clk.pos()) 
              << (rst_neg?rst.neg():rst.pos()) ;
  }

  void rtl() {
    T zero_T;
    sc_lv<data_width> zero_lv = sc_dt::Log_0;
    vector_to_type(zero_lv, signedtype, &zero_T);
    if (rst.read()==!rst_neg) {
      o_rdy.write(true); // always start requesting
      o_dat.write(0) ;
      o_vld.write(false) ;
      // reset storage registers
      for (int i=0;i<fifo_depth;i++) {
        storage[i] = zero_T;
      }
      write_index.write(0) ;
      // full is 0, empty is true
      full = false ;
      empty = true ;
    } else { // clock edge
      sc_uint<2> rw_condition ;
      rw_condition[1] = i_vld.read();
      rw_condition[0] = i_rdy.read();

      // tests on indexing
      index_is_zero = (write_index.read()==0) ;
      index_is_zero_plus_one = (write_index.read()==1) ;
      index_is_max = (write_index.read()==(fifo_depth-1)) ;
      index_is_max_plus_one = (write_index.read()==fifo_depth) ;

      // next state variables for full and empty default to no change
      bool full_next = full ;
      bool empty_next = empty ;
      // deal with full/empty flagging

      switch (rw_condition) {
        case 1 : // read_request with no write
          if (empty) { // do nothing as nothing to read
          } else if (full) {
            full_next = false ; // can't be full as a read without write
            // check for empty
            if (index_is_zero_plus_one) { // could be going empty (catches 1-deep)
              empty_next = true ;
            }
          } else { // not full or empty
            if (index_is_zero_plus_one) { // could be going empty
              empty_next = true ;
            }
          }
        break ;
        case 2 : // store data with no read
          if (empty) {
            empty_next = false ;
            // check for full
            if (index_is_max) { // going full (catches 1-deep)
              full_next = true ;
            }
          } else if (full) { // do nothing.
          } else { // neither full nor empty
            if (index_is_max) { // check for going full
              full_next = true ;
            }
          }
        break ;
        case 3 : // read and write at the same time means always shift and store - need to consider when empty or full
          if (empty) { // can accept data in, but no data will be consumed out
            empty_next = false ;
            // check for full
            if (index_is_max) {
              full_next = true ;
            }
          } else if (full) { // read only
            full_next = false ;
            if (index_is_zero_plus_one) { // going empty
              empty_next = true ;
            }
          } else { // neither full nor empty, read/write is OK.
          }
        break ;
        default : // do nothing on case zero as no read request, no write request
        break ;
      }

      // manipulate storage - always unrolled for registers implementation
      for (int i=0; i<fifo_depth; i++) {
        bool index_match = (i==write_index.read()) ;
        bool shift_match = (i==(write_index.read()-1)) ;
        T temp_data ;
        switch (rw_condition) {
          case 1 : // read_request with no write
            if (empty) { // do nothing as nothing to read
              temp_data = storage[i] ; // recirculate
            } else { // not empty, so one value will be consumed downstream - need to shift
              temp_data = (i == (fifo_depth-1)) ? zero_T : storage[i+1] ;
            }
          break ;
          case 2 : // store data with no read - just a write, no shift
            if (full) { // do nothing
              temp_data = storage[i] ;
            } else { // not full => store on index hit
              if ( index_match ) {
                vector_to_type(i_dat.read(),(signedtype?true:false),&temp_data);
              } else {
                temp_data = storage[i] ;
              }
            }
          break ;
          case 3 : // read request and write request at the same time
            if (empty) { // we just write
              if ( index_match ) {
                vector_to_type(i_dat.read(),(signedtype?true:false),&temp_data);
              } else {
                temp_data = storage[i] ;
              }
            } else if (full) { // we can just read 
              temp_data = (i == (fifo_depth-1)) ? zero_T : storage[i+1] ;
            } else { // neither empty nor full
                     // so can read and write which involves shift and update - inject data to top, regardless
              T data;
              vector_to_type(i_dat.read(),(signedtype?true:false),&data);
              temp_data = (shift_match) ? data : ((i == (fifo_depth-1)) ? data : storage[i+1]) ;
            }
          break ;
          default : 
            temp_data = storage[i] ;
          break ;
        }
        storage[i] = temp_data ;
      }

      // update write_index
      switch (rw_condition) {
        case 1 : // read_request with no write
          if (empty) { // do nothing as nothing to read
          } else { // must be something, so can decrement
            write_index.write(write_index.read()-1) ;
          }
        break ;
        case 2 : // store data with no read
          if (full) { // do nothing
          } else {
            write_index.write(write_index.read()+1) ;
          }
        break ;
        case 3 : // read and write at the same time means no change to write_index unless we were empty
          if (empty) { // increment as only write, no read
            write_index.write(write_index.read()+1) ;
          } else if (full) { // only allow a read
            write_index.write(write_index.read()-1) ;
          }
        break ;
        default : // do nothing on case zero
        break ;
      }

      // update full and empty signals
      full.write(full_next) ;
      empty.write(empty_next) ;
      // manage output flags
      o_rdy.write(!full_next) ;
      sc_lv<data_width> temp;
      type_to_vector(storage[0], (signedtype?true:false), temp);
      o_dat.write(temp) ;
      o_vld.write(!empty_next) ;
    }
  }
};

// Specialization for depth 1
// Feedback path is unregistered
template <class T,
          bool clk_neg,
          bool rst_neg>
struct p2p_fifo_impl<T,1,rst_neg,clk_neg> : ::sc_core::sc_module {
  // Get bitwidth and signness of T for conversion to sc_lv
  enum {signedtype = mc_typedef_T_traits<T>::issigned,
        data_width = mc_typedef_T_traits<T>::bitwidth,
        index_width = nbits<1>::val};

public:
  // Input with handshake
  sc_in <sc_lv<data_width> > i_dat ;
  sc_in <bool> i_vld ;
  sc_out <bool> o_rdy ;

  // Output with handhshake
  sc_out <sc_lv<data_width> > o_dat ;
  sc_out <bool> o_vld ;
  sc_in <bool> i_rdy ;

  // clock and reset
  sc_in <bool> clk ;
  sc_in <bool> rst ;

  // Local copies of output signals
  sc_signal <bool> vld, rdy;

  SC_CTOR( p2p_fifo_impl )
    : i_dat("i_dat")
    , i_vld("i_vld")
    , o_rdy("o_rdy")
    , o_dat("o_dat")
    , o_vld("o_vld")
    , i_rdy("i_rdy")
    , clk("clk")
    , rst("rst")
    , vld("vld")
    , rdy("rdy")
  {
    SC_METHOD(rtl);
    sensitive << (clk_neg?clk.neg():clk.pos())
              << (rst_neg?rst.neg():rst.pos()) ;
    SC_METHOD(back_pressure);
    sensitive << i_rdy << vld;
  }

  void rtl () {
    T zero_T;
    sc_lv<data_width> zero_lv = sc_dt::Log_0;
    vector_to_type(zero_lv, signedtype, &zero_T);
    if (rst.read()==!rst_neg) {
      o_dat.write(0) ;
      o_vld.write(false) ;
      vld.write(false);
    } else { // clock edge
      if ( i_vld.read() & rdy.read() )
        o_dat.write(i_dat.read());
      if ( rdy.read() ) {
        o_vld.write(i_vld.read());
        vld.write(i_vld.read());
      } 
    }
  }
  
  void back_pressure () {
    o_rdy.write(!vld.read() | i_rdy.read());
    rdy.write(!vld.read() | i_rdy.read());
  }

};

// Specialization for depth zero
// Just pass wires through
template <class T,
          bool clk_neg,
          bool rst_neg>
struct p2p_fifo_impl<T,0,rst_neg,clk_neg> : ::sc_core::sc_module {
  // Get bitwidth and signness of T for conversion to sc_lv
  enum {signedtype = mc_typedef_T_traits<T>::issigned,
        data_width = mc_typedef_T_traits<T>::bitwidth
       };

public:
  // Input with handshake
  sc_in <sc_lv<data_width> > i_dat ;
  sc_in <bool> i_vld ;
  sc_out <bool> o_rdy ;

  // Output with handhshake
  sc_out <sc_lv<data_width> > o_dat ;
  sc_out <bool> o_vld ;
  sc_in <bool> i_rdy ;

  // clock and reset
  sc_in <bool> clk ;
  sc_in <bool> rst ;

  SC_CTOR( p2p_fifo_impl )
    : i_dat("i_dat")
    , i_vld("i_vld")
    , o_rdy("o_rdy")
    , o_dat("o_dat")
    , o_vld("o_vld")
    , i_rdy("i_rdy")
    , clk("clk")
    , rst("rst")
  {
    SC_METHOD(assign_o_dat); sensitive << i_dat;
    SC_METHOD(assign_o_rdy); sensitive << i_rdy;
    SC_METHOD(assign_o_vld); sensitive << i_vld;
  }
  void assign_o_dat() { o_dat.write(i_dat.read()); }
  void assign_o_rdy() { o_rdy.write(i_rdy.read()); }
  void assign_o_vld() { o_vld.write(i_vld.read()); }
};

// Base Template
template <abstraction_t source_abstraction = AUTO>
class p2p_fifo  {
public:

// Base Template Definition, override defaults for synthesis
// Do not overried setting on impl_abstraction template parameter!
// For synthesis, always use SYN view
#if defined(__SYNTHESIS__) || defined (CALYPTO_SYSC)
  template <class T, 
            unsigned int fifo_depth = 16, 
            bool clk_neg = false, 
            bool rst_neg = true, 
            abstraction_t impl_abstraction = SYN> 
  	class chan {};
  template <class T, abstraction_t impl_abstraction = SYN> class assign {};
  template <class T, abstraction_t impl_abstraction = SYN> class in {};
  template <class T, abstraction_t impl_abstraction = SYN> class out {};
// For SCVerify, default to SYN unless hard-coded to TLM for testbench only channels
#elif defined (CCS_DUT_CYCLE) || defined (CCS_DUT_RTL)
 template <class T,
            unsigned int fifo_depth = 16,
            bool clk_neg = false,
            bool rst_neg = true,
            abstraction_t impl_abstraction = (source_abstraction==AUTO) ? SYN : source_abstraction>
  	class chan {};
  template <class T, abstraction_t impl_abstraction = (source_abstraction==AUTO) ? SYN : source_abstraction> class assign {};
  template <class T, abstraction_t impl_abstraction = (source_abstraction==AUTO) ? SYN : source_abstraction> class in {};
  template <class T, abstraction_t impl_abstraction = (source_abstraction==AUTO) ? SYN : source_abstraction> class out {};
#else
  template <class T,
            unsigned int fifo_depth = 16,
            bool clk_neg = false,
            bool rst_neg = true,
            abstraction_t impl_abstraction = (source_abstraction==AUTO) ? P2P_DEFAULT_VIEW : source_abstraction>
  	class chan {};
  template <class T, abstraction_t impl_abstraction = (source_abstraction==AUTO) ? P2P_DEFAULT_VIEW : source_abstraction> class assign {};
  template <class T, abstraction_t impl_abstraction = (source_abstraction==AUTO) ? P2P_DEFAULT_VIEW : source_abstraction> class in {};
  template <class T, abstraction_t impl_abstraction = (source_abstraction==AUTO) ? P2P_DEFAULT_VIEW : source_abstraction> class out {};
#endif

#ifndef CALYPTO_SYSC
  // TLM channel definition
  // NOTE:  Minimum FIFO depth is one or SystemC simulation will hang!
  //        Use ccs_wire for zero depth FIFO
  template <class T, unsigned int fifo_depth, bool clk_neg, bool rst_neg>
  class chan <T,fifo_depth,clk_neg,rst_neg,TLM> {
    p2p_checker rd_chk, wr_chk, clk_chk, rst_chk;

  public:
    tlm::tlm_fifo<T> fifo;

    chan(const char *name) 
      : rd_chk(name, "call reset_read()", "read from this channel")
      , wr_chk(name, "call reset_write()", "write to this channel")
      , clk_chk(name, "bind the clk port", "access this channel")
      , rst_chk(name, "bind the rst port", "access this channel")
      , fifo(ccs_concat(name, "fifo"),fifo_depth) 
      {}

    // Empty FIFO on reset
    void reset_read() {
      T temp; 
      while ( fifo.nb_get(temp)); 
      rd_chk.ok();
    }
    void reset_write() {wr_chk.ok();}

    T read() { 
      rd_chk.test(); 
      clk_chk.test();
      rst_chk.test();
      return fifo.get(); 
    }

    bool nb_read(T &data) { 
      rd_chk.test(); 
      clk_chk.test();
      rst_chk.test();
      return fifo.nb_get(data); 
    }

    void write(T data) { 
      wr_chk.test(); 
      clk_chk.test();
      rst_chk.test();
      fifo.put(data); 
    }

    bool nb_write(T data) { 
      wr_chk.test(); 
      clk_chk.test();
      rst_chk.test();
      return fifo.nb_put( data ); 
    }

    // Empty clock and reset bindings (not needed for TLM)
    void clk (sc_in<bool> &clock) {clk_chk.ok();}
    void clk (sc_signal<bool> &clock) {clk_chk.ok();}
    void rst (sc_in<bool> &reset) {rst_chk.ok();}
    void rst (sc_signal<bool> &reset) {rst_chk.ok();}

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

    void write(T data) { wr_chk.test(); o_fifo->put(data); }
    bool nb_write(T data) { wr_chk.test(); return o_fifo->nb_put( data ); }

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
        bind_out.write(bind_in.read());
      }
    }
  };
#endif
 
  // SYN channel definition
  template <class T, unsigned int fifo_depth, bool clk_neg, bool rst_neg>
  class chan <T,fifo_depth,clk_neg,rst_neg,SYN> {
    // Get bitwidth and signness of T for conversion to sc_lv
    enum {signedtype = mc_typedef_T_traits<T>::issigned,
          data_width = mc_typedef_T_traits<T>::bitwidth};

    p2p_fifo_impl <T,fifo_depth, clk_neg, rst_neg> fifo;
    p2p_checker rd_chk, wr_chk, clk_chk, rst_chk;

  public:
    sc_signal <sc_lv<data_width> > dat_in,dat_out;
    sc_signal <bool> vld_in,vld_out;
    sc_signal <bool> rdy_in,rdy_out;

    chan(const char *name) 
      : rd_chk(name, "call reset_read()", "read from this channel")
      , wr_chk(name, "call reset_write()", "write to this channel")
      , clk_chk(name, "bind the clk port", "access this channel")
      , rst_chk(name, "bind the rst port", "access this channel")
      , dat_in( ccs_concat(name,"dat_in") )
      , vld_in( ccs_concat(name,"vld_in") )
      , rdy_in( ccs_concat(name,"rdy_in") )
      , dat_out( ccs_concat(name,"dat_out") )
      , vld_out( ccs_concat(name,"vld_out") )
      , rdy_out( ccs_concat(name,"rdy_out") ) 
      , fifo( ccs_concat(name,"fifo") )
    {
      fifo.i_dat(dat_in);
      fifo.i_vld(vld_in);
      fifo.o_rdy(rdy_in);

      fifo.o_dat(dat_out);
      fifo.o_vld(vld_out);
      fifo.i_rdy(rdy_out);
    }

    void clk (sc_in<bool> &clock) {
      fifo.clk(clock);
      clk_chk.ok();
    }

    void clk (sc_signal<bool> &clock) {
      fifo.clk(clock);
      clk_chk.ok();
    }

    void rst (sc_in<bool> &reset) {
      fifo.rst(reset);
      rst_chk.ok();
    }

    void rst (sc_signal<bool> &reset) {
      fifo.rst(reset);
      rst_chk.ok();
    }

    void reset_write() {
      dat_in.write(0);
      vld_in.write(false);
      wr_chk.ok();
    }

    void reset_read() {
      rdy_out.write(false);
      rd_chk.ok();
    }

    #pragma design modulario
    T read() {
      rd_chk.test();
      clk_chk.test();
      rst_chk.test(); 
      do {
        rdy_out.write(true) ;
        wait() ;
      } while (vld_out.read() != true );
      rdy_out.write(false);
      T data;
      vector_to_type(dat_out.read(),(signedtype?true:false),&data);
      return data;
    }

    #pragma design modulario
    bool nb_read(T &data) {
      rd_chk.test();
      clk_chk.test();
      rst_chk.test(); 
      rdy_out.write(true) ;
      wait() ;
      rdy_out.write(false);
      vector_to_type(dat_out.read(),(signedtype?true:false),&data);
      return vld_out.read();
    }

    #pragma design modulario
    void write (T data) {
      wr_chk.test();
      clk_chk.test();
      rst_chk.test(); 
      do {
        sc_lv<data_width> temp;
        type_to_vector(data, (signedtype?true:false), temp);
        dat_in.write(temp);
        vld_in.write(true);
        wait();
      } while (rdy_in.read() != true );
      vld_in.write(false);
    }

    #pragma design modulario
    bool nb_write (T data) {
      wr_chk.test();
      clk_chk.test();
      rst_chk.test(); 
      sc_lv<data_width> temp;
      type_to_vector(data, (signedtype?true:false), temp);
      dat_in.write(temp);
      vld_in.write(true);
      wait();
      vld_in.write(false);
      // Return outputs to dc to save area
      sc_lv<data_width> dc;
      dat_in.write(dc);
      return rdy_in.read();
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
    sc_out <bool> o_rdy ;

    in(const char *name) 
      : rd_chk(name, "call reset_read()", "read from this port")
      , i_dat( ccs_concat(name,"i_dat") )
      , i_vld( ccs_concat(name,"i_vld") )
      , o_rdy( ccs_concat(name,"o_rdy") ) 
      {}

      void reset_read() {
        o_rdy.write(false);
        rd_chk.ok();
      }

    #pragma design modulario
    T read() {
      rd_chk.test();
      do {
        o_rdy.write(true) ;
        wait() ;
      } while (i_vld.read() != true );
      o_rdy.write(false);
      T data;
      vector_to_type(i_dat.read(),(signedtype?true:false),&data);
      return data;
    }

    #pragma design modulario
    bool nb_read(T &data) {
      rd_chk.test();
      o_rdy.write(true) ;
      wait() ;
      o_rdy.write(false);
      vector_to_type(i_dat.read(),(signedtype?true:false),&data);
      return i_vld.read();
    }

    void bind (in<T>& c) {
      i_vld(c.i_vld);
      i_dat(c.i_dat);
      o_rdy(c.o_rdy);
    }
    
    template <class C>
    void bind (C& c) {
      i_vld(c.vld_out);
      i_dat(c.dat_out);
      o_rdy(c.rdy_out);
    }

    template <class C>
    void operator() (C& c) {
      bind(c);
    }
  };

  // SYN output port definition
  template <class T>
  class out<T,SYN> {
  // Get bitwidth and signness of T for conversion to sc_lv
  enum {signedtype = mc_typedef_T_traits<T>::issigned,
        data_width = mc_typedef_T_traits<T>::bitwidth};
    p2p_checker wr_chk;

  public:
    sc_out <sc_lv<data_width> > o_dat ;
    sc_out <bool> o_vld ;
    sc_in <bool> i_rdy ;

    out(const char *name) 
      : wr_chk(name, "call reset_write()", "write to this port")
      , o_dat( ccs_concat(name,"o_dat") )
      , o_vld( ccs_concat(name,"o_vld") )
      , i_rdy( ccs_concat(name,"i_rdy") ) 
      {}

    void reset_write() {
      o_dat.write(0);
      o_vld.write(false);
      wr_chk.ok();
    }
  
    #pragma design modulario
    void write (T data) {
      wr_chk.test();
      do {
        sc_lv<data_width> temp;
        type_to_vector(data, (signedtype?true:false), temp);
        o_dat.write(temp);
        o_vld.write(true);
        wait();
        } while (i_rdy.read() != true );
      o_vld.write(false);
    }

    #pragma design modulario
    bool nb_write (T data) {
      wr_chk.test();
      sc_lv<data_width> temp;
      type_to_vector(data, (signedtype?true:false), temp);
      o_dat.write(temp);
      o_vld.write(true);
      wait();
      o_vld.write(false);
      // Return outputs to dc to save area
      sc_lv<data_width> dc;
      o_dat.write(dc);
      return i_rdy.read();
    }
  
    void bind (out<T>& c) {
      o_vld(c.o_vld);
      o_dat(c.o_dat);
      i_rdy(c.i_rdy);
    }
  
    template <class C>
    void bind (C& c) {
      o_vld(c.vld_in);
      o_dat(c.dat_in);
      i_rdy(c.rdy_in);
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
      SC_METHOD(assign_rdy);
      sensitive << bind_out.i_rdy;
    }

    void assign_dat() { bind_out.o_dat.write(bind_in.i_dat.read()); }
    void assign_vld() { bind_out.o_vld.write(bind_in.i_vld.read()); }
    void assign_rdy() { bind_in.o_rdy.write(bind_out.i_rdy.read()); }
  };

};
#endif
