/*	Author: Megan Miyazaki
 *  Partner(s) Name: 
 *	Lab Section:
 *	Assignment: Final Project
 *	Exercise Description: [optional - include for your own benefit]
 *	Finished the custom character and EEPROM complexity. Changed the menu to show the previous
 *	score as well as the high score, which is saved even when the system is turned off and on.
 *	
 * 	
 *	
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 * 
 *	Demo Link: https://drive.google.com/file/d/13Eega9phNWj2H41dZ0XVQXlYjVHp5tXp/view?usp=sharing
 */
#ifdef _SIMULATE_
#include <avr/io.h>
#include <io.h>
#include <Joystick_ADC.h>
#include <timer.h>
#include "simAVRHeader.h"
#include <shift.h>
#include <avr/eeprom.h>
#endif

unsigned char starChar[8] = {0x00, 0x04, 0x04, 0x1F, 0x0E, 0x0E, 0x0A, 0x11 };

void LCD_Custom_Char (unsigned char loc, unsigned char *msg) {
    unsigned char i;
    if(loc < 8) {
     LCD_WriteCommand (0x40 + (loc*8));  
       for(i = 0; i < 8; i++) 
           LCD_WriteData(msg[i]);      
    }   
}

typedef struct task {
	int state;
	unsigned long period;
	unsigned long elapsedTime;
	int (*TickFct)(int);
} task;

task tasks[4];
unsigned char numTasks = 4;
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
			if (!gameFlag) state = Game_wait_release;
			else {
				if (startButton) {
					state = Game_wait_press;
					gameFlag = 0;
				}
				else { state = Game_progress_release; }
			}
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

enum Character_states{Character} Character_state;
unsigned char charPos;

int Character_Tick(int state) {

	static int udInput;

	switch (state) {

		case Character :
			state = Character;
			break;
		
		default :
			state = Character;
			break;

	}

	switch (state) {

		case Character :
			if (!gameFlag) charPos = 3;
	
			else {
				udInput = Get_ADC();
				if (udInput > 1000) {
					if (charPos < 5) charPos++;
				}
				else if (udInput < 500) {
					if (charPos > 1) charPos--;
				}		
			}

			break;

		default :
			break;

	}

	return state;

}

enum Fruit_states{Fruit_fall} Fruit_state;

unsigned char FruitRow[5] = {0xB8, 0xE8, 0x78, 0xD8, 0xF0};
unsigned char FruitNum;
uint8_t FruitPos;
unsigned FruitFlag;
unsigned char scoreFlag;

int Fruit_Tick(int state) {

	switch (state) {

		case Fruit_fall :
			state = Fruit_fall;
			break;

		default :
			state = Fruit_fall;
			break;

	}

	switch (state) {

		case Fruit_fall :

			if (!gameFlag) {
				FruitNum = 0;
				FruitFlag = 0;
				FruitPos = 0b10000000;
			}

			else {
				if (!FruitFlag) FruitFlag = 1;
				else if (FruitPos) FruitPos = FruitPos >> 1;
				else {

					FruitPos = 0b10000000;
					if (FruitNum < 4) FruitNum++;
					else { FruitNum = 0; }
					scoreFlag = 0;

				}
				
			}

			break;

	}

	return state;

}

enum Output_states{Output_start, Output} Output_state;

