#include <elapsedMillis.h>

#include "LedControl.h"

#include <Ultrasonic.h>

/*
 * Pop-a-Shop Arcade Game
 * 
 * Ultrasonic sensor will detect a basketball goal.
 * An RGB LED will shine green if goal detected and will also animate if top score is reached.
 * A goal can only be scored every 500ms, this is easily adjustable.
 * 
 * [Score Board]
 * 
 *    Top Score    Current Score   
 *    Clock        Bonus          RGB LED
 * 
 * 
 * This code was configured for an Arduino Nano. 
 * Please adjust the pins to fit your board's requirements.
 * 
 * Written By Cory Guynn 
 * Internet of LEGO 2017
 * www.InternetOfLEGO.com
 * MIT License
 */

// User Defined Variables
int countdown = 30*60*1000; // 30 seconds
const long goalDelay = 500; // Adujust the Goal LED Delay in ms
int bonus = 30; // 30 points required for bonus time (clock reset) 


// Define Hardware
/*
Seven Segment Display
 Pin 8-4 is connected to the DataIn 
 Pin 9-5 is connected to the CLK 
 Pin 7-6 is connected to LOAD 
 */
LedControl lc=LedControl(8,9,7,1);
LedControl lc2=LedControl(4,5,6,1);

// Start Button
int startButtonPin = 12;        // PIN 12

// Distance Sensor
Ultrasonic ultrasonic(10, 13);  // PIN 10, 13

// RGB LED 
const int ledRedPin = 2;          // PIN 2
const int ledGreenPin = 3;          // PIN 3
const int ledBluePin = 11;         // PIN 11


// Segement selector (helper function used to create sections)
int topRightDisplay = 0;
int topLeftDisplay = 1;
int btmRightDisplay = 2;
int btmLeftDisplay = 3;

// Define initial LED states
int ledRedState = HIGH; 
int ledGreenState = LOW;
int ledBlueState = HIGH;

// Global Variables
int score = 0;
int highScore = 0;
int remaining = 0;
int topScore = 0;
unsigned long previousMillis = 0; // Timer reference - score
elapsedMillis sinceStart; // start of game
elapsedMillis statsMillis; // main loop 

// Number of overtime bonus sessions
int bonusCount = 0;
int lastScore = 0;

// State of game
boolean gameActive = false;

// Detect if ball has had a chance to clear rim
boolean deDuplicate = true;



// Helper Functions

void goal() {
  score += 2;
}

// Set Game Defaults
void resetGame(){

    // reset game variables
    score = 0;
    lastScore = 0;
    sinceStart = 0;

    // reset seven segment displays
    lc.clearDisplay(0);
    lc2.clearDisplay(0);
    displays(topLeftDisplay,topScore);
    displays(topRightDisplay,score);

    // reset RGB LED
    digitalWrite(ledGreenPin, false);
    digitalWrite(ledRedPin, false);
    digitalWrite(ledBluePin, true);
    
    // Spell "P0P" btmRightDislay
    lc2.setChar(0,7,'P',false);
    lc2.setChar(0,6,'0',false);
    lc2.setChar(0,5,'P',false);
    lc2.setChar(0,4,false,true);
    lc2.setChar(0,3,'5',false);
    lc2.setChar(0,2,'h',false);
    lc2.setChar(0,1,0,false);
    lc2.setChar(0,0,'P',true);
}


// Print stats to console
void stats(){
    Serial.print(" | Top Score");
    Serial.println(topScore);
    Serial.print(" | Score");
    Serial.println(score);
    Serial.print(" | Time Remaining: ");
    Serial.println(remaining);
}

// Formats and prints to Seven Segment displays consitently
void displays(int section,int number){
  uint8_t ones,tens,hundreds,thousands;
  thousands=number/1000;
  hundreds=number%1000/100;
  tens=number%100/10;
  ones=number%10;
  // set digits on 7 segment. Some digits are disabled (' ') for aesthetic reasons
  if(section == 0){                   // Top Right
    lc.setDigit(0,0,ones,false);
    lc.setDigit(0,1,tens,false);
    lc.setDigit(0,2,hundreds,false);
    //lc.setDigit(0,3,thousands,false); 
    lc.setChar(0,3,' ',false);
  }else if(section == 1){             // Top Left
    lc.setChar(0,4,' ',false);
    lc.setDigit(0,5,ones,false);
    lc.setDigit(0,6,tens,false);
    lc.setDigit(0,7,hundreds,false);
    //lc.setDigit(0,7,thousands,false);  
  }else if(section == 2){             // Bottom Right
    lc2.setDigit(0,0,ones,false);
    lc2.setDigit(0,1,tens,false);
    //lc2.setDigit(0,2,hundreds,false);
    //lc2.setDigit(0,3,thousands,false);
    lc2.setChar(0,2,' ',false);
    lc2.setChar(0,3,' ',false);
  } else if(section == 3){             // Bottom Left
    lc2.setDigit(0,4,ones,false);
    lc2.setDigit(0,5,tens,false);
    //lc2.setDigit(0,6,false,false);
    //lc2.setDigit(0,7,false,false);
    lc2.setChar(0,6,' ',false);
    lc2.setChar(0,7,' ',false);
  }
  
}

