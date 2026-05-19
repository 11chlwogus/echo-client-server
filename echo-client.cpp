#include <iostream>
#include <string>
#include <thread>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

void receive_from_server(int sock) {
    char buffer[1024];
    while (true) {
        memset(buffer, 0, sizeof(buffer));
        int bytes_received = recv(sock, buffer, sizeof(buffer) - 1, 0);
        
        if (bytes_received <= 0) {
            std::cout << "\n[클라이언트] 서버와의 연결이 끊어졌습니다.\n";
            exit(0); 
        }
        
        std::cout << "[서버 응답]: " << buffer;
    }
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Syntax : echo-client <ip> <port>\n";
        std::cerr << "Sample : echo-client 127.0.0.1 1234\n";
        return 1;
    }

    std::string ip = argv[1];
    int port = std::stoi(argv[2]);
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        std::cerr << "소켓 생성 실패\n";
        return 1;
    }

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    
    if (inet_pton(AF_INET, ip.c_str(), &server_addr.sin_addr) <= 0) {
        std::cerr << "잘못된 IP 주소 형식입니다.\n";
        return 1;
    }

    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        std::cerr << "서버 접속 실패\n";
        return 1;
    }

    std::cout << "[클라이언트] 서버에 연결되었습니다. 메시지를 입력하세요.\n";

    std::thread recv_thread(receive_from_server, client_socket);
    recv_thread.detach(); 

    std::string input;
    while (true) {
        std::getline(std::cin, input);
        if (input.empty()) continue;

        input += "\n"; 
        int bytes_sent = send(client_socket, input.c_str(), input.length(), 0);
        
        if (bytes_sent == -1) {
            std::cerr << "메시지 전송 실패\n";
            break;
        }
    }

    close(client_socket);
    return 0;
}
