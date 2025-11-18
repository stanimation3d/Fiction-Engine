// include/graphics/fe_shader_compiler.h

#ifndef FE_SHADER_COMPILER_H
#define FE_SHADER_COMPILER_H

#include <stdint.h>
#include <stdbool.h>
#include "error/fe_error.h"
#include "graphics/fe_render_types.h" // fe_shader_id_t için

// ----------------------------------------------------------------------
// 1. SHADER YAPISI
// ----------------------------------------------------------------------

/**
 * @brief Derlenmis bir Shader Programini temsil eden yapi.
 * * Raylib'in RGL.h yapilarina benzer, ancak motorun kendi tipini kullaniriz.
 */
typedef struct fe_shader {
    fe_shader_id_t id;      // GPU'daki OpenGL Program ID'si
    char name[64];          // Hata ayıklama/loglama için ad
    // İleride Uniform Konumları (Location) buraya eklenecektir.
} fe_shader_t;


// ----------------------------------------------------------------------
// 2. YÖNETİM VE KULLANIM FONKSİYONLARI
// ----------------------------------------------------------------------

/**
 * @brief Verilen dosya yollarından Vertex ve Fragment shader'lari derler ve baglar.
 * * @param vs_path Vertex shader dosya yolu.
 * @param fs_path Fragment shader dosya yolu.
 * @return Basari durumunda derlenen shader programinin ID'si, aksi takdirde 0.
 */
fe_shader_id_t fe_shader_load(const char* vs_path, const char* fs_path);

/**
 * @brief GPU'daki bir shader programini sistemden kaldirir.
 */
void fe_shader_unload(fe_shader_id_t shader_id);

/**
 * @brief Belirtilen shader programini aktif hale getirir (glUseProgram).
 */
void fe_shader_use(fe_shader_id_t shader_id);

/**
 * @brief Aktif shader programini devre disi birakir.
 */
void fe_shader_unuse(void);


// ----------------------------------------------------------------------
// 3. UNIFORM AYARLAMA FONKSİYONLARI
// ----------------------------------------------------------------------

/**
 * @brief Aktif shader'da bir float uniform degerini ayarlar.
 * * @param name Uniform degiskeninin adini.
 * @param value Ayarlanacak float degeri.
 */
void fe_shader_set_uniform_float(const char* name, float value);

/**
 * @brief Aktif shader'da bir integer (sampler) uniform degerini ayarlar.
 */
void fe_shader_set_uniform_int(const char* name, int value);

// TODO: fe_mat4_t ve fe_vec3_t uniform ayarlama fonksiyonlari ileride eklenecektir.

#endif // FE_SHADER_COMPILER_H