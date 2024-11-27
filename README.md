### Nicolás Durán R
# Tarea 4

## 1RA PARTE


# Implementación de Sistema de Permisos en xv6

Este documento describe los cambios realizados al sistema operativo xv6 para implementar un sistema de permisos básicos, incluyendo un permiso especial de inmutabilidad.

## Parte 1: Implementación Básica de Permisos

### Objetivo

Implementar un sistema de permisos en xv6 que permita modificar archivos para tener acceso de solo lectura o lectura/escritura.



### 1. Modificación de la Estructura de `inode` y `dinode`

Se añadió un campo `permissions` a las estructuras `inode` y `dinode` para almacenar los permisos de los archivos.

##### `file.h`

```c
struct inode {
  ...
  int permissions; // Añadir este campo
  ...
};
```


##### `fs.h`
```
struct dinode {
  ...
  int permissions; // Añadir este campo
  ...
};
```
### 2. Inicialización de Permisos al Crear un Inodo

Se inicializó el campo permissions al crear un nuevo inodo.

##### `fs.c`

```c
struct inode* ialloc(uint dev, short type) {
  ...
  for(inum = 1; inum < sb.ninodes; inum++){
    bp = bread(dev, IBLOCK(inum, sb));
    dip = (struct dinode*)bp->data + inum%IPB;
    if(dip->type == 0){  // a free inode
      memset(dip, 0, sizeof(*dip));
      dip->type = type;
      dip->permissions = 3; // Lectura y escritura por defecto
      log_write(bp);   // mark it allocated on the disk
      brelse(bp);
      return iget(dev, inum);
    }
    brelse(bp);
  }
  printf("ialloc: no inodes\n");
  return 0;
}
```

### 3. Actualización de Permisos en el Disco
Se actualizó el campo permissions en el disco al modificar un inodo.

##### `fs.c`
```c
void iupdate(struct inode *ip) {
  struct buf *bp;
  struct dinode *dip;
  ...

  dip->permissions = ip->permissions; // Copiar permisos
  memmove(dip->addrs, ip->addrs, sizeof(ip->addrs));
  log_write(bp);
  brelse(bp);
}
```

### 4. Modificación de sys_open
Se modificó sys_open para verificar los permisos antes de abrir un archivo.
##### `sysfile.c`

```c
int sys_open(void) {
  ...
  if((omode & O_WRONLY) && !(ip->permissions & 2)) {
    iunlockput(ip);
    end_op();
    return -1; // No tiene permiso de escritura
  }
  ...
}
```

### 5. Modificación de readi y writei
Se modificaron readi y writei para respetar los permisos de los archivos.

##### `fs.c`

```c
int readi(struct inode *ip, int user_dst, uint64 dst, uint off, uint n) {
  ...
  if(!(ip->permissions & 1)) {
    return -1; // No tiene permiso de lectura
  }
  ...
}

int writei(struct inode *ip, int user_src, uint64 src, uint off, uint n) {
  ...
  if(!(ip->permissions & 2)) {
    return -1; // No tiene permiso de escritura
  }
  ...
}

```

### 6. Implementación de la Llamada al Sistema chmod
Se implementó la llamada al sistema chmod para cambiar los permisos de un archivo.

##### `sysfile.c`

```c
uint64 sys_chmod(void) {
  char path[MAXPATH];
  int mode;
  struct inode *ip;

  if(argstr(0, path, MAXPATH) < 0 || argint(1, &mode) < 0)
    return -1;

  begin_op();
  if((ip = namei(path)) == 0){
    end_op();
    return -1;
  }
  ilock(ip);
  ip->permissions = mode;
  iupdate(ip); // Actualizar el inodo en el disco
  iunlockput(ip);
  end_op();

  return 0;
}
```

##### `sysfile.h`

```
#define SYS_chmod 22
```
##### `syscall.c`

```c
extern uint64 sys_chmod(void);

static uint64 (*syscalls[])(void) = {
  ...
  [SYS_chmod]   sys_chmod,
};
```
Por último, añadimos a usys.S
#### `usys.S`
```
.global chmod
chmod:
 li a7, SYS_chmod
 ecall
 ret
```

## Prueba Parte I 

añadimos el archivo prueba_chmod.c a `user` y luego añadimos la ruta del programa al `MAKEFILE` para poder hacer make qemu nuevamente.

```c
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fcntl.h"

int main(void) {
  int fd;
  char *filename = "testfile";
  char *data = "Hello, xv6!";
  
  // Creación del Archivo
  fd = open(filename, O_CREATE | O_RDWR);
  if(fd < 0){
    printf("Error: No se pudo crear el archivo\n");
    exit(1);
  }

  // Escritura Inicial
  if(write(fd, data, strlen(data)) != strlen(data)){
    printf("Error: No se pudo escribir en el archivo\n");
    close(fd);
    exit(1);
  }
  close(fd);

  // Cambio de Permisos a Solo Lectura
  if(chmod(filename, 1) < 0){
    printf("Error: No se pudo cambiar los permisos a solo lectura\n");
    exit(1);
  }

  // Prueba de Escritura con Solo Lectura
  fd = open(filename, O_WRONLY);
  if(fd >= 0){
    printf("Error: Se pudo abrir el archivo en modo escritura cuando debería ser solo lectura\n");
    close(fd);
    exit(1);
  } else {
    printf("Correcto: No se pudo abrir el archivo en modo escritura\n");
  }

  // Cambio de Permisos de Vuelta a Lectura/Escritura
  if(chmod(filename, 3) < 0){
    printf("Error: No se pudo cambiar los permisos a lectura/escritura\n");
    exit(1);
  }

  // Escritura Final
  fd = open(filename, O_RDWR);
  if(fd < 0){
    printf("Error: No se pudo abrir el archivo en modo lectura/escritura\n");
    exit(1);
  }
  if(write(fd, data, strlen(data)) != strlen(data)){
    printf("Error: No se pudo escribir en el archivo\n");
    close(fd);
    exit(1);
  }
  close(fd);

  printf("Pruebas completadas exitosamente\n");
  exit(0);
}

```


