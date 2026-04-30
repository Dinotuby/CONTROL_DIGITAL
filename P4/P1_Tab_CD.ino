#include <Controllino.h>        // Librería para usar los pines industriales del Controllino

const uint8_t espiral[] = {     // Arreglo que define el orden de encendido (patrón espiral)
  CONTROLLINO_D0, CONTROLLINO_D6, CONTROLLINO_D12, 
  CONTROLLINO_D13, CONTROLLINO_D14, CONTROLLINO_D8, 
  CONTROLLINO_D2, CONTROLLINO_D1, CONTROLLINO_D7
};

const uint8_t* ptrSecuencia = espiral;  // Puntero al arreglo (acceso por aritmética de punteros)
int8_t indiceActual = 0;                 // Índice actual del LED en la secuencia
int8_t indiceAnterior = 0;               // MEMORIA: Guarda qué LED apagar sin usar un bucle for
int8_t direccion = 0;                    // Dirección: 1=normal, -1=inverso, 0=detenido
unsigned long tiempoPrevio = 0;          // Guarda el tiempo de la última actualización
const long intervalo = 250;              // Tiempo entre cambios (250 ms)

void setup() {                           // Se ejecuta una sola vez al inicio
  for (int i = 0; i < 9; i++) { pinMode(espiral[i], OUTPUT); } // Setup sí puede usar for
  pinMode(CONTROLLINO_I16, INPUT);      // Botón 1: activa secuencia normal
  pinMode(CONTROLLINO_I17, INPUT);      // Botón 2: activa secuencia inversa
  pinMode(CONTROLLINO_I18, INPUT);      // Botón 3: reset/apagado
}

void loop() {                            // Se ejecuta continuamente (Bucle principal)

  unsigned long tiempoActual = millis(); // Obtenemos tiempo actual del sistema

  // --- MONITOREO DE BOTONES ---
  if (digitalRead(CONTROLLINO_I16) == HIGH) { direccion = 1; }
  else if (digitalRead(CONTROLLINO_I17) == HIGH) { direccion = -1; }
  else if (digitalRead(CONTROLLINO_I18) == HIGH) { 
    direccion = 0; 
    digitalWrite(*(ptrSecuencia + indiceActual), LOW); // Apaga el LED actual por dirección de memoria
    indiceActual = 0; 
    indiceAnterior = 0;
  }
  
  // --- LÓGICA DE TIEMPO NO BLOQUEANTE ---
  if (direccion != 0 && (tiempoActual - tiempoPrevio >= intervalo)) { 
    tiempoPrevio = tiempoActual;           // "Reseteamos" el cronómetro anotando la hora actual

    // LA NUEVA LÓGICA SIN FOR:
    digitalWrite(*(ptrSecuencia + indiceAnterior), LOW); // Apaga el LED de la iteración pasada
    digitalWrite(*(ptrSecuencia + indiceActual), HIGH);  // Enciende el LED de la iteración nueva
    
    indiceAnterior = indiceActual;         // El que acabamos de prender será el "viejo" en la próxima vuelta

    // --- CÁLCULO DE SIGUIENTE POSICIÓN ---
    indiceActual += direccion;             // Suma 1 o resta 1 según la dirección

    if (indiceActual > 8) indiceActual = 0; // Límite superior: vuelve al inicio
    if (indiceActual < 0) indiceActual = 8; // Límite inferior: vuelve al final
  }
}
