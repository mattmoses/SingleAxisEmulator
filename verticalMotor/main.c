// Vertical motor driver code 
// Oringially made by Kevin Wolfe 2009-2011
// Modified by Matt Moses 2010-2011
// 

#include <p18f1320.h>
//#include <stdlib.h>
//#include <delays.h>
//#include <math.h>


#define counterToSpeedScalar 150
#define numCountsPerStep 1
#define maxSpeed 255
#define Kp 35.0
#define Kd 0.0
#define Ki 0.0
#define dt 0.01
#define speedStep 1
#define errorTol 1
#define tempADSpeed 255
#define stepHomeThreshold 500
#define upTorqueLimit 150
#define downTorqueLimit -120

#pragma config IESO = ON
#pragma config FSCM = ON
#pragma config OSC = INTIO2        
#pragma config PWRT = OFF
#pragma config BOR = OFF
#pragma config WDT = OFF
#pragma config MCLRE = OFF

/****** DISABLE LVP (LOW-VOLTAGE Programming) ******/
/* This allows RB5 to operate as an interuppt on change I/O pin */
#pragma config LVP = OFF
/***************************************************/

void low_isr (void);
void high_isr (void);
void set_motor_output (unsigned char desiredPWM, unsigned char desiredDirection);

#pragma code high_vector_section=0x8
void high_vector (void)
{
_asm GOTO high_isr _endasm
}
#pragma code

#pragma code low_vector_section=0x18
void low_vector (void)
{
_asm GOTO low_isr _endasm
}
#pragma code

volatile int stepHighCounter = 0;

volatile char stepPulses = 0;
//volatile int stepCounts;
volatile int currentCounter = 0;
//volatile unsigned char currentADvalue;
volatile unsigned char trigger = 0;
//volatile structPrevStates prevStates;
//volatile enum { DIR_CW = 0, DIR_CCW = 1 } direction;
volatile enum { LOW = 0, HIGH = 1} prevA = LOW;

// changeToggle is a varb used for debugging
volatile enum {LOW = 0, HIGH = 1} changeToggle;

#pragma interrupt high_isr
void high_isr(void)
{
	if ( INTCONbits.INT0IF ) //step pulse flag 
	{

		// reset stepTimerFoo
		stepHighCounter = 1;

		// do important stuff
		if ( PORTBbits.RB1 )    //Check direction pin
		{
			stepPulses++;
		}
		else
		{
			stepPulses--;
		}
	
		INTCONbits.INT0IF = 0; //reset step flag
	}

	if ( INTCONbits.RBIF ) //PORTB change interrupt
	{

//		// This is a little block for debugging - toggles A every time a change on PORTB
//	
//		if (changeToggle) {
//			changeToggle = 0;
//			PORTAbits.RA1 = 0;
//		}
//		else {
//			changeToggle = 1;
//			PORTAbits.RA1 = 1;
//		} 

		if ( PORTBbits.RB5^prevA )  	//CW Rotation
		{
			currentCounter++;
			//PORTAbits.RA1 = 0;
		}
		else						//CCW Rotation
		{
			currentCounter--;
			//PORTAbits.RA1 = 1;
//			if (changeToggle) {
//				changeToggle = 0;
//				PORTAbits.RA1 = 0;
//			}
//			else {
//				changeToggle = 1;
//				PORTAbits.RA1 = 1;
//			} 
		}


		prevA = PORTBbits.RB4;
		PORTAbits.RA1 = PORTBbits.RB5;

		INTCONbits.RBIF = 0; //resetQuadFlag
	}
}

#pragma interruptlow low_isr
void low_isr(void)  //timer interupt
{
	trigger = 1;
	INTCONbits.TMR0IF = 0; 			// reset TMR0 interrupt flag
}


