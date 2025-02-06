#include <WiFi.h>
// wi-fi details
char ssid[] = "iot";
char password[] = "militarists72disapproval";
WiFiClient client;
// read buffer size for HTTP response
#define BUFSIZE 512

// sensor analog values & ESP pin assignments
int AnalogValue[5] = {0,0,0,0,0};
int AnalogPin[5] = {4,5,6,7,15};

// left motor pin assignments
int motor1PWM = 37;
int motor1Phase = 38;

// right motor pin assignments
int motor2PWM = 39;
int motor2Phase = 40;

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
  //Serial.print("The desired position is: ");
  //Serial.println(body);
  return body;
}

#define MAX_VALUES 10  // Adjust based on expected number of values
int route[MAX_VALUES]; // Array to store the parsed numbers
int routeLength = 0;   // Number of elements in the array

void parseRoute(String response) {
  // Get the body part of the response (excluding headers)
  String body = getResponseBody(response);
  
  // Convert String to char array for strtok()
  char buffer[body.length() + 1]; 
  body.toCharArray(buffer, body.length() + 1);

  char* token = strtok(buffer, ",");  // Split at commas
  routeLength = 0;

  while (token != NULL && routeLength < MAX_VALUES) {
    route[routeLength++] = atoi(token);  // Convert to int and store
    token = strtok(NULL, ",");           // Get next number
  }

  // Print parsed values
  Serial.print("Parsed route: ");
  for (int i = 0; i < routeLength; i++) {
    Serial.print(route[i]);
    Serial.print(" ");
  }
  Serial.println();
}

void sendGetRequest() {
  if (!client.connect(server, port)) {
    Serial.println("Error connecting to server");
    return;
  }
  client.println("GET /api/getRoute/etgf7354 HTTP/1.1");
  client.println("Host: 3.250.38.184");  // Include Host header
  //client.println("Connection: close");   // Ensure connection closes after response
  client.println(); // Blank line to end headers

  Serial.println("GET request sent");

  String response = readResponse(); // Read server response
  Serial.println("Response: " + response);

  parseRoute(response);  // Convert response to array
}


void setup() {
  Serial.begin(9600);
  delay(1000);
  connectToWiFi();
  connect();
  //client.println("GET /api/getRoute/etgf7354 HTTP/1.1");
  analogWrite(motor1PWM, 0);
  analogWrite(motor2PWM, 0);
  sendGetRequest();
}

void loop(){

  for(int i = 0; i < 5; i++){
    AnalogValue[i]=analogRead(AnalogPin[i]); //Read sensor data

    if(AnalogValue[0] < 500 && AnalogValue[1] < 500 && AnalogValue[3] < 500 && AnalogValue[4] < 500 && AnalogValue[2] < 500){ // if all sensors are white, send post request
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
      Serial.print("Current Position: ");
      Serial.println(postBody);
      //position++;
    }
  }
}
