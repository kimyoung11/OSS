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

#include "tetris.h"
#include "brick.c"
//#include <signal.h>
//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
//#include <unistd.h>



static void DieIfOutOfMemory(void *pointer) { // memory check{{{
	if (pointer == NULL) {
		printf("Error: Out of memory\n");
		exit(1);
	}
}

TetrisGame *NewTetrisGame(unsigned int width, unsigned int height) { // tetris information{{{
	TetrisGame *game = malloc(sizeof(TetrisGame));
	DieIfOutOfMemory(game);
	game->width = width;
	game->height = height;
	game->size = width * height;
	game->board = calloc(game->size, sizeof(char));
	DieIfOutOfMemory(game->board);
	game->isRunning = 1;
	game->isPaused  = 0;
	game->sleepUsec = 500000;
	game->score = 0;
	NextBrick(game); // fill preview
	NextBrick(game); // put into game
	// init terminal for non-blocking and no-echo getchar()
	struct termios term;
	tcgetattr(STDIN_FILENO, &game->termOrig);
	tcgetattr(STDIN_FILENO, &term);
	term.c_lflag &= ~(ICANON|ECHO);
	term.c_cc[VTIME] = 0;
	term.c_cc[VMIN] = 0;
	tcsetattr(STDIN_FILENO, TCSANOW, &term);
	// init signals for timer and errors
	struct sigaction signalAction;
	sigemptyset(&signalAction.sa_mask);
	signalAction.sa_handler = SignalHandler;
	signalAction.sa_flags = 0;
	sigaction(SIGINT,  &signalAction, NULL);
	sigaction(SIGTERM, &signalAction, NULL);
	sigaction(SIGSEGV, &signalAction, NULL);
	sigaction(SIGALRM, &signalAction, NULL);
	// init timer
	game->timer.it_value.tv_usec = game->sleepUsec;
	setitimer(ITIMER_REAL, &game->timer, NULL);
	return game;
}

void DestroyTetrisGame(TetrisGame *game) { // end game{{{
	if (game == NULL) return;
	tcsetattr(STDIN_FILENO, TCSANOW, &game->termOrig);
	printf("Your score: %li\n", game->score);
	printf("Game over.\n");
	free(game->board);
	free(game);
} 

unsigned char ColorOfBrickAt(FallingBrick *brick, int x, int y) { // set brick color
	if (brick->type < 0) return 0;
	int v = y - brick->y;
	if (v < 0 || v >= 4) return 0;
	int u = x - brick->x;
	if (u < 0 || u >= 4) return 0;
	for (int i = 0; i < 4; i++) {
		if (u + 4 * v == bricks[brick->type][brick->rotation][i])
			return brick->color;
	}
	return 0;
}

void Tick(TetrisGame *game) { // timer tick{{{
	if (game->isPaused) return;
	game->brick.y++;
	if (BrickCollides(game)) {
		game->brick.y--;
		LandBrick(game);
		ClearFullRows(game);
		NextBrick(game);
		if (BrickCollides(game))
			game->isRunning = 0;
	}
	PrintBoard(game);
} // }}}

static void PauseUnpause(TetrisGame *game) { // check pause unpause {{{ 
	if (game->isPaused) {
		// TODO de-/reactivate timer
		Tick(game);
	}
	game->isPaused ^= 1;
} // }}}

void ProcessInputs(TetrisGame *game) { // input process{{{
	char c = getchar();
	do {
		switch (c) {
			case ' ': MoveBrick(game, 0, 1); break;
			//case '?': DropBrick(game); break;
			case 'p': PauseUnpause(game); break;
			case 'q': game->isRunning = 0; break;
			case 27: // ESC
				getchar();
				switch (getchar()) {
					case 'A': RotateBrick(game,  1);  break; // up
					case 'B': RotateBrick(game, -1);  break; // down
					case 'C': MoveBrick(game,  1, 0); break; // right
					case 'D': MoveBrick(game, -1, 0); break; // left
				}
				break;
		}
	} while ((c = getchar()) != -1);
} // }}}

