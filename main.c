
//Drew Robert and Joe Niski
//ECE 2049
//Lab 4

#include <msp430.h>
#include "peripherals.h"
#include "math.h"

/*
 * DAC pin assignment is as follows
 * LDAC         P3.7
 * CS           P8.2
 * MOSI/SDI     P3.0
 * SCLK         P3.2
 */

//FUNCTIONS
char buttonStates();
void configBoardButtons();
void DACInit(void);
void DACSetValue(unsigned int dac_code);
void stoptimerA2(int reset);
void runtimerA2(void);
void SMCLKsetup();
unsigned int potValue(void);


//GLOBAL VARIABLES
int state = 0;
long unsigned int timer_cnt = 0;
int once = 1;
unsigned char pressed = 0xFF;
int dc = 0;
int square = 0;
int sawtooth = 0;
int triangle = 0;
int setup = 0;
int i = 0, k = 0, j = 0;
unsigned int volts_code = 0;
unsigned int potVal = 0;
unsigned char potArray[5] = {' '};

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;    // Stop watchdog timer.

    initLeds();

    _BIS_SR(GIE);

    DACInit();
    configDisplay();
    configKeypad();
    configBoardButtons();
    SMCLKsetup();

    // *** Intro Screen ***
    Graphics_clearDisplay(&g_sContext); // Clear the display

    while (1)
    {

        switch (state){
        case 0:

            while(once == 1)
            {
            setup = 1;
            runtimerA2();

            // Write some text to the display
            Graphics_drawStringCentered(&g_sContext, "Function", AUTO_STRING_LENGTH, 48, 15, TRANSPARENT_TEXT);
            Graphics_drawStringCentered(&g_sContext, "Generator", AUTO_STRING_LENGTH, 48, 25, TRANSPARENT_TEXT);
            Graphics_drawStringCentered(&g_sContext, "Button 1 = DC", AUTO_STRING_LENGTH, 48, 35, TRANSPARENT_TEXT);
            Graphics_drawStringCentered(&g_sContext, "Bt 2 = Square", AUTO_STRING_LENGTH, 48, 45, TRANSPARENT_TEXT);
            Graphics_drawStringCentered(&g_sContext, "Bt 3 = Sawtooth", AUTO_STRING_LENGTH, 48, 55, TRANSPARENT_TEXT);
            Graphics_drawStringCentered(&g_sContext, "Bt 4 = Triangle", AUTO_STRING_LENGTH, 48, 65, TRANSPARENT_TEXT);

            // Update display
            Graphics_flushBuffer(&g_sContext);
            once = 0;


            }

            if(pressed == 0x01){
                state = 1;
                once = 1;
                Graphics_clearDisplay(&g_sContext); // Clear the display

            }

            else if(pressed == 0x10){
                state = 2;
                once = 1;
                Graphics_clearDisplay(&g_sContext); // Clear the display

            }

            else if(pressed == 0x40){
                state = 3;
                once = 1;
                Graphics_clearDisplay(&g_sContext); // Clear the display

            }

            else if(pressed == 0x04){
                state = 4;
                once = 1;
                Graphics_clearDisplay(&g_sContext); // Clear the display

            }

            break;

        case 1:


            while(once == 1)
            {
            setup = 0;
            stoptimerA2(1);

            // Write some text to the display
            Graphics_drawStringCentered(&g_sContext, "DC", AUTO_STRING_LENGTH, 48, 15, TRANSPARENT_TEXT);

            // Update display
            Graphics_flushBuffer(&g_sContext);
            once = 0;
            }




        case 2:


            while(once == 1)
            {
            setup = 0;
            square = 1;
            runtimerA2();
            // Write some text to the display
            Graphics_drawStringCentered(&g_sContext, "Square Wave", AUTO_STRING_LENGTH, 48, 15, TRANSPARENT_TEXT);

            // Update display
            Graphics_flushBuffer(&g_sContext);
            once = 0;
            }

            DACSetValue(volts_code);

           break;



        case 3:


            while(once == 1)
            {
            setup = 0;
            stoptimerA2(1);
            sawtooth = 1;
            runtimerA2();

            // Write some text to the display
            Graphics_drawStringCentered(&g_sContext, "Sawtooth", AUTO_STRING_LENGTH, 48, 15, TRANSPARENT_TEXT);

            // Update display
            Graphics_flushBuffer(&g_sContext);
            once = 0;
            }

            while(1){
                DACSetValue(volts_code);
            }


        case 4:


            while(once == 1)
            {
            setup = 0;
            stoptimerA2(1);
            triangle = 1;
            runtimerA2();

            // Write some text to the display
            Graphics_drawStringCentered(&g_sContext, "Triangle", AUTO_STRING_LENGTH, 48, 15, TRANSPARENT_TEXT);

            // Update display
            Graphics_flushBuffer(&g_sContext);
            once = 0;
            }

            while(1){
                if (k < 4095)
                {
                    for (k = 0; k < 4095; k++)
                    {
                        DACSetValue(volts_code);
                    }
                }
                else if (k >= 4095)
                {
                    for (k = 4095; k > 0; k--)
                    {
                        DACSetValue(4095 - volts_code);
                    }
                }
            }



        }

    }
}

