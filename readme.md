# Nicolas Duran R                   #             Sistemas Opertativos UAI 2024

## Implementación de Comunicación entre Procesos (IPC) en xv6

Este proyecto implementa un sistema simple de comunicación entre procesos (IPC) en xv6 utilizando una **cola de mensajes** dentro del kernel. Se realizaron modificaciones al código fuente de xv6 para agregar dos nuevas llamadas al sistema: `sys_send` y `sys_receive`. Estos cambios permiten que un proceso envíe mensajes a una cola compartida y que otro proceso los reciba, garantizando sincronización y orden.

## Objetivos

1. Crear una cola de mensajes global dentro del kernel.
2. Implementar dos nuevas llamadas al sistema: `sys_send` y `sys_receive`.
3. Usar mecanismos de sincronización (`spinlock`, `sleep`, `wakeup`) para garantizar la correcta funcionalidad.
4. Validar el orden de los mensajes y el bloqueo/despertar del lector.

---

## Cambios realizados al código fuente

### 1. **Definición de la estructura de mensajes**

Se agregó la estructura `message` para representar los mensajes en la cola. Esto se realizó en el archivo `proc.h`:

```c
typedef struct message {
    int sender_pid;       // ID del proceso remitente
    char content[128];    // Contenido del mensaje
} message;
```

Además, se definió la estructura `message_queue` para la cola de mensajes y se declaró una instancia global en `proc.c`:

```c
#define MAX_MESSAGES 64

typedef struct message_queue {
    message messages[MAX_MESSAGES]; // Cola de mensajes
    int head;                        // Índice del primer mensaje
    int tail;                        // Índice del próximo espacio disponible
    int size;                        // Número actual de mensajes
    struct spinlock lock;            // Spinlock para sincronización
} message_queue;

message_queue msg_queue;
```

---

### 2. **Inicialización de la cola de mensajes**

La función `init_message_queue` inicializa el `spinlock` y las variables de la cola de mensajes. Fue implementada en `proc.c`:

```c
void init_message_queue() {
    initlock(&msg_queue.lock, "msg_queue");
    msg_queue.head = 0;
    msg_queue.tail = 0;
    msg_queue.size = 0;
}
```

Esta función se invoca desde `main()` en `main.c`:

```c
void main() {
    // Inicializaciones previas...
    init_message_queue(); // Inicializa la cola de mensajes
    userinit();
    mpmain();
}
```

---

### 3. **Implementación de `sys_send`**

La función `sys_send` permite que un proceso envíe un mensaje a la cola. Fue implementada en `sysproc.c`:

```c
int sys_send(void) {
    int sender_pid = myproc()->pid;
    char *content;

    if (argstr(0, (char **)&content) < 0)
        return -1;

    acquire(&msg_queue.lock);

    if (msg_queue.size >= MAX_MESSAGES) {
        release(&msg_queue.lock);
        return -1; // Cola llena
    }

    message msg;
    msg.sender_pid = sender_pid;
    safestrcpy(msg.content, content, sizeof(msg.content));

    msg_queue.messages[msg_queue.tail] = msg;
    msg_queue.tail = (msg_queue.tail + 1) % MAX_MESSAGES;
    msg_queue.size++;

    wakeup(&msg_queue);

    release(&msg_queue.lock);
    return 0;
}
```

Se agregó la declaración de `sys_send` en `syscall.h`:

```c
int sys_send(void);
```

Y se asoció en `syscall.c`:

```c
[SYS_send] sys_send,
```

---

### 4. **Implementación de `sys_receive`**

La función `sys_receive` permite que un proceso reciba mensajes de la cola. Fue implementada en `sysproc.c`:

```c
int sys_receive(void) {
    message *user_msg;

    if (argptr(0, (void *)&user_msg, sizeof(message)) < 0)
        return -1;

    acquire(&msg_queue.lock);

    while (msg_queue.size == 0) {
        sleep(&msg_queue, &msg_queue.lock);
    }

    *user_msg = msg_queue.messages[msg_queue.head];
    msg_queue.head = (msg_queue.head + 1) % MAX_MESSAGES;
    msg_queue.size--;

    release(&msg_queue.lock);
    return 0;
}
```

Se agregó la declaración de `sys_receive` en `syscall.h`:

```c
int sys_receive(void);
```

Y se asoció en `syscall.c`:

```c
[SYS_receive] sys_receive,
```

---

### 5. **Programa de prueba**

Se creó un programa `ipc_test.c` en la carpeta `user/` para validar la funcionalidad de `sys_send` y `sys_receive`:

```c
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(void) {
    int pid = fork();

    if (pid == 0) {
        // Proceso lector
        message msg;
        for (int i = 0; i < 5; i++) {
            if (receive(&msg) == 0) {
                printf("Lector recibió: '%s' de PID %d
", msg.content, msg.sender_pid);
            }
        }
        exit(0);
    } else {
        // Proceso escritor
        sleep(10); // Asegura que el lector se bloquee
        for (int i = 0; i < 5; i++) {
            char msg[128];
            snprintf(msg, sizeof(msg), "Mensaje %d", i);
            send(msg);
        }
        wait(0);
    }

    exit(0);
}
```

El programa se agregó al `Makefile` bajo `UPROGS`:

```make
UPROGS=\
  ...
  _ipc_test\
```

---

### **Validaciones realizadas**

1. **Orden de los mensajes:**
   Los mensajes enviados por el escritor se reciben en el mismo orden por el lector.

2. **Bloqueo del lector:**
   El lector se bloquea automáticamente cuando no hay mensajes en la cola.

3. **Despertar del lector:**
   El lector se despierta correctamente cuando el escritor agrega un nuevo mensaje.

---

### **Cómo ejecutar**

1. Compila y ejecuta xv6:
   ```bash
   make clean && make qemu
   ```

2. Corre el programa de prueba:
   ```bash
   $ ipc_test
   ```

---

### **Resultados esperados**

1. El lector muestra que está esperando mensajes.
2. Los mensajes enviados por el escritor son recibidos y mostrados en orden por el lector.
3. El lector se bloquea cuando no hay mensajes y se despierta al llegar nuevos mensajes.
