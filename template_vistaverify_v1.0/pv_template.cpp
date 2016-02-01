
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

/*@include "esl/pv_gen/HeaderPv.template"*/

#include "$(CLASS_NAME).h"
#include <iostream>
#include <string>

//#define VIVY_DEBUG


//constructor
$(CLASS_NAME)::$(CLASS_NAME)(sc_module_name module_name) 
  : $(PV_BASE_CLASS_NAME)(module_name) /*<*/
   ,$(SIGNALS_OUT)_fifo("$(SIGNALS_OUT)_fifo",-1) /*>*/
{
    if (! vivy_disable) 
    { /*<*/
        $(TARGET_WITH_CALLBACK)_socket_to_uvmc = new  tlm_utils::simple_initiator_socket<$(CLASS_NAME)>("$(TARGET_WITH_CALLBACK)_socket_to_uvmc"); /*>*/ /*<*/ 
        #ifdef ENABLE_$(TARGET_WITH_CALLBACK)_DEBUG_SOCKET 
          $(TARGET_WITH_CALLBACK)_debug_socket_to_uvmc = new  tlm_utils::simple_initiator_socket<$(CLASS_NAME)>("$(TARGET_WITH_CALLBACK)_debug_socket_to_uvmc"); 
        #endif /*>*/ /*<*/
        $(SIGNALS_IN)_socket_to_uvmc = new  tlm_utils::simple_initiator_socket<$(CLASS_NAME)>("$(SIGNALS_IN)_socket_to_uvmc"); /*>*/ /*<*/
        $(MASTER)_socket_from_uvmc = new  tlm_utils::simple_target_socket<$(CLASS_NAME)>("$(MASTER)_socket_from_uvmc"); /*>*/ /*<*/
        $(SIGNALS_OUT)_socket_from_uvmc = new  tlm_utils::simple_target_socket<$(CLASS_NAME)>("$(SIGNALS_OUT)_socket_from_uvmc"); /*>*/ /*<*/

        SC_THREAD(thread_$(SIGNALS_OUT)); /*>*/

        // Bindings to UVM Connect (uvmcName must be unique)
        string uvmcName;
        string instName = name();
        instName.erase(instName.length()-2,2); 

        // use colon for running Vista in a separate process, as well as single process
        string uvmcColon_prefix = ":uvmc_"; 

        // store hierarchical port name in 'uvmcConnectNames' for printf
        // Also store uvmc connect string in 'uvmcConnectNames' for uvmc_connect call
        // Use default uvmc connect string, or value from parameters.txt /*<*/
        uvmcConnectNames.push_back(string(instName + string("$(SIGNALS_IN)"))); 
        if (*uvmc_$(SIGNALS_IN) == 0) 
            uvmcName = uvmcColon_prefix + string("$(SIGNALS_IN)");
        else 
            uvmcName = string(uvmc_$(SIGNALS_IN));
        uvmcConnectNames.push_back(uvmcName);  
        // Using template parameter "<uvmc_xl_converter<tlm_generic_payload> >" enables "Fast Packers" 
        uvmc_connect <uvmc_xl_converter<tlm_generic_payload> > (*$(SIGNALS_IN)_socket_to_uvmc, uvmcName); /*>*/ /*<*/

        uvmcConnectNames.push_back(string(instName + string("$(TARGET_WITH_CALLBACK)")));
        if (*uvmc_$(TARGET_WITH_CALLBACK) == 0) 
            uvmcName = uvmcColon_prefix + string("$(TARGET_WITH_CALLBACK)");
        else 
            uvmcName = string(uvmc_$(TARGET_WITH_CALLBACK));
        uvmcConnectNames.push_back(uvmcName);  
        uvmc_connect <uvmc_xl_converter<tlm_generic_payload> > (*$(TARGET_WITH_CALLBACK)_socket_to_uvmc, uvmcName);/*>*/ /*<*/
        
        #ifdef ENABLE_$(TARGET_WITH_CALLBACK)_DEBUG_SOCKET 
          uvmcConnectNames.push_back(string(instName + string("dbg_$(TARGET_WITH_CALLBACK)")));
          if (*uvmc_dbg_$(TARGET_WITH_CALLBACK) == 0) 
              uvmcName = uvmcColon_prefix + string("dbg_$(TARGET_WITH_CALLBACK)");
          else 
              uvmcName = string(uvmc_dbg_$(TARGET_WITH_CALLBACK));
          uvmcConnectNames.push_back(uvmcName);  
          uvmc_connect <uvmc_xl_converter<tlm_generic_payload> > (*$(TARGET_WITH_CALLBACK)_debug_socket_to_uvmc, uvmcName);
        #endif /*>*/ /*<*/

        uvmcConnectNames.push_back(string(instName + string("$(MASTER)")));
        if (*uvmc_$(MASTER) == 0) 
            uvmcName = uvmcColon_prefix + string("$(MASTER)");
        else 
            uvmcName = string(uvmc_$(MASTER));
        uvmcConnectNames.push_back(uvmcName);  
        uvmc_connect <uvmc_xl_converter<tlm_generic_payload> > (*$(MASTER)_socket_from_uvmc, uvmcName);/*>*/ /*<*/

        uvmcConnectNames.push_back(string(instName + string("$(SIGNALS_OUT)")));
        if (*uvmc_$(SIGNALS_OUT) == 0) 
            uvmcName = uvmcColon_prefix + string("$(SIGNALS_OUT)");
        else 
            uvmcName = string(uvmc_$(SIGNALS_OUT));
        uvmcConnectNames.push_back(uvmcName);  
        uvmc_connect <uvmc_xl_converter<tlm_generic_payload> > (*$(SIGNALS_OUT)_socket_from_uvmc, uvmcName);/*>*/ 

        // register simple_target_socket transport callbacks /*<*/
        $(MASTER)_socket_from_uvmc->register_b_transport( this, &$(CLASS_NAME)::$(MASTER)_b_transport); /*>*//*<*/
        $(MASTER)_socket_from_uvmc->register_nb_transport_fw( this, &$(CLASS_NAME)::$(MASTER)_nb_transport_fw); /*>*//*<*/

        $(SIGNALS_OUT)_socket_from_uvmc->register_b_transport( this, &$(CLASS_NAME)::$(SIGNALS_OUT)_b_transport); /*>*//*<*/
        $(SIGNALS_OUT)_socket_from_uvmc->register_nb_transport_fw( this, &$(CLASS_NAME)::$(SIGNALS_OUT)_nb_transport_fw); /*>*//*<*/

        $(SIGNALS_IN)_socket_to_uvmc->register_nb_transport_bw( this, &$(CLASS_NAME)::$(SIGNALS_IN)_nb_transport_bw); /*>*//*<*/

        $(TARGET_WITH_CALLBACK)_socket_to_uvmc->register_nb_transport_bw( this, &$(CLASS_NAME)::$(TARGET_WITH_CALLBACK)_nb_transport_bw); /*>*/

    
        for (vector<string>::iterator it=uvmcConnectNames.begin(); it != uvmcConnectNames.end(); it=it+2) 
            cout <<" VistaVerify UVM Connect path for pin '"<< *(it) <<"' => '"<< *(it+1) <<"'"<<endl;
    } 
}


  /*is_exist(TARGET_WITH_CALLBACK){*/
  ////////////////////////////////////////////
  // Slave bus Transaction forwarded to UVMC
  //////////////////////////////////////////// /*}*/ /*<*/
  void $(CLASS_NAME)::$(TARGET_WITH_CALLBACK)_callback(tlm::tlm_base_protocol_types::tlm_payload_type& trans, sc_core::sc_time& t) 
  {
    if (trans.get_data_length() == 0) {
        cout <<name()<<" @ "<<sc_time_stamp()<<" $(TARGET_WITH_CALLBACK)_callback: Ignoring 0 length transaction "
             <<" addr="<<hex<<showbase<< trans.get_address() <<" streaming_width="<<dec<< trans.get_streaming_width()
             <<" data_length="<< trans.get_data_length() <<" is_write="<< trans.is_write() <<endl;
        return;
    }

    (*$(TARGET_WITH_CALLBACK)_socket_to_uvmc)->b_transport(trans, t);

    #if (defined VIVY_DEBUG || 0)
    if (trans.is_write())
        cout <<name()<<" @ "<<sc_time_stamp()<<" $(TARGET_WITH_CALLBACK)_callback: WRITE addr="<<hex<<showbase<< trans.get_address() 
                     <<" data[3:0]="<< *(unsigned int *)trans.get_data_ptr() <<" size="<<dec<< trans.get_data_length() <<endl;
    else
        cout <<name()<<" @ "<<sc_time_stamp()<<" $(TARGET_WITH_CALLBACK)_callback: READ  addr="<<hex<<showbase<< trans.get_address() 
                     <<" data[3:0]="<< *(unsigned int *)trans.get_data_ptr() <<" size="<<dec<< trans.get_data_length() <<endl;
    #endif

    if (trans.get_response_status() == tlm::TLM_INCOMPLETE_RESPONSE)
        trans.set_response_status(TLM_OK_RESPONSE);  // Not sure if this needed...
  } 

  tlm::tlm_sync_enum $(CLASS_NAME)::$(TARGET_WITH_CALLBACK)_nb_transport_bw(tlm::tlm_generic_payload& trans, tlm::tlm_phase& phase, sc_core::sc_time& delay) 
  { 
    cout <<name()<<" @ "<<sc_time_stamp()<<" WARNING: Hello from $(TARGET_WITH_CALLBACK)_nb_transport_bw() currently does nothing...\n";
    phase = END_RESP;
    trans.set_response_status(TLM_COMMAND_ERROR_RESPONSE);
    return(TLM_COMPLETED);
  } /*>*/ /*is_exist(SIGNALS_IN){*/

  ////////////////////////////////////////////
  // signal in Transaction from Vista to UVMC
  //////////////////////////////////////////// /*}*/ /*<*/
  void $(CLASS_NAME)::$(SIGNALS_IN)_callback()
  {
    sc_core::sc_time    t    = SC_ZERO_TIME;
    tlm::tlm_phase      phase= BEGIN_REQ;
    unsigned            size = getSystemCBaseModel()->get_port_width($(SIGNALS_IN)_idx);
    unsigned long long  data = 0;

    data = $(SIGNALS_IN).read();

    $(SIGNALS_IN)_trans.set_data_ptr(reinterpret_cast<unsigned char*>(&data));
    $(SIGNALS_IN)_trans.set_data_length(size);
    $(SIGNALS_IN)_trans.set_write();
    $(SIGNALS_IN)_trans.set_address(0);
    $(SIGNALS_IN)_trans.set_streaming_width($(SIGNALS_IN)_trans.get_data_length());
    $(SIGNALS_IN)_trans.set_byte_enable_ptr(0);
    $(SIGNALS_IN)_trans.set_byte_enable_length(0);
    $(SIGNALS_IN)_trans.set_dmi_allowed(0);


    #if (defined VIVY_DEBUG || 0)
        cout <<name()<<" @ "<<sc_time_stamp()<<" $(SIGNALS_IN)_callback: WRITE data="
             <<hex<< *(unsigned *)$(SIGNALS_IN)_trans.get_data_ptr() <<" sizeBytes= "<<dec<< size <<endl;
    #endif

    (*$(SIGNALS_IN)_socket_to_uvmc)->nb_transport_fw($(SIGNALS_IN)_trans, phase, t);
  } 
    
  // Unused nb_transport_bw for signals sent to UVMC (other side should early terminate the transaction)
  tlm::tlm_sync_enum $(CLASS_NAME)::$(SIGNALS_IN)_nb_transport_bw(tlm::tlm_generic_payload& trans, tlm::tlm_phase& phase, sc_core::sc_time& delay) { 
      cout <<name()<<" @ "<<sc_time_stamp()<<" WARNING: Hello from $(SIGNALS_IN)_nb_transport_bw() that shouldn't have been called...\n";
      return(TLM_COMPLETED);
  } /*>*/ /*is_exist(MASTER){*/

  //////////////////////////////////////////////////
  // Master bus Transaction from UVMC
  ////////////////////////////////////////////////// /*}*/ /*<*/
  void $(CLASS_NAME)::$(MASTER)_b_transport(tlm::tlm_generic_payload& trans, sc_core::sc_time& delay)
  {
    $(MASTER)_transport(trans);

    #if (defined VIVY_DEBUG || 0)
    if (trans.is_write())
        cout <<name()<<" @ "<<sc_time_stamp()<<" $(MASTER)_b_transport: WRITE addr="<<hex<<showbase<< trans.get_address() 
                     <<" data[3:0]="<< *(unsigned int *)trans.get_data_ptr() <<" size="<<dec<< trans.get_data_length() <<endl;
    else
        cout <<name()<<" @ "<<sc_time_stamp()<<" $(MASTER)_b_transport: READ  addr="<<hex<<showbase<< trans.get_address() 
                     <<" data[3:0]="<< *(unsigned int *)trans.get_data_ptr() <<" size="<<dec<< trans.get_data_length() <<endl;
    #endif

    trans.set_response_status(TLM_OK_RESPONSE);
  } 
  
  tlm::tlm_sync_enum $(CLASS_NAME)::$(MASTER)_nb_transport_fw(tlm::tlm_generic_payload& trans, tlm::tlm_phase& phase, sc_core::sc_time& delay)
  {
    cout <<name()<<" @ "<<sc_time_stamp()<<" WARNING: Hello from $(MASTER)_nb_transport_fw() is NOT yet implemented...\n";
    #if (defined VIVY_DEBUG || 0)
    if (trans.is_write())
        cout <<name()<<" @ "<<sc_time_stamp()<<" $(MASTER)_nb_transport_fw: WRITE addr="<<hex<<showbase<< trans.get_address() 
                     <<" data[3:0]="<< *(unsigned int *)trans.get_data_ptr() <<" size="<<dec<< trans.get_data_length() <<endl;
    else
        cout <<name()<<" @ "<<sc_time_stamp()<<" $(MASTER)_nb_transport_fw: READ  addr="<<hex<<showbase<< trans.get_address() 
                     <<" data[3:0]="<< *(unsigned int *)trans.get_data_ptr() <<" size="<<dec<< trans.get_data_length() <<endl;
    #endif
    trans.set_response_status(TLM_COMMAND_ERROR_RESPONSE);
    return(TLM_COMPLETED);
  } /*>*/ /*is_exist(SIGNALS_OUT){*/

  /////////////////////////////////////////////////
  // Callbacks from UVM Connect to Vista signal out
  ///////////////////////////////////////////////// /*}*/ /*<*/
  tlm::tlm_sync_enum $(CLASS_NAME)::$(SIGNALS_OUT)_nb_transport_fw(tlm::tlm_generic_payload& trans, tlm::tlm_phase& phase, sc_core::sc_time& delay)
  {
    unsigned long long SigOutData = 0;
    unsigned           size       = trans.get_data_length();

    if (size>sizeof(SigOutData)) {
        cout <<name()<<" @ "<<sc_time_stamp()<<" ERROR $(SIGNALS_OUT)_nb_transport_fw(): incoming data size truncated to 64 bits \n";
        size = sizeof(SigOutData);
    } 
    memcpy(&SigOutData, trans.get_data_ptr(), size);
    #if (defined VIVY_DEBUG || 0)
        cout <<name()<<" @ "<<sc_time_stamp()<<" $(SIGNALS_OUT)_nb_transport_fw: WRITE data="<< SigOutData <<endl;
    #endif

    // use of XlTLMConduit requires non-blocking return, use tlm_fifo and SC_THREAD to accomplish this.
    assert ($(SIGNALS_OUT)_fifo.nb_put(SigOutData) == true);

    phase = END_RESP;
    trans.set_response_status(TLM_OK_RESPONSE);
    return(TLM_COMPLETED);
  } 
  //
  // Thread to drive $(SIGNALS_OUT) 
  void $(CLASS_NAME)::thread_$(SIGNALS_OUT)() 
  {
    unsigned long long sigOutData;
    while (1) {
      sigOutData = $(SIGNALS_OUT)_fifo.get();
      #if (defined VIVY_DEBUG || 0)
        cout <<name()<<" @ "<<sc_time_stamp()<<" thread_$(SIGNALS_OUT): WRITE data="<< sigOutData <<endl;
      #endif
      $(SIGNALS_OUT).write(sigOutData);
    }
  } 
  // optional b_transport for $(SIGNALS_OUT) 
  void $(CLASS_NAME)::$(SIGNALS_OUT)_b_transport(tlm::tlm_generic_payload& trans, sc_core::sc_time& delay) 
  {
    unsigned long long data = 0;
    unsigned           size = trans.get_data_length();
    if (size>sizeof(data)) {
        cout <<name()<<" @ "<<sc_time_stamp()<<" ERROR $(SIGNALS_OUT)_b_transport(): incoming data size truncated to 64 bits \n";
        size = sizeof(data);
    } 
    memcpy(&data, trans.get_data_ptr(), size);
    #if (defined VIVY_DEBUG || 0)
        cout <<name()<<" @ "<<sc_time_stamp()<<" $(SIGNALS_OUT)_b_transport: WRITE data="<< data <<endl;
    #endif
    $(SIGNALS_OUT).write(data);
  } /*>*/ 