void runtimerA2(void)
{
    if (setup == 1){
        TA2CTL = TASSEL_1 + MC_1 + ID_0;
        TA2CCR0 = 327; // 327+1 = 328 ACLK tics = ~0.01 seconds
    }
    else if (dc == 1){
        //
    }
    else if (square == 1){
        TA2CTL = TASSEL_1 + MC_1 + ID_0;
        //needs period of 7.1428 ms
        TA2CCR0 = 233; // 327+1 = 328 ACLK tics = ~7.14 ms
    }
    else if (sawtooth == 1){
        TA2CTL = TASSEL_2 + MC_1 + ID_0;
        TA2CCR0 = 10; // 10+1 = 11 SMCLK tics = 2.75x10^-6 seconds
    }
    else if (triangle == 1){
        TA2CTL = TASSEL_2 + MC_1 + ID_0;
        TA2CCR0 = 6; // 6+1 = 7 SMCLK tics = 1.75x10^-6 seconds
    }

    TA2CCTL0 = CCIE; // TA2CCR0 interrupt enabled
}


void stoptimerA2(int reset)
{
// This function stops Timer A2 andresets the global time variable
// if input reset = 1
//
// Input: reset, Output: none
//
// smj, ECE2049, 17 Sep 2013
//
    TA2CTL = MC_0; // stop timer
    TA2CCTL0 &= ~CCIE; // TA2CCR0 interrupt disabled
    if (reset)
        timer_cnt = 0;
}

void SMCLKsetup(){
    P5SEL |= (BIT3|BIT2);
    UCSCTL6 &= ~(XT2OFF);
    UCSCTL4 = 0x0054;
}

// Timer A2 interrupt service routine
#pragma vector=TIMER2_A0_VECTOR
__interrupt void TimerA2_ISR(void)
{

    timer_cnt++;

    pressed = buttonStates(); //determine when the button is pressed
    if (dc == 1)
    {
        //add in something about accessing the actual voltage
    }
    else if (square == 1)
    {
        //unsigned int amplitude = potValue();
        //when timer_cnt%2 = 0, send low
        //when timer_cnt%2 = 1, send high
        //amplitude multiplies the high and low values
        if(timer_cnt % 2 == 0)
        {
            volts_code = 0;
        }
        else if(timer_cnt % 2 == 1)
        {
            volts_code = potValue();
        }
    }
    else if (sawtooth == 1)
    {
        volts_code = timer_cnt % 4096; //0-4095
    }
    else if (triangle == 1)
    {

    }


}

void configBoardButtons(){
    //4 Board Buttons: P7.0, P3.6, P2.2, P7.4

    P7SEL &= (BIT7|BIT6|BIT5|BIT3|BIT2|BIT1); //xxx0 xxx0
    P3SEL &= (BIT7|BIT5|BIT4|BIT3|BIT2|BIT1|BIT0); //x0xx xxxx
    P2SEL &= (BIT7|BIT6|BIT5|BIT4|BIT3|BIT1|BIT0); //xxxx x0xx

    P7DIR &= (BIT7|BIT6|BIT5|BIT3|BIT2|BIT1); //xxx0 xxx0
    P3DIR &= (BIT7|BIT5|BIT4|BIT3|BIT2|BIT1|BIT0); //x0xx xxxx
    P2DIR &= (BIT7|BIT6|BIT5|BIT4|BIT3|BIT1|BIT0); //xxxx x0xx

    P7REN |= (BIT0|BIT4);
    P3REN |= (BIT6);
    P2REN |= (BIT2);

    P7OUT |= (BIT0|BIT4);
    P3OUT |= (BIT6);
    P2OUT |= (BIT2);
}

