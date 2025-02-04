#define V 8 // Number of vertices in the graph

// A utility function to find the vertex with minimum distance value
int minDistance(int dist[], bool sptSet[]) {
  int min = INT_MAX, min_index;
  for (int v = 0; v < V; v++) {
    if (sptSet[v] == false && dist[v] <= min) {
      min = dist[v];
      min_index = v;
    }
  }
  return min_index;
}

// A utility function to print the constructed distance array
void printSolution(int dist[]) {
  Serial.println("Vertex \t Distance from Source");
  for (int i = 0; i < V; i++) {
    Serial.print(i);
    Serial.print(" \t\t\t\t");
    Serial.println(dist[i]);
  }
}

// Function that implements Dijkstra's algorithm for a graph represented using adjacency matrix representation
void dijkstra(int graph[V][V], int src) {
  int dist[V];      // The output array. dist[i] will hold the shortest distance from src to i
  bool sptSet[V];   // sptSet[i] will be true if vertex i is included in shortest path tree

  // Initialize all distances as INFINITE and sptSet[] as false
  for (int i = 0; i < V; i++) {
    dist[i] = INT_MAX;
    sptSet[i] = false;
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
      // Update dist[v] only if v is not in sptSet, there is an edge from u to v, and total weight of path from src to v through u is smaller than current dist[v]
      if (!sptSet[v] && graph[u][v] && dist[u] != INT_MAX && dist[u] + graph[u][v] < dist[v]) {
        dist[v] = dist[u] + graph[u][v];
      }
    }
  }

  // Print the constructed distance array
  printSolution(dist);
}

// The graph initialization (adjacency matrix)
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

  // Call dijkstra's algorithm with source vertex 0
  dijkstra(graph, 0);
}

void loop() {
  // Nothing needed in loop for this implementation
}
