#include  "msp430.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define TRIG_PIN			BIT1 // Corresponds to P2.1
#define ECHO_PIN			BIT0 // Corresponds to P2.0

#define TXD           BIT2 // TXD on P1.2EnableCapture
#define RXD           BIT1 // RXD on P1.1

unsigned int timer_reset = 0;


void write_uart_byte(char value){
  while (! (IFG2 & UCA0TXIFG)); // wait for TX buffer to be ready for new data
  // UCA0TXIFG register will be truthy when available to recieve new data to computer.
  UCA0TXBUF = value;
}
 
void write_uart_string(char *str) {
    unsigned int i = 0;
    while(str[i] != '\0'){
        write_uart_byte(str[i++]);
    }
}

void write_uart_long(unsigned long l) {
    char buf[sizeof(l)*8 +1];
    sprintf(buf,"%ld\n",l);
    write_uart_string(buf);
}

/* If Timer counts to zero before end_time recorded */
#if defined(__TI_COMPILER_VERSION__)
#pragma vector=TIMER0_A1_VECTOR
__interrupt void ta1_isr (void)
#else
  void __attribute__ ((interrupt(TIMER0_A1_VECTOR))) ta1_isr (void)
#endif
{
  timer_reset++;
  TACCTL1 &= ~CCIFG; // reset the interrupt flag
}


void wait_ms(unsigned int ms)
{
    unsigned int i;
    for (i = 0; i<= ms; i++){
      __delay_cycles(1000); // Clock is ~1MHz so 1E3/1E6 = 1E-3 (1ms) seconds
    }
}


/* Setup TRIGGER and ECHO pins */
void init_ultrasonic_pins(void){
	P2DIR &= ~ECHO_PIN;	// Set ECHO (P2.0) pin as INPUT
  P2DIR |= TRIG_PIN;  // Set TRIGGER (P2.1) pin as OUTPUT
	P2OUT &= ~TRIG_PIN;	// Set TRIGGER (P2.1) pin to LOW
}

/* Setup UART */
void init_uart(void){

  P1DIR |= TXD;
  P1OUT |= TXD;

  /* P1.1 = RXD, P1.2=TXD and special function enable */
  P1SEL = RXD + TXD; 
  P1SEL2 = RXD + TXD; 

  UCA0CTL1 |= UCSSEL_2; // Use SMCLK - 1MHz clock
  UCA0BR0 = 104; // Set baud rate to 9600 with 1MHz clock (Data Sheet 15.3.13)
  UCA0BR1 = 0; // Set baud rate to 9600 with 1MHz clock
  UCA0MCTL = UCBRS0; // Modulation UCBRSx = 1 
  UCA0CTL1 &= ~UCSWRST; // Initialize USCI state machine - enable 
}

void init_timer(void){
  TACCTL1 = CCIE + OUTMOD_3;     // TACCTL1 Capture Compare
  TACTL |= TASSEL_2 + MC_2 + ID_0;
}

void main(void){

	WDTCTL = WDTPW + WDTHOLD;		// Stop Watch Dog Timer

  /* Use internal calibrated 1MHz clock: */
  BCSCTL1 = CALBC1_1MHZ; // Set range
  DCOCTL = CALDCO_1MHZ;
  BCSCTL2 &= ~(DIVS_3); // SMCLK = DCO = 1MHz  

  init_ultrasonic_pins();
  init_uart();
  init_timer();

	// Global Interrupt Enable
	__enable_interrupt();

  unsigned int prev_echo_val = 0;
  unsigned int curr_echo_val;
  unsigned int cont = 1;
  unsigned long start_time;
  unsigned long end_time;
  unsigned long distance;

  char buffer[32*5 + 20];

	while (1)
	{
    //write_uart(TAR);
		P2OUT |= TRIG_PIN; // Enable TRIGGER
		__delay_cycles(10); // Send pulse for 10us
		P2OUT &= ~TRIG_PIN; // Disable TRIGGER

    cont = 1;
    while(cont){
      curr_echo_val = P2IN & ECHO_PIN;
      // Rising edge
      if(curr_echo_val > prev_echo_val){
        timer_reset = 0;
        start_time = TAR;
      } 
      // Falling edge
      else if(curr_echo_val < prev_echo_val){
        end_time = TAR;
        end_time+= timer_reset*0xFFFF;
        distance = (unsigned long)((end_time - start_time)/0.00583090379);
        //only accept values within HC-SR04 acceptible measure ranges
        if(distance/1000 >= 2.0 && distance/1000 <= 400){
          write_uart_long(distance);
        }
        
        cont = 0;
      }
      prev_echo_val = curr_echo_val;
    }

     //sprintf(buffer,"End (%ld), Start(%ld), Delta(%ld), timer_reset(%i)",end_time,start_time,end_time-start_time,timer_reset);
     //write_uart_string(buffer);

		wait_ms(500); // wait 0.5 second before repeating measurement

	}

}