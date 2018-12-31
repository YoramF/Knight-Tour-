/*
 * KnightMove.c
 *
 *  Created on: Dec 26, 2018
 *      Author: yoram
 *
 *  This verion uses 'Warnsdorf's rule' to move on the board
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include "../nfrmt/nfrmt.h"

#define MAXSIZE 31
#define MAXKNIGHTMOVES 8

typedef struct
{
	int raw;
	int col;
} square;

typedef struct _se
{
	square sq;
	struct _se *next;
} stackE;

int *board;
square allMoves [MAXKNIGHTMOVES] = {{-2,1},{-1, 2},{1,2},{2,1},{2,-1},{1,-2},{-1,-2},{-2,-1}};
stackE *stack = NULL;
int boardSize = 8;			// default size
int freeCells = 8 * 8;
long long int moveAttempts = 0;

#ifdef DEBUG
int loging = 0;
#endif

// print out the board
void printBoard (int *board)
{
	int i, j;

	for (i = 0; i < boardSize; i++)
	{
		for (j = 0; j < boardSize; j++)
			printf("%4d", board[i*boardSize+j]);
		printf("\n");
	}
}


// push elemet ontop of stack
stackE *push (stackE *stack, const square *sq)
{
	stackE *st;

	if ((st = malloc(sizeof(stackE))) == NULL)
	{
		printf("fatal error = failed to allocate RAM\n");
		return NULL;
	}

	memcpy(&(st->sq), sq, sizeof(square));

	st->next = stack;

	return st;
}

// pop an element from topof stack
stackE *pop(stackE *stack, square *sq)
{
	stackE *s = stack;

	if (s != NULL)
	{
		memcpy(sq, &(s->sq), sizeof(square));
		stack = s->next;
		free(s);
	}

	return stack;
}

// get all valid neighbors for a given cell
// *ind must be greater than 0 and smaller than SIZE
int nextFreeNeighbour(int *board, int r, int c, int *nr, int *nc, int *ind)
{
	int tr, tc, i = *ind;

	while (i < MAXKNIGHTMOVES)
	{
		tr = r + allMoves[i].raw;
		tc = c + allMoves[i].col;
		i++;

		if ((tr >= 0) && (tr < boardSize) && (tc >= 0) && (tc < boardSize))
		{
			if (board[tr*boardSize+tc] >= 0)
			{
				*nr = tr;
				*nc = tc;
				*ind = i;
				return 1;
			}
		}
	}
	return 0;
}


// get sorted list of all possible moves from a given location
// return nuber of moves. O if no possible move
int getNextMoves(int *board, square moves[MAXKNIGHTMOVES], int r, int c)
{
	int m, i, j, tr, tc;
	int scores [MAXKNIGHTMOVES];
	int k, t;
	square sq;

	// first get all possible moves
	m = 0;
	i = 0;
	while (nextFreeNeighbour(board, r, c, &tr, &tc, &i))
	{
				moves[m].raw = tr;
				moves[m].col = tc;
				scores[m] = board[tr*boardSize+tc];
				m++;
	}

	// sort the moves according to their scores
	for (i = 0; i < m - 1; i++)
	{
		k = i;
		for (j = i+1; j < m; j++)
			if (scores[k] > scores[j])
				k = j;
		if (k > i)
		{
			t = scores[k];
			sq = moves[k];
			scores[k] = scores[i];
			moves[k] = moves[i];
			scores[i] = t;
			moves[i] = sq;
		}
	}

	return m;
}

int calculate (int *board, int r, int c)
{
	int m = 0;
	int ind = 0;
	int tr, tc;

	while (nextFreeNeighbour(board, r, c, &tr, &tc, &ind))
		m++;

	return m;
}

void initBoard (int *board, int r, int c)
{
	int i,j;

	for (i = 0; i < boardSize; i++)
		for (j = 0; j < boardSize; j++)
			board[i*boardSize+j] = 1;

	board[r*boardSize+c] = -1;
	freeCells--;

	// calculate next move score for all board
	for (i = 0; i < boardSize; i++)
		for (j = 0; j < boardSize; j++)
		{
			if (board[i*boardSize+j] > 0)
				board[i*boardSize+j] = calculate(board, i, j);
		}

	printBoard(board);
}

// try moving the knight to row r column c. If target location is free,
// mark it as occupied and return 1. Otherwise return 0
// update v with current cell value. We will need it in case of canceling this move
int moveTo(int *board, int r, int c, int *v)
{
	char str [30];
	time_t t;
	int i, tc, tr;

	moveAttempts++;
	if (!(moveAttempts % 1000000000))
	{
		t = time(NULL);
		printf("Moves-> %s - %s", frmt(str, 30, "20", INT, &moveAttempts), asctime(localtime(&t)));
	}

#ifdef DEBUG
	if ((loging == 2) && (!(moveAttempts % 1000000)))
	{
		printf("\n");
		printBoard(board);
	}
#endif

	if (board[r*boardSize+c] >= 0)
	{
		*v = board[r*boardSize+c];
		board[r*boardSize+c] = -1;
		freeCells--;

#ifdef DEBUG
		if (loging == 1)
			printf("Move to [%d,%d]\n", r, c);
#endif

		// decrease score of all neighbours
		i = 0;
		while (nextFreeNeighbour(board, r, c, &tr, &tc, &i))
				board[tr*boardSize+tc]--;

		return 1;
	}
	else
		return 0;
}

// reverse a move
void unMoveTo(int *board, int r, int c, int v)
{
	int i, tc, tr;

#ifdef DEBUG
	if (loging == 1)
		printf("Move back from [%d,%d]\n", r, c);
#endif

	board[r*boardSize+c] = v;
	freeCells++;

	// increase score for all neigbors
	i = 0;
	while (nextFreeNeighbour(board, r, c, &tr, &tc, &i))
		board[tr*boardSize+tc]++;
}


// solve the board
// return 1 if all cells were filled
// return 0 if failed to fill all cells and there is no more available moves
int solveBoard(int *board, int raw, int col)
{
	int i = 0, r, c, s, v, m;
	square sq, moves[MAXKNIGHTMOVES];

	// check if we managed to fill the board
	if (freeCells == 0)
		return 1;

	s = 0;
	m = getNextMoves(board, moves, raw, col);

	for (i = 0; i < m; i++)
	{
		r = moves[i].raw;
		c = moves[i].col;

		if (moveTo(board, r, c, &v))
		{
			if (solveBoard(board, r, c))
			{
				// push current location
				sq.raw = r;
				sq.col = c;
				stack = push(stack, &sq);
				s = 1;
				break;
			}
			unMoveTo(board, r, c, v);
		}
	}

	return s;
}

int main (int argc, char *argv[])
{
	char opt;
	int raw = -1, col = -1, i = 0;
	square sq;
	char str [30];
	time_t t;

	srand((unsigned)time(&t));

	// get commad line input
	if (argc > 1)
	{
#ifdef DEBUG
		while ((opt = getopt(argc, argv, "s:r:c:l:")) > 0)
#else
		while ((opt = getopt(argc, argv, "s:r:c:")) > 0)
#endif
			switch (opt)
			{
			case 'r':
				raw = atoi(optarg);
				break;
			case 'c':
				col = atoi(optarg);
				break;
#ifdef DEBUG
			case 'l':
				loging = atoi(optarg);
				break;
#endif
			case 's':
				boardSize = atoi(optarg);
				boardSize = (boardSize < MAXSIZE? boardSize: MAXSIZE);
				freeCells = boardSize * boardSize;
				break;
			default:
				printf("wrong input\n");
				return -1;
			}
		}

	// check if we need to generate raw and col
	if (col < 0)
		col = rand()%boardSize;
	if (raw < 0)
		raw = rand()%boardSize;

	if ((board = malloc(boardSize*boardSize*sizeof(int))) == NULL)
	{
		printf("failed to allocate memory for board\n");
		return -1;
	}

	initBoard(board, raw, col);

	if (solveBoard(board, raw, col))
	{
		board[raw*boardSize+col] = ++i;
		while(stack != NULL)
		{
			stack = pop(stack, &sq);
			printf("[%d] moved from [%d,%d] to [%d,%d]\n", ++i, raw, col, sq.raw, sq.col);
			raw = sq.raw;
			col = sq.col;
			board[raw*boardSize+col] = i;
		}
		printf("Board====>>>\n");
		printBoard(board);
	}
	else
		printf("failed to solve board\n");

	printf("Total Move attempts: %s\n", frmt(str, 30, "20", INT, &moveAttempts));

	return 1;
}
