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
