#include <DigiCDC.h>

#define DIT 0
#define DAH 2
#define KEY 1 // P1 with on-board LED
#define SPEED 0 // P5

#define DAH_WEIGHT 2.5
#define PAUSE_WEIGHT 1.5
#define STEP 2 // delay() granularity

char send[100];
char *send_ptr;
int duration;
volatile int state = 0;

void setup() {
  SerialUSB.begin();
  pinMode(DIT, INPUT_PULLUP);
  pinMode(DAH, INPUT_PULLUP);
  pinMode(KEY, OUTPUT);
  char *send = '\0';
  send_ptr = send;
}

static inline bool dit_press() {
  return !digitalRead(DIT);
}

static inline bool dah_press() {
  return !digitalRead(DAH);
}

static inline int get_duration() {
  return (1120 - analogRead(SPEED)) / 4;
}

static void get_usb()
{
  if (SerialUSB.available()) {
    char input = SerialUSB.read();
    *send_ptr = input;
    send_ptr++;
  }
}

static inline void wait(int duration) {
  SerialUSB.delay(duration);
}

static char *morse_char(char c) {
  switch (c) {
    case ' ': return " ";
    case 'A': return ".-";
    case 'B': return "-...";
    case 'C': return "-.-.";
    case 'D': return "-..";
    case 'E': return ".";
    case 'F': return "..-.";
    case 'G': return "--.";
    case 'H': return "....";
    case 'I': return "..";
              /*
    case 'J': return ".---";
    case 'K': return "-.-";
    case 'L': return ".-..";
    case 'M': return "--";
    case 'N': return "-.";
    case 'O': return "---";
    case 'P': return ".--.";
    case 'Q': return "--.-";
    case 'R': return ".-.";
    case 'S': return "...";
    case 'T': return "-";
    case 'U': return "..-";
    case 'V': return "...-";
    case 'W': return ".--";
    case 'X': return "-..-";
    case 'Y': return "--.-";
    case 'Z': return "--..";
    case '0': return "-----";
    case '1': return ".----";
    case '2': return "..---";
    case '3': return "...--";
    case '4': return "....-";
    case '5': return ".....";
    case '6': return "-....";
    case '7': return "--...";
    case '8': return "---..";
    case '9': return "----.";
    case '/': return "-..-.";
    case '=': return "-...-";
    case '-': return "-....-";
    */
    default: return NULL;
  }
}

static void send_symbol(int level, int duration, int dit_state, int dah_state)
{
  int i;
  digitalWrite(KEY, level);
  for (i = 0; i < duration; i += STEP) {
    wait(STEP);
    get_usb();
    if (dit_state && dit_press())
      state = dit_state;
    if (dah_state && dah_press())
      state = dah_state;
  }
}

void loop() {
  int i;
  char *morse;

  while(1) {
    duration = get_duration();

    switch (state) {

      case 0: /* start */
        digitalWrite(KEY, LOW);
        *send = '\0';
        send_ptr = send;
        state = 100; /* go to main loop */
        break;

      case 100: /* main loop */
        wait(STEP);
        get_usb();
        if (dit_press())
          state = 1;
        else if (dah_press())
          state = 2;
        if (*send)
          state = 10;

        break;

      case 1: /* dit */
        state = 3; /* go to pause after dit */
        send_symbol(HIGH, duration, 0, 6); /* go to pause before dah */
        break;

      case 2: /* dah */
        state = 4; /* go to pause after dah */
        send_symbol(HIGH, 3*duration, 5, 0); /* go to pause before dit */
        break;

      case 3: /* pause after dit */
        state = 0; /* go to idle */
        send_symbol(LOW, duration, 0, 2); /* go to dah */
        break;

      case 4: /* pause after dah */
        state = 0; /* go to idle */
        send_symbol(LOW, duration, 1, 0); /* go to dit */
        break;

      case 5: /* pause before dit */
        state = 1; /* go to dit */
        send_symbol(LOW, duration, 1, 0); /* still go to dit */
        break;

      case 6: /* pause before dah */
        state = 2; /* go to dah */
        send_symbol(LOW, duration, 2, 0); /* still go to dah */
        break;

      case 10: /* USB loop */
        if (!*send) {
          state = 11;
          break;
        }
        morse = morse_char(*send);
        if (!morse) {
          state = 11;
          break;
        }
        for (int i = 0; send[i+1]; i++) {
          send[i] = send[i+1]; /* shift array left */
        }
        send_ptr--;
        *send_ptr = '\0';

        for (char *p = morse; *p; p++) {
          switch (*p) {
            case '.':
              send_symbol(HIGH, duration, 11, 11);
              break;
            case '-':
              send_symbol(HIGH, DAH_WEIGHT * duration, 11, 11);
              break;
            case ' ':
              send_symbol(LOW, PAUSE_WEIGHT * duration, 11, 11);
              break;
          }
          send_symbol(LOW, duration, 11, 11);
          if (state == 11) break;
        }
        send_symbol(LOW, DAH_WEIGHT * duration, 11, 11);

        break;

      case 11: /* cancel USB operation */
        digitalWrite(KEY, LOW);
        while (dit_press() || dah_press()) {
          wait(STEP);
          get_usb();
        }
        state = 0;
        break;

    }
  }
}
