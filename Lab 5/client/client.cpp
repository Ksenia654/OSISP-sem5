#include <iostream>
#include <string>
#include <thread>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

void receive_messages(SOCKET client_socket) {
    char buffer[1024];
    int bytes_received;

    while (true) {
        bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
        if (bytes_received <= 0) {
            cout << "Вы отключены от чата." << endl;
            break;
        }
        buffer[bytes_received] = '\0';
        cout << buffer << endl;
    }
}

int main() {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    string nickname;
    cout << "Введите ваше имя: ";
    getline(cin, nickname);

    SOCKET client_socket = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(12345);
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

    connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr));
    send(client_socket, nickname.c_str(), static_cast<int>(nickname.size()), 0);

    thread(receive_messages, client_socket).detach();

    string message;
    while (true) {
        getline(cin, message);

        string formatted;
        if (message.rfind("/msg ", 0) == 0) {
            size_t spacePos = message.find(' ', 5);
            if (spacePos != string::npos) {
                string receiver = message.substr(5, spacePos - 5);
                string text = message.substr(spacePos + 1);
                formatted = nickname + "|" + receiver + "|" + text;
            }
        }
        else {
            formatted = nickname + "|ALL|" + message;
        }

        send(client_socket, formatted.c_str(), static_cast<int>(formatted.size()), 0);
    }

    closesocket(client_socket);
    WSACleanup();
    return 0;
}
