// include/data_structures/fe_array.h

#ifndef FE_ARRAY_H
#define FE_ARRAY_H

#include <stdint.h>
#include <stddef.h> // size_t için
#include <stdbool.h>

// ----------------------------------------------------------------------
// 1. DİNAMİK DİZİ YAPISI (fe_array_t)
// ----------------------------------------------------------------------

/**
 * @brief Dinamik olarak büyüyebilen, tip güvenli olmayan (void* tabanlı) dizi yapisi.
 * * Verimli rastgele erisim ve hizli ekleme/silme (sonda) saglar.
 */
typedef struct fe_array {
    void* data;             // Veri elemanlarının bellek başlangıç adresi
    size_t element_size;    // Dizideki her bir elemanın boyutu (byte cinsinden)
    size_t count;           // Dizideki mevcut eleman sayısı
    size_t capacity;        // Dizinin mevcut bellek kapasitesi (tahsis edilen eleman sayısı)
} fe_array_t;

// ----------------------------------------------------------------------
// 2. YÖNETİM VE YAŞAM DÖNGÜSÜ
// ----------------------------------------------------------------------

/**
 * @brief Yeni bir dinamik dizi olusturur.
 * @param element_size Dizinin saklayacagi veri tipinin boyutu (sizeof(MyStruct)).
 * @return Yeni olusturulmus fe_array_t* veya NULL (hata durumunda).
 */
fe_array_t* fe_array_create(size_t element_size);

/**
 * @brief Diziyi bellekten serbest birakir.
 */
void fe_array_destroy(fe_array_t* arr);

/**
 * @brief Diziyi temizler ancak bellek kapasitesini (capacity) korur.
 */
void fe_array_clear(fe_array_t* arr);

// ----------------------------------------------------------------------
// 3. ERİŞİM VE MANİPÜLASYON
// ----------------------------------------------------------------------

/**
 * @brief Dizinin sonuna bir eleman ekler (push).
 * * Gerekirse diziyi yeniden boyutlandırır.
 * @param element_ptr Eklenecek verinin adresi.
 * @return Basariliysa true, degilse false.
 */
bool fe_array_push(fe_array_t* arr, const void* element_ptr);

/**
 * @brief Dizinin belirtilen indeksindeki elemanı döndürür.
 * * @param index Erisilecek indeks.
 * @return Elemanin adresi (void*) veya NULL (indeks geçersizse).
 */
void* fe_array_get(fe_array_t* arr, size_t index);

/**
 * @brief Diziden son elemanı kaldırır (pop).
 * @param dest_ptr Kaldirilan elemanin kopyalanacagi hedef adres (NULL olabilir).
 * @return Basariliysa true, degilse false.
 */
bool fe_array_pop(fe_array_t* arr, void* dest_ptr);

/**
 * @brief Dizinin belirtilen indeksindeki elemanı kaldırır ve diziyi sıkıştırır.
 * * Dikkat: O(N) karmaşıklığa sahiptir.
 * @param index Silinecek elemanın indeksi.
 * @param dest_ptr Kaldirilan elemanin kopyalanacagi hedef adres (NULL olabilir).
 * @return Basariliysa true, degilse false.
 */
bool fe_array_remove_at(fe_array_t* arr, size_t index, void* dest_ptr);

// ----------------------------------------------------------------------
// 4. BİLGİ FONKSİYONLARI (Inline olarak tanımlanabilir)
// ----------------------------------------------------------------------

#define fe_array_count(arr)     ((arr) ? (arr)->count : 0)
#define fe_array_capacity(arr)  ((arr) ? (arr)->capacity : 0)
#define fe_array_is_empty(arr)  ((arr) ? ((arr)->count == 0) : true)

#endif // FE_ARRAY_H