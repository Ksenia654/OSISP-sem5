#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <sstream>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

struct ClientInfo {
    SOCKET socket;
    string nickname;
};

vector<ClientInfo> clients;

void send_to_client(const string& receiver, const string& message, SOCKET sender_socket) {
    for (auto& c : clients) {
        if (c.nickname == receiver) {
            send(c.socket, message.c_str(), static_cast<int>(message.size()), 0);
            return;
        }
    }
    string error = "Сервер: пользователь " + receiver + " не найден.";
    send(sender_socket, error.c_str(), static_cast<int>(error.size()), 0);
}

void broadcast(const string& message, SOCKET sender_socket) {
    for (auto& c : clients) {
        if (c.socket != sender_socket) {
            send(c.socket, message.c_str(), static_cast<int>(message.size()), 0);
        }
    }
}

void handle_client(ClientInfo clientInfo) {
    char buffer[1024];
    int bytes_received;

    while (true) {
        bytes_received = recv(clientInfo.socket, buffer, sizeof(buffer), 0);
        if (bytes_received <= 0) {
            cout << "Пользователь " << clientInfo.nickname << " отключен." << endl;
            closesocket(clientInfo.socket);
            break;
        }
        buffer[bytes_received] = '\0';
        string message(buffer);

        stringstream ss(message);
        string sender, receiver, text;
        getline(ss, sender, '|');
        getline(ss, receiver, '|');
        getline(ss, text);

        if (receiver == "ALL") {
            broadcast(sender + ": " + text, clientInfo.socket);
        }
        else {
            send_to_client(receiver, sender + " (лично): " + text, clientInfo.socket);
        }
    }
}

void start_server(int port) {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    SOCKET server_socket = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in server_addr, client_addr;
    int client_addr_len = sizeof(client_addr);

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr));
    listen(server_socket, 5);

    cout << "Сервер запущен на порту " << port << endl;

    while (true) {
        SOCKET client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_len);

        char nickname[50];
        int bytes = recv(client_socket, nickname, sizeof(nickname) - 1, 0);
        if (bytes > 0) {
            nickname[bytes] = '\0';
            ClientInfo info{ client_socket, string(nickname) };
            clients.push_back(info);
            cout << "Пользователь " << info.nickname << " подключен." << endl;

            thread(handle_client, info).detach();
        }
    }

    closesocket(server_socket);
    WSACleanup();
}

int main() {
    int port = 12345;
    start_server(port);
    return 0;
}
