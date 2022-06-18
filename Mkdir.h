#ifndef VACASJUNIO_MIA_PROYECTO1_MKDIR_H
#define VACASJUNIO_MIA_PROYECTO1_MKDIR_H

#include <iostream>
#include <string>
#include <string.h>
#include "Carpeta.h"
#include "Structs.h"

using namespace std;

class Mkdir {
private:
    bool buscarCarpeta(string nombreParticion, string ruta, string nombre_carpeta);
    void crearBloqueCarpeta(string nombreParticion, string ruta, string nombre_carpeta, int inodo_padre);

public:
    Mkdir();

    void crearCarpeta(string nombreParticion, string ruta, string usuario, Carpeta *carpeta_nueva);
};


Mkdir::Mkdir(){}


void Mkdir::crearCarpeta(string nombreParticion, string ruta, string usuario, Carpeta *carpeta_nueva){

    FILE *archivo;

    archivo = fopen(ruta.c_str(), "rb+");

    if(archivo == NULL){
        cout << endl << " ***  El archivo no se ha encontrado. *** " << endl << endl;
    }

    int tam_particion = 0;
    int inicio_particion = 0;

    MBR mbr_;
    fseek(archivo, 0, SEEK_SET);
    fread(&mbr_, sizeof(MBR), 1, archivo);

    for(int i = 0; i < 4; i++){
        if(strcmp(mbr_.particiones[i].name, nombreParticion.c_str()) == 0){
            //cout << " >> Size: " << mbr_.mbr_particions[i].part_size << " \n";
            inicio_particion = mbr_.particiones[i].start;
            tam_particion = mbr_.particiones[i].size;
            break;
        }

    }

    // superbloque auxiliar
    SuperBloque sb_aux;
    fseek(archivo, inicio_particion, SEEK_SET);
    fread(&sb_aux, sizeof(SuperBloque), 1, archivo);

    // Declaro mis inodos y bloques usados
    //int inodos_usados = sb_aux.s_inodes_count - sb_aux.s_free_inodes_count;
    int bloques_usados = sb_aux.blocks_count - sb_aux.free_blocks_count;

    Inodo inodo_raiz;
    // posicionarme en el inodo raiz
    fseek(archivo, sb_aux.inode_start, SEEK_SET);
    fread(&inodo_raiz, sizeof(Inodo), 1, archivo);

    // buscar la carpeta
    bool existe = buscarCarpeta(nombreParticion, ruta, carpeta_nueva->getNombreCarpeta());

    if(existe == false){
        cout << " >> Crear la carpeta \n";
        for(int i = 0; i < 15; i++){

            if(inodo_raiz.block[i] == -1){
                // buscar bloque
                inodo_raiz.block[i] = bloques_usados + 1; // crear nuevo apuntador abajo
                // crearbloque
                crearBloqueCarpeta(nombreParticion, ruta, carpeta_nueva->getNombreCarpeta(), i);

                break;

            }
        }

    }else{
        cout << " >> La carpeta ya existe. \n";
    }

}

bool Mkdir::buscarCarpeta(string nombreParticion, string ruta, string nombre_carpeta){

    FILE *archivo;

    bool encontrado = false;
    archivo = fopen(ruta.c_str(), "rb+");

    int tam_particion = 0;
    int inicio_particion = 0;

    MBR mbr_;
    fseek(archivo, 0, SEEK_SET);
    fread(&mbr_, sizeof(MBR), 1, archivo);

    for(int i = 0; i < 4; i++){
        if(strcmp(mbr_.particiones[i].name, nombreParticion.c_str()) == 0){
            //cout << " >> Size: " << mbr_.mbr_particions[i].part_size << " \n";
            inicio_particion = mbr_.particiones[i].start;
            tam_particion = mbr_.particiones[i].size;
            break;
        }

    }

    // superbloque auxiliar
    SuperBloque sb_aux;
    fseek(archivo, inicio_particion, SEEK_SET);
    fread(&sb_aux, sizeof(SuperBloque), 1, archivo);

    // Declaro mis bloques usados
    int bloques_usados = sb_aux.blocks_count - sb_aux.free_blocks_count;

    BloqueCarpeta bl_carpeta_aux;

    fseek(archivo, sb_aux.block_start, SEEK_SET);
    for(int i = 0; i < bloques_usados; i++){
        fseek(archivo, sb_aux.block_start + (i*64), SEEK_SET);
        fread(&bl_carpeta_aux, 64, 1, archivo);
        for(int j = 0; j < 4; j++){
            cout << j << "\n";
            if(strcmp(bl_carpeta_aux.content[j].name, nombre_carpeta.c_str()) == 0){
                cout << " >> La carpeta ya existe \n";
                encontrado = true;
            }
        }
    }

    fclose(archivo);

    return encontrado;
}


