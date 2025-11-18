// src/data_structures/fe_stack.c

#include "data_structures/fe_stack.h"
#include "utils/fe_logger.h" // Hata loglaması için
#include <stdlib.h> // malloc, realloc, free
#include <string.h> // memcpy

// Yeniden boyutlandırma için varsayılan başlangıç kapasitesi
#define FE_STACK_DEFAULT_CAPACITY 8
// Yeniden boyutlandırma çarpanı
#define FE_STACK_RESIZE_FACTOR 2

/**
 * @brief Yığının kapasitesini iki katına çıkarır.
 * @return Basariliysa true, degilse false.
 */
static bool fe_stack_grow(fe_stack_t* s) {
    size_t new_capacity = s->capacity * FE_STACK_RESIZE_FACTOR;
    
    // Minimum kapasiteyi koru
    if (new_capacity < s->count + 1) { 
        new_capacity = s->count + 1;
    }
    
    // Yeniden tahsis (realloc)
    void* new_data = realloc(s->data, new_capacity * s->element_size);
    
    if (new_data == NULL) {
        FE_LOG_ERROR("fe_stack: Bellek tahsisi basarisiz oldu. Kapasite %zu", new_capacity);
        return false;
    }
    
    s->data = new_data;
    s->capacity = new_capacity;
    
    return true;
}


// ----------------------------------------------------------------------
// 2. YÖNETİM VE YAŞAM DÖNGÜSÜ UYGULAMALARI
// ----------------------------------------------------------------------

/**
 * Uygulama: fe_stack_create
 */
fe_stack_t* fe_stack_create(size_t element_size) {
    if (element_size == 0) {
        FE_LOG_ERROR("fe_stack: Element boyutu sifir olamaz.");
        return NULL;
    }
    
    fe_stack_t* s = (fe_stack_t*)calloc(1, sizeof(fe_stack_t));
    if (!s) return NULL;
    
    s->element_size = element_size;
    s->count = 0;
    s->capacity = FE_STACK_DEFAULT_CAPACITY;
    
    // Başlangıç belleği tahsis et
    s->data = malloc(s->capacity * element_size);
    
    if (!s->data) {
        FE_LOG_ERROR("fe_stack: Ilk bellek tahsisi basarisiz oldu.");
        free(s);
        return NULL;
    }
    
    return s;
}

/**
 * Uygulama: fe_stack_destroy
 */
void fe_stack_destroy(fe_stack_t* s) {
    if (s) {
        if (s->data) {
            free(s->data);
        }
        free(s);
    }
}

/**
 * Uygulama: fe_stack_clear
 */
void fe_stack_clear(fe_stack_t* s) {
    if (s) {
        s->count = 0;
    }
}


// ----------------------------------------------------------------------
// 3. LIFO İŞLEMLERİ UYGULAMALARI
// ----------------------------------------------------------------------

/**
 * Uygulama: fe_stack_push
 */
bool fe_stack_push(fe_stack_t* s, const void* element_ptr) {
    if (!s || !element_ptr) return false;
    
    // Kapasite kontrolü
    if (s->count >= s->capacity) {
        if (!fe_stack_grow(s)) {
            return false;
        }
    }
    
    // Veriyi yığının tepesine (current count index) kopyala
    void* dest = (char*)s->data + s->count * s->element_size;
    memcpy(dest, element_ptr, s->element_size);
    
    s->count++;
    return true;
}

/**
 * Uygulama: fe_stack_pop
 */
bool fe_stack_pop(fe_stack_t* s, void* dest_ptr) {
    if (fe_stack_is_empty(s)) {
        return false;
    }
    
    s->count--;
    
    // Kaldırılan elemanın adresi (artık count - 1'de)
    void* src = (char*)s->data + s->count * s->element_size;
    
    // Eğer istenirse, kaldırılan elemanı kopyala
    if (dest_ptr != NULL) {
        memcpy(dest_ptr, src, s->element_size);
    }
    
    return true;
}

/**
 * Uygulama: fe_stack_peek
 */
bool fe_stack_peek(fe_stack_t* s, void* dest_ptr) {
    if (fe_stack_is_empty(s) || !dest_ptr) {
        return false;
    }
    
    // En üstteki elemanın adresi (count - 1)
    void* src = (char*)s->data + (s->count - 1) * s->element_size;
    
    // Veriyi kopyala
    memcpy(dest_ptr, src, s->element_size);
    
    return true;
}