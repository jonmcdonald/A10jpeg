
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

//*<
//* Generated By Model Builder, Mentor Graphics Computer Systems, Inc.
//*
//* This file contains the PV class for driver.
//* This is a template file: You may modify this file to implement the 
//* behavior of your component. 
//* 
//* Model Builder version: 4.1.0
//* Generated on: Jan. 27, 2016 03:09:19 PM, (user: mbradley)
//*>

const unsigned JPEG_BASE    = 0xE0000000;
const unsigned JPEG_SRC     = JPEG_BASE+0x0;
const unsigned JPEG_W_H     = JPEG_BASE+0x4; // width(rows) << 16 | height(cols)
const unsigned JPEG_DEST    = JPEG_BASE+0x8;
const unsigned JPEG_OUT_LEN = JPEG_BASE+0xC;
const unsigned JPEG_START   = JPEG_BASE+0x14; // write 1 to start,  0 to clear irq

const unsigned BMP_MEM      = 0x10000000;
const unsigned JPEG_MEM     = 0x10010000;

#include "driver_pv.h"
#include <iostream>

using namespace sc_core;
using namespace sc_dt;
using namespace std;

//constructor
driver_pv::driver_pv(sc_module_name module_name) 
  : driver_pv_base(module_name) 
{
    SC_THREAD(thread);
    fp = fopen("driver_out.jpg", "wb");
}      

void driver_pv::thread()
{
    unsigned* rData = new unsigned[0x8000];
    int error = 0;
    unsigned outLength;
    unsigned width, height, w_h;

    cout <<name()<<" @ "<<sc_time_stamp()<<" Copy bmp data to memory"<<endl;
    master_write(BMP_MEM, (char *)bmp_data, sizeof(bmp_data) ); 
    master_read(BMP_MEM, (char *)rData, sizeof(bmp_data) ); 

    cout <<name()<<" @ "<<sc_time_stamp()<<" Test bmp data in memory"<<endl;
    for (unsigned i=0; i<sizeof(bmp_data)/4; i++) {
        if (rData[i] != bmp_data[i]) {
            error++;
            cout <<" ERROR: bmp_data check mismatch "<<hex<< rData[i] <<" != "<< bmp_data[i] <<endl;
            if (i>20) {
                cout <<" .... truncated subsequent errors";
                break;
            }
        }
        //else
        //    printf(",0x%08X\n",rData[i]);

    }

    wait(20*clock);
    
    // init jpeg
    master_write(JPEG_SRC , BMP_MEM);
    master_write(JPEG_DEST, JPEG_MEM);
  
    // for fractal_96_216.bmp
    width = 96;    // 14 bits, left side
    height = 216;  // 13 bits, right side
    w_h = ((width & 0x3FFF) << 13) | (height & 0x1FFF);

    master_write(JPEG_W_H, w_h); 
    master_write(JPEG_START, 0x1); 

    wait(gotIrqEvent);

    // acknowlege irq
    master_write(JPEG_START, 0x0);
   
    wait(5*clock);
    // check irq cleared
    if (irq.read() != 0) {
            error++;
            cout <<" ERROR: irq not acknowledged"<<endl;
    } else
            cout <<" irq succussfully acknowledged"<<endl;

    // check results
    master_read(JPEG_OUT_LEN, outLength); 
    cout <<" Reading JPEG results outLength= "<<dec<< outLength <<endl;
    master_read(JPEG_MEM, (char *)rData, outLength); 

    for (unsigned i=0; i<outLength/4; i++) 
    {
        fwrite(&(rData[i]), 4, 1, fp);
        //printf(" %02X",rData[i]);
#if 0
        if (rData[i] != jpeg_result[i]) {
            error++;
            cout <<" ERROR: jpeg result check mismatch ("<<dec<< i <<")"<<hex<< rData[i] <<" != "<< jpeg_result[i] <<endl;
            if (i>20) {
                cout <<" .... truncated subsequent errors";
                break;
            }
        }
#endif
    }


    if (error == 0)
        cout <<" TEST PASSED !! \n";
    else
        cout <<" TEST FAILED :-( \n";

}
 

// callback for any change in signal: irq of type: sc_in<bool>
void driver_pv::irq_callback() {

    cout <<name()<<" @ "<<sc_time_stamp()<<" Got irq "<< irq.read() <<endl;

    if (irq.read() == 1)
        gotIrqEvent.notify();
}