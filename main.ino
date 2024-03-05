#include <Wire.h>
#include "Adafruit_MPR121.h"
#ifndef _BV
#define _BV(bit) (1 << (bit)) 
#endif

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
 #include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

#include "SPIFFS.h"
using namespace std;


#define DO1_frequence 523
#define RE_frequence 587
#define MI_frequence 659
#define FA_frequence 698
#define SOL_frequence 784
#define LA_frequence 880
#define SI_frequence 988
#define DO2_frequence 1046

#define DO1_PIXEL 7
#define RE_PIXEL 6
#define MI_PIXEL 5
#define FA_PIXEL 4
#define SOL_PIXEL 3
#define LA_PIXEL 2
#define SI_PIXEL 1
#define DO2_PIXEL 0

#define DO1_TouchPin 7
#define RE_TouchPin 6
#define MI_TouchPin 5
#define FA_TouchPin 4
#define SOL_TouchPin 3
#define LA_TouchPin 2
#define SI_TouchPin 1
#define DO2_TouchPin 0

#define DO1_CHANNEL 1
#define RE_CHANNEL 2
#define MI_CHANNEL 3
#define FA_CHANNEL 4
#define SOL_CHANNEL 5
#define LA_CHANNEL 6
#define SI_CHANNEL 7
#define DO2_CHANNEL 8

// TO DEFINE 
#define DO1_ChannelPin 32
#define RE_ChannelPin 35
#define MI_ChannelPin 25
#define FA_ChannelPin 26
#define SOL_ChannelPin 27
#define LA_ChannelPin 14
#define SI_ChannelPin 12
#define DO2_ChannelPin 13

//LEDS
#define LED_PIN  4 
#define NUMPIXELS 8 
Adafruit_NeoPixel pixels(NUMPIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);

// TOUCH
// You can have up to 4 on one i2c bus but one is enough for testing!
Adafruit_MPR121 cap = Adafruit_MPR121();
uint32_t lasttouched = 0;
uint32_t currtouched = 0;

// SOUND
const int PWM_RESOLUTION = 8; // We'll use same resolution as Uno (8 bits, 0-255) but ESP32 can go up to 16 bits 
// The max duty cycle value based on PWM resolution (will be 255 if resolution is 8 bits)
const int MAX_DUTY_CYCLE = (int)(pow(2, PWM_RESOLUTION) - 1); 
int dc = 150;

uint32_t note_index = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("0");
  
  setup_mpr121();
  setup_neopixel();
  
  Serial.println("1");
  if (!SPIFFS.begin(true)) {
    Serial.println("An error occurred while formatting SPIFFS");
    return;
  }

  // write the song in the file TODO: find another way to stock it
  File file = SPIFFS.open("/song.txt", FILE_WRITE);

  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }

  // Write the content to the file
  file.println("DO1,500");
  file.println("RE,500");


  // Close the file
  file.close();

  // SET UP FOR THE SOUND 
  // Sets up a channel (0-15), a PWM duty cycle frequency, and a PWM resolution (1 - 16 bits) 
  // ledcSetup(uint8_t channel, double freq, uint8_t resolution_bits);
  ledcSetup(DO1_CHANNEL, DO1_frequence, PWM_RESOLUTION);
  ledcSetup(RE_CHANNEL, RE_frequence, PWM_RESOLUTION);
  ledcSetup(MI_CHANNEL, MI_frequence, PWM_RESOLUTION);
  ledcSetup(FA_CHANNEL, FA_frequence, PWM_RESOLUTION);
  ledcSetup(SOL_CHANNEL, SOL_frequence, PWM_RESOLUTION);
  ledcSetup(LA_CHANNEL, LA_frequence, PWM_RESOLUTION);
  ledcSetup(SI_CHANNEL, SI_frequence, PWM_RESOLUTION);
  ledcSetup(DO2_CHANNEL, DO2_frequence, PWM_RESOLUTION);

  // ledcAttachPin(uint8_t pin, uint8_t channel);
  /*ledcAttachPin(DO1_ChannelPin, DO1_CHANNEL);
  ledcAttachPin(RE_ChannelPin, RE_CHANNEL);
  ledcAttachPin(MI_ChannelPin, MI_CHANNEL);
  ledcAttachPin(FA_ChannelPin, FA_CHANNEL);
  ledcAttachPin(SOL_ChannelPin, SOL_CHANNEL);
  ledcAttachPin(LA_ChannelPin, LA_CHANNEL);
  ledcAttachPin(SI_ChannelPin, SI_CHANNEL);
  ledcAttachPin(DO2_ChannelPin, DO2_CHANNEL);*/
}

