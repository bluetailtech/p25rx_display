/* This file was automatically generated.  Do not edit! */
#ifndef STD_IO__H
#define STD_IO__H

#include <stdarg.h>
#include <stdint.h>

#define MAX_PRINTF_BUFFER 1460*8
#define MAX_NET_BUFFER 1460*8

int printf( const char *format, ... );
int __io_putchar( int ch );
void printf_uart( char *net_buffer );
extern uint8_t printf_buf[MAX_PRINTF_BUFFER];
extern uint8_t net_buffer[MAX_NET_BUFFER];
extern UART_HandleTypeDef huart6;



#endif
