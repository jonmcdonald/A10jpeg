
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



#include "apb_bus_t.h"


using namespace std;
using namespace sc_dt;
using namespace sc_core;

apb_bus_t_base* apb_bus_t_base::create_t(const char* name, long simulation, 
                                                     esl::sc_sim::user_model::Arbiter* arbiter)
{
  return new apb_bus_t(name, simulation, arbiter);
}