// Top Score Celebration Routing
void finishTopScore(){
    Serial.print("NEW TOP SCORE !");
    displays(topLeftDisplay,topScore);
    int y = false;
    for (int x = 0; x < 1024; x = x + 20){
      for (int y = 100; y < 1024; y++){
        analogWrite(ledGreenPin, x);
        analogWrite(ledRedPin, y);
      }
       // Spell "P0P"
      lc2.setChar(0,3,' ',false);
      lc2.setChar(0,2,'P',false);
      lc2.setChar(0,1,0,false);
      lc2.setChar(0,0,'P',true);
      delay(50);
      lc2.clearDisplay(0);       
    }
}

// display final messages and LED routines
void finishLowScore(){
  for (int x = 0; x <6; x++){
      lc2.setChar(0,7,false,false);
      lc2.setChar(0,6,false,false);
      lc2.setChar(0,5,'-',false);
      lc2.setChar(0,4,'-',false);
      displays(topRightDisplay,score);
      digitalWrite(ledRedPin, true);
      delay(500);
      displays(topRightDisplay,0);
      lc2.setChar(0,7,false,false);
      lc2.setChar(0,6,false,false);
      lc2.setChar(0,5,'0',false);
      lc2.setChar(0,4,'0',false);
      digitalWrite(ledRedPin, false);
      digitalWrite(ledGreenPin, false);
      digitalWrite(ledBluePin, false);
      delay(500);
    }
  delay(50);
  lc2.clearDisplay(0);  
}

elapsedMillis standbyRoutineMillis;
boolean blueCountUp = true;
void standbyRoutine(){

  if (standbyRoutineMillis > 50){

    // Fade Blue in and out
    if(blueCountUp){
      if (ledBlueState < 800){
        analogWrite(ledBluePin, ledBlueState++);    
      }else{
        blueCountUp = false; 
      }
    }else{
        if (ledBlueState > 0){
          analogWrite(ledBluePin, ledBlueState--);    
        }else{
          blueCountUp = true;
        }
    }

    // reset timer
    standbyRoutineMillis = 0;
  }
}


void runGame(){
  // Determine remaining game time
  remaining = (countdown - sinceStart) / 1000;

  // Display Countdown Clock
  displays(btmLeftDisplay,remaining);
  

  // Check for ball, with delay to avoid duplicates
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= goalDelay) {   
    ledRedState = HIGH;
    ledGreenState = LOW;
    ledBlueState = HIGH;
    digitalWrite(ledGreenPin, ledGreenState);
    digitalWrite(ledRedPin, ledRedState);
    digitalWrite(ledBluePin, ledBlueState);
    Serial.print("distance ");
    Serial.println(ultrasonic.distanceRead());

    // Detect Ball
    if (ultrasonic.distanceRead() < 10 && deDuplicate){
      deDuplicate = false;  
      // save the last time a goal happened
      previousMillis = currentMillis;
      Serial.print("Ball Detected!");
      score += 2;
      Serial.print("Score: ");
      Serial.println(score);
      ledGreenState = HIGH;
      ledRedState = LOW;
      ledBlueState = LOW;

      // set the LED with the ledState of the variable:
      digitalWrite(ledGreenPin, ledGreenState);
      digitalWrite(ledRedPin, ledRedState);
      digitalWrite(ledBluePin, ledBlueState);
    }else{
      deDuplicate = true;
    }

  // End game loop
  if(remaining < 1){
    // Check if bonus time is added
    if(score-lastScore >= bonus){
      bonusCount++;
      displays(btmRightDisplay,bonusCount);
      lastScore = score;   
      // add extended bonus time by resetting start counter
      sinceStart = 0;
    }else{
          // End Game
          if (score > topScore){
            topScore = score;
            finishTopScore();      
          } else{
            finishLowScore();     
          }
          resetGame(); 
          gameActive = false;                      
    }
  } 
  // write score to 7 Segment
  displays(topRightDisplay,score);  
  }

}





// Run ONCE
void setup() {
  Serial.begin(57600);
  pinMode(ledRedPin, OUTPUT);
  pinMode(ledGreenPin, OUTPUT);
  pinMode(ledBluePin, OUTPUT);
  pinMode(startButtonPin, INPUT);

  // LED 7 Segment logic
  /*
   The MAX72XX is in power-saving mode on startup,
   we have to do a wakeup call
   */
  lc.shutdown(0,false);
  lc2.shutdown(0,false);
  /* Set the brightness to a medium values */
  lc.setIntensity(0,8);
  lc2.setIntensity(0,8);
  /* and clear the display */
  lc.clearDisplay(0);
  lc2.clearDisplay(0);

  resetGame();
}


// MAIN LOOP
void loop() {

  // Start Game Button
  if(digitalRead(startButtonPin)){
    Serial.print("Game Started");
    // reset score
    resetGame();

    // Clear last messages
    lc2.clearDisplay(0);
    // set game state
    gameActive = true;
  }

  // Run Game
  if (gameActive){
    runGame();

    if (statsMillis > 500) {    
      statsMillis = 0;         // reset the counter to 0 so the counting starts over...
      stats();
    }
  }else{
    standbyRoutine();
  }
}
