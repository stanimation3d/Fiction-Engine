// src/editor/fe_editor.c

#include "editor/fe_editor.h"
#include "utils/fe_logger.h"
#include <stdlib.h> // malloc, free
#include <string.h> // memset

// ----------------------------------------------------------------------
// YARDIMCI VE YAŞAM DÖNGÜSÜ FONKSİYONLARI
// ----------------------------------------------------------------------

/**
 * Uygulama: fe_node_graph_create
 */
fe_node_graph_t* fe_node_graph_create(uint32_t target_id) {
    fe_node_graph_t* graph = (fe_node_graph_t*)calloc(1, sizeof(fe_node_graph_t));
    if (!graph) return NULL;

    graph->target_id = target_id;
    
    // Alt koleksiyonları başlat (fe_array_t kullanıldığı varsayılır)
    graph->nodes = fe_array_create(sizeof(fe_editor_node_t));
    graph->links = fe_array_create(sizeof(fe_node_link_t));

    return graph;
}

/**
 * Uygulama: fe_node_graph_destroy
 */
void fe_node_graph_destroy(fe_node_graph_t* graph) {
    if (graph) {
        // Tüm düğümler ve pinler serbest bırakılır
        fe_array_destroy(graph->nodes);
        fe_array_destroy(graph->links);
        free(graph);
    }
}

// ----------------------------------------------------------------------
// GRAFİK YÜRÜTME (EXECUTIVE)
// ----------------------------------------------------------------------

// Yürütme akışını izlemek için basit bir stack yapısı kullanılabilir.
fe_stack_t execution_stack;

/**
 * @brief Belirli bir düğümün işlevini (fonksiyon çağrısı) yürütür.
 * * Gerçek motor kodu bu fonksiyonda çağrılır.
 */
static void fe_execute_node_function(fe_editor_node_t* node, fe_node_graph_t* graph, void* context) {
    // 1. GEREKLİ GİRİŞ VERİLERİNİ HESAPLA (DATA FLOW)
    //    * Her input pin'i için bağlı olduğu output pin'inden veri al.
    //    * Eğer bağlı değilse, pin'in varsayılan değerini kullan.
    
    // 2. İLGİLİ MOTOR FONKSİYONUNU ÇAĞIR
    
    // Yer Tutucu Örn:
    if (node->id == 10) { // Varsayalım ki bu "PrintString" düğümü
        FE_LOG_DEBUG("GRAPH: PrintString düğümü yürütüldü!");
    } else if (node->id == 20) { // Varsayalım ki bu "MoveEntity" düğümü
        // fe_move_entity(context->entity_id, input_vec3);
        FE_LOG_DEBUG("GRAPH: MoveEntity düğümü yürütüldü.");
    }

    // 3. ÇIKIŞ VERİLERİNİ AYARLA (Sonraki düğümler için)
}

/**
 * Uygulama: fe_editor_execute_graph
 */
bool fe_editor_execute_graph(fe_node_graph_t* graph, uint32_t event_node_id, void* execution_context) {
    if (!graph) return false;
    
    // Yürütme Stack'ini başlat
    fe_stack_clear(&execution_stack); 
    
    // Başlangıç düğümünü stack'e ekle
    fe_stack_push(&execution_stack, &event_node_id);

    uint32_t current_node_id = event_node_id;
    
    FE_LOG_INFO("Görsel Grafik Yürütülmeye Basladi (Başlangıç: Node %u)", current_node_id);

    // Yürütme Stack boşalana kadar devam et
    while (true /* !fe_stack_is_empty(&execution_stack) */) {
        current_node_id = *(uint32_t*)fe_stack_pop(&execution_stack);
        
        // 1. Düğümü bul
        fe_editor_node_t* current_node = fe_find_node_by_id(graph, current_node_id);
        
        // 2. Düğümün fonksiyonunu çalıştır
        fe_execute_node_function(current_node, graph, execution_context);
        
        // 3. Yürütme Çıkış Pinlerini kontrol et (EXECUTION FLOW)
        //    * Output Execution Pin'i bul.
        //    * Bu pine bağlı olan Link'i bul.
        //    * Link'in hedef (End) pinine bağlı olan sonraki düğümü bul.
        //    * Sonraki düğümü stack'e ekle.
        
        // Yer tutucu döngü sonlandırma:
        if (current_node_id == 100) { // Bir "Return" veya "End" düğümü simülasyonu
            break;
        }
        
        // Simülasyon: İlerideki düğüme geç
        current_node_id++; 

        // Not: Gerçek yürütme, döngü veya koşullu dallanma (Branch) düğümleri
        // ile karmaşık hale gelecektir.
    }

    FE_LOG_INFO("Görsel Grafik Yürütme Tamamlandi.");
    return true;
}
