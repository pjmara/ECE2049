/************** ECE2049 DEMO CODE ******************/
/**************  13 March 2019   ******************/
/***************************************************/

#include <msp430.h>
#include <stdlib.h>
#include <stdio.h>

/* Peripherals.c and .h are where the functions that implement
 * the LEDs and keypad, etc are. It is often useful to organize
 * your code by putting like functions together in files.
 * You include the header associated with that file(s)
 * into the main file of your project. */
#include "peripherals.h"

// Function Prototypes
void swDelay(char numLoops);

// Declare globals here

enum state{INITIAL_SCREEN, GENERATE_LEVEL, DRAW_SCREEN, CHECK_KEYPAD, GAME_CONDITIONS, COUNTDOWN, GAME_CONDITION};

typedef struct alien{
    int x;
    int y;
    char id[2];
}alien;

char randomChar(){
    int num = (rand() % 5) + 1;
    return num + '0';
}

void lowerAliens(int* numberAliens, alien alienList[40]){

    if (*numberAliens > 0){
        int i;
        for( i = 0; i < *numberAliens; i++){
            alienList[i].y = alienList[i].y + 10;
        }
    }
}


int killLowest(char input, int *numberAliens, alien alienList[40]){
    int i = 0;
    int lowest = 0;
    int lowestIndex = -1;
    for(; i < *numberAliens; i++){
        if(alienList[i].id[0] == input && alienList[i].y > lowest){
                lowestIndex = i;
                lowest = alienList[i].y;
        }
    }
    if(lowest != 0){
        i = lowestIndex;
        for (; i < 39; i++){
            alienList[i] = alienList[i+1];
        }
        *numberAliens = *numberAliens - 1;
        return 0;
    }
    return -1;
}

char * intTostring(int num){
    char str[2];
    str[0] = num + '0';
    str[1] = '\0';
    return str;
}


// Main
void main(void)

