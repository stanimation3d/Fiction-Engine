// src/graphics/opengl/fe_gl_commands.c

#include "graphics/opengl/fe_gl_commands.h"
#include "utils/fe_logger.h"
#include <GL/gl.h> // Doğrudan OpenGL komutları için


// ----------------------------------------------------------------------
// 1. BAĞLAMA (Binding) KOMUTLARI
// ----------------------------------------------------------------------

/**
 * Uygulama: fe_gl_cmd_bind_vao
 */
void fe_gl_cmd_bind_vao(fe_buffer_id_t vao_id) {
    // Hata kontrolü yok: Hız için optimize edildi
    glBindVertexArray(vao_id);
}

/**
 * Uygulama: fe_gl_cmd_unbind_vao
 */
void fe_gl_cmd_unbind_vao(void) {
    glBindVertexArray(0);
}

/**
 * Uygulama: fe_gl_cmd_bind_shader
 */
void fe_gl_cmd_bind_shader(fe_shader_id_t program_id) {
    // glUseProgram, glUseProgram(0) cagrısını icerir.
    // fe_shader_compiler.c'deki fe_shader_use fonksiyonu bu islevi görür.
    glUseProgram(program_id);
}

/**
 * Uygulama: fe_gl_cmd_bind_texture
 */
void fe_gl_cmd_bind_texture(fe_texture_id_t texture_id, uint32_t texture_unit) {
    // Doku birimini aktif hale getir
    glActiveTexture(GL_TEXTURE0 + texture_unit);
    
    // Kaplamayı aktif birime bağla
    glBindTexture(GL_TEXTURE_2D, texture_id);
}


// ----------------------------------------------------------------------
// 2. ÇİZİM (Drawing) KOMUTLARI
// ----------------------------------------------------------------------

/**
 * Uygulama: fe_gl_cmd_draw_indexed
 */
void fe_gl_cmd_draw_indexed(uint32_t index_count, uint32_t primitive_type) {
    // Primitive tipi NULL ise veya 0 ise varsayilan GL_TRIANGLES kullanilir.
    if (primitive_type == 0) primitive_type = GL_TRIANGLES;
    
    // VAO'nun daha once baglandigi varsayilir.
    glDrawElements(
        primitive_type, 
        (GLsizei)index_count, 
        GL_UNSIGNED_INT,    // Index'lerin 32-bit unsigned integer oldugu varsayilir.
        NULL                // Index Buffer'in VAO'ya baglı oldugu varsayılır.
    );
}

/**
 * Uygulama: fe_gl_cmd_draw_indexed_instanced
 */
void fe_gl_cmd_draw_indexed_instanced(uint32_t index_count, uint32_t instance_count, uint32_t primitive_type) {
    if (primitive_type == 0) primitive_type = GL_TRIANGLES;
    
    glDrawElementsInstanced(
        primitive_type, 
        (GLsizei)index_count, 
        GL_UNSIGNED_INT, 
        NULL, 
        (GLsizei)instance_count
    );
}

/**
 * Uygulama: fe_gl_cmd_draw_arrays
 */
void fe_gl_cmd_draw_arrays(uint32_t vertex_count, uint32_t primitive_type) {
    if (primitive_type == 0) primitive_type = GL_TRIANGLES;

    glDrawArrays(
        primitive_type, 
        0,                      // Başlangıç indeksi
        (GLsizei)vertex_count
    );
}