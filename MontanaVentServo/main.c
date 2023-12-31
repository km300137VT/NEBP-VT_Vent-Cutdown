/**
 * -------------------------------------------------
 * BOREALIS Valve Control
 * -------------------------------------------------
 * TARGET DEVICE: MSP430G2230
 *
 * Authored by Kristoffer Allick 12/30/2022
 */


#include <msp430.h> 
// Configure pins (all in P1)
#define ON_LED BIT2
#define RESISTOR BIT5
#define XBEE3  BIT6
#define SERVO  BIT7
//servo
#define CLOSED  0
#define OPEN 1
//const unsigned char OPEN = 1;
#define FALSE 0
#define TRUE 1
#define SERVO_ANGLE_BIG 1000  //input desired pulse width here from roughly 1700-400 to change angle
#define SERVO_ANGLE_SMALL 1000 //input desired pulse width here from roughly 1700-400 to change angle
// ---------------------------------------------------------
// =========================================================


// Function declarations
void initTimer_A(void);
void delay_overflows(int n);



//Global Variables
unsigned char servo_status;
unsigned int temp;
unsigned long overflow_counter; // Used by delay62MS function & timer

/**
 * main.c
 */
int main(void){
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer    [ NOTE :: watchdog timer protects system against failure of software; resets when limit reached ]
    servo_status= CLOSED;                              // [ NOTE :: set servo_status to 0 ]


    // Configure output pins
    /*
        [ NOTE :: Pin 2 - LED; Pin 5 - Resistor; Pin 7 - SERVO ]
        [ NOTE :: starts with LED ON, Resistor ON, Servo OFF ]
        [ NOTE :: P1DIR - Set Outputs; P1OUT - when to send data to a pin; THIS CODE SEGMENT IS INITIAL OUTPUTS ]
     */
    P1DIR |= ON_LED; //P1.2 LED  enable output          // [ NOTE :: set P1DIR to 00000100 aka set Pin 2 (LED) to HIGH ]
    P1OUT |= ON_LED; //LED on
    P1DIR |= SERVO; // P1.SERVO enable output           // [ NOTE :: set P1DIR to 10000100 aka set Pin 7 (SERVO) to HIGH ]
    P1OUT &=~(SERVO);//Start off
    P1DIR |= RESISTOR; //P1.5 RESISTOR enable output    // [ NOTE :: set P1DIR to 10100100 aka set Pin 5 (Resistor) to HIGH ]
    P1OUT |=(RESISTOR);//Start on


    // Configure input pin
    P1DIR &= ~(XBEE3);//P1.6(XBEE3) enable input              // [ NOTE :: Pin 6 (XBEE3) is set by this operation : P1DIR & 1011111 ]
    P1IES &=~(XBEE3); //Start looking for low to high edge    // [ NOTE :: This will cause an interrupt when Pin 6 Changes from LOW to HIGH ] -- [ NOTE :: when P1IES 0 -> 1, P1IFG is set, so an interrupt will occur immediately ]
    P1IFG &=~ (XBEE3);                                        // [ NOTE :: Because of the condition previously mentioned, this code will clear any interrupt caused by P1IFG ]
    P1IE |=(XBEE3);  // Interrupt Enable

    // [ BG INFO :: the internal DOC (Digitally Controller Oscillator) is configured with 2 registers-- BCSCTL1 and DCOCTL (basic clock system control 1 and DCO control) ]
    //MCLK = SMCLK = 1MHz        [ NOTE :: master clock and sub-main clock are sourced from DCOCTL which has a frequency of approx 1MHz (calibrated value I think for this device) ]
    DCOCTL =0;                // [ NOTE :: this clears DCOCTL and sets it to the lowest setting ]
    BCSCTL1 = CALBC1_1MHZ;    // [ NOTE :: Using the preconfigured (in the factory) bytes as a load in the respective registers (BCSCTL1 and DCOCTL) to get approx 1MHz)
    DCOCTL = CALDCO_1MHZ;     // [ NOTE :: IN CONCLUSION -- this part of the code is configuring the board so that DCOCTL is running at 1 MHz ]

    initTimer_A();
    __enable_interrupt();
    while(TRUE){};

}
//------------------Functions-----------------------------------
void initTimer_A(void){
    //Timer0_A3 Config
    TACCTL0 |=CCIE; //Enable CCRO interrupt
    TACCTL1 |=CCIE; //Enable CCR1 interrupt
    TACCR1 = 20000; //CCR1 intial value (Period)
    TACCR0 = TACCR1+SERVO_ANGLE_SMALL+(SERVO_ANGLE_BIG*servo_status); // CCR0 initial value (Pulse Width)
    TACTL = TASSEL_2 + ID_0 + MC_2; //Use SMCLK, SMLK/1, Counting Continous Mode
}
//Delays the inputted overflow
void delay_overflows(int n){
    unsigned long desired_overflows= overflow_counter+n; //cool maths for desired delay in 65 ms intervals
    while (overflow_counter < desired_overflows);//do nothing until desired delay in 65 ms intervals
}
void delay_SEC(int sec){
    unsigned long desired_overflows = overflow_counter+15*sec; //cool maths for desired delay in minutes
    while (overflow_counter < desired_overflows);//do nothing until desired delay in minutes
}

void Resist(void){

}

//------------------------Interrupt Service Routines---------------------
//Timer ISR
#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer_A_CCR0_ISR(void) {
   P1OUT &=~(SERVO);        //set SERVO low
   TACCR0 = TACCR1+SERVO_ANGLE_SMALL+(SERVO_ANGLE_BIG*servo_status);    //Add one Period
}
//Servo ISR
#pragma vector = TIMER0_A1_VECTOR
__interrupt void Timer_A_CCR1_ISR(void){
    temp = TAIV;
    if (temp == TAIV_TACCR1){
        P1OUT |=(SERVO);    //set SERVO High
        TACCR1 +=20000;     //add one Period
    }
}

//PWM pin to SERVO pin ISR
#pragma vector = PORT1_VECTOR
__interrupt void Servo_change(void){
    if ((P1IES & XBEE3) == 0){  //rising edge trigger
        P1IES |=(XBEE3);        //start looking for falling edge
        servo_status = OPEN;
    }
    else{                   //falling edge trigger
        P1IES &=~(XBEE3);   //start looking for rising edge
        servo_status = CLOSED;
    }
    P1IFG &=~(XBEE3);
}
