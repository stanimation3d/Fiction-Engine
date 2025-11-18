// src/graphics/opengl/fe_gl_pipeline.c

#include "graphics/opengl/fe_gl_pipeline.h"
#include "utils/fe_logger.h"
#include <GL/gl.h> // Doğrudan OpenGL komutları için
#include <string.h> // memset için

// ----------------------------------------------------------------------
// 1. DAHİLİ DURUM ÖNBELLEĞİ (State Caching)
// ----------------------------------------------------------------------

/**
 * @brief Mevcut OpenGL pipeline ayarlarini tutar. 
 * * Bu, fazladan GL durum degistirme cagrilarini engeller.
 */
typedef struct fe_gl_pipeline_state {
    bool depth_test_enabled;
    fe_depth_func_t depth_func;
    bool depth_write_enabled;
    fe_cull_mode_t cull_mode;
    bool blend_enabled;
    fe_blend_factor_t src_blend_factor;
    fe_blend_factor_t dst_blend_factor;
    // ...
} fe_gl_pipeline_state_t;

static fe_gl_pipeline_state_t s_gl_state_cache;

// ----------------------------------------------------------------------
// 2. YARDIMCI DÖNÜŞÜM FONKSİYONLARI
// ----------------------------------------------------------------------

/**
 * @brief fe_depth_func_t enum'unu GL sabitine cevirir.
 */
static GLenum fe_to_gl_depth_func(fe_depth_func_t func) {
    switch (func) {
        case FE_DEPTH_NEVER:    return GL_NEVER;
        case FE_DEPTH_LESS:     return GL_LESS;
        case FE_DEPTH_EQUAL:    return GL_EQUAL;
        case FE_DEPTH_LEQUAL:   return GL_LEQUAL;
        case FE_DEPTH_GREATER:  return GL_GREATER;
        case FE_DEPTH_NOTEQUAL: return GL_NOTEQUAL;
        case FE_DEPTH_GEQUAL:   return GL_GEQUAL;
        case FE_DEPTH_ALWAYS:   return GL_ALWAYS;
        default:                return GL_LEQUAL; // Güvenli varsayılan
    }
}

/**
 * @brief fe_blend_factor_t enum'unu GL sabitine cevirir.
 */
static GLenum fe_to_gl_blend_factor(fe_blend_factor_t factor) {
    switch (factor) {
        case FE_BLEND_ZERO:             return GL_ZERO;
        case FE_BLEND_ONE:              return GL_ONE;
        case FE_BLEND_SRC_ALPHA:        return GL_SRC_ALPHA;
        case FE_BLEND_ONE_MINUS_SRC_ALPHA: return GL_ONE_MINUS_SRC_ALPHA;
        default:                        return GL_ONE; 
    }
}


// ----------------------------------------------------------------------
// 3. ARABİRİM UYGULAMALARI
// ----------------------------------------------------------------------

/**
 * Uygulama: fe_gl_pipeline_init
 */
void fe_gl_pipeline_init(void) {
    memset(&s_gl_state_cache, 0, sizeof(fe_gl_pipeline_state_t));
    
    // Varsayılan GL durumunu önbelleğe yansıt ve ilk ayarları yap.
    // Bu varsayılanlar, motorun fe_gl_backend_init'indeki ilk ayarlarla uyumlu olmalıdır.
    s_gl_state_cache.depth_test_enabled = true;
    s_gl_state_cache.depth_func = FE_DEPTH_LEQUAL;
    s_gl_state_cache.depth_write_enabled = true;
    s_gl_state_cache.cull_mode = FE_CULL_BACK;
    s_gl_state_cache.blend_enabled = true;
    s_gl_state_cache.src_blend_factor = FE_BLEND_SRC_ALPHA;
    s_gl_state_cache.dst_blend_factor = FE_BLEND_ONE_MINUS_SRC_ALPHA;
    
    // İlk GL çağrıları (Emin olmak için, fe_gl_backend'de de yapılabilir)
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glDepthMask(GL_TRUE);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    FE_LOG_DEBUG("GL Pipeline durum onbellegi baslatildi.");
}

