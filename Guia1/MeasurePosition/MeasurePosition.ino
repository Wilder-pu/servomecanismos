#include <util/atomic.h>

#define ENCA 2
#define ENCB 3

volatile int posi = 0;

void setup() {
  Serial.begin(9600);
  pinMode(ENCA, INPUT);
  pinMode(ENCB, INPUT);
  attachInterrupt(digitalPinToInterrupt(ENCA), readEncoder, RISING);
}

void loop() {
  int pos = 0;
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    pos = posi;
  }
  Serial.println(pos);
  delay(100);
}

void readEncoder(){
  int b = digitalRead(ENCB);
  if(b>0){
    posi++;
  }
  else{
    posi--;
  }
}
