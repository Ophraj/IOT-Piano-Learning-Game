#include <Arduino.h>
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

#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include "song.h"

#define DO1_frequence 523
#define RE_frequence 587
#define MI_frequence 659
#define FA_frequence 698
#define SOL_frequence 784
#define LA_frequence 880
#define SI_frequence 988
#define DO2_frequence 1046

#define DO1_PIXEL 0
#define RE_PIXEL 1
#define MI_PIXEL 2
#define FA_PIXEL 3
#define SOL_PIXEL 4
#define LA_PIXEL 5
#define SI_PIXEL 6
#define DO2_PIXEL 7

#define DO1_TouchPin 0
#define RE_TouchPin 1
#define MI_TouchPin 2
#define FA_TouchPin 3
#define SOL_TouchPin 4
#define LA_TouchPin 5
#define SI_TouchPin 6
#define DO2_TouchPin 7

#define DO1_CHANNEL 0
#define RE_CHANNEL 2
#define MI_CHANNEL 4
#define FA_CHANNEL 6
#define SOL_CHANNEL 8
#define LA_CHANNEL 10
#define SI_CHANNEL 12
#define DO2_CHANNEL 14

#define DO1_ChannelPin 32
#define RE_ChannelPin 33
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
int dc = 128;

uint32_t note_index = 0;

//DATABASE
// Insert your network credentials
#define WIFI_SSID "Ophra Iphone"
#define WIFI_PASSWORD "ophraaaa"

// Insert Firebase project API Key
#define API_KEY "AIzaSyDfufadFI30mQoQpQtXNv4N3Mc9DHsQPEc"

// Insert RTDB URLefine the RTDB URL */
#define DATABASE_URL "https://piano-learning-3f147-default-rtdb.europe-west1.firebasedatabase.app/" 

//Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

String mode = "EASY";

// Stats
int errors_count = 0;
unsigned long start_time = 0;
unsigned long touch_time= 0;
unsigned long duration_total = 0;

void setup() {
  Serial.begin(115200);
  
  //set up touch
  setup_mpr121();

  //set up pixels
  setup_neopixel();
  
  Serial.println("1");
  if (!SPIFFS.begin(true)) {
    Serial.println("An error occurred while formatting SPIFFS");
    return;
  }

  // SET UP FOR THE SOUND 
  setup_sound();

  //Welcome sound and led
  int count_rainbow=0;
  String note = "None";
  note_index=0;
  for(long firstPixelHue = 0; firstPixelHue < 5*65536; firstPixelHue += 256) {

    pixels.rainbow(firstPixelHue);
    // Above line is equivalent to: strip.rainbow(firstPixelHue, 1, 255, 255, true);
    pixels.show(); // Update strip with new contents
    if (count_rainbow>=49){
      turnOff_son_note(note);
      if (note_index>=15)
      {
        pixels.clear();
        pixels.show();
        break;
      }
      note = notes_welcome[note_index];
      note_index++;
      turnOn_son_note(note);
      count_rainbow=0;
    }
    delay(10);
    count_rainbow+=1; 
  }
  delay(1000);
  note_index=0;
}

void loop() {
  
  if (mode == "EASY"){
    easy_mode();
    mode = "FREE";
  }
  else {
      free_playing();
  }
}


