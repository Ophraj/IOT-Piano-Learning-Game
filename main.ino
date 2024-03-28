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
#include <HTTPClient.h>

#include "song.h"

//Screen 
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// FREQUENCES
#define DO1_frequence 523
#define RE_frequence 587
#define MI_frequence 659
#define FA_frequence 698
#define SOL_frequence 784
#define LA_frequence 880
#define SI_frequence 988
#define DO2_frequence 1046

// PIXELS PIN 
#define DO1_PIXEL 0
#define RE_PIXEL 1
#define MI_PIXEL 2
#define FA_PIXEL 3
#define SOL_PIXEL 4
#define LA_PIXEL 5
#define SI_PIXEL 6
#define DO2_PIXEL 7

// TOUCH SENSOR PIN
#define DO1_TouchPin 0
#define RE_TouchPin 1
#define MI_TouchPin 2
#define FA_TouchPin 3
#define SOL_TouchPin 4
#define LA_TouchPin 5
#define SI_TouchPin 6
#define DO2_TouchPin 7

// CHANNEL (multiple of 2)
#define DO1_CHANNEL 0
#define RE_CHANNEL 2
#define MI_CHANNEL 4
#define FA_CHANNEL 6
#define SOL_CHANNEL 8
#define LA_CHANNEL 10
#define SI_CHANNEL 12
#define DO2_CHANNEL 14

// CHANNEL PIN 
#define DO1_ChannelPin 32
#define RE_ChannelPin 33
#define MI_ChannelPin 25
#define FA_ChannelPin 26
#define SOL_ChannelPin 27
#define LA_ChannelPin 14
#define SI_ChannelPin 12
#define DO2_ChannelPin 13

//LEDS PARAMETERS AND SETUP
#define LED_PIN  4 
#define NUMPIXELS 8 
Adafruit_NeoPixel pixels(NUMPIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);

// TOUCH PARAMETERS AND SETUP
Adafruit_MPR121 cap = Adafruit_MPR121();
uint32_t currtouched = 0;

// SOUND PARAMETERS AND SETYP
const int PWM_RESOLUTION = 8; 
const int MAX_DUTY_CYCLE = (int)(pow(2, PWM_RESOLUTION) - 1); 
int dc = 128;

// GOOGLE SHEET 
#define WIFI_SSID "Ophra Iphone"
#define WIFI_PASSWORD "ophraaaa"
String GOOGLE_SCRIPT_ID = "AKfycbxXSI4WI69hEjqqtUgDvyadQF84r0Jkc7M-9XHJstTIqqyZywKauGdRqMFZYKvHzms2Aw";    // change Google script ID

// Screen and buttons 
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

const int buttonUpPin = 18; 
const int buttonDownPin = 19; 
const int buttonSelectPin= 23;
const int buttonReturnPin = 15;

int buttonUpState = 0;  // variable for reading the pushbutton status
int buttonDownState = 0;  // variable for reading the pushbutton status
int buttonSelectState = 0;  // variable for reading the pushbutton status
int buttonReturnState = 0;  // variable for reading the pushbutton status

#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define EASY_MODE 1
#define MEDIUM_MODE 2
#define ADVANCED_MODE 3
#define FREE_MODE 4
#define RECORD_MODE 5

// Global variables

int count_note = 0;
String notes[200]; // Assuming a maximum of 200 notes
int duration_note[200]; // Assuming a maximum of 200 durations
int num_notes[200]; // Assuming a maximum of 200 numbers

String SongNames[1000];
int NumSongsTotal = 3;
int mode = 1;
int songNum = 0;
int songIndex = 1;

String song1;
String song2; 
String song3; 
String song4; 
String song5;

int returnValue=0;

uint32_t note_index = 0;
uint32_t num_note_index = 0;


// Stats variables
int errors_count = 0;
unsigned long start_time = 0;
unsigned long touch_time= 0;
unsigned long duration_total = 0;

void setup() {
  Serial.begin(115200);
  
  // SET UP FOR GOOGLE SHEET 
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  //set up touch
  setup_mpr121();

  //set up pixels
  setup_neopixel();
  
  if (!SPIFFS.begin(true)) {
    Serial.println("An error occurred while formatting SPIFFS");
    return;
  }

  // SET UP FOR THE SOUND 
  setup_sound();

  // SET UP FOR SCREEN 
  setup_screen();
  
  //Welcome sound and led
  welcome_sound_and_led();
}

void loop() {
  menu_mode();
}


void start_game() {
  note_index=0;
  Serial.println("Starting game");

  if (mode == EASY_MODE){
    easy_mode();
    return;
  }

  if(mode== FREE_MODE) {
      free_playing();
      return;
  }

  if (mode== MEDIUM_MODE){
    medium_mode();
    return;
  }

  /*if (mode== RECORD_MODE){
    record_mode();
    return;
  }*/

  if (mode == ADVANCED_MODE){
    advanced_mode();
    return;
  }
  returnValue=1;
}

// PLAY THE SONG
void play_song(){
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
  delay(10);
  }
  delay(1000);
}

// turn on sound according to the note
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

// turn off sound according to the note
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

