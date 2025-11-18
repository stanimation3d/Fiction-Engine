// include/data_structures/fe_queue.h

#ifndef FE_QUEUE_H
#define FE_QUEUE_H

#include <stdint.h>
#include <stddef.h> // size_t için
#include <stdbool.h>

// ----------------------------------------------------------------------
// 1. KUYRUK YAPISI (fe_queue_t)
// ----------------------------------------------------------------------

/**
 * @brief Dinamik olarak büyüyebilen, dairesel tampon (circular buffer) tabanlı kuyruk yapısı.
 * * FIFO (First-In, First-Out) prensibiyle çalışır.
 */
typedef struct fe_queue {
    void* data;             // Veri elemanlarının bellek başlangıç adresi
    size_t element_size;    // Kuyruktaki her bir elemanın boyutu (byte)
    size_t head;            // Kuyrukta ilk elemanın indeksi (Dequeue buradan yapılır)
    size_t tail;            // Kuyrukta yeni elemanın ekleneceği indeksi (Enqueue buraya yapılır)
    size_t count;           // Kuyruktaki mevcut eleman sayısı
    size_t capacity;        // Kuyruğun mevcut bellek kapasitesi (eleman sayısı olarak)
} fe_queue_t;


// ----------------------------------------------------------------------
// 2. YÖNETİM VE YAŞAM DÖNGÜSÜ
// ----------------------------------------------------------------------

/**
 * @brief Yeni bir dairesel kuyruk olusturur.
 * @param element_size Kuyrugun saklayacagi veri tipinin boyutu.
 * @param initial_capacity Başlangıç kapasitesi (0 ise varsayilan kullanilir).
 * @return Yeni olusturulmus fe_queue_t* veya NULL.
 */
fe_queue_t* fe_queue_create(size_t element_size, size_t initial_capacity);

/**
 * @brief Kuyrugu bellekten serbest birakir.
 */
void fe_queue_destroy(fe_queue_t* q);

/**
 * @brief Kuyrugu temizler ancak bellek kapasitesini korur.
 */
void fe_queue_clear(fe_queue_t* q);


// ----------------------------------------------------------------------
// 3. FIFO İŞLEMLERİ
// ----------------------------------------------------------------------

/**
 * @brief Kuyrugun sonuna bir eleman ekler (Enqueue).
 * * Gerekirse kuyrugu yeniden boyutlandırır.
 * @param element_ptr Eklenecek verinin adresi.
 * @return Başarılıysa true, degilse false.
 */
bool fe_queue_enqueue(fe_queue_t* q, const void* element_ptr);

/**
 * @brief Kuyrugun basindan bir elemanı kaldirir ve döndürür (Dequeue).
 * @param dest_ptr Kaldırılan elemanın kopyalanacagi hedef adres (NULL olabilir).
 * @return Başarılıysa true, degilse false.
 */
bool fe_queue_dequeue(fe_queue_t* q, void* dest_ptr);

/**
 * @brief Kuyrugun başındaki elemanı kopyalar, ancak kuyruktan kaldırmaz (Peek).
 * @param dest_ptr Elemanın kopyalanacagi hedef adres.
 * @return Başarılıysa true, degilse false.
 */
bool fe_queue_peek(fe_queue_t* q, void* dest_ptr);

// ----------------------------------------------------------------------
// 4. BİLGİ FONKSİYONLARI
// ----------------------------------------------------------------------

#define fe_queue_count(q)       ((q) ? (q)->count : 0)
#define fe_queue_capacity(q)    ((q) ? (q)->capacity : 0)
#define fe_queue_is_empty(q)    ((q) ? ((q)->count == 0) : true)

#endif // FE_QUEUE_H