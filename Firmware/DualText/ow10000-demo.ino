/*
 +-------------------------------------------------+
 |:.      OpenWest 2016 Electronic Badges        .:|
 +-------------------------------------------------+
  \|                                             |/
   |  Code Contributors / Monkeys:               |
   |   d3c4f       d3c4f@sausage.land            |
   |     - Hardware Abstraction                  |
   |     - Text Library                          |
   |     - Nibble                                |
   |                                             |
   |  Hardware Engineering:                      |
   |   devino      devino@sausage.land           |
   |                                             |
  /|                                             |\
 +-+--------+---------------------------+--------+-+
 |:         |   Files and Information:  |         :|
 |          |  github.com/thetransistor |          |
 |          |     thetransistor.com     |          |
 |:.        |:.      openwest.org     .:|        .:|
 +----------+---------------------------+----------+
*/


// Include the librarys, and instantiate a global instance of badge
#include "ow10000-hardware.h"
#include "ow10000-text.h"
#include "ow10000-nibble.h"
#include "ow10000-tetris.h"
#include <EEPROM.h>

OW10000HAL badge;
OW10000_text scrollingText1(&badge);
OW10000_text scrollingText2(&badge);
OW10000_tetris tetrisGame(&badge);
OW10000_nibble nibbleGame(&badge);

const unsigned int EEPROM_BYTES = 1024;      // Number of EEPROM bytes
const unsigned int E_SIGNATURE = 0;          // Offset for Signature
const unsigned int E_VERSION = 1;            // Offset for Version
const unsigned int E_BRIGHTNESS = 2;         // Offset for brightness setting
const unsigned int E_TEXT1_LENGTH = 3;       // Offset for number of Characters stored for Text1
const unsigned int E_TEXT1_START = 4;        // Offset for First Character of Text1
const unsigned int E_TEXT1_END = 260;        // Offset for Last Character of Text1
const unsigned int E_TEXT2_LENGTH = 261;     // Offset for Number of Characters stored for Text2
const unsigned int E_TEXT2_START = 262;      // Offset for First Character of Text2
const unsigned int E_TEXT2_END = 518;        // Offset for Last Character of Text2

const unsigned int SIGNATURE = 42;       // Signature
const unsigned int VERSION = 2;          // Version

String text1;  // String for button1 / default
String text2;  // String for button2
String text_temp;  // String for misc

// Initialize Stuffs. Mostly the timer interupt. Badge hardware is initialized in the badge library.
void setup() {
	// TIMER1 SETUP
	noInterrupts();           // disable all interrupts
	TCCR1A = 0;               // set entire register to 0
	TCCR1B = 0;               // set entire register to 0
	TCNT1 = 0;                // reset the 16-bit timer counter
	OCR1A = 180;               // compare match register
	TCCR1B |= (1 << WGM12);   // CTC mode (Clear timer on compare match)
	//TCCR1B |= (1 << CS10);    // CS10 and CS11 set = 64 prescaler
	TCCR1B |= (1 << CS11);    // CS11 only set = 8 prescaler
	TIMSK1 |= (1 << OCIE1A);  // enable timer compare interrupt
	interrupts();

	scrollingText1.setBounce(false);
	scrollingText1.setDiminsions(0,0,15);

  scrollingText2.setBounce(false);
  scrollingText2.setDiminsions(0,8,16);
  
  scrollingText1.setScrollRate(45);  // Fast Scroll Rate
  scrollingText2.setScrollRate(30);
	
	setupEEPROM();
	
	scrollingText1.setTextString(text1);
  scrollingText2.setTextString(text2);

	badge.clear();
}


// Read in settings from NVM to RAM
// and initialize if needed
void setupEEPROM(){
	byte temp = 0;
	
	if(EEPROM.read(E_SIGNATURE) == SIGNATURE && EEPROM.read(E_VERSION) == VERSION){
		// (Badge is all up to date)
		// It's a UNIX system! I know this!
		// Initialize
		
		// Read in Frame Drop / Brightness / Battery Save / whatever you want to call it
		badge.setDropFrames(EEPROM.read(E_BRIGHTNESS));

		// Read in Text 1
		temp = EEPROM.read(E_TEXT1_LENGTH);
		for(int x = 0; x < temp; x++){
			text1 += (char) EEPROM.read(E_TEXT1_START + x);
		}

		// Read in Text 2
		temp = EEPROM.read(E_TEXT2_LENGTH);
		for(int x = 0; x < temp; x++){
			text2 += (char) EEPROM.read(E_TEXT2_START + x);
		}
	} else {
		// New Badge, Setup
		text1 = " OpenWest 2016 ";
    text2 = " theTransistor@github.com ";
		
		// Write the current version / signature
		EEPROM.write(E_SIGNATURE, (byte) SIGNATURE);
		EEPROM.write(E_VERSION, (byte) VERSION);

		// Save the brightness
		badge.setDropFrames(0b0000010);
		EEPROM.write(E_BRIGHTNESS, (byte) 0b00000010);
		
		// Write the default text strings out
		saveText1();
		saveText2();
	}
}


