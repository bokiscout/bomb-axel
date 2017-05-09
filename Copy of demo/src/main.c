#include "lpc17xx_pinsel.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_timer.h"

#include "joystick.h"	// joystick
#include "rgb.h"	// rgb led
#include "oled.h"	// oled display
#include "led7seg.h"	// seven segment display
#include "pca9532.h" // led stripe
#include "acc.h"	// accelerometer
#include <time.h>

#define NOTE_PIN_HIGH() GPIO_SetValue(0, 1<<26);
#define NOTE_PIN_LOW()  GPIO_ClearValue(0, 1<<26);

// reset flag
int flag = 1;
int youstick_flag = 0;

// oled width and height;
int oled_max_width = OLED_DISPLAY_WIDTH -3;
int oled_max_height = OLED_DISPLAY_HEIGHT -3;

// sounds
static uint32_t notes[] = {
        2272, // A - 440 Hz
        2024, // B - 494 Hz
        3816, // C - 262 Hz
        3401, // D - 294 Hz
        3030, // E - 330 Hz
        2865, // F - 349 Hz
        2551, // G - 392 Hz
        1136, // a - 880 Hz
        1012, // b - 988 Hz
        1912, // c - 523 Hz
        1703, // d - 587 Hz
        1517, // e - 659 Hz
        1432, // f - 698 Hz
        1275, // g - 784 Hz
};


static void playNote(uint32_t note, uint32_t durationMs) {

    uint32_t t = 0;

    if (note > 0) {

        while (t < (durationMs*1000)) {
            NOTE_PIN_HIGH();
            Timer0_us_Wait(note / 2);
            //delay32Us(0, note / 2);

            NOTE_PIN_LOW();
            Timer0_us_Wait(note / 2);
            //delay32Us(0, note / 2);

            t += note;
        }

    }
    else {
    	Timer0_Wait(durationMs);
        //delay32Ms(0, durationMs);
    }
}

void refresh_led_stripe_red(int n, uint16_t *led_stripe_red){
	// translate integer to hex refresh the leds acording to the integer
	if(n == 0){
		led_stripe_red = 0x0;
	}
	if(n == 1){
		led_stripe_red = 0x1;
	}
	if(n == 2){
		led_stripe_red = 0x3;
	}
	if(n == 3){
		led_stripe_red = 0x7;
	}
	if(n == 4){
		led_stripe_red = 0xf;
	}
	if(n == 5){
		led_stripe_red = 0x1f;
	}
	if(n == 6){
		led_stripe_red = 0x3f;
	}
	if(n == 7){
		 led_stripe_red = 0x7f;
	}
	// turn off all lights
	pca9532_setLeds(0x0, 0x7f);
	// turn on what should be on xD
	pca9532_setLeds(led_stripe_red, 0x0); //lights on
}

void refresh_led_stripe_green(int n, uint16_t *led_stripe_green){
	// translate integer to hex refresh the leds acording to the integer
	if(n == 0){
		led_stripe_green = 0x0000;
	}
	if(n == 1){
		led_stripe_green = 0x8000;
	}
	if(n == 2){
		led_stripe_green = 0xC000;
	}
	if(n == 3){
		led_stripe_green = 0xE000;
	}
	if(n == 4){
		led_stripe_green = 0xF000;
	}
	if(n == 5){
		led_stripe_green = 0xF800;
	}
	if(n == 6){
		led_stripe_green = 0xFC00;
	}
	if(n == 7){
		led_stripe_green = 0xFE00;
	}
	// turn of all leds
	pca9532_setLeds(0x0, 0xff00);
	pca9532_setLeds(led_stripe_green, 0x0); //lights on
}

uint8_t int_to_char(int n){
	if(n == 0){
		return '0';
	}
	if(n == 1){
		return '1';
	}
	if(n == 2){
		return '2';
	}
	if(n == 3){
		return '3';
	}
	if(n == 4){
		return '4';
	}
	if(n == 5){
		return '5';
	}
	if(n == 6){
		return '6';
	}
	if(n == 7){
		return '7';
	}
	if(n == 8){
		return '8';
	}
	if(n == 9){
		return '9';
	}
}