void loop() {
  double note_success = 0;
  double notes_total=0;
  //Serial.println(song.size());

  
  Serial.println("2");
  File file = SPIFFS.open("/song.txt", "r");
  if (!file) {
    Serial.println("Failed to open file for reading");
  }
  Serial.println("3");
  easy_mode(file);

}

void easy_mode(File file){
  
  Serial.println(note_index);
  while (file.available()) {
    Serial.println(note_index);
    // GET THE NEXT NOTE 
    String note = file.readStringUntil(','); 
    String time = file.readStringUntil('\n');
    Serial.println("note");
    Serial.println(note);
      
   // TURN ON THE CORRESPONDING LED
    light_led_note(note);

  // WAIT FOR THE TOUCH AND WHEN TOUCHED MAKE THE SOUND
    touch_and_sound(note);

    // TURN OFF THE LED
    Serial.println("turn off pixels");
    pixels.clear();
    pixels.show();  
    delay(500);
    note_index+=1;
  }
   pixels.clear();
}

void touch_and_sound(String note){
  currtouched = cap.touched();
  
  if (note=="DO1"){
    // while if *is NOT* touched wait!
    while (1){
      currtouched = cap.touched();
      Serial.println("wait for touch");
      if ((currtouched & _BV(DO1_TouchPin))){
        lasttouched=1;
        break;
      }
      continue;
    }
    //turn on the sound
    Serial.println("TOUCHED");
    dc = 150;
    ledcAttachPin(DO1_ChannelPin, DO1_CHANNEL);
    ledcWrite(DO1_CHANNEL, 150); 
    pixels.setPixelColor(DO1_PIXEL, pixels.Color(0, 255, 0));
    pixels.show(); 
    Serial.println("green");

    while(1){
      Serial.println("wait for release");
      currtouched = cap.touched();
      // if it *was* touched and now *isnt*, alert!
      if (!(currtouched & _BV(DO1_TouchPin))) {
        Serial.println("RELEASED");
        dc = 0;
        ledcWrite(DO1_CHANNEL, 0); 
        lasttouched=0;
        break;
      }
    }

    return;
  }

  if (note=="RE"){
    // while if *is NOT* touched wait!
    while (1){
      currtouched = cap.touched();
      Serial.println("wait for touch");
      if ((currtouched & _BV(RE_TouchPin))){
        lasttouched=1;
        break;
      }
      continue;
    }
    //turn on the sound
    Serial.println("TOUCHED");
    dc = 150;
    ledcAttachPin(DO1_ChannelPin, RE_CHANNEL);
    ledcWrite(RE_CHANNEL, 150); 
    pixels.setPixelColor(RE_PIXEL, pixels.Color(0, 255, 0));
    pixels.show(); 

    while(1){
      currtouched = cap.touched();
      Serial.println("wait for the release");
      // if it *was* touched and now *isnt*, alert!
      if (!(currtouched & _BV(RE_TouchPin))) {
        Serial.println("released");
        dc = 0;
        ledcWrite(RE_CHANNEL, 0); 
        lasttouched=0;
        break;
      }
    }
    return;
  }

/*
  if (note=="MI"){
    // while if *is NOT* touched wait!
    while (1){
      currtouched = cap.touched();
      Serial.println("wait for touch");
      if ((currtouched & _BV(MI_TouchPin)) && !(lasttouched & _BV(MI_TouchPin)) ){
        lasttouched=1;
        break;
      }
      continue;
    }
    //turn on the sound
    Serial.println("TOUCHED");
    dc = 150;
    ledcWrite(MI_CHANNEL, 150); 
    pixels.setPixelColor(MI_PIXEL, pixels.Color(0, 255, 0));
    pixels.show();

    while(1){
      currtouched = cap.touched();
      // if it *was* touched and now *isnt*, alert!
      if (!(currtouched & _BV(MI_TouchPin)) && (lasttouched & _BV(MI_TouchPin)) ) {
        dc = 0;
        ledcWrite(MI_CHANNEL, dc); 
        lasttouched=0;
        break;
      }
    }
    return;
  }

  if (note=="FA"){
    // while if *is NOT* touched wait!
    while (1){
      currtouched = cap.touched();
      Serial.println("wait for touch");
      if ((currtouched & _BV(FA_TouchPin)) && !(lasttouched & _BV(FA_TouchPin)) ){
        lasttouched=1;
        break;
      }
      continue;
    }
    //turn on the sound
    Serial.println("TOUCHED");
    dc = 150;
    ledcWrite(FA_CHANNEL, 150); 
    pixels.setPixelColor(FA_PIXEL, pixels.Color(0, 255, 0));
    pixels.show();

    while(1){
      currtouched = cap.touched();
      // if it *was* touched and now *isnt*, alert!
      if (!(currtouched & _BV(FA_TouchPin)) && (lasttouched & _BV(FA_TouchPin)) ) {
        dc = 0;
        ledcWrite(FA_CHANNEL, dc); 
        lasttouched=0;
        break;
      }
    }
    return;
  }
  
  if (note=="SOL"){
    // while if *is NOT* touched wait!
    while (1){
      currtouched = cap.touched();
      Serial.println("wait for touch");
      if ((currtouched & _BV(SOL_TouchPin)) && !(lasttouched & _BV(SOL_TouchPin)) ){
        lasttouched=1;
        break;
      }
      continue;
    }
    //turn on the sound
    Serial.println("TOUCHED");
    dc = 150;
    ledcWrite(SOL_CHANNEL, 150); 
    pixels.setPixelColor(SOL_PIXEL, pixels.Color(0, 255, 0));
    pixels.show();

    while(1){
      currtouched = cap.touched();
      // if it *was* touched and now *isnt*, alert!
      if (!(currtouched & _BV(SOL_TouchPin)) && (lasttouched & _BV(SOL_TouchPin)) ) {
        dc = 0;
        ledcWrite(SOL_CHANNEL, dc); 
        lasttouched=0;
        break;
      }
    }
    return;
  }

  if (note=="LA"){
    // while if *is NOT* touched wait!
    while (1){
      currtouched = cap.touched();
      Serial.println("wait for touch");
      if ((currtouched & _BV(LA_TouchPin)) && !(lasttouched & _BV(LA_TouchPin)) ){
        lasttouched=1;
        break;
      }
      continue;
    }
    //turn on the sound
    Serial.println("TOUCHED");
    dc = 150;
    ledcWrite(LA_CHANNEL, 150); 
    pixels.setPixelColor(LA_PIXEL, pixels.Color(0, 255, 0));
    pixels.show();

    while(1){
      currtouched = cap.touched();
      // if it *was* touched and now *isnt*, alert!
      if (!(currtouched & _BV(LA_TouchPin)) && (lasttouched & _BV(LA_TouchPin)) ) {
        dc = 0;
        ledcWrite(LA_CHANNEL, dc); 
        lasttouched=0;
        break;
      }
    }
    return;
  }

  if (note=="SI"){
    // while if *is NOT* touched wait!
    while (1){
      currtouched = cap.touched();
      Serial.println("wait for touch");
      if ((currtouched & _BV(SI_TouchPin)) && !(lasttouched & _BV(SI_TouchPin)) ){
        lasttouched=1;
        break;
      }
      continue;
    }
    //turn on the sound
    Serial.println("TOUCHED");
    dc = 150;
    ledcWrite(SI_CHANNEL, 150); 
    pixels.setPixelColor(SI_PIXEL, pixels.Color(0, 255, 0));
    pixels.show();

    while(1){
      currtouched = cap.touched();
      // if it *was* touched and now *isnt*, alert!
      if (!(currtouched & _BV(SI_TouchPin)) && (lasttouched & _BV(SI_TouchPin)) ) {
        dc = 0;
        ledcWrite(SI_CHANNEL, dc); 
        lasttouched=0;
        break;
      }
    }
    return;
  }

  if (note=="DO2"){
    // while if *is NOT* touched wait!
    while (1){
      currtouched = cap.touched();
      Serial.println("wait for touch");
      if ((currtouched & _BV(DO2_TouchPin)) && !(lasttouched & _BV(DO2_TouchPin)) ){
        lasttouched=1;
        break;
      }
      continue;
    }
    //turn on the sound
    Serial.println("TOUCHED");
    dc = 150;
    ledcWrite(DO2_CHANNEL, 150); 
    pixels.setPixelColor(DO2_PIXEL, pixels.Color(0, 255, 0));
    pixels.show();

    while(1){
      currtouched = cap.touched();
      // if it *was* touched and now *isnt*, alert!
      if (!(currtouched & _BV(DO2_TouchPin)) && (lasttouched & _BV(DO2_TouchPin)) ) {
        dc = 0;
        ledcWrite(DO2_CHANNEL, dc); 
        lasttouched=0;
        break;
      }
    }
    return;
  }
*/
}

