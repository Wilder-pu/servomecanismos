#include <util/atomic.h>

// Encoder pins
#define ENCA 2
#define ENCB 3

// L298N pins
#define PWM 5
#define IN1 6
#define IN2 7

// Prototipos de funciones
void setMotor(int dir, int pwmVal, int pwm, int in1, int in2);
void readEncoder();
void showMenu();
void clear_serial_buffer();
void read_encoder_loop();
void measure_position_loop();
void drive_motor_loop();
void controlMotorLoop();

// Variable volatil
volatile int posi = 0;
long prevT = 0;
float eprev = 0;
float eintegral = 0;

// Ganancias PID (ajustables desde el serial en la opcion 4)
float kp = 1.0;
float kd = 0.025;
float ki = 0.0;

// Posicion objetivo (ajustable desde el serial en la opcion 4)
float target = 0;

void setup() {
  Serial.begin(9600);
  pinMode(ENCA, INPUT);
  pinMode(ENCB, INPUT);
  attachInterrupt(digitalPinToInterrupt(ENCA), readEncoder, RISING);

  pinMode(PWM, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);

  showMenu();
}

void loop() {
  if (Serial.available() > 0) {
    char option = Serial.read();

    if (option == '\n' || option == '\r') {
      return;
    }

    switch (option) {
      case '1':
        read_encoder_loop();
        break;
      case '2':
        measure_position_loop();
        break;
      case '3':
        drive_motor_loop();
        break;
      case '4':
        controlMotorLoop();
        break;
      case '5':
        showMenu();
        break;
      default:
        Serial.println();
        Serial.println("Opcion no valida.");
        Serial.println("Seleccione una opcion del 1 al 5.");
        Serial.println();
        showMenu();
        break;
    }
  }
}

void showMenu() {
  Serial.println();
  Serial.println("===== MENU =====");
  Serial.println("1: Leer encoder (A/B)");
  Serial.println("2: Medir posicion");
  Serial.println("3: Probar motor (drive_motor)");
  Serial.println("4: Control PID (controlMotor)");
  Serial.println("5: Mostrar menu");
  Serial.println("================");
}

void clear_serial_buffer() {
  while (Serial.available() > 0) {
    Serial.read();
  }
}

// PRIMERA PARTE, LECTURA DEL ENCODER
void read_encoder() {
  int a = digitalRead(ENCA);
  int b = digitalRead(ENCB);
  Serial.print(a * 5);
  Serial.print(" ");
  Serial.print(b * 5);
  Serial.println();
}

void read_encoder_loop() {
  Serial.println("Leyendo encoder. Envie cualquier caracter para detener.");
  delay(10);
  clear_serial_buffer();
  while (Serial.available() == 0) {
    read_encoder();
    delay(50);
  }
  clear_serial_buffer();
  showMenu();
}

// SEGUNDA PARTE, LECTURA DE POSICION
void measure_position() {
  int pos = 0;
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    pos = posi;
  }
  Serial.println(pos);
}

void measure_position_loop() {
  Serial.println("Leyendo posicion. Envie cualquier caracter para detener.");
  delay(10);
  clear_serial_buffer();
  while (Serial.available() == 0) {
    measure_position();
    delay(50);
  }
  clear_serial_buffer();
  showMenu();
}

void readEncoder() {
  int b = digitalRead(ENCB);
  if (b > 0) {
    posi++;
  } else {
    posi--;
  }
}

// CONTROLADOR DEL MOTOR L298N
void setMotor(int dir, int pwmVal, int pwm, int in1, int in2) {
  analogWrite(pwm, pwmVal);
  if (dir == 1) {
    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);
  } else if (dir == -1) {
    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);
  } else {
    digitalWrite(in1, LOW);
    digitalWrite(in2, LOW);
  }
}

