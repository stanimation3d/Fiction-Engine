// include/graphics/opengl/fe_gl_commands.h

#ifndef FE_GL_COMMANDS_H
#define FE_GL_COMMANDS_H

#include <stdint.h>
#include "graphics/fe_render_types.h" // fe_buffer_id_t için

// ----------------------------------------------------------------------
// 1. BAĞLAMA (Binding) KOMUTLARI
// ----------------------------------------------------------------------

/**
 * @brief Bir Vertex Array Object (VAO) baglar.
 * * Bir mesh'i cizmeden once cagrılmalıdır.
 * @param vao_id Baglanacak VAO ID'si.
 */
void fe_gl_cmd_bind_vao(fe_buffer_id_t vao_id);

/**
 * @brief Gecici olarak VAO baglantisini cozer (glBindVertexArray(0)).
 */
void fe_gl_cmd_unbind_vao(void);

/**
 * @brief Bir Shader Programini aktif hale getirir.
 * * Bu cagrı aslinda fe_shader_compiler.c'de uygulanmistir, ancak 
 * * baglami korumak icin buraya eklenmistir. (glUseProgram).
 * @param program_id Baglanacak Shader Program ID'si.
 */
void fe_gl_cmd_bind_shader(fe_shader_id_t program_id);

/**
 * @brief Bir Kaplamayi (Texture) belirli bir birime baglar.
 * @param texture_id Baglanacak Kaplama ID'si.
 * @param texture_unit Aktiflestirilecek doku birimi (0'dan baslar).
 */
void fe_gl_cmd_bind_texture(fe_texture_id_t texture_id, uint32_t texture_unit);


// ----------------------------------------------------------------------
// 2. ÇİZİM (Drawing) KOMUTLARI
// ----------------------------------------------------------------------

/**
 * @brief Index Buffer (EBO) kullanarak cizim yapar (glDrawElements).
 * * @param index_count Cizilecek toplam index sayisi.
 * @param primitive_type Cizim primitif tipi (GL_TRIANGLES, GL_LINES vb. - iceride GL_TRIANGLES varsayilir).
 */
void fe_gl_cmd_draw_indexed(uint32_t index_count, uint32_t primitive_type);

/**
 * @brief Instancing (Ornekleme) kullanarak index buffer ile cizim yapar (glDrawElementsInstanced).
 * * @param index_count Cizilecek toplam index sayisi.
 * @param instance_count Kac adet ornek cizilecegi.
 * @param primitive_type Cizim primitif tipi.
 */
void fe_gl_cmd_draw_indexed_instanced(uint32_t index_count, uint32_t instance_count, uint32_t primitive_type);

/**
 * @brief Index Buffer kullanmadan dogrudan Vertex Buffer'dan cizim yapar (glDrawArrays).
 * * @param vertex_count Cizilecek toplam vertex sayisi.
 * @param primitive_type Cizim primitif tipi.
 */
void fe_gl_cmd_draw_arrays(uint32_t vertex_count, uint32_t primitive_type);


#endif // FE_GL_COMMANDS_H