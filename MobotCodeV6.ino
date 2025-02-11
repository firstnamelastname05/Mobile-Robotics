#define V 8 // define number of checkpoints on the map

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

// distance sensor analog value & pin assignment 
int distancePin = 16;
int distanceValue = 0;

// motor speed variables
int speed_left = 175;
int speed_right = 175;

// check for count of distance sensor readings
int targetCount = 5;
int consecCount = 0;

// define a specific route
//int route[] = {0, 3, 1};
// get route array length
//int routeLength = sizeof(route) / sizeof(route[0]);

// array of no. of checkpoints to skip between checkpoints in route array
int skippedCount[10];

// variable for iterating through skippedCount array
int skipIndex = 0;

// set line following sensors threshold value
int thresholdLS = 750;

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

int firstStop = 1;

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

// function to find the checkpoint with minimum distance value
int minDistance(int dist[], bool sptSet[]) {
  int min = INT_MAX, min_index;
  for (int v = 0; v < V; v++) {
    if (!sptSet[v] && dist[v] <= min) {
      min = dist[v];
      min_index = v;
    }
  }
  return min_index;
}

// function to extract the shortest path from source to a destination
void extractPath(int parent[], int dest, int path[], int &pathIndex) {
  if (dest == -1) return;
  extractPath(parent, parent[dest], path, pathIndex);
  path[pathIndex++] = dest;
}

// dijkstra's algorithm to find the shortest path from source
void dijkstra(int graph[V][V], int src, int parent[], int dist[]) {
  bool sptSet[V]; // sptSet[i] will be true if checkpoint i is included in shortest path tree

  // initialize all distances as INFINITE and sptSet[] as false
  for (int i = 0; i < V; i++) {
    dist[i] = INT_MAX;
    sptSet[i] = false;
    parent[i] = -1; // no parent initially
  }

  // distance of source checkpoint from itself is always 0
  dist[src] = 0;

  // find shortest path for all checkpoints
  for (int count = 0; count < V - 1; count++) {
    // pick the minimum distance checkpoint from the set of checkpoints not yet processed
    int u = minDistance(dist, sptSet);

    // mark the picked checkpoint as processed
    sptSet[u] = true;

    // update dist value of the adjacent checkpoints of the picked checkpoint
    for (int v = 0; v < V; v++) {
      if (!sptSet[v] && graph[u][v] && dist[u] != INT_MAX && dist[u] + graph[u][v] < dist[v]) {
        dist[v] = dist[u] + graph[u][v];
        parent[v] = u; // set parent
      }
    }
  }
}

// process the given route and determine the number of checkpoints skipped for each segment
void processRoute(int graph[V][V], int route[], int routeLength) {
  int dist[V], parent[V];
  int path[V];
  int pathIndex;

  for (int i = 0; i < routeLength - 1; i++) {  // Start from index 0
    int src = route[i];
    int dest = route[i+1];

    // Run Dijkstra's algorithm from the current source
    dijkstra(graph, src, parent, dist);

    // Extract the shortest path from source to destination
    pathIndex = 0;
    extractPath(parent, dest, path, pathIndex);

    // Add number of checkpoints to skip to skippedCount array
    skippedCount[i] = (pathIndex > 2) ? (pathIndex - 2) : 0;

    // Debug output for skipped checkpoints
    Serial.print("Skipped checkpoints from ");
    Serial.print(src);
    Serial.print(" to ");
    Serial.print(dest);
    Serial.print(": ");
    Serial.println(skippedCount[i]);
  }
}

// set distances between checkpoints on the map
int graph[V][V] = {
  {0, 0, 0, 0, 8, 0, 0, 7},
  {0, 0, 0, 0, 0, 0, 2, 5},
  {0, 0, 0, 8, 0, 0, 0, 7},
  {0, 0, 8, 0, 0, 0, 10, 0},
  {8, 0, 0, 0, 0, 0, 10, 0},
  {0, 0, 0, 0, 0, 0, 10, 0},
  {0, 2, 0, 10, 10, 10, 0, 0},
  {7, 5, 7, 0, 0, 0, 0, 0},
};