void light_led_note(String note){
  if (note=="DO1"){
      pixels.setPixelColor(DO1_PIXEL, pixels.Color(255, 255, 255));
      pixels.show();   // Send the updated pixel colors to the hardware.
  }
   if (note=="RE"){
      pixels.setPixelColor(RE_PIXEL, pixels.Color(255, 255, 255));
      pixels.show();   // Send the updated pixel colors to the hardware.
  }
   if (note=="MI"){
      pixels.setPixelColor(MI_PIXEL, pixels.Color(255, 255, 255));
      pixels.show();   // Send the updated pixel colors to the hardware.
  }
   if (note=="FA"){
      pixels.setPixelColor(FA_PIXEL, pixels.Color(255, 255, 255));
      pixels.show();   // Send the updated pixel colors to the hardware.
  }
   if (note=="SOL"){
      pixels.setPixelColor(SOL_PIXEL, pixels.Color(255, 255, 255));
      pixels.show();   // Send the updated pixel colors to the hardware.
  }
   if (note=="LA"){
      pixels.setPixelColor(LA_PIXEL, pixels.Color(255, 255, 255));
      pixels.show();   // Send the updated pixel colors to the hardware.
  }
   if (note=="SI"){
      pixels.setPixelColor(SI_PIXEL, pixels.Color(255, 255, 255));
      pixels.show();   // Send the updated pixel colors to the hardware.
  }
   if (note=="DO2"){
      pixels.setPixelColor(DO2_PIXEL, pixels.Color(255, 255, 255));
      pixels.show();   // Send the updated pixel colors to the hardware.
  }

}

