//
//  main.c
//  TetrisProject
//
//  Created by Sagar Chadha on 2019-03-28.
//  Copyright © 2019 Sagar Chadha and Japvinit Kour. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>

//Definitions
#define pbEdge ((volatile long *) 0xFF20005C)
#define RLEDs ((volatile long *) 0xFF200000)
#define hexDisplay ((volatile long *) 0xFF200020)

//Global Variables
volatile int pixel_buffer_start; // global variable
int hexArray[10] = {0b00111111, 0b00000110, 0b01011011, 0b01001111, 0b01100110,
                    0b01101101, 0b01111101, 0b00000111, 0b01111111, 0b01100111};

//1 cube is 16 x 16 pixels
//Game width is 160 pixels starting from 80 to 240
struct Shapes {
    int shapeID;
    int rotation_state;
    short int colour;
    int pos1x, pos1y;
    int pos2x, pos2y;
    int pos3x, pos3y;
    int pos4x, pos4y;
};

//Funciton prototypes
void plot_pixel(int x, int y, short int colour);
void wait_for_vsync();
void setupShapes(struct Shapes shapesArray[]);
void drawBlock(int x, int y, short int colour);
void clear_screen();
void setupBoard(short int board[160][240]);
void drawBoard(short int board[160][240]);
int updateScore(int score, int rowsFilledCount);
void rotate(short int board[320][240], int shapeid, int* state, int* current1x, int* current1y, int* current2x,
            int* current2y, int* current3x, int* current3y, int* current4x, int* current4y);
bool check_valid(short int board[320][240], int* currentx, int* currenty, int dx, int dy);
void draw_borders(short int board[320][240]);
void display_score(unsigned score);

//extern short MYIMAGE [240][320];
//extern short ENDGAMEIMAGE [240][320];


