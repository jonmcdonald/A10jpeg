
/**************************************************************/
/*                                                            */
/*      Copyright Mentor Graphics Corporation 2006 - 2012     */
/*                  All Rights Reserved                       */
/*                                                            */
/*       THIS WORK CONTAINS TRADE SECRET AND PROPRIETARY      */
/*         INFORMATION WHICH IS THE PROPERTY OF MENTOR        */
/*         GRAPHICS CORPORATION OR ITS LICENSORS AND IS       */
/*                 SUBJECT TO LICENSE TERMS.                  */
/*                                                            */
/**************************************************************/
//=================================================================================
// VIVY (VIsta VerifY) Vista template for connecting with RTL simulation/emulation
// History:
// 7/10/2015  v1.0  Initial Release  Mike Bradley 
//
//=================================================================================

/*@include "esl/pv_gen/HeaderPv.template"*/

#pragma once
#include "$(BASE_CLASS_INCLUDE)"
#include "uvmc.h"
// May need "tlm2/tlm_utils" to find the simple sockets (e.g. for NCsim)
#include <tlm_utils/simple_initiator_socket.h>
#include <tlm_utils/simple_target_socket.h>

// Note: To debug UVM Connect interprocess calls: 
//       "setenv XL_TLM_ENABLE_IPC_SNOOPING yes" on Vista and/or Verilog side

using namespace tlm; 
/*is_exist(TARGET_WITH_CALLBACK){*/
// Uncomment #defines below for those target busses that implement debug transactions
// This is implemented as a separate UVM Connect socket to Verilog /*}*/ /*<*/
//#define ENABLE_$(TARGET_WITH_CALLBACK)_DEBUG_SOCKET /*>*/

//This class inherits from the $(PV_BASE_CLASS_NAME) class
class $(CLASS_NAME) : public $(PV_BASE_CLASS_NAME) {

  SC_HAS_PROCESS($(CLASS_NAME));
 public:
  // Constructor
  // Do not add parameters here.
  // To add parameters - use the Model Builder form (under PV->Parameters tab)
  $(CLASS_NAME)(sc_module_name module_name);/*is_exist(REG_WITH_CALLBACK_ON_READ_LIST){*/

 ////////////////////////////////////////
 // read callbacks of registers
 /////////////////////////////////////////
 protected:/*}*//*<*/
  // This function will be called whenever the register $(REG_WITH_CALLBACK_ON_READ_LIST) is read 
  $(REG_WITH_CALLBACK_ON_READ_DATA_TYPE_LIST) cb_read_$(REG_WITH_CALLBACK_ON_READ_LIST)();/*>*/ /*is_exist(REG_WITH_CALLBACK_ON_WRITE_LIST){*/
  
 /////////////////////////////////////////
 // write callbacks of registers
 /////////////////////////////////////////
 protected: /*}*/ /*<*/
  // This function will be called whenever the register $(REG_WITH_CALLBACK_ON_WRITE_LIST) is written
  void cb_write_$(REG_WITH_CALLBACK_ON_WRITE_LIST)($(REG_WITH_CALLBACK_ON_WRITE_DATA_TYPE_LIST) newValue); /*>*/  /*is_exist(IO_RANGE_LIST){*/

  /////////////////////////////////////////
 // write callbacks of io_memory
 /////////////////////////////////////////
  protected: /*}*/
  /*<*/
  virtual void cb_write_$(IO_RANGE_LIST)(uint64_t address, unsigned char* data, unsigned length);
  virtual void cb_read_$(IO_RANGE_LIST)(uint64_t address, unsigned char* data, unsigned length);/*>*/ /*is_exist(TARGET_WITH_CALLBACK){*/
 
