#include "user.h"

int
main(void)
{
    int ppid = getppid();   // Llama a la función que acabas de implementar
    printf("PID del padre: %d\n", ppid);
    exit(0);
}
