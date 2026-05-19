#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <algorithm>

std::vector<int> client_sockets;
std::mutex clients_mutex;

bool is_echo = false;
bool is_broadcast = false;

void handle_client(int client_socket){
    char buffer[1024];

    while(true){
        memset(buffer, 0, sizeof(buffer));
        int bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
        
        if(bytes_received <= 0){
            std::cout << "상대방이 연결을 끊었거나 에러가 발생했음. socket: " << client_socket << " \n";
            break;
        }

        std::cout << "[수신값][Socket " << client_socket << "]: " << buffer;

        if (is_broadcast){
            std::lock_guard<std::mutex> lock(clients_mutex);
            for(int sock : client_sockets){
                 send(sock, buffer, bytes_received, 0);
            }
        }
        else if (is_echo){
            send(client_socket, buffer, bytes_received, 0);
        }
    }
    
    close(client_socket);
    {
        std::lock_guard<std::mutex> lock(clients_mutex);
        client_sockets.erase(std::remove(client_sockets.begin(), client_sockets.end(), client_socket), client_sockets.end());
    }
}

int main(int argc, char* argv[]){
    if(argc < 2){
        std::cerr << "syntax : echo-server <port> [-e[-b]]\n";
        std::cerr << "sample : echo-server 1234 -e -b\n";
        return 1;
    }

    int port = std::stoi(argv[1]);

    for(int i = 2; i < argc; i++){
        std::string arg = argv[i];
        if(arg == "-e") is_echo = true;
        if(arg == "-b") is_broadcast = true;
    }

    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(server_socket == -1){
        std::cerr << "소켓 생성 실패\n";
        return 1;
    }

    int opt = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if(bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1){
        std::cerr << "bind 실패\n";
        return 1;
    }

    if(listen(server_socket, 10) == -1){
        std::cerr << "listen 실패\n";
        return 1;
    }

    std::cout <<"서버 포트 " << port << "번에서 대기 중\n";

    while (true){
        sockaddr_in client_addr{};
        socklen_t client_addr_len = sizeof(client_addr);
        int client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_len);

        if(client_socket == -1){
            std::cerr << "accept 실패\n";
            continue;
        }

        std::cout << "서버에 클라이언트 접속 성공 socket: "<< client_socket <<"\n";

        {
            std::lock_guard<std::mutex> lock(clients_mutex);
            client_sockets.push_back(client_socket);
        }

        std::thread t(handle_client, client_socket);
        t.detach();
    }

    close(server_socket);
    return 0;
}
