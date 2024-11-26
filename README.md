Primera Parte: Modificación de la estructura de inode

    Editar inode en fs.h:
        Abrimos el archivo fs.h.
        Modificamos la estructura de struct inode para agregar un campo llamado "perm" que contendrá los permisos.
        Este campo será un int y debe ser inicializado en 3 (read/write) cuando se cree un nuevo archivo.
´´´

'''´´´
    struct inode {
    ...
    int perm; // Permisos del archivo: 0 = sin acceso, 1 = solo lectura, 2 = solo escritura, 3 = lectura/escritura
};
'''´´´
´´´
ok