//Rotem Maimon 322745167
//Ram Nagid 318692779 


#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <vector>

using namespace std;
    
int main(int argc, char *argv[]) {
    if (argc != 5) {
        cerr << "Usage: " << argv[0] << " <ip> <port> <v1> <v2>" << endl;
        return 1;
    }

    const char* ip = argv[1];
    int port = atoi(argv[2]);
    int source = atoi(argv[3]);
    int dest = atoi(argv[4]);

    int sock = 0;
    struct sockaddr_in serv_addr;

    

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        cerr << "Socket creation error" << endl;
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    if(inet_pton(AF_INET, ip, &serv_addr.sin_addr) <= 0) {
        cerr << "Invalid address/ Address not supported" << endl;
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        cerr << "Connection Failed" << endl;
        return -1;
    }

    write(sock, &source, sizeof(source));
    write(sock, &dest, sizeof(dest));


    vector <int> path;
    int node;
    while (read(sock, &node, sizeof(node)) > 0) {
        path.push_back(node);
    }

    for (int node : path) {
        cout << node << " ";
    }
    cout << endl;

    close(sock);
    return 0;
}
