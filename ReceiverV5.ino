#include <RCSwitch.h>
#include <EEPROM.h>

RCSwitch rx = RCSwitch();

int maxR, maxL;
const int RLimit = 520; // < 520 NOT SAFE ! pulseWidth
const int LLimit = 2480; // > 2480 NOT SAFE ! pulseWidth
const int neutral = 1500; // pulseWidth
const int freeze = 300; // milliseconds
const int stepSize = 70; // pulseWidth

bool settingsMode = false;
const int RX_PIN = 2; // Physical pin 7 on ATtiny85
const int SERVO_PIN = 3; // Physical pin 2 on ATtiny85

const int CMD_SEND_L = 1;
const int CMD_SET_L = 2;
const int CMD_STEP_L = 3;
const int CMD_SAVE = 4;
const int CMD_STEP_R = 5;
const int CMD_SET_R = 6;
const int CMD_SEND_R = 7;

// Manual servo control functions
void disableRCSwitch() {
  rx.disableReceive(); // Disable RCSwitch interrupts
}

void enableRCSwitch() {
  rx.enableReceive(0); // Re-enable RCSwitch on interrupt 0
}

void writeServo(int pulseWidth, bool step = false) {
  // PWM manually for SG90 servo control
  // SG90 expects 50Hz (20ms period): 1000-2000µs pulse width
  // pulseWidth: 1000µs=0°, 1500µs=90°, 2000µs=180°
  unsigned long startTime = millis();
  int calcFreeze = freeze;
  if(step) {
    calcFreeze = 150;
  }
  
  while (millis() - startTime < calcFreeze) {
    digitalWrite(SERVO_PIN, HIGH);
    delayMicroseconds(pulseWidth);
    digitalWrite(SERVO_PIN, LOW);
    delayMicroseconds(20000 - pulseWidth); // Complete 20ms cycle
  }
}

void moveServo(int target, bool backToNeutral = false, bool step = false) {
  disableRCSwitch();
  pinMode(SERVO_PIN, OUTPUT);
  writeServo(target, step);
  
  if (backToNeutral) {
    writeServo(neutral);
  }
  pinMode(SERVO_PIN, INPUT);
  enableRCSwitch();
}

void saveSettings() {
  EEPROM.write(0, maxL & 0xFF);
  EEPROM.write(1, maxL >> 8);
  EEPROM.write(2, maxR & 0xFF);
  EEPROM.write(3, maxR >> 8);
}

void loadSettings() {
  maxL = EEPROM.read(0) | (EEPROM.read(1) << 8);
  maxR = EEPROM.read(2) | (EEPROM.read(3) << 8);

  if( maxR < RLimit || maxR > neutral || maxL > LLimit || maxL < neutral ) {
    maxR = neutral;
    maxL = neutral;
    saveSettings();
  }
}

void setup() {
  pinMode(SERVO_PIN, INPUT);  // Start as input
  pinMode(RX_PIN, INPUT);     // RX pin as input

  loadSettings();
  moveServo(1500);
  moveServo(800);
  moveServo(1500);
  moveServo(2200);
  moveServo(1500);
  enableRCSwitch();
}

void settingsLoop(bool isRight) {
  bool inSettings = true;
  int maxL_OLD = maxL;
  int maxR_OLD = maxR;

  while (inSettings) {
    if (rx.available()) {
      int cmd = rx.getReceivedValue();

      if (cmd == CMD_STEP_R) {
        if (isRight && maxR - stepSize >= RLimit) {
          maxR -= stepSize;
          moveServo(maxR, false, true);
        }
        if (!isRight && maxL - stepSize >= neutral) {
          maxL -= stepSize;
          moveServo(maxL, false, true);
        }
      }
      else if (cmd == CMD_STEP_L) {
        if (!isRight && maxL + stepSize <= LLimit) {
          maxL += stepSize;
          moveServo(maxL, false, true);
        }
        if (isRight && maxR + stepSize <= neutral) {
          maxR += stepSize;
          moveServo(maxR, false, true);
        }
      }
      else if (cmd == CMD_SAVE) {
        if( maxL != maxL_OLD || maxR != maxR_OLD ) {
          saveSettings();
          delay(50);
        }
        moveServo(neutral);
        inSettings = false;
      }
      delay(150);
      rx.resetAvailable();
    }
  }
}

void loop() {
  if (rx.available()) {
    int cmd = rx.getReceivedValue();
    
    if (cmd == CMD_SET_R) {
      moveServo(maxR);
      settingsLoop(true);
    }
    else if (cmd == CMD_SET_L) {
      moveServo(maxL);
      settingsLoop(false);
    }
    else if (cmd == CMD_SEND_L) {
      moveServo(maxL, true);
    }
    else if (cmd == CMD_SEND_R) {
      moveServo(maxR, true);
    }
    rx.resetAvailable();
  }
}