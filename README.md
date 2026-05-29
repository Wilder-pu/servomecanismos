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

## 6. Aproximación Práctica (Simplificación)

En la gran mayoría de los motores de CD prácticos, la inductancia de la armadura ($L$) es extremadamente pequeña comparada con la resistencia ($R$) ($L \approx 0$). Si despreciamos $L$, la función de transferencia se simplifica a un sistema de **segundo orden**:

$$G(s) = \frac{K_t}{s \left[ R(Js + b) + K_t K_e \right]} = \frac{K_t}{s \left[ RJs + (Rb + K_t K_e) \right]}$$

Dividiendo todo entre $(Rb + K_t K_e)$ para llevarlo a la forma estándar:

$$G(s) = \frac{K_m}{s(\tau_m s + 1)}$$

Donde:
* **$K_m = \frac{K_t}{Rb + K_t K_e}$** es la ganancia del motor.
* **$\tau_m = \frac{RJ}{Rb + K_t K_e}$** es la constante de tiempo mecánica del motor.

> **Nota:** La presencia de la $s$ solitaria en el denominador ($s \cdot [...]$) actúa como un **integrador puro**. Esto significa que si aplicas un voltaje constante, el motor girará a una *velocidad* constante, lo que hace que la *posición* $(\theta)$ crezca linealmente de forma indefinita.
