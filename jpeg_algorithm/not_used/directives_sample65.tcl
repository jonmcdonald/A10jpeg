#TEST
solution new -state initial
solution options defaults

options set Output/OutputVHDL false
flow package require /SCVerify
#disable auto flush of testbench
solution options set /Flows/SCVerify/DISABLE_EMPTY_INPUTS true

solution file add ./dct.cpp -type C++
solution file add ./jpeg.cpp -type C++
solution file add ./huff.cpp -type C++
solution file add ./bmp.cpp -type C++ -exclude true
solution file add ./bitstream.cpp -type C++ -exclude true
solution file add ./main.cpp -type C++ -exclude true

#First solution runs with all blocks II=1 but interconnect FIFOs unconstrained
go analyze
solution library add mgc_sample-065nm-dw_beh_dc -- -rtlsyntool DesignCompiler -vendor Sample -technology 065nm -Designware Yes
solution library add ram_sample-065nm-singleport_beh_dc


directive set -DESIGN_HIERARCHY {pixelpipe dct dct_h dct_v {mult_add<ac_int<8, true>, ac_int<9, true>, ac_int<21, true>>} {mult_add<ac_int<16, true>, ac_int<17, true>, ac_int<31, true>>} convert quantize encode broadcast}
go compile
directive set -OPT_CONST_MULTS 0
go libraries
directive set -CLOCKS {clk {-CLOCK_PERIOD 3.3333 -CLOCK_EDGE rising -CLOCK_UNCERTAINTY 0.0 -CLOCK_HIGH_TIME 1.67 -RESET_SYNC_NAME rst -RESET_ASYNC_NAME arst_n -RESET_KIND sync -RESET_SYNC_ACTIVE high -RESET_ASYNC_ACTIVE low -ENABLE_ACTIVE high}}
directive set -CLOCK_NAME clk
directive set /pixelpipe/mult_add<ac_int<16,true>,ac_int<17,true>,ac_int<31,true>> -MAP_TO_MODULE {[CCORE]}
directive set /pixelpipe/mult_add<ac_int<16,true>,ac_int<17,true>,ac_int<31,true>> -CCORE_TYPE combinational
directive set /pixelpipe/mult_add<ac_int<8,true>,ac_int<9,true>,ac_int<21,true>> -MAP_TO_MODULE {[CCORE]}
directive set /pixelpipe/mult_add<ac_int<8,true>,ac_int<9,true>,ac_int<21,true>> -CCORE_TYPE combinational
go assembly
directive set /pixelpipe/dct_h/core/main -PIPELINE_INIT_INTERVAL 8
directive set /pixelpipe/dct_h/core/COPY_ROW0 -UNROLL yes
directive set /pixelpipe/dct_h/core/COL0 -UNROLL yes
directive set /pixelpipe/dct_v/core/main -PIPELINE_INIT_INTERVAL 8
directive set /pixelpipe/dct_v/core/COPY_ROW -UNROLL yes
directive set /pixelpipe/dct_v/core/ROW1 -UNROLL yes
directive set /pixelpipe/convert/core/main -PIPELINE_INIT_INTERVAL 1
directive set /pixelpipe/quantize/core/main -PIPELINE_INIT_INTERVAL 1
directive set /pixelpipe/encode/core/main -PIPELINE_INIT_INTERVAL 1
go extract

#Second solution upsizes FIFOs to find high-water mark
go assembly
directive set /pixelpipe/blocktype1:cns -FIFO_DEPTH 128
directive set /pixelpipe/blocktype2:cns -FIFO_DEPTH 128
directive set /pixelpipe/converted:cns -FIFO_DEPTH 128
directive set /pixelpipe/quantized:cns -FIFO_DEPTH 128
go architect
go extract

#Third solution sets FIFO size based on high-water mark
go assembly
directive set /pixelpipe/blocktype1:cns -FIFO_DEPTH 5
directive set /pixelpipe/blocktype2:cns -FIFO_DEPTH 2
directive set /pixelpipe/converted:cns -FIFO_DEPTH 2
directive set /pixelpipe/quantized:cns -FIFO_DEPTH 63
go architect
go extract

#Fourth solution enables pipeline flushing to allow testbench to finish
go assembly
directive set /PIPELINE_STALL_MODE flush
go architect
go extract