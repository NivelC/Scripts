/* 
Pomodoro Tracker 0.1 by Damián M.G.(IgorJorobus)
================================================

Materiales físicos utilizados
=============================
* 3 leds 5mm verdes
* 2 leds 5mm azules
* 1 led 5mm rojo
* 7 resistencias 100 ohm
* 1 buzzer
* 1 switch
* 1 button
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
* 9 = INPUT, botón
* 12 = OUTPUT, buzzer

Funcionamiento
==============
* El switch "alimenta" o no el Arduino. El botón cumple las funciones de iniciar los procesos y detenerlos. 
* En rasgos generales el PT sensa 25 minutos, descansa 5, sensa 25, descansa 5, sensa 25, descansa 5, sensa 25, descansa 15 y así sucesivamente en bucles.
* El inicio de los pomodoros es manual.
* Se entra en descanso con el botón paro el descanso y si lo presiono una vez mas comienzo el nuevo lapso laboral.
* A los 12 pomodoros se hace un juego de luces y buzzer, a los 22 hace otro juego de luces y buzzer.
* Cuando el switch se setea off el contador se reinicia a 0.
* Los leds verdes indican el progreso del pomodoro, cuando termine el pomodoro van a oscilar 5 veces y switchear a la parte break.
* Cuando se enciende(switch on) se realiza un juego de luces.
* Si un led azul está encendido se está en proceso de break corto, si los dos están encendidos se está en un break largo.
* Cuando termina break los leds azules oscilan 2 veces y después switchean al rojo.
* El led rojo denota "espera"(hold).
* Si se para un pomodoro o break directamente se switchea a led rojo.
* Cuando se da inicio a un pomodoro el led verde oscila una vez y luego queda encendido.
* Primer led verde enciende a los 0 segundos de comenzado el pomodoro, el segundo led enciende al minuto 9, el ultimo enciende al minuto 18.
* El buzzer suena cuando se interrumpe un pomodoro y cuando finaliza un pomodoro.
*/

// include aliases of sounds as frecuencies
#include "pitches.h"

/* Function prototypes */
void checkSwitch(void);
void checkButton(void);
void startPomodoro(void);
void resetEverything(void);
void cancelCurrentPomodoro(void);
void checkCurrentPomodoro(void);
void startBreak(void);
void makePomodoroFinishedLightGame(void);
void cancelBreak(void);
void checkCurrentBreak(void);
void makeSystemOnLightGame(void);
void makePomodoroN12FinishedLightGame(void);
void makePomodoroN22FinishedLightGame(void);
void makeBreakFinishedLightGame(void);
void finishBreak(void);
void soundSadBuzzer(void);
void soundHappyBuzzer(void);

/* Global variables */
int switchInitialPosition = 0;
// system is on when the switch is not in the initial position
bool systemOn = false;
int pomodorosFinished = 0;
// the button will be censused by default, but will hold if pressed for 1 second
bool buttonCensusOn = true;
// is the button pressed? Start as off, but at the very beginning is checked
bool buttonPressed = false;
// is a pomodoro currently running?
bool pomodoroRunning = false;
// or there is a break right now?
bool breakRunning = false;
// or a stop is currently happening?
bool stopped = true;
unsigned long lastTimeButtonWasPressed = 0;
unsigned long startTimeOfCurrentPomodoro = 0;
unsigned long startTimeOfCurrentBreak = 0;


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
  pinMode(9, INPUT);
  pinMode(12, OUTPUT);
  // system never start "on", doesn't matter in which position is the switch, its turning-on depends on the contrary state in which it begins
  switchInitialPosition = digitalRead(8);
}

// This can run 16000 times per second, but most of the time runs twice per second.
void loop() {
  // check if system should be on
  checkSwitch();
  // check if button is being pressed
  checkButton();
  // main process
  if(systemOn) {
    // what is currently happening?
    if(pomodoroRunning) {
      if(buttonPressed) {
        cancelCurrentPomodoro();
        // event realized
        buttonPressed = false;
      } else {
        // check how many time has passed since start time of current pomodoro, turn on led if needed, or finish it is 25 minutes has been reached
        checkCurrentPomodoro();
      }
    } else if(breakRunning) {
      if(buttonPressed) {
        // break the break
        cancelBreak();
        // event realized
        buttonPressed = false;
      } else {
        // check current break, may be finished
        checkCurrentBreak();
      }
    } else if(stopped) {
      // only if button is currently pushed this state will change
      if(buttonPressed) {
        // shut off stop led
        digitalWrite(7, LOW);
        // system should not be stopped now, start pomodoro
        stopped = false;
        startPomodoro();
        // event realized
        buttonPressed = false;
      }
    }
  } else {
    // shut down everything and reset some variables
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
    }
  }
}

// Triggered when system is turned on. The button census is ignored until this function finishes.
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

