/*
  Quick and dirty realtime accelerometer display by wsanders.net
  Press button A to zero out stray static accelerations

  Based on https://github.com/m5stack/M5Stack/tree/master/examples/Advanced/Display/Sprite examples
  A 16 bit Sprite occupies (2 * width * height) bytes in RAM.
  An 8 bit Sprite occupies (width * height) bytes in RAM.
*/

#include <M5StickCPlus.h>

#define MAXX 240
#define MAXY 135
#define TEXTW 32
#define TEXTH 18
//WTF
#define GRAPHW (MAXX-TEXTW)
#define GRAPHH 135
#define SCALE 26.0
#define XCENT 34.0
#define YCENT 66.0
#define ZCENT 100.0

// Enough room for a full frame buffer?
// M5stackC-Plus: 520K RAM = 16k probably not, 8k OK
// Graph all 3 accel values
TFT_eSprite sgraph = TFT_eSprite(&M5.Lcd); 
// One sprite for each axis value on LH of graph
// Sprites that overlap will flicker
TFT_eSprite stextX = TFT_eSprite(&M5.Lcd); 
TFT_eSprite stextY = TFT_eSprite(&M5.Lcd);
TFT_eSprite stextZ = TFT_eSprite(&M5.Lcd);

float accX;
float accY;
float accZ;
float correctionX = 0.0;
float correctionY = 0.0;
float correctionZ = 0.0;
int dot = 0;

void setup() {
    M5.begin();
    M5.IMU.Init();
    M5.Lcd.fillScreen(TFT_BLACK);
    M5.Lcd.setRotation(3); // USB cable to the left
    
    // sprite with the traces
    sgraph.setColorDepth(8);
    sgraph.createSprite(GRAPHW, GRAPHH);
    sgraph.fillSprite(BLUE);  // black default

    // NUMBER: Create a sprite for the numbers. (Font 2 is 16p?)
    stextX.setColorDepth(8);
    stextX.createSprite(32, 18);
    stextX.setTextColor(TFT_WHITE);

    stextY.setColorDepth(8);
    stextY.createSprite(32, 18);
    stextY.setTextColor(TFT_WHITE);

    stextZ.setColorDepth(8);
    stextZ.createSprite(32, 18);
    stextZ.setTextColor(TFT_WHITE);
}

void loop() {

    M5.IMU.getAccelData(&accX, &accY, &accZ);
    accX = accX + correctionX;
    accY = accY + correctionY;
    accZ = accZ + correctionZ;

    stextX.fillSprite(DARKGREY);
    stextY.fillSprite(DARKGREY);
    stextZ.fillSprite(DARKGREY);
    stextX.drawFloat(accX, 1, 2, 2);
    stextY.drawFloat(accY, 1, 2, 2);
    stextZ.drawFloat(accZ, 1, 2, 2);  

    // 1/2 G guidelines
    if (dot == 0) { 
    sgraph.drawPixel(GRAPHW-1, XCENT + SCALE/2, RED);
    sgraph.drawPixel(GRAPHW-1, YCENT + SCALE/2, GREEN);
    sgraph.drawPixel(GRAPHW-1, ZCENT + SCALE/2, YELLOW);
    sgraph.drawPixel(GRAPHW-1, XCENT - SCALE/2, RED);
    sgraph.drawPixel(GRAPHW-1, YCENT - SCALE/2, GREEN);
    sgraph.drawPixel(GRAPHW-1, ZCENT - SCALE/2, YELLOW);
    }
    dot = (dot+1) % 4;
    sgraph.drawPixel(GRAPHW-1, int(XCENT + accX * SCALE ), RED);
    sgraph.drawPixel(GRAPHW-1, int(YCENT + accY * SCALE), GREEN);
    sgraph.drawPixel(GRAPHW-1, int(ZCENT + (accZ) * SCALE), YELLOW);
    sgraph.scroll(-1, 0);   // scroll graph 1 pixel left, 0 up/down

    // text sprite still flicker when pushed on top of graph sprite
    stextX.pushSprite(0, 0);
    stextY.pushSprite(0, 60);
    stextZ.pushSprite(0, 120);
    sgraph.pushSprite(TEXTW, 0);
    M5.update();
    // use wasReleased because code might finish before you release the button!
    if (M5.BtnA.wasReleased()) {
        calibrate();
    }
}

void calibrate() { 
    int i = 1000;
    M5.Lcd.fillScreen(DARKGREY);
    M5.Lcd.setTextColor(ORANGE, DARKGREY);
    M5.Lcd.setCursor(20, 20, 4);
    M5.Lcd.println("CALIBRATE:");
    M5.Lcd.setCursor(20, 50, 2);
    M5.Lcd.println("Put the device face up on a flat, vibration-free surface and press Button A");
    while (! M5.BtnA.wasReleased()) {
        delay(10);
        M5.update();
    }
    delay(500);
    // X and Y should be zero, Z should be 1.0
    while (i > 0) {
        M5.IMU.getAccelData(&accX, &accY, &accZ);
        correctionX = correctionX + accX;
        correctionY = correctionY + accY;
        correctionZ = correctionZ + accZ;
        i--;
    }
    correctionX = -(correctionX / 1000.0);
    correctionY = -(correctionY / 1000.0);
    correctionZ = -(correctionZ / 1000.0);
    M5.Lcd.fillScreen(DARKGREY);
    M5.Lcd.setCursor(20, 20, 4);
    M5.Lcd.println(correctionX);
    M5.Lcd.setCursor(20, 50, 4);
    M5.Lcd.println(correctionY);
    M5.Lcd.setCursor(20, 80, 4);
    M5.Lcd.println(correctionZ);
    delay(1000);   
    M5.Lcd.fillScreen(BLACK);
}
