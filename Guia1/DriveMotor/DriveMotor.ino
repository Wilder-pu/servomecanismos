#include <util/atomic.h>

#define ENCA 2
#define ENCB 3
#define PWM 5
#define IN2 9
#define IN1 8
volatile int posi = 0;

void setup() {
  Serial.begin(9600);
  pinMode(ENCA, INPUT);
  pinMode(ENCB, INPUT);
  attachInterrupt(digitalPinToInterrupt(ENCA), readEncoder, RISING);

  pinMode(PWM, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
}

void loop() {

  int pos = 0;

  setMotor(1, 255, PWM, IN1, IN2);
  delay(500);

  ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
    pos = posi;
  }
  Serial.println(pos);

  setMotor(0, 255, PWM, IN1, IN2);
  delay(500);

  ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
    pos = posi;
  }
  Serial.println(pos);

  setMotor(-1, 255, PWM, IN1, IN2);
  delay(500);

  ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
    pos = posi;
  }
  Serial.println(pos);
  setMotor(0, 255, PWM, IN1, IN2);
  delay(500);

  ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
    pos = posi;
  }
  Serial.println(pos);
}


void setMotor(int dir, int pwmVal, int pwm, int in1, int in2){
  analogWrite(pwm, pwmVal);
  if(dir == 1){
    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);
  }
  else if(dir == -1){
    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);
  }
  else{
    digitalWrite(in1, LOW);
    digitalWrite(in2, LOW);
  }
}

void readEncoder(){
  int b = digitalRead(ENCB);

  if(b == HIGH){
    posi--;
  }
  else{
    posi++;
  }
}