## Nicolas Durán R 

## Implementación de `mprotect` y `munprotect` en xv6

Este informe detalla los pasos para agregar las syscalls `mprotect` y `munprotect` en el sistema operativo xv6, con el objetivo de permitir que los procesos puedan proteger y desproteger regiones de memoria.

---

## Paso 1: Modificación de `syscall.c` y `syscall.h`

### 1.1 En `syscall.h`
definimos los números de syscall para `mprotect` y `munprotect`.

```c
#define SYS_mprotect 22  // Asigna un número único no usado (ajustar si necesario)
#define SYS_munprotect 23
```

### 1.2 En `syscall.c`
Agregamos `sys_mprotect` y `sys_munprotect` en la lista de syscalls.

```c
extern uint64 sys_mprotect(void);
extern uint64 sys_munprotect(void);

static uint64 (*syscalls[])(void) = {
    // ... otras syscalls ...
    [SYS_mprotect]   sys_mprotect,
    [SYS_munprotect] sys_munprotect,
};
```

---

## Paso 2: Implementación en `sysproc.c`

En este archivo, se definen las funciones `sys_mprotect` y `sys_munprotect` que reciben los parámetros de usuario (`addr` y `len`) y llaman a las funciones `mprotect` y `munprotect` en `vm.c`.

```c
#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "proc.h"
#include "vm.h"

uint64 sys_mprotect(void) {
    uint64 addr;
    int len;

    if (argaddr(0, &addr) < 0 || argint(1, &len) < 0) {
        return -1;
    }
    return mprotect(addr, len);
}

uint64 sys_munprotect(void) {
    uint64 addr;
    int len;

    if (argaddr(0, &addr) < 0 || argint(1, &len) < 0) {
        return -1;
    }
    return munprotect(addr, len);
}
```

---

## Paso 3: Implementación de `mprotect` y `munprotect` en `vm.c`

En `vm.c`, se implementan las funciones `mprotect` y `munprotect` que manipulan las entradas de la tabla de páginas (PTE) para modificar los permisos de escritura.

```c
#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "memlayout.h"
#include "proc.h"

int mprotect(uint64 addr, int len) {
    struct proc *p = myproc();
    uint64 end = addr + len * PGSIZE;
    
    for (uint64 a = addr; a < end; a += PGSIZE) {
        pte_t *pte = walk(p->pagetable, a, 0);
        if (pte == 0 || (*pte & PTE_V) == 0) {
            return -1;  // Dirección no válida
        }
        *pte &= ~PTE_W;  // Elimina el bit de escritura
    }
    return 0;
}

int munprotect(uint64 addr, int len) {
    struct proc *p = myproc();
    uint64 end = addr + len * PGSIZE;

    for (uint64 a = addr; a < end; a += PGSIZE) {
        pte_t *pte = walk(p->pagetable, a, 0);
        if (pte == 0 || (*pte & PTE_V) == 0) {
            return -1;  // Dirección no válida
        }
        *pte |= PTE_W;  // Restaura el bit de escritura
    }
    return 0;
}
```

---

## Paso 4: Declaración en `defs.h`

Se agregan las declaraciones de las funciones `mprotect` y `munprotect` en `defs.h` para que puedan ser utilizadas en otros archivos del sistema.

```c
int mprotect(uint64 addr, int len);
int munprotect(uint64 addr, int len);
```



---

## Paso 5: Adición de `mprotect` y `munprotect` en `usys.S`

Para hacer que estas syscalls estén disponibles en el espacio de usuario, agregamos las entradas en `usys.S` para `mprotect` y `munprotect`.

```asm
.globl mprotect
.type mprotect,@function
mprotect:
    li a7, SYS_mprotect
    ecall
    ret

.globl munprotect
.type munprotect,@function
munprotect:
    li a7, SYS_munprotect
    ecall
    ret
```


## Paso 6: Adición de programa de prueba `mprotect_test.c` 



```c
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


```

## Paso 7: Cambio en archivo MAKEFILE para agregar el programa de prueba y poder vovler a compilar.

```
UPROGS=\
    _cat\
    _echo\
    ...
    _mprotect_test\
```
## Paso 8: Compilamos

```
make clean
make qemu
```
