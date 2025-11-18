// include/editor/fe_editor.h

#ifndef FE_EDITOR_H
#define FE_EDITOR_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "data_structures/fe_array.h" // Düğümleri ve bağlantıları tutmak için

// ----------------------------------------------------------------------
// 1. TEMEL TİPLER (PİN VE DÜĞÜM)
// ----------------------------------------------------------------------

/**
 * @brief Düğüm pinlerinin/giriş/çıkışlarının türleri.
 */
typedef enum fe_pin_type {
    FE_PIN_TYPE_EXECUTION,  // Yürütme akışı (Beyaz çizgi - "Ne Zaman")
    FE_PIN_TYPE_DATA,       // Veri akışı (Renk kodlu - "Ne")
    FE_PIN_TYPE_BOOL,
    FE_PIN_TYPE_INT,
    FE_PIN_TYPE_FLOAT,
    FE_PIN_TYPE_VEC3,
    // ... Motorun diğer temel tipleri eklenebilir
} fe_pin_type_t;

/**
 * @brief Bir düğümdeki giriş veya çıkış noktasını (Pin) temsil eder.
 */
typedef struct fe_node_pin {
    uint32_t id;                // Pin'in benzersiz ID'si
    fe_pin_type_t type;         // Pin türü
    bool is_input;              // true ise giriş (Input), false ise çıkış (Output)
    // char* name;               // Pin'in görünen adı
    // void* default_value;      // Varsayılan değer (Input pinleri için)
    // uint32_t connected_pin_id; // Bağlandığı diğer pinin ID'si
} fe_node_pin_t;

/**
 * @brief Görsel komut dosyasının temel yürütme birimi (Fonksiyon, Olay, Değişken).
 */
typedef struct fe_editor_node {
    uint32_t id;                    // Düğümün benzersiz ID'si
    // char* name;                   // Düğümün adı (örn: "AddVector", "OnBeginPlay")
    // char* func_name;              // Düğümün çağırdığı C fonksiyonunun adı (yürütme için)
    fe_array_t input_pins;          // fe_node_pin_t dizisi
    fe_array_t output_pins;         // fe_node_pin_t dizisi
    // fe_vec2_t position;           // Düzenleyici penceresindeki pozisyonu (görselleştirme için)
} fe_editor_node_t;


// ----------------------------------------------------------------------
// 2. GRAFİK VE YÜRÜTME YAPISI
// ----------------------------------------------------------------------

/**
 * @brief Bir düğüm arasındaki bağlantıyı temsil eder (Çıkış Pini -> Giriş Pini).
 */
typedef struct fe_node_link {
    uint32_t id;
    uint32_t start_pin_id;      // Kaynak (Output) Pinin ID'si
    uint32_t end_pin_id;        // Hedef (Input) Pinin ID'si
} fe_node_link_t;

/**
 * @brief Bir varlık (Entity) veya seviyeye (Level) ait görsel komut dosyasını tutan ana grafik.
 */
typedef struct fe_node_graph {
    uint32_t target_id;         // Bu grafiğin ait olduğu Entity/Level ID'si
    fe_array_t nodes;           // fe_editor_node_t dizisi
    fe_array_t links;           // fe_node_link_t dizisi
} fe_node_graph_t;


// ----------------------------------------------------------------------
// 3. ÇALIŞMA ZAMANI YÜRÜTÜCÜ (EXECUTIVE)
// ----------------------------------------------------------------------

/**
 * @brief Verilen bir düğüm grafiğini yürütür (Çalıştırır).
 * * Bu, Blueprint'in çalışma zamanı mantığıdır.
 * @param graph Yürütülecek fe_node_graph_t.
 * @param event_node_id Yürütmenin baslayacagi Olay (Event) düğümünün ID'si (örn: OnBeginPlay).
 * @param execution_context Yürütme sırasındaki anlık verileri tutan bir yapı.
 * @return Başarılıysa true, degilse false.
 */
bool fe_editor_execute_graph(fe_node_graph_t* graph, uint32_t event_node_id, void* execution_context);

/**
 * @brief Yeni bir düğüm grafiği olusturur.
 */
fe_node_graph_t* fe_node_graph_create(uint32_t target_id);

/**
 * @brief Düğüm grafiğini bellekten serbest birakir.
 */
void fe_node_graph_destroy(fe_node_graph_t* graph);

#endif // FE_EDITOR_H
