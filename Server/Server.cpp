#include <cstdint>
#include <iostream>
#include <cstring>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <thread>

// --- Functions ---
std::pair<int, std::string> Receive(int client_fd);

void HandleClient(int client_fd, sockaddr_in client_addr) {
    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(client_addr.sin_addr), client_ip, INET_ADDRSTRLEN);
    int client_port = ntohs(client_addr.sin_port);

    std::cout << "[+] Client connected from IP: " << client_ip << ", Port: " << client_port << "\n";

    while (true) {
        auto [success, message] = Receive(client_fd);
        if (success == 0) {
            std::cout << '<' << client_ip << ':' << client_port << "> " << message;
        } else {
            break;
        }
    }

    std::cout << "[-] Client " << client_ip << " disconnected.\n";
    close(client_fd);
}

int main(int argc, char *argv[]) {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("socket failed");
        return 1;
    }

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    int sp = static_cast<int16_t>(std::stoi(argv[1]));
    server_addr.sin_port = htons(sp);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_fd, (sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind failed");
        return 1;
    }

    if (listen(server_fd, SOMAXCONN) < 0) {
        perror("listen failed");
        return 1;
    }

    std::cout << "[*] Server listening on port " << ntohs(server_addr.sin_port) << "...\n";

    while (true) {
        sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);
        int client_fd = accept(server_fd, (sockaddr*)&client_addr, &client_len);

        if (client_fd < 0) {
            perror("accept failed");
            continue;
        }

        std::thread(HandleClient, client_fd, client_addr).detach();
    }

    close(server_fd);
    return 0;
}

std::pair<int, std::string> Receive(int client_fd) {
    char buffer[1024];
    ssize_t bytesReceived = recv(client_fd, buffer, sizeof(buffer) - 1, 0);

    if (bytesReceived > 0) {
        buffer[bytesReceived] = '\0';
        return {0, std::string(buffer)};
    } else {
        return {1, ""};
    }
}