/*is_exist(REG_WITH_CALLBACK_ON_READ_LIST){*/
/////////////////////////////////////////////////////////////////////////////////
// Use these functions to define the behavior of your model when there is a 
// read event on one of the registers as defined in the Model Builder form.
// These functions are called before the read callbacks on the port.
//////////////////////////////////////////////////////////////////////////////////*}*/ /*<*/ 

// Read callback for $(REG_WITH_CALLBACK_ON_READ_LIST) register.
// The value that this function returns, will be returned to the initiator port that requested its value.
$(REG_WITH_CALLBACK_ON_READ_DATA_TYPE_LIST) $(CLASS_NAME)::cb_read_$(REG_WITH_CALLBACK_ON_READ_LIST)() {
  
  return $(REG_WITH_CALLBACK_ON_READ_LIST);
}
/*>*/ /*is_exist(REG_WITH_CALLBACK_ON_WRITE_LIST){*/ 

/////////////////////////////////////////////////////////////////////////////////
// Use these functions to define the behavior of your model when there is a 
// write event on one of the registers as defined in the Model Builder form.
// These functions are called before the write callbacks on the port.
//////////////////////////////////////////////////////////////////////////////////*}*/ /*<*/ 

// Write callback for $(REG_WITH_CALLBACK_ON_WRITE_LIST) register.
// The newValue has been already assigned to the $(REG_WITH_CALLBACK_ON_WRITE_LIST) register.
void $(CLASS_NAME)::cb_write_$(REG_WITH_CALLBACK_ON_WRITE_LIST)($(REG_WITH_CALLBACK_ON_WRITE_DATA_TYPE_LIST) newValue) {
  
}
/*>*/ /*is_exist(IO_RANGE_LIST){*/ 

