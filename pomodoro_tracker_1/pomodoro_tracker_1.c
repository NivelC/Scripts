/*
Pomodoro Tracker 1.0 by Damian M.G.(IgorJorobus)
================================================

Materiales físicos utilizados
=============================
* 3 leds 5mm verdes
* 2 leds 5mm azules
* 1 led 5mm rojo
* 8 resistencias 100 ohm
* 1 buzzer
* 1 switch
* Arduino Uno
* Cables 

Para ensamblar
==============
* Soldador
* Aislador(silicona)
* Contenedor

Pines utilizados
================
* 2 = OUTPUT, led verde 0
* 3 = OUTPUT, led verde 1
* 4 = OUTPUT, led verde 2
* 5 = OUTPUT, led azul 0
* 6 = OUTPUT, led azul 1
* 7 = OUTPUT, led rojo 0
* 8 = INPUT, switch
* 12 = OUTPUT, buzzer

Cambios vs. versión 0.1
=======================
* Se construye software con Ruby en donde ocurre mucho de lo que hacía el Arduino mismo.
* Se utiliza fuertemente la lectura del puerto serie.
* El Arduino pasa a ser bastante mas pasivo. Se elimina el botón de inicio/break.
* Solamente se deja el switch que define si el Arduino está activo, leyendo, si no está activo todos los leds se apagan y queda en hang.
* El sistema no tiene que hacer flush del puerto serie por si mismo, salvo cuando se censa el switch "on".
* El sistema puede leer un evento o un estado. El estado es uno de pomodoro running, break running o stopped. Los eventos son cualquiera de los juegos de luces y sonidos.
*/

// include aliases of sounds as frecuencies
#include "pitches.h"

/* Function prototypes */
void checkSwitch(void);
void makeSystemOnLightGame(void);
void soundSadBuzzer(void);
void soundHappyBuzzer(void);
void makePomodoroFinishedLightGame(void);
void makePomodoroN12FinishedLightGame(void);
void makePomodoroN22FinishedLightGame(void);
void makeBreakFinishedLightGame(void);
void resetEverything(void);
void inspectSerialPortInput(void);
boolean isCodeAnEvent(String);
void showPomodoroRunning(long secondsSincePomodoroStart);
void showBreakRunning(long pomodorosCompleted);
void showSystemStopped(void);
/* Deprecated function prototypes
void checkButton(void);
void startPomodoro(void);
void cancelCurrentPomodoro(void);
void checkCurrentPomodoro(void);
void startBreak(void);
void cancelBreak(void);
void checkCurrentBreak(void);
void finishBreak(void); 
*/

/* Global variables */
int switchInitialPosition = 0;
// system is on when the switch is not in the initial position
bool systemOn = false;

/* Arduino functions*/
// This runs once.
void setup() {
  // set input/output pings
  pinMode(2, OUTPUT); 
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(7, OUTPUT);
  pinMode(8, INPUT);
  pinMode(12, OUTPUT);
  // system never start "on", doesn't matter in which position is the switch, its turning-on depends on the contrary state in which it begins
  switchInitialPosition = digitalRead(8);
  // open thy serial port
  Serial.begin(9600);
}

// This can run up to 16000 times per second, but most of the time runs twice per second.
void loop() {
  // check if system should be on
  checkSwitch();
  // main process
  if(systemOn) {
    // inspect serial port looking for input
    inspectSerialPortInput();
  } else {
    // shut down everything
    resetEverything();
  }
}

/* Helper functions */
// Check if the switch has changed its original state.
void checkSwitch() {
  // see if the switch is on
  if(systemOn) {
    if(digitalRead(8) == switchInitialPosition) {
      // the initial position is reached again
      systemOn = false;
    }
  } else {
    if(digitalRead(8) != switchInitialPosition) {
      systemOn = true;
      // execute light game on on
      makeSystemOnLightGame();
      // flush the serial port
      Serial.readString();
    }
  }
}

// Triggered when system is turned on.
void makeSystemOnLightGame() {
  digitalWrite(7, HIGH);
  delay(500);
  digitalWrite(7, LOW);
  digitalWrite(6, HIGH);
  delay(500);
  digitalWrite(6, LOW);
  digitalWrite(5, HIGH);
  delay(500);
  digitalWrite(5, LOW);
  digitalWrite(4, HIGH);
  delay(500);
  digitalWrite(4, LOW);
  digitalWrite(3, HIGH);
  delay(500);
  digitalWrite(3, LOW);
  digitalWrite(2, HIGH);
  delay(500);
  digitalWrite(2, LOW);
  delay(500);
  digitalWrite(7, HIGH);
}

