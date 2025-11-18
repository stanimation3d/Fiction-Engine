// src/graphics/fe_post_processing.c

#include "graphics/fe_post_processing.h"
#include "graphics/fe_renderer.h"
#include "graphics/fe_shader_compiler.h"
#include "graphics/fe_material_editor.h"
#include "utils/fe_logger.h"
#include <raylib.h> // rlgl.h, GetScreenWidth vb. için
#include <rlgl.h> 
#include <stdlib.h> // malloc/free için

// ----------------------------------------------------------------------
// 1. YÖNETİM VERİLERİ VE ÇEKİRDEK YAPILAR
// ----------------------------------------------------------------------

// Ekranı kaplayan dörtgenin Vertex Array Object (VAO) ID'si
static fe_buffer_id_t s_full_screen_quad_vao = 0;
// Render geçişleri arasında geçiş yapmak için kullanılan iki ara FBO.
static fe_framebuffer_t s_ping_pong_fbo[2] = {0}; 
// Efekt Boru Hattı
#define MAX_EFFECTS 16
static fe_post_effect_type_t s_active_effects[MAX_EFFECTS];
static int s_effect_count = 0;


/**
 * @brief Basit bir ekran kaplayan dörtgen (quad) olusturur.
 */
static fe_buffer_id_t fe_post_processing_create_quad_vao() {
    // Normalleştirilmiş cihaz koordinatlarında (NDC) ekranı kaplayan dörtgen (Z=0)
    float vertices[] = {
        // Konum (XYZ)  /  UV Koordinatları (UV)
        -1.0f,  1.0f, 0.0f, 0.0f, 1.0f, // Sol Üst
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, // Sol Alt
         1.0f, -1.0f, 0.0f, 1.0f, 0.0f, // Sağ Alt
         1.0f,  1.0f, 0.0f, 1.0f, 1.0f  // Sağ Üst
    };
    uint32_t indices[] = { 0, 1, 2, 2, 3, 0 }; // İki üçgen

    fe_buffer_id_t vao;
    fe_buffer_id_t vbo;
    fe_buffer_id_t ebo;

    // VAO oluşturma
    rlglGenVertexArrays(1, &vao);
    rlglBindVertexArray(vao);

    // VBO oluşturma ve veri yükleme
    rlglGenBuffers(1, &vbo);
    rlglBindBuffer(GL_ARRAY_BUFFER, vbo);
    rlglBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // EBO oluşturma ve veri yükleme
    rlglGenBuffers(1, &ebo);
    rlglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    rlglBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Vertex niteliklerini ayarla
    // Konum (loc 0)
    rlglEnableVertexAttrib(0);
    rlglVertexAttribPointer(0, 3, GL_FLOAT, false, 5 * sizeof(float), (void*)0);
    // UV Koordinatları (loc 1)
    rlglEnableVertexAttrib(1);
    rlglVertexAttribPointer(1, 2, GL_FLOAT, false, 5 * sizeof(float), (void*)(3 * sizeof(float)));

    // Bağlantıyı çöz
    rlglBindVertexArray(0);
    rlglDeleteBuffers(vbo); // VBO ve EBO'yu silebiliriz, VAO onları hatırlar.
    rlglDeleteBuffers(ebo); 
    
    return vao;
}


/**
 * Uygulama: fe_post_processing_init
 */
fe_error_code_t fe_post_processing_init(void) {
    FE_LOG_INFO("Post-Processing sistemi baslatiliyor...");
    
    // 1. Ekranı kaplayan dörtgeni oluştur
    s_full_screen_quad_vao = fe_post_processing_create_quad_vao();
    if (s_full_screen_quad_vao == 0) {
        FE_LOG_FATAL("Ekran Dortgeni (Quad) olusturulamadi.");
        return FE_ERR_GRAPHICS_API_ERROR;
    }
    
    // 2. Ping-Pong FBO'larını başlat (FBO'ların yaratılması burada yapılmadı, varsayılıyor)
    // Gerçek bir uygulamada, s_ping_pong_fbo[0] ve s_ping_pong_fbo[1] burada
    // ekran boyutunda veya yarı boyutunda oluşturulmalı ve bağlanmalıdır.
    // Şimdilik sadece dörtgen oluşturulduğunu varsayalım.
    
    FE_LOG_INFO("Post-Processing sistemi hazir. Quad VAO ID: %u", s_full_screen_quad_vao);
    return FE_OK;
}

/**
 * Uygulama: fe_post_processing_shutdown
 */