int Output_Tick(int state) {

	static unsigned char stringCnt;
	static unsigned char score;
	static unsigned char tempScore = 0;
	static unsigned char highScore = 0;
	static unsigned char displayFlag = 0;
	static unsigned char life;
	static unsigned char waitMsg[12] = "Press Start!";
	static unsigned char scoreMsg[12] = "Last Score: ";
	static unsigned char highScoreMsg[12] = "High Score: ";
	switch (state) {

		case Output_start :
			if (eeprom_read_byte((uint8_t*)5) > 100) {eeprom_write_byte ((uint8_t*)5, highScore);}
			//eeprom_write_byte((uint8_t*)5, tempScore);	//using this to reset highscore to 0 for demo
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
				score = 0;
				life = 3;
				PORTA &= 0xF1;
				PORTA |= 0x00;
				scoreFlag = 0;
				PORTD &= 0x20;
				if (!displayFlag) {
					LCD_ClearScreen();
					LCD_Cursor(1);
					for (stringCnt = 0; stringCnt < 12; ++stringCnt) {
						LCD_WriteData(scoreMsg[stringCnt]);
					}
					if (tempScore < 10) LCD_WriteData(tempScore + '0');
					else {
						LCD_WriteData(tempScore/10 + '0');
						LCD_WriteData(tempScore%10 + '0');
					}
					LCD_Cursor(17);

					LCD_WriteData(0);
					highScore = eeprom_read_byte((uint8_t*)5);
					for (stringCnt = 0; stringCnt < 12; ++stringCnt) {
						LCD_WriteData(highScoreMsg[stringCnt]);
					}
					if (highScore < 10) LCD_WriteData(highScore + '0');
					else {
						LCD_WriteData(highScore/10 + '0');
						LCD_WriteData(highScore%10 + '0');
					}

					LCD_WriteData(0);
					displayFlag = 1;
				}
				shift_WriteData(0b00000000);
			}
			else {
				LCD_ClearScreen();
				displayFlag = 0;
				LCD_Cursor(1);
				for (stringCnt = 5; stringCnt < 12; ++stringCnt) {

					LCD_WriteData(scoreMsg[stringCnt]);

				}
				shift_WriteData(FruitPos);
				PORTB = FruitRow[FruitNum];
				PORTD &= 0x20;
				PORTD |= (0x01 << (charPos - 1));
				LCD_Cursor(8);
				if (FruitPos == 0b00000001 && !scoreFlag) {

					if ((~(FruitRow[FruitNum] & 0xF8)) & (0x80 >> (charPos - 1))) {
						score++;
						scoreFlag = 1;
					}
					else {
						life--;
						scoreFlag = 1;
					}
				}
				
				if (score < 10) LCD_WriteData(score + '0');
				else {
					LCD_WriteData(score/10 + '0');
					LCD_WriteData(score%10 + '0');
				}
				PORTA &= 0xF1;
				if (life > 0) {
			
					if (life == 3) PORTA |= 0x0E;
					else if (life == 2) PORTA |= 0x06;
					else {PORTA |= 0x02;}

				}
				else {
					PORTA |= 0x00;
					gameFlag = 0;
				}
				tempScore = score;
				if (tempScore > eeprom_read_byte((uint8_t*)5)) eeprom_write_byte((uint8_t*)5, tempScore);
			}
			
			break;

	}

	return state;

}

int main(void) {
    /* Insert DDR and PORT initializations */
	DDRA = 0x0E;	PORTA = 0xF1;
	DDRB = 0xFF;	PORTB = 0x00;
	DDRC = 0xFF;	PORTC = 0x00;
	DDRD = 0xDF;	PORTD = 0x20;
	unsigned char i;

	LCD_init();
	ADC_init();
	shift_init();
	TimerSet(periodGCD);
	TimerOn();
	LCD_Custom_Char(0, starChar);

	tasks[0].state = Game_start;
	tasks[0].period = periodGCD;
	tasks[0].elapsedTime = 0;
	tasks[0].TickFct = &Game_Tick;

	tasks[1].state = Character;
	tasks[1].period = periodGCD;
	tasks[1].elapsedTime = 0;
	tasks[1].TickFct = &Character_Tick;

	tasks[2].state = Fruit_fall;
	tasks[2].period = 200;
	tasks[2].elapsedTime = 0;
	tasks[2].TickFct = &Fruit_Tick;

	tasks[3].state = Output_start;
	tasks[3].period = periodGCD;
	tasks[3].elapsedTime = 0;
	tasks[3].TickFct = &Output_Tick;

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
