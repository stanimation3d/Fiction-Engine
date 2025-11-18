// src/graphics/opengl/fe_gl_mesh.c

#include "graphics/opengl/fe_gl_mesh.h"
#include "graphics/opengl/fe_gl_device.h" // Bufferlari oluşturmak ve silmek için
#include "graphics/fe_render_types.h"
#include "utils/fe_logger.h"
#include <GL/gl.h> // Doğrudan OpenGL komutları için
#include <stdlib.h> // malloc, free için


// ----------------------------------------------------------------------
// 1. DAHİLİ YARDIMCI FONKSİYONLAR
// ----------------------------------------------------------------------

/**
 * @brief fe_vertex_t yapisinin niteliklerini VAO'ya kaydeder (Vertex Attrib Pointers).
 * * Bu, VBO baglandiktan sonra cagrılmalıdır.
 */
static void fe_gl_setup_vertex_attributes(void) {
    size_t stride = sizeof(fe_vertex_t);
    size_t offset = 0;
    
    // Konum (Location 0): float position[3]
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, (GLsizei)stride, (const void*)offset);
    offset += sizeof(float) * 3;
    
    // Normal (Location 1): float normal[3]
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, (GLsizei)stride, (const void*)offset);
    offset += sizeof(float) * 3;

    // UV Koordinatları (Location 2): float texcoord[2]
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, (GLsizei)stride, (const void*)offset);
    offset += sizeof(float) * 2;
    
    // Teğet Vektörü (Location 3): float tangent[3]
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, (GLsizei)stride, (const void*)offset);
    offset += sizeof(float) * 3;
    
    // Renk (Location 4): uint8_t color[4] - Normalized
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 4, GL_UNSIGNED_BYTE, GL_TRUE, (GLsizei)stride, (const void*)offset); // GL_TRUE: Normalized
    // offset += sizeof(uint8_t) * 4; // Bütün stride kullanıldı.
    
    // Toplam Offset = 3*4 + 3*4 + 2*4 + 3*4 + 4*1 = 12+12+8+12+4 = 48 bytes (fe_vertex_t boyutu)
}


// ----------------------------------------------------------------------
// 2. ARABİRİM UYGULAMALARI
// ----------------------------------------------------------------------

/**
 * Uygulama: fe_gl_mesh_create
 */
fe_mesh_t* fe_gl_mesh_create(const fe_vertex_t* vertices, uint32_t vertex_count, 
                            const uint32_t* indices, uint32_t index_count) {
    
    if (vertex_count == 0 || index_count == 0) {
        FE_LOG_ERROR("Gecersiz mesh verisi: Vertex veya Index sayisi sifir.");
        return NULL;
    }
    
    fe_mesh_t* mesh = (fe_mesh_t*)calloc(1, sizeof(fe_mesh_t));
    if (!mesh) return NULL;
    
    mesh->vertex_count = vertex_count;
    mesh->index_count = index_count;

    // 1. VBO (Vertex Buffer Object) Olustur
    size_t vbo_size = vertex_count * sizeof(fe_vertex_t);
    mesh->vertex_buffer_id = fe_gl_device_create_buffer(vbo_size, vertices, FE_BUFFER_USAGE_STATIC);
    
    // 2. EBO (Element Buffer Object / Index Buffer) Olustur
    size_t ebo_size = index_count * sizeof(uint32_t);
    // EBO, GL_ELEMENT_ARRAY_BUFFER tipiyle olusturulmali, ancak fe_gl_device'daki 
    // fe_gl_device_create_buffer genel bir tampon oldugundan, burada manuel olarak GL çağrısı yapmamiz gerekir.
    // Ancak daha tutarlı olması için fe_gl_device'ı kullanalım, VBO/EBO'yu VAO'ya baglarken tipini belirtecegiz.
    mesh->index_buffer_id = fe_gl_device_create_buffer(ebo_size, indices, FE_BUFFER_USAGE_STATIC);

    if (mesh->vertex_buffer_id == 0 || mesh->index_buffer_id == 0) {
        FE_LOG_ERROR("Mesh olusturulamadi: VBO/EBO basarisiz.");
        fe_gl_mesh_destroy(mesh);
        return NULL;
    }

    // 3. VAO (Vertex Array Object) Olusturma ve Baglama
    glGenVertexArrays(1, &mesh->vao_id);
    if (mesh->vao_id == 0) {
        FE_LOG_ERROR("Mesh olusturulamadi: VAO basarisiz.");
        fe_gl_mesh_destroy(mesh);
        return NULL;
    }
    
    glBindVertexArray(mesh->vao_id);
    
    // VBO'yu VAO'ya bagla (GL_ARRAY_BUFFER)
    glBindBuffer(GL_ARRAY_BUFFER, mesh->vertex_buffer_id);
    
    // EBO'yu VAO'ya bagla (GL_ELEMENT_ARRAY_BUFFER)
    // VAO, EBO'yu hatirlayacak.
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->index_buffer_id);

    // Vertex niteliklerini ayarla
    fe_gl_setup_vertex_attributes();
    
    // Baglantilari Coz
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    // EBO'nun baglantisi, VAO baglantisi cozülürken cozülür.
    
    FE_LOG_INFO("Mesh olusturuldu (VAO: %u, V: %u, I: %u)", mesh->vao_id, vertex_count, index_count);
    return mesh;
}

/**
 * Uygulama: fe_gl_mesh_destroy
 */
void fe_gl_mesh_destroy(fe_mesh_t* mesh) {
    if (!mesh) return;

    // GPU kaynaklarini fe_gl_device ile sil
    if (mesh->vao_id != 0) {
        glDeleteVertexArrays(1, &mesh->vao_id);
    }
    if (mesh->vertex_buffer_id != 0) {
        fe_gl_device_destroy_buffer(mesh->vertex_buffer_id);
    }
    if (mesh->index_buffer_id != 0) {
        fe_gl_device_destroy_buffer(mesh->index_buffer_id);
    }
    
    free(mesh);
    FE_LOG_DEBUG("Mesh yok edildi.");
}

/**
 * Uygulama: fe_gl_mesh_update_vertices
 */
void fe_gl_mesh_update_vertices(fe_mesh_t* mesh, const fe_vertex_t* vertices, uint32_t vertex_count) {
    if (!mesh || mesh->vertex_buffer_id == 0 || mesh->vertex_count != vertex_count) {
        FE_LOG_ERROR("Gecersiz mesh veya boyut uyusmazligi nedeniyle VBO guncellenemedi.");
        return;
    }
    
    size_t vbo_size = vertex_count * sizeof(fe_vertex_t);
    
    // fe_gl_device'daki update fonksiyonunu kullan
    fe_gl_device_update_buffer(mesh->vertex_buffer_id, 0, vbo_size, vertices);
    
    FE_LOG_TRACE("Mesh VBO guncellendi (V: %u).", vertex_count);
}