#ifndef VACASJUNIO_MIA_PROYECTO1_MKFS_H
#define VACASJUNIO_MIA_PROYECTO1_MKFS_H

#include "Mount.h"
#include "Structs.h"
#include <cstring>

using namespace std;

class Mkfs {
public:
    Mkfs();
    bool formatearFS(string id, string type, int add, string unit, Mount montaje);
    void getDatosID(string id, Mount montaje, string * path, int *inicioParticion, int * sizePart, string * nombrePart, int * error);
    string getFecha();
};

Mkfs::Mkfs() {}

void Mkfs::getDatosID(string id, Mount montaje, string * path, int *inicioParticion, int * sizePart, string *nombrePart, int * error){
    //Obtener particion y su size.
    char letra = id.c_str()[2];
    char numero = id.c_str()[3];
    bool existePath = false;
    int numeroInt = (int)numero - 48;
    string nombreParticion = "";

    char pathDisco[100] = "";

    for (int i = 0; i < 26; i++) {
        if (montaje.discos[i].letra == letra && montaje.discos[i].particiones[numeroInt-1].estado == 1) {
            strcpy(pathDisco, montaje.discos[i].path);
            nombreParticion = montaje.discos[i].particiones[numeroInt-1].name;
            existePath = true;
            break;
        }
    }

    if (!existePath) {
        cout << endl << " *** Error: id no existe, la particion debe estar montada***" << endl << endl;
        *error = 1;
        return;
    }

    int sizeParticion = -1;
    int startParticion = -1;

    FILE *bfile1 = fopen(pathDisco, "rb+");
    if(bfile1 != NULL){
        rewind(bfile1);
        MBR discoLeer;
        fread(&discoLeer, sizeof(MBR), 1, bfile1);
        for(int i = 0; i < 4; i++) {
            if(discoLeer.particiones[i].name == nombreParticion){
                sizeParticion = discoLeer.particiones[i].size;
                startParticion = discoLeer.particiones[i].start;
            }
        }

        fclose(bfile1);
    }else{
        cout << endl << " *** Error: Path no existente *** " << endl << endl;
        *error = 1;
        return;
    }

    *nombrePart = string(nombreParticion);
    *error = 0;
    *path = string(pathDisco);
    *inicioParticion = startParticion;
    *sizePart = sizeParticion;
}

