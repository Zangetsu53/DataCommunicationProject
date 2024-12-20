#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

int main() {
    WSADATA wsa;
    SOCKET client_socket;
    struct sockaddr_in server_addr;
    char buffer[1024];
    char play_again[10];

    printf("Winsock başlatılıyor...\n");
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Winsock başlatılamadı. Hata Kodu: %d\n", WSAGetLastError());
        return 1;
    }

    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == INVALID_SOCKET) {
        printf("Soket oluşturulamadı. Hata Kodu: %d\n", WSAGetLastError());
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(12345);

    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        printf("Sunucuya bağlanılamadı. Hata Kodu: %d\n", WSAGetLastError());
        return 1;
    }

    printf("Sunucuya bağlandınız!\n");

    do {
        while (1) {
            // Kullanıcıdan tahmin al
            printf("Tahmininizi girin: ");
            fgets(buffer, sizeof(buffer), stdin);
            send(client_socket, buffer, strlen(buffer), 0);

            // Sunucudan gelen mesajı al ve yazdır
            memset(buffer, 0, sizeof(buffer));
            recv(client_socket, buffer, sizeof(buffer), 0);
            printf("Sunucudan gelen mesaj: %s", buffer);

            // Doğru tahmin durumunda döngüden çık
            if (strstr(buffer, "Doğru tahmin!")) {
                break;
            }
        }

        // Yeni oyun isteğini kontrol et
        memset(buffer, 0, sizeof(buffer));
        recv(client_socket, buffer, sizeof(buffer), 0);
        printf("%s", buffer); // "Yeni bir oyun oynamak ister misiniz?" mesajını göster

        // Kullanıcının yanıtını al ve gönder
        memset(play_again, 0, sizeof(play_again));
        fgets(play_again, sizeof(play_again), stdin);
        send(client_socket, play_again, strlen(play_again), 0);

        // Yanıta göre döngüye devam et veya çık
        if (play_again[0] == 'H' || play_again[0] == 'h') {
            printf("Sunucudan bağlantı kesiliyor...\n");
            break;
        }
    } while (1);

    closesocket(client_socket);
    WSACleanup();

    return 0;
}
