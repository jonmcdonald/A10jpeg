#ifndef __UART_2_TCP_h
#define __UART_2_TCP_h

#include "model_builder.h"
//#include "systemc"

class uart_2_tcp {
public:
  uart_2_tcp(unsigned short uart_2_tcp_port);
  ~uart_2_tcp();
public:
  int write(const unsigned char* buf, int buf_len);
  int read(unsigned char* buf, int buf_len);
  int b_read(unsigned char* buf, int buf_len);
  int read_status();
private:
  void fatal_sys_error(const char*);
  void start_connection();
  void finish_connection();
private:
  int socket_fd;
  int tcp_fd;
  sc_core::sc_event socket_fd_event;
  sc_core::sc_event tcp_fd_event;
};

#endif