// Save text1 to the EEPROM
void saveText1(){
	unsigned int length = text1.length();
	
	if(length > E_TEXT1_END - E_TEXT1_START){
		length = E_TEXT1_END - E_TEXT1_START;
	} 
	EEPROM.write(E_TEXT1_LENGTH, (byte) length);
	
	for(int x = 0; x < length; x++){
		EEPROM.write(E_TEXT1_START + x, (byte) text1[x]);
	}
}


// Save text2 to the EEPROM
void saveText2(){
	unsigned int length = text2.length();

	if(length > E_TEXT2_END - E_TEXT2_START){
		length = E_TEXT2_END - E_TEXT2_START;
	} 
	EEPROM.write(E_TEXT2_LENGTH, (byte) length);
	
	for(int x = 0; x < length; x++){
		EEPROM.write(E_TEXT2_START + x, (byte) text2[x]);
	}
}


// For diagnostics only, delete later
int freeRam () {
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}


// Edit a string
String stringEditor(String newText){
	badge.clear();
	badge.setPixel (6,12,1);
	badge.setPixel (7,12,1);
	badge.setPixel (8,12,1);
	badge.setPixel (9,12,1);
	badge.setPixel (10,12,1);
	
	if(newText.length() < 1){
		newText = " ";
	}

	scrollingText1.setDiminsions(0,4,15);
	scrollingText1.setTextString(newText);
	unsigned int currentCharacter = 0;
	
	scrollingText1.jumpToOffset((currentCharacter * 6) + 10);
	
	while(!badge.buttonA_debounce()){
		
		if( badge.buttonB_debounce() ){
			// Delete the current character
			if(newText.length() > 1){
				newText = newText.substring(0,currentCharacter) + newText.substring(currentCharacter+1);
				
				if(currentCharacter > 0){
					currentCharacter --;
				}
				scrollingText1.setTextString(newText);
				scrollingText1.jumpToOffset((currentCharacter * 6)+10);
			}
		}
		
		// Change Current Character - 1
		if( badge.buttonU_repeat(85) ){
			if((int)newText[currentCharacter] > 32 ){
				newText[currentCharacter] = newText[currentCharacter] - 1;
			} else {
				newText[currentCharacter] = (32 + 95);
			}
			scrollingText1.setTextString(newText);
			scrollingText1.jumpToOffset((currentCharacter * 6)+10);
		}
		
		// Change Current Character + 1
		if( badge.buttonD_repeat(85) ){
			if((int)newText[currentCharacter] < (32 + 95) ){
				newText[currentCharacter] = newText[currentCharacter] + 1;
			} else {
				newText[currentCharacter] = 32;
			}
			scrollingText1.setTextString(newText);
			scrollingText1.jumpToOffset((currentCharacter * 6)+10);
		}

		// Previous Character in String
		if( badge.buttonL_debounce(10) ){
			if(currentCharacter > 0){
				currentCharacter--;
			} else {
				currentCharacter = 0;
			}
			scrollingText1.setTextString(newText);
			scrollingText1.jumpToOffset((currentCharacter * 6)+10);
		}

		// Next Character in String
		if( badge.buttonR_debounce(10) ){
			currentCharacter++;
			
			// Create a new Character, as needed
			if( currentCharacter >= newText.length()){
				newText += " ";
				//currentCharacter = newText.length();
			}
			scrollingText1.setTextString(newText);
			scrollingText1.jumpToOffset((currentCharacter * 6)+10);
		}
	}
	
	badge.clear();
  scrollingText1.setDiminsions(0,0,15);
	return newText;
}


// Infinity...
void loop() {
	scrollingText1.update();
  scrollingText2.update();
	
	if(badge.buttonA_debounce()){
		badge.clear();
		scrollingText1.setTextString(text1);
    scrollingText2.setTextString(text2);
	}
	
	if(badge.buttonB_debounce()){
		badge.clear();
		scrollingText1.setTextString(text2);
    scrollingText2.setTextString(text1);
	}

  if(badge.buttonU_debounce()){
    badge.clear();
    scrollingText1.setTextString(text1);
    scrollingText2.setTextString(text2);
  }
	if(badge.buttonD_debounce()){
		badge.clear();
		text_temp =  "Battery: ";
		scrollingText1.setTextString(text_temp);
    text_temp = String(badge.battery_level())  + "%  ";
    scrollingText2.setTextString(text_temp);
	}
	
	if(badge.buttonR()){
		scrollingText1.setScrollRate(35);  // Slow Scroll Rate
    scrollingText2.setScrollRate(20);
	} else {
	
	}

  if(badge.buttonL()){
    scrollingText1.setScrollRate(65);  // Fast Scroll Rate
    scrollingText2.setScrollRate(40);
  } else {
  
  }
  
	// Ahh yeah, Menutime!
	if(badge.buttonAB_debounce(200)) {
		switch(menu()){
			case(1):    // Customize Text 1
				text1 = stringEditor(text1);
				saveText1();
				break;
			
			case(2):    // Clear Text 1
				text1 = "";
				break;
			
			case(3):    // Customize Text 2
				text2 = stringEditor(text2);
				saveText2();
				break;
			
			case(4):    // Clear Text 2
				text2 = "";
				break;
			
			case(5):    // Play Tetris
				tetrisGame.play();
				break;	
				
			case(6):    // Play Nibble
				nibbleGame.play();  // Play the nibbles!
				break;
				
			case(7):    // Set Brightness
				setBadgeBright();	
				break;

			case(0):    // Back to default
			default:
				break;
			
		}
    scrollingText1.setTextString(text1);
    scrollingText2.setTextString(text2);
		badge.clear();
	}
}