/*Vector<String> setup_file(){

  
  if(!SPIFFS.begin(true)){
     Serial.println("An Error has occurred while mounting SPIFFS");
     return null;
  }

  File file = SPIFFS.open("./song.txt", "r");
  if (!file) {
    Serial.println("Failed to open file for reading");
    return null;
  }

  Vector<String> song;
  while (file.available()) {
    song.push_back(file.readStringUntil('\n'));
  }
  file.close();
  return song;
}*/

void setup_mpr121(){

  while (!Serial) { // needed to keep leonardo/micro from starting too fast!
    delay(10);
  }
  
  Serial.println("Adafruit MPR121 Capacitive Touch sensor test"); 
  
  // Default address is 0x5A, if tied to 3.3V its 0x5B
  // If tied to SDA its 0x5C and if SCL then 0x5D
  if (!cap.begin(0x5A)) {
    Serial.println("MPR121 not found, check wiring?");
  }
  Serial.println("MPR121 found!");
}

void setup_neopixel(){
  // These lines are specifically to support the Adafruit Trinket 5V 16 MHz.
  // Any other board, you can remove this part (but no harm leaving it):
#if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
  clock_prescale_set(clock_div_1);
#endif
  // END of Trinket-specific code.

  pixels.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)
  pixels.show();
  pixels.setBrightness(50); // Set BRIGHTNESS to about 1/5 (max = 255)
}