void fe_post_processing_shutdown(void) {
    if (s_full_screen_quad_vao != 0) {
        rlglDeleteVertexArrays(s_full_screen_quad_vao);
        s_full_screen_quad_vao = 0;
    }
    // TODO: s_ping_pong_fbo FBO'larını da sil.
    
    s_effect_count = 0;
    FE_LOG_INFO("Post-Processing sistemi kapatildi.");
}

/**
 * Uygulama: fe_post_processing_add_effect
 */
fe_error_code_t fe_post_processing_add_effect(fe_post_effect_type_t effect_type) {
    if (s_effect_count >= MAX_EFFECTS) {
        FE_LOG_ERROR("Post-Processing boru hatti dolu! Efekt eklenemiyor.");
        return FE_ERR_OUT_OF_RESOURCES;
    }

    s_active_effects[s_effect_count++] = effect_type;
    // TODO: Burada efekte özel materyal ve shader yüklemesi yapılmalıdır.
    
    FE_LOG_DEBUG("Boru hattina yeni efekt eklendi: %d", effect_type);
    return FE_OK;
}


/**
 * @brief Tek bir post-processing efektini uygular.
 * @param source_texture Girdi olarak kullanılacak kaynak doku ID'si.
 * @param material Uygulanacak efekti içeren malzeme.
 * @param target_fbo Ciktinin yazilacagi hedef FBO.
 */
static void fe_post_processing_draw_pass(fe_texture_id_t source_texture, fe_material_t* material, fe_framebuffer_t* target_fbo) {
    // 1. Hedef FBO'yu bağla (fe_render_pass_begin'in basit versiyonu)
    if (target_fbo) {
        rlglBindFramebuffer(target_fbo->fbo_id);
        rlViewport(0, 0, target_fbo->width, target_fbo->height);
    } else {
        rlglBindFramebuffer(0); // Ana ekrana çiz
        rlViewport(0, 0, GetScreenWidth(), GetScreenHeight());
    }
    
    // 2. Materyali (Shader ve UBO'yu) bağla
    fe_material_bind(material);
    
    // 3. Giriş dokusunu bağla (Genellikle 0. veya 1. yuvaya (slot) bağlanır)
    rlglActiveTexture(RL_TEXTURE_UNIT_MAX); // 0. Doku Birimi (varsayılan)
    rlglBindTexture(RL_TEXTURE_2D, source_texture);
    
    // 4. Çekirdek dörtgeni (Quad) çiz
    rlglBindVertexArray(s_full_screen_quad_vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0); // 6 indeksli (2 üçgen) çiz
    rlglBindVertexArray(0);

    // 5. Bağlantıları çöz
    fe_shader_unuse();
    rlglBindTexture(RL_TEXTURE_2D, 0);
    rlglBindFramebuffer(0);
}


/**
 * Uygulama: fe_post_processing_apply
 */
void fe_post_processing_apply(fe_framebuffer_t* scene_color_fbo, fe_framebuffer_t* target_fbo) {
    if (s_effect_count == 0) {
        FE_LOG_DEBUG("Post-Processing boru hatti bos. Islem atlandi.");
        return;
    }
    
    // Sahne FBO'sunun renk çıktısını al
    fe_texture_id_t current_source_tex = scene_color_fbo->color_texture_id;
    int current_fbo_index = 0; // Ping veya Pong FBO'larından hangisi kullanılacak?
    
    for (int i = 0; i < s_effect_count; i++) {
        fe_post_effect_type_t effect = s_active_effects[i];
        // Örnek: Efekt için gerekli materyali al
        fe_material_t* effect_material = NULL; // fe_material_create/get_material_by_effect(effect) çağrılmalıdır.
        
        // Eğer bu son geçiş ise, hedef FBO'ya (target_fbo) veya ana ekrana çiz
        fe_framebuffer_t* next_target = NULL;
        if (i == s_effect_count - 1) {
            next_target = target_fbo; // NULL ise ana ekrana çizilir
        } else {
            // Son geçiş değilse, sonraki Ping-Pong FBO'ya çiz
            current_fbo_index = (current_fbo_index + 1) % 2; 
            next_target = &s_ping_pong_fbo[current_fbo_index];
        }

        // fe_post_processing_draw_pass(current_source_tex, effect_material, next_target);
        // current_source_tex = next_target->color_texture_id; // Yeni kaynak dokusu, son çıktı olur.
        
        FE_LOG_DEBUG("Efekt uygulaniyor: %d", effect);
    }
}