int main() {
    *pbEdge = 15;
    bool gameOver = false;
    bool gameRestart = false;
    
    //Setting up the board
    short int board[320][240];
    
    //Seting up the shapes
    int numShapes = 4;
    struct Shapes shapesArray[numShapes];
    setupShapes(shapesArray);
    //srand(time(NULL));
    
    volatile int * pixel_ctrl_ptr = (int *)0xFF203020;
    // set front pixel buffer to start of FPGA On-chip memory
    *(pixel_ctrl_ptr + 1) = 0xC8000000; // first store the address in the
    // back buffer
    //now, swap the front/back buffers, to set the front buffer location
    wait_for_vsync();
    // initialize a pointer to the pixel buffer, used by drawing functions
    pixel_buffer_start = *pixel_ctrl_ptr;
    clear_screen(); // pixel_buffer_start points to the pixel buffer
    //set back pixel buffer to start of SDRAM memory
    *(pixel_ctrl_ptr + 1) = 0xC0000000;
    pixel_buffer_start = *(pixel_ctrl_ptr + 1); // we draw on the back buffer
    
    //**FUNCTION TO DISPLAY LOAD SCREEN**//
    
    while (!gameRestart) {
        unsigned score = 0;
        display_score(score);
        bool gameRestart = false;
        gameOver = false;
        //Display the title screen → comment for cpulator
//        volatile short * pixelbuf = 0xc8000000;
//        int i, j;
//        for (i=0; i<240; i++){
//            for (j=0; j<320; j++){
//                *(pixelbuf + (j<<0) + (i<<9)) = MYIMAGE[i][j];
//            }
//        }
        
        //Wait for user to press key0 to start the game
        while (*pbEdge != 1) {
            if (*pbEdge != 0 && *pbEdge != 1)
                *pbEdge = 15;
        }
        *pbEdge = 15;
        
        setupBoard(board);
        drawBoard(board);

        //Funciton to start dropping items and basically perform the game
        while (!gameOver) {
            //Load new shape
            struct Shapes currentShape = shapesArray[rand() % (numShapes)];
            *pbEdge = 15;
            gameOver = board[currentShape.pos1x][currentShape.pos1y] != 0x0 ||
             board[currentShape.pos2x][currentShape.pos2y] != 0x0 ||
             board[currentShape.pos3x][currentShape.pos3y] != 0x0 ||
             board[currentShape.pos4x][currentShape.pos4y] != 0x0;
            //gameOver = false;
            if (gameOver){
                break;
            }
            
            int current1x = currentShape.pos1x;
            int current1y = currentShape.pos1y;
            int current2x = currentShape.pos2x;
            int current2y = currentShape.pos2y;
            int current3x = currentShape.pos3x;
            int current3y = currentShape.pos3y;
            int current4x = currentShape.pos4x;
            int current4y = currentShape.pos4y;
            
            bool moveOver = false;
            while(!moveOver) {
                drawBoard(board);
                drawBlock(current1x, current1y, currentShape.colour);
                drawBlock(current2x, current2y, currentShape.colour);
                drawBlock(current3x, current3y, currentShape.colour);
                drawBlock(current4x, current4y, currentShape.colour);

                //Move to right based on key input
                if (*pbEdge == 1 && current1x < 239 && current2x < 239 &&
                    current3x < 239 && current4x < 239 &&
                    (board[current1x + 16][current1y] == 0) &&
                    (board[current2x + 16][current2y] == 0) &&
                    (board[current3x + 16][current3y] == 0) &&
                    (board[current4x + 16][current4y] == 0)) {
                    current1x += 16;
                    current2x += 16;
                    current3x += 16;
                    current4x += 16;
                    *pbEdge = 15;
                }
                //Move to left based on key input
                if (*pbEdge == 2 && current1x > 95 && current2x > 95 &&
                    current3x > 95  && current4x > 95 &&
                    (board[current1x - 16][current1y] == 0) &&
                    (board[current2x - 16][current2y] == 0) &&
                    (board[current3x - 16][current3y] == 0) &&
                    (board[current4x - 16][current4y] == 0)) {
                    current1x -= 16;
                    current2x -= 16;
                    current3x -= 16;
                    current4x -= 16;
                    *pbEdge = 15;
                }

                //Rotate
                if (*pbEdge == 4) {
                    rotate(board, currentShape.shapeID, &currentShape.rotation_state, &current1x, &current1y, &current2x,
                           &current2y, &current3x, &current3y, &current4x, &current4y);
                    *pbEdge = 15;
                }

                if (*pbEdge != 1 && *pbEdge != 2 && *pbEdge != 4)
                    *pbEdge = 15;
                
                //Move down if the next block is black
                if (current1y < 239 && current2y < 239 && current3y < 239 && current4y < 239 &&
                    (board[current1x][current1y + 15] == 0) &&
                    (board[current2x][current2y + 15] == 0) &&
                    (board[current3x][current3y + 15] == 0) &&
                    (board[current4x][current4y + 15] == 0)) {
                        current1y += 16;
                        current2y += 16;
                        current3y += 16;
                        current4y += 16;
                }
                //Place the shape onto the board
                else {
                    int i, j;
                    for (i = 0; i < 16; ++i) {
                        for (j = 0; j < 16; ++j) {
                            board[current1x - i][current1y - j] = currentShape.colour;
                            board[current2x - i][current2y - j] = currentShape.colour;
                            board[current3x - i][current3y - j] = currentShape.colour;
                            board[current4x - i][current4y - j] = currentShape.colour;
                        }
                    }
                    moveOver = true;
                }
//                int i = 0;
//                for(i = 0; i < 10000000000; ++i);
                wait_for_vsync();                           // swap front and back buffers on VGA vertical sync
                pixel_buffer_start = *(pixel_ctrl_ptr + 1); // new back buffer
            }

            //Check for if any rows are to be removed
            bool rowsFilled[15];
            int rowCount = 0;
            int i, j;
            for (j = 0; j < 15; ++j) {
                for (i = 95; i < 240; ++i) {
                    if (board[i][j*16 + 15] != 0) {
                        rowsFilled[j] = true;
                    }
                    else {
                        rowsFilled[j] = false;
                        break;
                    }
                }
                if (rowsFilled[j]) ++rowCount;
            }
            
            //Clears out filled rows and shifts everything on the board down
            for (j = 0; j < 15; ++j) {
                if (rowsFilled[j]) {
                    for (i = 79; i < 240; ++i) {
                        for (int k = 0; k <= 16; ++k) {
                            for (int l = 0; l <= 16; ++l) {
                                if (i < 240 - 16)
                                    board[i + k][j*16 + l] = 0;
                            }
                        }
                        
                        for (int k = j*16 + 15; k >= 0; --k) {
                            if (k <= 15)
                                board[i][k] = 0;
                            else
                                board[i][k] = board[i][k - 16];
                        }
                    }
                }
            }
            
            //Update score based on lines removed
            score = updateScore(score, rowCount);
            display_score(score);
        }

        //**FUNCTION TO DISPLAY GAME OVER SCREEN**//
//        int x, y;
//        for (x=0; x<240; x++){
//            for (y=0; y<320; y++){
//                *(pixelbuf + (y<<0) + (x<<9)) = ENDGAMEIMAGE[x][y];
//            }
//        }
        
        wait_for_vsync();                           // swap front and back buffers on VGA vertical sync
        pixel_buffer_start = *(pixel_ctrl_ptr + 1); // new back buffer

        //Wait for user to press all of the keys
        while (*pbEdge != 1) {
            if (*pbEdge != 0 && *pbEdge != 1)
                *pbEdge = 15;
        }
        *pbEdge = 15;
        setupBoard(board);
        gameRestart = true;
    }
    return 0;
}

