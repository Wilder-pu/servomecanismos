#include <util/atomic.h>

// =========================
// Pines Arduino
// =========================
#define ENCA 2
#define ENCB 3
#define PWM_PIN 5
#define IN2 9
#define IN1 8

// =========================
// Variables globales
// =========================
volatile int posi = 0;
volatile int target = 0;

long prevT = 0;
float eprev = 0;
float eintegral = 0;

// =========================
// Constantes PID
// =========================
float kp = 0.0009;
float ki = 0.0;
float kd = 0.00005;

// =========================
// Variables para Serial Plotter
// =========================
int lineaCero = 0;
float limiteSuperior = 0;

// =========================
// Interrupción encoder
// =========================
void readEncoder() {

  int b = digitalRead(ENCB);

  if (b > 0) {
    posi++;
  }
  else {
    posi--;
  }
}

// =========================
// Setup
// =========================
void setup() {

  Serial.begin(9600);

  pinMode(ENCA, INPUT);
  pinMode(ENCB, INPUT);

  attachInterrupt(digitalPinToInterrupt(ENCA), readEncoder, RISING);

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(PWM_PIN, OUTPUT);

  Serial.println("Sistema PID listo");
  Serial.println("Numero -> Target");
  Serial.println("P valor -> Cambiar Kp");
  Serial.println("I valor -> Cambiar Ki");
  Serial.println("D valor -> Cambiar Kd");
}

// =========================
// Loop principal
// =========================
void loop() {

  int pos = 0;

  // Lectura atómica
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    pos = posi;
  }

  // =========================
  // Lectura serial
  // =========================
  if (Serial.available()) {

    String input = Serial.readStringUntil('\n');
    input.trim();

    if (input.startsWith("P ")) {

      kp = input.substring(2).toFloat();

      Serial.print("Nuevo Kp: ");
      Serial.println(kp);
    }

    else if (input.startsWith("I ")) {

      ki = input.substring(2).toFloat();

      Serial.print("Nuevo Ki: ");
      Serial.println(ki);
    }

    else if (input.startsWith("D ")) {

      kd = input.substring(2).toFloat();

      Serial.print("Nuevo Kd: ");
      Serial.println(kd);
    }

    else {

      target = input.toInt();

      Serial.print("Nuevo Target: ");
      Serial.println(target);
    }
  }

  // =========================
  // Tiempo
  // =========================
  long currT = micros();

  float deltaT = ((float)(currT - prevT)) / 1.0e6;

  prevT = currT;

  // =========================
  // Error
  // =========================
  int e = pos - target;

  // =========================
  // Derivada
  // =========================
  float dedt = (e - eprev) / deltaT;

  // =========================
  // Integral
  // =========================
  eintegral = eintegral + e * deltaT;

  // =========================
  // Señal PID
  // =========================
  float u = kp * e + kd * dedt + ki * eintegral;

  // =========================
  // Compensación fricción
  // =========================
  float deadband = 5;

  if (abs(e) > deadband) {

    float frictionComp = 80;

    if (u > 0) u += frictionComp;

    if (u < 0) u -= frictionComp;
  }

  // =========================
  // Potencia
  // =========================
  float pwr = fabs(u);

  if (pwr > 255) {
    pwr = 255;
  }

  // Convertir a 0-255
  
  // int pwmValue = map(pwmPercent, 0, 100, 0, 255);
  // =========================
  // Dirección
  // =========================
  int dir = 1;

  if (u < 0) {
    dir = -1;
  }

  // =========================
  // Mover motor
  // =========================
  setMotor(dir, pwr, PWM_PIN, IN1, IN2);

  // Guardar error previo
  eprev = e;

  // =========================
  // Variables para escala
  // =========================
  limiteSuperior = target * 1.1;

  // =========================
  // Serial Plotter
  // =========================
  Serial.print("Cero:");
  Serial.print(lineaCero);
  Serial.print(",");

  Serial.print("Target:");
  Serial.print(target);
  Serial.print(",");

  Serial.print("Posicion:");
  Serial.print(pos);
  Serial.print(",");

  Serial.print("Limite:");
  Serial.println(limiteSuperior);

  delay(10);
}

// =========================
// Función motor
// =========================
void setMotor(int dir, int pwmVal, int pwm, int in1, int in2) {

  analogWrite(pwm, pwmVal);

  if (dir == 1) {

    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);
  }

  else if (dir == -1) {

    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);
  }

  else {

    digitalWrite(in1, LOW);
    digitalWrite(in2, LOW);
  }
}