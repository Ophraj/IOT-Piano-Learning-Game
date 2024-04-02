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
//#define WIFI_SSID "Ophra Iphone"
//#define WIFI_PASSWORD "ophraaaa"

#define WIFI_SSID "OnePlus 9 Pro 5G"
#define WIFI_PASSWORD "Nookico9456"
String GOOGLE_SCRIPT_ID = "AKfycbwlg2OwYamMHBB0E_JjJCKnY6CgBRMNJrfkk1A5IylR23Fppmiq28LNF1IXVHFMFYY_6w";    // change Google script ID

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
#define STATS_MODE 6

// Global variables
int wifi_connected=0;
int count_note = 0;
String notes[200]; // Assuming a maximum of 200 notes
int duration_note[200]; // Assuming a maximum of 200 durations
int num_notes[200]; // Assuming a maximum of 200 numbers

String notes_TOADD; 
String duration_note_TOADD; 
String num_notes_TOADD; 
int count_TOADD=0;

String SongNames[1000];
int NumSongsTotal = 3;
int mode = 1;
int songNum = 0;
int songIndex = 1;
int wifi_mode = 0;

int error_display=0;
double time_display=0;

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
double duration_average=0;

int count_note_welcome = 15;
String notes_welcome[] = {"MI","MI","FA","SOL","SOL","FA","MI","RE","DO1","DO1","RE","MI","MI","RE","RE"};
int duration_note_welcome[] = {500,500,500,500,500,500,500,500,500,500,500,500,500,500,1000};
int num_notes_welcome[] = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};

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

  // SET UP FOR SCREEN and google sheet
  setup_screen();
  
  //Welcome sound and led
  
  welcome_sound_and_led();
}

void loop() {
  menu_wifi();
}


void start_game() {
  note_index=0;
  num_note_index=0;
  errors_count=0;
  duration_total=0;
  duration_average=0;
  Serial.println("Starting game");

  if (mode == EASY_MODE){
    play_all_modes();
    if (returnValue!=1){
      display_stats();
    }
    return;
  }

  if (mode== MEDIUM_MODE){
    play_all_modes();
    if (returnValue!=1){
      display_stats();
    }    
    return;
  }

  if (mode == ADVANCED_MODE){
    play_all_modes();
    if (returnValue!=1){
      display_stats();
    }
    return;
  }

  if(mode== FREE_MODE) {
      Serial.println("FREE mode");
      free_playing();
      return;
  }

  if (mode== RECORD_MODE){
    Serial.println("RECORD mode");

    notes_TOADD= ""; 
    duration_note_TOADD = ""; 
    num_notes_TOADD = ""; 
    count_TOADD=0;

    String name_TOADD = "SongNum" + String(NumSongsTotal);
    display.clearDisplay();
    display.setCursor(0,0); 
    display.setTextSize(1);            
    display.setTextColor(SSD1306_WHITE);

    display.println(F("Record Mode"));
    display.println(F(" "));
    display.print(F("Song name: "));
    display.println(name_TOADD);
    display.println(F(" "));
    display.println(F("To validate press:"));
    display.print(F("SELECT"));
    display.display();

    record_mode();
    return;
  }
}

void display_stats(){
  display.clearDisplay();
  display.setCursor(0,0); 
  display.setTextSize(1);            
  display.setTextColor(SSD1306_WHITE);
  //unsigned long display_start=millis();

  if (errors_count == 0){
    display.println(F("You had no errors!"));
    display.println(F("     Perfect :)!"));
  }

  else if (errors_count == 1){
    display.println(F("You had 1 error!"));
    display.println(F("  Almost perfect!"));
  }

  else if (errors_count <= 3){
    display.print(F("You had "));
    display.print(errors_count);
    display.println(F(" errors."));

    display.println(F("You can do better!"));
  }
  else {
    display.print(F("You had "));
    display.print(errors_count);
    display.println(F(" errors."));

    display.println(F("Practice again!"));
  }
  display.println(F(""));
  display.print(F("average response time is: "));
  display.print(duration_average);
  display.println(F(" sec"));
  display.println(F(" "));
  display.println(F("To skip press SELECT"));

  display.display();
  writeStatstoGoogleSheets(songNum, errors_count, duration_average);
  
  while (1)
  {
    buttonSelectState = digitalRead(buttonSelectPin);
    if (buttonSelectState==HIGH) {
      delay(100);
      return;
    }
  }

}

