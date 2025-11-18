// src/graphics/opengl/fe_gl_device.c

#include "graphics/opengl/fe_gl_device.h"
#include "utils/fe_logger.h"
#include <GL/gl.h> // Doğrudan OpenGL komutları için
#include <raylib.h> // OpenGL yüklemesini ve diğer yardımcıları kullanmak için (isteğe bağlı)


// ----------------------------------------------------------------------
// DAHİLİ YARDIMCI FONKSİYONLAR
// ----------------------------------------------------------------------

/**
 * @brief fe_buffer_usage_t enum'unu OpenGL kullanım bayrağına cevirir.
 */
static uint32_t fe_to_gl_usage(fe_buffer_usage_t usage) {
    switch (usage) {
        case FE_BUFFER_USAGE_STATIC: return GL_STATIC_DRAW;
        case FE_BUFFER_USAGE_DYNAMIC: return GL_DYNAMIC_DRAW;
        case FE_BUFFER_USAGE_STREAM: return GL_STREAM_DRAW;
        default: return GL_STATIC_DRAW;
    }
}

/**
 * @brief fe_texture_format_t enum'unu OpenGL dahili format ve veri formatına cevirir.
 */
static void fe_to_gl_texture_format(fe_texture_format_t format, uint32_t* internal_format, uint32_t* data_format, uint32_t* data_type) {
    *data_type = GL_UNSIGNED_BYTE; // Varsayılan: 8 bit/kanal
    switch (format) {
        case FE_TEXTURE_FORMAT_RGB8:
            *internal_format = GL_RGB8;
            *data_format = GL_RGB;
            break;
        case FE_TEXTURE_FORMAT_RGBA8:
            *internal_format = GL_RGBA8;
            *data_format = GL_RGBA;
            break;
        case FE_TEXTURE_FORMAT_D24S8: // Derinlik + Stencil
            *internal_format = GL_DEPTH24_STENCIL8;
            *data_format = GL_DEPTH_STENCIL;
            *data_type = GL_UNSIGNED_INT_24_8;
            break;
        default: // Güvenli varsayılan
            *internal_format = GL_RGBA8;
            *data_format = GL_RGBA;
            break;
    }
}

/**
 * @brief OpenGL hata kontrolü (Hata ayıklama modunda kullanışlı).
 */
static bool fe_gl_check_error(const char* func) {
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        FE_LOG_ERROR("OpenGL Hatasi %x: %s fonksiyonunda.", err, func);
        return false;
    }
    return true;
}


// ----------------------------------------------------------------------
// 2. BUFFER YÖNETİMİ UYGULAMALARI
// ----------------------------------------------------------------------

/**
 * Uygulama: fe_gl_device_create_buffer
 */
fe_buffer_id_t fe_gl_device_create_buffer(size_t size, const void* data, fe_buffer_usage_t usage) {
    fe_buffer_id_t buffer_id;
    uint32_t gl_usage = fe_to_gl_usage(usage);
    
    glGenBuffers(1, &buffer_id);
    if (buffer_id == 0) {
        FE_LOG_ERROR("glGenBuffers basarisiz oldu.");
        return 0;
    }

    glBindBuffer(GL_ARRAY_BUFFER, buffer_id);
    glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)size, data, gl_usage);
    glBindBuffer(GL_ARRAY_BUFFER, 0); // Bağlantıyı çöz

    if (!fe_gl_check_error(__func__)) {
        glDeleteBuffers(1, &buffer_id);
        return 0;
    }
    
    return buffer_id;
}

/**
 * Uygulama: fe_gl_device_update_buffer
 */
void fe_gl_device_update_buffer(fe_buffer_id_t buffer_id, size_t offset, size_t size, const void* data) {
    if (buffer_id == 0) return;

    glBindBuffer(GL_ARRAY_BUFFER, buffer_id);
    glBufferSubData(GL_ARRAY_BUFFER, (GLintptr)offset, (GLsizeiptr)size, data);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    fe_gl_check_error(__func__);
}

