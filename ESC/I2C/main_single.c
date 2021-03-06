/*****************************************************************************
*
* Atmel Corporation
*
* File              : main.c
* Compiler          : IAR EWAAVR 2.28a/3.10c
* Revision          : $Revision: 2516 $
* Date              : $Date: 2007-09-27 10:41:15 +0200 (to, 27 sep 2007) $
* Updated by        : $Author: mlarsson $
*
* Support mail      : avr@atmel.com
*
* Supported devices : All devices with a TWI module can be used.
*                     The example is written for the ATmega16
*
* AppNote           : AVR311 - TWI Slave Implementation
*
* Description       : Example of how to use the driver for TWI slave 
*                     communication.
*
****************************************************************************/
/*! \page MISRA
 *
 * General disabling of MISRA rules:
 * * (MISRA C rule 1) compiler is configured to allow extensions
 * * (MISRA C rule 111) bit fields shall only be defined to be of type unsigned int or signed int
 * * (MISRA C rule 37) bitwise operations shall not be performed on signed integer types
 * As it does not work well with 8bit architecture and/or IAR

 * Other disabled MISRA rules
 * * (MISRA C rule 109) use of union - overlapping storage shall not be used
 * * (MISRA C rule 61) every non-empty case clause in a switch statement shall be terminated with a break statement
*/

#include <ioavr.h>
#include <inavr.h>
#include "TWI_Slave.h"

// Sample TWI transmission commands
#define TWI_CMD_MASTER_WRITE 0x10
#define TWI_CMD_MASTER_READ  0x20

// When there has been an error, this function is run and takes care of it
unsigned char TrataErroTransI2C ( unsigned char TWIerrorMsg );


void main( void )
{
  unsigned char messageBuf[TWI_BUFFER_SIZE];
  unsigned char TWI_slaveAddress;
  
  // Feedback (opcional)
  /* DDRB  = 0xFF; // Set to output
     PORTB = 0x55; // Startup pattern */
  // Le os pinos 0-2 para endereçamento
  DDRC  = ~((1 << 0) | (1 << 1) | (1 << 2)); //Bits 0-2 como entrada
  
  // Own TWI slave address
  TWI_slaveAddress = 0x10 + 0b00000111 & PORTD;

  // Initialise TWI module for slave operation. Include address and/or enable General Call.
  TWI_Slave_Initialise( (unsigned char)((TWI_slaveAddress<<TWI_ADR_BITS) | (TRUE<<TWI_GEN_BIT) )); 
                       
  __enable_interrupt();

  // Start the TWI transceiver to enable reseption of the first command from the TWI Master.
  TWI_Start_Transceiver();

  // This example is made to work together with the AVR315 TWI Master application note. In adition to connecting the TWI
  // pins, also connect PORTB to the LEDS. The code reads a message as a TWI slave and acts according to if it is a 
  // general call, or an address call. If it is an address call, then the first byte is considered a command byte and
  // it then responds differently according to the commands.

  // This loop runs forever. If the TWI is busy the execution will just continue doing other operations.
  for(;;)
  {    
    
    // Check if the TWI Transceiver has completed an operation.
    if ( ! TWI_Transceiver_Busy() )                              
    {
      // Check if the last operation was successful
      if ( TWI_statusReg.lastTransOK )
      {
        // Confere se ha algo no buffer
        if ( TWI_statusReg.RxDataInBuf )
        {
          TWI_Get_Data_From_Transceiver(messageBuf, 2);         
          // Confere se o ultimo pedido foi um General Call
          if ( TWI_statusReg.genAddressCall )
          {
            // Trata o "broadcast"
            PORTB = messageBuf[0];
          }               
          else // Ends up here if the last operation was a reception as Slave Address Match   
          {
            // Example of how to interpret a command and respond.
            
            // TWI_CMD_MASTER_WRITE stores the data to PORTB
            if (messageBuf[0] == TWI_CMD_MASTER_WRITE)
            {
              PORTB = messageBuf[1];                            
            }
            // TWI_CMD_MASTER_READ prepares the data from PINB in the transceiver buffer for the TWI master to fetch.
            if (messageBuf[0] == TWI_CMD_MASTER_READ)
            {
              messageBuf[0] = PINB;
              TWI_Start_Transceiver_With_Data( messageBuf, 1 );
            }
          }
        }                
        // Apos a operacao ele reinicia o receptor
        if ( ! TWI_Transceiver_Busy() )
        {
          TWI_Start_Transceiver();
        }
      }
      else // Trata erro de transmissao
      {
        TrataErroTransI2C( TWI_Get_State_Info() );
      }
    }
  }
}


unsigned char TrataErroTransI2C ( unsigned char TWIerrorMsg )
{
// Se houver falha, reinicia a recepcao e acende LED
  TWI_Start_Transceiver();
  PD3 = 1;
  return TWIerrorMsg; 
}

/*  
  // A simplified example.
  // This will store data received on PORTB, and increment it before sending it back.

  TWI_Start_Transceiver( );    
         
  for(;;)
  {
    if ( ! TWI_Transceiver_Busy() )                              
    {
      if ( TWI_statusReg.RxDataInBuf )
      {
        TWI_Get_Data_From_Transceiver(&temp, 1);  
        PORTB = temp;
      }
      temp = PORTB + 1;
      TWI_Start_Transceiver_With_Data(&temp, 1); 
    }
    __no_operation();   // Do something else while waiting
  }
}

*/
