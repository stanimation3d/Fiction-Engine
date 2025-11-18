// include/graphics/fe_render_types.h

#ifndef FE_RENDER_TYPES_H
#define FE_RENDER_TYPES_H

#include <stdint.h>
#include <stdbool.h>

// Fiction Engine'in kullanacağı temel matematik kütüphanesini dahil ediyoruz (ileride fe_math.h olarak tanımlanacak)
// Şimdilik sadece float'lar ve matrisler için ön tanımlamalar yapıyoruz.
// typedef struct fe_vec3 { float x, y, z; } fe_vec3_t;
// typedef struct fe_mat4 { float m[16]; } fe_mat4_t;


// ----------------------------------------------------------------------
// 1. VERTEKS (Vertex) YAPILARI
// ----------------------------------------------------------------------

/**
 * @brief Standart 3D Vertex Yapısı.
 * Fiction Engine'in çoğu modeli bu formatı kullanacaktır.
 */
typedef struct fe_vertex {
    float position[3];  // Konum (X, Y, Z)
    float normal[3];    // Yüzey Normali (Nx, Ny, Nz)
    float texcoord[2];  // UV Koordinatları (U, V)
    float tangent[3];   // Teğet Vektörü (Normal Mapping için)
    uint8_t color[4];   // Renk (R, G, B, A) - 8 bit/kanal
} fe_vertex_t;


// ----------------------------------------------------------------------
// 2. BUFFER (Tampon) YAPILARI
// ----------------------------------------------------------------------

/**
 * @brief API'ye özgü genel Buffer Tanıtıcısı.
 * OpenGL'de VBO, EBO, UBO veya SSBO ID'lerini tutar.
 */
typedef uint32_t fe_buffer_id_t;


/**
 * @brief Buffer kullanım amacını tanımlar (GPU'ya nasıl yükleneceği).
 */
typedef enum fe_buffer_usage {
    FE_BUFFER_USAGE_STATIC,     // Veri nadiren değişir (Örn: Statik geometri)
    FE_BUFFER_USAGE_DYNAMIC,    // Veri sık sık değişir (Örn: Animasyon, DynamicR)
    FE_BUFFER_USAGE_STREAM      // Veri her karede değişir (Örn: Parçacık sistemleri)
} fe_buffer_usage_t;


/**
 * @brief Bir Görüntü Tamponu (Frame Buffer) Nesnesi.
 * Off-screen render (gölge haritaları, post-processing) için kullanılır.
 */
typedef struct fe_framebuffer {
    fe_buffer_id_t fbo_id;          // API'ye özgü Frame Buffer ID
    fe_buffer_id_t color_texture_id; // Renk tamponu (Kaplama olarak)
    fe_buffer_id_t depth_texture_id; // Derinlik tamponu (Kaplama olarak)
    int width;
    int height;
} fe_framebuffer_t;


// ----------------------------------------------------------------------
// 3. MESH (Kafes) YAPILARI
// ----------------------------------------------------------------------

/**
 * @brief 3D Geometriyi temsil eden temel yapı.
 * fe_gl_backend'in çizim komutları için gereken tüm ID'leri tutar.
 */
typedef struct fe_mesh {
    fe_buffer_id_t vao_id;         // Vertex Array Object ID (OpenGL)
    fe_buffer_id_t vertex_buffer_id; // Vertex Buffer Object ID (VBO)
    fe_buffer_id_t index_buffer_id;  // Element Buffer Object ID (EBO)
    
    uint32_t vertex_count;      // Toplam Vertex sayısı
    uint32_t index_count;       // Toplam Index sayısı
    
    // Yüksek Seviye Veri (CPU'da tutulursa)
    // fe_vertex_t* vertices;
    // uint32_t* indices;
} fe_mesh_t;


// ----------------------------------------------------------------------
// 4. SHADER VE KAPLAMA (Texture) YAPILARI
// ----------------------------------------------------------------------

/**
 * @brief Grafik API'sinde derlenmiş Shader Programı Tanıtıcısı.
 */
typedef fe_buffer_id_t fe_shader_id_t;


/**
 * @brief Grafik API'sinde yüklenmiş Kaplama Tanıtıcısı.
 */
typedef fe_buffer_id_t fe_texture_id_t;

/**
 * @brief Temel Kaplama formatları.
 */
typedef enum fe_texture_format {
    FE_TEXTURE_FORMAT_RGB8,
    FE_TEXTURE_FORMAT_RGBA8,
    FE_TEXTURE_FORMAT_D24S8, // Derinlik + Şablon (Depth + Stencil)
    FE_TEXTURE_FORMAT_COUNT
} fe_texture_format_t;

#endif // FE_RENDER_TYPES_H