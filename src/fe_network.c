// src/network/fe_network.c

#include "network/fe_network.h"
#include "utils/fe_logger.h"
#include <stdlib.h> // malloc, free
#include <string.h> // memset, memcpy

// Yer tutucu (Dummy) Soket Fonksiyonları (Gerçek kütüphane yerine)
#define PLATFORM_SOCKET_API_INIT()      (1) // Winsock/BSD/POSIX init simülasyonu
#define PLATFORM_SOCKET_API_SHUTDOWN()  // Winsock/BSD/POSIX cleanup simülasyonu
#define PLATFORM_SOCKET_CREATE(tcp)     (rand() % 1000 + 1) // Geçerli bir soket ID'si üret
#define PLATFORM_SOCKET_CLOSE(sock)     // Kapatma simülasyonu
#define PLATFORM_SOCKET_CONNECT(sock, ip, port) (rand() % 2) // Bağlantı başarılı/başarısız simülasyonu
#define PLATFORM_SOCKET_LISTEN(sock)    (0) // Dinleme başarılı simülasyonu
#define PLATFORM_SOCKET_SEND(sock, data, size) ((int32_t)size) // Gönderilen bayt sayısı simülasyonu
#define PLATFORM_SOCKET_RECV(sock, buf, size) (0) // Alınan bayt sayısı simülasyonu (0 veya > 0)


// ----------------------------------------------------------------------
// 2. SİSTEM YÖNETİMİ UYGULAMALARI
// ----------------------------------------------------------------------

/**
 * Uygulama: fe_network_init
 */
bool fe_network_init(void) {
    if (PLATFORM_SOCKET_API_INIT() != 0) {
        FE_LOG_INFO("Ağ Kütüphanesi baslatildi (Soket API simülasyonu).");
        return true;
    }
    FE_LOG_ERROR("Ağ Kütüphanesi baslatilamadi.");
    return false;
}

/**
 * Uygulama: fe_network_shutdown
 */
void fe_network_shutdown(void) {
    PLATFORM_SOCKET_API_SHUTDOWN();
    FE_LOG_INFO("Ağ Kütüphanesi kapatildi.");
}

/**
 * Uygulama: fe_network_update
 */
void fe_network_update(float dt) {
    // Gerçek uygulamada:
    // 1. Tüm açık soketleri kontrol etmek için select() veya poll() çağrılır.
    // 2. Dinleme soketinden yeni bağlantılar (accept) kabul edilir.
    // 3. Veri almaya hazır soketlerden fe_receive çağrılır ve mesajlar kuyruğa alınır.
    // 4. Kuyruktaki giden mesajlar gönderilir.
}


// ----------------------------------------------------------------------
// 3. BAĞLANTI İŞLEMLERİ UYGULAMALARI
// ----------------------------------------------------------------------

/**
 * Uygulama: fe_connect_to_server
 */
fe_session_t* fe_connect_to_server(const char* ip, uint16_t port, bool is_tcp) {
    fe_session_t* session = (fe_session_t*)calloc(1, sizeof(fe_session_t));
    if (!session) return NULL;
    
    session->sock = PLATFORM_SOCKET_CREATE(is_tcp);
    if (session->sock == FE_SOCKET_INVALID) {
        FE_LOG_ERROR("fe_connect: Soket olusturulamadi.");
        free(session);
        return NULL;
    }

    session->remote_ip = 0x01020304; // IP'nin int temsili simülasyonu
    session->remote_port = port;
    session->state = FE_CONN_CONNECTING;

    FE_LOG_INFO("İstemci: %s:%d adresine bağlanılıyor...", ip, port);
    
    // Gerçek bağlantı denemesi (Bloklama/Bloklamayan olabilir)
    if (PLATFORM_SOCKET_CONNECT(session->sock, ip, port) == 0) {
        session->state = FE_CONN_CONNECTED;
        FE_LOG_INFO("Bağlantı başarılı: %s:%d", ip, port);
    } else {
        // Bloklamayan soketlerde bu, genellikle hemen gerçekleşmez.
        FE_LOG_WARN("Bağlantı denemesi başlatıldı. Sonuç fe_network_update'te belli olacak.");
    }
    
    return session;
}

/**
 * Uygulama: fe_listen_on_port
 */
fe_session_t* fe_listen_on_port(uint16_t port, bool is_tcp) {
    fe_session_t* session = (fe_session_t*)calloc(1, sizeof(fe_session_t));
    if (!session) return NULL;
    
    session->sock = PLATFORM_SOCKET_CREATE(is_tcp);
    if (session->sock == FE_SOCKET_INVALID) {
        FE_LOG_ERROR("fe_listen: Soket olusturulamadi.");
        free(session);
        return NULL;
    }
    
    // Bağlama (bind) ve dinleme (listen) işlemleri burada gerçekleşir
    if (PLATFORM_SOCKET_LISTEN(session->sock) == 0) {
        session->remote_port = port;
        session->state = FE_CONN_LISTENING;
        FE_LOG_INFO("Sunucu dinlemede: Port %d (%s)", port, is_tcp ? "TCP" : "UDP");
    } else {
        FE_LOG_ERROR("fe_listen: Dinleme basarisiz.");
        PLATFORM_SOCKET_CLOSE(session->sock);
        free(session);
        return NULL;
    }
    
    return session;
}

/**
 * Uygulama: fe_disconnect
 */
void fe_disconnect(fe_session_t* session) {
    if (!session) return;
    
    if (session->sock != FE_SOCKET_INVALID) {
        PLATFORM_SOCKET_CLOSE(session->sock);
    }
    session->state = FE_CONN_DISCONNECTED;
    FE_LOG_INFO("Oturum kesildi (Port: %d).", session->remote_port);
    
    // fe_queue_destroy(&session->incoming_messages); // Gelen mesajları temizle
    free(session);
}


// ----------------------------------------------------------------------
// 4. VERİ İLETİMİ UYGULAMALARI
// ----------------------------------------------------------------------

/**
 * Uygulama: fe_send
 */
int32_t fe_send(fe_session_t* session, const void* data, size_t size) {
    if (!session || session->state != FE_CONN_CONNECTED) return -1;
    
    // Veri paketleme ve başlık ekleme mantığı burada yer alır.
    
    int32_t sent_bytes = PLATFORM_SOCKET_SEND(session->sock, data, size);
    if (sent_bytes < 0) {
        FE_LOG_WARN("Veri gönderme hatasi.");
    }
    return sent_bytes;
}

/**
 * Uygulama: fe_receive
 */
int32_t fe_receive(fe_session_t* session, void* buffer, size_t buffer_size) {
    if (!session || session->state != FE_CONN_CONNECTED) return -1;
    
    int32_t received_bytes = PLATFORM_SOCKET_RECV(session->sock, buffer, buffer_size);
    
    if (received_bytes > 0) {
        // Alınan veri paket çözücü (Packet De-serializer) tarafından işlenir.
        // Mesajlar burada fe_session_t::incoming_messages kuyruğuna alınır.
    }
    
    return received_bytes;
}