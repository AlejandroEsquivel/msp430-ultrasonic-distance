#include "msp430.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define TRIG_PIN BIT1 // Corresponds to P2.1
#define ECHO_PIN BIT1 // Corresponds to P1.1

#define TXD BIT2 // TXD on P1.2

volatile unsigned long start_time;
volatile unsigned long end_time;
volatile unsigned long delta_time;
volatile unsigned long distance;
volatile unsigned int ignore_measurement = 0;

void wait_ms(unsigned int ms)
{
  unsigned int i;
  for (i = 0; i <= ms; i++)
  {
    __delay_cycles(1000); // Clock is ~1MHz so 1E3/1E6 = 1E-3 (1ms) seconds
  }
}

/* Write byte to USB-Serial interface */
void write_uart_byte(char value)
{
  while (!(IFG2 & UCA0TXIFG));
  // wait for TX buffer to be ready for new data
  // UCA0TXIFG register will be truthy when available to recieve new data to computer.
  UCA0TXBUF = value;
}

void write_uart_string(char *str)
{
  unsigned int i = 0;
  while (str[i] != '\0')
  {
    //write string byte by byte
    write_uart_byte(str[i++]);
  }
}

void write_uart_long(unsigned long l)
{
  char buf[sizeof(l) * 8 + 1];
  sprintf(buf, "%ld\n", l);
  write_uart_string(buf);
}

#if defined(__TI_COMPILER_VERSION__)
#pragma vector = TIMER0_A0_VECTOR
__interrupt void ta1_isr(void)
#else
void __attribute__((interrupt(TIMER0_A0_VECTOR))) ta1_isr(void)
#endif
{
  switch (TAIV)
  {
  //Timer overflow
  case 10:
  break;
  //Otherwise Capture Interrupt
  default:
    // Read the CCI bit (ECHO signal) in CCTL0
    // If ECHO is HIGH then start counting (rising edge)
    if (CCTL0 & CCI)
    {
      start_time = CCR0;
    } // If ECHO is LOW then stop counting (falling edge)
    else
    {
      end_time = CCR0;
      delta_time = end_time - start_time;
      distance = (unsigned long)(delta_time / 0.00583090379);

      //only accept values within HC-SR04 acceptible measure ranges
      if (ignore_measurement == 0 && distance / 10000 >= 2.0 && distance / 10000 <= 400)
      {
        write_uart_long(distance);
      }
      else if (ignore_measurement == 1)
      {
        //clear ignore measurement flag
        ignore_measurement = 0;
      }
      else if (distance > 400)
      {
        //Ignore next measurement if ECHO signal timed out 
        //as the delayed ultrasonic pulse might come during next measurement.
        //- this should be removed if the sensor is not in an environment that
        //- will mostly provide measurements within the range [2,400] cm.
        //- also, should be removed if wait interval (500ms) is reduced.
        ignore_measurement = 1;
      }
    }
    break;
  }
  TACTL &= ~CCIFG; // reset the interrupt flag
}

/* Setup TRIGGER and ECHO pins */
void init_ultrasonic_pins(void)
{
  P1DIR &= ~ECHO_PIN; // Set ECHO (P1.1) pin as INPUT
  P1SEL |= ECHO_PIN;  // Set P1.1 as CCI0A (Capture Input signal).
  P2DIR |= TRIG_PIN;  // Set TRIGGER (P2.1) pin as OUTPUT
  P2OUT &= ~TRIG_PIN; // Set TRIGGER (P2.1) pin to LOW
}

/* Setup UART */
void init_uart(void)
{

  // Set P1.2 as TXD
  P1DIR |= TXD;
  P1OUT |= TXD;
  P1SEL |= TXD;
  P1SEL2 |= TXD;

  UCA0CTL1 |= UCSSEL_2; // Use SMCLK - 1MHz clock
  UCA0BR0 = 104;        // Set baud rate to 9600 with 1MHz clock (Data Sheet 15.3.13)
  UCA0BR1 = 0;          // Set baud rate to 9600 with 1MHz clock
  UCA0MCTL = UCBRS0;    // Modulation UCBRSx = 1
  UCA0CTL1 &= ~UCSWRST; // Initialize USCI state machine - enable
}

void init_timer(void)
{
  TACTL = MC_0; // Stop timer before modifiying configuration
  /* 
  1. Capture Rising & Falling edge of ECHO signal
  2. Sync with clock
  3. Set Capture Input signal to CCI0A
  4. Enable capture mode
  5. Enable interrupt
  */
  CCTL0 |= CM_3 + SCS + CCIS_0 + CAP + CCIE;
  // Select SMCLK with no divisions, continous mode.
  TACTL |= TASSEL_2 + MC_2 + ID_0;
}

void reset_timer(void)
{
  TACTL |= TACLR;
}

void main(void)
{

  WDTCTL = WDTPW + WDTHOLD; // Stop Watch Dog Timer

  /* Use internal calibrated 1MHz clock: */
  BCSCTL1 = CALBC1_1MHZ; // Set range
  DCOCTL = CALDCO_1MHZ;
  BCSCTL2 &= ~(DIVS_3); // SMCLK = DCO = 1MHz

  init_ultrasonic_pins();
  init_uart();
  init_timer();

  // Global Interrupt Enable
  __enable_interrupt();

  while (1)
  {
    // send ultrasonic pulse
    reset_timer();
    P2OUT |= TRIG_PIN;  // Enable TRIGGER
    __delay_cycles(10); // Send pulse for 10us
    P2OUT &= ~TRIG_PIN; // Disable TRIGGER
    wait_ms(500); // wait 500ms until next measurement
  }
}