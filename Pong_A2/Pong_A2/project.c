/*
 * project.c
 *
 * Main file
 *
 * Authors: Peter Sutton, Luke Kamols, Jarrod Bennett, Cody Burnett
 * Modified by <YOUR NAME HERE>
 */

#include <stdio.h>
#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#define F_CPU 8000000UL
#include <util/delay.h>

#include "game.h"
#include "display.h"
#include "ledmatrix.h"
#include "buttons.h"
#include "serialio.h"
#include "terminalio.h"
#include "timer0.h"

// Function prototypes - these are defined below (after main()) in the order
// given here
void initialise_hardware(void);
void start_screen(void);
void new_game(void);
void play_game(void);
void handle_game_over(void);

#define ESCAPE_CHAR 27

/////////////////////////////// main //////////////////////////////////
int main(void) {
	// Setup hardware and call backs. This will turn on 
	// interrupts.
	initialise_hardware();
	
	// Show the splash screen message. Returns when display
	// is complete.
	start_screen();
	
	// Loop forever and continuously play the game.
	while(1) {
		new_game();
		play_game();
		handle_game_over();
	}
}

void initialise_hardware(void) {
	ledmatrix_setup();
	init_button_interrupts();
	// Setup serial port for 19200 baud communication with no echo
	// of incoming characters
	init_serial_stdio(19200, 0);
	
	init_timer0();
	
	// Turn on global interrupts
	sei();
}

void start_screen(void) {
	// Start Screen (Level 1: 4 marks)
	// Clear terminal screen and output a message
	clear_terminal();
	show_cursor();
	move_terminal_cursor(10,10);
	printf_P(PSTR("PONG"));
	move_terminal_cursor(10,12);
	printf_P(PSTR("CSSE2010/7201 A2 by NAM Hiu Yi - 46604563"));
	
	// Output the static start screen and wait for a push button 
	// to be pushed or a serial input of 's'
	show_start_screen();

	uint32_t last_screen_update, current_time;
	last_screen_update = get_current_time();
	
	uint8_t frame_number = 0;

	// Wait until a button is pressed, or 's' is pressed on the terminal
	while(1) {
		// First check for if a 's' is pressed
		// There are two steps to this
		// 1) collect any serial input (if available)
		// 2) check if the input is equal to the character 's'
		char serial_input = -1;
		if (serial_input_available()) {
			serial_input = fgetc(stdin);
		}
		// If the serial input is 's', then exit the start screen
		if (serial_input == 's' || serial_input == 'S') {
			break;
		}
		// Next check for any button presses
		int8_t btn = button_pushed();
		if (btn != NO_BUTTON_PUSHED) {
			break;
		}

		current_time = get_current_time();
		if (current_time - last_screen_update > 500) {
			update_start_screen(frame_number);
			frame_number = (frame_number + 1) % 12;
			last_screen_update = current_time;
		}
	}
}

void new_game(void) {
	// Clear the serial terminal
	clear_terminal();
	
	// Initialise the game and display
	initialise_game();
	
	// Clear a button push or serial input if any are waiting
	// (The cast to void means the return value is ignored.)
	(void)button_pushed();
	clear_serial_input_buffer();
}

void play_game(void) {
	
	uint32_t last_ball_move_time, current_time;
	uint8_t btn; // The button pushed
	
	char escape_sequence_char;
	uint8_t chars_in_escape_sequence = 0;
	
	last_ball_move_time = get_current_time();
	
	// We play the game until it's over
	while (!is_game_over()) {
				
		// We need to check if any button has been pushed, this will be
		// NO_BUTTON_PUSHED if no button has been pushed
		// Checkout the function comment in `buttons.h` and the implementation
		// in `buttons.c`.
		btn = button_pushed();
		
		char serial_input = 1;
		escape_sequence_char = -1;
		
		if (serial_input_available()) {
			serial_input = fgetc(stdin);
			if (chars_in_escape_sequence == 0 && serial_input == ESCAPE_CHAR) {
				chars_in_escape_sequence ++;
				serial_input = -1;
				} else if (chars_in_escape_sequence == 1 && serial_input == '[') {
				chars_in_escape_sequence ++;
				serial_input = -1;
				} else if (chars_in_escape_sequence == 2) {
				escape_sequence_char = serial_input;
				serial_input = -1;
				chars_in_escape_sequence = 0;
				} else {
				chars_in_escape_sequence = 0;
			}
		}

		// Move Paddles with Buttons (Level 1: 8 marks)
		// Move Paddles with Terminal Input (Level 1: 4 marks)
		if (serial_input == 'w' || serial_input == 'W' || btn == BUTTON3_PUSHED) {
			// If button 3 is pushed, move player 1 one space up
			// YOU WILL NEED TO IMPLEMENT THIS FUNCTION
			move_player_paddle(PLAYER_1, UP);
		} else if (serial_input == 's' || serial_input == 'S' || 
			serial_input == 'd' || serial_input == 'D' || btn == BUTTON2_PUSHED) {
			// If button 2 is pushed, move player 1 one space down
			move_player_paddle(PLAYER_1, DOWN);
		} else if (serial_input == 'o' || serial_input == 'O' || btn == BUTTON1_PUSHED) {
			// If button 1 is pushed, mover player 2 one space up
			move_player_paddle(PLAYER_2, UP);
		} else if (serial_input == 'k' || serial_input == 'K' ||
			serial_input == 'l' || serial_input == 'L' || btn == BUTTON0_PUSHED) {
			// If button 0 is pushed, move player 2 one space down
			move_player_paddle(PLAYER_2, DOWN);
		}
		
		current_time = get_current_time();
		if (current_time >= last_ball_move_time + 500) {
			// 500ms (0.5 second) has passed since the last time we move the
			// ball, so update the position of the ball based on current x
			// direction and y direction
			update_ball_position();
			
			// Update the most recent time the ball was moved
			last_ball_move_time = current_time;
		}
	}
	// We get here if the game is over.
}

void handle_game_over() {
	move_terminal_cursor(10,14);
	printf_P(PSTR("GAME OVER"));
	move_terminal_cursor(10,15);
	printf_P(PSTR("Press a button or 's'/'S' to start a new game"));
	
	// Do nothing until a button is pushed. Hint: 's'/'S' should also start a
	// new game
	while (button_pushed() == NO_BUTTON_PUSHED) {
		; // wait
	}
}
