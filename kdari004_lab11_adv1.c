/*	Author: lab
 *  Partner(s) Name: Kathleen Mae Dario
 *	Lab Section:
 *	Assignment: Lab 11  Exercise 2
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 *
 *	Demo link:
 */
#include <avr/io.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

#include "scheduler.h"
#include "timer.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

//========================================
// Global variables
//========================================
unsigned char playTwo = 0; //0=singlePlay (AI) , 1=twoPlayers (playerTwo)

unsigned char plOnePattern = 0x80;
unsigned char plOneRow = 0x11;

unsigned char ballPattern = 0x02;
unsigned char ballRow = 0x1B;

unsigned char aiPattern = 0x01;
unsigned char aiRow = 0x11;

unsigned char StartGame = 0;

//========================================
// Start Game
//========================================

enum gameStates {WaitStart, Start, WaitReset, Reset};

int gameStartSM(int state){
	unsigned char startButton = ~PINB & 0x01;
	switch(state){
		case WaitStart:
			if(startButton){
				StartGame = 1;
				state = Start;
			}
			break;
		case Start:
			if(!startButton)
				state = WaitReset;
			break;
		case WaitReset:
			if(startButton){
				StartGame = 0;
				
				//reset playerOne
				plOnePattern = 0x80;
				plOneRow = 0x11;
				
				//reset ball
				ballPattern = 0x02;
				ballRow = 0x1B;
				
				aiPattern = 0x01;
				aiRow = 0x11;


				state = Reset;
			}
			break;
		case Reset:
			if(!startButton)
				state = WaitStart;
			break;
		default:
			state = WaitStart;
			break;
	}
	return state;
}

//============================================
//PickPlayer2
//============================================

enum pickPlTwoStates {WaitPick, SwitchPlTwo};

int pickPlTwoSM(int state){
	unsigned char switchPl = (~PINB & 0x08) >> 3;
	switch(state){
		case WaitPick:
			if(switchPl){
				playTwo = (playTwo) ? 0 : 1;
				state = SwitchPlTwo;
			}
			else
				state = WaitPick;
			break;
		case SwitchPlTwo:
			state = (switchPl)? SwitchPlTwo : WaitPick;
			break;
		default:
			state = WaitPick;
			break;
	}
	return state;
}


//===================================
//PlayerOne SM
//===================================
enum playerOne {Wait, Up, Down};

int movePlayer(int state){
	
	unsigned char upButton = ~PINB & 0x02;
	unsigned char downButton = ~PINB & 0x04;
	
	plOneRow = plOneRow & 0x1F;
	
	if(StartGame == 1){
		switch(state){
			case Wait:
				if(!upButton && !downButton)
					state = Wait;
				else if(upButton && !downButton){
					state = Up;
					if(plOneRow  == 0x03 || plOneRow == 0x11)
						plOneRow = ((plOneRow >> 1) | 0x10) & 0x1F;
				}
				else{
					state = Down;
					if(plOneRow == 0x18 || plOneRow == 0x11)
						plOneRow = ((plOneRow << 1) | 0x01) & 0x1F;
				}
				break;
			case Up:
				if(upButton)
					state = Up;
				else
					state = Wait;
				break;
			case Down:
				if(downButton)
					state = Down;
				else
					state = Wait;
				break;
			default:
				state = Wait;
				break;
	
		}
	}

	return state;
}


//===================================
//Ball
//===================================

enum BallStates { WaitBall, MoveStraightLeft, MoveUpLeft, MoveDownLeft, MoveStraightRight, MoveUpRight, MoveDownRight};