// Checks if Serial port has any data written on it. If it does, read it, and interpret it.
void inspectSerialPortInput() {
  // check if there's at least one char to read
  if(Serial.available()) {
    // there is some data available to read
    String code = Serial.readStringUntil('-');
    // detect if the code talks about an event or state
    if(isCodeAnEvent(code)) {
      // event, find out which
      if(code == "MSOLG") {
        makeSystemOnLightGame();
      } else if(code == "MPFLG") {
        makePomodoroFinishedLightGame();
      } else if(code == "MPN12FLG") {
        makePomodoroN12FinishedLightGame();
      } else if(code == "MPN22FLG") {
        makePomodoroN22FinishedLightGame();
      } else if(code == "MBFLG") {
        makeBreakFinishedLightGame();
      } else if(code == "SSB") {
        soundSadBuzzer();
      } else if(code == "SHB") {
        soundHappyBuzzer();
      } 
    else
      // state, kind of "16R0288", this is pomodoros completed, actual state, seconds since beggining of actual phase, first thing of interest is state taking part now
      switch(code[2]) {
        case 'R':
          // pomodoro running, pass the seconds since the start of it
          showPomodoroRunning(code.substring(3).toInt());
          break;
        case 'B':
          // break running, pass the amount of pomodoros completed
          showBreakRunning(code.substring(0, 2).toInt());
          break;
        case 'S':
          // stopped
          showSystemStopped();
          break;    
      }
    }
  }
}

// The code will have 7 characters length if it's a state, otherwise will be an event.
boolean isCodeAnEvent(String code) {
  int codeLength = code.length();
  if(codeLength != 7) {
    return(true);
  } else {
    return(false);
  }
}

// Shows to the user leds that represent a pomodoro running.
void showPomodoroRunning(long secondsSincePomodoroStart) {
  // depending on how many seconds has passed since the start of the pomodoro, 1, 2 or 3 green leds will be on
  if(secondsSincePomodoroStart > 1000) {
    // 3 leds on
    digitalWrite(2, HIGH);
    digitalWrite(3, HIGH);
    digitalWrite(4, HIGH);
    digitalWrite(5, LOW);
    digitalWrite(6, LOW);
    digitalWrite(7, LOW);
  } else if(secondsSincePomodoroStart > 500) {
    // 2 leds on
    digitalWrite(2, HIGH);
    digitalWrite(3, HIGH);
    digitalWrite(4, LOW);
    digitalWrite(5, LOW);
    digitalWrite(6, LOW);
    digitalWrite(7, LOW);
  } else {
    // 1 led on
    digitalWrite(2, HIGH);
    digitalWrite(3, LOW);
    digitalWrite(4, LOW);
    digitalWrite(5, LOW);
    digitalWrite(6, LOW);
    digitalWrite(7, LOW);
  }
}

// Shows to the user leds that represent a break running.
void showBreakRunning(long pomodorosCompleted) {
  // check if on long or short break
  if((pomodorosCompleted % 4) == 0) {
    // long
    digitalWrite(2, LOW);
    digitalWrite(3, LOW);
    digitalWrite(4, LOW);
    digitalWrite(5, HIGH);
    digitalWrite(6, HIGH);
    digitalWrite(7, LOW);
  } else {
    // short
    digitalWrite(2, LOW);
    digitalWrite(3, LOW);
    digitalWrite(4, LOW);
    digitalWrite(5, HIGH);
    digitalWrite(6, LOW);
    digitalWrite(7, LOW);
  }
}

// Shows to the user leds that represent the system stopped.
void showSystemStopped() {
  digitalWrite(2, LOW);
  digitalWrite(3, LOW);
  digitalWrite(4, LOW);
  digitalWrite(5, LOW);
  digitalWrite(6, LOW);
  digitalWrite(7, HIGH);
}

