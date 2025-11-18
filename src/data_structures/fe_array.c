// src/data_structures/fe_array.c

#include "data_structures/fe_array.h"
#include "utils/fe_logger.h" // Hata ve bilgi loglaması için
#include <stdlib.h> // malloc, realloc, free
#include <string.h> // memcpy, memmove

// Yeniden boyutlandırma için varsayılan başlangıç kapasitesi
#define FE_ARRAY_DEFAULT_CAPACITY 8
// Yeniden boyutlandırma çarpanı
#define FE_ARRAY_RESIZE_FACTOR 2

/**
 * @brief Dizinin kapasitesini iki katına çıkarır.
 * @return Basariliysa true, degilse false.
 */
static bool fe_array_grow(fe_array_t* arr) {
    size_t new_capacity = arr->capacity * FE_ARRAY_RESIZE_FACTOR;
    
    // Minimum kapasiteyi koru
    if (new_capacity < arr->capacity + 1) { 
        new_capacity = arr->capacity + 1; // Taşma durumunu önle
    }
    
    // Yeniden tahsis (realloc)
    void* new_data = realloc(arr->data, new_capacity * arr->element_size);
    
    if (new_data == NULL) {
        FE_LOG_ERROR("fe_array: Bellek tahsisi basarisiz oldu. Kapasite %zu", new_capacity);
        return false;
    }
    
    arr->data = new_data;
    arr->capacity = new_capacity;
    
    return true;
}


// ----------------------------------------------------------------------
// 2. YÖNETİM VE YAŞAM DÖNGÜSÜ UYGULAMALARI
// ----------------------------------------------------------------------

/**
 * Uygulama: fe_array_create
 */
fe_array_t* fe_array_create(size_t element_size) {
    if (element_size == 0) {
        FE_LOG_ERROR("fe_array: Element boyutu sifir olamaz.");
        return NULL;
    }
    
    fe_array_t* arr = (fe_array_t*)calloc(1, sizeof(fe_array_t));
    if (!arr) return NULL;
    
    arr->element_size = element_size;
    arr->count = 0;
    arr->capacity = FE_ARRAY_DEFAULT_CAPACITY;
    
    // Başlangıç belleği tahsis et
    arr->data = malloc(arr->capacity * element_size);
    
    if (!arr->data) {
        FE_LOG_ERROR("fe_array: Ilk bellek tahsisi basarisiz oldu.");
        free(arr);
        return NULL;
    }
    
    return arr;
}

/**
 * Uygulama: fe_array_destroy
 */
void fe_array_destroy(fe_array_t* arr) {
    if (arr) {
        if (arr->data) {
            free(arr->data);
        }
        free(arr);
    }
}

/**
 * Uygulama: fe_array_clear
 */
void fe_array_clear(fe_array_t* arr) {
    if (arr) {
        arr->count = 0;
        // Kapasite korunur, data silinmez.
    }
}


// ----------------------------------------------------------------------
// 3. ERİŞİM VE MANİPÜLASYON UYGULAMALARI
// ----------------------------------------------------------------------

/**
 * Uygulama: fe_array_push
 */
bool fe_array_push(fe_array_t* arr, const void* element_ptr) {
    if (!arr || !element_ptr) return false;
    
    // Kapasite kontrolü
    if (arr->count >= arr->capacity) {
        if (!fe_array_grow(arr)) {
            return false;
        }
    }
    
    // Veriyi sona kopyala
    void* dest = (char*)arr->data + arr->count * arr->element_size;
    memcpy(dest, element_ptr, arr->element_size);
    
    arr->count++;
    return true;
}

/**
 * Uygulama: fe_array_get
 */
void* fe_array_get(fe_array_t* arr, size_t index) {
    if (!arr || index >= arr->count) {
        return NULL; // Geçersiz indeks
    }
    
    // İstenen elemanın bellekteki adresini hesapla
    return (char*)arr->data + index * arr->element_size;
}

/**
 * Uygulama: fe_array_pop
 */
bool fe_array_pop(fe_array_t* arr, void* dest_ptr) {
    if (fe_array_is_empty(arr)) {
        return false;
    }
    
    arr->count--;
    
    // Eğer istenirse, kaldırılan elemanı kopyala
    if (dest_ptr != NULL) {
        void* src = (char*)arr->data + arr->count * arr->element_size;
        memcpy(dest_ptr, src, arr->element_size);
    }
    
    // Kapasiteyi küçültme (Shrink) işlemi genellikle pop'ta yapılmaz, manuel yapılır.
    return true;
}

/**
 * Uygulama: fe_array_remove_at
 */
bool fe_array_remove_at(fe_array_t* arr, size_t index, void* dest_ptr) {
    if (!arr || index >= arr->count) {
        return false;
    }
    
    // 1. Silinecek elemanı kopyala (istenirse)
    if (dest_ptr != NULL) {
        void* src = (char*)arr->data + index * arr->element_size;
        memcpy(dest_ptr, src, arr->element_size);
    }
    
    // 2. Kalan elemanları kaydır (Shift - O(N) maliyetli)
    size_t num_elements_to_move = arr->count - index - 1;
    
    if (num_elements_to_move > 0) {
        void* dest_start = (char*)arr->data + index * arr->element_size;
        void* src_start = (char*)arr->data + (index + 1) * arr->element_size;
        
        // memmove, kaynak ve hedef bellek alanları çakışsa bile güvenlidir.
        memmove(dest_start, src_start, num_elements_to_move * arr->element_size);
    }
    
    arr->count--;
    return true;
}
