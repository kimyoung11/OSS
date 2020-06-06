/* tetris-term - Classic Tetris for your terminal.
 *
 * Copyright (C) 2014 Gjum
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
//tesetiT

//#include <signal.h>
//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
//#include <time.h>
//#include <unistd.h>
//
//#include "tetris.h"
#include "print.c"

void SignalHandler(int signal) { //signal process{{{

	switch(signal) {
		case SIGINT:
		case SIGSEGV:
			game->isRunning = 0;
			break;
		
		case SIGALRM:
			Tick(game);
			game->timer.it_value.tv_usec = game->sleepUsec;
			setitimer(ITIMER_REAL, &game->timer, NULL);
			break;
	}

	return;
}

int main(int argc, char **argv) { 
	
	srand(time(SEED_VALUE));
	
	Welcome();
	
	game = NewTetrisGame(BOARD_WIDTH,BOARD_HEIGHT);
	// create space for the board
	for (int i = 0; i < game->height + 2; i++) printf("\n");
	
	PrintBoard(game);
	
	while (game->isRunning) {
		usleep(50000);
		ProcessInputs(game);
	}

	DestroyTetrisGame(game);
		
}