// PLAY THE SONG
void play_song(){
  num_note_index=0;
  note_index=0;  
  while (note_index<count_note) {
    int num_note = num_notes[num_note_index]; // How many notes in the same time
    num_note_index+=1;

    buttonSelectState = digitalRead(buttonSelectPin);
    if (buttonSelectState==HIGH) {
      turnOffAllSounds();
      pixels.clear();
      pixels.show();
      note_index=0;
      num_note_index=0;
      delay(200);
      return;
    }

    if (num_note==1){
      String note = notes[note_index];
      int time = duration_note[note_index];

      unsigned long start = millis();
      unsigned long end = millis();
      while (end-start<time)
      {
        buttonSelectState = digitalRead(buttonSelectPin);
        if (buttonSelectState==HIGH) {
          turnOffAllSounds();
          pixels.clear();
          pixels.show();
          note_index=0;
          num_note_index=0;
          delay(200);
          return;
        }   
        Serial.println("play song check return");
        if (checkReturnButton()==1)
        {
          return; 
        }
        end = millis();
        dc = 128;
        light_led_note(note); 
        turnOn_son_note(note);
      }

      turnOff_son_note(note);
      pixels.clear();
      pixels.show();
      if (checkReturnButton()==1)
        {
          return; 
        }  
      note_index++;
      delay(10);
    }

    if(num_note==2){
      //first note
      String note1 = notes[note_index];
      int time1 = duration_note[note_index];
      note_index++;
      
      //second note
      String note2 = notes[note_index];

      unsigned long start = millis();
      unsigned long end = millis();
      while (end-start<time1)
      {
        if (checkReturnButton()==1)
        {
          return; 
        }
        end = millis();
        dc = 128;
        light_led_note(note1);
        light_led_note(note2); 
        turnOn_son_note(note1);
        turnOn_son_note(note2);
      }

      turnOff_son_note(note1);
      turnOff_son_note(note2);
      pixels.clear();
      pixels.show();
      if (checkReturnButton()==1)
        {
          return; 
        }  
      note_index++;
      delay(10);
    }
  }

  //song played 
  delay(1000);
}

// turn on sound according to the note
void turnOn_son_note(String note){

  if (checkReturnButton()==1)
  {
    return; 
  }

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
  if (checkReturnButton()==1)
  {
    return; 
  } 
}

// light the led according to the note in white
void light_led_note(String note){

  if (checkReturnButton()==1)
  {
    return; 
  }
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

//FREE PLAYING FUNCTION
void free_playing(){
  Serial.println("into free playing ");

  while (1){
    if (checkReturnButton()==1)
    {
      return; 
    } 
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
}


// EASY, ADVANCED, MEIDUM MODES
void play_all_modes(){
  Serial.println("advanced mode");
  
  play_song();
  display.clearDisplay();
  display.setCursor(0,0); 
  display.println(F("Let's play!!"));
  display.display();

  if(returnValue==1)
  {
    return;
  }
  num_note_index=0;
  note_index=0;

  while (note_index<count_note) {
    if (checkReturnButton()==1)
    {
        return; 
    }
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
      if(returnValue==1)
      {
        return;
      }
      touch_time= millis();
      duration_total += (touch_time-start_time);

      // TURN OFF THE LED
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
      if (returnValue==1)
      {
        return;
      }
      touch_time= millis();
      duration_total += (touch_time-start_time);

      // TURN OFF THE LED
      pixels.clear();
      pixels.show();  
      note_index+=1;
      continue;
    }
  }

  pixels.clear();
  duration_average = duration_total/count_note;
  duration_average = duration_average/1000;
  Serial.print("You had ");
  Serial.print(errors_count);
  Serial.println(" mistakes");

  Serial.print("Your average response time is: ");
  Serial.println(duration_average);
  note_index=0;
  checkReturnButton();
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

  // wait for touch
  while (!(currtouched & _BV(touch_pin))){
    currtouched = cap.touched();
    if (checkReturnButton()==1)
    {
      return; 
    } 
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
          if (returnValue==1){
            return;
          }
          light_led_note(note); //Show our note led in white again
        }
        
        else if (mode==MEDIUM_MODE){ 
          errors_count += 1;
          false_touched_note_medium_mode(currtouched); //play the wrong key (sound and red color)
          if (returnValue==1){
            return;
          }
        }
        
        else {
          errors_count += 1;
          false_touched_note_advanced_one_key(touch_pin); //play the wrong keys 
          if (returnValue==1){
            return;
          }
          pixels.clear();
          pixels.show();
          return;
        }
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
    
    if (checkReturnButton()==1)
    {
      return; 
    }

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
        }
      }
    }
  }

  checkReturnButton();
  return;
}


