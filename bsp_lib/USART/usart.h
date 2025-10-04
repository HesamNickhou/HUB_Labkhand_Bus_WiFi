#ifndef __USART_H
#define __USART_H
#include "sys.h"
#include "stdio.h"	 

/// Size of the receive buffer used by the PDC, in bytes.
#define BUFFER_SIZE         1024

/// Number of bytes received between two timer ticks.
extern volatile unsigned int rxd_counter;
extern volatile unsigned int rxd_wr_index;
extern volatile unsigned int rxd_rd_index;

extern unsigned int CheckSum;

/// Receive buffer.
extern unsigned char RXDBuffer[];

extern unsigned char Connection;


void uart1_init(u32 bound);
void uart2_init(u32 bound);
void uart3_init(u32 bound);

#endif	   
