## 2DA PARTE
# Implementación del Permiso de Inmutabilidad

### 1. Modificación de sys_chmod

##### `sysfile.c`
```c
uint64 sys_chmod(void) {
  char path[MAXPATH];
  int mode;
  struct inode *ip;

  if(argstr(0, path, MAXPATH) < 0 || argint(1, &mode) < 0)
    return -1;

  begin_op();
  if((ip = namei(path)) == 0){
    end_op();
    return -1;
  }
  ilock(ip);

  // Verificar si el archivo es inmutable
  if(ip->permissions == 5){
    iunlockput(ip);
    end_op();
    return -1; // No se puede cambiar los permisos de un archivo inmutable
  }

  ip->permissions = mode;
  iupdate(ip); // Actualizar el inodo en el disco
  iunlockput(ip);
  end_op();

  return 0;
}
```

### 2. Modificación de sys_open, readi y writei

##### `sysfile.c`
```c
int sys_open(void) {
  ...
  if((omode & O_WRONLY) && (!(ip->permissions & 2) || ip->permissions == 5)) {
    iunlockput(ip);
    end_op();
    return -1; // No tiene permiso de escritura o es inmutable
  }
  ...
}
```

##### `fs.c`

```c
int readi(struct inode *ip, int user_dst, uint64 dst, uint off, uint n) {
  ...
  if(!(ip->permissions & 1)) {
    return -1; // No tiene permiso de lectura
  }
  ...
}

int writei(struct inode *ip, int user_src, uint64 src, uint off, uint n) {
  ...
  if(!(ip->permissions & 2) || ip->permissions == 5) {
    return -1; // No tiene permiso de escritura o es inmutable
  }
  ...
}

```

## Prueba Parte II

añadimos el archivo prueba_parte2.c a `user` y luego añadimos la ruta del programa al `MAKEFILE` para poder hacer make qemu nuevamente.

```c
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fcntl.h"

int main(void) {
  int fd;
  char *filename = "testfile";
  char *data = "Hello, xv6!";
  
  // Creación del Archivo
  fd = open(filename, O_CREATE | O_RDWR);
  if(fd < 0){
    printf("Error: No se pudo crear el archivo\n");
    exit(1);
  }

  // Escritura Inicial
  if(write(fd, data, strlen(data)) != strlen(data)){
    printf("Error: No se pudo escribir en el archivo\n");
    close(fd);
    exit(1);
  }
  close(fd);

  // Cambio de Permisos a Solo Lectura
  if(chmod(filename, 1) < 0){
    printf("Error: No se pudo cambiar los permisos a solo lectura\n");
    exit(1);
  }

  // Prueba de Escritura con Solo Lectura
  fd = open(filename, O_WRONLY);
  if(fd >= 0){
    printf("Error: Se pudo abrir el archivo en modo escritura cuando debería ser solo lectura\n");
    close(fd);
    exit(1);
  } else {
    printf("Correcto: No se pudo abrir el archivo en modo escritura\n");
  }

  // Cambio de Permisos de Vuelta a Lectura/Escritura
  if(chmod(filename, 3) < 0){
    printf("Error: No se pudo cambiar los permisos a lectura/escritura\n");
    exit(1);
  }

  // Escritura Final
  fd = open(filename, O_RDWR);
  if(fd < 0){
    printf("Error: No se pudo abrir el archivo en modo lectura/escritura\n");
    exit(1);
  }
  if(write(fd, data, strlen(data)) != strlen(data)){
    printf("Error: No se pudo escribir en el archivo\n");
    close(fd);
    exit(1);
  }
  close(fd);

  // Cambio de Permisos a Inmutable
  if(chmod(filename, 5) < 0){
    printf("Error: No se pudo cambiar los permisos a inmutable\n");
    exit(1);
  }

  // Prueba de Escritura con Solo Lectura (Inmutable)
  fd = open(filename, O_WRONLY);
  if(fd >= 0){
    printf("Error: Se pudo abrir el archivo en modo escritura cuando debería ser inmutable\n");
    close(fd);
    exit(1);
  } else {
    printf("Correcto: No se pudo abrir el archivo en modo escritura\n");
  }

  // Intento de Cambio de Permisos de Vuelta a Lectura/Escritura
  if(chmod(filename, 3) == 0){
    printf("Error: Se pudieron cambiar los permisos de un archivo inmutable\n");
    exit(1);
  } else {
    printf("Correcto: No se pudieron cambiar los permisos de un archivo inmutable\n");
  }

  printf("Pruebas completadas exitosamente\n");
  exit(0);
}
```