// :( Why did you interrupted that pomodoro? Bio meaby? Is ok...
void soundSadBuzzer() {
  // setup of notes
  int melody[] = {
    NOTE_E3, NOTE_D3, NOTE_C3
   };
  int noteDurations[] = {
    8, 8, 2
  };
  // iterate over the notes of the melody:
  for (int thisNote = 0; thisNote < 3; thisNote++) {
    // to calculate the note duration, take one second divided by the note type i.e.: quarter note = 1000 / 4, eighth note = 1000/8, etc.
    int noteDuration = 1000 / noteDurations[thisNote];
    tone(12, melody[thisNote], noteDuration);
    // to distinguish the notes, set a minimum time between them; the note's duration + 30% seems to work well
    int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);
    // stop the tone playing:
    noTone(12);
  }
}

// Yes! Completed!
void soundHappyBuzzer() {
  // setup of notes
  int melody[] = {
    NOTE_C6, NOTE_D6, NOTE_E6
   };
  int noteDurations[] = {
    8, 8, 8
  };
  // iterate over the notes of the melody:
  for (int thisNote = 0; thisNote < 3; thisNote++) {
    // to calculate the note duration, take one second divided by the note type i.e.: quarter note = 1000 / 4, eighth note = 1000/8, etc.
    int noteDuration = 1000 / noteDurations[thisNote];
    tone(12, melody[thisNote], noteDuration);
    // to distinguish the notes, set a minimum time between them; the note's duration + 30% seems to work well
    int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);
    // stop the tone playing:
    noTone(12);
  }
}

// Executes the light game triggered by the finish of a pomodoro.
void makePomodoroFinishedLightGame() {
  // clean leds just in case
  resetEverything();
  // do the game
  for(int counter = 0; counter < 5; counter++) {
    // turn off all green leds
    digitalWrite(2, LOW);
    digitalWrite(3, LOW);
    digitalWrite(4, LOW);
    // rest
    delay(1000);
    // turn on all green leds
    digitalWrite(2, HIGH);
    digitalWrite(3, HIGH);
    digitalWrite(4, HIGH);
    // rest
    delay(1000);
  }
  // and off every green leds
  digitalWrite(2, LOW);
  digitalWrite(3, LOW);
  digitalWrite(4, LOW);
  delay(1000);
}

// Called when pomodoro number 12 is reached.
void makePomodoroN12FinishedLightGame() {
  // clean leds just in case
  resetEverything();
  // do the game
  for(int counter = 0; counter < 4; counter++) {
    digitalWrite(2, HIGH);
    digitalWrite(7, HIGH);
    delay(500);
    digitalWrite(3, HIGH);
    digitalWrite(6, HIGH);
    delay(500);
    digitalWrite(4, HIGH);
    digitalWrite(5, HIGH);
    delay(500);
    digitalWrite(4, LOW);
    digitalWrite(5, LOW);
    delay(500);
    digitalWrite(3, LOW);
    digitalWrite(6, LOW);
    delay(500);
    digitalWrite(2, LOW);
    digitalWrite(7, LOW);
    delay(500);
  }
  delay(500);
}

// Called when pomodoro number 22 is reached.
void makePomodoroN22FinishedLightGame() {
  // clean leds just in case
  resetEverything();
  // do the game
  int led;
  bool currentState = 1;
  digitalWrite(2, HIGH);
  delay(100);
  digitalWrite(2, LOW);
  for(int counter = 0; counter < 20; counter++) {
    for(led = 3; led < 8; led++) {
      digitalWrite(led, HIGH);
      delay(100);
      digitalWrite(led, LOW);
    }
    for(led = 6; led > 1; led--) {
      digitalWrite(led, HIGH);
      delay(100);
      digitalWrite(led, LOW);
    }
  }
}

// Triggered when break time is done.
void makeBreakFinishedLightGame() {
  // clean leds just in case
  resetEverything();
  // do the game
  for(int counter = 0; counter < 2; counter++) {
    digitalWrite(5, LOW);
    digitalWrite(6, LOW);
    delay(1000);
    digitalWrite(5, HIGH);
    digitalWrite(6, HIGH);
    delay(1000);
  }
  // clean leds
  resetEverything();
}

// Called when system has been turned off. Resets everything to its pristine status.
void resetEverything() {
  digitalWrite(2, LOW);
  digitalWrite(3, LOW);
  digitalWrite(4, LOW);
  digitalWrite(5, LOW);
  digitalWrite(6, LOW);
  digitalWrite(7, LOW);
}