void Mkdir::crearBloqueCarpeta(string nombreParticion, string ruta, string nombre_carpeta, int inodo_padre){

    FILE *archivo;

    bool encontrado = false;
    archivo = fopen(ruta.c_str(), "rb+");

    int tam_particion = 0;
    int inicio_particion = 0;

    MBR mbr_;
    fseek(archivo, 0, SEEK_SET);
    fread(&mbr_, sizeof(MBR), 1, archivo);

    for(int i = 0; i < 4; i++){
        if(strcmp(mbr_.particiones[i].name, nombreParticion.c_str()) == 0){
            //cout << " >> Size: " << mbr_.mbr_particions[i].part_size << " \n";
            inicio_particion = mbr_.particiones[i].start;
            tam_particion = mbr_.particiones[i].size;
            break;
        }

    }

    // superbloque auxiliar
    SuperBloque sb_aux;
    fseek(archivo, inicio_particion, SEEK_SET);
    fread(&sb_aux, sizeof(SuperBloque), 1, archivo);

    int inodos_usados = sb_aux.inodes_count - sb_aux.free_inodes_count;


    // bloque de carpetas nuevas
    BloqueCarpeta bl_nueva_carpeta;
    bl_nueva_carpeta.content[0].inodo = inodos_usados;
    strcpy(bl_nueva_carpeta.content[0].name, nombre_carpeta.c_str());
    bl_nueva_carpeta.content[1].inodo = -1;
    strcpy(bl_nueva_carpeta.content[1].name, " -- ");
    bl_nueva_carpeta.content[2].inodo = -1;
    strcpy(bl_nueva_carpeta.content[2].name, " -- ");
    bl_nueva_carpeta.content[3].inodo = -1;
    strcpy(bl_nueva_carpeta.content[3].name, " -- ");

    // crear un inodo para ese nuevo bloque

    // posicionarme en donde quiero escribir el nuevo bloque de carpeta
    int num_bloques_usados = sb_aux.blocks_count - sb_aux.free_blocks_count;
    fseek(archivo, sb_aux.block_start + (64 * num_bloques_usados), SEEK_SET);
    fwrite(&bl_nueva_carpeta, 64, 1, archivo);


    fseek(archivo, inicio_particion, SEEK_SET);
    sb_aux.free_blocks_count--;
    fwrite(&sb_aux, sizeof(SuperBloque), 1, archivo);

    cout << " >> Carpeta nueva creada \n";
    string directorio = "/" + nombre_carpeta;
    // Escribir el journal
    int inicio_journaling = inicio_particion + sizeof(SuperBloque);
    int num_journalings = 0;

    journaling aux;
    time_t tiempo = time(0);
    struct tm *tlocal = localtime(&tiempo);
    char fechaActual[16];
    strftime(fechaActual,16,"%d/%m/%y %H:%M:%S",tlocal);

    for(int i = 0; i < sb_aux.inodes_count; i++){
        fseek(archivo, inicio_journaling+ sizeof(Journaling)*i, SEEK_SET);
        fread(&aux, sizeof(Journaling), 1, archivo);
        if(aux.estado == 1){
            num_journalings++;
        }

    }
    if(sb_aux.filesystem_type == 3){
        journaling journalArchivo;
        journalArchivo.estado = 1;
        strcpy(journalArchivo.tipo_op, "mkdir");
        strcpy(journalArchivo.fecha_op, fechaActual);
        strcpy(journalArchivo.path, directorio.c_str());
        journalArchivo.id_propietario = '1';
        strcpy(journalArchivo.contenido, " -- ");
        journalArchivo.tipo = '0';

        fseek(archivo, inicio_journaling + sizeof(Journaling)*num_journalings, SEEK_SET);
        fwrite(&journalArchivo, sizeof(Journaling), 1, archivo);
        cout << " >> Journaling escrito \n";


    }

    // modificar el bitmap de bloques
    char bloques[sb_aux.blocks_count];
    //fseek(archivo, sb_aux.s_bm_block_start, SEEK_SET);
    for(int i = 0; i < sb_aux.blocks_count; i++){

        fseek(archivo, sb_aux.bm_block_start+ i*sizeof(char), SEEK_SET);
        if(bloques[i] == '0'){

            bloques[i] = '1';
            fwrite(&bloques, sizeof(char), 1, archivo);
            break;
        }

    }


    fclose(archivo);


}

#endif //VACASJUNIO_MIA_PROYECTO1_MKDIR_H
