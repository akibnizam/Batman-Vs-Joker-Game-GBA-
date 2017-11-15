#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "myLib.h"
#include "my_images.h"

u16* videoBuffer = (u16*)0x6000000;

#define REG_DISPCTL *(unsigned short *)0x4000000
#define MODE3 3

//#define SCANLINECOUNTER *(volatile unsigned short *) 0x4000006

#define SCREENHEIGHT 149

#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define ABS(x) ((x) < 0 ? -(x) : (x))



extern const unsigned char fontdata_6x8[12288];



#define KEY_DOWN_NOW(key)  (~(BUTTONS) & key)


int gameStat = 0;




//unsigned short *videoBuffer = (unsigned short *)0x6000000;

// Prototypes


int main()
{
	REG_DISPCTL = MODE3 | BG2_ENABLE;
	
	bool drawn = false;
	
	while (1) {
		
		if (gameStat == 0)
		{	
			if (!drawn) {
				drawImage3(0, 0, START_WIDTH, START_HEIGHT, start);
				drawString(90, 90, "Start Game", YELLOW);
				drawString(100, 85, "PRESS ENTER", YELLOW);
				drawn = true;

			}
			
			if(KEY_DOWN_NOW(BUTTON_START)) {
				
				drawRect(0, 0, 400, 300, BLACK);
				gameStat = 1;
				drawn = false;
			}

		} else if (gameStat == 1) {
			game();
		} else if (gameStat == 2) {
			done(true, &drawn);
		} else if (gameStat == 3) {
			done(false, &drawn);
		}
	}





}

void setPixel(int row, int col, unsigned short color, int width)
{
	
	DMA[3].src = &color;
    DMA[3].dst = (videoBuffer + (col +( (row) * 240 ) ));
    DMA[3].cnt = width | DMA_SOURCE_FIXED | DMA_DESTINATION_INCREMENT | DMA_ON;
}

void drawRect(int row, int col, int height, int width, unsigned short color)
{
	for(int r=0; r<height; r++)
	{
		setPixel(row+r, col, color, width);
		
	}
}


void delay(int n)
{
	volatile int x = 0;
	for(int i=0; i<n*5000; i++)
	{
		x = x + 1;
	}
}

void waitForVblank()
{
	while(SCANLINECOUNTER > 160);
	while(SCANLINECOUNTER < 160);
}

// void drawChar(int row, int col, char ch, unsigned short color)
// {
// 	for(int r = 0; r< 8; r++)
// 	{
// 		for(int c=0; c< 6; c++)
// 		{
// 			if( fontdata_6x8[OFFSET(r, c, 6) + ch*48] == 1)
// 			{
// 				setPixel(r+row, col + c, color, 6);
// 			}
// 		}

// 	}
// }

// void drawString(int row, int col, char str[], unsigned short color)
// {
// 	while(*str)
// 	{
// 		drawChar(row, col, *str++, color);
// 		col += 6;
// 	}
// }


void drawImage3(int row, int col, int width, int height, const u16* image) {
    for (int r = 0; r < height; r++) {
    	DMA[1].src = (image + width * r );

    	DMA[1].dst = (videoBuffer + (col +( (row + r) * 240 ) ));
        DMA[1].cnt = width | DMA_SOURCE_INCREMENT | DMA_DESTINATION_INCREMENT | DMA_ON;

    }
}

void drawImage3FlippedLR(int row, int col, int width, int height, const u16* image) {
    for (int r = 0; r < height; r++) {
    	DMA[3].src = (image + (r * width + width - 1) );

    	DMA[3].dst = (videoBuffer + (col +( (row + r) * 240 ) ));
        DMA[3].cnt = width | DMA_SOURCE_DECREMENT | DMA_DESTINATION_INCREMENT | DMA_ON;

    }
}

void petPixel(int row, int col, unsigned short color)
{
	videoBuffer[OFFSET(row, col, 240)] = color;
}

