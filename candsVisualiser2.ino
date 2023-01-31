//MUSIC & CV GRAPHIC VISUALISER - CIRCUITS & SOUNDS (SEBASTIAN YOUNG) 2022


//HOW THE CODE WORKS (BASICALLY...)


//1 - THE STATE OF THE ENCODER/POTS/SWITCHES IS BEING CONSTANTLY MONITORED, WHICH IS USED TO CHANGE BETWEEN & ADJUST THE PARAMETERS OF THE VISUAL

//2 - THE GRAPHIC VISUALS ARE DISPLAYED BY UTILIZING AN INCREASING VARIABLE (i) BETWEEN 0 - 30(MAX BUFFER SIZE - ANYTHING BEYOND THIS MAKES PROGRAM TOO SLOW & UNRESPONSIVE), THAT CHANGES THE PARAMETERS OF THE LINE/CIRCLE FUNCTIONS NATIVE TO THE TFT LIBRARY
// IE:
// tft.drawLine(x0[i], y0[i], x1[i], y1[i], C); 
// tft.fillCircle(x0[i], y0[i], radius[i], C);

// X0[i] = START POINT AT X COORDINATE
// Y0[i] = START POINT AT Y COORDINATE
// X1[i] = END POINT AT X COORDINATE
// Y1[i] = END POINT AT Y COORDINATE
// RADIUS[i] = RADIUS OF CIRCLE
// C = COLOUR (0 TO 65336)

//3- THE SHAPE OF THE VISUAL IS DETERMINED BY UTILIZING THE TRIGONOMETRY FUNCTIONS NATIVE TO ARDUINO 
// IE:
// X0[i] = SIN(i) 
// Y0[i] =  COS(i)

//4 - EVERY LINE/CIRCLE PARAMETER IS BEING CONSTANTLY UPDATED IN A SET OF ARRAYS THE SIZE OF THE MAX BUFFER (ONE SET MONITORS THE CURRENT POSITION & THE OTHER SET MONITORS THE PREVIOUS POSITION)

//5 - IF THE PARAMETERS CHANGE VALUES/POSITIONS AT ANY POINT, THEN THE LINE/CIRCLE AT THAT LOCATION IS BLANKED (SET TO BLACK OR WHITE DEPENDING ON STATE OF INVERSION SWITCH) & A NEW LINE/CIRCLE REPLACES THE OLD ONE IN THE UPDATED POSITION

//6 - ???

//7 - PROFIT

#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SPI.h>             // Arduino SPI library

// ST7789 TFT module connections
#define TFT_CS    10  // define chip select pin
#define TFT_DC     9  // define data/command pin
#define TFT_RST    8  // define reset pin, or set to -1 and connect to Arduino RESET pin

// Initialize Adafruit ST7789 TFT library
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

#define MAX_WIDTH 320
#define MAX_LENGTH 240

#define BUFF_SIZE 30 //MAX NUMBER OF PIXEL ELEMENTS IN VISUAL 

int pot0 = 0; //PARAMETER 1
int pot1 = 0; //PARAMETER 2
int pot2 = 0; //PARAMETER 3
int pot3 = 0; //PARAMETER 4
int pot4 = 0; //PARAMETER 5

int x0[BUFF_SIZE] = {0}; //ARRAY USED TO STORE CURRENT X0 PARAMETER OF VISUAL
int y0[BUFF_SIZE] = {0}; //ARRAY USED TO STORE CURRENT Y0 PARAMETER OF VISUAL
int x1[BUFF_SIZE] = {0}; //ARRAY USED TO STORE CURRENT X1 PARAMETER OF VISUAL
int y1[BUFF_SIZE] = {0}; //ARRAY USED TO STORE CURRENT Y1 PARAMETER OF VISUAL

int x0Prev[BUFF_SIZE] = {0}; //ARRAY USED TO STORE PREVIOUS X0 PARAMETER OF VISUAL
int y0Prev[BUFF_SIZE] = {0}; //ARRAY USED TO STORE PREVIOUS Y0 PARAMETER OF VISUAL
int x1Prev[BUFF_SIZE] = {0}; //ARRAY USED TO STORE PREVIOUS X1 PARAMETER OF VISUAL
int y1Prev[BUFF_SIZE] = {0}; //ARRAY USED TO STORE PREVIOUS Y1 PARAMETER OF VISUAL

