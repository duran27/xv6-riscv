#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/sysproc.c"
#include "kernel/proc.h"

// Programa principal
int main(void) {
    int pid = fork();

    if (pid == 0) {
        // Proceso lector
        message msg; // Estructura para recibir mensajes
        printf("Proceso lector esperando mensajes...\n");
        for (int i = 0; i < 5; i++) { // Recibe 5 mensajes como ejemplo
            if (receive(&msg) < 0) {
                printf("Error al recibir el mensaje\n");
            } else {
                printf("Mensaje recibido de PID %d: %s\n", msg.sender_pid, msg.content);
            }
            sleep(10); // Simula una pequeña espera entre recepciones
        }
        exit(0);
    } else if (pid > 0) {
        // Proceso escritor
        sleep(10); // Da tiempo al lector para que se bloquee
        for (int i = 0; i < 5; i++) { // Envia 5 mensajes como ejemplo
            char msg[128];
            snprintf(msg, sizeof(msg), "Hola, soy el escritor! Mensaje %d", i);
            if (send(msg) < 0) {
                printf("Error al enviar el mensaje %d\n", i);
            } else {
                printf("Mensaje enviado: %s\n", msg);
            }
            sleep(10); // Simula un pequeño retardo entre envíos
        }
        wait(0); // Espera a que el lector termine
    } else {
        printf("Error al hacer fork\n");
    }

    exit(0);
}
