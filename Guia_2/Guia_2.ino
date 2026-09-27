#include <util/atomic.h>

// =================================================================
// DEFINICIÓN DE PINES Y CONSTANTES
// =================================================================
#define ENCA 2      // Encoder canal A (Interrupción)
#define ENCB 3      // Encoder canal B
#define PWM  5      // Salida PWM hacia el driver L298N
#define IN1  6      // Dirección 1 driver
#define IN2  7      // Dirección 2 driver

// Conversión de resolución de encoder (Cuentas por Revolución eje de salida)
#define COUNTS_PER_REV 625.0 

// =================================================================
// VARIABLES GLOBALES
// =================================================================
// Interrupciones / Atómicas
volatile int pos_i = 0;
volatile float velocity_i = 0.0; // Velocidad instantánea en cuentas/s (Método 2)
volatile long prevT_i = 0;

// Variables de tiempo y posición previa (Método 1)
long prevT = 0;
int posPrev = 0;

// Variables de Filtrado (Filtro Paso Bajo 25 Hz)
float v1Filt = 0.0, v1Prev = 0.0;
float v2Filt = 0.0, v2Prev = 0.0;

// Variables Control PID
float kp = 5.0;
float ki = 10.0;
float kd = 0.0;
float eintegral = 0.0;
float eprev = 0.0;
float vt_target = 0.0; // Velocidad deseada en RPM

// =================================================================
// DECLARACIÓN DE FUNCIONES
// =================================================================
void readEncoder();
void setMotor(int dir, int pwmVal, int pwm_pin, int in1_pin, int in2_pin);
void clear_serial_buffer();
void showMenu();

void ejecutarParte1_LazoAbierto();
void ejecutarParte2_MedicionVelocidad();
void ejecutarParte3_FiltradoSenal();
void ejecutarParte4_ControlPIDVelocidad();

// =================================================================
// SETUP & LOOP PRINCIPAL
// =================================================================
void setup() {
  Serial.begin(115200);

  pinMode(ENCA, INPUT);
  pinMode(ENCB, INPUT);
  pinMode(PWM, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);

  // Configurar la interrupción en el canal A (Flanco de subida)
  attachInterrupt(digitalPinToInterrupt(ENCA), readEncoder, RISING);

  showMenu();
}

void loop() {
  if (Serial.available() > 0) {
    char opcion = Serial.read();
    clear_serial_buffer();

    switch (opcion) {
      case '1':
        ejecutarParte1_LazoAbierto();
        break;
      case '2':
        ejecutarParte2_MedicionVelocidad();
        break;
      case '3':
        ejecutarParte3_FiltradoSenal();
        break;
      case '4':
        ejecutarParte4_ControlPIDVelocidad();
        break;
      default:
        Serial.println("Opción invalida. Ingrese un numero del 1 al 4.");
        showMenu();
        break;
    }
  }
}

// =================================================================
// MENÚ PRINCIPAL
// =================================================================
void showMenu() {
  Serial.println(F("\n=================================================="));
  Serial.println(F("   GUIA 2: CONTROL DE VELOCIDAD PID DE MOTOR DC   "));
  Serial.println(F("=================================================="));
  Serial.println(F("1. Parte 1: Lazo Abierto (Prueba de PWM/Zona muerta)"));
  Serial.println(F("2. Parte 2: Medición de Velocidad (Método 1 vs Método 2)"));
  Serial.println(F("3. Parte 3: Filtrado de Señal (Filtro Paso Bajo)"));
  Serial.println(F("4. Parte 4: Control de Velocidad en Lazo Cerrado (PID)"));
  Serial.println(F("=================================================="));
  Serial.print(F("Seleccione una opción (1-4): "));
}

void clear_serial_buffer() {
  while (Serial.available() > 0) {
    Serial.read();
  }
}

