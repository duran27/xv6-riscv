# Informe de Instalación y Configuración de xv6

## Información del Estudiante
* **Nicolás Durán R** 
* **Curso: Sistemas Operativos**
* **Universidad Adolfo Ibañez** 


### 1. Pasos Seguidos
* **Fork del repositorio:** Se ha creado un fork del repositorio original de xv6 en mi cuenta de GitHub.
* **Clonación local:** Se ha clonado el fork personal en una máquina virtual con Linux.
* **Clonación local de Toolchain:** Se ha clonado el repositorio de Toolchain oficial de la documentaciónde xv6.
* **Compilación de Toolchain:** se ha compilado Toolchain con "make".
* **Compilación de xv6:** se ha compilado xv6 con "make".
* **Ejecución de xv6:** se ha ejecutado xv6 con "make qemu".


### 2. Problemas Encontrados y Soluciones
* **Problema:** Al compilar xv6 había un error porque no encontraba el Toolchain.
* **Solución:**  probé instalando varias versiones y no me funcionaba. lo solucioné clonando y compilando el Toolchain de la documentación oficial.

### 3. Confirmación de Funcionamiento
Se ha confirmado que xv6 funciona correctamente al ejecutar los siguientes comandos:
```bash
ls
echo "Hola xv6"
cat README
