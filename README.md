# Función de Transferencia de un Motor de CD (Posición vs Voltaje)

Para obtener la función de transferencia de un motor de corriente directa (CD) que relaciona la **posición angular de la armadura ($\theta$)** con el **voltaje de entrada ($V$)**, debemos analizar las ecuaciones eléctricas y mecánicas del sistema.

Aquí tienes el desarrollo paso a paso utilizando el modelo lineal clásico de un motor controlado por armadura.

---

## 1. Variables y Parámetros del Sistema

* $V(s)$: Voltaje de entrada en la armadura.
* $\Theta(s)$: Posición angular del eje del motor.
* $I(s)$: Corriente de la armadura.
* $R$: Resistencia de la armadura.
* $L$: Inductancia de la armadura.
* $K_e$: Constante de fuerza electromotriz (f.e.m. inversa).
* $K_t$: Constante de torque del motor.
* $J$: Momento de inercia del rotor y la carga.
* $b$: Coeficiente de fricción viscosa del motor.

---

## 2. Ecuaciones Fundamentales (Dominio del Tiempo)

1. **Circuito Eléctrico:** El voltaje de entrada se consume en la resistencia, la inductancia y la f.e.m. inversa ($e_b = K_e \frac{d\theta}{dt}$):
   $$v(t) = R i(t) + L \frac{di(t)}{dt} + K_e \frac{d\theta(t)}{dt}$$

2. **Balance Mecánico:** El torque generado por el motor ($T_m = K_t i$) acelera la inercia y vence la fricción:
   $$T_m(t) = K_t i(t) = J \frac{d^2\theta(t)}{dt^2} + b \frac{d\theta(t)}{dt}$$

---

## 3. Transformada de Laplace (Condiciones iniciales cero)

Aplicando la transformada de Laplace a ambas ecuaciones, pasamos al dominio de la frecuencia compleja ($s$):

1. **Ecuación Eléctrica:**
   $$V(s) = (R + Ls)I(s) + K_e s\Theta(s)$$

2. **Ecuación Mecánica:**
   $$K_t I(s) = (Js^2 + bs)\Theta(s) = s(Js + b)\Theta(s)$$

---

## 4. Obteniendo la Función de Transferencia

Para encontrar $G(s) = \frac{\Theta(s)}{V(s)}$, primero despejamos la corriente $I(s)$ de la ecuación mecánica:

$$I(s) = \frac{s(Js + b)}{K_t}\Theta(s)$$

Ahora, sustituimos este valor de $I(s)$ en la ecuación eléctrica:

$$V(s) = (R + Ls)\left[ \frac{s(Js + b)}{K_t}\Theta(s) \right] + K_e s\Theta(s)$$

Factorizamos $s\Theta(s)$ en el lado derecho:

$$V(s) = s \left[ \frac{(R + Ls)(Js + b) + K_t K_e}{K_t} \right] \Theta(s)$$

Finalmente, reordenando los términos para dejar la relación Posición/Voltaje, obtenemos la **función de transferencia general**:

> $$G(s) = \frac{\Theta(s)}{V(s)} = \frac{K_t}{s \left[ (Ls + R)(Js + b) + K_t K_e \right]}$$

Si desarrollamos el denominador, la ecuación queda de la siguiente forma de tercer orden:

$$G(s) = \frac{K_t}{s \left[ LJs^2 + (RJ + Lb)s + (Rb + K_t K_e) \right]}$$

---
## 5. Sección de preguntas
### Encoder & sensing
- What is the fundamental physical principle behind a magnetic incremental encoder?
- How does it distinguish clockwise from counterclockwise rotation?
- Why must encoder outputs be connected to interrupt-capable pins (pins 2 and 3 on Arduino Uno), rather than any digital pin?
- Why is the volatile keyword required for the posi variable, and what exact failure can occur without it?
- What is the purpose of ATOMIC_BLOCK(ATOMIC_RESTORESTATE)?
- What race condition does it prevent, and why is volatile alone insufficient?
- The encoder has a gear reduction ratio of 1:45 and outputs 12 PPR per loop.
- How many encoder counts correspond to one full revolution of the output shaft?
- How does this affect position resolution?

