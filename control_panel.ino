#include <Keyboard.h>
 
// Knopjes
const int power = 13;
const int dis1 = 2;
const int dis2 = 3;
const int gates = 4;
const int restraints = 5;
const int estop = 6;
const int reset = 7;
const int functie1 = 8;
 
// Lampjes
const int dis1l = 9;
const int dis2l = 10;
const int resetl = 11;
const int functionl = 12;
 
// Opties
bool gatesOpen = true;
bool resOpen = true;
bool estopped = false;
bool canDispatch = true;
bool trainParked = true;
bool lightTest = false;
bool systemError = false; // Als je deze aan zou zetten zou je moeten beginnen met een reset
bool keyboardState = false;
bool preLoad = true; // In principe zou je vanaf het begin meteen moeten kunnen preloaden
 
// Dispatch
const long DispatchDelay = 5000;
unsigned long DispatchStartTime = 0;
 
// Ledjes
unsigned long currentMillis;
unsigned long previousBlinkMillis = 0;
const long blinkInterval = 500;
const long estopInterval = 250;
int ledState = LOW;
 
void setup() {
  // Pins knopjes
  pinMode(power, INPUT_PULLUP);
  pinMode(dis1, INPUT_PULLUP);
  pinMode(dis2, INPUT_PULLUP);
  pinMode(gates, INPUT_PULLUP);
  pinMode(restraints, INPUT_PULLUP);
  pinMode(estop, INPUT_PULLUP);
  pinMode(reset, INPUT_PULLUP);
  pinMode(functie1, INPUT_PULLUP);
 
  // Pins lampjes
  pinMode(dis1l, OUTPUT);
  pinMode(dis2l, OUTPUT);
  pinMode(resetl, OUTPUT);
  pinMode(functionl, OUTPUT);
}
 
// E-stop reset
void emergencyReset() {
  estopped = false;
  digitalWrite(resetl, LOW);
  Serial.write("RESETTING...\n");
  Keyboard.press(0x72);
  delay(5000);
  Serial.write("RESET!\n");
  Keyboard.releaseAll();
  delay(200);
}

// Reset fault doet nog niks
void faultReset() {
  systemError = false;
  digitalWrite(resetl, LOW);
  Serial.write("RESET!\n");
  delay(200);
}
 
// E-stop
void emergency() {
  Keyboard.press(0x73);
  delay(200);
  Keyboard.releaseAll();
 
  estopped = true;
  digitalWrite(resetl, HIGH);
  digitalWrite(dis1l, LOW);
  digitalWrite(dis2l, LOW);
  Serial.write("E-STOPPED\n");
 
  while (estopped == true) {
    if (digitalRead(reset) == LOW && digitalRead(estop) == LOW) {
      emergencyReset();
    }
    delay(300);
  }
}

// Vrijgave
void dispatch() {
  Keyboard.press(' ');
  Serial.write("DISPATCHED");
  delay(5000);
  Keyboard.releaseAll();
  trainParked = false;
  DispatchStartTime = currentMillis;
  delay(10000);
  trainParked = true;
}

// Functie/ advance
void functie() {
  Keyboard.press(KEY_RETURN);
  Serial.write("ADVANCED");
  delay(5000);
  Keyboard.releaseAll();
  preLoad = false;
  delay(10000); // Delay tussen staat van de knop
  preLoad = true;
}
 
// Poortjes
void openGates() {
  Serial.write("OPEN GATES\n");
  Keyboard.press(KEY_RIGHT_ARROW);
  gatesOpen = true;
  canDispatch = false;
  delay(2000);
  Keyboard.releaseAll();
}
 
void closeGates() {
  Serial.write("CLOSE GATES\n");
  Keyboard.press(KEY_LEFT_ARROW);
  gatesOpen = false;
  canDispatch = true;
  delay(2000);
  Keyboard.releaseAll();
}
 
// Beugels
void openRestraints() {
  Serial.write("OPEN RESTRAINTS\n");
  Keyboard.press(KEY_UP_ARROW);
  resOpen = true;
  canDispatch = false;
  delay(1000);
  Keyboard.releaseAll();
}
 
void closeRestraints() {
  Serial.write("CLOSE RESTRAINTS\n");
  Keyboard.press(KEY_DOWN_ARROW);
  resOpen = false;
  canDispatch = true;
  delay(1000);
  Keyboard.releaseAll();
}

// Update LED
void updateLights() {
  if (currentMillis - previousBlinkMillis >= blinkInterval) {
    previousBlinkMillis = currentMillis;
    if (ledState == LOW) {
      ledState = HIGH;
    } else {
      ledState = LOW;
    }
  }
  if (preLoad && !estopped) {
    digitalWrite(functionl, ledState);
  } else {
    digitalWrite(functionl, LOW);
  }
  if (canDispatch && !resOpen && !gatesOpen && !estopped && trainParked && !systemError) {
    digitalWrite(dis1l, ledState);
    digitalWrite(dis2l, ledState);
  } else {
    digitalWrite(dis1l, LOW);
    digitalWrite(dis2l, LOW);
  }
}
 
void loop() {
  currentMillis = millis();
  // Power
  if (digitalRead(power) == LOW) {
    // Lamptest
    if (!lightTest) {
      digitalWrite(dis1l, HIGH);
      digitalWrite(dis2l, HIGH);
      digitalWrite(resetl, HIGH);
      digitalWrite(functionl, HIGH);
      delay(1000);
      digitalWrite(dis1l, LOW);
      digitalWrite(dis2l, LOW);
      digitalWrite(resetl, LOW);
      digitalWrite(functionl, LOW);
      lightTest = true;
    }
    updateLights();
    // Zet keyboard aan
    if (!keyboardState) {
      Serial.begin(9600);
      Keyboard.begin();
      keyboardState = true;
    }
    // Dispatch
    if (digitalRead(dis1) == LOW && digitalRead(dis2) == LOW) {
      if (!gatesOpen && !resOpen && !estopped && !systemError) {
        dispatch();
        delay(1000);
      } else {
        systemError = true;
      }
    }
 
  // Functie
  if (digitalRead(functie1) == LOW) {
    if (!gatesOpen && !resOpen && !estopped) {
      functie();
    } else {
      systemError = true;
    }
  }

  // Poortjes
  if (digitalRead(gates) == HIGH && gatesOpen == false) {
    openGates();
  } else {
    systemError = true;
  }
 
  if (digitalRead(gates) == LOW && gatesOpen == true) {
    closeGates();
  } else {
    systemError = true;
  }
 
  // Beugels
  if (digitalRead(restraints) == HIGH && resOpen == false) {
    openRestraints();
  } else {
    systemError = true;
  }
 
  if (digitalRead(restraints) == LOW && resOpen == true) {
    closeRestraints();
  } else {
    systemError = true;
  }
 
  // E-stop
  if (digitalRead(estop) == HIGH && estopped == false) {
    emergency();
  }

  // Reset
  if (digitalRead(reset) == LOW && systemError == true) {
    faultReset();
  }

  } else {
    // Zet alles uit
    if (keyboardState) {
      Keyboard.end();
      keyboardState = false;
    }
    digitalWrite(dis1l, LOW);
    digitalWrite(dis2l, LOW);
    digitalWrite(resetl, LOW);
    digitalWrite(functionl, LOW);
    lightTest = false;
  }
}