int ballSM(int state){	
	if(StartGame == 0)
		return WaitBall;

	switch(state){
		case WaitBall:
			if(StartGame){
				state = MoveStraightLeft;
			}
			break;
		case MoveStraightLeft:
			if(ballPattern == 0x40){
				if(plOneRow == 0x18){ //if paddle is at top (top, mid-top, center)
					if(ballRow == 0x1E)	//ball at top
						state = MoveDownRight;
					else if(ballRow == 0x1D)	//ball at mid-top
						state = MoveStraightRight;
					else if(ballRow == 0x1B)	//ball at center
						state = MoveDownRight;
					else{ //PlayerOne loses (ball was not hit)
						StartGame = 0;
						state = WaitBall;
					}
				}
				else if(plOneRow == 0x11){	//if paddle is at center (mid-top, center, mid-bottom)
					if(ballRow == 0x1D)	//if ball at mid-top
						state = MoveUpRight;
					else if(ballRow == 0x1B)	//if ball at center
						state = MoveStraightRight;
					else if(ballRow == 0x17)	//if ball at mid-bottom
						state = MoveDownRight;
					else{ //PlayerOne loses (ball was not hit)
						StartGame = 0;
						state = WaitBall;
					}
				}
				else{	//if paddle is at bottom (center, mid-bottom, bottom)
					if(ballRow == 0x1B)	//ball at center
						state = MoveUpRight;
					else if(ballRow == 0x17)	//if ball at mid-bottom
						state = MoveStraightRight;
					else if(ballRow == 0x0F)	//if ball at bottom
						state = MoveUpRight;
					else{	//PlayerOne loses (ball was not hit)
						StartGame = 0;
						state = WaitBall;
					}
				}
			}
			else{
				state = MoveStraightLeft;
			}
			break;
		case MoveUpLeft:
			if(ballPattern == 0x40){        //need to bounce to right if hit
				if(plOneRow == 0x18){ //if paddle is at top (top, mid-top, center)
					if(ballRow == 0x1E)     //ball at top
                                                state = MoveDownRight;
                                        else if(ballRow == 0x1D)        //ball at mid-top
                                                state = MoveStraightRight;
                                        else if(ballRow == 0x1B)        //ball at center
                                                state = MoveDownRight;
                                        else{ //PlayerOne loses (ball was not hit)
                                                StartGame = 0;
                                                state = WaitBall;
                                        }
				}
				else if(plOneRow == 0x11){      //if paddle is at center (mid-top, center, mid-bottom)
                                        if(ballRow == 0x1D)     //if ball at mid-top
                                                state = MoveUpRight;
                                        else if(ballRow == 0x1B)        //if ball at center
                                                state = MoveStraightRight;
                                        else if(ballRow == 0x17)        //if ball at mid-bottom
                                                state = MoveDownRight;
                                        else{ //PlayerOne loses (ball was not hit)
                                                StartGame = 0;
                                                state = WaitBall;
                                        }
                                }
                                else{   //if paddle is at bottom (center, mid-bottom, bottom)
                                        if(ballRow == 0x1B)     //ball at center
                                                state = MoveUpRight;
                                        else if(ballRow == 0x17)        //if ball at mid-bottom
                                                state = MoveStraightRight;
                                        else if(ballRow == 0x0F)        //if ball at bottom
                                                state = MoveUpRight;
                                        else{   //PlayerOne loses (ball was not hit)
                                                StartGame = 0;
                                                state = WaitBall;
                                        }
                                }

                        }
                        else{   //keep moving left
                                if(ballRow == 0x1E) //if ball at top
                                        state = MoveDownLeft;
                                else
                                        state = MoveUpLeft;
                        }

			break;
		case MoveDownLeft:
			if(ballPattern == 0x40){        //need to bounce to left if hit
                                if(plOneRow == 0x18){ //if paddle is at top (top, mid-top, center)
                                        if(ballRow == 0x1E)     //ball at top
                                                state = MoveDownRight;
                                        else if(ballRow == 0x1D)        //ball at mid-top
                                                state = MoveStraightRight;
                                        else if(ballRow == 0x1B)        //ball at center
                                                state = MoveDownRight;
                                        else{ //Player One loses (ball was not hit)
                                                StartGame = 0;
                                                state = WaitBall;
                                        }
                                }
                                else if(plOneRow == 0x11){      //if paddle is at center (mid-top, center, mid-bottom)
                                        if(ballRow == 0x1D)     //if ball at mid-top
                                                state = MoveUpRight;
                                        else if(ballRow == 0x1B)        //if ball at center
                                                state = MoveStraightRight;
                                        else if(ballRow == 0x17)        //if ball at mid-bottom
                                                state = MoveDownRight;
                                        else{ //AI loses (ball was not hit)
                                                StartGame = 0;
                                                state = WaitBall;
                                        }
                                }
				else{   //if paddle is at bottom (center, mid-bottom, bottom)
                                        if(ballRow == 0x1B)     //ball at center
                                                state = MoveUpRight;
                                        else if(ballRow == 0x17)        //if ball at mid-bottom
                                                state = MoveStraightRight;
                                        else if(ballRow == 0x0F)        //if ball at bottom
                                                state = MoveUpRight;
                                        else{   //AI loses (ball was not hit)
                                                StartGame = 0;
                                                state = WaitBall;
                                        }
                                }


                        }
                        else{   //keep moving left
                                if(ballRow == 0x0F)
                                        state = MoveUpLeft;
                                else
                                        state = MoveDownLeft;
                        }                   

			break;
		case MoveStraightRight:
			if(ballPattern == 0x02){
				if(aiRow == 0x18){ //if paddle2 is at top (top, mid-top, center)
                                        if(ballRow == 0x1E)     //ball at top
                                                state = MoveDownLeft;
                                        else if(ballRow == 0x1D)        //ball at mid-top
                                                state = MoveStraightLeft;
                                        else if(ballRow == 0x1B)        //ball at center
                                                state = MoveDownLeft;
                                        else{ //PlayerOne loses (ball was not hit)
                                                StartGame = 0;
                                                state = WaitBall;
                                        }
                                }
                                else if(aiRow == 0x11){      //if paddle is at center (mid-top, center, mid-bottom)
                                        if(ballRow == 0x1D)     //if ball at mid-top
                                                state = MoveUpLeft;
                                        else if(ballRow == 0x1B)        //if ball at center
                                                state = MoveStraightLeft;
                                        else if(ballRow == 0x17)        //if ball at mid-bottom
                                                state = MoveDownLeft;
                                        else{ //PlayerOne loses (ball was not hit)
                                                StartGame = 0;
                                                state = WaitBall;
                                        }
                                }
				else{   //if paddle is at bottom (center, mid-bottom, bottom)
                                        if(ballRow == 0x1B)     //ball at center
                                                state = MoveUpLeft;
                                        else if(ballRow == 0x17)        //if ball at mid-bottom
                                                state = MoveStraightLeft;
                                        else if(ballRow == 0x0F)        //if ball at bottom
                                                state = MoveUpLeft;
                                        else{   //PlayerOne loses (ball was not hit)
                                                StartGame = 0;
                                                state = WaitBall;
                                        }
                                }
			}
			else{
				state = MoveStraightRight;
			}
			break;
		case MoveUpRight:
			if(ballPattern == 0x02){ 	//need to bounce to left if hit
				if(aiRow == 0x18){ //if paddle is at top (top, mid-top, center)
                                        if(ballRow == 0x1E)     //ball at top
                                                state = MoveDownLeft;
                                        else if(ballRow == 0x1D)        //ball at mid-top
                                                state = MoveStraightLeft;
                                        else if(ballRow == 0x1B)        //ball at center
                                                state = MoveDownLeft;
                                        else{ //AI loses (ball was not hit)
                                                StartGame = 0;
                                                state = WaitBall;
                                        }
                                }
                                else if(aiRow == 0x11){      //if paddle is at center (mid-top, center, mid-bottom)
                                        if(ballRow == 0x1D)     //if ball at mid-top
                                                state = MoveUpLeft;
                                        else if(ballRow == 0x1B)        //if ball at center
                                                state = MoveStraightLeft;
                                        else if(ballRow == 0x17)        //if ball at mid-bottom
                                                state = MoveDownLeft;
                                        else{ //AI loses (ball was not hit)
                                                StartGame = 0;
                                                state = WaitBall;
                                        }
                                }
				else{   //if paddle is at bottom (center, mid-bottom, bottom)
                                        if(ballRow == 0x1B)     //ball at center
                                                state = MoveUpLeft;
                                        else if(ballRow == 0x17)        //if ball at mid-bottom
                                                state = MoveStraightLeft;
                                        else if(ballRow == 0x0F)        //if ball at bottom
                                                state = MoveUpLeft;
                                        else{   //AI loses (ball was not hit)
                                                StartGame = 0;
                                                state = WaitBall;
                                        }
                                }

			}
			else{	//keep moving right
				if(ballRow == 0x1E) //if ball at top
					state = MoveDownRight;
				else
					state = MoveUpRight;
			}
			break;
		case MoveDownRight:
			if(ballPattern == 0x02){	//need to bounce to left if hit
				if(aiRow == 0x18){ //if paddle is at top (top, mid-top, center)
                                        if(ballRow == 0x1E)     //ball at top
                                                state = MoveDownLeft;
                                        else if(ballRow == 0x1D)        //ball at mid-top
                                                state = MoveStraightLeft;
                                        else if(ballRow == 0x1B)        //ball at center
                                                state = MoveDownLeft;
                                        else{ //AI loses (ball was not hit)
                                                StartGame = 0;
                                                state = WaitBall;
                                        }
                                }
                                else if(aiRow == 0x11){      //if paddle is at center (mid-top, center, mid-bottom)
                                        if(ballRow == 0x1D)     //if ball at mid-top
                                                state = MoveUpLeft;
                                        else if(ballRow == 0x1B)        //if ball at center
                                                state = MoveStraightLeft;
                                        else if(ballRow == 0x17)        //if ball at mid-bottom
                                                state = MoveDownLeft;
                                        else{ //AI loses (ball was not hit)
                                                StartGame = 0;
                                                state = WaitBall;
                                        }
                                }
				else{   //if paddle is at bottom (center, mid-bottom, bottom)
                                        if(ballRow == 0x1B)     //ball at center
                                                state = MoveUpLeft;
                                        else if(ballRow == 0x17)        //if ball at mid-bottom
                                                state = MoveStraightLeft;
                                        else if(ballRow == 0x0F)        //if ball at bottom
                                                state = MoveUpLeft;
                                        else{   //AI loses (ball was not hit)
                                                StartGame = 0;
                                                state = WaitBall;
                                        }
                                }


			}
			else{	//keep moving right
				if(ballRow == 0x0F)
					state = MoveUpRight;
				else
					state = MoveDownRight;
			}
			break;
		default:
			state = WaitBall;
			break;
	}
	switch(state){
		case WaitBall:
                        break;
                case MoveStraightLeft:
                        ballPattern <<= 1;
                        break;
                case MoveUpLeft:
			ballPattern <<= 1;
			ballRow = (ballRow >> 1) | 0x10;
                        break;
		case MoveDownLeft:
			ballPattern <<= 1;
			ballRow = (ballRow << 1) | 0x01;
			break;
                case MoveStraightRight:
			ballPattern >>= 1;
                        break;
                case MoveUpRight:
			ballPattern >>= 1;
			ballRow = (ballRow >> 1) | 0x10;
                        break;
		case MoveDownRight:
			ballPattern >>= 1;
			ballRow = (ballRow << 1) | 0x01;
			break;
                default:
                        break;
	}
	ballRow = ballRow & 0x1F;
	return state;
}

