#include <espnow.h>
#include <ESP8266WiFi.h>

// Change this to MAC of Receiver
uint8_t REMOTE_MAC[] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66};

#define BUTTON1 33

#define WIFI_CHANNEL 1

struct __attribute__((packed)) BUTTON_DATA {
  uint8_t version;
  uint8_t devicetype;
  uint8_t state;
} buttonData;


// Init ESP Now with fallback
void InitESPNow() {
  WiFi.disconnect();
  if (esp_now_init() != 0) {
    Serial.println("ESPNow Init Failed");
    ESP.restart();
  }

  Serial.println("ESPNow Init Success");
  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
}

void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  char macStr[18];
  Serial.print("Packet to:");
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
         mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.print(macStr);
  Serial.print(" send status: ");
  if (sendStatus == 0){
    Serial.println("Delivery success");
  }
  else{
    Serial.println("Delivery fail");
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(BUTTON1, INPUT_PULLUP);

  WiFi.mode(WIFI_STA);
  // This is the mac address of the Slave in AP Mode
  Serial.print("STA MAC: "); Serial.println(WiFi.macAddress());
  Serial.print("AP MAC: "); Serial.println(WiFi.softAPmacAddress());

  // Init ESPNow
  InitESPNow();

  // Register data sent callback
  esp_now_register_send_cb(OnDataSent);
  
  // Register peer
  esp_now_add_peer(REMOTE_MAC, ESP_NOW_ROLE_SLAVE, WIFI_CHANNEL, NULL, 0);
  
  Serial.println("Boot Done");
}

void loop()
{
  buttonData.version = 1;
  buttonData.devicetype = 1; // Button
  buttonData.state = digitalRead(BUTTON1);

  esp_now_send(0, (uint8_t *) &buttonData, sizeof(buttonData));

  Serial.println();

  // Sleep for while
  delay(1000);
}