void set_motor_output(unsigned char desiredPWM, unsigned char desiredDirection)
{
	if ( desiredDirection )
	{
		PORTBbits.RB2 = 0;
		CCPR1L = desiredPWM;  			// This is the register for the 8 MSB of the PWM pin RB3.
	}
	else
	{
		PORTBbits.RB2 = 1;
		CCPR1L = (desiredPWM );  	// This is the register for the 8 MSB of the PWM pin RB3.
	}
}

long abs(long i) 
{ 
	if (i < 0) 
		return -i; 
	else 
		return i; 
} 

char sgn(long i)
{
	if (i < 0)
		return -1;
	else if (i > 0)
		return 1;
	else
		return 0;
}


void main (void)
{
	long prevCounter = 0;
	unsigned char tempTrigger = 0; 
	char tempStepPulses = 0;
	long tempCurrentCounter = 0;
	
	unsigned char currentADvalue = tempADSpeed;
	long desiredCounter = 0;
	long error = 0;
	long prevError = 0;

	long velocity = 0;

	float integral = 0.0;
	float derivative = 0.0;

	long output = 0;

	char testState = 0;
	long testCounter = 0;



	char homeFlag = 0;
    enum { LOW = 0, HIGH = 1} homeDir = LOW;

	/*************** SETUP INTERRUPTS ******************/
	INTCONbits.GIEH = 0; 		// Temporarily disable global interrupts
	INTCONbits.GIEL = 0; 		// Temporarily disable peripheral interrupts
	INTCONbits.TMR0IE = 1; 		// Enable TMR0 Overflow interrupt
	INTCONbits.INT0IE = 1; 		// Enable external interrupt on INT0/RB0
	INTCONbits.RBIE = 1; 		// Enable Port B change interrupt

	//INTCON2bits.RBPU = 0; 		// Enable Port B pull-ups
	INTCON2bits.RBPU = 1; 		// Disable Port B pull-ups
	INTCON2bits.INTEDG0 = 1; 	// Interrupt on rising edge for INT0
	INTCON2bits.TMR0IP = 0; 	// Set TMR0 interrupt priority to Low
	INTCON2bits.RBIP = 1; 		// Set Port B change interrupt to High

	RCONbits.IPEN = 1;			// Enable priority levels on interrupts
	/****************************************************/




	/***************** SETUP TMR0 ***********************/
	//T0CONbits.TMR0ON = 0; 	// Temporarily disable TMR0 until the end of the setup;
	T0CONbits.T08BIT = 1; 	// Use an 8-bit timer (1). Use a 16-bit timer (0).
	T0CONbits.T0CS = 0; 	// Use internal clock cycles for the timer.
	T0CONbits.PSA = 0; 		// Use prescalar to adjust overflow rate (0). Do not use prescalar (1).
	
	/* Assign Prescalar */
	T0CONbits.T0PS2 = 0;
	T0CONbits.T0PS1 = 1;
	T0CONbits.T0PS0 = 1;

	/* This makes go slower */
	//T0CONbits.T0PS2 = 1;
	//T0CONbits.T0PS1 = 1;
	//T0CONbits.T0PS0 = 1;

	T0CONbits.TMR0ON = 1;
	/***************************************************/
	



	/************** SETUP I/O PINS *********************/
	TRISBbits.TRISB4 = 1;	// Encoder ChA input
	TRISBbits.TRISB5 = 1;	// Encoder ChB input
	TRISBbits.TRISB6 = 0;	// Set as output to avoid Port B interrupt on change 
	TRISBbits.TRISB7 = 0;	// Set as output to avoid Port B interrupt on change 

	TRISBbits.TRISB0 = 1;	// Step pulse input
	TRISBbits.TRISB1 = 1;	// Direction input

	//TRISAbits.TRISA4 = 0; 	// Possible motor output pin RA4

	TRISAbits.TRISA0 = 1;	// Set RA0 as input for SPEED pin (analog input for A/D)
	TRISAbits.TRISA1 = 0;	// Set RA1 as output for diagnostic LED

	//ADCON0bits.VCFG0 = 0;	// Set A/D to External Vref(+/-) using 1 and 1
	//ADCON0bits.VCFG1 = 0;	// Set A/D to Vss and Vdd references using 0 and 0
	/***************************************************/




	/**************** SETUP A/D ************************/
	/* Configure analog and digital pins */
	ADCON1bits.PCFG0 = 1; // Set AN0 port as digital input (Home switch)
	ADCON1bits.PCFG1 = 1; // RA1/AN1 diagnostic LED
	ADCON1bits.PCFG2 = 1; // Reserved - test pad on AN2
	ADCON1bits.PCFG3 = 1; // Reserved - test pad on AN3
	ADCON1bits.PCFG4 = 1; // Set AN4/RB0 as a digital pin (Step pulse input)
	ADCON1bits.PCFG5 = 1; // Set AN5/RB1 as a digital pin (Direction input)
	ADCON1bits.PCFG6 = 1; // Set AN6/RB4 as a digital pin (ChA input)
//
//	/* Select A/D input channel as AN0 */
//	ADCON0bits.CHS0 = 0;
//	ADCON0bits.CHS1 = 0;
//	ADCON0bits.CHS2 = 0;
//
//	/* Set A/D clock as Fosc/2 */
//	ADCON2bits.ADCS0 = 0;
//	ADCON2bits.ADCS1 = 0;
//	ADCON2bits.ADCS2 = 0;
//
//	ADCON2bits.ADFM = 0; // Left justify A/D conversion so only the 8 MSB are read later
//
//	/* Turn on A/D module */
//	ADCON0bits.ADON = 1;
	
	/* Turn off A/D module */
	ADCON0bits.ADON = 0;

	/***************************************************/




	/**************** SETUP PWM ************************/
	
	/* Configure PWM pins P1A and P1B as inputs. */
	TRISBbits.TRISB3 = 1;
	TRISBbits.TRISB2 = 1;

	/* Use PR2 to set the PWM period */
	PR2 = 0xFF;

	// Equation 15-1 PWM Period = [(PR2 + 1)] * 4 * TOSC * TMR2 Prescaler
	// or about 1 / 490Hz

	/* Set PWM Mode with P1A as a single modulated output */
	CCP1CONbits.CCP1M0 = 0;
	CCP1CONbits.CCP1M1 = 0;
	CCP1CONbits.CCP1M2 = 1;
	CCP1CONbits.CCP1M3 = 1;
	CCP1CONbits.P1M0 = 0;
	CCP1CONbits.P1M1 = 0;

	/* LSB of the PWM duty cycle */
	CCP1CONbits.DC1B1 = 0;
	CCP1CONbits.DC1B0 = 0;

	/* MSB of the PWM duty cycle */
	CCPR1L = 0x00;
	
	/* Configure TMR2 */
	PIR1bits.TMR2IF = 0;	// Clear interupt flag for TMR2 equal to PR2
	T2CONbits.T2CKPS1 = 1;  // Set prescaler to 16 to decrease PWM frequency
	T2CONbits.T2CKPS0 = 1;

	T2CONbits.TMR2ON = 1; // Turn on Timer2
	
	/* Wait for the TMR2IF flag go high before procedding */
	while (!(PIR1bits.TMR2IF))
	{
		int temp = 0;
	}	

	/* Set the PWM pins P1A and P1B as outputs. */
	TRISBbits.TRISB3 = 0;
	TRISBbits.TRISB2 = 0;

	ECCPASbits.ECCPASE = 0;
	/*****************************************************/

	/***************** SET OSC SPEED *********************/

    //OSCCONbits.IRCF0 = 1;
	//OSCCONbits.IRCF1 = 1;
	//OSCCONbits.IRCF2 = 1;

    OSCCONbits.IRCF0 = 1;
	OSCCONbits.IRCF1 = 1;
	OSCCONbits.IRCF2 = 1;

	/*****************************************************/
	


	// This line is part of the initial check sequence and is not
	// needed for normal operation:
	desiredCounter = 0;

	trigger = 0;
	stepPulses = 0;
	currentCounter = 0;
	


	set_motor_output( 0, 0 );


	/****************** ENABLE INTERRUPTS *****************/
	INTCONbits.GIEH = 1; 		// Enable global interrupts
	INTCONbits.GIEL = 1; 		// Enable peripheral innterupts
	/******************************************************/
	
	while (1)
	{

		INTCONbits.GIEH = 0;
			tempTrigger = trigger;
		INTCONbits.GIEH = 1;
		
		if ( tempTrigger )
		{	

			INTCONbits.GIEH = 0;
				trigger = 0;
				tempCurrentCounter = currentCounter;
				tempStepPulses = stepPulses;
				stepPulses = 0;

				// Check if step pin has been held HIGH.  If it has been held HIGH
				// long enough to indicate a HOME command, set home flag, otherwise
				// just increment stepHighCounter.  stepHighCounter gets reset in
				// in interrupt routine that reads step and direction lines.  
				if (stepHighCounter) {
					if (stepHighCounter > stepHomeThreshold) {
						homeFlag = 1;
						stepHighCounter = 0;
						homeDir = PORTBbits.RB1; //set home direction to direction input pin
					}
					else {
						if (PORTBbits.RB0) {
							stepHighCounter++;
						}
					}
				}
				//stepTimerFoo++;
				//tempStepTimerFoo = stepTimerFoo;
			INTCONbits.GIEH = 1;
	


	
			// if tempStepTimerFoo > something 
			//    then set homeFlag, set homeDir

			// if homeflag
				// if homeswitch
				//		turn off homing
				//	else
				//		set motor go to home in homeDir direction
				//	end
			//else
			//	do normal motor control
			//end
				
		




/*
			INTCONbits.GIEH = 0;
				trigger = 0;
			INTCONbits.GIEH = 1;

			

			//if (PORTAbits.RA1)
			//	PORTAbits.RA1 = 0;
			//else
			//	PORTAbits.RA1 = 1;

			INTCONbits.GIEH = 0;
				tempCurrentCounter = currentCounter;
				tempStepPulses = stepPulses;
				stepPulses = 0;
			INTCONbits.GIEH = 1; */

		//	velocity = (tempCurrentCounter - prevCounter)*counterToSpeedScalar;
		//	prevCounter = tempCurrentCounter;

				
			// Uncomment this for normal operation
			desiredCounter = desiredCounter + (tempStepPulses*numCountsPerStep);
			


			if (homeFlag) // If we are in homing mode
			{
				if (PORTAbits.RA0) // if homeswitch is ON
				{
					// We're home!
					set_motor_output(0,0);
					homeFlag = 0;
					desiredCounter = 0;
					
					// turn off interrupts to clear sensitive variables
					INTCONbits.GIEH = 0;
						currentCounter = 0;
					INTCONbits.GIEH = 1;
				}
				else // if homeswitch is OFF - we are still going home
				{	 
					if (homeDir) // going up
					{
						set_motor_output(abs(upTorqueLimit),1);
					}
					else // going down
					{
						set_motor_output(abs(downTorqueLimit), 0);
					}
				}
			}
			else
			{
				error = desiredCounter - tempCurrentCounter;
				output =  Kp * error;

				////////////// Set motor output //////////////////////////////
				//
				// The variable "output" controls voltage to motor. Examples:
				//
				// this makes the motor go up:
				// set_motor_output(30,1);
            	//
				// this makes the motor go down:
				// set_motor_output(30,0);
				//
				if (output <= 0)
				{
					if (output < downTorqueLimit)
					{
						output = downTorqueLimit;
					}
					set_motor_output(abs(output),0);
				}
				else
				{
					if (output > upTorqueLimit)
					{
						output = upTorqueLimit;
					}
					set_motor_output(abs(output),1);
				}
			}
		}
	}
}


