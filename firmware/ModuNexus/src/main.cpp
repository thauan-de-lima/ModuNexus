#include <Arduino.h>
#include <WiFi.h>
#include <ModbusIP_ESP8266.h>
#include "secrets.h"

const char* SSID       = WIFI_SSID;
const char* PASSWORD   = WIFI_PASSWORD;
IPAddress   CODESYS_IP(CODESYS_IP_1, CODESYS_IP_2, CODESYS_IP_3, CODESYS_IP_4);
const int   MODBUS_PORT = 502;

const int PIN_SENSOR = 7;  // sensor IR → GPIO7
const int PIN_IN1    = 4;  // L298N IN1 → GPIO4
const int PIN_IN2    = 5;  // L298N IN2 → GPIO5
const int PIN_ENA    = 6;  // L298N ENA → GPIO6

ModbusIP mb;

bool     sensorState   = false;
bool     conveyorState = false;
uint16_t conveyorReg   = 0;

void setup() {
    Serial.begin(115200);

    pinMode(PIN_SENSOR, INPUT_PULLUP);
    pinMode(PIN_IN1,    OUTPUT);
    pinMode(PIN_IN2,    OUTPUT);
    pinMode(PIN_ENA,    OUTPUT);

    digitalWrite(PIN_IN2, LOW);   // IN2 sempre LOW
    digitalWrite(PIN_ENA, HIGH);  // ENA sempre HIGH — motor habilitado

    WiFi.begin(SSID, PASSWORD);
    Serial.print("Conectando ao Wi-Fi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWi-Fi conectado! IP: " + WiFi.localIP().toString());

    mb.client();
    mb.autoConnect(true);
    mb.connect(CODESYS_IP, MODBUS_PORT);
}

void loop() {
    mb.task();

    if (!mb.isConnected(CODESYS_IP)) {
        Serial.println("Aguardando conexão Modbus...");
        mb.connect(CODESYS_IP, MODBUS_PORT);
        delay(500);
        return;
    }

    // Lê sensor IR físico
    sensorState = (digitalRead(PIN_SENSOR) == LOW);

    // Envia sensor ao CODESYS (Coil 0)
    mb.writeCoil(CODESYS_IP, 0, sensorState);

    mb.task();
    delay(50);

    // Lê ESTEIRA do CODESYS (Input Register 0)
    mb.readIreg(CODESYS_IP, 0, &conveyorReg);

    mb.task();
    delay(50);

    conveyorState = (conveyorReg & 0x0001);

    // Aciona motor via IN1
    digitalWrite(PIN_IN1, conveyorState ? HIGH : LOW);

    Serial.printf("Sensor: %d | ConveyorReg: %d | Conveyor: %d\n",
                  sensorState, conveyorReg, conveyorState);
}