int int_to_seven_value(int n){
	// return integer with value from 1 to 7
	// becouse of 7 led's
	// actualy 8 but we use only 7
	int result=0;

	if(n >= 0 && n<50){
		result = 7;
	}
	if(n >= 50 && n < 100){
		result = 6;
	}
	if(n >= 100 && n < 150){
		result = 5;
	}
	if(n >= 150 && n < 200){
		result = 4;
	}
	if(n >= 200 && n < 400){
		result = 3;
	}
	if(n >= 400 && n < 600){
		result = 2;
	}
	if(n >= 600 && n < 800){
		result = 1;
	}
	if(n >= 800){
		result = 0;
	}
	return result;
}

void drawDot(int ball_x, int ball_y){
	oled_clearScreen(OLED_COLOR_WHITE);
	oled_fillRect(ball_x, ball_y, ball_x + 2, ball_y+ 2, OLED_COLOR_BLACK );
}

void game_over_loop(struct Bomb *bombs, int bomb_size, int number_of_bombs){
	flag =1 ;
	while(flag){
		uint8_t btn1 = ((GPIO_ReadValue(0) >> 4) & 0x01);
		if(btn1==0){
			flag = 0;
		}
		oled_clearScreen(OLED_COLOR_WHITE);
		drawBomb(bombs, bomb_size, number_of_bombs);
		Timer0_Wait(1000);
		oled_clearScreen(OLED_COLOR_WHITE);
		draw_game_over_text();
		Timer0_Wait(1000);
	}
}

void draw_game_over_text(){
	// hardcoded position, it should be relative to the display instead
	int x_positon = 20;
	int y_positon = 30;

    oled_clearScreen(OLED_COLOR_WHITE);
    oled_putString(x_positon, y_positon,  (uint8_t*)"Game Over", OLED_COLOR_BLACK, OLED_COLOR_WHITE);
}

void draw_welcome_message(){
	int x_positon = 20;
	int y_positon = 30;

	oled_clearScreen(OLED_COLOR_WHITE);
	oled_putString(x_positon, y_positon,  (uint8_t*)"Go!!!", OLED_COLOR_BLACK, OLED_COLOR_WHITE);
}

struct Bomb {
   int x_position;
   int y_position;
};

void drawBomb(struct Bomb *bombs, int bomb_size, int number_of_bombs) {
	int i=0;
	for(i=0; i<number_of_bombs; i++){
		oled_fillRect(bombs[i].x_position - bomb_size/2,
						bombs[i].y_position - bomb_size/2,
						bombs[i].x_position + bomb_size/2,
						bombs[i].y_position + bomb_size/2,
						OLED_COLOR_BLACK);
	}
}

void initBombs(struct Bomb *bombs, int number_of_bombs){
	// use random values for x and y!!!
	int i;
	int x;
	int y;
	int r;
	srand(time(NULL));
	for(i=0; i< number_of_bombs; i++){
			x = ((rand()+i*2) % 90);
			y = ((rand()+i*5) % 60);

				bombs[i].x_position = x;
				bombs[i].y_position = y;
	}
}

void init_ball_position(int *ball_x, int *ball_y){
	*ball_x = 1;
	*ball_y = oled_max_height / 2;
}

int check_if_ball_step_on_bomb(struct Bomb *bombs, int ball_x, int ball_y, int bomb_size, int number_of_bombs){
	int result = 10000;

	int i;
	for(i=0; i<number_of_bombs; i++){
		float distance = (ball_x - bombs[i].x_position) * (ball_x - bombs[i].x_position) +
						(ball_y - bombs[i].y_position) * (ball_y - bombs[i].y_position);

			if(distance < bomb_size * bomb_size/2){
				result = 1;
				return result;
			}
//			might not <
			if(distance < result){
				result = distance;
			}
	}

	return result;
}