### Motor driving
- Explain the relationship between duty cycle and average output voltage in PWM. If the supply is 12 V and the duty cycle is 60%, what is the average motor voltage?
- Describe the H-Bridge switching logic. Which switch pairs must activate to reverse motor direction, and why does activating all four simultaneously cause a short circuit?
- The L298N introduces approximately a 2 V drop. How does this affect the maximum achievable speed when using a 12 V supply? What are the implications for the control signal range?
- Why is a DC motor considered an integral plant (type 1 system) in control terms? What does this imply about open-loop position stability?

### Feedback loop
- Draw and explain the block diagram of the closed-loop position control system used in this guide. Identify the plant, sensor, controller, and actuator.
- What is the error signal e(t) in this system? What are the consequences of defining it as pos - target versus target - pos on the sign convention of the control output?
- Why does this system use a sinusoidal reference target = 250*sin(prevT/1e6) instead of a step input for testing? What does it reveal about system performance that a step cannot?

### PID theory
- State the PID control law m(t) = Kp·e(t) + Ki·∫e(t)dt + Kd·de/dt. What is the physical interpretation of each term and what aspect of system behavior does each correct?
- The guide approximates the integral as eintegral += e * deltaT. What numerical integration method is this? What are its error characteristics compared to trapezoidal integration?
- What is integrator windup? Under what conditions does it occur in this implementation, and what strategies could be added to prevent it?
- The derivative term uses (e - eprev) / deltaT. Why is derivative action sensitive to noise? What is "derivative kick" and how can it be mitigated?
- In the code, Ki is set to 0. With only proportional and derivative action (PD control), can the system achieve zero steady-state error for a constant reference? Justify your answer analytically.
### Tuning & performance
- Describe the manual tuning procedure starting with Kp alone. What observable behavior tells you Kp is too low, adequate, or too high?
- Define settling time, overshoot, and steady-state error. Given that the lab grades on minimizing all three simultaneously, explain the trade-offs involved in achieving this.
- How does increasing Kd affect overshoot and settling time? At what point does excessive Kd become detrimental to system stability?
- What is the effect of the gear reduction on the system dynamics from the controller's perspective — specifically on gain, inertia reflected to the motor shaft, and bandwidth?
### Implementation
- The sample time deltaT is computed as (currT - prevT) / 1e6. Why is variable sample time used instead of a fixed timer interrupt? What problems can arise from non-uniform sampling?
- The PWM output is clamped to [0, 255] and direction is determined by the sign of u. What is the effect of this nonlinearity (saturation) on the integral term, and how does it interact with windup?
- If you wanted to accept a target position via serial input from the keyboard (the optional deliverable), what modifications to the code structure would be required? What parsing and safety considerations apply?
- The Serial.println() calls inside the main loop add latency. How does this affect the effective sampling rate and control loop timing? How would you measure and mitigate this?

## 6. Aproximación Práctica (Simplificación)

En la gran mayoría de los motores de CD prácticos, la inductancia de la armadura ($L$) es extremadamente pequeña comparada con la resistencia ($R$) ($L \approx 0$). Si despreciamos $L$, la función de transferencia se simplifica a un sistema de **segundo orden**:

$$G(s) = \frac{K_t}{s \left[ R(Js + b) + K_t K_e \right]} = \frac{K_t}{s \left[ RJs + (Rb + K_t K_e) \right]}$$

Dividiendo todo entre $(Rb + K_t K_e)$ para llevarlo a la forma estándar:

$$G(s) = \frac{K_m}{s(\tau_m s + 1)}$$

Donde:
* **$K_m = \frac{K_t}{Rb + K_t K_e}$** es la ganancia del motor.
* **$\tau_m = \frac{RJ}{Rb + K_t K_e}$** es la constante de tiempo mecánica del motor.

> **Nota:** La presencia de la $s$ solitaria en el denominador ($s \cdot [...]$) actúa como un **integrador puro**. Esto significa que si aplicas un voltaje constante, el motor girará a una *velocidad* constante, lo que hace que la *posición* $(\theta)$ crezca linealmente de forma indefinita.