// =================================================================
// RUTINA DE INTERRUPCIÓN (ENCODER)
// =================================================================
void readEncoder() {
  int b = digitalRead(ENCB);
  int increment = (b > 0) ? 1 : -1;

  pos_i += increment;

  // Cálculo de velocidad por Método 2 (Tiempo entre pulsos)
  long currT = micros();
  float deltaT = ((float)(currT - prevT_i)) / 1.0e6;
  if (deltaT > 0) {
    velocity_i = increment / deltaT;
  }
  prevT_i = currT;
}

// =================================================================
// CONTROL DEL MOTOR (DRIVER L298N)
// =================================================================
void setMotor(int dir, int pwmVal, int pwm_pin, int in1_pin, int in2_pin) {
  analogWrite(pwm_pin, pwmVal);
  if (dir == 1) {
    digitalWrite(in1_pin, LOW);
    digitalWrite(in2_pin, HIGH);
  } else if (dir == -1) {
    digitalWrite(in1_pin, HIGH);
    digitalWrite(in2_pin, LOW);
  } else {
    digitalWrite(in1_pin, LOW);
    digitalWrite(in2_pin, LOW);
  }
}

// =================================================================
// PARTE 1: LAZO ABIERTO
// =================================================================
void ejecutarParte1_LazoAbierto() {
  Serial.println(F("\n--- PARTE 1: LAZO ABIERTO ---"));
  Serial.println(F("Ingrese el valor de PWM (0-255) o 'stop' para salir:"));

  bool active = true;
  int pwm_val = 100;
  int dir = 1;

  while (active) {
    if (Serial.available() > 0) {
      String input = Serial.readStringUntil('\n');
      input.trim();

      if (input.equalsIgnoreCase("stop")) {
        active = false;
      } else {
        int tempVal = input.toInt();
        if (tempVal < 0) {
          dir = -1;
          pwm_val = constrain(abs(tempVal), 0, 255);
        } else {
          dir = 1;
          pwm_val = constrain(tempVal, 0, 255);
        }
        Serial.print(F("Aplicando PWM: "));
        Serial.print(pwm_val);
        Serial.print(F(" | Direccion: "));
        Serial.println(dir);
      }
    }

    setMotor(dir, pwm_val, PWM, IN1, IN2);
    delay(20);
  }

  setMotor(0, 0, PWM, IN1, IN2);
  Serial.println(F("Prueba finalizada."));
  showMenu();
}

// =================================================================
// PARTE 2: MEDICIÓN DE VELOCIDAD (MÉTODO 1 vs MÉTODO 2) - INTERACTIVO
// =================================================================
void ejecutarParte2_MedicionVelocidad() {
  Serial.println(F("\n--- PARTE 2: MEDICION DE VELOCIDAD (RPM) ---"));
  Serial.println(F("Comandos disponibles mientras se grafica:"));
  Serial.println(F("  pwm=<0-255>  -> Cambiar valor de PWM (Ej: pwm=180)"));
  Serial.println(F("  dir=<1 o -1> -> Cambiar sentido de giro (Ej: dir=-1)"));
  Serial.println(F("  stop         -> Detener motor y volver al menú"));
  delay(1000);

  int pwm_val = 150; // PWM inicial por defecto
  int dir = 1;       // Dirección inicial (1: Horario, -1: Antihorario)

  setMotor(dir, pwm_val, PWM, IN1, IN2); // Arrancar motor
  prevT = micros();

  bool active = true;
  while (active) {
    // Lectura de comandos en tiempo real por el Puerto Serial
    if (Serial.available() > 0) {
      String cmd = Serial.readStringUntil('\n');
      cmd.trim();

      if (cmd.equalsIgnoreCase("stop")) {
        active = false;
      } else if (cmd.startsWith("pwm=")) {
        pwm_val = constrain(cmd.substring(4).toInt(), 0, 255);
        setMotor(dir, pwm_val, PWM, IN1, IN2);
      } else if (cmd.startsWith("dir=")) {
        int tempDir = cmd.substring(4).toInt();
        if (tempDir == 1 || tempDir == -1) {
          dir = tempDir;
          setMotor(dir, pwm_val, PWM, IN1, IN2);
        }
      }
    }

    // 1. Lectura atómica de variables de la interrupción
    int pos = 0;
    float velocity2_counts = 0;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
      pos = pos_i;
      velocity2_counts = velocity_i;
    }

    // 2. Método 1: Diferencia de posición sobre tiempo
    long currT = micros();
    float deltaT = ((float)(currT - prevT)) / 1.0e6;
    if (deltaT <= 0) deltaT = 1.0e-3; // Prevenir división por cero

    float velocity1_counts = (pos - posPrev) / deltaT;
    posPrev = pos;
    prevT = currT;

    // 3. Conversión de cuentas/segundo a RPM
    float rpm_m1 = (velocity1_counts / COUNTS_PER_REV) * 60.0;
    float rpm_m2 = (velocity2_counts / COUNTS_PER_REV) * 60.0;

    // 4. Salida para el Serial Plotter
    Serial.print("Metodo1_RPM:");
    Serial.print(rpm_m1);
    Serial.print(" Metodo2_RPM:");
    Serial.println(rpm_m2);

    delay(10);
  }

  // Apagar motor al salir
  setMotor(0, 0, PWM, IN1, IN2);
  clear_serial_buffer();
  showMenu();
}

