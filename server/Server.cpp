//Rotem Maimon 322745167
//Ram Nagid 318692779

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <thread>
#include <mutex>
#include <queue>
#include <utility>

using namespace std;

static const int CACHE_SIZE = 10;
vector<vector<int>> graph; 
int graphSize = 0;

// cache of input (src and dst) and the output (shortest path)
vector<pair<pair<int,int>,vector<int>>> resultCache(CACHE_SIZE);
int oldestIndex = 0;
std::mutex cacheMutex;

void loadGraph(const char* filename) {
    ifstream file(filename);
    string line;
    while (getline(file, line)) {
        stringstream ss(line);
        int u, v;
        ss >> u >> v;
        graphSize = max(graphSize, max(u, v) + 1);
        graph.resize(graphSize);
        graph[u].push_back(v);
        graph[v].push_back(u); 
    }
    file.close();
}

vector<int> shortestPath(int source, int dest) {
    // BFS
    vector<int> dist(graphSize, -1);
    vector<int> prev(graphSize, -1);
    dist[source] = 0;
    queue<int> q;
    q.push(source);
    while (!q.empty()) {
        int u = q.front();
        q.pop();
        for (int v : graph[u]) {
            if (dist[v] == -1) {
                dist[v] = dist[u] + 1;
                prev[v] = u;
                q.push(v);
            }
        }
    }

    vector<int> path;
    for (int at = dest; at != -1; at = prev[at])
        path.push_back(at);
    reverse(path.begin(), path.end());
    return path;
}

void saveRecentResult(int src ,int dst ,vector<int> result)
{
    cacheMutex.lock();
    
    resultCache[oldestIndex] = make_pair(make_pair(src,dst),result);
    oldestIndex = (oldestIndex + 1) % CACHE_SIZE;
    
    cacheMutex.unlock();
}

vector<int> findCachedResult(int src , int dst)
{
    vector<int> result;
    cacheMutex.lock();
    
    for (auto itr = resultCache.begin() ; itr != resultCache.end() ; ++itr)
    {
        if (src == itr->first.first && dst == itr->first.second)
        {
            result = itr->second;
            break;
        }   
    }

    cacheMutex.unlock();
    return result;
}



void handleConnection(int clientSocket) {
    int source ,dest;
    read(clientSocket , &source ,sizeof(source));
    read(clientSocket , &dest ,sizeof(dest));

    vector<int> path = findCachedResult(source,dest);
    if(path.empty())
    {
        path = shortestPath(source ,dest);
        saveRecentResult(source,dest,path);
    }
    write(clientSocket , path.data(), path.size() * sizeof(int));

    close(clientSocket);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        cerr << "Usage: " << argv[0] << " <filename> <port>" << endl;
        return 1;
    }

    const char* filename = argv[1];
    int port = atoi(argv[2]);

    loadGraph(filename);

    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    while (1) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }
    // Create a new thread to handle the client
    std::thread(handleConnection, new_socket).detach();
    }
    
    return 0;
    }
