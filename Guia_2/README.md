# Función de Transferencia y Control de Velocidad de un Motor de CD (Velocidad vs Voltaje)

Para obtener la función de transferencia de un motor de corriente directa (CD) que relaciona la **velocidad angular del eje ($\Omega$)** con el **voltaje de entrada ($V$)**, debemos analizar las ecuaciones eléctricas y mecánicas del sistema en estado dinámico.

A continuación se presenta el desarrollo paso a paso utilizando el modelo lineal clásico de un motor controlado por armadura.

---

## 1. Variables y Parámetros del Sistema

* $V(s)$: Voltaje de entrada en la armadura.
* $\Omega(s)$: Velocidad angular del eje del motor ($\Omega(s) = s\Theta(s)$).
* $I(s)$: Corriente de la armadura.
* $R$: Resistencia de la armadura.
* $L$: Inductancia de la armadura.
* $K_e$: Constante de fuerza electromotriz (f.e.m. inversa).
* $K_t$: Constante de torque del motor.
* $J$: Momento de inercia del rotor y la carga.
* $b$: Coeficiente de fricción viscosa del motor.

---

## 2. Ecuaciones Fundamentales (Dominio del Tiempo)

1. **Circuito Eléctrico:** El voltaje de entrada se consume en la resistencia, la inductancia y la f.e.m. inversa proporcional a la velocidad angular ($e_b = K_e \omega(t)$):
   $$v(t) = R i(t) + L \frac{di(t)}{dt} + K_e \omega(t)$$

2. **Balance Mecánico:** El torque generado por el motor ($T_m = K_t i$) acelera la inercia del rotor y vence la fricción viscosa:
   $$T_m(t) = K_t i(t) = J \frac{d\omega(t)}{dt} + b \omega(t)$$

---

## 3. Transformada de Laplace (Condiciones iniciales cero)

Aplicando la transformada de Laplace a ambas ecuaciones, pasamos al dominio de la frecuencia compleja ($s$):

1. **Ecuación Eléctrica:**
   $$V(s) = (R + Ls)I(s) + K_e \Omega(s)$$

2. **Ecuación Mecánica:**
   $$K_t I(s) = (Js + b)\Omega(s)$$

---

## 4. Obteniendo la Función de Transferencia

Para encontrar $G_v(s) = \frac{\Omega(s)}{V(s)}$, despejamos la corriente $I(s)$ de la ecuación mecánica:

$$I(s) = \frac{Js + b}{K_t}\Omega(s)$$

Sustituimos $I(s)$ en la ecuación eléctrica:

$$V(s) = (R + Ls)\left[ \frac{Js + b}{K_t}\Omega(s) \right] + K_e \Omega(s)$$

Factorizamos $\Omega(s)$ en el lado derecho:

$$V(s) = \left[ \frac{(R + Ls)(Js + b) + K_t K_e}{K_t} \right] \Omega(s)$$

Reordenando los términos para dejar la relación Velocidad/Voltaje, obtenemos la **función de transferencia general**:

> $$G_v(s) = \frac{\Omega(s)}{V(s)} = \frac{K_t}{(Ls + R)(Js + b) + K_t K_e}$$

Desarrollando el denominador, obtenemos una ecuación cuadrática de segundo orden:

$$G_v(s) = \frac{K_t}{LJs^2 + (RJ + Lb)s + (Rb + K_t K_e)}$$

---

## 5. Sección de Preguntas

### Encoder & Speed Sensing
- What is the difference between frequency measurement (Method 1: $\Delta \text{pos}/\Delta t$) and period measurement (Method 2: $1/\Delta t_{\text{ISR}}$) for speed calculation?
- Why does Method 1 introduce quantization noise ("staircase effect") at low speeds, and why is Method 2 superior in that range?
- How does gear reduction and encoder resolution (PPR) directly impact the calculation of RPM?
- Why are `volatile` variables and `ATOMIC_BLOCK(ATOMIC_RESTORESTATE)` mandatory when sharing pulse timestamps between ISRs and the main execution loop?

### Open-Loop Speed Dynamics & Motor Driving
- What is deadband (dead zone) in DC motors, and why does the shaft fail to rotate at low PWM values in open loop?
- Why is speed control considered a Type 0 system, and how does its open-loop step response differ from position control (Type 1)?
- How does the internal voltage drop of the L298N driver limit the maximum achievable RPM under load?

### Signal Filtering & Processing
- Why does numerical differentiation of position to compute speed introduce high-frequency noise (>40 Hz)?
- Explain the discrete 1st-order Low-Pass Butterworth filter equation used in the lab: $y[n] = 0.854 y[n-1] + 0.0728 x[n] + 0.0728 x[n-1]$.
- Why must the sum of all filter coefficients equal 1, and what happens to steady-state gain if this condition is violated?
- How does selecting a cutoff frequency of 25 Hz balance high-frequency noise attenuation with control loop phase lag?

### PID Speed Control
- Draw and explain the block diagram of closed-loop speed control including the low-pass filter.
- Why is Integral action ($K_i$) strictly necessary to achieve zero steady-state speed error in a Type 0 system?
- Why is Derivative action ($K_d$) usually set to zero or very small values in speed control systems?
- What is integrator windup in speed control, and how does PWM output saturation $[0, 255]$ interact with it?

### Tuning & Performance
- Describe the manual tuning procedure ($K_p \rightarrow K_i \rightarrow K_d$) for speed tracking.
- How does applying an external mechanical load (friction) to the output shaft test the disturbance rejection capacity of $K_i$?
- How does filter lag affect the maximum usable $K_p$ before closed-loop instability occurs?

### Implementation
- Why is variable sample time ($\Delta t$) calculated using `micros()` instead of assuming a constant timer loop?
- How does live serial parsing (`vt=`, `kp=`, `ki=`, `kd=`) allow tuning without recompiling, and what safety checks prevent invalid memory or motor commands?

---

## 6. Aproximación Práctica (Simplificación a 1.er Orden)

En la mayoría de los motores de CD pequeños y medianos, la inductancia de la armadura ($L$) es despreciable frente a la resistencia ($R$) ($L \approx 0$). Despreciando $L$, la función de transferencia de velocidad se simplifica a un sistema de **primer orden**:

$$G_v(s) = \frac{K_t}{R(Js + b) + K_t K_e} = \frac{K_t}{RJs + (Rb + K_t K_e)}$$

Dividiendo entre el término constante $(Rb + K_t K_e)$ para llevarlo a la forma estándar de primer orden:

$$G_v(s) = \frac{K_m}{\tau_m s + 1}$$

Donde:
* **$K_m = \frac{K_t}{Rb + K_t K_e}$** es la ganancia de velocidad del motor ($\text{RPM/Voltio}$).
* **$\tau_m = \frac{RJ}{Rb + K_t K_e}$** es la constante de tiempo mecánica del motor.

> **Nota:** A diferencia del modelo de posición, el denominador **no tiene un integrador puro ($s$)**. Esto confirma que el sistema de velocidad es de **Tipo 0**, lo que significa que ante un voltaje de entrada constante, la velocidad alcanza un valor estable finito en estado estacionario.