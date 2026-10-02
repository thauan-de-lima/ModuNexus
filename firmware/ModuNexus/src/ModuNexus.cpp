// ─── ModuNexus — Módulo NEXUS (Principal) ────────────────────────────────────
// ESP32 30 pinos + Shield W5500
// IP: 192.168.1.20 | MAC: DE:AD:BE:EF:FE:E0
// Modbus TCP Server — CODESYS conecta nele
//
// Registros Modbus expostos ao CODESYS:
//   TODO: definir mapa de registradores conforme hardware final
// ─────────────────────────────────────────────────────────────────────────────

#include <SPI.h>
#include <Ethernet.h>
#include <ModbusEthernet.h>
#include "esp_system.h"

// ─── W5500 ───────────────────────────────────────────────
#define W5500_SCK  18
#define W5500_MISO 19
#define W5500_MOSI 23
#define W5500_CS    5
#define W5500_RST   4

// ─── Pinos ───────────────────────────────────────────────
// TODO: definir pinos reais dos relés e periféricos
const int PIN_LED_VERDE    = 2;
const int PIN_LED_VERMELHO = 15;
// Ex: const int RELE_1 = 25;

// ─── Rede ────────────────────────────────────────────────
byte      mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xE0 };
IPAddress ip(192, 168, 1, 20);

ModbusEthernet mb;

// ─── Watchdog W5500 ──────────────────────────────────────
unsigned long ultimoLinkOK = 0;
const unsigned long WD_TIMEOUT = 15000;

void resetW5500() {
    Serial.println("[WD] Resetando W5500...");
    // TODO: desligar relés durante o reset

    digitalWrite(W5500_RST, LOW); delay(150);
    digitalWrite(W5500_RST, HIGH); delay(250);
    SPI.begin(W5500_SCK, W5500_MISO, W5500_MOSI, W5500_CS);
    Ethernet.init(W5500_CS);
    Ethernet.begin(mac, ip);
    delay(1000);

    mb.server();
    // TODO: re-adicionar registradores conforme mapa final
    // mb.addCoil(0, false);  // relé 1
    // mb.addCoil(1, false);  // relé 2

    ultimoLinkOK = millis();
    Serial.print("[WD] Reiniciado. IP: ");
    Serial.println(Ethernet.localIP());
}

// ─────────────────────────────────────────────────────────
void setup() {
    Serial.begin(115200);
    delay(500);
    Serial.println("=== ModuNexus — Módulo NEXUS ===");

    pinMode(PIN_LED_VERDE,    OUTPUT); digitalWrite(PIN_LED_VERDE, LOW);
    pinMode(PIN_LED_VERMELHO, OUTPUT); digitalWrite(PIN_LED_VERMELHO, LOW);
    // TODO: inicializar pinos de relés (OUTPUT, LOW)

    pinMode(W5500_RST, OUTPUT);
    resetW5500();

    digitalWrite(PIN_LED_VERDE, HIGH);
    Serial.println("Aguardando conexão do CODESYS...");
}

// ─────────────────────────────────────────────────────────
void loop() {
    mb.task();

    // TODO: ler coils do CODESYS e acionar relés
    // bool rele1 = mb.Coil(0);
    // digitalWrite(RELE_1, rele1 ? HIGH : LOW);

    if (Ethernet.linkStatus() == LinkON) ultimoLinkOK = millis();
    if (millis() - ultimoLinkOK > WD_TIMEOUT) {
        Serial.println("[WD] W5500 travado — resetando");
        resetW5500();
    }

    delay(10);
}