// car goes forward in straight line
void straight() {
    speed_left = 175;
    speed_right = 175;
    analogWrite(motor1PWM, speed_left);
    analogWrite(motor2PWM, speed_right);
}

// car performs 180 degree turn in place
void reverse() {
    analogWrite(motor2Phase, 0);
    speed_left = 255;
    speed_right = 255;
    analogWrite(motor1PWM, speed_left);
    analogWrite(motor2PWM, speed_right);
    delay(550);
    analogWrite(motor2Phase, 255);
}

// car stops in place
void stop() {
    speed_left = 0;
    speed_right = 0;
    analogWrite(motor1PWM, speed_left);
    analogWrite(motor2PWM, speed_right);
}

// car turns 90 degrees right in place
void rightTurn() {
    straight(); // stops car turning too early
    delay(200);

    stop(); // stop for set time
    delay(250);

    // rotate mobot 90 degrees right
    analogWrite(motor1Phase, 0);
    speed_left = 255;
    speed_right = 255;
    analogWrite(motor1PWM, speed_left);
    analogWrite(motor2PWM, speed_right);
    delay(275);
    analogWrite(motor1Phase, 255);

    // check when middle sensor returns to path
    if(AnalogValue[2] < thresholdLS) {
        // drive straight
        straight();
        delay(125);
    }
}

// car turns 90 degrees left in place
void leftTurn() {
    straight(); // stops car turning too early
    delay(200);

    stop(); // stop for set time
    delay(250);

    // rotate mobot 90 degrees left
    analogWrite(motor2Phase, 0);
    speed_left = 255;
    speed_right = 255;
    analogWrite(motor1PWM, speed_left);
    analogWrite(motor2PWM, speed_right);
    delay(275);
    analogWrite(motor2Phase, 255);

    // check when middle sensor returns to path
    if(AnalogValue[2] < thresholdLS) {
        // drive straight
        straight();
        delay(125);
    }
}

// adjust each motor speed to turn left when inside sensor detects black
void leftInsideSensorCorrection() {
    speed_left = 175;
    speed_right = 125;
    analogWrite(motor1PWM, speed_left);
    analogWrite(motor2PWM, speed_right);
}

// adjust each motor speed to turn right when inside sensor detects black
void rightInsideSensorCorrection() {
    speed_left = 125;
    speed_right = 175;
    analogWrite(motor1PWM, speed_left);
    analogWrite(motor2PWM, speed_right);
}

// adjust each motor speed to turn left when outside sensor detects black
void leftOutsideSensorCorrection() {
    speed_left = 255;
    speed_right = 50;
    analogWrite(motor1PWM, speed_left);
    analogWrite(motor2PWM, speed_right);
}

// adjust each motor speed to turn right when outside sensor detects black
void rightOutsideSensorCorrection() {
    speed_left = 50;
    speed_right = 255;
    analogWrite(motor1PWM, speed_left);
    analogWrite(motor2PWM, speed_right);
}

void updateCloud() {
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
      String body = getResponseBody(response);
              
      // check if at final destination
      if (body.equals("Finished")) {
        destination = body.toInt();
        while(true);
      }
      else {
        int newPosition = body.toInt();
        if (newPosition > 0){
          position = newPosition;
        }
        else {
          Serial.println("Invalid position");
        }
      }
    }
    delay(1000);

    Serial.print("The response is: ");
    Serial.println(response);
    Serial.print("Current Position: ");
    Serial.println(postBody);

    delay(100);
}

