#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fcntl.h"

int
main(void)
{
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