int radiusArr[BUFF_SIZE] = {0}; //ARRAY USED TO STORE CURRENT RADIUS PARAMETER
int radiusArrPrev[BUFF_SIZE] = {0}; //ARRAY USED TO STORE PREVIOUS RADIUS PARAMETER


int colour = 0; //VARIABLE TO STORE COLOUR DATA
int radius = 0; //VARIABLE TO STORE CIRCLE RADIUS

int randC = 0; //VARIABLE TO STORE RANDOM COLOUR
int randW = 0; //VARIABLE USED TO INTRODUCE RANDOM WOBBLE/PERTUBATION TO VISUAL IN AUTO MODE, WHEN POSITIVE GOING CLK PULSE IS RECEIVED

int i = 0; //VARIABLE TO STEP THROUGH EACH PIXEL LOCATION


#define  ENCODER_OPTIMIZE_INTERRUPTS
#include <Wire.h>
#include <Encoder.h>

Encoder myEnc(2, 3);
float oldPosition1  = -999;
float newPosition1  = -999;
float oldPosition2  = -999;
float newPosition2  = -999;
float oldPosition3  = -999;
float newPosition3  = -999;

byte enc_switch_in = 0;
unsigned long enc_switch_timer = 0;
bool enc_switch_latch = false;
byte enc_switch_counter = 0;
byte enc_switch = 0;
byte toggle_switch = 0;//0=EXT,1=ON
static int DOUBLE_CLICK_TIME = 500;

byte bank = 0; //VARIABLE TO STORE BANK SELECTION
byte visual = 0; //VARIABLE TO STORE VISUAL SELECTION

bool clockIn = false;
bool oldClockIn = false;

bool visualIn = false;
bool oldVisualIn = false;

bool bankIn = false;
bool oldBankIn = false;

bool modeSw = false;
bool invSw = false;

void setup() {

  tft.init(MAX_LENGTH, MAX_WIDTH);    // Init ST7789 display 320x240 pixel

  // if the screen is flipped, remove this command
  tft.setRotation(3);

  tft.fillScreen(ST77XX_BLACK);

  //  Serial.begin(9600);

  randomSeed(analogRead(A7));

  pinMode(4, INPUT_PULLUP); //ENCODER BUTTON
  pinMode(5, INPUT); //BANK IN
  pinMode(6, INPUT_PULLUP); //MODE SWITCH
  pinMode(7, INPUT_PULLUP); //INVERT SCREEN SWITCH
  pinMode(12, INPUT); //VISUAL IN
  pinMode(18, INPUT); //CLOCK IN
}


void loop() {

  hardwareCheck(); //CHECK VALUES ON ALL I/O
  setTFT(); //DISPLAY SELECTED VISUAL ON TFT SCREEN

}


