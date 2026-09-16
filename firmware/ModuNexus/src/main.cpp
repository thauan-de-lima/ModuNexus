#include <SPI.h>
#include <Ethernet.h>
#include <ModbusEthernet.h>

#define W5500_CS    5
#define W5500_RST   26
#define LED_PIN     2
#define RELE_PIN    4
#define RPWM_PIN    25
#define R_EN_PIN    13
#define L_EN_PIN    14
#define RPWM_CH     0
#define PWM_FREQ    20000
#define PWM_RES     8

byte      mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192, 168, 1, 11);
ModbusEthernet mb;

unsigned long lastCheck = 0;
const unsigned long CHECK_INTERVAL = 10000; // verifica a cada 10 segundos

void resetW5500() {
  ledcWrite(RPWM_CH, 0);
  digitalWrite(W5500_RST, LOW);
  delay(150);
  digitalWrite(W5500_RST, HIGH);
  delay(250);
  Ethernet.init(W5500_CS);
  Ethernet.begin(mac, ip);
  delay(500);
  mb.server();
  for (int i = 0; i < 8; i++) mb.addCoil(i);
  mb.addHreg(0);
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT); digitalWrite(LED_PIN, LOW);
  pinMode(RELE_PIN, OUTPUT); digitalWrite(RELE_PIN, HIGH);
  pinMode(R_EN_PIN, OUTPUT); digitalWrite(R_EN_PIN, HIGH);
  pinMode(L_EN_PIN, OUTPUT); digitalWrite(L_EN_PIN, HIGH);
  ledcSetup(RPWM_CH, PWM_FREQ, PWM_RES);
  ledcAttachPin(RPWM_PIN, RPWM_CH);
  ledcWrite(RPWM_CH, 0);
  pinMode(W5500_RST, OUTPUT);
  resetW5500();
  Serial.print("IP: ");
  Serial.println(Ethernet.localIP());
}

void loop() {
  mb.task();

  digitalWrite(LED_PIN, mb.Coil(0));
  digitalWrite(RELE_PIN, mb.Coil(1) ? LOW : HIGH);
  uint16_t speed = mb.Hreg(0);
  if (speed > 255) speed = 255;
  ledcWrite(RPWM_CH, (uint8_t)speed);

  // Watchdog: verifica só a cada 10 segundos, não em cada iteração
  unsigned long now = millis();
  if (now - lastCheck >= CHECK_INTERVAL) {
    lastCheck = now;
    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
      resetW5500();
    }
  }
}