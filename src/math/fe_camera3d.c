// src/math/fe_camera3d.c

#include "math/fe_camera3d.h"
#include "math/fe_vector.h"
#include "math/fe_matrix.h"
#include "utils/fe_logger.h"
#include <stdlib.h> // malloc, free için
#include <math.h>   // sin, cos, tanf için
#include <string.h> // memcpy için

// Pitch açısının sınırlanacağı değer (Radyan)
#define MAX_PITCH_RAD (M_PI_2 - 0.01f) // Yaklaşık 89.4 derece

// ----------------------------------------------------------------------
// 1. KAMERA YÖNETİMİ
// ----------------------------------------------------------------------

/**
 * Uygulama: fe_camera3d_create
 */
fe_camera3d_t* fe_camera3d_create(float fov_y, float aspect_ratio, float near_p, float far_p) {
    fe_camera3d_t* camera = (fe_camera3d_t*)calloc(1, sizeof(fe_camera3d_t));
    if (!camera) {
        FE_LOG_FATAL("Kamera yapisi icin bellek ayrilamadi.");
        return NULL;
    }

    // Varsayılan Ayarlar
    camera->position = (fe_vec3_t){0.0f, 0.0f, 0.0f};
    camera->yaw = 0.0f; // Dünya Z eksenine bakış
    camera->pitch = 0.0f;
    
    // Görüş Ayarları
    camera->fov_y = fov_y;
    camera->aspect_ratio = aspect_ratio;
    camera->near_plane = near_p;
    camera->far_plane = far_p;
    
    // Hareket Ayarları
    camera->move_speed = 5.0f; // 5 birim/saniye
    camera->turn_speed = 1.0f; // 1 radyan/saniye
    
    // İlk hesaplamaları yap
    fe_camera3d_update_vectors(camera);
    fe_camera3d_update_matrices(camera);
    
    FE_LOG_DEBUG("3D Kamera olusturuldu. FOV: %f, Near/Far: %f/%f", fov_y, near_p, far_p);
    return camera;
}

/**
 * Uygulama: fe_camera3d_destroy
 */
void fe_camera3d_destroy(fe_camera3d_t* camera) {
    if (camera) {
        free(camera);
        FE_LOG_DEBUG("3D Kamera yok edildi.");
    }
}

// ----------------------------------------------------------------------
// 2. MATRİS VE YÖN HESAPLAMALARI
// ----------------------------------------------------------------------

/**
 * Uygulama: fe_camera3d_update_vectors
 * Hesaplama: yaw ve pitch açılarını kullanarak ileri, sağ ve yukarı vektörlerini (forward, right, up) hesaplar.
 */
void fe_camera3d_update_vectors(fe_camera3d_t* camera) {
    // Trigonometrik hesaplamalar
    float cos_yaw = cosf(camera->yaw);
    float sin_yaw = sinf(camera->yaw);
    float cos_pitch = cosf(camera->pitch);
    float sin_pitch = sinf(camera->pitch);
    
    // 1. Forward (İleri) Vektör Hesaplaması (Yön vektörü)
    // Dünya koordinat sisteminde Y-Up ve -Z ileri varsayımı ile
    camera->forward.x = cos_yaw * cos_pitch;
    camera->forward.y = sin_pitch; // Pitch, Y ekseni etrafında dönme olarak kabul edilir.
    camera->forward.z = sin_yaw * cos_pitch;
    
    camera->forward = fe_vec3_normalize(camera->forward);
    
    // 2. Right (Sağ) Vektör Hesaplaması (Forward ve Dünya Yukarı (0, 1, 0) Vektöründen Çapraz Çarpım)
    fe_vec3_t world_up = {0.0f, 1.0f, 0.0f}; // Dünya yukarı vektörü
    camera->right = fe_vec3_cross(camera->forward, world_up);
    camera->right = fe_vec3_normalize(camera->right);
    
    // 3. Up (Yukarı) Vektör Hesaplaması (Sağ ve İleri Vektöründen Çapraz Çarpım)
    // Not: Normalde Right x Forward = Up'tır, ancak koordinat sistemine dikkat edilmeli.
    // fe_vec3_cross(Right, Forward) bize kameranın yerel Up vektörünü verir.
    camera->up = fe_vec3_cross(camera->right, camera->forward); 
    // Up'ın normalize edilmesine genellikle gerek yoktur, çünkü Right ve Forward zaten normalize edilmiştir
}