{
    srand (time(NULL));


    enum state currentState = INITIAL_SCREEN;
    unsigned char currKey=0, dispSz = 3;
    alien alienList[40];
    int numberAliens = 3;

    long unsigned int checkFrequency = 20000;
    int levelAliens = 5;
    int kill = 0;
    long unsigned int counter = 0;
    bool updateKill = false;
    bool updateLowering = false;

    bool firstKeyPress = true;
    unsigned long int timeOffset = checkFrequency;
    bool overlap = false;




    //Initializing array
    alien one;
    one.x = (rand() % 5) * 18 + 12;
    one.y = 10;
    one.id[0] = '1';
    one.id[1] = '\0';
    alienList[0] = one;
    alien two;
    two.x = (rand() % 5) * 18 + 12;
    two.y = 10;
    two.id[0] = '2';
    two.id[1] = '\0';
    alienList[1] = two;
    alien three;
    three.x = (rand() % 5) * 18 + 12;
    three.y = 10;
    three.id[0] = '3';
    three.id[1] = '\0';
    alienList[2] = three;


    WDTCTL = WDTPW | WDTHOLD;    // Stop watchdog timer. Always need to stop this!!
                                 // You can then configure it properly, if desired

    // Initializing things
    initLeds();
    configDisplay();
    configKeypad();

    Graphics_clearDisplay(&g_sContext); // Clear the display
    while (1)    // Forever loop
    {

        switch(currentState){
        case INITIAL_SCREEN:


                // Write some text to the display
                Graphics_drawStringCentered(&g_sContext, "Welcome", AUTO_STRING_LENGTH, 48, 35, TRANSPARENT_TEXT);
                Graphics_drawStringCentered(&g_sContext, "to", AUTO_STRING_LENGTH, 48, 45, TRANSPARENT_TEXT);
                Graphics_drawStringCentered(&g_sContext, "Space Invaders", AUTO_STRING_LENGTH, 48, 55, TRANSPARENT_TEXT);
                Graphics_drawStringCentered(&g_sContext, "* to Start", AUTO_STRING_LENGTH, 48, 65, TRANSPARENT_TEXT);


                // Draw a box around everything because it looks nice
                Graphics_Rectangle box = {.xMin = 5, .xMax = 91, .yMin = 5, .yMax = 91 };
                Graphics_drawRectangle(&g_sContext, &box);

                // We are now done writing to the display.  However, if we stopped here, we would not
                // see any changes on the actual LCD.  This is because we need to send our changes
                // to the LCD, which then refreshes the display.
                // Since this is a slow operation, it is best to refresh (or "flush") only after
                // we are done drawing everything we need.
                Graphics_flushBuffer(&g_sContext);
                currKey = getKey();
                if (currKey == '*'){
                    currentState = COUNTDOWN;
                }
                break;
        case COUNTDOWN:
            Graphics_clearDisplay(&g_sContext); // Clear the display
            Graphics_drawStringCentered(&g_sContext, "3...", AUTO_STRING_LENGTH, 48, 45, TRANSPARENT_TEXT);
            Graphics_flushBuffer(&g_sContext);
            swDelay(2);

            Graphics_clearDisplay(&g_sContext); // Clear the display
                        Graphics_drawStringCentered(&g_sContext, "2...", AUTO_STRING_LENGTH, 48, 45, TRANSPARENT_TEXT);
                        Graphics_flushBuffer(&g_sContext);
                        swDelay(2);

                        Graphics_clearDisplay(&g_sContext); // Clear the display
                                    Graphics_drawStringCentered(&g_sContext, "1...", AUTO_STRING_LENGTH, 48, 45, TRANSPARENT_TEXT);
                                    Graphics_flushBuffer(&g_sContext);
                                    swDelay(2);
                                    currentState = DRAW_SCREEN;

        case DRAW_SCREEN:
            Graphics_clearDisplay(&g_sContext); // Clear the display
            int i;
            for (i = 0; i < numberAliens; i++){
                Graphics_drawStringCentered(&g_sContext, alienList[i].id, AUTO_STRING_LENGTH, alienList[i].x, alienList[i].y, TRANSPARENT_TEXT);
            }
            //Graphics_Rectangle box = {.xMin = 5, .xMax = 91, .yMin = 5, .yMax = 91 };
            Graphics_drawRectangle(&g_sContext, &box);
            Graphics_flushBuffer(&g_sContext);
            currentState = CHECK_KEYPAD;
            break;
        case CHECK_KEYPAD:
            currKey = getKey();
            int ret = -1;
            updateKill = false;
            if (currKey == '1' || currKey == '2' || currKey == '3' || currKey == '4' || currKey == '5'){
                if (firstKeyPress) {
                    ret =  killLowest(currKey, &numberAliens, alienList);
                    timeOffset = counter;
                    if (counter + 2000 > checkFrequency) {
                        overlap = true;
                    }
                    else {
                        overlap = false;
                    }
                    firstKeyPress = false;
                }
                else if (timeOffset + 2000 < counter || (overlap && (timeOffset + 2000 - checkFrequency < counter))) {
                    firstKeyPress = true;
                }



                if(ret == 0){
                    kill++;
                    updateKill = true;
                }
                else{
                    updateKill = false;
                }
            }
            currentState = GAME_CONDITION;


            break;

        case GAME_CONDITION:


            if(counter >= checkFrequency){
                lowerAliens(&numberAliens, alienList);
                updateLowering = true;
                counter = 0;
            }
            else{
                updateLowering = false;
            }


            if (updateLowering && kill + numberAliens < levelAliens && numberAliens < 40){
               if (rand() % 2){
                   alien temp;
                   temp.x = (rand() % 5) * 18 + 12;
                   temp.y = 10;
                   temp.id[0] = randomChar();
                   temp.id[1] = '\0';
                   alienList[numberAliens] = temp;
                   numberAliens++;
               }
            }

            if (updateLowering || updateKill){
                currentState = DRAW_SCREEN;
            }
            else{
                currentState = CHECK_KEYPAD;
            }

            if (kill >= levelAliens){
                kill = 0;
                firstKeyPress = true;
                levelAliens += 5;
                int level = levelAliens / 5;
                checkFrequency = checkFrequency - (150 * level * level);
                Graphics_clearDisplay(&g_sContext); // Clear the display
                Graphics_drawStringCentered(&g_sContext, "Level Cleared", AUTO_STRING_LENGTH, 48, 45, TRANSPARENT_TEXT);
                Graphics_flushBuffer(&g_sContext);
                swDelay(2);
            }

            break;





        }
        counter++;
        if (timeOffset + 2000 < counter || (overlap && (timeOffset + 2000 - checkFrequency < counter))) {
            firstKeyPress = true;
        }


     /*   // Check if any keys have been pressed on the 3x4 keypad
        currKey = getKey();
        if (currKey == '*')
            BuzzerOn();
        if (currKey == '#')
            BuzzerOff();
        if ((currKey >= '0') && (currKey <= '9'))
            setLeds(currKey - 0x30);

        if (currKey)
        {
            dispThree[1] = currKey;
            // Draw the new character to the display
            Graphics_drawStringCentered(&g_sContext, dispThree, dispSz, 48, 55, OPAQUE_TEXT);

            // Refresh the display so it shows the new data
            Graphics_flushBuffer(&g_sContext);

            // wait awhile before clearing LEDs
            swDelay(1);
            setLeds(0);
        }*/

    }  // end while (1)
}


void swDelay(char numLoops)
{
	// This function is a software delay. It performs
	// useless loops to waste a bit of time
	//
	// Input: numLoops = number of delay loops to execute
	// Output: none
	//
	// smj, ECE2049, 25 Aug 2013

	volatile unsigned int i,j;	// volatile to prevent removal in optimization
			                    // by compiler. Functionally this is useless code

	for (j=0; j<numLoops; j++)
    {
    	i = 50000 ;					// SW Delay
   	    while (i > 0)				// could also have used while (i)
	       i--;
    }
}