//FREE PLAYING FUNCTION
void free_playing(){
  //turn on if touched 
  currtouched = cap.touched();
  if ((currtouched & _BV(DO1_TouchPin))){
    Serial.println("DO1 IS touched");

    ledcWrite(DO1_CHANNEL, 128); 
    pixels.setPixelColor(DO1_PIXEL, pixels.Color(255, 255, 255));
    pixels.show();
  }

  if ((currtouched & _BV(RE_TouchPin))){
    Serial.println("RE IS touched");

    ledcWrite(RE_CHANNEL, 128); 
    pixels.setPixelColor(RE_PIXEL, pixels.Color(255, 255, 255));
    pixels.show();
  }

  if ((currtouched & _BV(MI_TouchPin))){
    ledcWrite(MI_CHANNEL, 128); 
    pixels.setPixelColor(MI_PIXEL, pixels.Color(255, 255, 255));
    pixels.show();
  }

  if ((currtouched & _BV(FA_TouchPin))){
    ledcWrite(FA_CHANNEL, 128); 
    pixels.setPixelColor(FA_PIXEL, pixels.Color(255, 255, 255));
    pixels.show();

  }

  if ((currtouched & _BV(SOL_TouchPin))){
    ledcWrite(SOL_CHANNEL, 128); 
    pixels.setPixelColor(SOL_PIXEL, pixels.Color(255, 255, 255));
    pixels.show();
  }

  if ((currtouched & _BV(LA_TouchPin))){
    ledcWrite(LA_CHANNEL, 128); 
    pixels.setPixelColor(LA_PIXEL, pixels.Color(255, 255, 255));
    pixels.show();
  }

  if ((currtouched & _BV(SI_TouchPin))){
    ledcWrite(SI_CHANNEL, 128); 
    pixels.setPixelColor(SI_PIXEL, pixels.Color(255, 255, 255));
    pixels.show();
  }

  if ((currtouched & _BV(DO2_TouchPin))){
    ledcWrite(DO2_CHANNEL, 128); 
    pixels.setPixelColor(DO2_PIXEL, pixels.Color(255, 255, 255));
    pixels.show();
  }


  //turn off if released
  if (!(currtouched & _BV(DO1_TouchPin))) {
        ledcWrite(DO1_CHANNEL, 0); 
        pixels.setPixelColor(DO1_PIXEL, pixels.Color(0, 0, 0));
        pixels.show();
  }

  if (!(currtouched & _BV(RE_TouchPin))) {
        ledcWrite(RE_CHANNEL, 0); 
        pixels.setPixelColor(RE_PIXEL, pixels.Color(0, 0, 0));
        pixels.show();
  }

  if (!(currtouched & _BV(MI_TouchPin))) {
        ledcWrite(MI_CHANNEL, 0); 
        pixels.setPixelColor(MI_PIXEL, pixels.Color(0, 0, 0));
        pixels.show();
  }

  if (!(currtouched & _BV(FA_TouchPin))) {
        ledcWrite(FA_CHANNEL, 0); 
        pixels.setPixelColor(FA_PIXEL, pixels.Color(0, 0, 0));
        pixels.show();
  }

  if (!(currtouched & _BV(SOL_TouchPin))) {
        ledcWrite(SOL_CHANNEL, 0); 
        pixels.setPixelColor(SOL_PIXEL, pixels.Color(0, 0, 0));
        pixels.show();
  }

  if (!(currtouched & _BV(LA_TouchPin))) {
        ledcWrite(LA_CHANNEL, 0); 
        pixels.setPixelColor(LA_PIXEL, pixels.Color(0, 0, 0));
        pixels.show();
  }

  if (!(currtouched & _BV(SI_TouchPin))) {
        ledcWrite(SI_CHANNEL, 0); 
        pixels.setPixelColor(SI_PIXEL, pixels.Color(0, 0, 0));
        pixels.show();
  }

  if (!(currtouched & _BV(DO2_TouchPin))) {
        ledcWrite(DO2_CHANNEL, 0); 
        pixels.setPixelColor(DO2_PIXEL, pixels.Color(0, 0, 0));
        pixels.show();
  }
}

