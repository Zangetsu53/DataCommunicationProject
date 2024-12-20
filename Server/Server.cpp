#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <winsock2.h>
#include <windows.h>

#pragma comment(lib, "ws2_32.lib")

// İş parçacığının istemci ile iletişim kurduğu fonksiyon
DWORD WINAPI handle_client(LPVOID client_socket_ptr) {
    SOCKET client_socket = *(SOCKET*)client_socket_ptr;
    char buffer[1024];
    char play_again[10];
    int secret_number, guess;

    printf("Yeni bir istemci iş parçacığında işleniyor.\n");

    while (1) {
        // Yeni oyun başlat
        srand(time(NULL));
        secret_number = rand() % 100 + 1;
        printf("Yeni bir sayı belirlendi: %d (İstemcinin görmemesi gerekiyor!)\n", secret_number);

        while (1) {
            memset(buffer, 0, sizeof(buffer));
            recv(client_socket, buffer, sizeof(buffer), 0);
            guess = atoi(buffer);

            if (guess > secret_number) {
                send(client_socket, "Daha küçük bir sayı deneyin.\n", 28, 0);
            }
            else if (guess < secret_number) {
                send(client_socket, "Daha büyük bir sayı deneyin.\n", 29, 0);
            }
            else {
                send(client_socket, "Doğru tahmin! Tebrikler!\n", 25, 0);
                break;
            }
        }

        // Yeni bir oyun isteği kontrolü
        send(client_socket, "Yeni bir oyun oynamak ister misiniz? (E/H): ", 44, 0);
        memset(play_again, 0, sizeof(play_again));
        recv(client_socket, play_again, sizeof(play_again), 0);

        if (play_again[0] == 'H' || play_again[0] == 'h') {
            printf("İstemci yeni oyun istemedi, bağlantı kapatılıyor.\n");
            break;
        }
    }

    closesocket(client_socket);
    printf("İstemci bağlantısı kapatıldı.\n");
    return 0;
}

int main() {
    WSADATA wsa;
    SOCKET server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    int client_len;

    printf("Winsock başlatılıyor...\n");
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Winsock başlatılamadı. Hata Kodu: %d\n", WSAGetLastError());
        return 1;
    }

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == INVALID_SOCKET) {
        printf("Soket oluşturulamadı. Hata Kodu: %d\n", WSAGetLastError());
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(12345);

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        printf("Bağlama başarısız. Hata Kodu: %d\n", WSAGetLastError());
        return 1;
    }

    printf("Bağlantılar bekleniyor...\n");
    listen(server_socket, 5);

    while (1) { // Sonsuz döngü ile istemcileri kabul et
        client_len = sizeof(client_addr);
        client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);
        if (client_socket == INVALID_SOCKET) {
            printf("Bağlantı kabul edilemedi. Hata Kodu: %d\n", WSAGetLastError());
            continue;
        }

        printf("Yeni bağlantı kabul edildi.\n");

        // Her yeni istemci için bir iş parçacığı oluştur
        HANDLE thread_handle;
        DWORD thread_id;
        thread_handle = CreateThread(
            NULL,               // Varsayılan güvenlik öznitelikleri
            0,                  // Varsayılan yığın boyutu
            handle_client,      // İş parçacığı fonksiyonu
            (LPVOID)&client_socket, // İş parçacığına geçirilen parametre (istemci soketi)
            0,                  // Varsayılan oluşturma bayrakları
            &thread_id          // İş parçacığı kimliği
        );

        if (thread_handle == NULL) {
            printf("İş parçacığı oluşturulamadı. Hata Kodu: %d\n", GetLastError());
            closesocket(client_socket);
        }
        else {
            CloseHandle(thread_handle); // İş parçacığı ile işimiz bitti, ana sunucu bu handle'ı kapatabilir.
        }
    }

    closesocket(server_socket);
    WSACleanup();

    return 0;
}

