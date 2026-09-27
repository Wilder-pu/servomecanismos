# Laboratorio de Control Digital: Motores de CD (Posición y Velocidad)

Este repositorio contiene las guías de laboratorio, modelos matemáticos, algoritmos de filtrado de señales y la implementación en tiempo real de sistemas de **control en lazo cerrado (PID)** para motores de corriente directa (CD) utilizando la plataforma Arduino, controladores Puente H (L298N) y encoders incrementales.

---

## 🎯 Objetivo General del Proyecto

Diseñar, analizar e implementar controladores digitales de **posición angular ($\Theta$)** y **velocidad angular ($\Omega$)** sobre un motor de CD con caja reductora y encoder magnético, comprendiendo la dinámica física del sistema, el impacto del ruido de medición, las técnicas de filtrado digital y la sintonización práctica del algoritmo PID.

---

## 📚 Estructura de las Guías

```mermaid

flowchart TD
    subgraph HW["SISTEMA ELECTROMECÁNICO"]
        direction LR
        ARD["[Arduino]"] --> DRV["[Driver L298N]"] --> MOT["[Motor DC GA25-370]"]
        MOT -.->|Feedback| ENC["[Encoder]"] -.-> ARD
    end

    HW --> G1
    HW --> G2

    subgraph G1["<b>GUÍA 1</b><br>Control de Posición Angular"]
        direction TB
        G1_D["• Variable: Posición θ(t)<br>• Sistema: Tipo 1 (Integrador)<br>• Modelo: 2.º Orden (L ≈ 0)<br>• Entrada: Perfil Sinusoidal<br>• Control: PD / PID"]
    end

    subgraph G2["<b>GUÍA 2</b><br>Control de Velocidad Angular"]
        direction TB
        G2_D["• Variable: Velocidad Ω(t)<br>• Sistema: Tipo 0 (Sin integrador)<br>• Modelo: 1.er Orden (L ≈ 0)<br>• Proceso: Filtrado PB (25 Hz)<br>• Control: PI / PID"]
    end
    
---

### 📌 Guía 1: Control de Posición Angular ($\Theta$)
Se aborda el seguimiento de trayectoria de posición del eje de salida del motor.

* **Fundamento:** La posición es la integral de la velocidad. Al tener un integrador puro en la planta, el sistema es de **Tipo 1** y de **2.º Orden**.
* **Puntos Clave:**
  * Lectura de cuadratura del encoder mediante interrupciones de hardware (`RISING`/`CHANGE`).
  * Prevención de condiciones de carrera mediante bloques atómicos (`ATOMIC_BLOCK`).
  * Seguimiento de trayectorias sinusoidales y escalón.
  * Sintonización de control PD/PID para minimizar sobrepico y tiempo de asentamiento.

---

### 📌 Guía 2: Control de Velocidad Angular ($\Omega$)
Se analiza el comportamiento dinámico de la velocidad y la necesidad de acondicionar la señal leída por el encoder antes de aplicarla al lazo de control.

* **Fundamento:** El sistema de velocidad no posee integrador natural, clasificándose como un sistema de **Tipo 0** y de **1.er Orden**.
* **Puntos Clave:**
  * **Métodos de Medición:** Comparación entre el Método 1 (conteo por intervalo fijo) y el Método 2 (tiempo entre pulsos por interrupción).
  * **Filtrado Digital:** Diseño e implementación de un filtro paso bajo Butterworth de 1.er orden a 25 Hz para eliminar el ruido de cuantización de alta frecuencia.
  * **Control Lazo Cerrado:** Necesidad crítica de la acción Integral ($K_i$) para eliminar el error en estado estable producido por fricción y cargas mecánicas.

---

## 📊 Comparativa Técnica: Posición vs. Velocidad

| Parámetro / Propiedad | Guía 1: Posición Angular ($\Theta$) | Guía 2: Velocidad Angular ($\Omega$) |
| :--- | :--- | :--- |
| **Variable a Controlar** | Ángulo / Vueltas del eje | Revoluciones Por Minuto (RPM) |
| **Tipo de Sistema** | **Tipo 1** (Posee un integrador puro $\frac{1}{s}$) | **Tipo 0** (Sin integradores puros) |
| **Orden de la Planta ($L \approx 0$)** | **2.º Orden**: $G(s) = \frac{K_m}{s(\tau_m s + 1)}$ | **1.er Orden**: $G_v(s) = \frac{K_m}{\tau_m s + 1}$ |
| **Respuesta en Lazo Abierto** | Crece indefinidamente ante un voltaje fijo | Se estabiliza en una velocidad constante |
| **Procesamiento de Señal** | Conteo directo de pulsos del encoder | Derivación + Filtro Paso Bajo Digital (25 Hz) |
| **Acción $K_i$ en el PID** | Opcional (el sistema elimina error por sí solo) | **Mandatoria** (necesaria para error cero) |

---

## 🛠️ Requisitos de Hardware y Conexiones

* **Microcontrolador:** Arduino Uno / Nano / Mega.
* **Actuador:** Motor de CD GA25-370 (12V DC, 100 RPM nominales con reductora).
* **Driver de Potencia:** L298N o equivalente Puente H.
* **Sensor:** Encoder Incremental Integrado (11 PPR base en motor, ~625 cuentas/vuelta en eje de salida).

### Diagrama de Pines General (Arduino)
* `Pin 2 (INT0)` $\rightarrow$ Canal A del Encoder (Interrupción).
* `Pin 3 (INT1)` $\rightarrow$ Canal B del Encoder.
* `Pin 5 (PWM)`  $\rightarrow$ ENA / IN1 (Control de velocidad PWM).
* `Pin 6`        $\rightarrow$ IN1 (Dirección 1).
* `Pin 7`        $\rightarrow$ IN2 (Dirección 2).

---

## 💻 Interfaz de Comando Serial (Tiempo Real)

Ambos códigos incluyen un analizador sintáctico por puerto serial a 115200 baudios que permite modificar los parámetros del sistema durante la ejecución sin necesidad de re-compilar:

* `vt=<valor>` : Asigna la velocidad o posición objetivo (Ej: `vt=120`).
* `kp=<valor>` : Ajusta la ganancia proporcional (Ej: `kp=3.5`).
* `ki=<valor>` : Ajusta la ganancia integral (Ej: `ki=12.0`).
* `kd=<valor>` : Ajusta la ganancia derivativa (Ej: `kd=0.05`).
* `stop`       : Detiene de inmediato el motor por seguridad.