 ////////////////////////////////////////////////////////////
 // target port TLM transaction access, and socket to UVM
 ////////////////////////////////////////////////////////////
 protected:/*}*/ /*<*/
  virtual void $(TARGET_WITH_CALLBACK)_callback(tlm::tlm_base_protocol_types::tlm_payload_type& trans, sc_core::sc_time& t);
  tlm::tlm_sync_enum $(TARGET_WITH_CALLBACK)_nb_transport_bw(tlm::tlm_generic_payload& trans, tlm::tlm_phase& phase, sc_core::sc_time& delay);
  tlm_utils::simple_initiator_socket<$(CLASS_NAME)>* $(TARGET_WITH_CALLBACK)_socket_to_uvmc;
  #ifdef ENABLE_$(TARGET_WITH_CALLBACK)_DEBUG_SOCKET 
    tlm_utils::simple_initiator_socket<$(CLASS_NAME)>* $(TARGET_WITH_CALLBACK)_debug_socket_to_uvmc;
  #endif /*>*/ 


 /*is_exist(SIGNALS_IN){*/
 /////////////////////////////////////////////////////
 // signals in callbacks and initiator to UVM Connect
 ///////////////////////////////////////////////////// 
 protected:/*}*/ /*<*/
  void $(SIGNALS_IN)_callback();
  tlm::tlm_base_protocol_types::tlm_payload_type $(SIGNALS_IN)_trans;  // used in callback
  tlm::tlm_sync_enum $(SIGNALS_IN)_nb_transport_bw(tlm::tlm_generic_payload& trans, tlm::tlm_phase& phase, sc_core::sc_time& delay);
  tlm_utils::simple_initiator_socket<$(CLASS_NAME)>* $(SIGNALS_IN)_socket_to_uvmc;/*>*/ /*is_exist(MASTER){*/

 ///////////////////////////////////////////////////////////
 // Master port socket and socket callback from UVM Connect
 ///////////////////////////////////////////////////////////
 protected:/*}*/ /*<*/
  tlm_utils::simple_target_socket<$(CLASS_NAME)>* $(MASTER)_socket_from_uvmc;
  void $(MASTER)_b_transport(tlm::tlm_generic_payload& trans, sc_core::sc_time& delay);
  tlm::tlm_sync_enum $(MASTER)_nb_transport_fw(tlm::tlm_generic_payload& trans, tlm::tlm_phase& phase, sc_core::sc_time& delay);/*>*/ /*is_exist(SIGNALS_OUT){*/

 ///////////////////////////////////////////////////////////
 // Signals_Out: socket callback from UVM Connect
 ///////////////////////////////////////////////////////////
 protected:/*}*/ /*<*/
  tlm_utils::simple_target_socket<$(CLASS_NAME)>* $(SIGNALS_OUT)_socket_from_uvmc;
  void $(SIGNALS_OUT)_b_transport(tlm::tlm_generic_payload& trans, sc_core::sc_time& delay); 
  tlm::tlm_sync_enum $(SIGNALS_OUT)_nb_transport_fw(tlm::tlm_generic_payload& trans, tlm::tlm_phase& phase, sc_core::sc_time& delay);/*>*/ /*is_exist(TARGET_WITH_CALLBACK){*/


 ///////////////////////////////////////////////////////////////
 // debug and dmi callbacks for target TLM ports (return false)
 ///////////////////////////////////////////////////////////////
 protected: /*}*/ /*<*/
  unsigned $(TARGET_WITH_CALLBACK)_callback_read_dbg(config::uint64 address, unsigned char* data, unsigned size/*<*/, long $($(TARGET_WITH_CALLBACK)#1_PV_PARAMETERS)/*>*/);
  unsigned $(TARGET_WITH_CALLBACK)_callback_write_dbg(config::uint64 address, unsigned char* data, unsigned size/*<*/, long $($(TARGET_WITH_CALLBACK)#1_PV_PARAMETERS)/*>*/);
  bool $(TARGET_WITH_CALLBACK)_get_direct_memory_ptr(config::uint64 address, tlm::tlm_dmi& dmiData); /*>*/ 

 ///////////////////////////////////////////////////////////////
 // functions, threads, data for VIVY operation
 ///////////////////////////////////////////////////////////////

  vector<string> uvmcConnectNames; /*<*/ 
  void thread_$(SIGNALS_OUT)(); /*>*/ /*<*/ 
  tlm_fifo<unsigned long long> $(SIGNALS_OUT)_fifo; /*>*/

};
