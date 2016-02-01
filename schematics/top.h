#pragma once
#include "mgc_vista_schematics.h"
$includes_begin;
#include <systemc.h>
#include "../models/driver_model.h"
#include "../models/axi_bus_model.h"
#include "../models/JPEG_ENCODER_model.h"
#include "../models/sram_model.h"
$includes_end;

$module_begin("top");
SC_MODULE(top) {
public:
  typedef top SC_CURRENT_USER_MODULE;
  top(::sc_core::sc_module_name name):
    ::sc_core::sc_module(name)
    $initialization_begin
$init("mem"),
mem(0)
$end
$init("jpeg"),
jpeg(0)
$end
$init("driver"),
driver(0)
$end
$init("bus"),
bus(0)
$end
    $initialization_end
{
    $elaboration_begin;
$create_component("mem");
mem = new sram_pvt("mem");
$end;
$create_component("jpeg");
jpeg = new JPEG_ENCODER_pvt("jpeg");
$end;
$create_component("driver");
driver = new driver_pvt("driver");
$end;
$create_component("bus");
bus = new axi_bus_pvt("bus");
$end;
$bind("jpeg->irq","driver->irq");
vista_bind(jpeg->irq, driver->irq);
$end;
$bind("bus->sram_master","mem->slave");
vista_bind(bus->sram_master, mem->slave);
$end;
$bind("driver->master","bus->driver");
vista_bind(driver->master, bus->driver);
$end;
$bind("bus->jpeg_regs","jpeg->slave");
vista_bind(bus->jpeg_regs, jpeg->slave);
$end;
$bind("jpeg->master","bus->jpeg_dma");
vista_bind(jpeg->master, bus->jpeg_dma);
$end;
    $elaboration_end;
  $vlnv_assign_begin;
m_library = "Schematics";
m_vendor = "";
m_version = "";
  $vlnv_assign_end;
  }
  ~top() {
    $destructor_begin;
$destruct_component("mem");
delete mem; mem = 0;
$end;
$destruct_component("jpeg");
delete jpeg; jpeg = 0;
$end;
$destruct_component("driver");
delete driver; driver = 0;
$end;
$destruct_component("bus");
delete bus; bus = 0;
$end;
    $destructor_end;
  }
public:
  $fields_begin;
$component("mem");
sram_pvt *mem;
$end;
$component("jpeg");
JPEG_ENCODER_pvt *jpeg;
$end;
$component("driver");
driver_pvt *driver;
$end;
$component("bus");
axi_bus_pvt *bus;
$end;
  $fields_end;
  $vlnv_decl_begin;
public:
const char* m_library;
const char* m_vendor;
const char* m_version;
  $vlnv_decl_end;
};
$module_end;