// TERCERA PARTE, DRIVE THE MOTOR
void drive_motor_loop() {
  Serial.println("Iniciando prueba manual de motor.");
  Serial.println("Comandos: pwm=<0-255>  dir=<-1, 0, 1>  stop");
  delay(10);
  clear_serial_buffer();

  bool running = true;
  int current_pwm = 0;
  int current_dir = 0;
  unsigned long lastPrintTime = 0;

  while (running) {
    if (Serial.available() > 0) {
      String cmd = Serial.readStringUntil('\n');
      cmd.trim();

      if (cmd.equalsIgnoreCase("stop")) {
        running = false;
      } else if (cmd.startsWith("pwm=")) {
        current_pwm = cmd.substring(4).toInt();
        if (current_pwm > 255) current_pwm = 255;
        if (current_pwm < 0) current_pwm = 0;
        Serial.print("Nuevo PWM = ");
        Serial.println(current_pwm);
      } else if (cmd.startsWith("dir=")) {
        current_dir = cmd.substring(4).toInt();
        Serial.print("Nueva DIR = ");
        Serial.println(current_dir);
      } else if (cmd.length() > 0) {
        Serial.println("Comando no reconocido. Use pwm=, dir= o stop");
      }
    }

    setMotor(current_dir, current_pwm, PWM, IN1, IN2);

    if (millis() - lastPrintTime > 50) {
      int pos = 0;
      ATOMIC_BLOCK(ATOMIC_RESTORESTATE) { 
        pos = posi; 
      }
      Serial.print("Posicion: ");
      Serial.println(pos);
      
      lastPrintTime = millis();
    }
  }

  setMotor(0, 0, PWM, IN1, IN2);
  clear_serial_buffer();
  showMenu();
}

// PARTE CUATRO, CONTROL THE MOTOR
void controlMotor() {
  long currT = micros();
  float deltaT = ((float)(currT - prevT)) / (1.0e6);
  prevT = currT;

  int pos = 0;
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    pos = posi;
  }

  int e = pos - target;
  float dedt = (e - eprev) / (deltaT);
  eintegral = eintegral + e * deltaT;

  float u = kp * e + kd * dedt + ki * eintegral;

  float pwr = fabs(u);
  if (pwr > 255) {
    pwr = 255;
  }

  int dir = 1;
  if (u < 0) {
    dir = -1;
  }

  setMotor(dir, pwr, PWM, IN1, IN2);

  eprev = e;
  Serial.print("Target:");
  Serial.print(target);
  Serial.print(" Posicion:");
  Serial.println(pos);
}

void controlMotorLoop() {
  Serial.println("Iniciando control PID.");
  Serial.println("Comandos: pos=<valor>  kp=<valor>  kd=<valor>  ki=<valor>  stop");
  delay(10);
  clear_serial_buffer();

  bool running = true;
  while (running) {
    if (Serial.available() > 0) {
      String cmd = Serial.readStringUntil('\n');
      cmd.trim();

      if (cmd.equalsIgnoreCase("stop")) {
        running = false;
      } else if (cmd.startsWith("pos=")) {
        target = cmd.substring(4).toFloat();
        Serial.print("target = ");
        Serial.println(target);
      } else if (cmd.startsWith("kp=")) {
        kp = cmd.substring(3).toFloat();
        Serial.print("kp = ");
        Serial.println(kp);
      } else if (cmd.startsWith("kd=")) {
        kd = cmd.substring(3).toFloat();
        Serial.print("kd = ");
        Serial.println(kd);
      } else if (cmd.startsWith("ki=")) {
        ki = cmd.substring(3).toFloat();
        Serial.print("ki = ");
        Serial.println(ki);
      } else if (cmd.length() > 0) {
        Serial.println("Comando no reconocido. Use pos=, kp=, kd=, ki= o stop");
      }
    }
    controlMotor();
  }

  setMotor(0, 0, PWM, IN1, IN2);
  clear_serial_buffer();
  showMenu();
}