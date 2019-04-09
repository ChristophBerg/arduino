#define RED 1
#define GREEN 4
#define BLUE 0

// the setup routine runs once when you press reset:
void setup() {                
  // initialize the digital pin as an output.
  pinMode(RED, OUTPUT);
  pinMode(GREEN, OUTPUT); //LED on Model A   
  pinMode(BLUE, OUTPUT);
}

// the loop routine runs over and over again forever:
void loop() {
  int r = 0;
  int g = 0;
  int b = 0;
  while(1) {
    int r2 = random(0, 16);
    int g2 = random(0, 16);
    int b2 = random(0, 16);
    int i;
    r2 = r2*r2; if (r2 == 256) r2 = 255;
    g2 = g2*g2; if (g2 == 256) g2 = 255;
    b2 = b2*b2; if (b2 == 256) b2 = 255;
    for (i = 0; i <= 100; i++) {
      analogWrite(RED,   r + i*(r2-r)/100.0);
      analogWrite(GREEN, g + i*(g2-g)/100.0);
      analogWrite(BLUE,  b + i*(b2-b)/100.0);
      delay(20);
    }
    delay(2000);
    r = r2;
    g = g2;
    b = b2;
  }
}
