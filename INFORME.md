# Nicolás Durán R 
Tarea 1 Sistemas Operativos (Parte I)

## Funcionamiento de las llamadas al sistema
- **getppid()** retorna el PID del proceso padre. El proceso padre es quién creó el proceso actual dentro del sistema operativo.
- a modo de ejemplo la llamada **getpid()** retorna el PID del proceso actual.

## Modificaciones código fuente:
### KERNEL

- **sysproc.c** → contiene la implementación de llamadas
  - agregamos `getppid()`.
  ```c
    uint64
    sys_getppid(void)
    {
      struct proc *p =myproc();
      return p->  parent -> pid;
    } // devuelve el puntero actual, apunta al proceso padre, identificador del proceso
  }
  ```

- **syscall.h**
  - agregamos:
    ```c
    #define SYS_getppid 25
    ```

- **syscall.c**
  - agregamos:
    ```c
    [SYS_getppid] = sys_getppid,  // mapping
    extern uint64 sys_getppid(void);  // prototipo
    ```

### USER

- **user.h** → contiene las declaraciones de funciones
  - agregamos:
    ```c
    int getppid(void);
    ```
- **usys.pl** 
  - agregamos:
    ```c
    entry("getppid");
    ```
- **yosoytupadre.c**
  -agregamos programa de prueba:
    ```c
	#include "user.h"
    #include "kernel/stat.h"
    #include "kernel/types.h"


    int main(void)
    {
    
        printf("Yo soy tu padre - dijo el proceso: %i\n", getppid());
        exit(0);
    }
    ```


## Dificultades encontradas
<p style="text-align: justify;">Encontré una dificultad a la hora compilar el código luego de haber realizado las modificaciones para la implementación. La verdad si bien entendía el error que me mostraba por consola no pude resolverlo.
Lo que hice fue crear otra rama de mi Fork y volver a realizar los pasos, el único cambio que realicé con respecto a mi iteración anterior fue que el numero de proceso que le asigné a esta nueva llamada fue el 25, el cual lo elegí de manera arbitraria.
Finalmente, vovlí a compilar y ejecutar xv6 sin problemas y realizar las pruebas.</p>


