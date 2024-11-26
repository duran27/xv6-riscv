*Nicolás Durán R
**Tarea 4



I: Modificación de la estructura de inode

1.Editar inode en fs.h:
    Abrimos el archivo fs.h.
    Modificamos la estructura de struct inode para agregar un campo llamado "perm" que contendrá los permisos.
    Este campo será un int y debe ser inicializado en 3 (read/write) cuando se cree un nuevo archivo.


    struct inode {
    ...
    int perm; // Permisos del archivo: 0 = sin acceso, 1 = solo lectura, 2 = solo escritura, 3 = lectura/escritura
    };

2.Inicializar el valor de perm en ialloc:
    Edita fs.c en la función ialloc para que inicialice el nuevo campo perm en 3 al crear un nuevo archivo.

    ip->perm = 3; // Inicializar con permisos de lectura/escritura


II: Modificar las operaciones de apertura, lectura y escritura

1.Validar permisos en sys_open (archivo sysfile.c):
    Editamos la función sys_open para verificar que los permisos definidos en el inode coincidan con el modo de apertura (omode).
    Ahora devuelve un error si el archivo se abre en un modo no permitido.

Cambios en sys_open:

    if ((ip->perm & 1) == 0 && (omode & O_RDONLY)) {
        end_op();
        return -1; // Error: El archivo no tiene permisos de lectura
    }
    if ((ip->perm & 2) == 0 && (omode & O_WRONLY)) {
        end_op();
        return -1; // Error: El archivo no tiene permisos de escritura
    }

2. Validar permisos en readi y writei (en fs.c):
    Modificamos las funciones readi y writei para verificar si los permisos permiten la operación.

    if ((ip->perm & 1) == 0) {
        return -1; // Error: No se permiten lecturas
    }

    if ((ip->perm & 2) == 0) {
        return -1; // Error: No se permiten escrituras
    }

III: Crear la llamada al sistema chmod

1. Declarar chmod en syscall.h:
    agregamos un prototipo para chmod:

    int chmod(char *path, int mode);

2. Implementar chmod en sysfile.c:
    Agregamos nueva función que reciba el nombre del archivo y el nuevo modo (int mode)
    Usamos namei para buscar el archivo y validar que existe.
    Cambiamos el valor del campo "perm" del inode al valor recibido en mode

    int chmod(char *path, int mode) {
        struct inode *ip;
        begin_op();
        if ((ip = namei(path)) == 0) {
            end_op();
            return -1; // Error: Archivo no encontrado
        }
        ilock(ip);
        ip->perm = mode; // Cambiar permisos
        iunlock(ip);
        end_op();
        return 0; // Éxito
    }

3. Agregar chmod al sistema de llamadas (syscall.c):
    Registramos la función chmod en syscall.c para que esté disponible como una llamada al sistema.

    extern int sys_chmod(void);
    ...
    [SYS_chmod] sys_chmod,

en syscall.h: 

    #define SYS_chmod <número correspondiente>

en syscall.c:

    int sys_chmod(void) {
        char *path;
        int mode;
        if (argstr(0, &path) < 0 || argint(1, &mode) < 0)
            return -1;
        return chmod(path, mode);
    }