//============================================
// AI Player
//============================================

enum AI_States {Move};

int AI_SM(int state){
	if(StartGame == 1 && playTwo == 0){
		unsigned char num = rand() % 3; //Generate random number 0 to 3

		if(num == 0){ //moves 25% of the time
			if(ballRow == 0x1E && aiRow != 0x18){ //if ball is top
				aiRow = (aiRow >> 1) | 0x10;
			}
			else if(ballRow == 0x1D && aiRow != 0x18) //if ball is mid-top
				aiRow = (aiRow >> 1) | 0x10;
			else if(ballRow == 0x1B){ //if ball is center
				if(aiRow == 0x18) 		//if ai paddle is at top, move down
					aiRow = (aiRow << 1) | 0x01;
				else if(aiRow == 0x03) 	//if ai paddle is at bottom, move up
					aiRow = (aiRow >> 1) | 0x10;
			}
			else if(ballRow == 0x17 && aiRow != 0x03){ //if ball is mid-bottom
				aiRow = (aiRow << 1) | 0x01;
			}
			else if(ballRow == 0x0F && aiRow != 0x03) //if ball is bottom
				aiRow = (aiRow << 1) | 0x01;
		}
	}	
	
	aiRow = aiRow & 0x1F;
	return Move;
}

//============================================
//Player2
//============================================