// Menu
// 0 - Exit Menu
// 1 - Customize Text 1 (Default)
// 2 - Clear Text 1
// 3 - Customize Text 2
// 4 - Clear Text 2
// 5 - Play Tetris
// 6 - Play Nibble
// 7 - Set Brightness
unsigned int menu(){
	long tempMillis;
	
	unsigned int currentSelection = 1;
	setMenuText(currentSelection);

	// Display Menu welcome for a bit
	tempMillis = millis();
	scrollingText1.setBounce(false);
	scrollingText1.setDiminsions(0,4,15);

	// Main Menu Loop
	while(!badge.buttonA_debounce()){
		scrollingText1.update();
		
		// Move up the menu
		if(badge.buttonU_debounce(85)){
			if(currentSelection<=0){
				currentSelection = 7;
			} else {
				currentSelection--;
			}
			setMenuText(currentSelection);
		}
		
		// Move down the menu
		if(badge.buttonD_debounce(85)){
			if(currentSelection >= 7){
				currentSelection = 0;
			} else {
				currentSelection++;
			}
			setMenuText(currentSelection);
		}

		if(badge.buttonR()){
			scrollingText1.setScrollRate(65);  // Fast Scroll Rate
      scrollingText2.setScrollRate(40);  // Fast Scroll Rate
		} else {
			
		}

    if(badge.buttonL()) {
      scrollingText1.setScrollRate(35);  // Normal Scroll Rate
      scrollingText2.setScrollRate(20);  // Normal Scroll Rate
    } else {

    }
	}
	scrollingText1.setDiminsions(0,0,15);
	badge.clear();
	return currentSelection;
}

// Display the correct menu text
void setMenuText(int textSelection) {
	char menuText1[] = "1) Customize Text1   ";
	char menuText2[] = "2) Clear Text1   ";
	char menuText3[] = "3) Customize Text2   ";
	char menuText4[] = "4) Clear Text2   ";
	char menuText5[] = "5) Tetris   ";
	char menuText6[] = "6) Nibble   ";
	char menuText7[] = "7) Set Brightness   ";
	char menuText0[] = "Exit Menu  ";
	
	switch (textSelection){
		case 1:
			scrollingText1.setTextString(menuText1);
			break;
		case 2:
			scrollingText1.setTextString(menuText2);
			break;
		case 3:
			scrollingText1.setTextString(menuText3);
			break;
		case 4:
			scrollingText1.setTextString(menuText4);
			break;
		case 5:
			scrollingText1.setTextString(menuText5);
			break;
		case 6:
			scrollingText1.setTextString(menuText6);
			break;
		case 7:
			scrollingText1.setTextString(menuText7);
			break;
		case 0:
		default:
			scrollingText1.setTextString(menuText0);
			break;
	}
}


// Set the badge brightness
void setBadgeBright(){
	bool selectionMade = false;
	unsigned int currentSelection = badge.getDropFrames();
	setBadgeBrightnessDisplay(currentSelection);
	
	while(!selectionMade){
		selectionMade = badge.buttonA_debounce();
		
		// Move up the menu
		if(badge.buttonD_repeat(120)){
			if(currentSelection >= 15){
				currentSelection = 15;
			} else {
				currentSelection++;
			}
			badge.setDropFrames(currentSelection);
			setBadgeBrightnessDisplay(currentSelection);
		}
		
		// Move down the menu
		if(badge.buttonU_repeat(120)){
			if(currentSelection<=0){
				currentSelection = 0;
			} else {
				currentSelection--;
			}
			badge.setDropFrames(currentSelection);
			setBadgeBrightnessDisplay(currentSelection);
		}
	}
	
	// Write to NVM
	EEPROM.write(E_BRIGHTNESS, (byte) currentSelection);
}


// Set the text for the brightness menu
void setBadgeBrightnessDisplay(unsigned int selection){
	selection = 15 - selection;
	badge.clear();
	for (int x = selection; x >= 0; x--){
		badge.frameBuffer[15-x][0] = 0xFFFF;
		badge.frameBuffer[15-x][1] = 0xFFFF;
	}
}


// Timer1 interupt: pushes the framebuffer to drive the display
ISR(TIMER1_COMPA_vect) {
	badge.processFB();
}