//FREE PLAYING FUNCTION
void free_playing(){
  //turn on if touched 
  currtouched = cap.touched();
  if ((currtouched & _BV(DO1_TouchPin))){

    ledcWrite(DO1_CHANNEL, 128); 
    pixels.setPixelColor(DO1_PIXEL, pixels.Color(255, 255, 255));
    pixels.show();
  }

  if ((currtouched & _BV(RE_TouchPin))){
    //Serial.println("RE IS touched");

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
  Serial.println("You are in easy mode");

  // choose_song();
  play_song();
  note_index=0;

  // Pass on every notes
  while (note_index<count_note) {
    //Serial.println(note_index);
    
    // GET THE NEXT NOTE 
    String note = notes[note_index];
    int time = duration_note[note_index];

    Serial.println("note");
    Serial.println(note);
      
   // TURN ON THE CORRESPONDING LED
    light_led_note(note);

  // WAIT FOR THE TOUCH AND WHEN TOUCHED MAKE THE SOUND
    start_time = millis();
    play_one_key_note(note);
    touch_time= millis();
    duration_total += (touch_time-start_time);

    // TURN OFF THE LED
    pixels.clear();
    pixels.show();  
    note_index+=1;
  }

  pixels.clear();
  double duration_average = duration_total/count_note;
  Serial.print("You had ");
  Serial.print(errors_count);
  Serial.println(" mistakes");

  if (errors_count == 0){
    Serial.println("Well Done!");
  }
  else if (errors_count == 1){
    Serial.println("Almost perfect, good job :)");
  }
  else if (errors_count <= 3){
    Serial.println("good ,you can do better");
  }
  else {
    Serial.println("Keep practicing, eventually you will learn to play the piano");
  }
  Serial.print("Your average response time is: ");
  Serial.println(duration_average);
  note_index=0;
  errors_count=0;
}


// Call function to play the key according to the note - ONE NOTE
void play_one_key_note(String note){

  Serial.println("note is " + note);
  if (note=="DO1"){
    play_one_key_params(note, DO1_TouchPin, DO1_CHANNEL, DO1_PIXEL);
    return;
  }

  if (note=="RE"){
    play_one_key_params(note, RE_TouchPin, RE_CHANNEL, RE_PIXEL);
    return;
  }

  if (note=="MI"){
    play_one_key_params(note, MI_TouchPin, MI_CHANNEL, MI_PIXEL);
    return;
  }

  if (note=="FA"){
    play_one_key_params(note, FA_TouchPin, FA_CHANNEL, FA_PIXEL);
    return;
  }
  
  if (note=="SOL"){
    play_one_key_params(note, SOL_TouchPin, SOL_CHANNEL, SOL_PIXEL);
    return;
  }

  if (note=="LA"){
    play_one_key_params(note, LA_TouchPin, LA_CHANNEL, LA_PIXEL);
    return;
  }

  if (note=="SI"){
    play_one_key_params(note, SI_TouchPin, SI_CHANNEL, SI_PIXEL);
    return;
  }

  if (note=="DO2"){
    play_one_key_params(note, DO2_TouchPin, DO2_CHANNEL, DO2_PIXEL);
    return;
  }
}

// Function for playing one note 
// wait for touch, when touched played the key, signal error 
void play_one_key_params(String note, int touch_pin, int channel, int pixel){
    
    unsigned long start_play = 0;
    unsigned long time_played = 0;
    Serial.println("note is " + note + " and pin " + touch_pin);

    // wait for touch
    while (!(currtouched & _BV(touch_pin))){
      currtouched = cap.touched();
      if ((currtouched)){
        // RIGHT PIN TOUCHED
        if ((currtouched & _BV(touch_pin))){
          start_play = millis();
          break;
        }
        // WRONG PIN TOUCHED
        else {
            if (mode==EASY_MODE){
              error_touch(); // show 3 times red lights
              light_led_note(note); //Show our note led in white again
            }
            else { //Medium and advanced
              errors_count += 1;
              false_touched_note_medium_mode(currtouched); //play the wrong key (sound and red color)
            }
            continue;
        }
      }
      continue;
    }
    //The key is touched turn on the sound and light in green
    ledcWrite(channel, 128); 
    pixels.setPixelColor(pixel, pixels.Color(0, 255, 0));
    pixels.show(); 
    int error_made=0;

    // wait for release
    while(1){
      time_played = millis()-start_play;
      Serial.print("time played : ");
      Serial.println(time_played);
      Serial.print("duration : ");
      Serial.println(duration_note[note_index]);

      //Serial.println("wait for release");
      currtouched = cap.touched();
      if (!(currtouched & _BV(touch_pin))) {
        ledcWrite(channel, 0); 
        break;
      }

      // if mode advanced, color code
      if (mode==ADVANCED_MODE)
      {
        // if less than 100 ms until the end of the duration expected => warn with yellow light
        if (duration_note[note_index]>time_played){
          
          Serial.print("you still need to play: ");
          Serial.print(duration_note[note_index]-time_played);
          Serial.println(" ms ");

          if (duration_note[note_index]-time_played < 250)
          {
            Serial.println("show yellow ");
            pixels.setPixelColor(pixel, pixels.Color(255,255,0)); // yellow
            pixels.show(); 

          }
        }

        // if played more the duration expected but less than 100ms => light orange light 
        if (duration_note[note_index]<=time_played){

          Serial.print("You have played too much by: ");
          Serial.print(time_played-duration_note[note_index]);
          Serial.println(" ms ");

          if (time_played-duration_note[note_index] < 300)
          {
            Serial.println("show orange ");
            pixels.setPixelColor(pixel, pixels.Color(255, 165, 0)); // orange
            pixels.show(); 

          }

          if (time_played-duration_note[note_index] >= 300)
          {

            Serial.println("show red ");
            pixels.setPixelColor(pixel, pixels.Color(255,0,0)); // turn red
            errors_count+=1; //count an error
            pixels.show(); 
            if (error_made==0){
              note_index = note_index-1; //try this note again
              Serial.print("removing one: ");
              Serial.println(note_index);
            }
            error_made=1;
          }
        }
      }
    }

    return;
}


// show all red leds 3 times and add one to errors_count
void error_touch(){
  for(int j=0; j<3; j++) { 
      for(int i=0; i<NUMPIXELS; i++) { 
          pixels.setPixelColor(i, pixels.Color(255, 0, 0));     //light in red 3 times  
          pixels.show();                           
      }
      delay(300);                           
      pixels.clear();
      pixels.show();
      delay(300);                           
  }
  errors_count += 1;
  return;
}

// light the led according to the note in white
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


// MEDIUM MODE FUNCTIONS 
void medium_mode(){
  Serial.println("You are in medium mode");

  //play_song();
  note_index=0;
  while (note_index<count_note) {
     
    int num_note = num_notes[note_index]; // How many notes in the same time

    // if one note 
    if (num_note==1){

      // GET THE NOTE
      String note = notes[note_index];
      int time = duration_note[note_index];
      // LIGHT THE LED
      light_led_note(note);

      // LET PLAYER PLAY THE KEY
      start_time = millis();
      play_one_key_note(note);
      touch_time= millis();
      duration_total += (touch_time-start_time);

      // TURN OFF THE LED
      //Serial.println("turn off pixels");
      pixels.clear();
      pixels.show();  
      note_index+=1;
      continue;
    }

    // if two notes 
    if (num_note==2){
      Serial.println("There are 2 notes");

      //first note
      String note1 = notes[note_index];
      int time1 = duration_note[note_index];
      note_index++;
      
      //second note
      String note2 = notes[note_index];
      int time2 = duration_note[note_index];
      Serial.println(note1);
      Serial.println(note2);

      // light notes in white
      light_led_note(note1);
      light_led_note(note2);

      // LET THE PLAYER PLAY THE 2 NOTES
      start_time = millis();
      play_two_keys_notes(note1, note2);
      touch_time= millis();
      duration_total += (touch_time-start_time);

      // TURN OFF THE LED
      pixels.clear();
      pixels.show();  
      note_index+=1;
      continue;
    }
  }
}


// CALL TO FUNCTION TO PLAY THE 2 KEYS WITH THE RIGHT PARAMETERS 
void play_two_keys_notes(String note1, String note2){

  // DEFINE NOTE 1 PARAMETERS
  int touchpin1 = DO1_TouchPin;
  int channel1 = DO1_CHANNEL;
  int pixel1 = DO1_PIXEL;

  if (note1=="RE"){
    touchpin1 = RE_TouchPin;
    channel1 = RE_CHANNEL;
    pixel1 = RE_PIXEL;
  }


  if (note1=="MI"){
    touchpin1 = MI_TouchPin;
    channel1 = MI_CHANNEL;
    pixel1 = MI_PIXEL;  
  }

  if (note1=="FA"){
    touchpin1 = FA_TouchPin;
    channel1 = FA_CHANNEL;
    pixel1 = FA_PIXEL;  
  }
  
  if (note1=="SOL"){
    touchpin1 = SOL_TouchPin;
    channel1 = SOL_CHANNEL;
    pixel1 = SOL_PIXEL;
  }

  if (note1=="LA"){
    touchpin1 = LA_TouchPin;
    channel1 = LA_CHANNEL;
    pixel1 = LA_PIXEL;
  }

  if (note1=="SI"){
    touchpin1 = SI_TouchPin;
    channel1 = SI_CHANNEL;
    pixel1 = SI_PIXEL;
  }

  if (note1=="DO2"){
    touchpin1 = DO2_TouchPin;
    channel1 = DO2_CHANNEL;
    pixel1 = DO2_PIXEL;
  }


  // DEFINE NOTE 2 PARAMETERS
  int touchpin2 = DO1_TouchPin;
  int channel2 = DO1_CHANNEL;
  int pixel2 = DO1_PIXEL;

  if (note2=="RE"){
    touchpin2 = RE_TouchPin;
    channel2 = RE_CHANNEL;
    pixel2 = RE_PIXEL;
  }


  if (note2=="MI"){
    touchpin2 = MI_TouchPin;
    channel2 = MI_CHANNEL;
    pixel2 = MI_PIXEL;  
  }

  if (note2=="FA"){
    touchpin2 = FA_TouchPin;
    channel2 = FA_CHANNEL;
    pixel2 = FA_PIXEL;  
  }
  
  if (note2=="SOL"){
    touchpin2 = SOL_TouchPin;
    channel2 = SOL_CHANNEL;
    pixel2 = SOL_PIXEL;
  }

  if (note2=="LA"){
    touchpin2 = LA_TouchPin;
    channel2 = LA_CHANNEL;
    pixel2 = LA_PIXEL;
  }

  if (note2=="SI"){
    touchpin2 = SI_TouchPin;
    channel2 = SI_CHANNEL;
    pixel2 = SI_PIXEL;
  }

  if (note2=="DO2"){
    touchpin2 = DO2_TouchPin;
    channel2 = DO2_CHANNEL;
    pixel2 = DO2_PIXEL;
  }

  play_two_keys_params(touchpin1, channel1, pixel1, touchpin2, channel2, pixel2);

}

// to play 2 keys 
// wait for the 2 keys to be played, play sound if touched, alert error if error 
void play_two_keys_params(int touch_pin1, int channel1, int pixel1, int touch_pin2, int channel2, int pixel2){
  currtouched = cap.touched();
  int first_touched=0;
  int second_touched=0;
  int two_notes_touched=0;
  unsigned long start_firt_touch=0;
  unsigned long start_both_touched=0;

  while (two_notes_touched==0){
    currtouched = cap.touched();
    // if two notes are touched, break
    if (first_touched==1 && second_touched==1){
      two_notes_touched=1;
      start_both_touched=millis();
      break;
    }

    //if first note is touched ==> light on the sound and turn led green 
    if (currtouched & _BV(touch_pin1)){
      //Serial.println("1 is touched ");

      // start chrono to check if touched in the same time
      if (second_touched==0 && first_touched==0){
        start_firt_touch = millis();
      }
      first_touched=1;

      ledcWrite(channel1, 128); 
      pixels.setPixelColor(pixel1, pixels.Color(0, 255, 0));
      pixels.show();
    }

    // if first note was touched and is now released turn off sound (can happen 0.5 secs)
    if (!(currtouched & _BV(touch_pin1)) && first_touched==1){
      first_touched=0;
      ledcWrite(channel1, 0); 
      pixels.setPixelColor(pixel1, pixels.Color(255, 255, 255));
      pixels.show(); 
    }

    //if second note is touched ==> light on the sound and led green
    if (currtouched & _BV(touch_pin2)){
      //Serial.println("2 is touched ");

      // start chrono to check if touched in the same time
      if (second_touched==0 && first_touched==0){
        start_firt_touch = millis();
      }
      second_touched=1;
      ledcWrite(channel2, 128); 
      pixels.setPixelColor(pixel2, pixels.Color(0, 255, 0));
      pixels.show(); 
    }

    // if second note was touched and is now released turn off sound 
    if (!(currtouched & _BV(touch_pin2)) && second_touched==1){
      first_touched=0;
      ledcWrite(channel2, 0); 
      pixels.setPixelColor(pixel2, pixels.Color(255, 255, 255));
      pixels.show(); 
    }

    //if there is a touch but its none of the notes exepcted ==> error 
    if (currtouched && !(currtouched & _BV(touch_pin1)) && !(currtouched & _BV(touch_pin2))){
        ledcWrite(channel2, 0); 
        ledcWrite(channel1, 0); 
        false_touched_note_medium_mode(currtouched); // turn in red and make sound
        //error_touch(); //LIGHT IN RED

        // turn on leds on white
        pixels.setPixelColor(pixel1, pixels.Color(255, 255, 255));
        pixels.setPixelColor(pixel2, pixels.Color(255, 255, 255));
        pixels.show();
        //reset 
        first_touched=0;
        second_touched=0;
        two_notes_touched=0;
        start_firt_touch=0;
        continue; 
    }

    // when there is a touch and it's not an expected note => error
    if (currtouched && false_touched_pin(touch_pin1, touch_pin2)){

        // turn off sound in case it was played before
        ledcWrite(channel2, 0); 
        ledcWrite(channel1, 0); 
        false_touched_note_medium_mode(currtouched); // play the wrong touched note + red led

        // turn on leds on white 
        pixels.setPixelColor(pixel1, pixels.Color(255, 255, 255));
        pixels.setPixelColor(pixel2, pixels.Color(255, 255, 255));
        pixels.show();
        //reset 
        first_touched=0;
        second_touched=0;
        two_notes_touched=0;
        start_firt_touch=0;
        continue; 
    }

    //if one of the note has been touched, check the time
    if (start_firt_touch!=0)
    {
      unsigned long time_passed = millis()- start_firt_touch;
      
      //too long to touch the second one 
      if (time_passed>300){
        Serial.println("more than 500");

        ledcWrite(channel2, 0); 
        ledcWrite(channel1, 0);
        error_touch(); //LIGHT IN RED and add error

        //turn on leds on white
        pixels.setPixelColor(pixel1, pixels.Color(255, 255, 255));
        pixels.setPixelColor(pixel2, pixels.Color(255, 255, 255));
        pixels.show();
          
        //reset 
        first_touched=0;
        second_touched=0;
        two_notes_touched=0;
        start_firt_touch=0;
        continue;
      }
    }
  }

  Serial.println("2 are touched waiting for release");
  // the two notes have been touched 
  // waiting for release
  unsigned long start_firt_release=0;
  int first_released=0;
  int second_released=0;
  unsigned long time_played = 0;
  int error_made = 0;
  while(1){
    currtouched = cap.touched();
    time_played = millis()-start_both_touched;

    // if mode advanced, color code
    if (mode==ADVANCED_MODE)
    {
      // if less than 100 ms until the end of the duration expected => warn with yellow light
      if (duration_note[note_index]>time_played){
        
        /*Serial.print("you still need to play: ");
        Serial.print(duration_note[note_index]-time_played);
        Serial.println(" ms ");*/

        if (duration_note[note_index]-time_played < 250)
        {
          //Serial.println("show yellow ");
          pixels.setPixelColor(pixel1, pixels.Color(255,255,0)); // yellow
          pixels.setPixelColor(pixel2, pixels.Color(255,255,0)); // yellow
          pixels.show(); 
        }
      }

      // if played more the duration expected but less than 100ms => light orange light 
      if (duration_note[note_index]<=time_played){

        /*Serial.print("You have played too much by: ");
        Serial.print(time_played-duration_note[note_index]);
        Serial.println(" ms ");*/

        if (time_played-duration_note[note_index] < 300)
        {
          //Serial.println("show orange ");
          pixels.setPixelColor(pixel1, pixels.Color(255, 165, 0)); // orange
          pixels.setPixelColor(pixel2, pixels.Color(255,165,0)); // orange
          pixels.show(); 

        }

        if (time_played-duration_note[note_index] >= 300)
        {

          //Serial.println("show red ");
          pixels.setPixelColor(pixel1, pixels.Color(255,0,0)); //  red
          pixels.setPixelColor(pixel2, pixels.Color(255,0,0)); // red

          errors_count+=1; //count an error
          pixels.show(); 
          if (error_made==0){
            note_index = note_index-2; //try these notes again
            num_note_index-=1;
            Serial.print("removing one: ");
            Serial.println(note_index);
            Serial.println(num_note_index);
          }
          error_made=1;
        }
      }
    }
    // the two notes are released 
    if (!(currtouched & _BV(touch_pin2)) && !(currtouched & _BV(touch_pin1)))
    {
        Serial.println("2 are released");
      //light down sound and turn off led
        pixels.setPixelColor(pixel1, pixels.Color(0, 0, 0));
        pixels.setPixelColor(pixel2, pixels.Color(0, 0, 0));
        pixels.show();
        ledcWrite(channel2, 0); 
        ledcWrite(channel1, 0); 
        break;
    }

    // the note 2 is released 
    if (!(currtouched & _BV(touch_pin2))){
        ledcWrite(channel2, 0); 
        pixels.setPixelColor(pixel2, pixels.Color(255, 0, 0));
        pixels.show();
        // start chrono
        if (first_released==0 && second_released==0){
          start_firt_release=millis();
        }
        second_released=1;

    }

    // the note 1 is released
    if (!(currtouched & _BV(touch_pin1))){
        ledcWrite(channel1, 0); 
        pixels.setPixelColor(pixel1, pixels.Color(255, 0, 0));
        pixels.show();
        if (first_released==0 && second_released==0){
          start_firt_release=millis();
        }
        first_released=1;
    }

    // check the time btw the releases of the 2 notes
    if (start_firt_release!=0){
      unsigned long time_released = millis()- start_firt_release;
      if (time_released>500){
        ledcWrite(channel2, 0); 
        ledcWrite(channel1, 0); 
        error_touch(); //red
        note_index-=2; // make the user play again those notes 
        num_note_index-=1;
        break;
      }
    }
  }
}

// Check if a pin was falsly touched 
int false_touched_pin(int touchpin1, int touchpin2){
  for (int i=0; i<7; i++){
    if (i!=touchpin1 && i!=touchpin2)
    {
      if ((currtouched & _BV(i))){
        return 1;
      }
    }
  }
  return 0;
}

// play the wrong note and turn the led red 
void false_touched_note_medium_mode(uint32_t currtouched){
    
    while (currtouched & _BV(DO1_TouchPin)){
      //light in red and make sound
      pixels.setPixelColor(DO1_PIXEL, pixels.Color(255, 0, 0));
      pixels.show();
      ledcWrite(DO1_CHANNEL, 128);
      currtouched = cap.touched();
      //light down led and turn off sound
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


// ADVANCED MODE 
void advanced_mode(){
  Serial.println("advanced mode");
  
  //play_song();
  num_note_index=0;
  note_index=0;
  while (note_index<count_note) {
     Serial.print("note index is ");
     Serial.println(note_index);
    int num_note = num_notes[num_note_index]; // How many notes in the same time
    num_note_index+=1;
    Serial.println("num notes is ");
    Serial.println(num_note);

    // if one note 
    if (num_note==1){

      // GET THE NOTE
      String note = notes[note_index];
      int time = duration_note[note_index];
      // LIGHT THE LED
      light_led_note(note);

      // LET PLAYER PLAY THE KEY
      start_time = millis();
      play_one_key_note(note);
      touch_time= millis();
      duration_total += (touch_time-start_time);

      // TURN OFF THE LED
      //Serial.println("turn off pixels");
      pixels.clear();
      pixels.show();  
      note_index+=1;
      continue;
    }

    // if two notes 
    if (num_note==2){
      Serial.println("There are 2 notes");

      //first note
      String note1 = notes[note_index];
      int time1 = duration_note[note_index];
      note_index++;
      
      //second note
      String note2 = notes[note_index];
      int time2 = duration_note[note_index];
      Serial.println(note1);
      Serial.println(note2);

      // light notes in white
      light_led_note(note1);
      light_led_note(note2);

      // LET THE PLAYER PLAY THE 2 NOTES
      start_time = millis();
      play_two_keys_notes(note1, note2);
      touch_time= millis();
      duration_total += (touch_time-start_time);

      // TURN OFF THE LED
      pixels.clear();
      pixels.show();  
      note_index+=1;
      continue;
    }
  }
}

// RECORD MODE 
/*void record_mode(){
  unsigned long last_time_touched = millis();

  while (millis()-last_time_touched<3000){
    currtouched = cap.touched();
    
    // note is touched 
    if ((currtouched & _BV(DO1_TouchPin))){

      ledcWrite(DO1_CHANNEL, 128); 
      pixels.setPixelColor(DO1_PIXEL, pixels.Color(255, 255, 255));
      pixels.show();

      
    }

    if ((currtouched & _BV(RE_TouchPin))){
      //Serial.println("RE IS touched");

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
}*/


// Screen functions 
void menu_mode(){
  
  //show the menu with the cursor (inverse)
  show_menu_mode(mode);
  
  while(1){
    buttonUpState = digitalRead(buttonUpPin);
    buttonDownState = digitalRead(buttonDownPin);
    buttonSelectState = digitalRead(buttonSelectPin);

    // UP 
    if (buttonUpState == HIGH) {
      if (mode!=1){
        mode= mode-1;
      }
      break;
    }

    // DOWN
    if (buttonDownState == HIGH) {
      if(mode!=5){
        mode=mode+1;
      }
      break;
    }

    // SELECT
    if (buttonSelectState == HIGH) {
      //mode number (1,2,3,4,5) is in "mode" variable
      Serial.print("Selected mode ");
      Serial.println(mode);
      if (mode==FREE_MODE){
        start_game();
      }
      else{
        call_menu_song();
      }
      break;
    }
  }
}

void show_menu_mode(int mode){

  display.clearDisplay();
  display.setCursor(0,0);             // Start at top-left corner

  display.setTextSize(1);             // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);        // Draw white text
  display.println(F("Choose mode: "));
  
  if (mode==EASY_MODE){
    display.setTextColor(SSD1306_BLACK, SSD1306_WHITE); // Draw 'inverse' text
    display.println(F("Easy"));

    display.setTextColor(SSD1306_WHITE);        // Draw white text
    display.println(F("Medium"));
    display.println(F("Advanced"));
    display.println(F("Free"));
    display.println(F("Record"));

  }

  if (mode==MEDIUM_MODE){
    display.println(F("Easy"));

    display.setTextColor(SSD1306_BLACK, SSD1306_WHITE); // Draw 'inverse' text
    display.println(F("Medium"));
    
    display.setTextColor(SSD1306_WHITE);        // Draw white text
    display.println(F("Advanced"));
    display.println(F("Free"));
    display.println(F("Record"));
  }

  if(mode==ADVANCED_MODE){
    display.println(F("Easy"));
    display.println(F("Medium"));
    
    display.setTextColor(SSD1306_BLACK, SSD1306_WHITE); // Draw 'inverse' text
    display.println(F("Advanced")); 
    display.setTextColor(SSD1306_WHITE);

    display.println(F("Free"));       
    display.println(F("Record"));
  }

  if(mode==FREE_MODE){
    display.println(F("Easy"));
    display.println(F("Medium"));
    display.println(F("Advanced"));
    
    display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
    display.println(F("Free")); 
    display.setTextColor(SSD1306_WHITE);

    display.println(F("Record"));
  }

  if(mode==RECORD_MODE){
    display.println(F("Easy"));
    display.println(F("Medium"));
    display.println(F("Advanced"));
    display.println(F("Free")); 
    
    display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
    display.println(F("Record"));
  }
  display.display();
  delay(250);
}

void call_menu_song(){
  while(returnValue==0){
    menu_song();
  }
  returnValue=0;
}

void menu_song(){

  //SONG NUM IS 0,5,10... (multiples of 5)
  //SONG INDEX IS 1,2,3,4,5 (on screen)
  //We show from 5 songs from songNum
  //When chosing we do songNum+songIndex-1 to know which song has been chosen
  show_menu_song(); 
  
  while(1){
    buttonUpState = digitalRead(buttonUpPin);
    buttonDownState = digitalRead(buttonDownPin);
    buttonSelectState = digitalRead(buttonSelectPin);
    buttonReturnState = digitalRead(buttonReturnPin);
    
    // UP 
    if (buttonUpState == HIGH) {
      Serial.println("HIGH");
      //if we are all up on the screen and there is a page before 
      if (songIndex == 1){
        if(songNum!=0) {
          songNum -=5; //get 5 before (we display them 5 by 5)
        }
      }
      else{
        songIndex -=1; //go up by 1
      }
      break;
    }

    // DOWN
    if (buttonDownState == HIGH) {
      Serial.println("DOWN");
      Serial.print("songNum: ");
      Serial.println(songNum);
      Serial.print("songIndex: ");
      Serial.println(songIndex);
      //to check here the second condition
      if (songIndex == 5){ 
        if (songNum+5<NumSongsTotal){
          songNum +=5; //get 5 after (we display them 5 by 5)
        }
      }
      else if (songIndex+songNum<NumSongsTotal){
        songIndex +=1; //go down by 1
      }
      break;
    }

    // SELECT
    if (buttonSelectState == HIGH) {
      songNum = songNum+songIndex-1;
      Serial.print("Song selected is ");
      Serial.println(songNum);
      // LOADING THE SONG
      display.clearDisplay();
      display.setCursor(0,0); 
      display.setTextSize(1);                     // Normal 1:1 pixel scale
      display.setTextColor(SSD1306_WHITE); 
      display.println(F("loading the song..... "));
      display.display();
      
      String payload = readFromGoogleSheets(songNum);
      if (payload!=""){
        split_arrays(payload); //keep the music array (notes,dur,num) in global arrays
      } 

      display.clearDisplay();
      display.setCursor(0,0); 
      display.println(F("Let's play!!"));
      display.display();

      Serial.println(payload);

      start_game();
      songNum=0;
      songIndex=1;
      return;
    }

    if (buttonReturnState==HIGH) {
      songNum=0;
      songIndex=1;
      mode=1;
      returnValue=1;
      return;
    }
  }
}

void show_menu_song(){
  display.clearDisplay();
  display.setCursor(0,0);             // Start at top-left corner
  Serial.println("show menu songs");

  // take 5 next songs from songNum 
  if (songNum%5==0){
    song1 = SongNames[songNum];
    Serial.print("song 1 is: ");
    Serial.println(song1);
    song2 = SongNames[songNum+1];
    Serial.print("song 2 is: ");
    Serial.println(song2);
    song3 = SongNames[songNum+2];
    Serial.print("song 3 is: ");
    Serial.println(song3);
    song4 = SongNames[songNum+3];
    Serial.print("song 4 is: ");
    Serial.println(song4);
    song5 = SongNames[songNum+4];
    Serial.print("song 5 is: ");
    Serial.println(song5);
  }

  display.setTextSize(1);                     // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);        // Draw white text
  display.println(F("Choose song: "));

  if (songIndex==1){
    display.setTextColor(SSD1306_BLACK, SSD1306_WHITE); // Draw 'inverse' text
    display.println(song1);
    display.setTextColor(SSD1306_WHITE);        // Draw white text

    display.println(song2);
    display.println(song3);
    display.println(song4);
    display.println(song5);

  }

  if (songIndex==2){
    display.println(song1);

    display.setTextColor(SSD1306_BLACK, SSD1306_WHITE); // Draw 'inverse' text
    display.println(song2);
    display.setTextColor(SSD1306_WHITE);        // Draw white text

    display.println(song3);
    display.println(song4);
    display.println(song5);
  }

  if(songIndex==3){
    display.println(song1);
    display.println(song2);
    
    display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
    display.println(song3);
    display.setTextColor(SSD1306_WHITE);        // Draw white text

    display.println(song4);
    display.println(song5);

  }

  if(songIndex==4){
    display.println(song1);
    display.println(song2);
    display.println(song3);

    display.setTextColor(SSD1306_BLACK, SSD1306_WHITE); // Draw 'inverse' text
    display.println(song4);
    display.setTextColor(SSD1306_WHITE);        // Draw white text

    display.println(song5);
  }

  if(songIndex==5){
    display.println(song1);
    display.println(song2);
    display.println(song3);
    display.println(song4);
    display.setTextColor(SSD1306_BLACK, SSD1306_WHITE); // Draw 'inverse' text
    display.println(song5);
  }
  display.display();
  delay(250);
}

// GOOGLE SHEET FUNCTIONS // 
void split_arrays(String payload){

  char* token; // Pointer to hold each token

  // Convert the payload string to a character array
  char payloadArray[payload.length() + 1];
  payload.toCharArray(payloadArray, payload.length() + 1);

  // Split the payload by '/'
  token = strtok(payloadArray, "/");
  String song = String(token); // name of song

  token = strtok(NULL, "/");
  count_note = atoi(token); // count number

  token = strtok(NULL, "/");
  String notesString = String(token); // notes array: DO,RE,MI,FA

  token = strtok(NULL, "/");
  String durationString = String(token); // durationa array: 500,500

  token = strtok(NULL, "/");
  String numString = String(token); // num array: 1,1,8

  // Split notes, duration, and num strings by ','
  int i = 0;
  token = strtok(notesString.begin(), ",");
  while (token != NULL) {
    notes[i++] = String(token);
    token = strtok(NULL, ",");
  }

  i = 0;
  token = strtok(durationString.begin(), ",");
  while (token != NULL) {
    duration_note[i++] = atoi(token);
    token = strtok(NULL, ",");
  }

  
  i = 0;
  token = strtok(numString.begin(), ",");
  while (token != NULL) {
    num_notes[i++] = atoi(token);
    token = strtok(NULL, ",");
  }

  // Output the values
  Serial.print("Song: ");
  Serial.println(song);
  Serial.print("Count: ");
  Serial.println(count_note);
  Serial.print("Notes: ");
  for (int i = 0; i < 200; i++) { // Assuming a maximum of 4 notes
    Serial.print(notes[i]);
    Serial.print(", ");
  }
  Serial.println();
  Serial.print("Duration: ");
  for (int i = 0; i < 200; i++) { // Assuming a maximum of 2 durations
    Serial.print(duration_note[i]);
    Serial.print(", ");
  }
  Serial.println();
  Serial.print("Num: ");
  for (int i = 0; i < 200; i++) { // Assuming a maximum of 3 numbers
    Serial.print(num_notes[i]);
    Serial.print(", ");
  }
  Serial.println();

}


void split_song_names(String payload){
  char* token; // Pointer to hold each token

  // Convert the payload string to a character array
  char payloadArray[payload.length() + 1];
  payload.toCharArray(payloadArray, payload.length() + 1);

  // Initialize song count
  int songCount = 0;

  // Split the payload by '/'
  token = strtok(payloadArray, "/");
  
  // Parse each token until reaching the maximum number of songs or the end of the payload
  while (token != NULL && songCount < NumSongsTotal) {
    // Store the song name in the array
    SongNames[songCount++] = String(token);
    // Get the next token
    token = strtok(NULL, "/");
  }
  Serial.println("Song names: ");
  for (int i = 0; i < 1000; i++) { // Assuming a maximum of 4 notes
    Serial.print(SongNames[i]);
    Serial.print(", ");
  }
  Serial.println(" ");
}

// https://script.google.com/macros/s/AKfycbxRAWCE3StioChkFAVhGty7HhOpmaQUy2o_HJHqLdWCUnuJZCls9yPk9l1o_CuXOQbXUA/exec?action=read&row=1
// https://script.google.com/macros/s/AKfycbxRAWCE3StioChkFAVhGty7HhOpmaQUy2o_HJHqLdWCUnuJZCls9yPk9l1o_CuXOQbXUA/exec?action=read&row=1&name=1&notes=DO,RE&duration=500,500,500&num=2,2,2,2
// https://script.google.com/macros/s/AKfycbxRAWCE3StioChkFAVhGty7HhOpmaQUy2o_HJHqLdWCUnuJZCls9yPk9l1o_CuXOQbXUA/exec?action=write&song=Happy&count=12&

void writetoGoogleSheets(String song, int coount, String notes, String duration, String num)
{
  if (WiFi.status() == WL_CONNECTED) {

      String url = "https://script.google.com/macros/s/" + GOOGLE_SCRIPT_ID+"/exec" ;
      url += "?action=write";
      url += "&song=" + song;
      url += "&count=" + String(coount);
      url += "&notes=" + notes;
      url += "&duration=" + duration;
      url += "&num=" + num;

      Serial.println(url);
      HTTPClient http;
      http.begin(url.c_str());
      http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
      int httpCode = http.GET(); 

      String payload;
      if (httpCode > 0) {
          payload = http.getString();
          Serial.println("Payload: "+ payload);    
      }
      http.end();
    }
    Serial.println("Request sent to write data to Google Sheets");
}


String readFromGoogleSheets(int song) {
 if (WiFi.status() == WL_CONNECTED) {

    String url = "https://script.google.com/macros/s/" + GOOGLE_SCRIPT_ID+"/exec" ;
    url += "?action=read";
    url += "&row=" + String(song);

    HTTPClient http;
    http.begin(url.c_str());
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    int httpCode = http.GET(); 
    String payload;

    if (httpCode > 0) { //Check for the returning code
      payload = http.getString();
    }
    return payload;
  }
}

String readNameFromGoogleSheets(int song) {
 if (WiFi.status() == WL_CONNECTED) {

    String url = "https://script.google.com/macros/s/" + GOOGLE_SCRIPT_ID+"/exec" ;
    url += "?action=read";
    url += "&row=" + String(song);
    url += "&name=1";

    HTTPClient http;
    http.begin(url.c_str());
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    int httpCode = http.GET(); 
    String payload;

    if (httpCode > 0) { //Check for the returning code
      payload = http.getString();
    }
    return payload;
  }
}

String readColumnFromGoogleSheets(){
  if (WiFi.status() == WL_CONNECTED) {

    String url = "https://script.google.com/macros/s/" + GOOGLE_SCRIPT_ID+"/exec" ;
    url += "?action=read";
    url += "&column=1";

    Serial.println(url);
    HTTPClient http;
    http.begin(url.c_str());
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    int httpCode = http.GET(); 
    String payload;

    if (httpCode > 0) { //Check for the returning code
      payload = http.getString();
    }
    return payload;
  }
}


// SET UP FUNCTIONS //
void welcome_sound_and_led(){
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

void setup_screen(){
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  display.display();
  String column = readColumnFromGoogleSheets();
  Serial.println("songs are: ");
  Serial.println(column);
  split_song_names(column);
  //delay(2000);

  // Clear the buffer
  display.clearDisplay();
  pinMode(buttonUpPin, INPUT);
  pinMode(buttonDownPin, INPUT);
  pinMode(buttonSelectPin, INPUT);
  pinMode(buttonReturnPin, INPUT);
}

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
}

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

