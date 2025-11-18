// src/platform/fe_platform_io.c

// Bu dosya, sadece fe_platform_io.h başlık dosyasının kendisini derleyicinin
// tanımasını sağlamak ve gerekirse gelecekteki merkezi I/O mantığını barındırmak
// için burada durmaktadır. Şu an için sadece genel başlığı dahil etmesi yeterlidir.

#include "platform/fe_platform_io.h"
#include "utils/fe_logger.h"

// İleride, dosya I/O işlemlerini izlemek veya profil oluşturmak
// için merkezi fonksiyonları burada tanımlayabilirsiniz.
// Şimdilik, tüm çağrılar makrolar aracılığıyla doğrudan platforma özel
// dosyalara yönlendirildiği için bu dosya boş kalabilir.