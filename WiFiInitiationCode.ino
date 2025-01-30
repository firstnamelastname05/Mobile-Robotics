#include <WiFi.h>
// wi-fi details
char ssid[] = "iot";
char password[] = "anklet29serpentwood";  //anklet29serpentwood (io48), militarists72disapproval (io38)
WiFiClient client;
// read buffer size for HTTP response
#define BUFSIZE 512

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
int position = 0;

bool connect(){
  if(!client.connect(server, port)){
    Serial.println("error connecting to server");
    return false;
  }
  return true;
}

String readResponse() {
  char buffer[BUFSIZE];
  memset(buffer, 0, BUFSIZE);
  client.readBytes(buffer, BUFSIZE);
  String response(buffer);
  return response;
}

int getStatusCode(String& response) {
  String code = response.substring(9, 12);
  return code.toInt();
}

String getResponseBody(String& response) {
  int split =
  response.indexOf("\r\n\r\n");
  String body =
  response.substring(split+4,
  response.length());
  body.trim();
  Serial.print("The desired position is: ");
  Serial.println(body);
  return body;
}

void setup() {
  Serial.begin(9600);
  delay(1000);
  connectToWiFi();
  connect();
  //client.println("GET /api/getRoute/etgf7354 HTTP/1.1");
}

void loop(){
  String postBody("position=");
  postBody += position;
  // send post request and headers
  client.println("POST /api/arrived/etgf7354 HTTP/1.1");
  client.println("Content-Type: application/x-www-form-urlencoded");
  client.print("Content-Length: ");
  client.println(postBody.length());
  client.println();
  // send post body
  client.println(postBody);

    int destination;
  // read response
  String response = readResponse();
  // get status code
  int statusCode = getStatusCode(response);

  if (statusCode == 200) {
    // success, read body
    String body =
  getResponseBody(response);
  // check if at final destination
    if (body.equals("Finished")) {
      destination = body.toInt();
      while(true);
    }
    else{
      int newPosition = body.toInt();
      if (newPosition > 0){
        position = newPosition;
      }
      else{
        Serial.println("Invalid position");
      }
    }
  }
  delay(1000);

  Serial.print("The response is: ");
  Serial.println(response);
 // Serial.print("The current(?) position is: ");
 // Serial.println(position);
  Serial.print("The postbody is: ");
  Serial.println(postBody);
  //position++;
}
