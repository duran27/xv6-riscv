#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "proc.c"

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return wait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int n;

  argint(0, &n);
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;

  argint(0, &n);
  if(n < 0)
    n = 0;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(killed(myproc())){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

int sys_send(void) {
    int sender_pid = myproc()->pid; // Obtén el PID del proceso actual
    char *content;                 // Puntero al contenido del mensaje

    // Obtén el argumento del contenido (pasado desde espacio de usuario)
    if (argstr(0, (void *)&content) < 0)
        return -1;

    // Asegúrate de que el contenido no sea NULL
    if (content == 0)
        return -1;

    // Protege el acceso a la cola de mensajes
    acquire(&msg_queue.lock);

    // Verifica si la cola está llena
    if (msg_queue.size >= MAX_MESSAGES) {
        release(&msg_queue.lock);
        return -1; // Retorna error si la cola está llena
    }

    // Crea el mensaje
    message msg;
    msg.sender_pid = sender_pid;
    safestrcpy(msg.content, content, sizeof(msg.content));

    // Añade el mensaje a la cola
    msg_queue.messages[msg_queue.tail] = msg;
    msg_queue.tail = (msg_queue.tail + 1) % MAX_MESSAGES;
    msg_queue.size++;

    // Despierta a los procesos que podrían estar esperando mensajes
    wakeup(&msg_queue);

    release(&msg_queue.lock); // Libera el spinlock

    return 0; // Éxito
}


int sys_receive(void) {
    message *user_msg;  // Puntero al espacio de usuario para almacenar el mensaje

    // Obtiene el puntero del argumento (pasado desde el espacio de usuario)
    if (argptr(0, (void *)&user_msg, sizeof(message)) < 0)
        return -1;

    acquire(&msg_queue.lock);  // Protege la cola con un spinlock

    // Si no hay mensajes en la cola, bloquea el proceso hasta que llegue uno
    while (msg_queue.size == 0) {
        sleep(&msg_queue, &msg_queue.lock);  // Bloquea el proceso y libera el spinlock
    }

    // Extrae el mensaje de la cola
    *user_msg = msg_queue.messages[msg_queue.head];
    msg_queue.head = (msg_queue.head + 1) % MAX_MESSAGES;
    msg_queue.size--;

    release(&msg_queue.lock);  // Libera el spinlock

    return 0;  // Éxito
}