// Make a complex checking on the button. If a change is detected on input, will hold 1 second until next census. If button keep pressed for more than 1 second then another event will raise for the whole system and so on. Is the
// responsability of external code to the function to reset the buttonPressed flag to false once they dealt with the event.
void checkButton() {
  // if button being censused
  if(buttonCensusOn) {
    // if button is pressed
    if(digitalRead(9)) {
      // pressing the button will have different meanings depending in the current state of the pomodoro tracker, so a flag will be setup
      buttonPressed = true;
      // do not census for 1 second
      buttonCensusOn = false;
      lastTimeButtonWasPressed = millis();
    }
  } else {
    // if a second has passed since the button was pressed then start censusing the button again
    if((millis() - lastTimeButtonWasPressed) >= 10000) {
      buttonCensusOn = true;
    }
  }
}

// Casted when button pressed while a pomodoro is running.
void cancelCurrentPomodoro() {
  pomodoroRunning = false;
  stopped = true;
  // turn on stop led
  digitalWrite(7, HIGH);
  // turn off leds
  digitalWrite(2, LOW);
  digitalWrite(3, LOW);
  digitalWrite(4, LOW);
  // buzzer is not happy :(
  soundSadBuzzer();
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

// In every main loop, if a pomodoro is running check if a led is needed to be turned on, or maybe the pomodoro is already completed.
void checkCurrentPomodoro() {
  // check how many time has passed since pomodoro beginning
  unsigned long pomodoroCurrentTimeCount = millis() - startTimeOfCurrentPomodoro;
  if(pomodoroCurrentTimeCount >= 1500000) {
    // pomodoro completed
    pomodorosFinished++;
    // make the happy sound
    soundHappyBuzzer();
    // make the light game, I'll refuse the census of the button here
    makePomodoroFinishedLightGame();
    pomodoroRunning = false;
    // check out if this is the pomodoro number 12 or 22
    switch(pomodorosFinished) {
      case 12 :
        makePomodoroN12FinishedLightGame();
        break;
      case 22 :
        makePomodoroN22FinishedLightGame();
        break;   
    }
    // begin break
    startBreak();
  } else if(pomodoroCurrentTimeCount >= 1080000) {
    // turn green led #3 on
    digitalWrite(4, HIGH);
  } else if(pomodoroCurrentTimeCount >= 540000) {
    // turn green led #2 on
    digitalWrite(3, HIGH);
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

// When a pomodoro finish, the break start.
void startBreak() {
  breakRunning = true;
  // inquire what pomodor break is this, may be a long one
  if((pomodorosFinished % 4) == 0) {
    // long one
    digitalWrite(5, HIGH);
    digitalWrite(6, HIGH);
  } else {
    // short one
    digitalWrite(5, HIGH);
  }
  // save when the break started
  startTimeOfCurrentBreak = millis();
}

// Triggered when button is pressed while a break is happening.
void cancelBreak() {
  // turn off blue leds
  breakRunning = false;
  digitalWrite(5, LOW);
  digitalWrite(6, LOW);
  // turn on thy stop red led
  stopped = true;
  digitalWrite(7, HIGH);
}

// In every main loop check if break has finished.
void checkCurrentBreak() {
  // check how many time has passed since start of the break(depends on if 4th multiple or not)
  if((pomodorosFinished % 4) == 0) {
    // long break
    if((millis() - startTimeOfCurrentBreak) >= 900000) {
      // break is done
      finishBreak();
    }
  } else {
    // short break
    if((millis() - startTimeOfCurrentBreak) >= 300000) {
      // break is done
      finishBreak();
    }
  }
}

// Break is finished so do this.
void finishBreak() {
  // make the dance
  makeBreakFinishedLightGame();
  // turn blue leds off
  digitalWrite(5, LOW);
  digitalWrite(6, LOW);
  breakRunning = false;
  // turn stop red led on
  digitalWrite(7, HIGH);
  stopped = true;
}

// Triggered when break time is done.
void makeBreakFinishedLightGame() {
  for(int counter = 0; counter < 2; counter++) {
    digitalWrite(5, LOW);
    digitalWrite(6, LOW);
    delay(1000);
    digitalWrite(5, HIGH);
    digitalWrite(6, HIGH);
    delay(1000);
  }
}

// Called after a button press while current state is stopped.
void startPomodoro() {
  // pomodoro is now running
  pomodoroRunning = true;
  // the button will be holding for 1 second until next census, so this can be done quietly
  digitalWrite(2, HIGH);
  delay(500);
  digitalWrite(2, LOW);
  delay(500);
  digitalWrite(2, HIGH);
  // now is time to rest, keep track of start time
  startTimeOfCurrentPomodoro = millis();
}

// Called when system has been turned off. Resets everything to its pristine status.
void resetEverything() {
  // reset/initialize some variables
  pomodorosFinished = 0;
  // and leds
  digitalWrite(2, LOW);
  digitalWrite(3, LOW);
  digitalWrite(4, LOW);
  digitalWrite(5, LOW);
  digitalWrite(6, LOW);
  digitalWrite(7, LOW);
}
