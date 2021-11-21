/*	Author: Megan Miyazaki
 *  Partner(s) Name: 
 *	Lab Section:
 *	Assignment: Final Project
 *	Exercise Description: [optional - include for your own benefit]
 *	Completed the joystick complexity and game start button function.
 * 	The system would start by prompting the user to press the "start" button connected to D5 with the message, "Press start!" 
 *	displayed on the LCD screen. Once the start button has been pressed,
 * 	the joystick's inputs will be represented on the LED's B7, B4, B3, and B0, and the message, "Joystick test 
 *	in progress..." will be displayed on the LCD. If the joystick points to the right, B0 will light. If the joystick
 * 	points to the left, B7 will light. If the joystick is in its "neutral position", then B4 - B3 will light. 
 *	Pressing the "start" button will end this testing period and go back to prompting the user to press the 
 *	"start" button with the message, "Press start!" displayed in the LCD screen.
 * 	Important Note: I changed pinout of project. The joystick connects only A0 instead of A2 - A0, since
 *	I will only be using the left/right input of the joystick for my game.
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 * 
 *	Demo Link: https://drive.google.com/file/d/1AMUi2I0sBytprq2prrbuBXMO3DVbCmeZ/view?usp=sharing
 */
#include <avr/io.h>
#include <io.h>
#include <Joystick_ADC.h>
#include <timer.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

typedef struct task {
	int state;
	unsigned long period;
	unsigned long elapsedTime;
	int (*TickFct)(int);
} task;

task tasks[2];
unsigned char numTasks = 2;
unsigned long periodGCD = 50;
unsigned char gameFlag = 0;

enum Game_states{Game_start, Game_wait_release, Game_progress_press, Game_progress_release, Game_wait_press} Game_state;

int Game_Tick(int state) {

	static unsigned char startButton;

	switch (state) {

		case Game_start :
			gameFlag = 0;
			state = Game_wait_release;
			break;

		case Game_wait_release :
			startButton = ~PIND & 0x20;
			if (startButton) {
				state = Game_progress_press;
				gameFlag = 1;
			}
			else { state = Game_wait_release; }
			break;

		case Game_progress_press :
			startButton = ~PIND & 0x20;
			if (startButton) state = Game_progress_press;
			else { state = Game_progress_release; }
			break;

		case Game_progress_release :
			startButton = ~PIND & 0x20;
			if (startButton) {
				state = Game_wait_press;
				gameFlag = 0;
			}
			else { state = Game_progress_release; }
			break;

		case Game_wait_press :
			startButton = ~PIND & 0x20;
			if (startButton) state = Game_wait_press;
			else { state = Game_wait_release; }
			break;

		default :
			state = Game_start;
			break;

	}

	return state;

}

enum Output_states{Output_start, Output} Output_state;

int Output_Tick(int state) {

	static int lrInput;
	static unsigned char stringCnt;
	static unsigned char waitMsg[12] = "Press Start!";
	static unsigned char testMsg1[13] = "Joystick test";
	static unsigned char testMsg2[16] = "in progress...  ";
	switch (state) {

		case Output_start :
			state = Output;
			break;

		case Output :
			state = Output;
			break;

		default :
			state = Output_start;
			break;

	}

	switch (state) {

		case Output :
			if (!gameFlag) {
				LCD_ClearScreen();
				for (stringCnt = 0; stringCnt < 12; ++stringCnt) {
					LCD_Cursor(stringCnt + 1);
					LCD_WriteData(waitMsg[stringCnt]);
				}
				PORTB = 0;
			}
			else {
				LCD_ClearScreen();
				for (stringCnt = 0; stringCnt < 13; ++stringCnt) {
					LCD_Cursor(stringCnt + 1);
					LCD_WriteData(testMsg1[stringCnt]);
				}
				for (stringCnt = 0; stringCnt < 16; ++stringCnt) {
					LCD_Cursor(stringCnt + 17);
					LCD_WriteData(testMsg2[stringCnt]);
				}
				lrInput = Get_ADC();
				if (lrInput > 1000) PORTB = 1;
				else if (lrInput < 500) PORTB = 0x80;
				else {PORTB = 0x18;}
			}
			break;

	}

	return state;

}

int main(void) {
    /* Insert DDR and PORT initializations */
	DDRA = 0x00;	PORTA = 0xFF;
	DDRB = 0xFF;	PORTB = 0x00;
	DDRC = 0xFF;	PORTC = 0x00;
	DDRD = 0xDF;	PORTD = 0x20;
	unsigned char i;

	LCD_init();
	ADC_init();
	TimerSet(periodGCD);
	TimerOn();

	tasks[0].state = Game_start;
	tasks[0].period = periodGCD;
	tasks[0].elapsedTime = 0;
	tasks[0].TickFct = &Game_Tick;

	tasks[1].state = Output_start;
	tasks[1].period = periodGCD;
	tasks[1].elapsedTime = 0;
	tasks[1].TickFct = &Output_Tick;

    /* Insert your solution below */
    while (1) {
	for (i = 0; i < numTasks; ++i) {

		if (tasks[i].elapsedTime >= tasks[i].period) {

			tasks[i].state = tasks[i].TickFct(tasks[i].state);
			tasks[i].elapsedTime = 0;

		}
		tasks[i].elapsedTime += periodGCD;

	}
	while(!TimerFlag) {}
	TimerFlag = 0;
    }
    return 1;
}
