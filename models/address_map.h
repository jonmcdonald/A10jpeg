#ifndef __address_map_h__
#define __address_map_h__

#define mem_addr 0x0

#define jpeg_addr (4*3*MAX_ROWS*MAX_COLUMNS + 3*MAX_ROWS*MAX_COLUMNS)

#define jpeg_inputaddr_addr    (jpeg_addr + 0x0)
#define jpeg_inputlength_addr  (jpeg_addr + 0x4)
#define jpeg_outputaddr_addr   (jpeg_addr + 0x8)
#define jpeg_outputlength_addr (jpeg_addr + 0xC)
#define jpeg_status_addr       (jpeg_addr + 0x10)
#define jpeg_start_addr        (jpeg_addr + 0x14)

#endif
