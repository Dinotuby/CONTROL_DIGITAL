#include <ArduinoRS485.h>          // Librería para RS485 (necesaria para Modbus)
#include <ArduinoModbus.h>         // Librería para Modbus RTU
#include "Controllino.h"           // Define pines industriales (CONTROLLINO_D0, etc.)

// ========== DEFINICIÓN DE PINES ==========
const int led1Pin = CONTROLLINO_D0;   // Primer LED (Do del tablero)
const int led2Pin = CONTROLLINO_D1;   // Segundo LED (cámbialo si tu tablero usa otro pin PWM)
const int btn1Pin = CONTROLLINO_I16;  // Botón para habilitar/deshabilitar LED1
const int btn2Pin = CONTROLLINO_I17;  // Botón para habilitar/deshabilitar LED2
// ========== VARIABLES DE ESTADO ==========
bool led1Enabled = true;              // true = el LED1 obedece a la HMI, false = apagado por botón
bool led2Enabled = true;              // igual para LED2

uint16_t hmiDuty1 = 0;                // Último duty cycle leído de la HMI para LED1 (0-100) (puedo cambiar si esta apagado)
uint16_t hmiDuty2 = 0;                // Último duty cycle leído de la HMI para LED2 (0-100)

// Variables para detección de flanco de botones (evita múltiples disparos)
int lastBtn1 = HIGH;                  // Estado anterior del botón1 (HIGH porque tienen pull-up)
int lastBtn2 = HIGH;                  // Estado anterior del botón2

void setup() {
  Serial.begin(19200);                // Monitor serial para depuración (opcional)

  // ---------- Configuración Modbus ----------
    if (!ModbusRTUServer.begin(1, 19200)) {while (1); }   // Si falla, se detiene aquí                              // Si falla, se queda aquí (único posible bloqueo)
  
  ModbusRTUServer.configureHoldingRegisters(0x00, 2); // Dos registros: offset 0 y 1 (40001 y 40002)
  ModbusRTUServer.holdingRegisterWrite(0, 25);  // Valor inicial LED1 = 0%
  ModbusRTUServer.holdingRegisterWrite(1, 25);  // Valor inicial LED2 = 0%

  // ---------- Configuración de pines ----------
  pinMode(led1Pin, OUTPUT);
  pinMode(led2Pin, OUTPUT);
  pinMode(btn1Pin, INPUT_PULLUP);      // Pull-up interno, botón conecta a GND al presionar
  pinMode(btn2Pin, INPUT_PULLUP);
  //Iniciamos con el sistema apagado
  analogWrite(led1Pin, 0);             // Apagar LED1 al inicio
  analogWrite(led2Pin, 0);             // Apagar LED2 al inicio

}

void loop() {
  // 1. Atender comunicación Modbus (permite recibir y enviar datos)
  ModbusRTUServer.poll();   // No bloquea, revisa si hay trama nueva y la procesa

  // 2. Leer botones y aplicar prioridad (lógica no bloqueante)
  int btn1 = digitalRead(btn1Pin);      // Lee estado actual del botón1
  if (btn1 == LOW && lastBtn1 == HIGH) { // Flanco descendente (se acaba de presionar)
    led1Enabled = !led1Enabled;          // Alterna estado (true <-> false)
    if (!led1Enabled) {analogWrite(led1Pin, 0);           // Apaga LED1 inmediatamente si es false  
    } else {actualizarLED(led1Pin, hmiDuty1);} } // Restaura el brillo actual desde HMI    
  lastBtn1 = btn1;                       // Guarda estado actual para próxima comparación

  int btn2 = digitalRead(btn2Pin);
  if (btn2 == LOW && lastBtn2 == HIGH) {led2Enabled = !led2Enabled;
    if (!led2Enabled) {analogWrite(led2Pin, 0);
    } else {actualizarLED(led2Pin, hmiDuty2);} }
  lastBtn2 = btn2;

  // 3. Leer valores desde la HMI (Modbus) y actualizar LEDs si es necesario
  uint16_t newDuty1 = ModbusRTUServer.holdingRegisterRead(0);  // Lee offset 0 (40001)
  if (newDuty1 != hmiDuty1) {           // Solo actúa si cambió el valor
    hmiDuty1 = newDuty1;  // Limita a rango 0-100 (seguridad)
    if (led1Enabled) {actualizarLED(led1Pin, hmiDuty1);}} // Solo si el botón lo permite    

  uint16_t newDuty2 = ModbusRTUServer.holdingRegisterRead(1);  // Lee offset 1 (40002)
  if (newDuty2 != hmiDuty2) {
    hmiDuty2 = newDuty2;
    if (led2Enabled) {actualizarLED(led2Pin, hmiDuty2);} }
}

// Función auxiliar: escala de 0-100 a 0-255 y aplica PWM
void actualizarLED(int pin, uint16_t dutyPercent) {
  uint16_t pwmVal = map(dutyPercent, 0, 100, 0, 255);  // Conversión lineal
  analogWrite(pin, pwmVal);
}