void drawChar(int row, int col, char ch, unsigned short color)
{
	for(int r = 0; r< 8; r++)
	{
		for(int c=0; c< 6; c++)
		{
			if( fontdata_6x8[OFFSET(r, c, 6) + ch*48] == 1)
			{
				petPixel(r+row, c+col, color);
			}
		}

	}
}

void drawString(int row, int col, char str[], unsigned short color)
{
	while(*str)
	{
		drawChar(row, col, *str++, color);
		col += 6;
		
	}
}


// Dir ----  UP (1),  RIGHT (2),  DOWN (3),  LEFT (4)
void jokeLogic(int *row, int *col, int *rdel, int *cdel, int enRow, int enCol, int dir) {
	if (dir == 1 || dir == 3)
	{	
		
		if (ABS(enRow - (SCREENHEIGHT - BATMAN_HEIGHT)) < enRow)
		{
			*rdel = *rdel < 0 ? *rdel : -(*rdel);

			if (*row > 0)
			{
				*row = *row + *rdel;
			}


		} else {
			*rdel = *rdel < 0 ? -(*rdel) : *rdel;
			if (*row < SCREENHEIGHT - JOKER_HEIGHT)
			{
				*row = *row + *rdel;
			}

		}
		
	}

	if (dir == 2 || dir == 4)
	{
		if (ABS(enCol - 240) < enCol)
		{	
			*cdel = *cdel < 0 ? *cdel : -(*cdel);
			
			if (*col > 0)
			{
				*col = *col + *cdel;
			}


		} else {
			*cdel = *cdel < 0 ? -(*cdel) : *cdel;
			if (*col < 240 - JOKER_WIDTH)
			{
				*col = *col + *cdel;
			}
		}
	} 
}


// void begin() {

// 	if(!KEY_DOWN_NOW(BUTTON_START)) {

// 		drawImage3(0, 0, START_WIDTH, START_HEIGHT, start);
// 		drawString(20, 20, "Start Game", YELLOW);

// 	} else {
// 		drawRect(0, 0, 400, 300, BLACK);
// 		gameStat = 1;
// 	}
	
// }


jWeap jokes[100];
jWeap bats[100];

int bat_count = 0;
int joke_count = 0;
int dir_joker = 1;
int speed = 1;
int sqDim = 5;



void makeJoke(int row, int col, int rdel) {
	jWeap joke;
	joke.row = row;
	joke.col = col;
	joke.cdel = dir_joker == 1 ? -speed : speed;
	joke.rdel = rdel;

	joke.oldRow = joke.row + 10;
	joke.oldCol = joke.col + 10;
	
	joke.color = RED;

	jokes[joke_count++] = joke;
}


void checkCollision(int radius, int enRadius, int row, int col, int *lives, int enRow, int enCol, bool *isHit, int *timer) {
	int rad = radius + enRadius;
	rad = rad * rad;
	int dx = ABS(row - enRow);
	int dy = ABS(col - enCol);
	int d  = (dx * dx) + (dy * dy);

	if (d < (rad) && !(*isHit))
	{
		*lives = *lives - 1;
		//*lives = *lives < 0 ? 0 : *lives;
		*isHit = true;
		*timer = 0;
	}
}