int check_if_ball_step_on_finish(int finish_x, int finish_y, int ball_x, int ball_y, int bomb_size){
	int result = 0;
	float disatnce = (ball_x - finish_x) * (ball_x - finish_x) +
					(ball_y - finish_y) * (ball_y - finish_y);

	if(disatnce < bomb_size * bomb_size/2){
		result =1;
	}

	return result;
}

void drawFinish(int x, int y){
	oled_circle(x, y, 5, OLED_COLOR_BLACK);
}

void drow_finish_message(){
	while(1){
		oled_clearScreen(OLED_COLOR_WHITE);
		oled_putString(10,25,"Bravo!", OLED_COLOR_BLACK, OLED_COLOR_WHITE);
		Timer0_Wait(1000);
	}
}

static void init_i2c(void)
{
	PINSEL_CFG_Type PinCfg;

	/* Initialize I2C2 pin connect */
	PinCfg.Funcnum = 2;
	PinCfg.Pinnum = 10;
	PinCfg.Portnum = 0;
	PINSEL_ConfigPin(&PinCfg);
	PinCfg.Pinnum = 11;
	PINSEL_ConfigPin(&PinCfg);

	// Initialize I2C peripheral
	I2C_Init(LPC_I2C2, 100000);

	/* Enable I2C1 operation */
	I2C_Cmd(LPC_I2C2, ENABLE);
}

int sound_acording_to_leds(int nled){
	if(nled==7){
		return 10;
	}
	if(nled==6){
			return 20;
		}
	if(nled==5){
			return 30;
		}
	if(nled==4){
			return 40;
		}
	if(nled==3){
			return 70;
		}
	if(nled<=2){
			return 0;
	}
}

struct DefuseKit {
   int x_position;
   int y_position;
};

void draw_defuse(int number_of_defusals, struct DefuseKit *defusals ){
		int i;
		for(i=0; i< number_of_defusals; i++){
			oled_putPixel(defusals[i].x_position, defusals[i].y_position, OLED_COLOR_BLACK);
		}
}

int check_if_ball_step_on_defuse(struct DefuseKit * defusals, int ball_x, int ball_y, int number_of_defusals){
	int result = 10000;

	int i;
	for(i=0; i<number_of_defusals; i++){
		float distance = (ball_x - defusals[i].x_position) * (ball_x - defusals[i].x_position) +
						(ball_y - defusals[i].y_position) * (ball_y - defusals[i].y_position);

			if(distance < 3){
				result = 1;
				return result;
			}
//			might not <
			if(distance < result){
				result = distance;
			}
	}

	return result;
}

void init_defusals(struct DefuseKit *defuses, int number_of_defusals){
	// use random values for x and y!!!
	int i;
	int x;
	int y;
	int r;
	srand(time(NULL));
	for(i=0; i< number_of_defusals; i++){
			x = ((rand()+i*3) % 90);
			y = ((rand()+i*4) % 60);

				defuses[i].x_position = x;
				defuses[i].y_position = y;
	}
}

//void init_defaults(uint16_t *led_red_stripe, uint16_t *led_red_strip){
//	 // turn off led stripes
//	    refresh_led_stripe_red(0, &led_stripe_red);
//
//	    // initialize ball on start position
//	    init_ball_position(&ball_x, &ball_y);
//
//	    // initialize bombs
//	    initBombs(&bombs, number_of_bombs);
//
//	    // initialize defusals
//	    init_defusals(&defuses, number_of_defusals);
//
//	    //display welcome message
//	    draw_welcome_message();
//		playNote(2865, 60);
//		Timer0_Wait(400);
//	    playNote(2865, 60);
//	    Timer0_Wait(400);
//	    playNote(2865, 60);
//	    Timer0_Wait(400);
//	    playNote(2272, 200);
//	    // wait 1 second and draw ball at initial position
//	    //Timer0_Wait(3000);
//	    drawDot(ball_x, ball_y);
//	    drawBomb(&bombs, bomb_size, number_of_bombs);
//	    draw_defuse(number_of_defusals, &defuses);
//}

