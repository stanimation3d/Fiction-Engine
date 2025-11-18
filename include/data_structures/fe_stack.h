// include/data_structures/fe_stack.h

#ifndef FE_STACK_H
#define FE_STACK_H

#include <stdint.h>
#include <stddef.h> // size_t için
#include <stdbool.h>

// ----------------------------------------------------------------------
// 1. YIĞIN YAPISI (fe_stack_t)
// ----------------------------------------------------------------------

/**
 * @brief Dinamik olarak büyüyebilen, dizi tabanlı Yığın yapısı.
 * * LIFO (Last-In, First-Out) prensibiyle çalışır.
 * * fe_array_t yapısına benzer, ancak sadece sonuna erisime izin verir.
 */
typedef struct fe_stack {
    void* data;             // Veri elemanlarının bellek başlangıç adresi
    size_t element_size;    // Yığındaki her bir elemanın boyutu (byte)
    size_t count;           // Yığındaki mevcut eleman sayısı
    size_t capacity;        // Yığının mevcut bellek kapasitesi
} fe_stack_t;


// ----------------------------------------------------------------------
// 2. YÖNETİM VE YAŞAM DÖNGÜSÜ
// ----------------------------------------------------------------------

/**
 * @brief Yeni bir Yığın olusturur.
 * @param element_size Yığının saklayacagi veri tipinin boyutu.
 * @return Yeni olusturulmus fe_stack_t* veya NULL.
 */
fe_stack_t* fe_stack_create(size_t element_size);

/**
 * @brief Yığını bellekten serbest birakir.
 */
void fe_stack_destroy(fe_stack_t* s);

/**
 * @brief Yığını temizler (sayaci sifirlar) ancak bellek kapasitesini korur.
 */
void fe_stack_clear(fe_stack_t* s);


// ----------------------------------------------------------------------
// 3. LIFO İŞLEMLERİ
// ----------------------------------------------------------------------

/**
 * @brief Yığının tepesine bir eleman ekler (Push).
 * * Gerekirse yığını yeniden boyutlandırır.
 * @param element_ptr Eklenecek verinin adresi.
 * @return Başarılıysa true, degilse false.
 */
bool fe_stack_push(fe_stack_t* s, const void* element_ptr);

/**
 * @brief Yığının tepesinden bir elemanı kaldirir ve döndürür (Pop).
 * @param dest_ptr Kaldırılan elemanın kopyalanacagi hedef adres (NULL olabilir).
 * @return Başarılıysa true, degilse false.
 */
bool fe_stack_pop(fe_stack_t* s, void* dest_ptr);

/**
 * @brief Yığının tepesindeki elemanı kopyalar, ancak yığından kaldırmaz (Peek).
 * @param dest_ptr Elemanın kopyalanacagi hedef adres.
 * @return Başarılıysa true, degilse false.
 */
bool fe_stack_peek(fe_stack_t* s, void* dest_ptr);

// ----------------------------------------------------------------------
// 4. BİLGİ FONKSİYONLARI
// ----------------------------------------------------------------------

#define fe_stack_count(s)       ((s) ? (s)->count : 0)
#define fe_stack_capacity(s)    ((s) ? (s)->capacity : 0)
#define fe_stack_is_empty(s)    ((s) ? ((s)->count == 0) : true)

#endif // FE_STACK_H