enum p2States {Wait2, Up2, Down2};

void A2D_init() {
      ADCSRA |= (1 << ADEN) | (1 << ADSC) | (1 << ADATE);
	// ADEN: Enables analog-to-digital conversion
	// ADSC: Starts analog-to-digital conversion
	// ADATE: Enables auto-triggering, allowing for constant
	//	    analog to digital conversions.
}

// Pins on PORTA are used as input for A2D conversion
// The default channel is 0 (PA0)
// The value of pinNum determines the pin on PORTA
// used for A2D conversion
// Valid values range between 0 and 7, where the value
// represents the desired pin for A2D conversion
void Set_A2D_Pin(unsigned char pinNum) {
	ADMUX = (pinNum <= 0x07) ? pinNum : ADMUX;
	// Allow channel to stabilize
	static unsigned char i = 0;
	for ( i=0; i<15; i++ ) { asm("nop"); }
}

unsigned short joystickRest = 500;

int playTwoSM(int state){
	Set_A2D_Pin(0x02);
	unsigned short joystick = ADC;
	unsigned char upButton2 = ~PINA & 0x01;
	unsigned char downButton2 = ~PINA & 0x02;

		

	if(playTwo == 1){
		switch(state){
               	        case Wait:
                                /*if(joystick <= (520 + 50) && joystick >= (520 - 50))
                                        state = Wait;
                                else if(joystick > (520 + 50)){
                                        state = Up;
                                        if(aiRow  == 0x03 || aiRow == 0x11)
                                                aiRow = ((aiRow >> 1) | 0x10) & 0x1F;
                                }
                                else{
                                        state = Down;
                                        if(aiRow == 0x18 || aiRow == 0x11)
                                                aiRow = ((aiRow << 1) | 0x01) & 0x1F;
                                }*/
				if(!upButton2 && !downButton2)
                                        state = Wait;
                                else if(upButton2 && !downButton2){
                                        state = Up;
                                        if(aiRow  == 0x03 || aiRow == 0x11)
                                                aiRow = ((aiRow >> 1) | 0x10) & 0x1F;
                                }
                                else{
                                        state = Down;
                                        if(aiRow == 0x18 || aiRow == 0x11)
                                                aiRow = ((aiRow << 1) | 0x01) & 0x1F;
                                }
                                break;
                        case Up:
                                if(upButton2)
                                        state = Up;
                                else
                                        state = Wait;
                                break;
                        case Down:
                                if(downButton2)
                                        state = Down;
                                else
                                        state = Wait;
                                break;
                        default:
                                state = Wait;
                                break;
		}
	}
	return state;
}