// =================================================================
// PARTE 3: FILTRADO PASO BAJO - INTERACTIVO
// =================================================================
void ejecutarParte3_FiltradoSenal() {
  Serial.println(F("\n--- PARTE 3: FILTRADO PASO BAJO ---"));
  Serial.println(F("Comandos disponibles mientras se grafica:"));
  Serial.println(F("  pwm=<0-255>  -> Cambiar valor de PWM (Ej: pwm=180)"));
  Serial.println(F("  dir=<1 o -1> -> Cambiar sentido de giro (Ej: dir=-1)"));
  Serial.println(F("  stop         -> Detener motor y volver al menú"));
  delay(1000);

  int pwm_val = 150; // Valor PWM inicial
  int dir = 1;       // Dirección inicial

  setMotor(dir, pwm_val, PWM, IN1, IN2);
  prevT = micros();
  v1Filt = 0;
  v1Prev = 0;

  bool active = true;
  while (active) {
    // Lectura de comandos en tiempo real por el Puerto Serial
    if (Serial.available() > 0) {
      String cmd = Serial.readStringUntil('\n');
      cmd.trim();

      if (cmd.equalsIgnoreCase("stop")) {
        active = false;
      } else if (cmd.startsWith("pwm=")) {
        pwm_val = constrain(cmd.substring(4).toInt(), 0, 255);
        setMotor(dir, pwm_val, PWM, IN1, IN2);
      } else if (cmd.startsWith("dir=")) {
        int tempDir = cmd.substring(4).toInt();
        if (tempDir == 1 || tempDir == -1) {
          dir = tempDir;
          setMotor(dir, pwm_val, PWM, IN1, IN2);
        }
      }
    }

    // 1. Obtención de la posición atómicamente
    int pos = 0;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
      pos = pos_i;
    }

    // 2. Cálculo de velocidad sin filtrar (RPM)
    long currT = micros();
    float deltaT = ((float)(currT - prevT)) / 1.0e6;
    if (deltaT <= 0) deltaT = 1.0e-3; // Evitar división por cero

    float velocity1 = (pos - posPrev) / deltaT;
    posPrev = pos;
    prevT = currT;

    float rpm_raw = (velocity1 / COUNTS_PER_REV) * 60.0;

    // 3. Aplicación del Filtro Paso Bajo (25 Hz)
    v1Filt = 0.854 * v1Filt + 0.0728 * rpm_raw + 0.0728 * v1Prev;
    v1Prev = rpm_raw;

    // 4. Salida para el Serial Plotter (Comparación en tiempo real)
    Serial.print("RPM_SinFiltrar:");
    Serial.print(rpm_raw);
    Serial.print(" RPM_Filtrada:");
    Serial.println(v1Filt);

    delay(10);
  }

  // Apagar el motor al salir
  setMotor(0, 0, PWM, IN1, IN2);
  clear_serial_buffer();
  showMenu();
}