bool Mkfs::formatearFS(string id, string type, int add, string unit, Mount montaje) {
    if (add != 0) {
        if (unit == "k") {
            add = add * 1024;
        } else if (unit == "m") {
            add = add * 1024 * 1024;
        } else if (unit == "b") {
            add = add;
        } else {
            cout << endl << " *** Unit debe de ser k, b, m *** " << endl << endl;
            return false;
        }

        // Agregar codigo para agregar a la particion
        //
        //
        //
    } else {

        // Obtener Datos
        int nEstructuras = -1;
        string path = "";
        int sizeParticion;
        int startParticion;
        int error = 0;
        string nombreParticion;

        getDatosID(id, montaje, &path, &startParticion, &sizeParticion, &nombreParticion, &error);

        if(error == 1){
            return false;
        }

        // OBTENER EL DISCO
        FILE *archivo;
        archivo = fopen(path.c_str(), "rb+");

        if(archivo == NULL){
            cout << endl << " *** El disco no existe *** " << endl << endl;
        }

        // Tamanio de la particion
        int tam_particion = 0;
        int inicio_particion = 0;

        MBR mbr;
        fseek(archivo, 0, SEEK_SET);
        fread(&mbr, sizeof(mbr), 1, archivo);

        for(int i = 0; i < 4; i++){
            if(strcmp(mbr.particiones[i].name, nombreParticion.c_str()) == 0){
                cout << " >> Size: " << mbr.particiones[i].size << " \n";
                inicio_particion = mbr.particiones[i].start;
                tam_particion = mbr.particiones[i].size;
                break;
            }
        }

        // Valores para aplicar formula
        int n = 0;
        cout << " >> Formateando " << tam_particion << " bytes...\n";
        // APLICAR FORMULA
        n = (tam_particion - sizeof(SuperBloque)) / (4 + sizeof(Journaling) + sizeof(Inodo) + 3 * 64);

        cout << " n: " << n << endl;
        cout << " 3n: " << 3*n <<  endl;

        fseek(archivo, inicio_particion, SEEK_SET);
        if(type == "full"){

            char bytenulo = '0';
            // llenar de ceros la sección
            for(int i = 0; i < tam_particion; i++){

                fwrite(&bytenulo, sizeof(bytenulo), 1, archivo);
                fseek(archivo, inicio_particion + i, SEEK_SET);
            }
        }

        int numeroEstructuras = n;
        int numeroInodos = n;
        int numeroJournaling = n;
        int numeroBloques = 3*n;
        int bitmapInodos = n;
        int bitmapBloques = 3*n;


        time_t tiempo = time(0);
        struct tm *tlocal = localtime(&tiempo);
        char fechaActual[20];
        strftime(fechaActual,16,"%d/%m/%y %H:%M:%S",tlocal);

        SuperBloque super_b;
        super_b.filesystem_type = 3;
        super_b.inodes_count = numeroInodos;
        super_b.blocks_count = numeroBloques;
        super_b.free_blocks_count = 3*n;
        super_b.free_inodes_count = numeroInodos;
        strcpy(super_b.mtime, fechaActual);
        strcpy(super_b.umtime, "--");
        super_b.umtime[0] = '0';
        super_b.mnt_count = 1;
        super_b.magic = 0xEF53;
        super_b.inode_size = sizeof(Inodo);
        super_b.block_size = sizeof(BloqueArchivo);
        super_b.first_ino = 2;
        super_b.first_blo = 2;
        super_b.bm_inode_start = inicio_particion + sizeof(SuperBloque) + n*sizeof(Journaling);
        super_b.bm_block_start = super_b.bm_inode_start + n;
        super_b.block_start = super_b.bm_inode_start+n*sizeof(Inodo);
        super_b.inode_start = super_b.block_start + 3 * n;

        Journaling journal;
        journal.estado = 0;
        journal.fecha_op[0] ='\0';
        journal.path[0] = '\0';
        journal.tipo = '-1';
        strcpy(journal.contenido, " -- ");
        journal.tamanio = 0;
        journal.tipo_op[0] = '\0';

        fseek(archivo, inicio_particion, SEEK_SET);

        // escribiendo el superbloque al inicio de la particion
        fwrite(&super_b, sizeof(SuperBloque), 1, archivo);

        // posicionarme en el lugar donde quiero escribir el journaling
        cout << " 1. " << inicio_particion + sizeof(SuperBloque) << "\n";
        fseek(archivo, inicio_particion +sizeof(SuperBloque), SEEK_SET);

        // Escribir el journal de la carpeta
        Journaling journalCarpeta;
        journalCarpeta.estado = 1;
        strcpy(journalCarpeta.tipo_op, "mkfs");
        strcpy(journalCarpeta.fecha_op, fechaActual);
        strcpy(journalCarpeta.path, "/");
        journalCarpeta.id_propietario = '1';
        strcpy(journalCarpeta.contenido, "-");
        journalCarpeta.tipo = '0';

        int inicio_journaling = inicio_particion + sizeof(SuperBloque);
        //fseek(archivo, inicio_journaling, SEEK_SET);
        //fwrite(&journalCarpeta, sizeof(journaling), 1, archivo);

        // Escribir el journal del archivo
        Journaling journalArchivo;
        journalArchivo.estado = 1;
        strcpy(journalArchivo.tipo_op, "mkfs");
        strcpy(journalArchivo.fecha_op, fechaActual);
        strcpy(journalArchivo.path, "/users.txt");
        journalArchivo.id_propietario = '1';
        strcpy(journalArchivo.contenido, "1,G,root\n1,U,root,root,123\n");
        journalArchivo.tipo = '1';

        //fseek(archivo, inicio_journaling + sizeof(journaling), SEEK_SET);
        //fwrite(&journalArchivo, sizeof(journaling), 1, archivo);

        // Escribir el journaling
        for(int i = 0; i < numeroJournaling; i++){
            //fseek(archivo,inicio_journaling + i*sizeof(journaling) , SEEK_SET);
            if(i == 0){
                fseek(archivo,inicio_journaling, SEEK_SET);
                fwrite(&journalCarpeta, sizeof(Journaling), 1, archivo);
            } else if (i == 1){
                fseek(archivo,inicio_journaling + sizeof(Journaling) , SEEK_SET);
                fwrite(&journalArchivo, sizeof(Journaling), 1, archivo);
            }else if (i > 1){
                fseek(archivo,inicio_journaling + i*sizeof(Journaling) , SEEK_SET);
                fwrite(&journal, sizeof(Journaling), 1, archivo);
            }
        }

        // posicionarme en el lugar donde quiero escribir el bitmap de inodos
        cout << " 2. " <<inicio_particion + sizeof(SuperBloque)+numeroJournaling*(sizeof(Journaling)) << "\n";


        char inodos[n];
        char bloques[3*n];

        // lleno los inodos
        for(int i = 0; i < n; i++ ){
            inodos[i] = '0';
        }

        inodos[0] = '1';
        inodos[1] = '1';
        fseek(archivo, inicio_particion + sizeof(SuperBloque)+numeroJournaling*(sizeof(Journaling)), SEEK_SET);
        fwrite(&inodos, sizeof(inodos), 1, archivo);


        for(int i = 0; i < 3*n; i++){
            bloques[i] = '0';
        }

        bloques[0] = '1';
        bloques[1] = '1';

        cout << " 3. " << inicio_particion + sizeof(SuperBloque)+numeroJournaling*sizeof(Journaling)+bitmapInodos*sizeof(char) << "\n";
        fseek(archivo, inicio_particion + sizeof(SuperBloque)+numeroJournaling*sizeof(Journaling)+bitmapInodos*sizeof(char), SEEK_SET);
        //fseek(archivo, super_b.s_bm_block_start, SEEK_SET);
        fwrite(&bloques, sizeof(bloques), 1, archivo);

        // superbloque auxiliar
        SuperBloque sb_aux;
        fseek(archivo, inicio_particion, SEEK_SET);
        fread(&sb_aux, sizeof(SuperBloque), 1, archivo);

        // TABLA DE INODOS RAIZ
        Inodo tInodo;
        tInodo.uid = 1;
        tInodo.gid = 1;
        tInodo.size = 0;
        strcpy(tInodo.ctime, fechaActual);
        strcpy(tInodo.atime, fechaActual);
        strcpy(tInodo.mtime, fechaActual);
        for(int i = 0; i < 15; i++){
            tInodo.block[i] = -1;
            if(i == 14){
                tInodo.block[i] = -1;
            }
        }
        tInodo.type = '0';
        tInodo.perm = 664; // Guarda los permisos del archivo / carpeta
        tInodo.block[0] = 0;

        // ARREGLO DE BLOQUES

        BloqueCarpeta bl_carpeta;
        Content bl_content;
        bl_content.name[0] = '-';
        bl_content.inodo = 0;

        for(int i = 0; i < 4; i++){
            bl_carpeta.content[i] = bl_content;
        }

        // ESTRUCTURA DE LOS ARCHIVOS QUE VAN EN LA RAIZ
        BloqueCarpeta bloque_root;
        string nombre_archivo = "users.txt";
        string un_punto = ".";
        string dos_puntos = "..";
        string dir_raiz = "/";
        // primera posicion
        bloque_root.content[0].inodo = 0;
        strcpy(bloque_root.content[0].name, un_punto.c_str());
        // segunda posicion
        bloque_root.content[1].inodo = 0;
        strcpy(bloque_root.content[1].name, dos_puntos.c_str());
        // tercera posicion
        bloque_root.content[2].inodo = 1;
        strcpy(bloque_root.content[2].name, nombre_archivo.c_str());
        // cuarta posicion
        bloque_root.content[3].inodo = -1;
        strcpy(bloque_root.content[3].name, dir_raiz.c_str());

        fseek(archivo, sb_aux.inode_start, SEEK_SET);
        fwrite(&tInodo, sizeof(Inodo), 1, archivo); // escribiendo inodo raiz

        fseek(archivo, sb_aux.block_start, SEEK_SET);
        fwrite(&bloque_root, 64,1, archivo);

        // posicionarme al inicio de la particion
        fseek(archivo, inicio_particion, SEEK_SET);
        // restar la cantidad de bloques e inodos libres
        //sb_aux.s_free_blocks_count--;
        sb_aux.free_inodes_count--;
        fwrite(&sb_aux, sizeof(SuperBloque), 1, archivo);

        //
        string contenido = "1,G,root\n1,U,root,root,123\n";
        Inodo archivo_usuarios;// inodo del archivo de usuarios
        archivo_usuarios.gid = 1;
        archivo_usuarios.uid = 1;
        archivo_usuarios.size = sizeof(contenido);
        strcpy(archivo_usuarios.atime, fechaActual);
        strcpy(archivo_usuarios.ctime, fechaActual);
        strcpy(archivo_usuarios.mtime, fechaActual);

        for(int i = 0; i < 15; i++){
            archivo_usuarios.block[i] = -1;
        }

        archivo_usuarios.perm = 664;
        archivo_usuarios.block[0] = 1;
        archivo_usuarios.type = '1';// inodo de tipo archivo


        BloqueArchivo bl_archivo;
        strcpy(bl_archivo.content, contenido.c_str());

        fseek(archivo, sb_aux.bm_inode_start+sizeof(Inodo), SEEK_SET);
        fwrite(&archivo_usuarios,sizeof(Inodo),1, archivo);

        fseek(archivo, sb_aux.block_start + 64, SEEK_SET);
        fwrite(&bl_archivo, 64, 1, archivo);

        fseek(archivo, inicio_particion, SEEK_SET);
        // restar la cantidad de bloques e inodos libres
        sb_aux.free_inodes_count--;
        sb_aux.free_blocks_count--; // resto el bloque de carpeta
        sb_aux.free_blocks_count--; // resto el bloque de archivos
        fwrite(&sb_aux, sizeof(SuperBloque), 1, archivo);

        cout << endl << " >> Particion formateada en EXT3. \n";
        fclose(archivo);
    }

    return true;
}

#endif //VACASJUNIO_MIA_PROYECTO1_MKFS_H
