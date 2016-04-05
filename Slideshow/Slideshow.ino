/*
 * Copyright (c) 2014, Majenko Technologies
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 * 
 * * Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 * 
 * * Neither the name of Majenko Technologies nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * Slideshow
 * 
 * Copy the folder "slides" from the "files" folder onto an SD card.
 * This is meant to be run on a picLCD-35T
 */

#include <SPI.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library
#include <SD.h>
#include "BMPFile.h"

#define SD_CS    7  // Chip select line for SD card
#define TFT_CS  5  // Chip select line for TFT display
#define TFT_DC   4  // Data/command line for TFT
#define TFT_RST  -1  // Reset line for TFT (or connect to +5V)

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);


void setup() {
  Serial.begin(115200);
  
	tft.initR(INITR_BLACKTAB);
	tft.setRotation(0);
	tft.fillScreen(ST7735_BLACK);
 
	tft.setCursor(0, 0);
	tft.print("Initializing SD card...");
	if (!SD.begin(SD_CS)) {
		tft.print("failed");
		while(1);
	}
	tft.println("OK");
  File myFile = SD.open("kitten1.bmp");
//  File myFile = SD.open("parrot.bmp");
//  File myFile = SD.open("p_256.bmp");
//  File myFile = SD.open("p_16.bmp");
//  File myFile = SD.open("p_2.bmp");
//  File myFile = SD.open("r_24.bmp");
//  File myFile = SD.open("g_24.bmp");
//  File myFile = SD.open("b_24.bmp");
//  File myFile = SD.open("r_256.bmp");
//  File myFile = SD.open("g_256.bmp");
//  File myFile = SD.open("b_256.bmp");
  BMPFile bmp(myFile);
//  bmp.setBackgroundColor(ST7735_WHITE);
  bmp.draw(&tft, 0, 20);


  Serial.println(bmp.getWidth());
  Serial.println(bmp.getHeight());
  
  myFile.close();

  myFile = SD.open("parrot.bmp");
  tft.fillScreen(ST7735_BLACK);
  if(bmp.open(myFile)) bmp.draw(&tft, 0, 20);
  myFile.close();

  tft.fillScreen(ST7735_BLACK);
  if(bmp.open("r_256.bmp")) bmp.draw(&tft, 0, 20);
  bmp.close();
}

void loop() {
	
}
