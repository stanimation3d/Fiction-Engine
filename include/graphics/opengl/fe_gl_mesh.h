// include/graphics/opengl/fe_gl_mesh.h

#ifndef FE_GL_MESH_H
#define FE_GL_MESH_H

#include <stdint.h>
#include "error/fe_error.h"
#include "graphics/fe_render_types.h" // fe_mesh_t, fe_vertex_t için

// ----------------------------------------------------------------------
// 1. MESH YÖNETİM FONKSİYONLARI
// ----------------------------------------------------------------------

/**
 * @brief CPU verilerinden yeni bir OpenGL Mesh'i (VAO, VBO, EBO) olusturur.
 * * fe_gl_device'ı kullanarak veriyi GPU'ya yükler.
 * @param vertices Mesh'in fe_vertex_t yapisindaki kose verileri.
 * @param vertex_count Koselerdeki toplam eleman sayisi.
 * @param indices Mesh'in cizim siralamasini belirten index verileri.
 * @param index_count Index'lerdeki toplam eleman sayisi.
 * @return Olusturulan mesh'i temsil eden fe_mesh_t yapisinin pointer'i. Basarisiz olursa NULL.
 */
fe_mesh_t* fe_gl_mesh_create(const fe_vertex_t* vertices, uint32_t vertex_count, 
                            const uint32_t* indices, uint32_t index_count);

/**
 * @brief Bir OpenGL Mesh'i yok eder ve GPU kaynaklarini (VAO, VBO, EBO) serbest birakir.
 */
void fe_gl_mesh_destroy(fe_mesh_t* mesh);

/**
 * @brief Bir mesh'in kose verilerini (VBO) gunceller. 
 * * Yalnizca dinamik mesh'ler icin kullanilmalidir.
 */
void fe_gl_mesh_update_vertices(fe_mesh_t* mesh, const fe_vertex_t* vertices, uint32_t vertex_count);

#endif // FE_GL_MESH_H