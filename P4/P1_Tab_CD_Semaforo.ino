#include <Controllino.h>

// 1. ESTRUCTURA: Agrupamos los pines de cada semáforo 
struct Semaforo {
  int rojo;
  int amarillo;
  int verde;
};

// 2. ENUM: Nombres legibles para los estados de la FSM 
enum EstadosFSM {
  A_VERDE_B_ROJO,    // Semáforo A avanza, B espera (0)
  A_AMARILLO_B_ROJO, // Semáforo A advierte detención (1)
  A_ROJO_B_VERDE,    // Semáforo B avanza, A espera (2)
  A_ROJO_B_AMARILLO  // Semáforo B advierte detención (3)
};

// Inicialización de los objetos usando la estructura(es decir creamos los dos semaforos)
// Semáforo A: D0, D1, D2 | Semáforo B: D6, D7, D8
Semaforo semA = {CONTROLLINO_D0, CONTROLLINO_D1, CONTROLLINO_D2};
Semaforo semB = {CONTROLLINO_D6, CONTROLLINO_D7, CONTROLLINO_D8};

// Variables de control de la FSM
EstadosFSM estadoActual = A_VERDE_B_ROJO;  //Estado inicial de nuesta maquina
unsigned long tiempoPrevioFSM = 0;
unsigned long duracionEstado = 5000; // Iniciamos con 5 segundos

void setup() {
  // Configuración de salidas usando la estructura
  pinMode(semA.rojo, OUTPUT); pinMode(semA.amarillo, OUTPUT); pinMode(semA.verde, OUTPUT);
  pinMode(semB.rojo, OUTPUT); pinMode(semB.amarillo, OUTPUT); pinMode(semB.verde, OUTPUT);
}

void loop() {
  unsigned long tiempoActual = millis(); // Retardo no bloqueante

  // LÓGICA DE LA MÁQUINA DE ESTADOS (FSM)
  if (tiempoActual - tiempoPrevioFSM >= duracionEstado) {
    tiempoPrevioFSM = tiempoActual; // Reset del cronómetro para el siguiente estado

    switch (estadoActual) {
      
      case A_VERDE_B_ROJO: //(0)
        actualizarLuces(LOW, LOW, HIGH, HIGH, LOW, LOW); // A: Verde, B: Rojo
        estadoActual = A_AMARILLO_B_ROJO;               // Siguiente paso
        duracionEstado = 2000;                          // El amarillo dura menos (2s)
        break;

      case A_AMARILLO_B_ROJO: //(1)
        actualizarLuces(LOW, HIGH, LOW, HIGH, LOW, LOW); // A: Amar, B: Rojo
        estadoActual = A_ROJO_B_VERDE;
        duracionEstado = 5000;                          // El verde dura más (5s)
        break;

      case A_ROJO_B_VERDE:  //(2)
        actualizarLuces(HIGH, LOW, LOW, LOW, LOW, HIGH); // A: Rojo, B: Verde
        estadoActual = A_ROJO_B_AMARILLO;
        duracionEstado = 2000;
        break;

      case A_ROJO_B_AMARILLO:  //(3)
        actualizarLuces(HIGH, LOW, LOW, LOW, HIGH, LOW); // A: Rojo, B: Amar
        estadoActual = A_VERDE_B_ROJO;                  // Vuelve al inicio
        duracionEstado = 5000;
        break;
    }
  }
}

// FUNCIÓN AUXILIAR: Recibe los estados de los 6 pines para evitar repetir código
void actualizarLuces(int rA, int aA, int vA, int rB, int aB, int vB) {
  digitalWrite(semA.rojo, rA);
  digitalWrite(semA.amarillo, aA);
  digitalWrite(semA.verde, vA);
  
  digitalWrite(semB.rojo, rB);
  digitalWrite(semB.amarillo, aB);
  digitalWrite(semB.verde, vB);
}