/**
 * Uygulama: fe_gl_device_destroy_buffer
 */
void fe_gl_device_destroy_buffer(fe_buffer_id_t buffer_id) {
    if (buffer_id != 0) {
        glDeleteBuffers(1, &buffer_id);
        fe_gl_check_error(__func__);
    }
}


// ----------------------------------------------------------------------
// 3. KAPLAMA (Texture) YÖNETİMİ UYGULAMALARI
// ----------------------------------------------------------------------

/**
 * Uygulama: fe_gl_device_create_texture2d
 */
fe_texture_id_t fe_gl_device_create_texture2d(int width, int height, fe_texture_format_t format, const void* data) {
    fe_texture_id_t texture_id;
    uint32_t internal_format, data_format, data_type;
    
    fe_to_gl_texture_format(format, &internal_format, &data_format, &data_type);

    glGenTextures(1, &texture_id);
    if (texture_id == 0) {
        FE_LOG_ERROR("glGenTextures basarisiz oldu.");
        return 0;
    }
    
    glBindTexture(GL_TEXTURE_2D, texture_id);

    // Kaplama Verisini Yükle
    glTexImage2D(GL_TEXTURE_2D, 0, (GLint)internal_format, width, height, 0, data_format, data_type, data);

    // Varsayılan Kaplama Parametreleri (Daha sonra bir fe_texture_sampler modülünde olmalı)
    if (format != FE_TEXTURE_FORMAT_D24S8) {
        // Renk kaplamaları için mipmap ve filtreleme ayarları
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    } else {
        // Derinlik/Stencil kaplamaları için özel ayarlar
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
    
    glBindTexture(GL_TEXTURE_2D, 0);
    
    if (!fe_gl_check_error(__func__)) {
        glDeleteTextures(1, &texture_id);
        return 0;
    }
    
    return texture_id;
}

/**
 * Uygulama: fe_gl_device_destroy_texture
 */
void fe_gl_device_destroy_texture(fe_texture_id_t texture_id) {
    if (texture_id != 0) {
        glDeleteTextures(1, &texture_id);
        fe_gl_check_error(__func__);
    }
}


// ----------------------------------------------------------------------
// 4. FRAMEBUFFER YÖNETİMİ UYGULAMALARI
// ----------------------------------------------------------------------

/**
 * Uygulama: fe_gl_device_create_framebuffer
 */
fe_buffer_id_t fe_gl_device_create_framebuffer(void) {
    fe_buffer_id_t fbo_id;
    glGenFramebuffers(1, &fbo_id);
    fe_gl_check_error(__func__);
    return fbo_id;
}

/**
 * Uygulama: fe_gl_device_attach_texture_to_fbo
 */
void fe_gl_device_attach_texture_to_fbo(fe_buffer_id_t fbo_id, uint32_t attachment, fe_texture_id_t texture_id) {
    if (fbo_id == 0 || texture_id == 0) return;

    glBindFramebuffer(GL_FRAMEBUFFER, fbo_id);
    
    if (attachment == GL_DEPTH_ATTACHMENT || attachment == GL_DEPTH_STENCIL_ATTACHMENT) {
        glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, texture_id, 0);
    } else {
        // GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 vb.
        glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, texture_id, 0);
    }

    // FBO durumu kontrolü (Hata ayıklama için)
    #ifdef _DEBUG
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        FE_LOG_ERROR("FBO %u olusturulduktan sonra tamamlanamadi!", fbo_id);
    }
    #endif

    glBindFramebuffer(GL_FRAMEBUFFER, 0); // Bağlantıyı çöz
    fe_gl_check_error(__func__);
}

/**
 * Uygulama: fe_gl_device_destroy_framebuffer
 */
void fe_gl_device_destroy_framebuffer(fe_buffer_id_t fbo_id) {
    if (fbo_id != 0) {
        glDeleteFramebuffers(1, &fbo_id);
        fe_gl_check_error(__func__);
    }
}