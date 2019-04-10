#include <DigiCDC.h>

#define DIT 0
#define DAH 2
#define KEY 1 // P1 with on-board LED
#define SPEED 0 // P5

#define STEP 5 // delay() granularity

char send[100];

void setup() {
  SerialUSB.begin();
  pinMode(DIT, INPUT_PULLUP);
  pinMode(DAH, INPUT_PULLUP);
  pinMode(KEY, OUTPUT);
  char *send = '\0';
}

static inline bool dit_press() {
  return !digitalRead(DIT);
}

static inline bool dah_press() {
  return !digitalRead(DAH);
}

static inline int duration() {
  return (1200 - analogRead(SPEED)) / 4;
}

static inline void wait(int d) {
  SerialUSB.delay(d);
}

void loop() {
  int state = 0;
  int d;
  int i;

  while(1) {
    switch (state) {

      case 0: /* start */
        if (SerialUSB.available()) {
          char input = SerialUSB.read();
          *send = input;
          state = 10;
        }

        if (dit_press())
          state = 2;
        else if (dah_press())
          state = 3;

        break;

      case 1: /* pause */
        d = duration();
        digitalWrite(KEY, LOW);
        state = 0;
        for (i = 0; i < d; i += STEP) {
          wait(STEP);
          if (dit_press())
            state = 2;
          else if (dah_press())
            state = 3;
        }
        break;

      case 2: /* dit */
        d = duration();
        digitalWrite(KEY, HIGH);
        state = 1;
        for (i = 0; i < d; i += STEP) {
          wait(STEP);
          if (dah_press())
            state = 5;
        }
        break;

      case 3: /* dah */
        d = 3 * duration();
        digitalWrite(KEY, HIGH);
        state = 1;
        for (i = 0; i < d; i += STEP) {
          wait(STEP);
          if (dit_press())
            state = 4;
        }
        break;

      case 4: /* pause before dit */
        d = duration();
        digitalWrite(KEY, LOW);
        state = 2;
        wait(d);
        break;

      case 5: /* pause before dah */
        d = duration();
        digitalWrite(KEY, LOW);
        state = 3;
        wait(d);
        break;

      case 10: /* USB input */
        switch (*send) {
          case 'A':
            digitalWrite(KEY, HIGH);
            wait(duration());
            digitalWrite(KEY, LOW);
            wait(duration());
            digitalWrite(KEY, HIGH);
            wait(3 * duration());
            digitalWrite(KEY, LOW);
            wait(duration());
        }
        state = 0;

    }
  }
}
