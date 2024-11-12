#include "types.h"
#include "stat.h"
#include "user.h"

int main() {
    char *addr = sbrk(0);  // Obtener la dirección actual del heap
    sbrk(4096);  // Reservar una página

    // Intentar proteger la nueva página
    if (mprotect(addr, 1) == -1) {
        printf("mprotect falló\n");
    }

    // Intentar escribir en la página protegida
    char *ptr = addr;
    *ptr = 'A';  // Esto debería fallar si la protección es exitosa
    printf("Valor en la dirección: %c\n", *ptr);  // Verificar el valor

    return 0;
}
