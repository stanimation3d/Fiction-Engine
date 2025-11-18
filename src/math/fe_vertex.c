// src/math/fe_vertex.c

#include "math/fe_vertex.h"
#include <string.h> // memcpy/memset için

// Vektör sıfır tanımlamaları (fe_vector.h dosyasında olması beklenir)
extern const fe_vec3_t FE_VEC3_ZERO; 
extern const fe_vec2_t FE_VEC2_ZERO; 

// ----------------------------------------------------------------------
// 1. YÖNETİM UYGULAMALARI
// ----------------------------------------------------------------------

/**
 * Uygulama: fe_vertex_create
 */
fe_vertex_t fe_vertex_create(fe_vec3_t pos, fe_vec3_t norm, fe_vec2_t uv) {
    fe_vertex_t new_vertex;
    
    // Konum, Normal ve UV ataması
    new_vertex.position = pos;
    new_vertex.normal = norm;
    new_vertex.uv = uv;

    // Diğerlerini varsayılan/sıfır olarak ayarla
    new_vertex.color = (fe_color_t){1.0f, 1.0f, 1.0f, 1.0f}; // Beyaz renk (Varsayılan)
    
    // TBN (Teğet, İkincil Normal) sıfır olarak ayarlanır, sonradan hesaplanması gerekir.
    // fe_vec3_t sıfır tanımlarının mevcut olduğu varsayılır
    new_vertex.tangent = (fe_vec3_t){0.0f, 0.0f, 0.0f}; 
    new_vertex.bitangent = (fe_vec3_t){0.0f, 0.0f, 0.0f};
    
    return new_vertex;
}

/**
 * Uygulama: fe_vertex_copy
 */
void fe_vertex_copy(fe_vertex_t* dest, const fe_vertex_t* src) {
    if (!dest || !src) return;
    // Tüm yapıyı tek seferde kopyala
    memcpy(dest, src, sizeof(fe_vertex_t));
}

/**
 * Uygulama: fe_vertex_set_tbn
 */
void fe_vertex_set_tbn(fe_vertex_t* vertex, fe_vec3_t tangent, fe_vec3_t bitangent) {
    if (!vertex) return;
    
    // TBN ataması
    vertex->tangent = tangent;
    vertex->bitangent = bitangent;
    
    // NOT: Normal haritalama için TBN (Teğet-İkincil Normal-Normal) matrisi kullanılır.
    // TBN matrisi, normal haritadaki (2D) rengi (normal vektörü), dünya uzayına (3D) dönüştürür.
}