/*
 * game.c
 *
 * Functionality related to the game state and features.
 *
 * Author: Jarrod Bennett, Cody Burnett
 */ 

#include "game.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "display.h"
#include "terminalio.h"

// Player paddle positions. y coordinate refers to lower pixel on paddle.
// x coordinates never change but are nice to have here to use when drawing to
// the display.
static const int8_t PLAYER_X_COORDINATES[] = {PLAYER_1_X, PLAYER_2_X};
static int8_t player_y_coordinates[] = {0, 0};

// Ball position
int8_t ball_x;
int8_t ball_y;

// Ball direction
int8_t ball_x_direction;
int8_t ball_y_direction;

void draw_player_paddle(uint8_t player_to_draw);
void erase_player_paddle(uint8_t player_to_draw);

// Initialise the player paddles, ball and display to start a game of PONG.
void initialise_game(void) {
	
	// initialise the display we are using.
	initialise_display();

	// Start players in the middle of the board
	player_y_coordinates[PLAYER_1] = BOARD_HEIGHT / 2 - 1;
	player_y_coordinates[PLAYER_2] = BOARD_HEIGHT / 2 - 1;

	draw_player_paddle(PLAYER_1);
	draw_player_paddle(PLAYER_2);

	// Clear the old ball
	update_square_colour(ball_x, ball_y, EMPTY_SQUARE);
	
	// Reset ball position and direction
	ball_x = BALL_START_X;
	ball_y = BALL_START_Y;

	ball_x_direction = LEFT;
	ball_y_direction = UP;
	
	// Draw new ball
	update_square_colour(ball_x, ball_y, BALL);
}

// Draw player 1 or 2 on the game board at their current position (specified
// by the `PLAYER_X_COORDINATES` and `player_y_coordinates` variables).
// This makes it easier to draw the multiple pixels of the players.
void draw_player_paddle(uint8_t player_to_draw) {
	int8_t player_x = PLAYER_X_COORDINATES[player_to_draw];
	int8_t player_y = player_y_coordinates[player_to_draw];

	for (int y = player_y; y < player_y + PLAYER_HEIGHT; y++) {
		update_square_colour(player_x, y, PLAYER);
	}
}

// Erase the pixels of player 1 or 2 from the display.
void erase_player_paddle(uint8_t player_to_draw) {
	int8_t player_x = PLAYER_X_COORDINATES[player_to_draw];
	int8_t player_y = player_y_coordinates[player_to_draw];

	for (int y = player_y; y < player_y + PLAYER_HEIGHT; y++) {
		update_square_colour(player_x, y, EMPTY_SQUARE);
	}
}

// Try and move the selected player's y coordinate by the amount specified.
// For example, to move player 1's paddle up one space, call the function
// as `move_player(PLAYER_1, 1)`. Use `-1` instead to move the paddle down. No
// pixels of the player paddles should be allowed to move off the display.
void move_player_paddle(int8_t player, int8_t direction) {
	/* suggestions for implementation:
	 * 1: Figure out the new location of the player paddle. Consider what should
	 *	  happen if the player paddle is at the edge of the board.
	 * 2: Remove the player from the display at the current position.
	 * 3: Update the positional knowledge of the player. This will involve the
	 *    player coordinate variables.
	 * 4: Display the player at their new position.
	 */	
	// YOUR CODE HERE
	int8_t player_y = player_y_coordinates[player];
	
	// Erase the old paddle
	erase_player_paddle(player);
	
	// Calculate the new y-coordinate for the player y paddle 
	int8_t new_player_y = player_y + direction;
	
	// Check if the paddle will move off the display
	if (new_player_y >= 0 && new_player_y + PLAYER_HEIGHT <= BOARD_HEIGHT) {
		// Check if the new paddle position is in the same column as the ball
		if (ball_x == PLAYER_X_COORDINATES[player]) {
			// Adjust the new position based on the ball's position
			if (direction > 0) {
				if (ball_y <= new_player_y + PLAYER_HEIGHT - 1) {
					new_player_y = ball_y - PLAYER_HEIGHT + 1;
				}
			} else if (direction < 0) {
				if (ball_y >= new_player_y) {
					new_player_y = ball_y + 1;
				}
			}
		}
		// Update the player's y-coordinate
		player_y_coordinates[player] = new_player_y;
	} else if (new_player_y < 0) {
		// Paddle reached the upper bound, set it to 0
		player_y_coordinates[player] = 0;
	} else {
		// Paddle reached the lower bound, set it to the maximum valid position
		player_y_coordinates[player] = BOARD_HEIGHT - PLAYER_HEIGHT;
	}
	// Draw the new paddle
	draw_player_paddle(player);
}

