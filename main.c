#include <msp430.h>
#include "onewire.h"
#include "delay.h"

int i; //used throughout code for counting in loops
onewire_t ow; //object setup for temp sensor
uint8_t scratchpad[9]; //temp sensor saved as 8 bytes which translate from -50 to 150 degrees celcius

void owsetup(void) //sets up the one wire dataline for the temperature sensor
{
    ow.port_out = &P1OUT;
    ow.port_in = &P1IN;
    ow.port_ren = &P1REN;
    ow.port_dir = &P1DIR;
    ow.pin = BIT2;
}

void UART_Setup(void)
{               //initialize UART
    P4SEL |= BIT4 + BIT5;                  // P4.4,5 = USCI_A1 TXD/RXD
    UCA1CTL1 |= UCSWRST;                 // **Put state machine in reset**
    UCA1CTL1 |= UCSSEL_2;                // SMCLK
    UCA1BR0 = 6;                         // 1MHz 9600 (see User's Guide)
    UCA1BR1 = 0;                         // 1MHz (BR0 = 9 for 115200)
    UCA1MCTL |= UCBRS_1 + UCBRF_0;       // Modulation UCBRSx=1, UCBRFx=0
    UCA1CTL1 &= ~UCSWRST;                // **Initialize USCI state machine**
    UCA1IE = UCRXIE;                     // Enable USCI_A0 RX interrupt

    //BCSCTL1 = CALBC1_8MHZ; ONLY WORKS ON G2, DCO setup
    //DCOCTL = CALDCO_8MHZ;
}

void initGPIO(void)
{                    //Initializing the GPIO Pin 1.4 for input

    P1DIR &= ~BIT3 + ~BIT4;                 // Set P1.3 and P1.4 to input direction
    P1REN |= BIT3 + BIT4;                  // Enable P1.3 and P1.4 internal resistance
    P1SEL &= ~BIT3 + ~BIT4;                 // Set P1.3 and P1.4 to input
    P1IE |= BIT3 + BIT4;                   // P1.3 and P1.4 interrupt enabled
    P1IES &= ~BIT3 + ~BIT4;                 // P1.3 and P1.4 rising edge
    P1IFG &= ~BIT3 + ~BIT4;                 // P1.3 and P1.4 IFG cleared

    P2DIR |= BIT4;                                //set output (for LED lightup on fingerprint success
    P2SEL &= ~BIT4;
}

void subscribe(void) //sends initial subscription over ESP8266
{
    char team[] = { '$', 't', 'e', 'a', 'm', '2', '\n' };
    for (i = 0; i < 7; i++)
        UCA1TXBUF = team[i];
}

void testmessage(void) //Sends initial message over ESP8266
{
    char test[] =
            { '#', 't', 'e', 't', 's', 'u', 'c', 'c', 'e', 's', 's', '\n' };
    for (i = 0; i < 12; i++)
        UCA1TXBUF = test[i];
}

int main(void)                           //Main function
{
    WDTCTL = WDTPW | WDTHOLD;            //stop watchdog timer
    UART_Setup();                    //Runs the intialize UART function
    initGPIO();                          //Runs the initialize the GPIO pin
    subscribe();                       //Runs initial subscription
    testmessage();                      //Runs initial message
    owsetup();                          //Temp sensor setup

    _BIS_SR(LPM0_bits + GIE); //Turns on low power mode and enables global interrupts
    while (1)               //contantly scans for temp sensor values (CURRENTLY DOES NOT WORK, ALL VALUES DETECTED AS 255 bits (MAX TEMPERATURE)
    {
        onewire_reset(&ow);
        onewire_write_byte(&ow, 0xcc); // skip ROM command
        onewire_write_byte(&ow, 0x44); // convert T command
        onewire_line_high(&ow);
        DELAY_MS(800); // at least 750 ms for the default 12-bit resolution
        onewire_reset(&ow);
        onewire_write_byte(&ow, 0xcc); // skip ROM command
        onewire_write_byte(&ow, 0xbe); // read scratchpad command
        for (i = 0; i < 9; i++)
            scratchpad[i] = onewire_read_byte(&ow); //reads value from sensor
    }
    //return 0;                            //Returns default 0, will never reach while in loop
}

#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)        // Port 1 interrupt service routine
{
    if (P1IN & BIT3)
    {                     //we got a 1 from the fingerprint sensor for eric's fingerprint
        char eric[] = { '#', 'e', 'r', 'i', 'c', 's', 'u', 'b', 'b', 'e', 'd',
                        '\n' };
        i = 0;                 //Initialize counter
        P2OUT |= BIT4;          //turns on LED confirmation
        while (i <= 11)
        {            //Loop while the counter is less than the length
            while (!(UCA1IFG & UCTXIFG))
                ;    //While the flags have gone off
            UCA1TXBUF = eric[i]; //TX buffer gets the next element in the array
            i++;                    //Increment counter
        }
        __delay_cycles(1000);
        P2OUT &= ~BIT4;         //turns off LED confirmation
    }
    else if (P1IN & BIT4)
    {                     //we got a 1 from the fingerprint sensor for kyle's fingerprint
        char kyle[] = { '#', 'e', 'r', 'i', 'c', 's', 'u', 'b', 'b', 'e', 'd',
                        '\n' };
        i = 0;                 //Initialize counter
        P2OUT |= BIT4;           //turns on LED confirmation
        while (i <= 11)
        {            //Loop while the counter is less than the length
            while (!(UCA1IFG & UCTXIFG))
                ;    //While the flags have gone off
            UCA1TXBUF = kyle[i]; //TX buffer gets the next element in the array
            i++;                    //Increment counter
        }
        __delay_cycles(1000);
        P2OUT &= ~BIT4;         //Turns off LED confirmation
    }
    else
    { //its a zero
        char unknown[] = { '#', 'F', 'I', 'N', 'G', 'E', 'R', 'P', 'R', 'I',
                           'N', 'T', 'U', 'N', 'K', 'N', 'O', 'W', 'N' };
        i = 0;                 //Initialize counter
        while (i <= 19)
        {            //Loop while the counter is less than the length
            while (!(UCA1IFG & UCTXIFG))
                ;    //While the flags have gone off
            UCA1TXBUF = unknown[i]; //TX buffer gets the next element in the array
            i++;                    //Increment counter
        }
    }
    P1IFG &= ~BIT3 + ~BIT4;                      //Clear interrupt flag for GPIO
}