// =================================================================
// PARTE 4: CONTROL PID DE VELOCIDAD EN LAZO CERRADO
// =================================================================
void ejecutarParte4_ControlPIDVelocidad() {
  Serial.println(F("\n--- PARTE 4: CONTROL PID DE VELOCIDAD ---"));
  Serial.println(F("Comandos disponibles:"));
  Serial.println(F("  vt=<valor>  -> Establecer RPM deseada"));
  Serial.println(F("  kp=<valor>  -> Ajustar constante Proporcional"));
  Serial.println(F("  ki=<valor>  -> Ajustar constante Integral"));
  Serial.println(F("  kd=<valor>  -> Ajustar constante Derivativa"));
  Serial.println(F("  stop        -> Detener y volver al menú"));

  eintegral = 0;
  eprev = 0;
  v1Filt = 0;
  v1Prev = 0;
  prevT = micros();

  bool active = true;
  while (active) {
    if (Serial.available() > 0) {
      String cmd = Serial.readStringUntil('\n');
      cmd.trim();

      if (cmd.equalsIgnoreCase("stop")) {
        active = false;
      } else if (cmd.startsWith("vt=")) {
        vt_target = cmd.substring(3).toFloat();
        Serial.print("vt = "); Serial.println(vt_target);
      } else if (cmd.startsWith("kp=")) {
        kp = cmd.substring(3).toFloat();
        Serial.print("kp = "); Serial.println(kp);
      } else if (cmd.startsWith("ki=")) {
        ki = cmd.substring(3).toFloat();
        Serial.print("ki = "); Serial.println(ki);
      } else if (cmd.startsWith("kd=")) {
        kd = cmd.substring(3).toFloat();
        Serial.print("kd = "); Serial.println(kd);
      }
    }

    // 1. Obtener posición atómicamente
    int pos = 0;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
      pos = pos_i;
    }

    // 2. Calcular velocidad cruda (RPM)
    long currT = micros();
    float deltaT = ((float)(currT - prevT)) / 1.0e6;
    if (deltaT <= 0) deltaT = 1.0e-3; // Prevenir división por cero

    float velocity1 = (pos - posPrev) / deltaT;
    posPrev = pos;
    prevT = currT;

    float rpm_raw = (velocity1 / COUNTS_PER_REV) * 60.0;

    // 3. Filtrar medición de velocidad
    v1Filt = 0.854 * v1Filt + 0.0728 * rpm_raw + 0.0728 * v1Prev;
    v1Prev = rpm_raw;

    // 4. Calcular Algoritmo PID
    float e = vt_target - v1Filt;
    eintegral += e * deltaT;
    float dedt = (e - eprev) / deltaT;
    eprev = e;

    float u = kp * e + ki * eintegral + kd * dedt;

    // 5. Determinar dirección y saturación
    int dir = 1;
    if (u < 0) {
      dir = -1;
    }
    int pwr = (int)fabs(u);
    if (pwr > 255) {
      pwr = 255;
    }

    setMotor(dir, pwr, PWM, IN1, IN2);

    // 6. Graficar en Serial Plotter
    Serial.print("Target_RPM:");
    Serial.print(vt_target);
    Serial.print(" RPM_Filtrada:");
    Serial.println(v1Filt);

    delay(10);
  }

  setMotor(0, 0, PWM, IN1, IN2);
  clear_serial_buffer();
  showMenu();
}