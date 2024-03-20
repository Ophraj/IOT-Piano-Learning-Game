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
// You can have up to 4 on one i2c bus but one is enough for testing!
Adafruit_MPR121 cap = Adafruit_MPR121();
uint32_t currtouched = 0;

// SOUND PARAMETERS AND SETYP
const int PWM_RESOLUTION = 8; 
    // We'll use same resolution as Uno (8 bits, 0-255) but ESP32 can go up to 16 bits 
    // The max duty cycle value based on PWM resolution (will be 255 if resolution is 8 bits)
const int MAX_DUTY_CYCLE = (int)(pow(2, PWM_RESOLUTION) - 1); 
int dc = 128;


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

// Global variables
String mode = "MEDIUM";
uint32_t note_index = 0;

// Stats variables
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
  
  if (!SPIFFS.begin(true)) {
    Serial.println("An error occurred while formatting SPIFFS");
    return;
  }

  // SET UP FOR THE SOUND 
  setup_sound();

  //Welcome sound and led
  welcome_sound_and_led();
}

void loop() {
  note_index=0;

  if (mode == "EASY"){
    easy_mode();
    mode = "FREE";
  }

  if(mode=="FREE_PLAYING") {
      free_playing();
  }

  if (mode=="MEDIUM"){
    medium_mode();
  }
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
            if (mode=="EASY"){
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

    // wait for release
    while(1){
      time_played = millis()-start_play;
      //Serial.println("wait for release");
      currtouched = cap.touched();
      if (!(currtouched & _BV(touch_pin))) {
        ledcWrite(channel, 0); 
        break;
      }

      // if mode advanced, color code
      if (mode=="ADVANCED")
      {
        // if less than 100 ms until the end of the duration expected => warn with yellow light
        if (duration_note[note_index]-time_played < 100 && duration_note[note_index]-time_played>=0)
        {
          pixels.setPixelColor(pixel, pixels.Color(255,255,0)); // yellow
        }

        // if played more the duration expected but less than 100ms => light orange light 
        if (time_played-duration_note[note_index] < 100 && time_played-duration_note[note_index]>=0)
        {
          pixels.setPixelColor(pixel, pixels.Color(255, 165, 0)); // orange
        }

        if (time_played-duration_note[note_index] >= 100)
        {
          pixels.setPixelColor(pixel, pixels.Color(255,0,0)); // turn red
          errors_count+=1; //count an error
          note_index -=1; //try this note again
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
        Serial.print(start_firt_touch);
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
        Serial.print(start_firt_touch);
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
      if (time_passed>500){
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
  
  while(1){
    currtouched = cap.touched();
    time_played = millis()-start_both_touched;

    if (mode== "ADVANCED")
    {
      // if less than 100 ms until the end of the duration expected => warn with yellow light
        if (duration_note[note_index]-time_played < 100 && duration_note[note_index]-time_played>=0)
        {
          pixels.setPixelColor(pixel, pixels.Color(255,255,0)); // yellow
        }

        // if played more the duration expected but less than 100ms => light orange light 
        if (time_played-duration_note[note_index] < 100 && time_played-duration_note[note_index]>=0)
        {
          pixels.setPixelColor(pixel, pixels.Color(255, 165, 0)); // orange
        }

        if (time_played-duration_note[note_index] >= 100)
        {
          pixels.setPixelColor(pixel, pixels.Color(255,0,0)); // turn red
          errors_count+=1; //count an error
          note_index -=1; //try this note again
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
  Serial.println("You are in advanced mode");

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

