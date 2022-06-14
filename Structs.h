#ifndef VACASJUNIO_MIA_PROYECTO1_STRUCTS_H
#define VACASJUNIO_MIA_PROYECTO1_STRUCTS_H

#include <ctime>

struct Particion {
    char    status = '0';
    char    type = ' ';
    char    fit = ' ';
    int     start = 0;
    int     size = 0;
    char    name[16] = "";
};

struct MBR {
    int         tamano;
    char        fecha_creacion[20];
    int         disk_signature;
    Particion   particiones[4];
};

struct EBR {
    char    status = '0';
    char    fit = ' ';
    int     part_start = 0;
    int     size = 0;
    int     next = -1;
    char    name[16] = "NoName";

};

struct SuperBloque {
    int     filesystem_type = 0;
    int     inodes_count = 0;
    int     blocks_count = 0;
    int     free_blocks_count = 0;
    int     free_inodes_count = 0;
    time_t  mtime;
    time_t  umtime;
    int     mnt_count = 0;
    int     magic = 0;
    int     inode_size = 0;
    int     block_size = 0;
    int     first_ino = 0;
    int     first_blo = 0;
    int     bm_inode_start = 0;
    int     bm_block_start = 0;
    int     inode_start = 0;
    int     block_start = 0;
};

struct Inodo {
    int     uid;
    int     gid;
    int     size;
    time_t  atime;
    time_t  ctime;
    time_t  mtime;
    int     block[15];
    char    type;
    int     perm;
};

// BLOQUE DE CARPETAS
struct Content {
    char    name[12];
    int     inodo;
};

struct BloqueCarpetas {
    Content content[4];
};

// BLOQUE DE ARCHIVOS
struct BloqueArchivos {
    char content[64];
};


// BLOQUE DE APUNTADORES
struct BloqueApuntadores {
    int pointers[16];
};

#endif //VACASJUNIO_MIA_PROYECTO1_STRUCTS_H