//Function to plot a pixel with a given colour
void plot_pixel(int x, int y, short int colour)
{
   *(short int *)(pixel_buffer_start + (y << 10) + (x << 1)) = colour;
}

//Function to draw the 16 x 16 pixels that make up a block
void drawBlock(int x, int y, short int colour) {
    int i = 0, j = 0;
    for (i = 0; i < 16; ++i) {
        for (j = 0; j < 16; ++j) {
            plot_pixel(x - i, y - j, colour);
        }
    }
}

//Function to clear the screen to be all black
void clear_screen(){
    int i = 0, j = 0;
    for(i = 0; i < 320; ++i){
        for(j = 0; j < 240; ++j){
            plot_pixel(i, j, 0x0);
        }
    }
}

//Function for syncing the pixel controller
void wait_for_vsync() {
    volatile int * pixel_ctrl_ptr = 0xFF203020; // pixel controller
    register int status;
    
    *pixel_ctrl_ptr = 1; // start the synchronization process
    
    status = *(pixel_ctrl_ptr + 3);
    while ((status & 0x01) != 0) {
        status = *(pixel_ctrl_ptr + 3);
    }
}

//Function to set up the shapes that are used
void setupShapes(struct Shapes shapesArray[]) {
    //Setting up a square
    shapesArray[0].shapeID = 0;
    shapesArray[0].rotation_state = 0;
    shapesArray[0].colour = 0x001F;
    shapesArray[0].pos1x = 159;
    shapesArray[0].pos1y = 31;
    shapesArray[0].pos2x = 175;
    shapesArray[0].pos2y = 31;
    shapesArray[0].pos3x = 159;
    shapesArray[0].pos3y = 15;
    shapesArray[0].pos4x = 175;
    shapesArray[0].pos4y = 15;
    
    //Setting up a rectangle
    shapesArray[1].shapeID = 1;
    shapesArray[1].rotation_state = 0;
    shapesArray[1].colour = 0x07E0;
    shapesArray[1].pos1x = 143;
    shapesArray[1].pos1y = 15;
    shapesArray[1].pos2x = 159;
    shapesArray[1].pos2y = 15;
    shapesArray[1].pos3x = 175;
    shapesArray[1].pos3y = 15;
    shapesArray[1].pos4x = 191;
    shapesArray[1].pos4y = 15;
    
    //Setting up an L shape
    shapesArray[2].shapeID = 2;
    shapesArray[2].rotation_state = 0;
    shapesArray[2].colour = 0xF800;
    shapesArray[2].pos1x = 143;
    shapesArray[2].pos1y = 15;
    shapesArray[2].pos2x = 143;
    shapesArray[2].pos2y = 31;
    shapesArray[2].pos3x = 159;
    shapesArray[2].pos3y = 31;
    shapesArray[2].pos4x = 175;
    shapesArray[2].pos4y = 31;
    
    //Setting up an L shape (reverse direction)
    shapesArray[3].shapeID = 3;
    shapesArray[3].rotation_state = 0;
    shapesArray[3].colour = 0xF81F;
    shapesArray[3].pos1x = 175;
    shapesArray[3].pos1y = 15;
    shapesArray[3].pos2x = 143;
    shapesArray[3].pos2y = 31;
    shapesArray[3].pos3x = 159;
    shapesArray[3].pos3y = 31;
    shapesArray[3].pos4x = 175;
    shapesArray[3].pos4y = 31;
}