// EASY MODE FUNCTIONS 
void easy_mode(){
  Serial.println("in easy mode");
  Serial.println(note_index);

  play_song_hb();
  note_index=0;
  while (note_index<count_note) {
    Serial.println(note_index);
    
    // GET THE NEXT NOTE 
    String note = notes[note_index];
    int time = duration_note[note_index];

    Serial.println("note");
    Serial.println(note);
      
   // TURN ON THE CORRESPONDING LED
    light_led_note(note);

  // WAIT FOR THE TOUCH AND WHEN TOUCHED MAKE THE SOUND
    start_time = millis();
    touch_and_sound(note);
    touch_time= millis();
    duration_total += (touch_time-start_time);

    // TURN OFF THE LED
    Serial.println("turn off pixels");
    pixels.clear();
    pixels.show();  
    note_index+=1;
  }

  pixels.clear();
  unsigned long duration_average = duration_total/count_note;
  Serial.println(errors_count);
  Serial.println(duration_average);
  note_index=0;
}

void play_song_welcome(){
  note_index=0;
  while (note_index<count_note_welcome) {

  String note = notes_welcome[note_index];
  int time = duration_note_welcome[note_index];

  unsigned long start = millis();
  unsigned long end = millis();
  while (end-start<time)
  {
    end = millis();
    turnOn_son_note(note);
  }

  turnOff_son_note(note);
  note_index++;
  }
  note_index=0;
}

void play_song_hb(){
  note_index=0;
  while (note_index<count_note) {

  String note = notes[note_index];
  int time = duration_note[note_index];

  unsigned long start = millis();
  unsigned long end = millis();
  while (end-start<time)
  {
    end = millis();
    dc = 128;
    light_led_note(note); 
    turnOn_son_note(note);
  }

  turnOff_son_note(note);
  pixels.clear();
  pixels.show();  
  note_index++;
  }
}