// Update ball position based on current x direction and y direction of ball
void update_ball_position(void) {
	
	// Determine new ball coordinates
	int8_t new_ball_x = ball_x + ball_x_direction;
	int8_t new_ball_y = ball_y + ball_y_direction;
	
	// Check for collision with top wall
	if (new_ball_y < 0) {
		// Invert vertical direction
		ball_y_direction = -ball_y_direction;
		// Set ball position at the top wall
		new_ball_y = 1;
	}
	
	// Check for collision with bottom wall
	if (new_ball_y > BOARD_HEIGHT - 1) {
		// Invert vertical direction
		ball_y_direction = -ball_y_direction;
		// Set ball position at the bottom wall
		new_ball_y = BOARD_HEIGHT - 2;
	}
	
	// Check for collision with left wall
	if (new_ball_x < 0) {
		// Reset ball position to (5, 4)
		new_ball_x = 5;
		new_ball_y = 4;
	}
	
	// Check for collision with right wall 
	if (new_ball_x >= BOARD_WIDTH) {
		// Reset ball position to (5, 4)
		new_ball_x = 5;
		new_ball_y = 4;
	}
	
	// Check for collision with player paddles 
	if (new_ball_x == PLAYER_1_X + 1 && ball_x_direction < 0) {
		// Ball is approaching player 1 paddle from the right
		if (new_ball_y >= player_y_coordinates[PLAYER_1] && new_ball_y < player_y_coordinates[PLAYER_1] + PLAYER_HEIGHT) {
			// Ball collides with player 1 paddle
			if (ball_y_direction < 0) {
				ball_x_direction = -ball_x_direction;	// Ball is moving up, invert horizontal direction 			
			} else if (ball_y_direction > 0) {
				// Ball is moving down, determine if it should bounce or pass
				int8_t paddle_collision_point = player_y_coordinates[PLAYER_1] + PLAYER_HEIGHT - 1;
				if (new_ball_y > paddle_collision_point) {
					// Ball passes below the paddle, no bounce
				} else {
					// Ball bounce off the paddle, invert horizontal direction 
					ball_x_direction = -ball_x_direction;
				}
			}
		}
	} else if (new_ball_x == PLAYER_2_X - 1 && ball_x_direction > 0) {
		// Ball is approaching player 2 paddle from the left
		if (new_ball_y >= player_y_coordinates[PLAYER_2] && new_ball_y < player_y_coordinates[PLAYER_2] + PLAYER_HEIGHT) {
			// Ball collides with player 2 paddle
			if (ball_y_direction < 0) {
				ball_x_direction = -ball_x_direction;	// Ball is moving up, invert horizontal direction
			} else if (ball_y_direction > 0) {
				// Ball is moving down, determine bounce or pass
				int8_t paddle_collision_point = player_y_coordinates[PLAYER_2] + PLAYER_HEIGHT - 1;
				if (new_ball_y > paddle_collision_point) {
					// Ball passes below the paddle, no bounce
				} else {
					// Ball bounce off the paddle, invert horizontal direction 
					ball_x_direction = -ball_x_direction;
				}
			}
		}
	}
	
	// Erase old ball
	update_square_colour(ball_x, ball_y, EMPTY_SQUARE);
	
	// Assign new ball coordinates
	ball_x = new_ball_x;
	ball_y = new_ball_y;
	
	// Draw new ball
	update_square_colour(ball_x, ball_y, BALL);
}

// Returns 1 if the game is over, 0 otherwise.
uint8_t is_game_over(void) {
	// YOUR CODE HERE
	// Detect if the game is over i.e. if a player has won.
	return 0;
}