/**
 * Uygulama: fe_camera3d_update_matrices
 */
void fe_camera3d_update_matrices(fe_camera3d_t* camera) {
    // 1. View Matrisi Hesaplaması (LookAt matrisine eşdeğer)
    // LookAt(Position, Position + Forward, Up)
    fe_vec3_t target = fe_vec3_add(camera->position, camera->forward);
    camera->view_matrix = fe_mat4_look_at(camera->position, target, camera->up);
    
    // 2. Projection Matrisi Hesaplaması (Perspective)
    camera->projection_matrix = fe_mat4_perspective(
        camera->fov_y, 
        camera->aspect_ratio, 
        camera->near_plane, 
        camera->far_plane
    );
    
    // 3. View * Projection Matrisi
    camera->view_proj_matrix = fe_mat4_multiply(camera->projection_matrix, camera->view_matrix);
    
    // 
}

// ----------------------------------------------------------------------
// 3. HAREKET VE DÖNME İŞLEMLERİ
// ----------------------------------------------------------------------

/**
 * Uygulama: fe_camera3d_set_transform
 */
void fe_camera3d_set_transform(fe_camera3d_t* camera, const fe_vec3_t* position, float yaw, float pitch) {
    memcpy(&camera->position, position, sizeof(fe_vec3_t));
    camera->yaw = yaw;
    camera->pitch = pitch;
    
    // Değişiklikler sonrası matrisleri ve vektörleri hemen güncelle
    fe_camera3d_update_vectors(camera);
    fe_camera3d_update_matrices(camera);
}

/**
 * Uygulama: fe_camera3d_move
 * Direction: Z=İleri/Geri, X=Sağ/Sol, Y=Yukarı/Aşağı
 */
void fe_camera3d_move(fe_camera3d_t* camera, const fe_vec3_t* direction, float delta_time) {
    float distance = camera->move_speed * delta_time;
    
    // İleri/Geri hareket (Yerel Z ekseni, yani Forward vektörü)
    if (fabsf(direction->z) > 0.0f) {
        fe_vec3_t move_vec = fe_vec3_scale(camera->forward, direction->z * distance);
        camera->position = fe_vec3_add(camera->position, move_vec);
    }
    
    // Sağ/Sol hareket (Yerel X ekseni, yani Right vektörü)
    if (fabsf(direction->x) > 0.0f) {
        fe_vec3_t move_vec = fe_vec3_scale(camera->right, direction->x * distance);
        camera->position = fe_vec3_add(camera->position, move_vec);
    }
    
    // Yukarı/Aşağı hareket (Genellikle Dünya Y ekseni, ancak Yerel Y de olabilir)
    // Burada dünya Y ekseni kullanılıyor (Fe engine'de Y-Up varsayımıyla)
    if (fabsf(direction->y) > 0.0f) {
        fe_vec3_t world_up = {0.0f, 1.0f, 0.0f};
        fe_vec3_t move_vec = fe_vec3_scale(world_up, direction->y * distance);
        camera->position = fe_vec3_add(camera->position, move_vec);
    }
    
    // Matrisi güncelle
    fe_camera3d_update_matrices(camera);
}

/**
 * Uygulama: fe_camera3d_rotate
 */
void fe_camera3d_rotate(fe_camera3d_t* camera, float delta_yaw, float delta_pitch) {
    float turn_amount = camera->turn_speed;
    
    // Yaw'i guncelle (Y ekseni etrafinda)
    camera->yaw += delta_yaw * turn_amount;
    
    // Pitch'i guncelle (Yerel X ekseni etrafinda)
    camera->pitch += delta_pitch * turn_amount;
    
    // Pitch sinirlamasi (Kameranin takla atmasini engelle)
    if (camera->pitch > MAX_PITCH_RAD) {
        camera->pitch = MAX_PITCH_RAD;
    } else if (camera->pitch < -MAX_PITCH_RAD) {
        camera->pitch = -MAX_PITCH_RAD;
    }
    
    // Vektörleri ve matrisleri yeniden hesapla
    fe_camera3d_update_vectors(camera);
    fe_camera3d_update_matrices(camera);
}