//============================================
//Display
//============================================

enum displayStates {displayP1, displayBall, displayAI};

int  display(int state){
	switch(state){
		case displayP1:
			PORTC = plOnePattern;
			PORTD = plOneRow;
			state = displayBall;
			break;
		case displayBall:
			PORTC = ballPattern;
			PORTD = ballRow;
			state = displayAI;
			break;
		case displayAI:
			PORTC = aiPattern;
			PORTD = aiRow;
			state = displayP1;
			break;
		default:
			state = displayP1;
			break;
	}
	return state;
}

int main(void) {
    /* Insert DDR and PORT initializations */
	DDRA = 0x00; PORTA = 0xFF;
	DDRB = 0x00; PORTB = 0xFF;
	DDRC = 0xFF; PORTC = 0x00;
	DDRD = 0xFF; PORTD = 0x00;

    /* Insert your solution below */
	static task task1, task2, task3, task4, task5, task6, task7;
	task *tasks[] = { &task1, &task2, &task3 , &task4, &task5, &task6, &task7};
	const unsigned short numTasks = sizeof(tasks)/sizeof(task*);

	time_t t;
        srand((unsigned) time(&t));     //Initializes random number generator (reference from tutorialspoint.com)
	
	A2D_init();
	Set_A2D_Pin(0x02);
	joystickRest = ADC;

	const char start = -1;
	//Task 1 (StartGame)
	task1.state = start;
	task1.period = 50;
	task1.elapsedTime = task1.period;
	task1.TickFct = &gameStartSM;
	//Task 2 (pickPlTwoSM)
        task2.state = start;
        task2.period = 200;
        task2.elapsedTime = task2.period;
        task2.TickFct = &pickPlTwoSM;
	//Task 3 (PlayerOne)
	task3.state = start;
	task3.period = 200;
	task3.elapsedTime = task3.period;
	task3.TickFct = &movePlayer;
	//Task 4 (BallSM)
        task4.state = start;
        task4.period = 800;
        task4.elapsedTime = task4.period;
        task4.TickFct = &ballSM;
	//Task 5 (AI_SM)
        task5.state = start;
        task5.period = 200;
        task5.elapsedTime = task5.period;
        task5.TickFct = &AI_SM;
	//Task 6 (playTwoSM)
        task6.state = start;
        task6.period = 200;
        task6.elapsedTime = task6.period;
        task6.TickFct = &playTwoSM;
	//Task 7 (Display)
        task7.state = start;
        task7.period = 50;
        task7.elapsedTime = task7.period;
        task7.TickFct = &display;


	//find GCD of all periods
	unsigned long GCD = tasks[0]->period;
	for(int g=1; g<numTasks; ++g){
		GCD = findGCD(GCD, tasks[g]->period);
	}
	

	TimerSet(GCD);
	TimerOn();

	unsigned short i;

	while (1) {
		for(i=0; i<numTasks; i++){
			if( tasks[i]->elapsedTime == tasks[i]->period ) {
				tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
				tasks[i]->elapsedTime = 0;
			}
			tasks[i]->elapsedTime += GCD;
		}

		TimerSet(GCD);
		while(!TimerFlag);
		TimerFlag = 0;

	}
    
	return 1;
}
