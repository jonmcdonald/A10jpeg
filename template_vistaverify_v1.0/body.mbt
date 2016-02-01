

add_generic NUM_BUS_SLAVES      1 -values {0 1 2 3 4 5 6 7 8 }
add_generic NUM_BUS_MASTERS     0 -values {0 1 2 3 4 5 6 7 8 }
add_generic NUM_SIGNAL_IN       0 
add_generic NUM_SIGNAL_OUT      0 
add_generic BUS_BITNESS         32 -values {8 16 32 64 128 256}
add_generic BUS_MASTER_PROTOCOL axi -values {generic axi ahb_master apb }
add_generic BUS_SLAVE_PROTOCOL  axi -values {generic axi ahb_slave apb }

add_generic SIGNAL_OUT_PREFIX   vivySigOut
add_generic SIGNAL_IN_PREFIX    vivySigIn
add_generic BUS_SLAVE_PREFIX    vivySlave
add_generic BUS_MASTER_PREFIX   vivyMaster

if { $NUM_BUS_SLAVES <= 0 && $NUM_BUS_MASTERS <= 0 && $NUM_SIGNAL_OUT <= 0 && $NUM_SIGNAL_IN <= 0} {
  error "Need at least one port"
}

;# Enable/Disable UVMC connection within Vivy component
add_parameter "vivy_disable" 0 uint32 {}

;# use ":" for vista in separate process
set colon ":"

;# slave ports
for { set index 0 } { $index < $NUM_BUS_SLAVES } { incr index } {
  if { $NUM_BUS_SLAVES == 1 } { 
      set postfix "" 
  } else { set postfix "$index" }
  add_port "${BUS_SLAVE_PREFIX}${postfix}" /TLM/protocols/$BUS_SLAVE_PROTOCOL slave $BUS_BITNESS -bind_mode mandatory
  ;# generics not updated for default string value of a parameter, leave blank for now
  ;#add_parameter "uvmc_${BUS_SLAVE_PREFIX}${postfix}" "${colon}uvmc_${BUS_SLAVE_PREFIX}${postfix}" string
  ;# Note: If user replaces port name in wizard, strings after ${BUS_SLAVE_PREFIX} are lost (hence uvmc_dbg must be in beginning)
  add_parameter "uvmc_${BUS_SLAVE_PREFIX}${postfix}" "" string
  add_parameter "uvmc_dbg_${BUS_SLAVE_PREFIX}${postfix}" "" string
  add_delay_policy ${BUS_SLAVE_PREFIX}${postfix} 0 -latency 0 -sync 1 -power 0
}

;# master ports
for { set index 0 } { $index < $NUM_BUS_MASTERS } { incr index } {
  if { $NUM_BUS_MASTERS == 1 } { 
      set postfix "" 
  } else { set postfix "$index" }
  add_port "${BUS_MASTER_PREFIX}${postfix}" /TLM/protocols/$BUS_MASTER_PROTOCOL master $BUS_BITNESS -bind_mode mandatory
  ;#add_parameter "uvmc_${BUS_MASTER_PREFIX}${postfix}" "${colon}uvmc_${BUS_MASTER_PREFIX}${postfix}" string
  add_parameter "uvmc_${BUS_MASTER_PREFIX}${postfix}" "" string
}

;# signals in
for { set index 0 } { $index < $NUM_SIGNAL_IN } { incr index } {
  if { $NUM_SIGNAL_IN == 1 } { 
      set postfix "" 
  } else { set postfix "$index" }
  add_port "${SIGNAL_IN_PREFIX}${postfix}" /TLM/protocols/signal slave 1 -bind_mode mandatory
  ;#add_parameter "uvmc_${SIGNAL_IN_PREFIX}${postfix}" "${colon}uvmc_${SIGNAL_IN_PREFIX}${postfix}" string
  add_parameter "uvmc_${SIGNAL_IN_PREFIX}${postfix}" "" string
}

;# signals out
for { set index 0 } { $index < $NUM_SIGNAL_OUT } { incr index } {
  if { $NUM_SIGNAL_OUT == 1 } { 
      set postfix "" 
  } else { set postfix "$index" }
  add_port "${SIGNAL_OUT_PREFIX}${postfix}" /TLM/protocols/signal master 1 -bind_mode mandatory
  ;#add_parameter "uvmc_${SIGNAL_OUT_PREFIX}${postfix}" "${colon}uvmc_${SIGNAL_OUT_PREFIX}${postfix}" string
  add_parameter "uvmc_${SIGNAL_OUT_PREFIX}${postfix}" "" string
}