// CALL TO FUNCTION TO PLAY THE 2 KEYS WITH THE RIGHT PARAMETERS 
void play_two_keys_notes(String note1, String note2){

  if (checkReturnButton()==1)
  {
    return; 
  }
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
  int first_touched=0;
  int second_touched=0;
  int two_notes_touched=0;
  unsigned long start_firt_touch=0;
  unsigned long start_both_touched=0;

  while (two_notes_touched==0){
    currtouched = cap.touched();
    if (checkReturnButton()==1)
    {
      return; 
    }
    // if two notes are touched, break
    if (first_touched==1 && second_touched==1){
      two_notes_touched=1;
      start_both_touched=millis();
      break;
    }

    //if first note is touched ==> light on the sound and turn led green 
    if (currtouched & _BV(touch_pin1)){
      // start chrono to check if touched around the same time
      if (second_touched==0 && first_touched==0){
        start_firt_touch = millis();
      }
      first_touched=1;

      ledcWrite(channel1, 128); 
      pixels.setPixelColor(pixel1, pixels.Color(0, 255, 0));
      pixels.show();
    }

    // if first note was touched and is now released turn off sound (can happen 0.3 secs)
    if (!(currtouched & _BV(touch_pin1)) && first_touched==1){
      first_touched=0;
      ledcWrite(channel1, 0); 
      pixels.setPixelColor(pixel1, pixels.Color(255, 255, 255));
      pixels.show(); 
    }

    //if second note is touched ==> light on the sound and led green
    if (currtouched & _BV(touch_pin2)){

      // start chrono to check if touched around the same time
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
      
      if (mode==EASY_MODE){
        error_touch();
      }
      
      else if (mode==MEDIUM_MODE){
        errors_count++;
        false_touched_note_medium_mode(currtouched); // turn in red and make sound
      }
      
      else { //advanced
        errors_count++;
        false_touched_note_advanced_two_keys(touch_pin1, touch_pin2);
        turnOffAllSounds();
        pixels.clear();
        pixels.show();
        return;
      }

      if (returnValue==1)
      {
        return;
      }

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
    if (currtouched && false_touched_pin_two_keys(touch_pin1, touch_pin2)){
      // turn off sound in case it was played before
      ledcWrite(channel2, 0); 
      ledcWrite(channel1, 0); 
      
      if (mode==EASY_MODE){
        error_touch();
      }
      else if (mode==MEDIUM_MODE){
        errors_count++;
        false_touched_note_medium_mode(currtouched); // turn in red and make sound
      }
      
      else {
        errors_count++;
        false_touched_note_advanced_two_keys(touch_pin1, touch_pin2);
        turnOffAllSounds();
        pixels.clear();
        pixels.show();
        return;
      }

      if (returnValue==1)
      {
        return;
      }

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
      if (mode==ADVANCED_MODE){
        if (time_passed>300){
          ledcWrite(channel2, 0); 
          ledcWrite(channel1, 0);
          error_touch(); //LIGHT IN RED and add error
          if (returnValue==1)
          {
            return;
          }
          return;
        }
      }

      if (mode==MEDIUM_MODE){
        if (time_passed>500){
          ledcWrite(channel2, 0); 
          ledcWrite(channel1, 0);
          error_touch(); //LIGHT IN RED and add error
          if (returnValue==1)
          {
            return;
          }
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

      if (mode==EASY_MODE){
        if (time_passed>1000){
          ledcWrite(channel2, 0); 
          ledcWrite(channel1, 0);
          error_touch(); //LIGHT IN RED and add error
          if (returnValue==1)
          {
            return;
          }
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

    if (checkReturnButton()==1)
    {
      return; 
    }
    // if mode advanced, color code
    if (mode==ADVANCED_MODE)
    {
      // if less than 100 ms until the end of the duration expected => warn with yellow light
      if (duration_note[note_index]>time_played){

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

        if (time_played-duration_note[note_index] < 300)
        {
          //Serial.println("show orange ");
          pixels.setPixelColor(pixel1, pixels.Color(255, 165, 0)); // orange
          pixels.setPixelColor(pixel2, pixels.Color(255,165,0)); // orange
          pixels.show(); 

        }

        if (time_played-duration_note[note_index] >= 300)
        {
          pixels.setPixelColor(pixel1, pixels.Color(255,0,0)); //  red
          pixels.setPixelColor(pixel2, pixels.Color(255,0,0)); // red

          errors_count+=1; //count an error
          pixels.show(); 
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
      if (mode==ADVANCED_MODE){
        if (time_released>300){
          ledcWrite(channel2, 0); 
          ledcWrite(channel1, 0); 
          error_touch(); //red
          if (mode!=ADVANCED_MODE){
            note_index-=2; // make the user play again those notes 
            num_note_index-=1;
          }
          break;
        }
      }

      if (mode==MEDIUM_MODE){
        if (time_released>500){
          ledcWrite(channel2, 0); 
          ledcWrite(channel1, 0); 
          error_touch(); //red
          note_index-=2; // make the user play again those notes 
          num_note_index-=1;
          break;
        }
      }
      if (mode==EASY_MODE){
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
  checkReturnButton();
}

// show all red leds 3 times and add one to errors_count
void error_touch(){
  for(int j=0; j<3; j++) { 
      for(int i=0; i<NUMPIXELS; i++) { 
          pixels.setPixelColor(i, pixels.Color(255, 0, 0));     //light in red 3 times  
          pixels.show();
          if (checkReturnButton()==1)
          {
            return; 
          }                          
      }
      delay(100);
      if (checkReturnButton()==1)
      {
        return; 
      }                            
      pixels.clear();
      pixels.show();
      delay(200); 
      if (checkReturnButton()==1)
      {
        return; 
      }                          
  }
  errors_count += 1;
  return;
}

// Check if a pin was falsly touched (two keys)
int false_touched_pin_two_keys(int touchpin1, int touchpin2){
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

// Check if a pin was falsly touched (one key)
int false_touched_pin_one_key(int touchpin1){
  for (int i=0; i<7; i++){
    if (i!=touchpin1)
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
      if (checkReturnButton()==1)
      {
        return; 
      }
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
      if (checkReturnButton()==1)
      {
        return; 
      }
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
      if (checkReturnButton()==1)
      {
        return; 
      }
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
      if (checkReturnButton()==1)
      {
        return; 
      }
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
      if (checkReturnButton()==1)
      {
        return; 
      }
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
      if (checkReturnButton()==1)
      {
        return; 
      }
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
      if (checkReturnButton()==1)
      {
        return; 
      }
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
      if (checkReturnButton()==1)
      {
        return; 
      }
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

void false_touched_note_advanced_two_keys(int touchpin1, int touchpin2){

  while (1){
    currtouched = cap.touched();

    // check only the PINS that are not touchpin1 & touchpin2
    false_touched_note_advanced_params_two_keys(DO1_CHANNEL, DO1_PIXEL, DO1_TouchPin, touchpin1, touchpin2);
    false_touched_note_advanced_params_two_keys(RE_CHANNEL, RE_PIXEL, RE_TouchPin, touchpin1, touchpin2);
    false_touched_note_advanced_params_two_keys(MI_CHANNEL, MI_PIXEL, MI_TouchPin, touchpin1, touchpin2);
    false_touched_note_advanced_params_two_keys(FA_CHANNEL, FA_PIXEL, FA_TouchPin, touchpin1, touchpin2);
    false_touched_note_advanced_params_two_keys(SOL_CHANNEL, SOL_PIXEL, SOL_TouchPin, touchpin1, touchpin2);
    false_touched_note_advanced_params_two_keys(LA_CHANNEL, LA_PIXEL, LA_TouchPin, touchpin1, touchpin2);
    false_touched_note_advanced_params_two_keys(SI_CHANNEL, SI_PIXEL, SI_TouchPin, touchpin1, touchpin2);
    false_touched_note_advanced_params_two_keys(DO2_CHANNEL, DO2_PIXEL, DO2_TouchPin, touchpin1, touchpin2);

    //if nothing is touched
    if (!currtouched){
      break;
    }

    // if a note is touched but its not any of the wrong key 
    if (currtouched && !false_touched_pin_two_keys(touchpin1, touchpin2))
    {
      break;
    }
  }
}

void false_touched_note_advanced_params_two_keys(int channel, int pixel_pin, int touch_pin, int touchpin1, int touchpin2){
  //if touched show red and sound
    if ((currtouched & _BV(touch_pin))){
      currtouched = cap.touched();

      ledcWrite(channel, 128); 
      if (touchpin1==touch_pin || touchpin2==touch_pin)
      {
        pixels.setPixelColor(pixel_pin, pixels.Color(0, 255, 0));

      }
      else {
        pixels.setPixelColor(pixel_pin, pixels.Color(255, 0, 0));
      }
      pixels.show();
    }

    // if released remove red and sound
    if (!(currtouched & _BV(touch_pin))) { //released
      ledcWrite(channel, 0); 
      if (touchpin1==touch_pin || touchpin2==touch_pin)
      {
        pixels.setPixelColor(pixel_pin, pixels.Color(255, 255, 255));
      }
      else {
        pixels.setPixelColor(pixel_pin, pixels.Color(0, 0, 0));
      }
      pixels.show();
    }
}

void false_touched_note_advanced_one_key(int touchpin1){

  while (1){
    currtouched = cap.touched();

    // check PINS : if touchpin green, it not touchpin red 
    false_touched_note_advanced_params_one_key(DO1_CHANNEL, DO1_PIXEL, DO1_TouchPin, touchpin1);
    false_touched_note_advanced_params_one_key(RE_CHANNEL, RE_PIXEL, RE_TouchPin, touchpin1);
    false_touched_note_advanced_params_one_key(MI_CHANNEL, MI_PIXEL, MI_TouchPin, touchpin1);
    false_touched_note_advanced_params_one_key(FA_CHANNEL, FA_PIXEL, FA_TouchPin, touchpin1);
    false_touched_note_advanced_params_one_key(SOL_CHANNEL, SOL_PIXEL, SOL_TouchPin, touchpin1);
    false_touched_note_advanced_params_one_key(LA_CHANNEL, LA_PIXEL, LA_TouchPin, touchpin1);
    false_touched_note_advanced_params_one_key(SI_CHANNEL, SI_PIXEL, SI_TouchPin, touchpin1);
    false_touched_note_advanced_params_one_key(DO2_CHANNEL, DO2_PIXEL, DO2_TouchPin, touchpin1);

    //if nothing is touched
    if (!currtouched){
      break;
    }

    // if a note is touched but its not any of the wrong key 
    if (currtouched && !false_touched_pin_one_key(touchpin1))
    {
      break;
    }

  }
}

void false_touched_note_advanced_params_one_key(int channel, int pixel_pin, int touch_pin, int touchpin1){
  //if touched show red and sound
  if ((currtouched & _BV(touch_pin))){
    currtouched = cap.touched();

    ledcWrite(channel, 128); 
    if (touchpin1==touch_pin)
    {
      pixels.setPixelColor(pixel_pin, pixels.Color(0, 255, 0));

    }
    else {
      pixels.setPixelColor(pixel_pin, pixels.Color(255, 0, 0));
    }      
    pixels.show();
  }

  // if not touched remove red/green and sound
  if (!(currtouched & _BV(touch_pin))) { //released
    ledcWrite(channel, 0); 
    if (touchpin1==touch_pin)
    {
      pixels.setPixelColor(pixel_pin, pixels.Color(255, 255, 255));
    }
    else {
      pixels.setPixelColor(pixel_pin, pixels.Color(0, 0, 0));
    }
    pixels.show();
  }
}

// RECORD MODE 
void record_mode(){
  unsigned long start_note = millis();
  int first_touched=1;
  String note_played = "DO1";
  unsigned long time_played = millis();
  delay(500);
  buttonSelectState = digitalRead(buttonSelectPin);

  while (buttonSelectState!=HIGH){
    currtouched = cap.touched();
    buttonSelectState = digitalRead(buttonSelectPin);

    if (checkReturnButton()==1)
    {
      return;
    }

    record_mode_note("DO1",  DO1_CHANNEL, DO1_PIXEL, DO1_TouchPin); 
    record_mode_note("RE",  RE_CHANNEL, RE_PIXEL, RE_TouchPin); 
    record_mode_note("MI",  MI_CHANNEL, MI_PIXEL, MI_TouchPin); 
    record_mode_note("FA",  FA_CHANNEL, FA_PIXEL, FA_TouchPin); 
    record_mode_note("SOL",  SOL_CHANNEL, SOL_PIXEL, SOL_TouchPin); 
    record_mode_note("LA",  LA_CHANNEL, LA_PIXEL, LA_TouchPin); 
    record_mode_note("SI",  SI_CHANNEL, SI_PIXEL, SI_TouchPin); 
    record_mode_note("DO2",  DO2_CHANNEL, DO2_PIXEL, DO2_TouchPin); 

  }
  
  if (returnValue==1)
  {
    return;
  }
  
  if (count_TOADD>0){
    String name_TOADD = "SongNum" + String(NumSongsTotal);
    writetoGoogleSheets(name_TOADD, count_TOADD, notes_TOADD, duration_note_TOADD, num_notes_TOADD);
    SongNames[NumSongsTotal] = name_TOADD;
    NumSongsTotal+=1;
  }
}

void record_mode_note(String note_played, int channel, int pixel_pin, int touch_pin){
  currtouched = cap.touched();
  unsigned long start_note = millis();

  while ((currtouched & _BV(touch_pin))){
    currtouched = cap.touched();

    ledcWrite(channel, 128); 
    pixels.setPixelColor(pixel_pin, pixels.Color(255, 255, 255));
    pixels.show();

    if (!(currtouched & _BV(touch_pin))) { //released
      ledcWrite(channel, 0); 
      pixels.setPixelColor(pixel_pin, pixels.Color(0, 0, 0));
      pixels.show();

      unsigned long time_played=millis()-start_note;

      // Adding the infos to the "arrays" (strings to googlesheet)
      notes_TOADD += note_played;
      notes_TOADD += ",";

      duration_note_TOADD += String(time_played); 
      duration_note_TOADD += ","; 

      num_notes_TOADD += "1,";
      count_TOADD+=1;
      break;
    }
  }
}

// Screen functions 

void menu_wifi(){

  show_menu_wifi();

  while(1){
    buttonUpState = digitalRead(buttonUpPin);
    buttonDownState = digitalRead(buttonDownPin);
    buttonSelectState = digitalRead(buttonSelectPin);

    if (buttonUpState == HIGH){
      wifi_mode=0;
      break;
    }

    if (buttonDownState == HIGH){
      wifi_mode=1;
      break;
    }

    if (buttonSelectState== HIGH)
    {

      if (wifi_mode == 0)
      {
        display.clearDisplay();
        display.setTextSize(1);                     // Normal 1:1 pixel scale
        display.setTextColor(SSD1306_WHITE); 
        display.setCursor(0,0); 
        display.println("FREE PLAYING");
        display.display();
        free_playing();
        break;
      }

      if (wifi_mode == 1)
      {
        display.clearDisplay();
        display.setTextSize(1);                     // Normal 1:1 pixel scale
        display.setTextColor(SSD1306_WHITE); 
        display.setCursor(0,0); 
        if (wifi_connected==0){
          display.println(F("Waiting for the wifi..."));
          display.display();

          // SET UP FOR GOOGLE SHEET 
          WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

          while (WiFi.status() != WL_CONNECTED) {
            delay(200);
          }

          String column = readColumnFromGoogleSheets();
          Serial.println("songs are: ");
          Serial.println(column);
          split_song_names(column);
          wifi_connected=1;
        }
        returnValue=0;
        call_menu_mode();
        break;
      }
    }
  }
}

void show_menu_wifi(){
  display.clearDisplay();
  display.setCursor(0,0);             // Start at top-left corner

  display.setTextSize(1);             // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);        // Draw white text
  display.println(F("Choose Wifi mode: "));
  
  if (wifi_mode==0){
    display.setTextColor(SSD1306_BLACK, SSD1306_WHITE); // Draw 'inverse' text
    display.println(F("No Wifi mode"));

    display.setTextColor(SSD1306_WHITE);        // Draw white text
    display.println(F("Wifi mode "));
  }

   if (wifi_mode==1){
    display.println(F("No Wifi mode"));

    display.setTextColor(SSD1306_BLACK, SSD1306_WHITE); // Draw 'inverse' text
    display.println(F("Wifi mode "));
  }

  display.display();
  delay(250);
}


void call_menu_mode(){
  while(returnValue==0){
    menu_mode();
  }
  wifi_mode = 0;
  mode=1;
  songIndex=1;
  songNum=0;
  returnValue=0;
}

void menu_mode(){
  
  //show the menu with the cursor (inverse)
  show_menu_mode(mode);
  
  while(1){
    buttonUpState = digitalRead(buttonUpPin);
    buttonDownState = digitalRead(buttonDownPin);
    buttonSelectState = digitalRead(buttonSelectPin);
    buttonReturnState = digitalRead(buttonReturnPin);
    // UP 
    if (buttonUpState == HIGH) {
      if (mode!=1){
        mode= mode-1;
      }
      break;
    }

    // DOWN
    if (buttonDownState == HIGH) {
      if(mode!=6){
        mode=mode+1;
      }
      break;
    }

    // SELECT
    if (buttonSelectState == HIGH) {
      //mode number (1,2,3,4,5,6) is in "mode" variable
      Serial.print("Selected mode ");
      buttonReturnState = digitalRead(buttonReturnPin);

      Serial.println(mode);
      if (mode==FREE_MODE){
        display.clearDisplay();
        display.setCursor(0,0); 
        display.setTextSize(1);             // Normal 1:1 pixel scale
        display.setTextColor(SSD1306_WHITE);
        display.println(F("Let's play!!"));
        display.display();
        start_game();
        returnValue=0;
        mode=1;
      }
      else if (mode==RECORD_MODE){
        start_game();
        returnValue=0;
        mode=1;
      }
      else{
        call_menu_song();
      }
      break;
    }

    if (buttonReturnState == HIGH)
    {
      returnValue=1;
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
    display.println(F("Statistics"));

  }

  if (mode==MEDIUM_MODE){
    display.println(F("Easy"));

    display.setTextColor(SSD1306_BLACK, SSD1306_WHITE); // Draw 'inverse' text
    display.println(F("Medium"));
    
    display.setTextColor(SSD1306_WHITE);        // Draw white text
    display.println(F("Advanced"));
    display.println(F("Free"));
    display.println(F("Record"));
    display.println(F("Statistics"));
  }

  if(mode==ADVANCED_MODE){
    display.println(F("Easy"));
    display.println(F("Medium"));
    
    display.setTextColor(SSD1306_BLACK, SSD1306_WHITE); // Draw 'inverse' text
    display.println(F("Advanced")); 
    display.setTextColor(SSD1306_WHITE);

    display.println(F("Free"));       
    display.println(F("Record"));
    display.println(F("Statistics"));
  }

  if(mode==FREE_MODE){
    display.println(F("Easy"));
    display.println(F("Medium"));
    display.println(F("Advanced"));
    
    display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
    display.println(F("Free")); 
    display.setTextColor(SSD1306_WHITE);

    display.println(F("Record"));
    display.println(F("Statistics"));

  }

  if(mode==RECORD_MODE){
    display.println(F("Easy"));
    display.println(F("Medium"));
    display.println(F("Advanced"));
    display.println(F("Free")); 
    
    display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
    display.println(F("Record"));
    display.setTextColor(SSD1306_WHITE);
    display.println(F("Statistics"));
  }

  if(mode==STATS_MODE){
    display.println(F("Easy"));
    display.println(F("Medium"));
    display.println(F("Advanced"));
    display.println(F("Free")); 
    display.println(F("Record"));
    display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
    display.println(F("Statistics"));
  }

  display.display();
  delay(250);
}

void call_menu_song(){
  while(returnValue==0){
    menu_song();
  }
  mode=1;
  songIndex=1;
  songNum=0;
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
          songIndex=5;
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
          songIndex=1;
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
      if (mode==STATS_MODE)
      {
        display.println(F("Loading the stats"));
        display.display();
        String payload = readStatsFromGoogleSheets(songNum);
        Serial.println(payload);
        if (payload!="/"){
          split_stats(payload); //keep the music array (notes,dur,num) in global arrays
       
          display.clearDisplay();
          display.setCursor(0,0); 
          display.setTextSize(1);                     // Normal 1:1 pixel scale
          display.setTextColor(SSD1306_WHITE);
          display.println(F("  Previous stats"));
          display.println(F(""));
          display.print(F("Num errors: "));
          display.println(error_display);
          display.println(F("Average response "));
          display.print(F("time: "));
          display.print(time_display);
          display.println(F(" secs "));
          display.display();
        }

        else {
          display.clearDisplay();
          display.setCursor(0,0); 
          display.setTextSize(1);                     // Normal 1:1 pixel scale
          display.setTextColor(SSD1306_WHITE);
          display.print(F("You have no statistics "));
          display.println(F("for this song"));
          display.display();
          delay(3000);
          songNum=0;
          songIndex=1;
          return;
        }

        while (1){
          buttonReturnState = digitalRead(buttonReturnPin);
          if (buttonReturnState==HIGH)
          {
            songNum=0;
            songIndex=1;
            delay(250);
            return;
          }
        }
      }
      
      display.println(F("Loading the song...."));
      display.display();
      
      String payload = readFromGoogleSheets(songNum);
      if (payload!=""){
        split_arrays(payload); //keep the music array (notes,dur,num) in global arrays
      } 

      // song loaded lets play
      display.clearDisplay();
      display.setCursor(0,0); 
      display.println(F("Let's play!!"));
      display.println(F("To skip the song demo press SELECT"));
      display.display();

      start_game();
      
      // TOASK
      //once it's over or clicked Return --> show the song menu again with same mode
      returnValue=0; 
      songNum=0; //cursor all the way up? TOASK
      songIndex=1;
      return;
    }

    if (buttonReturnState==HIGH) {
      songNum=0;
      songIndex=1;
      mode=1;
      returnValue=1; // return to the menu of mode
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

void split_stats(String payload){
  
  char* token; // Pointer to hold each token

  // Convert the payload string to a character array
  char payloadArray[payload.length() + 1];
  payload.toCharArray(payloadArray, payload.length() + 1);

  // Split the payload by '/'
  token = strtok(payloadArray, "/");
  error_display = atoi(token); // name of song

  token = strtok(NULL, "/");
  time_display = strtod(token, NULL); // count number
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
}

void writeStatstoGoogleSheets(int numSong, int errors, double time){
  if (WiFi.status() == WL_CONNECTED) {

      String url = "https://script.google.com/macros/s/" + GOOGLE_SCRIPT_ID+"/exec" ;
      url += "?action=write";
      url += "&stat=1";
      url += "&song=" + String(numSong);
      url += "&errors=" + String(errors);
      url += "&time=" + String(time);

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

String readStatsFromGoogleSheets(int song){
  if (WiFi.status() == WL_CONNECTED) {

    String url = "https://script.google.com/macros/s/" + GOOGLE_SCRIPT_ID+"/exec" ;
    url += "?action=read";
    url += "&error=1";
    url += "&row=" + String(song);

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

int checkReturnButton()
{
  buttonReturnState = digitalRead(buttonReturnPin);
  if (buttonReturnState==HIGH) {
    Serial.println("");
    Serial.println("Return has been clicked");
    returnValue= 1;
    turnOffAllSounds();
    pixels.clear();
    pixels.show();
    errors_count=0;
    note_index=0;
    num_note_index=0;
    return 1;
  }

  returnValue=0;
  return 0;
}

void turnOffAllSounds()
{
  ledcWrite(DO1_CHANNEL, 0);
  ledcWrite(RE_CHANNEL, 0);
  ledcWrite(MI_CHANNEL, 0);
  ledcWrite(FA_CHANNEL, 0);
  ledcWrite(SOL_CHANNEL, 0);
  ledcWrite(LA_CHANNEL, 0);
  ledcWrite(SI_CHANNEL, 0);
  ledcWrite(DO2_CHANNEL, 0);
}

// SET UP FUNCTIONS //
void welcome_sound_and_led(){
  display.clearDisplay();
  display.setTextSize(2);                     
  display.setTextColor(SSD1306_WHITE); 
  display.setCursor(0,0); 
  display.println(F("WELCOME TO"));
  display.println(F("pIanOT"));
  display.display();

  int count_rainbow=0;
  String note = "None";
  note_index=0;
  for(long firstPixelHue = 0; firstPixelHue < 5*65536; firstPixelHue += 256) {

    pixels.rainbow(firstPixelHue);
    // Above line is equivalent to: strip.rainbow(firstPixelHue, 1, 255, 255, true);
    pixels.show(); // Update strip with new contents
    if (count_rainbow>=49){
      turnOff_son_note(note);
      delay(10);
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
  delay(2000);

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

