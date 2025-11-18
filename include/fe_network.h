// include/network/fe_network.h

#ifndef FE_NETWORK_H
#define FE_NETWORK_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "data_structures/fe_queue.h" // Gelen mesaj kuyruğu için

// ----------------------------------------------------------------------
// 1. TEMEL TİPLER VE SABİTLER
// ----------------------------------------------------------------------

/**
 * @brief Temel soket tanımlayıcısı (Windows'ta SOCKET, Unix'te int).
 * * Basitlik için burada uint32_t kullanılmıştır, gerçekte platforma göre değişir.
 */
typedef uint32_t fe_socket_t;
#define FE_SOCKET_INVALID 0

/**
 * @brief Ağ bağlantısının mevcut durumu.
 */
typedef enum fe_connection_state {
    FE_CONN_DISCONNECTED,   // Bağlantı yok
    FE_CONN_CONNECTING,     // Bağlanmaya çalışılıyor
    FE_CONN_CONNECTED,      // Bağlantı aktif
    FE_CONN_LISTENING       // Sunucu dinlemede
} fe_connection_state_t;

/**
 * @brief Bir ağ oturumunu (soket, durum, adres) temsil eder.
 */
typedef struct fe_session {
    fe_socket_t sock;
    fe_connection_state_t state;
    uint32_t remote_ip;     // Uzak IP adresi
    uint16_t remote_port;   // Uzak port
    // fe_queue_t incoming_messages; // Gelen mesajlar için kuyruk
} fe_session_t;


// ----------------------------------------------------------------------
// 2. SİSTEM YÖNETİMİ
// ----------------------------------------------------------------------

/**
 * @brief Ağ kütüphanesini baslatir (örn: Winsock'u Windows'ta baslatir).
 * @return Başarılıysa true, degilse false.
 */
bool fe_network_init(void);

/**
 * @brief Ağ kütüphanesini kapatir.
 */
void fe_network_shutdown(void);

/**
 * @brief Tüm aktif oturumlari (session) kontrol eder, mesajlari alir ve gönderir.
 * * Motorun ana döngüsünde çağrılır.
 */
void fe_network_update(float dt);

// ----------------------------------------------------------------------
// 3. BAĞLANTI İŞLEMLERİ (İstemci/Sunucu)
// ----------------------------------------------------------------------

/**
 * @brief İstemci olarak uzak bir sunucuya bağlanır (TCP/UDP).
 * @param ip Uzak sunucunun IP adresi (string).
 * @param port Bağlanılacak port numarası.
 * @param is_tcp TCP kullanılıyorsa true, UDP kullanılıyorsa false.
 * @return Yeni olusturulan fe_session_t* veya NULL (hata).
 */
fe_session_t* fe_connect_to_server(const char* ip, uint16_t port, bool is_tcp);

/**
 * @brief Sunucu olarak gelen bağlantıları dinlemeye başlar.
 * @param port Dinlenecek port numarası.
 * @param is_tcp TCP kullanılıyorsa true, UDP kullanılıyorsa false.
 * @return Yeni dinleme fe_session_t* veya NULL.
 */
fe_session_t* fe_listen_on_port(uint16_t port, bool is_tcp);

/**
 * @brief Belirtilen oturumu kapatir ve bellekten serbest birakir.
 */
void fe_disconnect(fe_session_t* session);

// ----------------------------------------------------------------------
// 4. VERİ İLETİMİ
// ----------------------------------------------------------------------

/**
 * @brief Oturum üzerinden bir mesaj gönderir.
 * @param session Hedef oturum.
 * @param data Gönderilecek ham veri tamponu.
 * @param size Gönderilecek verinin boyutu.
 * @return Gönderilen bayt sayısı veya -1 (hata).
 */
int32_t fe_send(fe_session_t* session, const void* data, size_t size);

/**
 * @brief Oturum üzerinden veri alır.
 * * Non-blocking (Bloklamayan) çalıştığı varsayılır.
 * @param session Kaynak oturum.
 * @param buffer Verinin kopyalanacagi tampon.
 * @param buffer_size Tamponun maksimum boyutu.
 * @return Alınan bayt sayısı veya -1 (hata).
 */
int32_t fe_receive(fe_session_t* session, void* buffer, size_t buffer_size);

#endif // FE_NETWORK_H