/**
 * Uygulama: fe_gl_pipeline_set_depth_test_enabled
 */
void fe_gl_pipeline_set_depth_test_enabled(bool enabled) {
    if (s_gl_state_cache.depth_test_enabled != enabled) {
        if (enabled) {
            glEnable(GL_DEPTH_TEST);
        } else {
            glDisable(GL_DEPTH_TEST);
        }
        s_gl_state_cache.depth_test_enabled = enabled;
        FE_LOG_TRACE("GL_DEPTH_TEST: %s", enabled ? "ENABLED" : "DISABLED");
    }
}

/**
 * Uygulama: fe_gl_pipeline_set_depth_func
 */
void fe_gl_pipeline_set_depth_func(fe_depth_func_t func) {
    if (s_gl_state_cache.depth_func != func) {
        GLenum gl_func = fe_to_gl_depth_func(func);
        glDepthFunc(gl_func);
        s_gl_state_cache.depth_func = func;
        FE_LOG_TRACE("GL_DEPTH_FUNC ayarlandi.");
    }
}

/**
 * Uygulama: fe_gl_pipeline_set_depth_write_enabled
 */
void fe_gl_pipeline_set_depth_write_enabled(bool enabled) {
    if (s_gl_state_cache.depth_write_enabled != enabled) {
        glDepthMask(enabled ? GL_TRUE : GL_FALSE);
        s_gl_state_cache.depth_write_enabled = enabled;
        FE_LOG_TRACE("GL_DEPTH_MASK: %s", enabled ? "GL_TRUE" : "GL_FALSE");
    }
}

/**
 * Uygulama: fe_gl_pipeline_set_cull_mode
 */
void fe_gl_pipeline_set_cull_mode(fe_cull_mode_t mode) {
    if (s_gl_state_cache.cull_mode != mode) {
        if (mode == FE_CULL_NONE) {
            glDisable(GL_CULL_FACE);
        } else {
            if (s_gl_state_cache.cull_mode == FE_CULL_NONE) {
                glEnable(GL_CULL_FACE);
            }
            if (mode == FE_CULL_BACK) {
                glCullFace(GL_BACK);
            } else if (mode == FE_CULL_FRONT) {
                glCullFace(GL_FRONT);
            } else if (mode == FE_CULL_FRONT_AND_BACK) {
                glCullFace(GL_FRONT_AND_BACK);
            }
        }
        s_gl_state_cache.cull_mode = mode;
        FE_LOG_TRACE("GL_CULL_FACE modu ayarlandi.");
    }
}

/**
 * Uygulama: fe_gl_pipeline_set_blend_enabled
 */
void fe_gl_pipeline_set_blend_enabled(bool enabled) {
    if (s_gl_state_cache.blend_enabled != enabled) {
        if (enabled) {
            glEnable(GL_BLEND);
        } else {
            glDisable(GL_BLEND);
        }
        s_gl_state_cache.blend_enabled = enabled;
        FE_LOG_TRACE("GL_BLEND: %s", enabled ? "ENABLED" : "DISABLED");
    }
}

/**
 * Uygulama: fe_gl_pipeline_set_blend_func
 */
void fe_gl_pipeline_set_blend_func(fe_blend_factor_t src_factor, fe_blend_factor_t dst_factor) {
    if (s_gl_state_cache.src_blend_factor != src_factor || s_gl_state_cache.dst_blend_factor != dst_factor) {
        GLenum gl_src = fe_to_gl_blend_factor(src_factor);
        GLenum gl_dst = fe_to_gl_blend_factor(dst_factor);
        glBlendFunc(gl_src, gl_dst);
        s_gl_state_cache.src_blend_factor = src_factor;
        s_gl_state_cache.dst_blend_factor = dst_factor;
        FE_LOG_TRACE("GL_BLEND_FUNC ayarlandi.");
    }
}