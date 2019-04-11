#include <DigiCDC.h>

#define DIT 0
#define DAH 2
#define KEY 1 // P1 with on-board LED
#define SPEED 0 // P5

#define DAH_WEIGHT 2.5
#define STEP 2 // delay() granularity

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
  return (1120 - analogRead(SPEED)) / 4;
}

static inline void wait(int d) {
  SerialUSB.delay(d);
}

void loop() {
  volatile int state = 0;
  int d;
  int i;

  while(1) {
    d = duration();

    switch (state) {

      case 0: /* start */
        digitalWrite(KEY, LOW);

        if (SerialUSB.available()) {
          char input = SerialUSB.read();
          *send = input;
          state = 10;
        }

        if (dit_press())
          state = 1;
        else if (dah_press())
          state = 2;

        break;

      case 1: /* dit */
        digitalWrite(KEY, HIGH);
        state = 3; /* go to pause after dit */
        for (i = 0; i < d; i += STEP) {
          wait(STEP);
          if (dah_press())
            state = 6; /* go to pause before dah */
        }
        break;

      case 2: /* dah */
        d *= DAH_WEIGHT;
        digitalWrite(KEY, HIGH);
        state = 4; /* go to pause after dah */
        for (i = 0; i < d; i += STEP) {
          wait(STEP);
          if (dit_press())
            state = 5; /* go to pause before dit */
        }
        break;

      case 3: /* pause after dit */
        digitalWrite(KEY, LOW);
        state = 0; /* go to idle */
        for (i = 0; i < d; i += STEP) {
          wait(STEP);
          if (dah_press())
            state = 2; /* go to dah */
        }
        break;

      case 4: /* pause after dah */
        digitalWrite(KEY, LOW);
        state = 0; /* go to idle */
        for (i = 0; i < d; i += STEP) {
          wait(STEP);
          if (dit_press())
            state = 1; /* go to dit */
        }
        break;

      case 5: /* pause before dit */
        digitalWrite(KEY, LOW);
        state = 1; /* go to dit */
        for (i = 0; i < d; i += STEP) {
          wait(STEP);
          if (dit_press())
            state = 1; /* still go to dit */
        }
        break;

      case 6: /* pause before dah */
        digitalWrite(KEY, LOW);
        state = 2; /* go to dah */
        for (i = 0; i < d; i += STEP) {
          wait(STEP);
          if (dit_press())
            state = 2; /* still go to dah */
        }
        break;

      case 10: /* USB input */
        switch (*send) {
          case 'A':
            digitalWrite(KEY, HIGH);
            for (i = 0; i < d; i += STEP) {
              wait(STEP);
              if (dit_press() || dah_press()) {
                state = 11; /* go to cancel */
                break;
              }
            }
            digitalWrite(KEY, LOW);
            for (i = 0; i < d; i += STEP) {
              wait(STEP);
              if (dit_press() || dah_press()) {
                state = 11;
                break;
              }
            }
            digitalWrite(KEY, HIGH);
            for (i = 0; i < DAH_WEIGHT*d; i += STEP) {
              wait(STEP);
              if (dit_press() || dah_press()) {
                state = 11;
                break;
              }
            }
            digitalWrite(KEY, LOW);
            for (i = 0; i < d; i += STEP) {
              wait(STEP);
              if (dit_press() || dah_press()) {
                state = 11;
                break;
              }
            }
        }
        state = 11;
        break;

      case 11: /* cancel USB operation */
        while (dit_press() || dah_press()) {
          wait(STEP);
        }
        state = 0;
        break;

    }
  }
}
