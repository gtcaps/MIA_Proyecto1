//
// Created by gtcaps on 16/06/22.
//

#ifndef VACASJUNIO_MIA_PROYECTO1_MKFILE_H
#define VACASJUNIO_MIA_PROYECTO1_MKFILE_H

#include <iostream>
#include <vector>
#include <sstream>
#include "Mount.h"
#include "Mkfs.h"
#include "Carpeta.h"
#include "Mkdir.h"

using namespace std;

class Mkfile {

public:
    Mkfile();
    bool crearArchivo(string id, string path, string p, int size, string cont, Mount montaje);

};

Mkfile::Mkfile() {}

bool Mkfile::crearArchivo(string id, string path, string p, int size, string cont, Mount montaje) {

    string pathD= "";
    int sizeParticion;
    int startParticion;
    int error = 0;
    string nombreParticion;
    int numSize = 0;

    Mkfs mkfs;
    mkfs.getDatosID(id, montaje, &pathD, &startParticion, &sizeParticion, &nombreParticion, &error);

    if(error == 1){
        return false;
    }

    // -----

    FILE *archivo;

    archivo = fopen(pathD.c_str(), "rb+");

    if(archivo == NULL){
        cout << " >> El archivo no se ha encontrado. \n";
    }

    int tam_particion = 0;
    int inicio_particion = 0;

    MBR mbr_;
    fseek(archivo, 0, SEEK_SET);
    fread(&mbr_, sizeof(MBR), 1, archivo);

    for(int i = 0; i < 4; i++){
        if(strcmp(mbr_.particiones[i].name, nombreParticion.c_str()) == 0){
            cout << " >> Size: " << mbr_.particiones[i].size << " \n";
            inicio_particion = mbr_.particiones[i].start;
            tam_particion = mbr_.particiones[i].size;
            break;
        }

    }

    // superbloque auxiliar
    SuperBloque sb_aux;
    fseek(archivo, inicio_particion, SEEK_SET);
    fread(&sb_aux, sizeof(SuperBloque), 1, archivo);
    cout << " >> Inodo  "<< sb_aux.inodes_count << "\n";
    cout << " >> Bloque "<< sb_aux.blocks_count << "\n";
    cout << " >> Inodos libres "<< sb_aux.free_inodes_count << "\n";
    cout << " >> Bloques libres "<< sb_aux.free_blocks_count << "\n";

    // Declaro mis inodos y bloques usados
    int inodos_usados = sb_aux.inodes_count - sb_aux.free_inodes_count;
    int bloques_usados = sb_aux.blocks_count - sb_aux.free_blocks_count;


    cout <<" >> "<< sb_aux.inode_start << endl;

    Inodo inodo_aux;


    cout << " >> Ruta de archivo a crear: " << path << "\n";

    string str = path;
    string delimitador = "/";
    size_t pos = 0;
    string subdir;

    // Ver si tiene carpetas que la contienen
    while ((pos = str.find(delimitador)) != std::string::npos) {
        subdir = str.substr(0, pos);

        if(!subdir.empty()){
            //cout << " >> Carpeta: " <<subdir << endl;
            Carpeta *nueva_carpeta = new Carpeta();
            nueva_carpeta->setNombreCarpeta(subdir);

            // Crear nueva carpeta con un mkdir
            Mkdir mkdir;
            mkdir.crearCarpeta(nombreParticion, path, "root", nueva_carpeta);
        }

        str.erase(0, pos + delimitador.length());
    }

    cout << " >> Archivo a crear " << str << endl;
    // intentar recorrer un inodo
    for(int i = 0; i < inodos_usados; i++){

        fseek(archivo, sb_aux.inode_start + i*(sizeof(Inodo)), SEEK_SET);
        fread(&inodo_aux, sizeof(Inodo), 1, archivo);
        cout << inodo_aux.gid << "\n";

    }


    // intentar recorrer un bloque
    BloqueArchivo bl_archivo_aux;
    for(int i = 0; i < bloques_usados; i++){
        fseek(archivo, sb_aux.block_start + i*64, SEEK_SET);
        fread(&bl_archivo_aux, 64, 1, archivo);

        cout << bl_archivo_aux.content << "\n";

    }

    fclose(archivo);



}


#endif //VACASJUNIO_MIA_PROYECTO1_MKFILE_H
