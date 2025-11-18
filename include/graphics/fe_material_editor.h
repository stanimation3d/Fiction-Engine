// include/graphics/fe_material_editor.h

#ifndef FE_MATERIAL_EDITOR_H
#define FE_MATERIAL_EDITOR_H

#include <stdint.h>
#include "error/fe_error.h"
#include "graphics/fe_render_types.h" // fe_texture_id_t, fe_shader_id_t için

// ----------------------------------------------------------------------
// 1. PBR DOKU EŞLEME (Texture Mapping) TÜRLERİ
// ----------------------------------------------------------------------

// Bir PBR Malzemesinin kullanabileceği temel doku türleri.
typedef enum fe_material_texture_slot {
    FE_TEX_SLOT_ALBEDO,       // Temel renk (RGB) ve Opaklık (A)
    FE_TEX_SLOT_NORMAL,       // Normal Haritası
    FE_TEX_SLOT_METALLICITY,  // Metaliklik Haritası (Tek kanal)
    FE_TEX_SLOT_ROUGHNESS,    // Pürüzlülük Haritası (Tek kanal)
    FE_TEX_SLOT_AO,           // Ortam Kapanması (Ambient Occlusion)
    FE_TEX_SLOT_EMISSIVE,     // Işıma Haritası
    FE_TEX_SLOT_COUNT         // Slot sayısı
} fe_material_texture_slot_t;


// ----------------------------------------------------------------------
// 2. MALZEME YAPISI
// ----------------------------------------------------------------------

/**
 * @brief Bir nesnenin yüzey özelliklerini tanımlayan yapı.
 */
typedef struct fe_material {
    fe_shader_id_t shader_program_id; // Bu malzemeyi çizecek Shader Programı ID'si
    
    // PBR Parametreleri (Düz Renk Değerleri)
    float albedo_color[4];      // RGBA temel renk (Doku yoksa kullanılır)
    float metallic_value;       // Metaliklik katsayısı (0.0 ile 1.0 arası)
    float roughness_value;      // Pürüzlülük katsayısı (0.0 ile 1.0 arası)
    float emissive_color[3];    // Işıma rengi (RGB)

    // Doku Tanıtıcıları (fe_texture_id_t, fe_render_types.h'den gelir)
    fe_texture_id_t texture_ids[FE_TEX_SLOT_COUNT]; 
    
    // Uniform Buffer Object (UBO) ID'si (GPU'da hızlı erişim için)
    fe_buffer_id_t ubo_id;
    
    // Malzemenin dahili kimliği (Hash veya Dizin)
    uint32_t material_hash_id;
} fe_material_t;


// ----------------------------------------------------------------------
// 3. YÖNETİM VE DÜZENLEME FONKSİYONLARI
// ----------------------------------------------------------------------

/**
 * @brief Yeni bir malzeme örneği oluşturur ve varsayılan değerlerle başlatır.
 * * @param name Malzeme için benzersiz bir isim.
 * @return Yeni malzemenin pointer'ı. Başarısız olursa NULL.
 */
fe_material_t* fe_material_create(const char* name);

/**
 * @brief Bir malzemeyi sistemden kaldırır ve GPU kaynaklarını serbest bırakır.
 */
void fe_material_destroy(fe_material_t* material);

/**
 * @brief Malzemenin GPU'daki Uniform Buffer'ını günceller.
 * * Bu, renk veya katsayı parametreleri değiştiğinde çağrılmalıdır.
 */
fe_error_code_t fe_material_upload_to_gpu(fe_material_t* material);

/**
 * @brief Render sırasında bu malzemeyi aktif hale getirir (bind eder).
 * * Doğru shader programını kullanıma alır, dokuları bağlar ve UBO'yu bağlar.
 */
void fe_material_bind(const fe_material_t* material);

/**
 * @brief Malzeme ile ilişkili bir dokuyu ayarlar.
 * * @param material Hedef malzeme.
 * @param slot Hangi doku yuvasına (slot) ayarlanacak (ALBEDO, NORMAL vb.).
 * @param texture_id fe_texture_id_t olarak yüklenmiş doku tanıtıcısı.
 */
void fe_material_set_texture(fe_material_t* material, fe_material_texture_slot_t slot, fe_texture_id_t texture_id);

#endif // FE_MATERIAL_EDITOR_H
