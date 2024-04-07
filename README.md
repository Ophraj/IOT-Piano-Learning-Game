WELCOME TO pIanoT - our IOT piano :))

Our piano is composed of an ESP32 card, 8 keys (D0,RE,MI,FA,SOL,LA,SI,DO), a 8 leds band, a speaker, and a screen.
It is made for debutant players who seek to learn playing the piano easily and for more advanced players that want a portable, nice piano for training. 

The piano has 6 different modes:
- A free mode (that can be activated in No Wifi Mode): You can play every notes, every chords with no limit! 
- Three level modes that allow you to learn new songs: The led corresponding to the next key lights up so you can play it and play a beautiful song 
    * The easy mode is for debutants: When making an error, it lets the possibilty of playing the note again so you can be a champion! If there is a chord you have a second to touch the keys together and to release them. 
    * The medium mode is more challeging: When making an error, the sound of the wrong key is made so you have to be more careful. If there is a chord you only have 1/2 a second to touch the keys together and to release them.
    * The hard mode is for advanced players: When making an error, you can't try again and the wrong key is played! You also have to play the key in the rythm: if you take too long to press the key OR you press it for too long, it's an error and you need to play the next key!! If there is a chord you only you need to touch and release all of the keys together.
- A record mode that allows you to record in live, and to upload to the database of songs ANY song you want! You can record your favorites songs and learn them easily! 
- A statistic mode: You can see your last statistics for every song - how many errors you've made and what your average response time - so you can have an objective for the next learning session! 

The piano comes with a googleSheet where you can write new songs or see the notes of the songs already written! This is the google sheet link: 
https://docs.google.com/spreadsheets/d/1pZ8jRfhP_1pBa6_d6HEmbXH9C05siOqeHuzbPHwF22w/edit#gid=0
and this is the deployment key: AKfycbxUdnas0tGWlM5LPutQGvZZ1Wq5SBKchdfWq7itF9dPMG4jmHmJN7R0-cOLQ7Sc15U5DQ

In the code part we have: 

The main.io -  The arduino code 
Librairies in use: 
- Adafruit_MPR121 library for touch sensor
- Adrafruit_Neopixels for leds 
- Html and Wifi for http requests to the google sheet 
- Spi and Adafruit_GFX and Adafruit_SSD1306 for the screen

The googleSheet script that handled http requests to the googleSheet. 

ENJOY :))))
