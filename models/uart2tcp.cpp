#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <iostream>

#include "uart2tcp.h"

uart_2_tcp::uart_2_tcp(unsigned short uart_2_tcp_port) :
  socket_fd(-1),
  tcp_fd(-1)
{
  socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_fd < 0)
    fatal_sys_error("socket");

  int optval = 1;
  if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1) {
    perror("setsockopt:SO_REUSEADDR");
  }

  struct sockaddr_in my_addr;
  memset (&my_addr, 0, sizeof(my_addr));
  my_addr.sin_family = AF_INET;
  my_addr.sin_addr.s_addr = htonl (INADDR_ANY);
  my_addr.sin_port = htons (uart_2_tcp_port);
  if (bind(socket_fd, (struct sockaddr *) &my_addr, sizeof (my_addr)) < 0)
    fatal_sys_error("bind");
  if (listen(socket_fd, 1) < 0)
    fatal_sys_error("listen");

  std::cout << "Listening on port " << uart_2_tcp_port << std::endl;
  mb::physical_io::helper::register_sc_event(socket_fd, socket_fd_event);
}

uart_2_tcp::~uart_2_tcp()
{
  finish_connection();
  close(tcp_fd);
  close(socket_fd);
  mb::physical_io::helper::unregister_sc_event(socket_fd);
}

int uart_2_tcp::write(const unsigned char* buf, int buf_len)
{
  if (tcp_fd < 0)
    return -1;
  int r;
  do
    r = ::write(tcp_fd, buf, buf_len);
  while (r < 0 && errno == EINTR);
  return r;
}

int uart_2_tcp::read_status()
{
  if (tcp_fd < 0)
    start_connection();
  if (tcp_fd < 0)
    return 0;
  return mb::physical_io::helper::get_io_status(tcp_fd);
}

int uart_2_tcp::read(unsigned char* buf, int buf_len)
{
  int r = read_status();
  if (r <= 0)
    return r;
  r = mb::physical_io::helper::nonblocking_read(tcp_fd, buf, buf_len);
  if (r < 0) {
    // socket closed. prepare to the new connection
    finish_connection();
    return 0;
  }
  return r;
}

int uart_2_tcp::b_read(unsigned char* buf, int buf_len)
{
  if (tcp_fd < 0) {
    wait(socket_fd_event);
    start_connection();
  }
  if (tcp_fd < 0)
    return -1;
  int r = mb::physical_io::helper::blocking_read(tcp_fd, buf, buf_len);
  if (r < 0) {
    // socket closed. prepare to the new connection
    finish_connection();
    return 0;
  }
  return r;
}

void uart_2_tcp::fatal_sys_error(const char*msg)
{
  perror(msg);
  exit(1);
}

void uart_2_tcp::start_connection()
{
  if (!mb::physical_io::helper::get_io_status(socket_fd))
    return;
  do
    tcp_fd = accept(socket_fd, NULL, 0);
  while (tcp_fd < 0 && errno == EINTR);

  if (tcp_fd < 0)
    fatal_sys_error("accept");

  //  setsockopt(socket_fd, SOL_SOCKET, SO_KEEPALIVE, (char*) &optval, sizeof(optval));

  int fd_flags = fcntl(tcp_fd, F_GETFD);
  fcntl(tcp_fd, F_SETFD, fd_flags | FD_CLOEXEC);

  mb::physical_io::helper::register_sc_event(tcp_fd, tcp_fd_event);
}

void uart_2_tcp::finish_connection()
{
  if (tcp_fd >= 0) {
    close(tcp_fd);
    mb::physical_io::helper::unregister_sc_event(tcp_fd);
    tcp_fd = -1;
  }
}
