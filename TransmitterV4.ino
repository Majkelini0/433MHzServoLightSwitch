#include <RCSwitch.h>

RCSwitch tx = RCSwitch();

const int button1 = 2; // right
const int button2 = 4; // left
const int button3 = 3; // settings

bool lastB1 = HIGH;
bool lastB2 = HIGH;
bool lastB3 = HIGH;

const unsigned long debounceDelay = 150; // debounce time in ms
unsigned long lastB1Press = 0;
unsigned long lastB2Press = 0;
unsigned long lastB3Press = 0;

bool settingsMode = false;

const int CMD_SEND_L = 1;
const int CMD_SET_L = 2;
const int CMD_STEP_L = 3;
const int CMD_SAVE = 4;
const int CMD_STEP_R = 5;
const int CMD_SET_R = 6;
const int CMD_SEND_R = 7;

void setup() {
  pinMode(button1, INPUT_PULLUP);
  pinMode(button2, INPUT_PULLUP);
  pinMode(button3, INPUT_PULLUP);
  tx.enableTransmit(12);
  
  // Optional set protocol (default is 1, will work for most outlets)
  // tx.setProtocol(1);

  // Optional set pulse length.
  tx.setPulseLength(250); // Czas trwania wysylania sygnalu
  
  // Optional set number of transmission repetitions.
  // tx.setRepeatTransmit(15);
}

void test() {
  while(true) {
    tx.send(CMD_SEND_L, 24);
    delay(500);
    tx.send(CMD_SEND_R, 24);
    delay(500);
  }
}

void loop() {
  // test();

  bool b1 = digitalRead(button1);
  bool b2 = digitalRead(button2);
  bool b3 = digitalRead(button3);

  // Settings init
  if (!settingsMode) {

    if (b1 == LOW && b3 == LOW && (lastB1 == HIGH || lastB3 == HIGH)) {
      tx.send(CMD_SET_R, 24);
      settingsMode = true;

      while (digitalRead(button1) == LOW || digitalRead(button3) == LOW) {
        delay(10); // Small delay to prevent excessive CPU usage
      }
    }
    else if (b2 == LOW && b3 == LOW && (lastB2 == HIGH || lastB3 == HIGH)) {
      tx.send(CMD_SET_L, 24);
      settingsMode = true;

      while (digitalRead(button2) == LOW || digitalRead(button3) == LOW) {
        delay(10); // Small delay to prevent excessive CPU usage
      }
    }

    // Normal mode
    else if (b1 == LOW && lastB1 == HIGH && b3 == HIGH) {
      tx.send(CMD_SEND_R, 24);

      while (digitalRead(button1) == LOW || digitalRead(button3) == LOW) {
        delay(10); // Small delay to prevent excessive CPU usage
      }
    }
    else if (b2 == LOW && lastB2 == HIGH && b3 == HIGH) {
      tx.send(CMD_SEND_L, 24);

      while (digitalRead(button2) == LOW || digitalRead(button3) == LOW) {
        delay(10); // Small delay to prevent excessive CPU usage
      }
    }
  }

  // Settings mode
  if (settingsMode) {
    b1 = digitalRead(button1);
    b2 = digitalRead(button2);
    b3 = digitalRead(button3);

    if (b3 == LOW && lastB3 == HIGH && b1 == HIGH && b2 == HIGH) {
      tx.send(CMD_SAVE, 24);

      while (digitalRead(button3) == LOW) {
        delay(10); // Small delay to prevent excessive CPU usage
      }
      settingsMode = false;
    }
    else if (b1 == LOW && lastB1 == HIGH) {
      tx.send(CMD_STEP_R, 24);

      while (digitalRead(button1) == LOW) {
        delay(10); // Small delay to prevent excessive CPU usage
      }
    }
    else if (b2 == LOW && lastB2 == HIGH) {
      tx.send(CMD_STEP_L, 24);

      while (digitalRead(button2) == LOW) {
        delay(10); // Small delay to prevent excessive CPU usage
      }
    }
  }
  lastB1 = b1;
  lastB2 = b2;
  lastB3 = b3;
}
