#include <WiFi.h>
// wi-fi details
char ssid[] = "Yuvraj";
char password[] = "12345678";
WiFiClient client;
void connectToWiFi() {
  Serial.print("Connecting to network: ");
  Serial.print(ssid);
  Serial.flush();
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    Serial.flush();
    delay(300);
  }
  Serial.println("Connected");
  Serial.print("Obtaining IP address");
  Serial.flush();

  while (WiFi.localIP() == INADDR_NONE) {
    Serial.print(".");
    Serial.flush();
    delay(300);
  }
  Serial.println();
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

// server details
char server[] = "3.250.38.184";
int port = 8000;

bool connect(){
  if(!client.connect(server, port)){
    Serial.println("error connecting to server");
    return false;
  }
  Serial.println("connected to server");
  return true;
}


void setup() {
  Serial.begin(9600);
  delay(1000);
  connectToWiFi();
}

void loop(){

}