void maintainW(jWeap *jokes, int joke_count, int dimen, int radius, int row, int col, bool *isHit, int *lives, int *timer) {
	for (int i = 0; i < joke_count; ++i)
	{
		jokes[i].row = jokes[i].row + jokes[i].rdel;
		jokes[i].col = jokes[i].col + jokes[i].cdel;

		if(jokes[i].row < 0)
		{
			jokes[i].row = 0;
			jokes[i].rdel = -jokes[i].rdel;
		} else if(jokes[i].row > SCREENHEIGHT - dimen)
		{
			jokes[i].row = SCREENHEIGHT - dimen;
			jokes[i].rdel = -jokes[i].rdel;
		}

		if(jokes[i].col < 10)
		{
			jokes[i].col = 10;
			jokes[i].cdel = -jokes[i].cdel;
		} else if(jokes[i].col > 230)
		{
			jokes[i].col = 230;
			jokes[i].cdel = -jokes[i].cdel;
		}


		checkCollision(dimen, radius, jokes[i].row, jokes[i].col, lives, row, col, isHit, timer);		

		
		drawRect(jokes[i].row, jokes[i].col, dimen, dimen, jokes[i].color);
		drawRect(jokes[i].oldRow, jokes[i].oldCol, dimen, dimen, BLACK);
		
		jokes[i].oldRow = jokes[i].row;
		jokes[i].oldCol = jokes[i].col;
	}
}