/////////////////////////////////////////////////////////////////////////////////
// Use these functions to define the behavior of your model when there is a  
// write/read event on one of memory io ranges as defined in the Model Builder form.
//////////////////////////////////////////////////////////////////////////////////*}*/ /*<*/ 

// Write callback for $(IO_RANGE_LIST) memory range.
void $(CLASS_NAME)::cb_write_$(IO_RANGE_LIST)(uint64_t address, unsigned char* data, unsigned length) {

}

// Read callback for $(IO_RANGE_LIST) memory range.
void $(CLASS_NAME)::cb_read_$(IO_RANGE_LIST)(uint64_t address, unsigned char* data, unsigned length) {

} /*>*/ /*is_exist(TARGET_WITH_CALLBACK){*/ 

/////////////////////////////////////////////////////
// Read Debug callback for $(TARGET_WITH_CALLBACK) 
///////////////////////////////////////////////////// /*}*//*<*/
unsigned $(CLASS_NAME)::$(TARGET_WITH_CALLBACK)_callback_read_dbg(config::uint64 address, unsigned char* data, unsigned size/*<*/, long $($(TARGET_WITH_CALLBACK)#1_PV_PARAMETERS)/*>*/) 
{ 
    // Call nb_transport_fw in UVM-Connect to mimic SystemC TLM Debug call.
    // SystemVerilog implementation of nb_transport_fw should terminate it early by specifying a return phase of "END_RESP"
    tlm::tlm_generic_payload trans;
    tlm::tlm_phase           phase = BEGIN_REQ;
    sc_time                  delay = SC_ZERO_TIME;

    trans.set_read();
    trans.set_address(address);
    trans.set_data_ptr(data);
    trans.set_data_length(size);
    trans.set_streaming_width(size);

    #ifdef ENABLE_$(TARGET_WITH_CALLBACK)_DEBUG_SOCKET 
      (*$(TARGET_WITH_CALLBACK)_debug_socket_to_uvmc)->nb_transport_fw(trans, phase, delay);

      #if (defined VIVY_DEBUG || 0)
        cout <<name()<<" @ "<<sc_time_stamp()<<" $(TARGET_WITH_CALLBACK)_callback_READ_dgb addr="
             <<hex<<showbase<< address <<" size="<<dec<< trans.get_data_length() <<" data= 0x";
        for (int i=trans.get_data_length()-1; i>=0; i--)
            cout <<hex<<noshowbase<< (unsigned)trans.get_data_ptr()[i];
        cout <<endl;
      #endif

      return trans.get_data_length();
    #else
      cout <<name()<<" @ "<<sc_time_stamp()<<" ERROR: VIVY $(TARGET_WITH_CALLBACK)_callback_READ_dbg() not enabled in SC_MODULE= '$(CLASS_NAME)'"
           <<" use '#define ENABLE_$(TARGET_WITH_CALLBACK)_DEBUG_SOCKET'"
           <<" addr="<<hex<<showbase<< address <<endl;
      return 0; 
    #endif 
}/*>*/ /*is_exist(TARGET_WITH_CALLBACK){*/

/////////////////////////////////////////////////////
// Write Debug callback for $(TARGET_WITH_CALLBACK) 
///////////////////////////////////////////////////// /*}*//*<*/
unsigned $(CLASS_NAME)::$(TARGET_WITH_CALLBACK)_callback_write_dbg(config::uint64 address, unsigned char* data, unsigned size/*<*/, long $($(TARGET_WITH_CALLBACK)#1_PV_PARAMETERS)/*>*/) 
{ 
    // Call nb_transport_fw in UVM-Connect to mimic SystemC TLM Debug call.
    // SystemVerilog implementation of nb_transport_fw MUST terminate it early by specifying a return phase of "END_RESP"
    tlm::tlm_generic_payload trans;
    tlm::tlm_phase           phase = BEGIN_REQ;
    sc_time                  delay = SC_ZERO_TIME;

    trans.set_write();
    trans.set_address(address);
    trans.set_data_ptr(data);
    trans.set_data_length(size);
    trans.set_streaming_width(size);

    #ifdef ENABLE_$(TARGET_WITH_CALLBACK)_DEBUG_SOCKET 

      #if (defined VIVY_DEBUG || 0)
        cout <<name()<<" @ "<<sc_time_stamp()<<" $(TARGET_WITH_CALLBACK)_callback_WRITE_dgb addr="
             <<hex<<showbase<< address <<" size="<<dec<< trans.get_data_length() <<" data= 0x";
        for (int i=trans.get_data_length()-1; i>=0; i--)
            cout <<hex<<noshowbase<< (unsigned)trans.get_data_ptr()[i];
        cout <<endl;
      #endif
      (*$(TARGET_WITH_CALLBACK)_debug_socket_to_uvmc)->nb_transport_fw(trans, phase, delay);

      return trans.get_data_length();
    #else
      cout <<name()<<" @ "<<sc_time_stamp()<<" ERROR: VIVY $(TARGET_WITH_CALLBACK)_callback_WRITE_dbg() not enabled in SC_MODULE= '$(CLASS_NAME)'"
           <<" use '#define ENABLE_$(TARGET_WITH_CALLBACK)_DEBUG_SOCKET'"
           <<" addr="<<hex<<showbase<< address <<endl;
      return 0; 
    #endif 
}/*>*/ /*<*/

bool $(CLASS_NAME)::$(TARGET_WITH_CALLBACK)_get_direct_memory_ptr(config::uint64 address, tlm::tlm_dmi& dmiData) 
  { return false; }
/*>*/

