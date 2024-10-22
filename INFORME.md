# Nicolás Durán R 
Tarea 2 Sistemas Operativos 

# xv6-riscv: Implementación de un Scheduler con Prioridades

Este proyecto modifica el sistema operativo xv6 para implementar un algoritmo de planificación de procesos con prioridades. En este `INFORME.md` se detallan los pasos seguidos, los cambios realizados en los archivos `proc.h` y `proc.c`, y cómo compilar y ejecutar el sistema con estos cambios.

## Objetivo

Modificar el scheduler de xv6 para que los procesos sean seleccionados en base a una `prioridad`. Además, cada proceso tendrá un valor `boost` que incrementará o decrementará la prioridad dependiendo de si alcanza los límites superior o inferior.

## Archivos modificados

- `kernel/proc.h`: Se añadieron nuevas variables de prioridad y boost en la estructura `proc`.
- `kernel/proc.c`: Se modificó la función `scheduler()` para implementar la lógica de selección de procesos según su prioridad.

## Pasos realizados

### 1. Añadir campos de prioridad y boost en `proc.h`

En el archivo `proc.h`, dentro de la estructura `proc`, se añadieron dos nuevos campos:
- `prioridad`: Indica la prioridad del proceso. Un valor más bajo indica mayor prioridad.
- `boost`: Este valor se usa para incrementar o decrementar la prioridad.

```c
struct proc {
  ...
  int prioridad;  // Campo que indica la prioridad del proceso.
  int boost;      // Campo que indica si la prioridad debe incrementarse o decrementarse.
  ...
};
```

### 2. Modificar el scheduler en `proc.c`

En el archivo `proc.c`, se realizó una modificación en la función `scheduler()` para implementar la lógica de selección de procesos basada en la prioridad.

#### Cambios principales:

1. **Actualización de la prioridad**:
   - Se añadió lógica para aumentar o disminuir la prioridad de un proceso según el valor de `boost`.
   - Si la prioridad de un proceso llega a 9 (máximo), el `boost` se ajusta a -1 (decrementa).
   - Si la prioridad baja a 0 (mínimo), el `boost` se ajusta a 1 (incrementa).

```c
for(p = proc; p < &proc[NPROC]; p++) {
  acquire(&p->lock);
  if(p->state == RUNNABLE) {
    p->prioridad += p->boost;
    if(p->prioridad >= 9) {
      p->boost = -1;
    }
    if(p->prioridad <= 0) {
      p->boost = 1;
    }
  }
  release(&p->lock);
}
```

2. **Selección del proceso con mayor prioridad**:
   - Se recorre la lista de procesos en busca del que tenga la menor prioridad (recuerda que una menor prioridad numérica indica una prioridad más alta).
   - Se selecciona el proceso y se hace el cambio de contexto con `swtch()`.

```c
struct proc *chosen_proc = 0;
int highest_priority = 10;  // Mayor prioridad numérica (menos importante).

for(p = proc; p < &proc[NPROC]; p++ ) {
  acquire(&p->lock);
  if(p->state == RUNNABLE && p->prioridad < highest_priority) {
    highest_priority = p->prioridad;
    chosen_proc = p;
  }
  release(&p->lock);
}

if(chosen_proc != 0) {
  acquire(&chosen_proc->lock);
  chosen_proc->state = RUNNING;
  c->proc = chosen_proc;
  
  // Cambio de contexto.
  swtch(&c->context, &chosen_proc->context);
  
  // Proceso listo para ejecutar.
  c->proc = 0;
  release(&chosen_proc->lock);
} else {
  intr_on();
  asm volatile("wfi");
}
```

### 3. Compilación y ejecución

Para compilar y ejecutar xv6 con estos cambios, sigue los siguientes pasos:

1. Navega al directorio del kernel:

   ```bash
   cd kernel
   ```

2. Compila el sistema:

   ```bash
   make
   ```

3. Ejecuta el sistema en QEMU:

   ```bash
   make qemu
   ```



## errores y solución

- **"panic: kerneltrap"**: Este error puede ocurrir si hay accesos incorrectos a memoria o problemas en el manejo de prioridades. Verifica que todos los procesos estén protegidos con bloqueos y que no se intente acceder a procesos inválidos.
- **Compilación fallida**: Asegúrate de que todas las llaves y paréntesis de las estructuras de control (condicionales y bucles) estén correctamente cerrados.
