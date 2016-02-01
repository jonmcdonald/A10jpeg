/*----------------------------------------------------------------------+
|                                                                       |
|                                                                       |
|                             CALYPTO(tm)                               |
|                                                                       |
|                       PowerPro(tm) Software                           |
|                                                                       |
|                                                                       |
| This file contains confidential, trade secret information of          |
| Calypto Design Systems, Inc.  Any reproduction, use or disclosure     |
| of this file, without the express, prior written consent              |
| of Calypto Design Systems, Inc. is strictly prohibited.               |
|                                                                       |
|       Copyright (c)  Calypto Design Systems, Inc.  2002 - 2010.       |
|                         All rights reserved.                          |
|                                                                       |
| Calypto acknowledges trademarks or registered trademarks of other     |
| organizations for their respective products and services.             |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
//=========================================================================================
// SINGLE PORT SYNCHRONOUS MEMORY
//  -- Models cycle accurate behaviour
//  -- No memory hazard / coruption  is modeled  
//  -- Models Advanced Power Modes (Light Sleep, Deep Sleep, Shutdown) 
//  -- Models bit-level write bitmask (if mask bit is 1 then write is not done on that bit)
//  -- Models write-through behavior
//==========================================================================================

module calypto_mem_1p (q, clk, me, rw, wm, wadr, radr, d, ls , ds, sd);
parameter AW = 5;
parameter DW = 8;
parameter NW = 1<<AW;
parameter WT = 0;  // set to 1 for synchronous write through feature
parameter ds_sd_zero = 1;
parameter UHD_arch = 0;
parameter READ_BEFORE_WRITE = 0;
parameter LS_DEPTH = 1;
parameter TCSEP = 0.001;
parameter WC_CORRUPT_OVERLAP_BITS = 0;
parameter RC_CORRUPT_ALL_BITS = 0;
parameter RC_CORRUPT_WRITE = 0;

   
output [DW-1:0] q; //read data

input clk; // clock positive edge triggered
input me;  // memory is off when me == 0
input rw;  // write happens if  1 else read happens
input [AW-1:0] wadr; // write address 
input [AW-1:0] radr; // read  address 
input [DW-1:0] d; //write data
input [DW-1:0] wm;  // write mask. if write mode and wm[bit]==1, then that bit is not written
input ls;   // lightsleep
input ds;   // deep sleep
input sd;   // shut down


wire [DW-1:0] din;
reg [DW-1:0] rd_ff;
reg [DW-1:0] mem_core_array [0:(1<<AW)-1];
reg      ls_int;
   

`ifndef CALYPTO_SYNTH
integer i;
always @(posedge sd )
begin
  for ( i = 0 ; i < NW ; i = i + 1 )
  begin
    mem_core_array[i] = {DW{1'bX}};
  end
end
   always @(ls)
     ls_int = ls;
`else

(* ls_regs *)   reg [LS_DEPTH-1:0] ls_regs;
integer i;

   generate
      if ( LS_DEPTH < 2)
	begin
	   always @ ( posedge clk)
             ls_regs = ls;
        end
      else
	begin
	   always @ ( posedge clk)
             ls_regs = {ls_regs[LS_DEPTH-2:0], ls};
        end
   endgenerate

   always @(*)
     begin
	ls_int = ls;
	for (i=0; i<LS_DEPTH; i = i + 1)
	  begin
	     ls_int = ls_int | ls_regs[i];
	  end
     end
   
`endif

wire sel_pwr = sd | ds;
wire cs_int = me & !ls_int & !ds & !sd;
wire wen = cs_int & rw  ;
wire ren = cs_int & ~rw;

assign q = (sel_pwr) ? ((ds_sd_zero) ? {DW{1'b0}} : {DW{1'bX}}) : rd_ff;

assign din = (d & ~wm) | ( mem_core_array[wadr] & wm);

always @(posedge clk ) begin
   if (wen ) 
      mem_core_array[wadr] = din;
   if (sel_pwr ) 
     rd_ff = ds_sd_zero ? {DW{1'b0}} : {DW{1'bX}};
   else if (WT ? (wen | ren) : ren)
     rd_ff = mem_core_array[radr];
end


endmodule

