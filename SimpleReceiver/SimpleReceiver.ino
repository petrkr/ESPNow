#ifdef ESP8266
  #include <espnow.h>
  #include <ESP8266WiFi.h>
#endif

#ifdef ESP32
  #include <esp_now.h>
  #include <WiFiClientSecure.h>
#endif

#define CHANNEL 1

#define RXLED 2
#define RXBLINKDELAY 50
long lastrxblink = 0;


// Power GPIO
#define POWER_IO 18


#define KEEPALIVE 5000


// Init ESP Now with fallback
void InitESPNow() {
  WiFi.disconnect();
  if (esp_now_init() == 0) {
    Serial.println("ESPNow Init Success");
  }
  else {
    Serial.println("ESPNow Init Failed");
    ESP.restart();
  }
}

bool     espnow_received = false;
char     espnow_macStr[18];
uint8_t  espnow_dataLen;
uint8_t  *espnow_data;

// config AP SSID
void configDeviceAP() {
  WiFi.mode(WIFI_AP);

  const char *SSID = "ESPNow Simple receiver";
  bool result = WiFi.softAP(SSID, "donttellmeyourpassword", CHANNEL, 0);
  if (!result) {
    Serial.println("AP Config failed.");
  } else {
    Serial.println("AP Config Success. Broadcasting with AP: " + String(SSID));
    Serial.print("AP MAC: "); Serial.println(WiFi.softAPmacAddress());
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(RXLED, OUTPUT);
  pinMode(POWER_IO, OUTPUT);

  digitalWrite(POWER_IO, LOW);

  configDeviceAP();

  InitESPNow();

  esp_now_register_recv_cb(OnEspDataRecv);
}


void OnEspDataRecv(const uint8_t *mac_addr, const uint8_t *data, int data_len) {
  OnEspDataRecv(mac_addr, data, data_len);
}

void OnEspDataRecv(uint8_t * mac_addr, uint8_t *data, uint8_t data_len) {
  if (espnow_received) {
    Serial.println("Not processed yet, skipping message");
    return;
  }  

  snprintf(espnow_macStr, sizeof(espnow_macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  espnow_dataLen = data_len;
  espnow_data = (uint8_t*)malloc(data_len * sizeof(uint8_t));
  memcpy(espnow_data, data, data_len);
  espnow_received = true;
}


long previousKeepAliveMillis = 0;
void handleKeepAlive() {
  if (millis() - previousKeepAliveMillis >= KEEPALIVE) {
    previousKeepAliveMillis = millis();
    Serial.println("Keep Alive");
  }
}

void processEspNowData() {
  Serial.print("Last Packet Recv from: "); Serial.println(espnow_macStr);
  Serial.print("Last Packet Recv Len: "); Serial.println(espnow_dataLen);
  Serial.print("Last Packet Recv Data: ");

  for (int i = 0; i < espnow_dataLen; i++) {
    Serial.print(espnow_data[i], HEX);
    Serial.print(" ");
  }
  Serial.println();

  //check ESPnow data
  if (espnow_data[0] == 1 and espnow_data[1] == 1) {
    bool state = digitalRead(POWER_IO);
    Serial.print("Setting output ");
    Serial.println(state ? "OFF" : "ON");
    // toggle Power pin
    digitalWrite(POWER_IO, !state);
  }
}

void loop() {

  if (espnow_received)
  {
    lastrxblink = millis();
    Serial.println("Received ESP Now data");

    processEspNowData();

    free(espnow_data);
    espnow_received = false;
  }

  if ( millis() < (lastrxblink + RXBLINKDELAY) ) {
    digitalWrite(RXLED, 1);
  } else {
    digitalWrite(RXLED, 0);
  }

  handleKeepAlive();
}
