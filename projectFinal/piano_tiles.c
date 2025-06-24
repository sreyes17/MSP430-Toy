#include <msp430.h>
#include <libTimer.h>
#include "lcdutils.h"
#include "lcddraw.h"
#include "buzzer.h"
#include <stdlib.h> // included only for rand()


#define TILE_COLOR COLOR_BLACK
#define OUTLINE_COLOR COLOR_WHITE
#define BACKGROUND_COLOR COLOR_CYAN
#define GAMEOVER_COLOR COLOR_RED

#define TILE_WIDTH  32
#define TILE_HEIGHT 40

#define ROWS 4
#define COLS 4

#define SW1 1
#define SW2 2
#define SW3 4
#define SW4 8

#define NOTE_F5 250
#define NOTE_E5 260
#define NOTE_D5 294
#define NOTE_C5 317

#define NOTE_B4 381
#define NOTE_A4 440
#define NOTE_G4 493

int notes[] = {NOTE_G4, NOTE_A4, NOTE_B4,NOTE_D5,NOTE_E5, NOTE_F5, NOTE_D5, NOTE_E5, NOTE_C5, NOTE_D5, NOTE_A4, NOTE_G4, NOTE_A4, NOTE_B4, NOTE_D5, NOTE_E5, NOTE_F5, NOTE_D5, NOTE_E5, NOTE_C5, NOTE_D5, NOTE_A4, NOTE_A4, NOTE_B4, NOTE_D5, NOTE_C5, NOTE_D5, NOTE_C5, NOTE_D5, NOTE_C5, NOTE_D5, NOTE_A4, NOTE_A4, NOTE_A4, NOTE_A4, NOTE_A4, NOTE_G4};

#define SONG_LENGTH 37

#define SWITCHES (SW1 | SW2 | SW3 | SW4)


unsigned char grid[ROWS][COLS] = {0}; //makes a 4x4 array where 1's are tiles
int redrawScreen = 1;
int gameRunning = 1;
int currNote = 0;

static int timeIncrement = 188; //starts at 3/4ths of a second increments
static int count = 0;

//all my funtions
void makeTopTile();
void moveDown();
void displayTiles();
void switch_init();
void buttonPress(unsigned char button);
void playNextNote();

void makeTopTile() {
    for (int j = 0; j < COLS; j++) { //clears out the first row by turning it
        grid[0][j] = 0;              //all to 0's, useful when moving rows to n-1
  }

    int random = rand() % COLS; //generates a random number, modulus ensures                                that it is only between 0-4
    grid[0][random] = 1; // 1 = there is a tile
}
void gameOver(){
    //WDTCTL = WDTPW | WDTHOLD;
    gameRunning = 0;
    clearScreen(GAMEOVER_COLOR);
    drawString5x7(24,60,"!!Game Over!!", OUTLINE_COLOR, GAMEOVER_COLOR);
    drawString5x7(14,75, "Retry? (Click any", OUTLINE_COLOR, GAMEOVER_COLOR);
    drawString5x7(45,90, "Button)", OUTLINE_COLOR, GAMEOVER_COLOR);
}

void restartGame(){
    gameRunning = 1;
    redrawScreen = 1;
    currNote = 0;
    timeIncrement = 188;
    for (int i = 0; i < ROWS; i++) {     //clears the grid, no tiles when you
        for (int j = 0; j < COLS; j++) { //start
            grid[i][j] = 0;
        }
    }
    
    clearScreen(BACKGROUND_COLOR);
    makeTopTile();
}

void playNextNote(){
    if(currNote >= SONG_LENGTH){
        currNote = 0;
    }
    int key = notes[currNote];
    
    buzzer_set_period(key*10);
    __delay_cycles(2500000);
    buzzer_set_period(0);
    
    currNote++;
}


void moveDown() {
    for (int j = 0; j < COLS; j++) {
        if (grid[ROWS - 1][j] == 1) { //how you die--if there is a tile in the                              last column after the wdt is referenced
            gameOver();
            return;
        }
    }


    for (int i = ROWS - 1; i > 0; i--) {
        for (int j = 0; j < COLS; j++) {
            grid[i][j] = grid[i - 1][j]; // starts bottom up, 4th row copies 3rd                            row, 3rd copies 2nd, etc... 1st row                               remains as is but is deleted in                                   makeTopTile()
        }
    }
    makeTopTile();
}

void removeLastTile(int col) {
    for (int i = ROWS - 1; i >= 0; i--) {
        if (grid[i][col] == 1) { //checks if there is a tile where button was                           pressed
            int tileCol = col * TILE_WIDTH; //telling it where to start
            int tile_Row = i * TILE_HEIGHT;
            fillRectangle(tileCol, tileRow, TILE_WIDTH, TILE_HEIGHT, BACKGROUND_COLOR);

            grid[i][col] = 0; //after display is updated, update grid
            playNextNote();
        
            return;
        }
    }
}


void displayTiles() {
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            int tileCol = j * TILE_WIDTH;
            int tileRow = i * TILE_HEIGHT;

            if (grid[i][j] == 1) {
                fillRectangle(tileCol, tileRow, TILE_WIDTH, TILE_HEIGHT, OUTLINE_COLOR);   //outline,,just a bigger rectangle
        
                fillRectangle(tileCol + 2, tileRow + 2, TILE_WIDTH - 4, TILE_HEIGHT - 4, TILE_COLOR); //tile, only 2 pixels smaller on each side
            }
            else {
                    fillRectangle(tileCol, tileRow, TILE_WIDTH, TILE_HEIGHT, BACKGROUND_COLOR); //works as the "background". in reality, it's just 3 cyan colored rectangles
      }
    }
  }
}


void switch_init() {
    P2REN |= SWITCHES;   /* enables resistors for switches */
    P2IE |= SWITCHES;    /* enable interrupts from switches */
    P2OUT |= SWITCHES;   /* pull-ups for switches */
    P2DIR &= ~SWITCHES;  /* set switches' bits for input */
}


void buttonPress(unsigned char button) {
    
    if(gameRunning == 0){
        restartGame();
        return;
    }
    //returns column based on what button was pressed
    switch (button) {
        case SW1:
            removeLastTile(0);
            break;
        case SW2:
            removeLastTile(1);
            break;
        case SW3: 
            removeLastTile(2);
            break;
        case SW4: 
            removeLastTile(3);
            break;
        default: 
            break;
  }
}


void wdt_c_handler() {
    if (count >= timeIncrement) { //timeIncrement starts at 188, keeps getting                                faster to make the game harder
        count = 0;
        timeIncrement -=1;
        moveDown(); //after given time, game moves down
        redrawScreen = 1;   // since tiles were moved, vm must redraw screen. this triggers displayTiles in main
    }
    count++;
}


void __interrupt_vec(PORT2_VECTOR) Port_2() {
    if (P2IFG & SWITCHES) {        // P2IFG shows what port caused an                                           interrupt (displays a 1)
        unsigned char button = P2IFG & SWITCHES; // will only have a 1 on the                                             switch that was pressed
        P2IFG &= ~SWITCHES;
        buttonPress(button);
    }
}


void main() {
    configureClocks();
    //lcd_init();
    buzzer_init();
    switch_init();
    enableWDTInterrupts();
    or_sr(0x8);

    makeTopTile(); //game starts with just one tile coming down from the top


    while (1) { //forever
        if (redrawScreen && gameRunning) {
            redrawScreen = 0;
            displayTiles();
        }
        or_sr(0x10);
    }
}
