#define V 8 // Number of vertices in the graph

// A utility function to find the vertex with minimum distance value
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

// A utility function to extract the shortest path from source to a destination
void extractPath(int parent[], int dest, int path[], int &pathIndex) {
  if (dest == -1) return;
  extractPath(parent, parent[dest], path, pathIndex);
  path[pathIndex++] = dest;
}

// Function to print the number of checkpoints skipped
void printSkippedCheckpointCount(int path[], int pathLength, int src, int dest) {
  Serial.print("From ");
  Serial.print(src);
  Serial.print(" to ");
  Serial.print(dest);
  Serial.print(" -> Number of skipped checkpoints: ");

  // Count intermediate checkpoints (exclude source and destination)
  int skippedCount = (pathLength > 2) ? (pathLength - 2) : 0;
  Serial.println(skippedCount);
}

// Dijkstra's algorithm to find the shortest path from `src`
void dijkstra(int graph[V][V], int src, int parent[], int dist[]) {
  bool sptSet[V]; // sptSet[i] will be true if vertex i is included in shortest path tree

  // Initialize all distances as INFINITE and sptSet[] as false
  for (int i = 0; i < V; i++) {
    dist[i] = INT_MAX;
    sptSet[i] = false;
    parent[i] = -1; // No parent initially
  }

  // Distance of source vertex from itself is always 0
  dist[src] = 0;

  // Find shortest path for all vertices
  for (int count = 0; count < V - 1; count++) {
    // Pick the minimum distance vertex from the set of vertices not yet processed
    int u = minDistance(dist, sptSet);

    // Mark the picked vertex as processed
    sptSet[u] = true;

    // Update dist value of the adjacent vertices of the picked vertex
    for (int v = 0; v < V; v++) {
      if (!sptSet[v] && graph[u][v] && dist[u] != INT_MAX && dist[u] + graph[u][v] < dist[v]) {
        dist[v] = dist[u] + graph[u][v];
        parent[v] = u; // Set parent
      }
    }
  }
}

// Process the given route and determine the number of checkpoints skipped for each segment
void processRoute(int graph[V][V], int route[], int routeLength) {
  int dist[V], parent[V];
  int path[V];
  int pathIndex;

  for (int i = 0; i < routeLength - 1; i++) {
    int src = route[i];
    int dest = route[i + 1];

    // Run Dijkstra's algorithm from the current source
    dijkstra(graph, src, parent, dist);

    // Extract the shortest path from source to destination
    pathIndex = 0;
    extractPath(parent, dest, path, pathIndex);

    // Print the number of skipped checkpoints
    printSkippedCheckpointCount(path, pathIndex, src, dest);
  }
}

// The graph (adjacency matrix)
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

void setup() {
  // Start serial communication at 9600 baud
  Serial.begin(9600);

  // Define a specific route
  int route[] = {0, 2, 3, 0, 3};
  int routeLength = sizeof(route) / sizeof(route[0]);

  // Process the route
  processRoute(graph, route, routeLength);
}

void loop() {
  // Nothing needed in loop for this implementation
}