void hardwareCheck() {


  oldClockIn = clockIn;
  clockIn = digitalRead(18);

  if (oldClockIn == false && clockIn == true) { //CHANGE COLOUR & RANDOM WOBBLE FOR EACH CLK PULSE
    randC = random(0, 65536); //RANDOM COLOUR
    randW = random(0, 75); //RANDOM WOBBLE/PERTUBATION
  }


  oldVisualIn = visualIn;
  visualIn = digitalRead(12);

  if (oldVisualIn == false && visualIn == true  ) { //CHANGE VISUAL TYPE WHEN G8 PULSE IS RECEIVED
    visual++;

    if ( visual < 0) {
      visual = 29;
    }
    else if ( visual >= 30 ) {
      visual = 0;
    }
  }


  oldBankIn = bankIn;
  bankIn = digitalRead(5);

  if (oldBankIn == 0 && bankIn == 1  ) { //CHANGE VISUAL BANK WHEN G8 PULSE IS RECEIVED
    tft.fillScreen(ST77XX_WHITE);
    bank++;

    if ( bank < 0) {
      bank = 1;
    }
    else if ( bank >= 2 ) {
      bank = 0;
    }
  }

  invSw = !digitalRead(7);

  //  if (invSw == 1) {
  //    tft.fillScreen(ST77XX_WHITE);
  //  }
  //
  //  else {
  //    tft.fillScreen(ST77XX_BLACK);
  //  }

  enc_switch_in  = !digitalRead(4);

  if ((enc_switch_in == true) && (enc_switch_latch == false)) {  //Count Times Button Pressed
    enc_switch_counter++;
    enc_switch_timer = millis();
    enc_switch_latch = true;
  }
  else if (enc_switch_in == false) {
    enc_switch_latch = false;
  }

  if ((millis() >= enc_switch_timer + DOUBLE_CLICK_TIME) && (enc_switch_in == false)) {  //Reset Count when no Button Pressed
    enc_switch_counter = 0;
  }

  if ((enc_switch_in == false) && (enc_switch_counter == 0)) {
    enc_switch = 0;
  }

  else if ((enc_switch_in == true) && (enc_switch_counter == 1)) {
    enc_switch = 1;
  }

  else if ((enc_switch_in == true) && (enc_switch_counter >= 2)) {
    enc_switch = 2;
  }

  if (enc_switch  == 0) {//SELECT VISUAL TYPE

    newPosition1 = myEnc.read();
    if ( (newPosition1 - 3) / 4  > oldPosition1 / 4) {
      oldPosition1 = newPosition1;
      visual = visual - 1;
    }

    else if ( (newPosition1 + 3) / 4  < oldPosition1 / 4 ) {
      oldPosition1 = newPosition1;
      visual = visual + 1;
    }

    if (bank == 0) {
      if ( visual < 0) {
        visual = 17;
      }
      else if ( visual >= 18 ) {
        visual = 0;
      }
    }

    else if (bank == 1) {
      if ( visual < 0) {
        visual = 29;
      }
      else if ( visual >= 30 ) {
        visual = 0;
      }
    }

  }


  if (enc_switch  == 1) {//SELECT VISUAL BANK
    newPosition2 = myEnc.read();
    if ( (newPosition2 - 3) / 4  > oldPosition2 / 4) {
      oldPosition2 = newPosition2;
      bank = bank - 1;


      if (invSw == 1) {
        tft.fillScreen(ST77XX_WHITE);
      }

      else {
        tft.fillScreen(ST77XX_BLACK);
      }

    }

    else if ( (newPosition2 + 3) / 4  < oldPosition2 / 4 ) {
      oldPosition2 = newPosition2;
      bank = bank + 1;

      if (invSw == 1) {
        tft.fillScreen(ST77XX_WHITE);
      }

      else {
        tft.fillScreen(ST77XX_BLACK);
      }
    }

    if ( bank < 0) {
      bank = 1;
    }
    else if ( bank >= 2 ) {
      bank = 0;
    }
  }

  modeSw = !digitalRead(6);

  if (modeSw == true) { //MANUAL MODE
    switch (bank) {

      case 0: //STRINGBALL VISUALS
        pot0 = map(analogRead(A0), 0, 1023, 0, 100); //X0
        pot1 = map(analogRead(A1), 0, 1023, 0, 100);  //Y0
        pot2 = map(analogRead(A2), 0, 1023, 0, 100); //X1
        pot3 = map(analogRead(A3), 0, 1023, 0, 100);//Y1
        pot4 = map(analogRead(A5 - 512), 0, 1023, 50, 190);//POSITION
        break;

      case 1: //SPIROGRAPH VISUALS
        pot0 = map(analogRead(A0), 0, 1023, 0, 100); //X0
        pot1 = map(analogRead(A1), 0, 1023, 0, 100);  //Y0
        pot2 = map(analogRead(A2), 0, 1023, 50, 270); //POSITION X
        pot3 = map(analogRead(A3), 0, 1023, 50, 190);//POSITION Y
        pot4 = map(analogRead(A5), 0, 1023, 0, 50);//RADIUS
        break;

      default:
        pot0 = map(analogRead(A0), 0, 1023, 0, 100); //X0
        pot1 = map(analogRead(A1), 0, 1023, 0, 100);  //Y0
        pot2 = map(analogRead(A2), 0, 1023, 0, 100); //X1
        pot3 = map(analogRead(A3), 0, 1023, 0, 100);//Y1
        pot4 = map(analogRead(A5 - 512), 0, 1023, 50, 190);//POSITION
        break;
    }
  }

  else { //AUTO MODE
    switch (bank) {

      case 0: //STRINGBALL VISUALS
        pot0 = map((analogRead(A5) - 512), 0, 1023, 0, 100) - randW; //X0
        pot1 = map((analogRead(A5) + 512), 0, 1023, 100, 0) + randW; //Y0
        pot2 = map((analogRead(A5) - 512), 0, 1023, 0, 100) + randW; //X1
        pot3 = map((analogRead(A5) + 512), 0, 1023, 100, 0) - randW; //Y1
        pot4 = map((analogRead(A5) - 512), 0, 1023, 120, 240); //POSITION
        break;

      case 1: //SPIROGRAPH VISUALS
        pot0 = map((analogRead(A5) - 512), 0, 1023, 0, 100) + (randW / 2); //X0
        pot1 = map((analogRead(A5) + 512), 0, 1023, 100, 0) + (randW / 2); //Y0
        pot2 = map((analogRead(A5) - 512), 0, 1023, 120, 240); //POSITION X
        pot3 = map((analogRead(A5) - 512), 0, 1023, 120, 240); //POSITION Y
        pot4 = map((analogRead(A5) - 512), 0, 1023, 0, 50) + (randW / 4); //RADIUS
        break;

      default:
        pot0 = map((analogRead(A5) - 512), 0, 1023, 0, 100) - randW; //X0
        pot1 = map((analogRead(A5) + 512), 0, 1023, 100, 0) + randW; //Y0
        pot2 = map((analogRead(A5) - 512), 0, 1023, 0, 100) + randW; //X1
        pot3 = map((analogRead(A5) + 512), 0, 1023, 100, 0) - randW; //Y1
        pot4 = map((analogRead(A5) - 512), 0, 1023, 120, 240); //POSITION
        break;
    }
  }
}