void play_start_game_sound(){
	playNote(2865, 60);
	Timer0_Wait(400);
    playNote(2865, 60);
    Timer0_Wait(400);
    playNote(2865, 60);
    Timer0_Wait(400);
    playNote(2272, 200);
}

void move_right(int * ball_x, int oleed_max_width){
	*ball_x += 1;
	if(*ball_x > oled_max_width){
		*ball_x = oled_max_width;
	 }
}

void move_left(int * ball_x){
	*ball_x -= 1;
	if(*ball_x < 1){
		*ball_x = 1;
	}
}

void move_down(int * ball_y, int max_height){
	*ball_y += 1;
	if(*ball_y > oled_max_height){
		*ball_y = oled_max_height;
	}
}

void move_up(int * ball_y){
	*ball_y -= 1;
	if(*ball_y < 0){
		*ball_y = 0;
    }
}

void game_over_entry_sound(){
	playNote(2272, 60);
		playNote(2024, 50);
		playNote(2272, 60);
		playNote(3030, 100);
}

void play_entry_finish_sound(){
	playNote(3816, 200);
	Timer0_Wait(200);
	playNote(2551, 250);
	Timer0_Wait(200);
	playNote(1912, 450);
}
void display_keyboard_input_message(){
	oled_putString(1,2,"Hold btn2 to use joystick!",OLED_COLOR_WHITE, OLED_COLOR_BLACK);
	Timer0_Wait(2000);

	uint8_t btn2 = ((GPIO_ReadValue(0) >> 4) & 0x01);
			if(btn2 == 1){
				youstick_flag = 1;
			}
}