void game() {

	

	

	int score = 453;
	char buffer[41];
	sprintf(buffer, "Score: %d", score);
	//char message[] = "Dear Mom, Send Money!";
	//char *mess = "Dear Mom, Send Money!";
	//drawString(150, 5, buffer, YELLOW);

	int row = 80;
	int col = 120;
	int rdel = 1;
	int cdel = 1;
	
	int oldrow = row;
	int oldcol = col;

	int batRow = 60;
	int batCol = 60;
	int oldBatRow = batRow;
	int oldBatCol = batCol;
	
	int batRad = MAX(BATMAN_HEIGHT, BATMAN_WIDTH) / 2;
	int jRad   = MAX(JOKER_HEIGHT, JOKER_WIDTH) / 2;
	// POINT batCen;
	// batCen.x = (BATMAN_WIDTH / 2) + batCol; 
	// batCen.y = (BATMAN_HEIGHT / 2) + batRow; 

	int batLifes = 3;
	int jokeLifes = 3;

	bool isHit = false;
	bool isJHit = false;
	bool flipped = false;

	int timer = 0;
	int jtimer = 0;

	while(1)
	{	
		timer++;
		jtimer++;
		if (batLifes <= 0 || jokeLifes <= 0)
		{	
			gameStat = batLifes <= 0 ? 3 : 2;
			break;
		}
		
		if (KEY_DOWN_NOW(BUTTON_SELECT))
		{
			gameStat = 0;
			break;
		}

		if (batLifes == 0)
		{
			gameStat = 2;
			break;
		}

		if(KEY_DOWN_NOW(BUTTON_UP))
		{
			batRow -= 1;
			if(batRow < 0)
			{
				batRow = 0;
			}

			jokeLogic(&row, &col, &rdel, &cdel, batRow, batCol, 1);

		}

		if(KEY_DOWN_NOW(BUTTON_DOWN))
		{
			batRow += 1;
			if(batRow > SCREENHEIGHT - BATMAN_HEIGHT)
			{
				batRow = SCREENHEIGHT - BATMAN_HEIGHT;
			}
			jokeLogic(&row, &col, &rdel, &cdel, batRow, batCol, 3);
			
		}

		if(KEY_DOWN_NOW(BUTTON_RIGHT))
		{
			batCol += 1;
			if(batCol > 240 - BATMAN_WIDTH)
			{
				batCol = 240 - BATMAN_WIDTH;
			}
			jokeLogic(&row, &col, &rdel, &cdel, batRow, batCol, 2);
			

		}

		if(KEY_DOWN_NOW(BUTTON_LEFT))
		{
			batCol -= 1;
			if(batCol < 0)
			{
				batCol = 0;
			}
			jokeLogic(&row, &col, &rdel, &cdel, batRow, batCol, 4);
			
			
		}

		checkCollision(jRad, batRad, row, col, &jokeLifes, batRow, batCol, &isJHit, &jtimer);

		if (timer == 100)
		{
			timer = 0;
			if (joke_count < 8) {
				makeJoke(row, col, speed);
				// volatile int miniTime = 200;
				// while(miniTime-- != 0){};

				makeJoke(row, col, -speed);
			}
			
		} else if (timer == 50) {
			isHit = false;
		}



		if (jtimer == 50)
		{
			jtimer = 0;
			isJHit = false;

		}

		if (batCol > col)
		{
			flipped = true;
		} else {
			flipped = false;
		}


		
		maintainW(jokes, joke_count, sqDim, batRad, batRow, batCol, &isHit, &batLifes, &timer);	

		
		//BATMAN
		if (!isHit)
		{
			if (!flipped)
			{
				drawImage3(batRow, batCol, BATMAN_WIDTH, BATMAN_HEIGHT, batman);
			} else {
				drawImage3FlippedLR(batRow, batCol, BATMAN_WIDTH, BATMAN_HEIGHT, batman);
			}
			
		} else {
			if (timer % 5 == 0){
				if (!flipped)
				{
					drawImage3(batRow, batCol, BATMAN_WIDTH, BATMAN_HEIGHT, batman);
				} else {
					drawImage3FlippedLR(batRow, batCol, BATMAN_WIDTH, BATMAN_HEIGHT, batman);
				}
			}
		}
		//JOKER

		if (!isJHit)
		{
			if (!flipped)
			{
				drawImage3FlippedLR(row, col, JOKER_WIDTH, JOKER_HEIGHT, joker);
			} else {
				drawImage3(row, col, JOKER_WIDTH, JOKER_HEIGHT, joker);
			}
			
		} else {
			if (jtimer % 5 == 0){
				if (!flipped)
				{
					drawImage3FlippedLR(row, col, JOKER_WIDTH, JOKER_HEIGHT, joker);
				} else {
					drawImage3(row, col, JOKER_WIDTH, JOKER_HEIGHT, joker);
				}
			}	
		}


		waitForVblank();
		drawRect(150, 5, 10, 70, BLACK);
		
		// DRAW LIFE ICONS

		for (int i = 0; i < batLifes; ++i)
		{
			drawImage3(150, 5 + ((LIFE_WIDTH + 1) * i), LIFE_WIDTH, LIFE_HEIGHT, life);	
		}


		for (int i = 0; i < jokeLifes; ++i)
		{
			int dist = (LIFE_WIDTH + 1) + 10;
			drawImage3(150, dist + 20 + ((LIFE_WIDTH + 1) * i), JLIFE_WIDTH, JLIFE_HEIGHT, jlife);	
		}

		//score++;
		//sprintf(buffer, "Score: %d", score);
		//drawString(150, 5, buffer, YELLOW);





		drawRect(oldrow - 1, oldcol - 1, JOKER_WIDTH + 3, JOKER_HEIGHT + 3, BLACK);
		drawRect(oldBatRow - 1, oldBatCol - 1, BATMAN_WIDTH + 3, BATMAN_HEIGHT + 3, BLACK);
		


		oldrow = row;
		oldcol = col;
		
		
		oldBatRow = batRow;
		oldBatCol = batCol;
	}

	drawRect(0, 0, 400, 300, BLACK);
	joke_count = 0;
	timer = 0;
}

void done(bool didwin, bool *drawn) {
	if (didwin)
	{
		if (!*drawn) {
			drawImage3(0, 0, WON_WIDTH, WON_HEIGHT, won);
			*drawn = true;
		}

		char *j = "YOU WON";
		drawString(20, 20, j, YELLOW);
		drawString(30, 15, "PRESS START", YELLOW);
		
	} else if (!didwin) {
		if (!*drawn)
		{
			drawImage3(0, 0, LOST_WIDTH, LOST_HEIGHT, lost);
			*drawn = true;
		}
		
		char *k = "YOU LOST";
		drawString(20, 140, k, RED);
		drawString(30, 135, "PRESS START", RED);
		
	}

	if(KEY_DOWN_NOW(BUTTON_SELECT)) {

		drawRect(0, 0, 400, 300, BLACK);
		gameStat = 0;
		*drawn = false;

	}
}