void setTFT() {

  i++; //INCREASE PIXEL LOCATION

  if (i >= BUFF_SIZE) { //IF PIXEL LOCATION IS MORE THAN BUFFER SIZE (30) THEN RESET BACK TO 0
    i = 0;
  }


  //**TRIG FUNCTIONS ARE USED TO DETERMINE VISUAL TYPE, AS THE PIXEL LOCATION INCREASES
  switch (bank) {

    case 0: //STRINGBALL VISUALS

      switch (visual) {

        case 0:
          x0[i] = sin(i) * pot0 + pot4;
          y0[i] = sin(i) * pot1 + pot4;
          x1[i] = sin(i) * pot2 + pot4;
          y1[i] = cos(i) * pot3 + pot4;
          break;

        case 1:
          x0[i] = sin(i) * pot0 + pot4;
          y0[i] = sin(i) * pot1 + pot4;
          x1[i] = sin(i) * pot2 + pot4;
          y1[i] = tan(i) * pot3 + pot4;
          break;

        case 2:
          x0[i] = sin(i) * pot0 + pot4;
          y0[i] = sin(i) * pot1 + pot4;
          x1[i] = cos(i) * pot2 + pot4;
          y1[i] = sin(i) * pot3 + pot4;
          break;

        case 3:
          x0[i] = sin(i) * pot0 + pot4;
          y0[i] = sin(i) * pot1 + pot4;
          x1[i] = cos(i) * pot2 + pot4;
          y1[i] = cos(i) * pot3 + pot4;
          break;

        case 4:
          x0[i] = sin(i) * pot0 + pot4;
          y0[i] = sin(i) * pot1 + pot4;
          x1[i] = cos(i) * pot2 + pot4;
          y1[i] = tan(i) * pot3 + pot4;
          break;

        case 5:
          x0[i] = sin(i) * pot0 + pot4;
          y0[i] = sin(i) * pot1 + pot4;
          x1[i] = cos(i) * pot2 + pot4;
          y1[i] = tanh(i) * pot3 + pot4;
          break;

        case 6:
          x0[i] = sin(i) * pot0 + pot4;
          y0[i] = sin(i) * pot1 + pot4;
          x1[i] = tan(i) * pot2 + pot4;
          y1[i] = sin(i) * pot3 + pot4;
          break;

        case 7:
          x0[i] = sin(i) * pot0 + pot4;
          y0[i] = sin(i) * pot1 + pot4;
          x1[i] = tan(i) * pot2 + pot4;
          y1[i] = cos(i) * pot3 + pot4;
          break;

        case 8:
          x0[i] = sin(i) * pot0 + pot4;
          y0[i] = sin(i) * pot1 + pot4;
          x1[i] = tan(i) * pot2 + pot4;
          y1[i] = tan(i) * pot3 + pot4;
          break;

        case 9:
          x0[i] = sin(i) * pot0 + pot4;
          y0[i] = sin(i) * pot1 + pot4;
          x1[i] = tan(i) * pot2 + pot4;
          y1[i] = tanh(i) * pot3 + pot4;
          break;

        case 10:
          x0[i] = sin(i) * pot0 + pot4;
          y0[i] = sin(i) * pot1 + pot4;
          x1[i] = tanh(i) * pot2 + pot4;
          y1[i] = sin(i) * pot3 + pot4;
          break;

        case 11:
          x0[i] = sin(i) * pot0 + pot4;
          y0[i] = sin(i) * pot1 + pot4;
          x1[i] = tanh(i) * pot2 + pot4;
          y1[i] = cos(i) * pot3 + pot4;
          break;

        case 12:
          x0[i] = sin(i) * pot0 + pot4;
          y0[i] = sin(i) * pot1 + pot4;
          x1[i] = tanh(i) * pot2 + pot4;
          y1[i] = tan(i) * pot3 + pot4;
          break;

        case 13:
          x0[i] = sin(i) * pot0 + pot4;
          y0[i] = sin(i) * pot1 + pot4;
          x1[i] = tanh(i) * pot2 + pot4;
          y1[i] = tanh(i) * pot3 + pot4;
          break;

        case 14:
          x0[i] = sin(i) * pot0 + pot4;
          y0[i] = cos(i) * pot1 + pot4;
          x1[i] = sin(i) * pot2 + pot4;
          y1[i] = sin(i) * pot3 + pot4;
          break;

        case 15:
          x0[i] = sin(i) * pot0 + pot4;
          y0[i] = cos(i) * pot1 + pot4;
          x1[i] = sin(i) * pot2 + pot4;
          y1[i] = cos(i) * pot3 + pot4;
          break;

        case 16:
          x0[i] = sin(i) * pot0 + pot4;
          y0[i] = cos(i) * pot1 + pot4;
          x1[i] = sin(i) * pot2 + pot4;
          y1[i] = tan(i) * pot3 + pot4;
          break;

        case 17:
          x0[i] = sin(i) * pot0 + pot4;
          y0[i] = cos(i) * pot1 + pot4;
          x1[i] = sin(i) * pot2 + pot4;
          y1[i] = tanh(i) * pot3 + pot4;
          break;


        default:
          x0[i] = sin(i) * pot0 + pot4;
          y0[i] = cos(i) * pot1 + pot4;
          x1[i] = sin(i) * pot2 + pot4;
          y1[i] = cos(i) * pot3 + pot4;
          break;
      }

      radius = 3;

      tft.drawLine(x0[i], y0[i], x1[i], y1[i], randC); //DRAW LINE AT EACH PIXEL LOCATION
      tft.fillCircle(x0[i], y0[i],  radius, randC - 1000); //DRAW FILLED CIRCLE AT START POINT OF PIXEL LOCATION
      tft.fillCircle(x1[i], y1[i],  radius, randC - 4000); //DRAW FILLED CIRCLE AT END POINT OF PIXEL LOCATION


      //**BLANKING OF PIXELS THAT HAVE MOVED LOCATIONS**//
      if (x0[i] != x0Prev[i] || y0[i] != y0Prev[i] || x1[i] != x1Prev[i] || y1[i] != y1Prev[i]) { //IF ANY!! OF THE PIXEL LOCATIONS DO NOT MATCH THE PREVIOUS LOCATION

        if (invSw == 1) { //IF SCREEN IS INVERTED
          tft.drawLine(x0Prev[i], y0Prev[i], x1Prev[i], y1Prev[i], ST77XX_WHITE); //FILL LINE WITH WHITE
          tft.fillCircle(x0Prev[i], y0Prev[i],  radius, ST77XX_WHITE); //FILL CIRCLE WITH WHITE
          tft.fillCircle(x1Prev[i], y1Prev[i],  radius, ST77XX_WHITE); //FILL CIRCLE WITH WHITE
        }

        else { //ELSE IF SCREEN IS NOT INVERTED
          tft.drawLine(x0Prev[i], y0Prev[i], x1Prev[i], y1Prev[i], ST77XX_BLACK); //FILL LINE WITH BLACK
          tft.fillCircle(x0Prev[i], y0Prev[i],  radius, ST77XX_BLACK); //FILL CIRCLE WITH BLACK
          tft.fillCircle(x1Prev[i], y1Prev[i],  radius, ST77XX_BLACK); //FILL CIRCLE WITH BLACK
        }

        //SET PREVIOUS PIXEL LOCATIONS TO CURRENT PIXEL LOCATIONS
        x0Prev[i] = x0[i];
        y0Prev[i] = y0[i];
        x1Prev[i] = x1[i];
        y1Prev[i] = y1[i];
      }
      break;


    //**TRIG FUNCTIONS ARE USED TO DETERMINE VISUAL TYPE, AS THE PIXEL LOCATION INCREASES
    case 1: //SPIROGRAPH VISUALS

      switch (visual) {

        case 0:
          x0[i] = sin(i) * pot0 + pot2;
          y0[i] =  cos(i) * pot1 + pot3;
          radiusArr[i] = sin(i) * pot4;
          break;

        case 1:
          x0[i] = sin(i) * pot0 + pot2;
          y0[i] =  cos(i) * pot1 + pot3;
          radiusArr[i] = cos(i) * pot4;
          break;

        case 2:
          x0[i] = sin(i) * pot0 + pot2;
          y0[i] =  cos(i) * pot1 + pot3;
          radiusArr[i] = tan(i) * pot4;
          break;

        case 3:
          x0[i] = sin(i) * pot0 + pot2;
          y0[i] =  cos(i) * pot1 + pot3;
          radiusArr[i] = tanh(i) * pot4;
          break;

        case 4:
          x0[i] = sin(i) * pot0 + pot2;
          y0[i] =  tan(i) * pot1 + pot3;
          radiusArr[i] = sin(i) * pot4;
          break;

        case 5:
          x0[i] = sin(i) * pot0 + pot2;
          y0[i] =  tan(i) * pot1 + pot3;
          radiusArr[i] = cos(i) * pot4;
          break;

        case 6:
          x0[i] = sin(i) * pot0 + pot2;
          y0[i] =  tan(i) * pot1 + pot3;
          radiusArr[i] = tan(i) * pot4;
          break;

        case 7:
          x0[i] = sin(i) * pot0 + pot2;
          y0[i] =  tan(i) * pot1 + pot3;
          radiusArr[i] = tanh(i) * pot4;
          break;

        case 8:
          x0[i] = sin(i) * pot0 + pot2;
          y0[i] =  tanh(i) * pot1 + pot3;
          radiusArr[i] = sin(i) * pot4;
          break;

        case 9:
          x0[i] = cos(i) * pot0 + pot2;
          y0[i] =  sin(i) * pot1 + pot3;
          radiusArr[i] = sin(i) * pot4;
          break;

        case 10:
          x0[i] = cos(i) * pot0 + pot2;
          y0[i] =  sin(i) * pot1 + pot3;
          radiusArr[i] = cos(i) * pot4;
          break;

        case 11:
          x0[i] = cos(i) * pot0 + pot2;
          y0[i] =  sin(i) * pot1 + pot3;
          radiusArr[i] = tan(i) * pot4;
          break;

        case 12:
          x0[i] = cos(i) * pot0 + pot2;
          y0[i] =  sin(i) * pot1 + pot3;
          radiusArr[i] = tanh(i) * pot4;
          break;

        case 13:
          x0[i] = cos(i) * pot0 + pot2;
          y0[i] =  tan(i) * pot1 + pot3;
          radiusArr[i] = sin(i) * pot4;
          break;

        case 14:
          x0[i] = cos(i) * pot0 + pot2;
          y0[i] =  tan(i) * pot1 + pot3;
          radiusArr[i] = cos(i) * pot4;
          break;

        case 15:
          x0[i] = cos(i) * pot0 + pot2;
          y0[i] =  tan(i) * pot1 + pot3;
          radiusArr[i] = tan(i) * pot4;
          break;

        case 16:
          x0[i] = cos(i) * pot0 + pot2;
          y0[i] =  tan(i) * pot1 + pot3;
          radiusArr[i] = tanh(i) * pot4;
          break;

        case 17:
          x0[i] = tan(i) * pot0 + pot2;
          y0[i] =  sin(i) * pot1 + pot3;
          radiusArr[i] = sin(i) * pot4;
          break;

        case 18:
          x0[i] = tan(i) * pot0 + pot2;
          y0[i] =  sin(i) * pot1 + pot3;
          radiusArr[i] = cos(i) * pot4;
          break;

        case 19:
          x0[i] = tan(i) * pot0 + pot2;
          y0[i] =  sin(i) * pot1 + pot3;
          radiusArr[i] = tan(i) * pot4;
          break;

        case 20:
          x0[i] = tan(i) * pot0 + pot2;
          y0[i] =  sin(i) * pot1 + pot3;
          radiusArr[i] = tanh(i) * pot4;
          break;

        case 21:
          x0[i] = tan(i) * pot0 + pot2;
          y0[i] =  cos(i) * pot1 + pot3;
          radiusArr[i] = sin(i) * pot4;
          break;

        case 22:
          x0[i] = tan(i) * pot0 + pot2;
          y0[i] =  cos(i) * pot1 + pot3;
          radiusArr[i] = cos(i) * pot4;
          break;

        case 23:
          x0[i] = tan(i) * pot0 + pot2;
          y0[i] =  cos(i) * pot1 + pot3;
          radiusArr[i] = tan(i) * pot4;
          break;

        case 24:
          x0[i] = tan(i) * pot0 + pot2;
          y0[i] =  cos(i) * pot1 + pot3;
          radiusArr[i] = tanh(i) * pot4;
          break;

        case 25:
          x0[i] = tan(i) * pot0 + pot2;
          y0[i] =  tan(i) * pot1 + pot3;
          radiusArr[i] = tanh(i) * pot4;
          break;

        case 26:
          x0[i] = tan(i) * pot0 + pot2;
          y0[i] =  tanh(i) * pot1 + pot3;
          radiusArr[i] = tan(i) * pot4;
          break;

        case 27:
          x0[i] = tan(i) * pot0 + pot2;
          y0[i] =  tanh(i) * pot1 + pot3;
          radiusArr[i] = tanh(i) * pot4;
          break;

        case 28:
          x0[i] = tanh(i) * pot0 + pot2;
          y0[i] =  sin(i) * pot1 + pot3;
          radiusArr[i] = sin(i) * pot4;
          break;

        case 29:
          x0[i] = tanh(i) * pot0 + pot2;
          y0[i] =  tan(i) * pot1 + pot3;
          radiusArr[i] = tanh(i) * pot4;
          break;


        default:
          x0[i] = sin(i) * pot0 + pot2;
          y0[i] =  sin(i) * pot1 + pot3;
          radiusArr[i] = cos(i) * pot4;
          break;
      }

      tft.drawCircle(x0[i], y0[i],  radiusArr[i], randC); //DRAW CIRCLES AT EACH PIXEL LOCATION


      //**BLANKING OF PIXELS THAT HAVE MOVED LOCATIONS**//
      if (x0[i] != x0Prev[i] || y0[i] != y0Prev[i] || radiusArr[i] != radiusArrPrev[i]) { //IF ANY!! OF THE PIXEL LOCATIONS DO NOT MATCH THE PREVIOUS LOCATION

        if (invSw == 1) { //IF SCREEN IS INVERTED
          tft.drawCircle(x0Prev[i], y0Prev[i],  radiusArrPrev[i], ST77XX_WHITE); //FILL CIRCLE WITH WHITE
        }

        else { //ELSE IF SCREEN IS NOT INVERTED
          tft.drawCircle(x0Prev[i], y0Prev[i],  radiusArrPrev[i], ST77XX_BLACK); //FILL CIRCLE WITH BLACK
        }

        //SET PREVIOUS PIXEL LOCATIONS TO CURRENT PIXEL LOCATIONS
        x0Prev[i] = x0[i];
        y0Prev[i] = y0[i];
        radiusArrPrev[i] = radiusArr[i];
      }
      break;

  }

}