//Function for initializing the board at the beginning of the game
void setupBoard(short int board[320][240]){
    int i = 0, j = 0;
    for(i = 0; i < 320; ++i){
        for(j = 0; j < 240; ++j){
            if (i == 78 || i == 240)
                board[i][j] = 0xFFFF;
            else
                board[i][j] = 0;
        }
    }
}

//Function for drawing the board
void drawBoard(short int board[320][240]){
    int i = 0, j = 0;
    for(i = 0; i < 320; ++i){
        for(j = 0; j < 240; ++j){
            plot_pixel(i, j, board[i][j]);
        }
    }
}

//Function for updating the score based on lines removed
int updateScore(int score, int rowsFilledCount){
    if (rowsFilledCount == 1) return score + 5;
    if (rowsFilledCount == 2) return score + 15;
    if (rowsFilledCount == 3) return score + 30;
    if (rowsFilledCount >= 4) return score + 50;
    return score;
}

//Function for rotating the shapes
void rotate(short int board[320][240], int shapeid, int* state, int* current1x, int* current1y, int* current2x,
            int* current2y, int* current3x, int* current3y, int* current4x, int* current4y) {
    if (shapeid == 0)
        return;
    //Rotate Rectangle
    else if (shapeid == 1) {
        if (*state == 0) {
            if (check_valid(board, current1x, current1y, 16*2, -16*2) &&
            check_valid(board, current2x, current2y, 16, -16) &&
            check_valid(board, current4x, current4y, -16, 16)){
                *current1x = *current1x + 16*2;
                *current1y = *current1y - 16*2;
                *current2x = *current2x + 16;
                *current2y = *current2y - 16;
                *current4x = *current4x - 16;
                *current4y = *current4y + 16;
                *state = 1;
                return;
            }
        }
        else if (*state == 1) {
            if (check_valid(board, current1x, current1y, -16*2, 16*2) &&
                check_valid(board, current2x, current2y, -16, 16) &&
                check_valid(board, current4x, current4y, 16, -16)){
                *current1x = *current1x - 16*2;
                *current1y = *current1y + 16*2;
                *current2x = *current2x - 16;
                *current2y = *current2y + 16;
                *current4x = *current4x + 16;
                *current4y = *current4y - 16;
                *state = 0;
                return;
            }
        }
    }
    //Rotate L Shape
    else if (shapeid == 2) {
        if (*state == 0) {
            if (check_valid(board, current1x, current1y, 16*2, 0) &&
                check_valid(board, current2x, current2y, 16, -16) &&
                check_valid(board, current4x, current4y, 0, 16)){
                *current1x = *current1x + 16*2;
                *current2x = *current2x + 16;
                *current2y = *current2y - 16;
                *current4x = *current4x - 16;
                *current4y = *current4y + 16;
                *state = 1;
                return;
            }
        }
        else if (*state == 1) {
            if (check_valid(board, current1x, current1y, 0, 16*2) &&
                check_valid(board, current2x, current2y, 16, 16) &&
                check_valid(board, current4x, current4y, -16, -16)){
                *current1x = *current1x;
                *current1y = *current1y + 16*2;
                *current2x = *current2x + 16;
                *current2y = *current2y + 16;
                *current4x = *current4x - 16;
                *current4y = *current4y - 16;
                *state = 2;
                return;
            }
        }
        else if (*state == 2) {
            if (check_valid(board, current1x, current1y, -16*2, 0) &&
                check_valid(board, current2x, current2y, -16, 16) &&
                check_valid(board, current4x, current4y, 16, -16)){
                *current1x = *current1x - 16*2;
                *current1y = *current1y;
                *current2x = *current2x - 16;
                *current2y = *current2y + 16;
                *current4x = *current4x + 16;
                *current4y = *current4y - 16;
                *state = 3;
                return;
            }
        }
        else if (*state == 3) {
            if (check_valid(board, current1x, current1y, 0, -16*2) &&
                check_valid(board, current2x, current2y, -16, -16) &&
                check_valid(board, current4x, current4y, 16, 16)){
                *current1x = *current1x;
                *current1y = *current1y - 16*2;
                *current2x = *current2x - 16;
                *current2y = *current2y - 16;
                *current4x = *current4x + 16;
                *current4y = *current4y + 16;
                *state = 0;
                return;
            }
        }
    }
    //Rotate L Shape
    else if (shapeid == 3) {
        if (*state == 0) {
            if (check_valid(board, current1x, current1y, 0, 16*2) &&
                check_valid(board, current2x, current2y, 16, -16) &&
                check_valid(board, current4x, current4y, -16, 16)){
                *current1y = *current1y + 16*2;
                *current2x = *current2x + 16;
                *current2y = *current2y - 16;
                *current4x = *current4x - 16;
                *current4y = *current4y + 16;
                *state = 1;
                return;
            }
        }
        else if (*state == 1) {
            if (check_valid(board, current1x, current1y, -16*2, 0) &&
                check_valid(board, current2x, current2y, 16, 16) &&
                check_valid(board, current4x, current4y, -16, -16)){
                *current1x = *current1x - 16*2;
                *current1y = *current1y;
                *current2x = *current2x + 16;
                *current2y = *current2y + 16;
                *current4x = *current4x - 16;
                *current4y = *current4y - 16;
                *state = 2;
                return;
            }
        }
        else if (*state == 2) {
            if (check_valid(board, current1x, current1y, 0, -16*2) &&
                check_valid(board, current2x, current2y, -16, 16) &&
                check_valid(board, current4x, current4y, 16, -16)){
                *current1x = *current1x;
                *current1y = *current1y - 16*2;
                *current2x = *current2x - 16;
                *current2y = *current2y + 16;
                *current4x = *current4x + 16;
                *current4y = *current4y - 16;
                *state = 3;
                return;
            }
        }
        else if (*state == 3) {
            if (check_valid(board, current1x, current1y, 16*2, 0) &&
                check_valid(board, current2x, current2y, -16, -16) &&
                check_valid(board, current4x, current4y, 16, 16)){
                *current1x = *current1x  + 16*2;
                *current1y = *current1y;
                *current2x = *current2x - 16;
                *current2y = *current2y - 16;
                *current4x = *current4x + 16;
                *current4y = *current4y + 16;
                *state = 0;
                return;
            }
        }
    }
}

bool check_valid(short int board[320][240], int* currentx, int* currenty, int dx, int dy) {
    if (board[*currentx + dx][*currenty + dy] == 0 &&
        *currentx + dx >= 95 && *currentx + dx <= 239 &&
        *currenty + dy >= 0 && *currenty + dy <= 239)
        return true;
    else
        return false;
}

void display_score(unsigned score) {
    int Display = hexArray[score % 10];
    Display += hexArray[score/10 % 10] << 8;
    Display += hexArray[score/100 % 10] << 16;
    Display += hexArray[score/1000 % 10] << 24;
    *hexDisplay = Display;
}