int main (void) {
	// variables for acce;erometer
	int32_t xoff = 0;
	int32_t yoff = 0;
	int32_t zoff = 0;

	int8_t acc_x = 0;
	int8_t acc_y = 0;
	int8_t acc_z = 0;

	// coordinates for ball
	int ball_x;
	int ball_y;

	// nuber of leds on ledstrip to flash
    int n_led_red = 0;
    int n_led_green = 0;

    // finish positions;
    int finish_x = 94;
    int finish_y = 30;

    // variables for bombs
    int number_of_bombs =7;
    int bomb_sound_duration=0;
    struct Bomb bombs[number_of_bombs];

    // variables for rdefusal kit
    int number_of_defusals = number_of_bombs;
    struct DefuseKit defuses[number_of_defusals];

    // width and height of hole
    // the hole is a square
    int bomb_size = 5;

    // led stripe
    uint16_t led_stripe_red;
    uint16_t led_stripe_green;

    // variable for joystick input
    uint8_t joy = 0;

	////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////

    // initialize rgb leds, joystick and oled
    // proper libraries must be included before use!!!
    init_i2c();
    acc_init();
    rgb_init();
    joystick_init();
    oled_init();

    // turn off led stripes
    refresh_led_stripe_red(0, &led_stripe_red);

    // initialize ball on start position
    init_ball_position(&ball_x, &ball_y);

    // initialize bombs
    initBombs(&bombs, number_of_bombs);

    // initialize defusals
    init_defusals(&defuses, number_of_defusals);

    //display welcome message
    draw_welcome_message();
    play_start_game_sound();
    display_keyboard_input_message();

    // wait 1 second and draw ball at initial position
    // used for debugging
    drawDot(ball_x, ball_y);
    //drawBomb(&bombs, bomb_size, number_of_bombs);
    //draw_defuse(number_of_defusals, &defuses);

    // asume that the board is at 0g when initialized
    acc_read(&acc_x, &acc_y, &acc_z);
    xoff = 0-acc_x;
    yoff = 0- acc_y;
    zoff = 64- acc_z;

    while(1) {
    	// set 7seg display to the amount of active bombs
    	led7seg_setChar(int_to_char(number_of_bombs), FALSE);

    	playNote(2272, bomb_sound_duration);

    	// use accelerometer
    	if(youstick_flag == 1){
    		// read from accelerometer
    		        acc_read(&acc_x, &acc_y, &acc_z);
    		        acc_x = acc_x+xoff;
    		        acc_y = acc_y+yoff;
    		        acc_z = acc_z+zoff;

    		        // right
    		        if(acc_x < 0){
    		        	move_right(&ball_x, oled_max_width);
    		        }

    		        // left
    		        if(acc_x > 0){
    		        	move_left(&ball_x);
    		        }

    		        // down
    		        if(acc_y > 0){
    		        	move_down(& ball_y, oled_max_height);
    		        }

    		        // up
    		        if(acc_y < 0){
    		        	move_up(& ball_y);
    		        }

    	}
    	// use youstick
    	else{
        	// read from joystick
        	// if it is not pressed at the moment then joy = 0...
            joy = joystick_read();


            // cliar om joystick click
            // used for debuging
            if ((joy & JOYSTICK_CENTER) != 0) {
            	oled_clearScreen(OLED_COLOR_WHITE);
            }

            if ((joy & JOYSTICK_DOWN) != 0) {
            	move_down(& ball_y, oled_max_height);
            }

            if ((joy & JOYSTICK_LEFT) != 0) {
            	move_left(&ball_x);
            }

            if ((joy & JOYSTICK_UP) != 0) {
            	move_up(& ball_y);
            }

            if ((joy & JOYSTICK_RIGHT) != 0) {
            	move_right(&ball_x, oled_max_width);
            }
    	}

        // if key is not pressed don't draw anything, save CPU and batery!
 //       if(joy){

        	int is_steppen_on_bomb = check_if_ball_step_on_bomb(&bombs, ball_x, ball_y, bomb_size, number_of_bombs);
        	if(is_steppen_on_bomb == 1){
        		game_over_entry_sound();
        		game_over_loop(&bombs, bomb_size, number_of_bombs);
//        		init_defaults();
        	}

        	int is_stepped_on_defuse = check_if_ball_step_on_defuse(&defuses, ball_x, ball_y, number_of_defusals);
        	if(is_stepped_on_defuse == 1){
        		playNote(3816, 200);
        		number_of_bombs --;
        		number_of_defusals--;
        		if(number_of_bombs <0){
        			number_of_bombs = 0;
        			number_of_defusals =0;
        		}

        		// give hint by display bombs for 200ms
        		drawBomb(&bombs, bomb_size, number_of_bombs);
        		Timer0_Wait(200);
        		oled_clearScreen(OLED_COLOR_WHITE);
        	}

        	// check_if...() is modified to return the distance to the nearest bomb
        	//oled_putString(1,1,int_to_string(is_steppen_on_bomb),OLED_COLOR_BLACK,OLED_COLOR_WHITE);
        	n_led_red = int_to_seven_value(is_steppen_on_bomb);
        	refresh_led_stripe_red(n_led_red, &led_stripe_red);

        	n_led_green = int_to_seven_value(is_stepped_on_defuse);
        	refresh_led_stripe_green(n_led_green, &led_stripe_green);

        	bomb_sound_duration = sound_acording_to_leds(n_led_red);

        	int is_stepped_on_finish = check_if_ball_step_on_finish(finish_x, finish_y, ball_x, ball_y, bomb_size);
        	if(is_stepped_on_finish == 1){
        		play_entry_finish_sound();
        		drow_finish_message();
        	}
   //     }
       drawDot(ball_x, ball_y);
       //drawBomb(&bombs, bomb_size, number_of_bombs);
       //draw_defuse(number_of_defusals, &defuses);
       drawFinish(finish_x, finish_y);
       Timer0_Wait(100);
    }

}
// main end here
//////////////////////////