// setup code executes once upon running the code
void setup() {
    Serial.begin(9600);

    // set motor turn directions to move forwards
    analogWrite(motor1Phase, 255);
    analogWrite(motor2Phase, 255);

    // stop motors upon startup
    analogWrite(motor1PWM, 0);
    analogWrite(motor2PWM, 0);

    connectToWiFi();
    connect();
    sendGetRequest();

    updateCloud();

    processRoute(graph, route, routeLength);
    /*for (int x = 0; x < routeLength - 1; x++) {
        Serial.println(skippedCount[x]);
    }*/
}

// loop function executes repeatedly
void loop() {
    // cycle through sensor values for alignment detection
    int i;
    for (i=0;i<5;i++)
    {
        AnalogValue[i]=analogRead(AnalogPin[i]);
    
        /*Serial.print(AnalogValue[i]); // This prints the actual analog reading from the sensors
        Serial.print("\t"); //tab over on screen
        if(i==4)
        {
            Serial.println(""); //carriage return
            delay(600); // display new set of readings every 600mS
        }*/
    }

    // reset speed to default if inside & outside sensors are on black
    if(AnalogValue[0] > thresholdLS && AnalogValue[1] > thresholdLS && AnalogValue[3] > thresholdLS && AnalogValue[4] > thresholdLS && AnalogValue[2] < thresholdLS) {
      straight();
    }

    // when all / any 4 sensors are on white
    if(AnalogValue[0] < thresholdLS && AnalogValue[1] < thresholdLS && AnalogValue[3] < thresholdLS && AnalogValue[4] < thresholdLS && AnalogValue[2] < thresholdLS || AnalogValue[1] < thresholdLS && AnalogValue[3] < thresholdLS && AnalogValue[4] < thresholdLS && AnalogValue[2] < thresholdLS || AnalogValue[0] < thresholdLS && AnalogValue[1] < thresholdLS && AnalogValue[3] < thresholdLS && AnalogValue[2] < thresholdLS) {
        Serial.println("DETECTED");

        if (firstStop == 1) {
          firstStop = 0;
          skippedCount[0]++;
          stop();
          delay(1000);
          straight();
        }
        
        // if no checkpoints are to be skipped before desired checkpoint
        else if (skippedCount[skipIndex] == 0) {
          stop();
          updateCloud();
          straight();

          if (skipIndex <= routeLength - 1) {
            skipIndex++;
          }
          else {
            skipIndex = 0;
          }

          delay(250);
        }

        else if (skippedCount[skipIndex] == 1 && route[skipIndex + 1] == 1) {
          if (route[skipIndex] == 0 || route[skipIndex] == 3) {
            leftTurn();
            skippedCount[skipIndex]--;  // decrement skipped count
            straight();
            delay(250);
          }
          else if (route[skipIndex] == 2 || route[skipIndex] == 4) {
            rightTurn();
            skippedCount[skipIndex]--;  // decrement skipped count
            straight();
            delay(250);            
          }
        }

        // if skipping a checkpoint
        else {
          skippedCount[skipIndex]--;
          straight();
          delay(250);
        }
    }

    // inside sensors speed change
    if(AnalogValue[1] < thresholdLS)
    {
        leftInsideSensorCorrection();
    }
    if(AnalogValue[3] < thresholdLS)
    {
        rightInsideSensorCorrection();
    }

    // outside sensors speed change
    if(AnalogValue[0] < thresholdLS)
    {
        leftOutsideSensorCorrection();
    }
    if(AnalogValue[4] < thresholdLS)
    {
        rightOutsideSensorCorrection();
    }

    // read analog values of distance sensor
    distanceValue = analogRead(distancePin);

    // increase count when mobot close to obstruction/wall
    if(distanceValue > 1750) {
        consecCount++;
    }
    // if consecutive readings aren't high enough reset count
    else {
        consecCount = 0;
    }

    // when targetCount number of consecutive readings are high
    if(consecCount == targetCount) {
      stop();
      delay(250);

      // turn 180 degrees
      reverse();

      // when sensor returns to path
      if(AnalogValue[2] < thresholdLS) {
          straight();
          delay(125);
      }
    }    
}