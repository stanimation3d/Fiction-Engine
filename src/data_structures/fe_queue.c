// src/data_structures/fe_queue.c

#include "data_structures/fe_queue.h"
#include "utils/fe_logger.h" // Hata loglaması için
#include <stdlib.h> // malloc, realloc, free
#include <string.h> // memcpy, memset

// Yeniden boyutlandırma için varsayılan başlangıç kapasitesi
#define FE_QUEUE_DEFAULT_CAPACITY 8
// Yeniden boyutlandırma çarpanı
#define FE_QUEUE_RESIZE_FACTOR 2

/**
 * @brief Kuyruğun kapasitesini genişletir (Yeniden tahsis).
 * * Dairesel tampon mantığının korunması için veriler sıralı bir şekilde yeniden düzenlenir.
 * @return Başarılıysa true, degilse false.
 */
static bool fe_queue_grow(fe_queue_t* q) {
    size_t new_capacity = q->capacity * FE_QUEUE_RESIZE_FACTOR;
    
    // Yeni kapasite mevcut eleman sayısından küçük olamaz.
    if (new_capacity < q->count) {
        new_capacity = q->count + 1; 
    }
    
    void* new_data = malloc(new_capacity * q->element_size);
    if (new_data == NULL) {
        FE_LOG_ERROR("fe_queue: Bellek tahsisi basarisiz oldu. Kapasite %zu", new_capacity);
        return false;
    }
    
    // Verileri yeni tampona sıralı olarak kopyala (Dairesel tamponu düzleştir)
    if (q->count > 0) {
        // 1. Elemanları baştan (head) sona (capacity-1) kadar kopyala
        size_t first_segment_size = q->capacity - q->head;
        size_t first_copy_count = (first_segment_size > q->count) ? q->count : first_segment_size;
        
        // Kaynak adres: head * element_size
        void* src1 = (char*)q->data + q->head * q->element_size;
        // Hedef adres: 0
        memcpy(new_data, src1, first_copy_count * q->element_size);
        
        // 2. Eğer tamponun sonunda kesiklik varsa, kalan elemanları kopyala
        if (q->count > first_copy_count) {
            size_t second_copy_count = q->count - first_copy_count;
            // Kaynak adres: 0
            void* src2 = q->data;
            // Hedef adres: first_copy_count * element_size
            void* dest2 = (char*)new_data + first_copy_count * q->element_size;
            memcpy(dest2, src2, second_copy_count * q->element_size);
        }
    }

    // Eski belleği serbest bırak ve yeni verileri ata
    free(q->data);
    q->data = new_data;
    q->capacity = new_capacity;
    
    // Kuyruk düzleştirildiği için head ve tail indeksleri sıfırlanır
    q->head = 0;
    q->tail = q->count; 
    
    return true;
}

/**
 * @brief Dairesel tamponun indeksini hesaplar (Sıradaki indeks).
 */
static size_t fe_queue_next_index(fe_queue_t* q, size_t index) {
    return (index + 1) % q->capacity;
}


// ----------------------------------------------------------------------
// 2. YÖNETİM VE YAŞAM DÖNGÜSÜ UYGULAMALARI
// ----------------------------------------------------------------------

/**
 * Uygulama: fe_queue_create
 */
fe_queue_t* fe_queue_create(size_t element_size, size_t initial_capacity) {
    if (element_size == 0) {
        FE_LOG_ERROR("fe_queue: Element boyutu sifir olamaz.");
        return NULL;
    }
    
    fe_queue_t* q = (fe_queue_t*)calloc(1, sizeof(fe_queue_t));
    if (!q) return NULL;
    
    q->element_size = element_size;
    q->capacity = (initial_capacity > 0) ? initial_capacity : FE_QUEUE_DEFAULT_CAPACITY;
    q->count = 0;
    q->head = 0;
    q->tail = 0;
    
    q->data = malloc(q->capacity * element_size);
    
    if (!q->data) {
        FE_LOG_ERROR("fe_queue: Ilk bellek tahsisi basarisiz oldu.");
        free(q);
        return NULL;
    }
    
    return q;
}

/**
 * Uygulama: fe_queue_destroy
 */
void fe_queue_destroy(fe_queue_t* q) {
    if (q) {
        if (q->data) {
            free(q->data);
        }
        free(q);
    }
}

/**
 * Uygulama: fe_queue_clear
 */
void fe_queue_clear(fe_queue_t* q) {
    if (q) {
        q->count = 0;
        q->head = 0;
        q->tail = 0;
    }
}


// ----------------------------------------------------------------------
// 3. FIFO İŞLEMLERİ UYGULAMALARI
// ----------------------------------------------------------------------

/**
 * Uygulama: fe_queue_enqueue
 */
bool fe_queue_enqueue(fe_queue_t* q, const void* element_ptr) {
    if (!q || !element_ptr) return false;
    
    // Kapasite kontrolü
    if (q->count == q->capacity) {
        if (!fe_queue_grow(q)) {
            return false;
        }
    }
    
    // Veriyi kuyruk indeksine (tail) kopyala
    void* dest = (char*)q->data + q->tail * q->element_size;
    memcpy(dest, element_ptr, q->element_size);
    
    // Tail'ı dairesel olarak ilerlet
    q->tail = fe_queue_next_index(q, q->tail);
    q->count++;
    
    return true;
}

/**
 * Uygulama: fe_queue_dequeue
 */
bool fe_queue_dequeue(fe_queue_t* q, void* dest_ptr) {
    if (fe_queue_is_empty(q)) {
        return false;
    }
    
    // Kaldırılacak elemanın adresi
    void* src = (char*)q->data + q->head * q->element_size;
    
    // Eğer istenirse, elemanı kopyala
    if (dest_ptr != NULL) {
        memcpy(dest_ptr, src, q->element_size);
    }
    
    // Head'i dairesel olarak ilerlet
    q->head = fe_queue_next_index(q, q->head);
    q->count--;
    
    // Not: Kapasite küçültme (shrink) işlemi genellikle manuel yapılır veya bu fonksiyonda tetiklenmez.
    return true;
}

/**
 * Uygulama: fe_queue_peek
 */
bool fe_queue_peek(fe_queue_t* q, void* dest_ptr) {
    if (fe_queue_is_empty(q) || !dest_ptr) {
        return false;
    }
    
    // Başlangıç (head) adresini bul
    void* src = (char*)q->data + q->head * q->element_size;
    
    // Veriyi kopyala
    memcpy(dest_ptr, src, q->element_size);
    
    return true;
}