void touch_and_sound(String note){
  currtouched = cap.touched();
  if (note=="DO1"){
    // while if *is NOT* touched wait!
    while (!(currtouched & _BV(DO1_TouchPin))){
      currtouched = cap.touched();
      Serial.println("wait for touch");
      if ((currtouched)){
        if ((currtouched & _BV(DO1_TouchPin))){
          break;
        }
        else {
            errors_count += 1;
            false_touched_note_easy_mode(note);
            continue;
        }
      }
      continue;
    }
    //turn on the sound
    Serial.println("TOUCHED");
    dc = 128;
    ledcWrite(DO1_CHANNEL, 128); 
    pixels.setPixelColor(DO1_PIXEL, pixels.Color(0, 255, 0));
    pixels.show(); 
    Serial.println("green");

    while(1){
      //Serial.println("wait for release");
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
      if ((currtouched)){
        if ((currtouched & _BV(RE_TouchPin))){
          break;
        }
        else {
            errors_count += 1;
            false_touched_note_easy_mode(note);
            continue;
        }
      }
      continue;
    }
    //turn on the sound
    Serial.println("TOUCHED");
    dc = 128;
    ledcWrite(RE_CHANNEL, 128); 
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


  if (note=="MI"){
    // while if *is NOT* touched wait!
    while (1){
      currtouched = cap.touched();
      Serial.println("wait for touch");
      if ((currtouched)){
        if ((currtouched & _BV(MI_TouchPin))){
          break;
        }
        else {
            errors_count += 1;
            false_touched_note_easy_mode(note);
            continue;
        }
      }
      continue;
    }
    //turn on the sound
    Serial.println("TOUCHED");
    dc = 128;
    ledcWrite(MI_CHANNEL, 128); 
    pixels.setPixelColor(MI_PIXEL, pixels.Color(0, 255, 0));
    pixels.show();

    while(1){
      currtouched = cap.touched();
      // if RELEASED
      if (!(currtouched & _BV(MI_TouchPin))) {
        dc = 0;
        ledcWrite(MI_CHANNEL, 0); 
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
      if ((currtouched)){
        if ((currtouched & _BV(FA_TouchPin))){
          break;
        }
        else {
            errors_count += 1;
            false_touched_note_easy_mode(note);
            continue;
        }
      }
      continue;
    }
    //turn on the sound
    Serial.println("TOUCHED");
    dc = 128;
    ledcWrite(FA_CHANNEL, 128); 
    pixels.setPixelColor(FA_PIXEL, pixels.Color(0, 255, 0));
    pixels.show();

    while(1){
      currtouched = cap.touched();
      // if it *was* touched and now *isnt*, alert!
      if (!(currtouched & _BV(FA_TouchPin))) {
        dc = 0;
        ledcWrite(FA_CHANNEL, 0); 
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
      if ((currtouched)){
        if ((currtouched & _BV(SOL_TouchPin))){
          break;
        }
        else {
            errors_count += 1;
            false_touched_note_easy_mode(note);
            continue;
        }
      }
      continue;
    }
    //turn on the sound
    Serial.println("TOUCHED");
    dc = 128;
    ledcWrite(SOL_CHANNEL, 128); 
    pixels.setPixelColor(SOL_PIXEL, pixels.Color(0, 255, 0));
    pixels.show();

    while(1){
      currtouched = cap.touched();
      // if it *was* touched and now *isnt*, alert!
      if (!(currtouched & _BV(SOL_TouchPin))) {
        dc = 0;
        ledcWrite(SOL_CHANNEL, 0); 
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
      if ((currtouched)){
        if ((currtouched & _BV(LA_TouchPin))){
          break;
        }
        else {
            errors_count += 1;
            false_touched_note_easy_mode(note);
            continue;
        }
      }
      continue;
    }
    //turn on the sound
    Serial.println("TOUCHED");
    dc = 128;
    ledcWrite(LA_CHANNEL, 128); 
    pixels.setPixelColor(LA_PIXEL, pixels.Color(0, 255, 0));
    pixels.show();

    while(1){
      currtouched = cap.touched();
      // if it *was* touched and now *isnt*, alert!
      if (!(currtouched & _BV(LA_TouchPin))) {
        dc = 0;
        ledcWrite(LA_CHANNEL, 0); 
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
      if ((currtouched)){
        if ((currtouched & _BV(SI_TouchPin))){
          break;
        }
        else {
            errors_count += 1;
            false_touched_note_easy_mode(note);
            continue;
        }
      }
      continue;
    }
    //turn on the sound
    Serial.println("TOUCHED");
    dc = 128;
    ledcWrite(SI_CHANNEL, 128); 
    pixels.setPixelColor(SI_PIXEL, pixels.Color(0, 255, 0));
    pixels.show();

    while(1){
      currtouched = cap.touched();
      // if it *was* touched and now *isnt*, alert!
      if (!(currtouched & _BV(SI_TouchPin))) {
        dc = 0;
        ledcWrite(SI_CHANNEL, 0); 
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
      if ((currtouched)){
        if ((currtouched & _BV(DO2_TouchPin))){
          break;
        }
        else {
            errors_count += 1;
            false_touched_note_easy_mode(note);
            continue;
        }
      }
      continue;
    }
    //turn on the sound
    Serial.println("TOUCHED");
    dc = 128;
    ledcWrite(DO2_CHANNEL, 128); 
    pixels.setPixelColor(DO2_PIXEL, pixels.Color(0, 255, 0));
    pixels.show();

    while(1){
      currtouched = cap.touched();
      // if it *was* touched and now *isnt*, alert!
      if (!(currtouched & _BV(DO2_TouchPin))) {
        dc = 0;
        ledcWrite(DO2_CHANNEL, 0); 
        lasttouched=0;
        break;
      }
    }
    return;
  }
}

void false_touched_note_easy_mode(String note){
  for(int j=0; j<3; j++) { 
      for(int i=0; i<NUMPIXELS; i++) { 
          pixels.setPixelColor(i, pixels.Color(255, 0, 0));     //light in red 3 times  
          pixels.show();                           
      }
      delay(500);                           
      pixels.clear();
      pixels.show();
      delay(500);                           
  }
  light_led_note(note); //Show our note led in white
  return;
}

void light_led_note(String note){

  if (note=="None"){
    return;
  }

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

void turnOn_son_note(String note){

  if (note=="None"){
    return;
  }
  if (note=="DO1"){
      ledcWrite(DO1_CHANNEL, 128);   
  }
   if (note=="RE"){
      ledcWrite(RE_CHANNEL, 128);    
  }
   if (note=="MI"){
      ledcWrite(MI_CHANNEL, 128); 
  }
   if (note=="FA"){
      ledcWrite(FA_CHANNEL, 128); 
  }
   if (note=="SOL"){
      ledcWrite(SOL_CHANNEL, 128); 
  }
   if (note=="LA"){
      ledcWrite(LA_CHANNEL, 128); 
  }
   if (note=="SI"){
      ledcWrite(SI_CHANNEL, 128); 
  }
   if (note=="DO2"){
      ledcWrite(DO2_CHANNEL, 128); 
  }
}

void turnOff_son_note(String note){

  if (note=="None"){
    return;
  }

  if (note=="DO1"){
      ledcWrite(DO1_CHANNEL, 0);    // Send the updated pixel colors to the hardware.
  }
   if (note=="RE"){
      ledcWrite(RE_CHANNEL, 0);    // Send the updated pixel colors to the hardware.
  }
   if (note=="MI"){
      ledcWrite(MI_CHANNEL, 0); 
  }
   if (note=="FA"){
      ledcWrite(FA_CHANNEL, 0); 
  }
   if (note=="SOL"){
      ledcWrite(SOL_CHANNEL, 0); 
  }
   if (note=="LA"){
      ledcWrite(LA_CHANNEL, 0); 
  }
   if (note=="SI"){
      ledcWrite(SI_CHANNEL, 0); 
  }
   if (note=="DO2"){
      ledcWrite(DO2_CHANNEL, 0); 
  }
}

//Medium mode functions // 
void false_touched_note_medium_mode(String note, uint32_t currtouched){
    
    while (currtouched & _BV(DO1_TouchPin)){
      pixels.setPixelColor(DO1_PIXEL, pixels.Color(255, 0, 0));
      pixels.show();
      ledcWrite(DO1_CHANNEL, 128);
      currtouched = cap.touched();
      if (!(currtouched & _BV(DO1_TouchPin))){
        ledcWrite(DO1_CHANNEL, 0);
        pixels.setPixelColor(DO1_PIXEL, pixels.Color(0, 0, 0));
        pixels.show();
        return;
      }
    }

    while (currtouched & _BV(RE_TouchPin)){
      pixels.setPixelColor(RE_PIXEL, pixels.Color(255, 0, 0));
      pixels.show();
      ledcWrite(RE_CHANNEL, 128);
      currtouched = cap.touched();
      if (!(currtouched & _BV(RE_TouchPin))){
        ledcWrite(RE_CHANNEL, 0);
        pixels.setPixelColor(RE_PIXEL, pixels.Color(0, 0, 0));
        pixels.show();
        return;
      }
    }

     while (currtouched & _BV(MI_TouchPin)){
      pixels.setPixelColor(MI_PIXEL, pixels.Color(255, 0, 0));
      pixels.show();
      ledcWrite(MI_CHANNEL, 128);
      currtouched = cap.touched();
      if (!(currtouched & _BV(MI_TouchPin))){
        ledcWrite(MI_CHANNEL, 0);
        pixels.setPixelColor(MI_PIXEL, pixels.Color(0, 0, 0));
        pixels.show();
        return;
      }
    }

     while (currtouched & _BV(FA_TouchPin)){
      pixels.setPixelColor(FA_PIXEL, pixels.Color(255, 0, 0));
      pixels.show();
      ledcWrite(FA_CHANNEL, 128);
      currtouched = cap.touched();
      if (!(currtouched & _BV(FA_TouchPin))){
        ledcWrite(FA_CHANNEL, 0);
        pixels.setPixelColor(FA_PIXEL, pixels.Color(0, 0, 0));
        pixels.show();
        return;
      }
    }

     while (currtouched & _BV(SOL_TouchPin)){
      pixels.setPixelColor(SOL_PIXEL, pixels.Color(255, 0, 0));
      pixels.show();
      ledcWrite(SOL_CHANNEL, 128);
      currtouched = cap.touched();
      if (!(currtouched & _BV(SOL_TouchPin))){
        ledcWrite(SOL_CHANNEL, 0);
        pixels.setPixelColor(SOL_PIXEL, pixels.Color(0, 0, 0));
        pixels.show();
        return;
      }
    }

    while (currtouched & _BV(LA_TouchPin)){
      pixels.setPixelColor(LA_PIXEL, pixels.Color(255, 0, 0));
      pixels.show();
      ledcWrite(LA_CHANNEL, 128);
      currtouched = cap.touched();
      if (!(currtouched & _BV(LA_TouchPin))){
        ledcWrite(LA_CHANNEL, 0);
        pixels.setPixelColor(LA_PIXEL, pixels.Color(0, 0, 0));
        pixels.show();
        return;
      }
    }

    while (currtouched & _BV(SI_TouchPin)){
      pixels.setPixelColor(SI_PIXEL, pixels.Color(255, 0, 0));
      pixels.show();
      ledcWrite(SI_CHANNEL, 128);
      currtouched = cap.touched();
      if (!(currtouched & _BV(SI_TouchPin))){
        ledcWrite(SI_CHANNEL, 0);
        pixels.setPixelColor(SI_PIXEL, pixels.Color(0, 0, 0));
        pixels.show();
        return;
      }
    }

    while (currtouched & _BV(DO2_TouchPin)){
      pixels.setPixelColor(DO2_PIXEL, pixels.Color(255, 0, 0));
      pixels.show();
      ledcWrite(DO2_CHANNEL, 128);
      currtouched = cap.touched();
      if (!(currtouched & _BV(DO2_TouchPin))){
        ledcWrite(DO2_CHANNEL, 0);
        pixels.setPixelColor(DO2_PIXEL, pixels.Color(0, 0, 0));
        pixels.show();
        return;
      }
    }
}

// SET UP FUNCTIONS //

void setup_sound(){
  // SET UP FOR THE SOUND 
  //Connect channels and frequences 
  ledcSetup(DO1_CHANNEL, DO1_frequence, PWM_RESOLUTION);
  ledcSetup(RE_CHANNEL, RE_frequence, PWM_RESOLUTION);
  ledcSetup(MI_CHANNEL, MI_frequence, PWM_RESOLUTION);
  ledcSetup(FA_CHANNEL, FA_frequence, PWM_RESOLUTION);
  ledcSetup(SOL_CHANNEL, SOL_frequence, PWM_RESOLUTION);
  ledcSetup(LA_CHANNEL, LA_frequence, PWM_RESOLUTION);
  ledcSetup(SI_CHANNEL, SI_frequence, PWM_RESOLUTION);
  ledcSetup(DO2_CHANNEL, DO2_frequence, PWM_RESOLUTION);

  // ledcAttachPin(uint8_t pin, uint8_t channel);
  ledcAttachPin(DO1_ChannelPin, DO1_CHANNEL);
  ledcAttachPin(RE_ChannelPin, RE_CHANNEL);
  ledcAttachPin(MI_ChannelPin, MI_CHANNEL);
  ledcAttachPin(FA_ChannelPin, FA_CHANNEL);
  ledcAttachPin(SOL_ChannelPin, SOL_CHANNEL);
  ledcAttachPin(LA_ChannelPin, LA_CHANNEL);
  ledcAttachPin(SI_ChannelPin, SI_CHANNEL);
  ledcAttachPin(DO2_ChannelPin, DO2_CHANNEL);
};

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