char buttonStates(){

    char inBits7, inBits3, inBits2;
    char out72=0, out71=0, out3=0, out2=0, out=0;

    inBits7 = P7IN & (BIT0|BIT4); //000x 000x, keep bits 0 and 4
    inBits3 = P3IN & (BIT6); //0x00 0000, keep bit 6
    inBits2 = P2IN & (BIT2); //0000 0x00, keep bit 2

    if (!(inBits7 & BIT0))
        out71 = 0x01; //0000 0001 //Diesel
    else if (!(inBits7 & BIT4))
        out72 = 0x04; //Super
    else if (!(inBits3 & BIT6))
        out3 = 0x10; //Premium
    else if (!(inBits2 & BIT2))
        out2 = 0x40; //Regular

    out = (out71|out72|out3|out2);

    return out;
}

void DACInit(void)
{
    // Configure LDAC and CS for digital IO outputs
    DAC_PORT_LDAC_SEL &= ~DAC_PIN_LDAC;
    DAC_PORT_LDAC_DIR |=  DAC_PIN_LDAC;
    DAC_PORT_LDAC_OUT |= DAC_PIN_LDAC; // Deassert LDAC

    DAC_PORT_CS_SEL   &= ~DAC_PIN_CS;
    DAC_PORT_CS_DIR   |=  DAC_PIN_CS;
    DAC_PORT_CS_OUT   |=  DAC_PIN_CS;  // Deassert CS
}


void DACSetValue(unsigned int dac_code)
{
    // Start the SPI transmission by asserting CS (active low)
    // This assumes DACInit() already called
    DAC_PORT_CS_OUT &= ~DAC_PIN_CS;

    // Write in DAC configuration bits. From DAC data sheet
    // 3h=0011 to highest nibble.
    // 0=DACA, 0=buffered, 1=Gain=1, 1=Out Enbl
    dac_code |= 0x3000;     // Add control bits to DAC word

    uint8_t lo_byte = (unsigned char)(dac_code & 0x00FF);
    uint8_t hi_byte = (unsigned char)((dac_code & 0xFF00) >> 8);

    // First, send the high byte
    DAC_SPI_REG_TXBUF = hi_byte;

    // Wait for the SPI peripheral to finish transmitting
    while(!(DAC_SPI_REG_IFG & UCTXIFG)) {
        _no_operation();
    }

    // Then send the low byte
    DAC_SPI_REG_TXBUF = lo_byte;

    // Wait for the SPI peripheral to finish transmitting
    while(!(DAC_SPI_REG_IFG & UCTXIFG)) {
        _no_operation();
    }

    // We are done transmitting, so de-assert CS (set = 1)
    DAC_PORT_CS_OUT |=  DAC_PIN_CS;

    // This DAC is designed such that the code we send does not
    // take effect on the output until we toggle the LDAC pin.
    // This is because the DAC has multiple outputs. This design
    // enables a user to send voltage codes to each output and
    // have them all take effect at the same time.
    DAC_PORT_LDAC_OUT &= ~DAC_PIN_LDAC;  // Assert LDAC
     __delay_cycles(10);                 // small delay
    DAC_PORT_LDAC_OUT |=  DAC_PIN_LDAC;  // De-assert LDAC
}

unsigned int potValue(void)
{
    REFCTL0 &= ~REFMSTR;
    ADC12CTL0 = ADC12SHT0_9 | ADC12REFON | ADC12REF2_5V | ADC12ON;
    ADC12CTL1 = ADC12SHP;
    ADC12MCTL0 = ADC12SREF_0 + ADC12INCH_0;

    P6SEL |= BIT0;
    ADC12CTL0 &= ~ADC12SC;

    ADC12CTL0 |= ADC12SC + ADC12ENC;

    while (ADC12CTL1 & ADC12BUSY)
    {
        __no_operation();
    }

    potVal = ADC12MEM0 & 0x0FFF;
    return potVal;
}

void printPotVal(unsigned int gal)
{
    for (j = 4; j >= 0; j--)
    {
        potArray[j] = ((gal % 10) + 0x30);
        gal = gal / 10;
    }
}


