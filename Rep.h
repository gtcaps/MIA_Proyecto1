#ifndef VACASJUNIO_MIA_PROYECTO1_REP_H
#define VACASJUNIO_MIA_PROYECTO1_REP_H

#include <iostream>
#include <string>
#include <cstring>
#include <fstream>
#include "Mount.h"
#include "Structs.h"

using namespace std;

class Rep {
public:
    Rep();
    bool crearReporte(string path, string name, string id, Mount montaje);
    void reporteDisco(MBR discoEditar, string pathImagen, string pathDisco);
    void reporteMBR(MBR discoEditar, string pathImagen, string pathDisco);
    void reporteSB(string nombreParticion, string pathImagen, string pathDisco);
    void reporteBm_Inode(string nombreParticion, string pathImagen, string pathDisco);
    void reporteBm_Block(string nombreParticion, string pathImagen, string pathDisco);
    void reporteJournaling(string nombreParticion, string pathImagen, string pathDisco);
    void reporteInodos(string nombreParticion, string pathImagen, string pathDisco);
    void reporteBloques(string nombreParticion, string pathImagen, string pathDisco);
    void getDatosID(string id, Mount montaje, string * path, int *inicioParticion, int * sizePart, string * nombrePart, int * error);
};

Rep::Rep() {}

void Rep::getDatosID(string id, Mount montaje, string * path, int *inicioParticion, int * sizePart, string *nombrePart, int * error){
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
        cout << endl << " *** Error: id no existe, path no existente ***" << endl << endl;
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


bool Rep::crearReporte(string path, string name, string id, Mount montaje) {


    string pathD= "";
    int sizeParticion;
    int startParticion;
    int error = 0;
    string nombreParticion;

    getDatosID(id, montaje, &pathD, &startParticion, &sizeParticion, &nombreParticion, &error);


    if(error == 1){
        return false;
    }

    MBR discoEditar;
    FILE *bfile2 = fopen(pathD.c_str(), "rb+");
    if (bfile2 != NULL) {
        rewind(bfile2);
        fread(&discoEditar, sizeof(discoEditar), 1, bfile2);
    } else {
        cout << endl <<  " *** Error. Path no existente, disco no existente *** " << endl << endl;
        return false;
    }
    fclose(bfile2);

    if (name == "mbr") {
        reporteMBR(discoEditar, path, pathD);
    } else if (name == "disk") {
        reporteDisco(discoEditar, path, pathD);
    } else if (name == "sb") {
        reporteSB(nombreParticion, path, pathD);
    } else if (name == "bm_inode") {
        reporteBm_Inode(nombreParticion, path, pathD);
    } else if (name == "bm_block") {
        reporteBm_Block(nombreParticion, path, pathD);
    } else if (name == "inode") {
        reporteInodos(nombreParticion, path, pathD);
    } else if (name == "journaling") {
        reporteJournaling(nombreParticion, path, pathD);
    } else if (name == "block") {
        reporteBloques(nombreParticion, path, pathD);
    } else {
        cout << endl << " *** No existe el reporte que buscas *** " << endl << endl;
    }

    return true;
}

void Rep::reporteBloques(string nombreParticion, string pathImagen, string pathDisco) {
    FILE *archivo;

    archivo = fopen(pathDisco.c_str(), "rb+");

    if(archivo == NULL){
        cout << " >> El disco no existe. \n";
    }


    int inicio_particion = 0;

    MBR mbr_;
    fseek(archivo, 0, SEEK_SET);
    fread(&mbr_, sizeof(MBR), 1, archivo);

    for(int i = 0; i < 4; i++){
        if(strcmp(mbr_.particiones[i].name, nombreParticion.c_str()) == 0){
            //cout << " >> Size: " << mbr_.mbr_particions[i].part_size << " \n";
            inicio_particion = mbr_.particiones[i].start;
            //tam_particion = mbr_.mbr_particions[i].part_size;
            break;
        }

    }

    // superbloque auxiliar
    SuperBloque sb_aux;
    fseek(archivo, inicio_particion, SEEK_SET);
    fread(&sb_aux, sizeof(SuperBloque), 1, archivo);
    int inicio_bloques = sb_aux.block_start;
    int bloques_usados = sb_aux.blocks_count - sb_aux.free_blocks_count;

    string pathDot = pathImagen.substr(0, pathImagen.size() - 4) + ".dot";
    size_t lastSlash = pathImagen.find_last_of('/');
    string folderPathOfImage = pathImagen.substr(0, lastSlash);
    string imageExtension = pathImagen.substr(pathImagen.size() - 3);

    ofstream reporte;
    reporte.open(pathDot, ios::out);

    if (reporte.fail()) {
        string comando1 = "mkdir -p \"" + folderPathOfImage + "\"";
        system(comando1.c_str());
    }
    reporte.close();

    reporte.open(pathDot);

    reporte << "digraph Journaling { \n"
            << "node [shape=plaintext] \n";

    BloqueCarpeta bl_carpeta_aux;

    for(int i = 0; i < bloques_usados; i++){

        fseek(archivo, inicio_bloques + i*64, SEEK_SET);
        fread(&bl_carpeta_aux, 64, 1, archivo);
        reporte << "nodo"<< i <<"[\n";

        reporte << "\t label =< \n"
                << "\t\t <table border=\"0\" cellborder=\"1\" cellspacing=\"0\"> \n"
                << "\t\t\t <tr> \n"
                << "\t\t\t\t <td bgcolor=\"#30A6BB\">No. Bloque </td> <td bgcolor=\"#30A6BB\"> "<< i <<"  </td> \n"
                << "\t\t\t </tr> \n";

        reporte << "\t\t\t <tr> \n"
                << "\t\t\t\t <td bgcolor=\"#C7F6FF\"> b_name </td> <td bgcolor=\"#C7F6FF\"> b_inodo </td> \n"
                << "\t\t\t </tr> \n";

        for(int j = 0; j < 4; j++){
            reporte << "\t\t\t <tr> \n"
                    << "\t\t\t\t <td bgcolor=\"#C7F6FF\"> "<< bl_carpeta_aux.content[j].inodo << " </td> <td bgcolor=\"#C7F6FF\"> "<< bl_carpeta_aux.content[j].name <<" </td> \n"
                    << "\t\t\t </tr> \n";
        }

        reporte << "\t\t </table> \n"
                << "\t > \n"
                << "]; \n";

    }

    BloqueArchivo bl_archivo_aux;

    for(int i = 0; i < bloques_usados; i++){

        fseek(archivo, inicio_bloques + i*64, SEEK_SET);
        fread(&bl_archivo_aux, 64, 1, archivo);
        reporte << "nodoaux"<< i <<"[\n";

        reporte << "\t label =< \n"
                << "\t\t <table border=\"0\" cellborder=\"1\" cellspacing=\"0\"> \n"
                << "\t\t\t <tr> \n"
                << "\t\t\t\t <td bgcolor=\"#30A6BB\">No. Bloque </td> <td bgcolor=\"#30A6BB\"> "<< i <<"  </td> \n"
                << "\t\t\t </tr> \n";

        reporte << "\t\t\t <tr> \n"
                << "\t\t\t\t <td bgcolor=\"#C7F6FF\"> content </td> <td bgcolor=\"#C7F6FF\"> " << bl_archivo_aux.content <<" </td> \n"
                << "\t\t\t </tr> \n";


        reporte << "\t\t </table> \n"
                << "\t > \n"
                << "]; \n";

    }



    reporte << "}\n";

    reporte.close();
    fclose(archivo);

    string comando2 = "dot -T" + imageExtension + " \"" + pathDot + "\" -o \"" + pathImagen + "\"";
    system(comando2.c_str());

    string comandoOpen = "xdg-open \"" + pathImagen + "\"";
    system(comandoOpen.c_str());

    cout << endl << " *** Correcto: Se genero el reporte tipo MBR en - " << pathDot << endl << endl;

}

void Rep::reporteInodos(string nombreParticion, string pathImagen, string pathDisco) {
    FILE *archivo;

    archivo = fopen(pathDisco.c_str(), "rb+");

    if(archivo == NULL){
        cout << " >> El disco no existe. \n";
    }

    int inicio_particion = 0;

    MBR mbr_;
    fseek(archivo, 0, SEEK_SET);
    fread(&mbr_, sizeof(MBR), 1, archivo);

    for(int i = 0; i < 4; i++){
        if(strcmp(mbr_.particiones[i].name, nombreParticion.c_str()) == 0){
            //cout << " >> Size: " << mbr_.mbr_particions[i].part_size << " \n";
            inicio_particion = mbr_.particiones[i].start;
            //tam_particion = mbr_.mbr_particions[i].part_size;
            break;
        }

    }

    // superbloque auxiliar
    SuperBloque sb_aux;
    fseek(archivo, inicio_particion, SEEK_SET);
    fread(&sb_aux, sizeof(SuperBloque), 1, archivo);
    int inicio_inodos = sb_aux.inode_start;

    // Escribir el reporte del superbloque
    string pathDot = pathImagen.substr(0, pathImagen.size() - 4) + ".dot";
    size_t lastSlash = pathImagen.find_last_of('/');
    string folderPathOfImage = pathImagen.substr(0, lastSlash);
    string imageExtension = pathImagen.substr(pathImagen.size() - 3);

    ofstream reporte;
    reporte.open(pathDot, ios::out);

    if (reporte.fail()) {
        string comando1 = "mkdir -p \"" + folderPathOfImage + "\"";
        system(comando1.c_str());
    }
    reporte.close();

    reporte.open(pathDot);


    Inodo inodo_aux;
    int inodos_ocupados = sb_aux.inodes_count - sb_aux.free_inodes_count;

    reporte << "digraph Inode {\n"
            << "node [shape=plaintext] \n";




    for(int i = 0; i < inodos_ocupados; i++){
        fseek(archivo, inicio_inodos, SEEK_SET);
        fread(&inodo_aux, sizeof(Inodo), 1, archivo);
        reporte << "nodo"<< i <<" [\n"
                << "label =< \n";

        reporte << "<table border=\"0\" cellborder=\"1\" cellspacing=\"0\">"
                << "<tr> <td bgcolor=\"#30A6BB\">Nombre</td> <td bgcolor=\"#30A6BB\"> Valor </td> </tr>";

        reporte << "<tr>\n";
        reporte << "<td bgcolor=\"#89D8E7\"> i_uid </td>";
        reporte << "<td bgcolor=\"#C7F6FF\">" << inodo_aux.uid << " </td> \n";
        reporte << "</tr>";

        reporte << "<tr>\n";
        reporte << "<td bgcolor=\"#89D8E7\"> i_gid </td>";
        reporte << "<td bgcolor=\"#C7F6FF\">" << inodo_aux.gid << " </td> \n";
        reporte << "</tr>";

        reporte << "<tr>\n";
        reporte << "<td bgcolor=\"#89D8E7\"> i_atime </td>";
        reporte << "<td bgcolor=\"#C7F6FF\">" << inodo_aux.atime << " </td> \n";
        reporte << "</tr>";


        for(int j = 0; j < 15; j++){

            if(inodo_aux.block[j] != -1){
                reporte << "<tr>\n";
                reporte << "<td bgcolor=\"#89D8E7\"> i_block_" << j << " </td>";
                reporte << "<td bgcolor=\"#C7F6FF\">" << inodo_aux.block[j] << " </td> \n";
                reporte << "</tr>";
            }
        }

        reporte << "<tr>\n";
        reporte << "<td bgcolor=\"#89D8E7\"> i_perm </td>";
        reporte << "<td bgcolor=\"#C7F6FF\">" << inodo_aux.perm << " </td> \n";
        reporte << "</tr>";

        reporte << "<tr>\n";
        reporte << "<td bgcolor=\"#89D8E7\"> i_type </td>";
        reporte << "<td bgcolor=\"#C7F6FF\">" << inodo_aux.type << " </td> \n";
        reporte << "</tr>";

        reporte << "</table>\n";
        reporte << ">\n";
        reporte << "];\n";

    }

    reporte << "}";

    reporte.close();
    fclose(archivo);

    string comando2 = "dot -T" + imageExtension + " \"" + pathDot + "\" -o \"" + pathImagen + "\"";
    system(comando2.c_str());

    string comandoOpen = "xdg-open \"" + pathImagen + "\"";
    system(comandoOpen.c_str());

    cout << endl << " *** Correcto: Se genero el reporte tipo MBR en - " << pathDot << endl << endl;
}

void Rep::reporteJournaling(string nombreParticion, string pathImagen, string pathDisco) {

    FILE *archivo;

    archivo = fopen(pathDisco.c_str(), "rb+");

    if(archivo == NULL){
        cout << " >> El disco no existe. \n";
    }

    int inicio_particion = 0;

    MBR mbr_;
    fseek(archivo, 0, SEEK_SET);
    fread(&mbr_, sizeof(MBR), 1, archivo);

    for(int i = 0; i < 4; i++){
        if(strcmp(mbr_.particiones[i].name, nombreParticion.c_str()) == 0){
            //cout << " >> Size: " << mbr_.mbr_particions[i].part_size << " \n";
            inicio_particion = mbr_.particiones[i].start;
            //tam_particion = mbr_.mbr_particions[i].part_size;
            break;
        }

    }

    // superbloque auxiliar
    SuperBloque sb_aux;
    fseek(archivo, inicio_particion, SEEK_SET);
    fread(&sb_aux, sizeof(SuperBloque), 1, archivo);
    int num_journaling = sb_aux.inodes_count;

    // Escribir el reporte del superbloque
    string pathDot = pathImagen.substr(0, pathImagen.size() - 4) + ".dot";
    size_t lastSlash = pathImagen.find_last_of('/');
    string folderPathOfImage = pathImagen.substr(0, lastSlash);
    string imageExtension = pathImagen.substr(pathImagen.size() - 3);

    ofstream reporte;
    reporte.open(pathDot, ios::out);

    if (reporte.fail()) {
        string comando1 = "mkdir -p \"" + folderPathOfImage + "\"";
        system(comando1.c_str());
    }
    reporte.close();

    reporte.open(pathDot);

    int inicio_journaling = inicio_particion + sizeof(SuperBloque);

    Journaling journaling_aux;
    fseek(archivo, inicio_journaling + sizeof(Journaling), SEEK_SET);
    fread(&journaling_aux, sizeof(Journaling), 1, archivo);

    reporte << "digraph Journaling { \n"
            << "node [shape=plaintext] \n";

    for(int i = 0; i < sb_aux.inodes_count; i++){

        fseek(archivo, inicio_journaling + sizeof(Journaling)*i, SEEK_SET);

        if(journaling_aux.estado == 1){
            fread(&journaling_aux, sizeof(Journaling), 1 , archivo);
            cout << " Esta activo \n";

            reporte << "nodo"<< i <<"[\n";
            reporte << "\t label =< \n"
                    << "\t\t <table border=\"0\" cellborder=\"1\" cellspacing=\"0\"> \n"
                    << "\t\t\t <tr> \n"
                    << "\t\t\t\t <td bgcolor=\"#30A6BB\">No. </td> <td bgcolor=\"#30A6BB\"> Tipo operacion </td> \n"
                    << "\t\t\t\t <td bgcolor=\"#30A6BB\">Fecha operacion </td> <td bgcolor=\"#30A6BB\"> path </td> \n"
                    << "\t\t\t\t <td bgcolor=\"#30A6BB\">Tamanio </td> <td bgcolor=\"#30A6BB\"> Id propiertario </td> \n"
                    << "\t\t\t\t <td bgcolor=\"#30A6BB\">Contenido </td> <td bgcolor=\"#30A6BB\"> Tipo </td> \n"
                    << "\t\t\t </tr> \n";

            reporte << "\t\t\t <tr> \n"
                    << "\t\t\t\t <td bgcolor=\"#C7F6FF\"> " << i + 1 << "</td> <td bgcolor=\"#C7F6FF\"> " << journaling_aux.tipo <<" </td> \n"
                    << "\t\t\t\t <td bgcolor=\"#C7F6FF\"> " << journaling_aux.fecha_op << "</td> <td bgcolor=\"#C7F6FF\">" << journaling_aux.path <<" </td> \n"
                    << "\t\t\t\t <td bgcolor=\"#C7F6FF\"> " << journaling_aux.tamanio << "</td> <td bgcolor=\"#C7F6FF\">" << journaling_aux.id_propietario <<" </td> \n"
                    << "\t\t\t\t <td bgcolor=\"#C7F6FF\"> " << journaling_aux.contenido << "</td> <td bgcolor=\"#C7F6FF\">" << journaling_aux.tipo_op <<" </td> \n"
                    << "\t\t\t </tr> \n"
                    << "\t\t </table> \n"
                    << "\t > \n"
                    << "]; \n";

            // enlazando invisiblemente las tablas
            reporte << "nodo" << i << "->nodo" << i+1 << "[style=invis]\n";

        }
    }

    reporte << "}\n";

    reporte.close();
    fclose(archivo);

    string comando2 = "dot -T" + imageExtension + " \"" + pathDot + "\" -o \"" + pathImagen + "\"";
    system(comando2.c_str());

    string comandoOpen = "xdg-open \"" + pathImagen + "\"";
    system(comandoOpen.c_str());

    cout << endl << " *** Correcto: Se genero el reporte tipo MBR en - " << pathDot << endl << endl;

}

void Rep::reporteBm_Block(string nombreParticion, string pathImagen, string pathDisco) {
    FILE *archivo;

    archivo = fopen(pathDisco.c_str(), "rb+");

    if(archivo == NULL){
        cout << " >> El disco no existe. \n";
    }

    int inicio_particion = 0;

    MBR mbr_;
    fseek(archivo, 0, SEEK_SET);
    fread(&mbr_, sizeof(MBR), 1, archivo);

    for(int i = 0; i < 4; i++){
        if(strcmp(mbr_.particiones[i].name, nombreParticion.c_str()) == 0){
            //cout << " >> Size: " << mbr_.mbr_particions[i].part_size << " \n";
            inicio_particion = mbr_.particiones[i].start;
            //tam_particion = mbr_.mbr_particions[i].part_size;
            break;
        }

    }

    // superbloque auxiliar
    SuperBloque sb_aux;
    fseek(archivo, inicio_particion, SEEK_SET);

    // Escribir el reporte del bitmap de inodos
    string pathDot = pathImagen.substr(0, pathImagen.size() - 4) + ".dot";
    size_t lastSlash = pathImagen.find_last_of('/');
    string folderPathOfImage = pathImagen.substr(0, lastSlash);
    string imageExtension = pathImagen.substr(pathImagen.size() - 3);

    ofstream reporte;
    reporte.open(pathDot, ios::out);

    if (reporte.fail()) {
        string comando1 = "mkdir -p \"" + folderPathOfImage + "\"";
        system(comando1.c_str());
    }
    reporte.close();

    reporte.open(pathDot);

    // Empiezo a leer el superbloque
    fread(&sb_aux, sizeof(SuperBloque), 1, archivo);

    fseek(archivo, sb_aux.bm_block_start, SEEK_SET);

    // Escribo el bitmap de inodos en el txt
    char bitchar[sb_aux.blocks_count];

    for(int i = 0; i < sb_aux.blocks_count; i++){

        fseek(archivo, sb_aux.bm_block_start + i*sizeof(char), SEEK_SET);
        reporte << "  " << bitchar[i] << "  |  ";

        //cout << "  " <<bitchar[i] << "  |  ";
        if(i%20 == 0){
            reporte << "  +  \n";
            //cout << "  +  \n";
        }
    }

    reporte.close();
    fclose(archivo);
}

void Rep::reporteBm_Inode(string nombreParticion, string pathImagen, string pathDisco) {
    FILE *archivo;

    archivo = fopen(pathDisco.c_str(), "rb+");

    if(archivo == NULL){
        cout << " >> El disco no existe. \n";
    }

    int inicio_particion = 0;

    MBR mbr_;
    fseek(archivo, 0, SEEK_SET);
    fread(&mbr_, sizeof(MBR), 1, archivo);

    for(int i = 0; i < 4; i++){
        if(strcmp(mbr_.particiones[i].name, nombreParticion.c_str()) == 0){
            //cout << " >> Size: " << mbr_.mbr_particions[i].part_size << " \n";
            inicio_particion = mbr_.particiones[i].start;
            //tam_particion = mbr_.mbr_particions[i].part_size;
            break;
        }

    }

    // superbloque auxiliar
    SuperBloque sb_aux;
    fseek(archivo, inicio_particion, SEEK_SET);

    // Escribir el reporte del bitmap de inodos
    string pathDot = pathImagen.substr(0, pathImagen.size() - 4) + ".txt";
    size_t lastSlash = pathImagen.find_last_of('/');
    string folderPathOfImage = pathImagen.substr(0, lastSlash);
    string imageExtension = pathImagen.substr(pathImagen.size() - 3);

    ofstream reporte;
    reporte.open(pathDot, ios::out);

    if (reporte.fail()) {
        string comando1 = "mkdir -p \"" + folderPathOfImage + "\"";
        system(comando1.c_str());
    }
    reporte.close();

    reporte.open(pathDot);

    if(reporte.fail()){
        cout << " >> No se pudo abrir \n";
    }

    // Empiezo a leer el superbloque
    fread(&sb_aux, sizeof(SuperBloque), 1, archivo);

    fseek(archivo, sb_aux.bm_inode_start, SEEK_SET);

    // Escribo el bitmap de inodos en el txt
    char bitchar[sb_aux.inodes_count];

    for(int i = 0; i < sb_aux.inodes_count; i++){

        fseek(archivo, sb_aux.bm_inode_start + i*sizeof(char), SEEK_SET);
        reporte << "  " << bitchar[i] << "  |  ";

        //cout << "  " <<bitchar[i] << "  |  ";
        if(i%20 == 0){
            reporte << "  +  \n";
            //cout << "  +  \n";
        }
    }

    reporte.close();
    fclose(archivo);

    cout << endl << " *** Correcto: Se genero el reporte tipo BM Inode en - " << pathDot << endl << endl;
}

void Rep::reporteSB(string nombreParticion, string pathImagen, string pathDisco) {

    FILE *archivo;

    archivo = fopen(pathDisco.c_str(), "rb+");

    if(archivo == NULL){
        cout << endl << " >> *** El disco no existe *** " << endl << endl;
        return;
    }

    int inicio_particion = 0;

    MBR mbr;
    fseek(archivo, 0, SEEK_SET);
    fread(&mbr, sizeof(mbr), 1, archivo);

    for(int i = 0; i < 4; i++){
        if(strcmp(mbr.particiones[i].name, nombreParticion.c_str()) == 0){
            //cout << " >> Size: " << mbr_.mbr_particions[i].part_size << " \n";
            inicio_particion = mbr.particiones[i].start;
            //tam_particion = mbr_.mbr_particions[i].part_size;
            break;
        }

    }

    // superbloque auxiliar
    SuperBloque sb_aux;
    fseek(archivo, inicio_particion, SEEK_SET);

    // Escribir el reporte del superbloque
    string pathDot = pathImagen.substr(0, pathImagen.size() - 4) + ".dot";
    size_t lastSlash = pathImagen.find_last_of('/');
    string folderPathOfImage = pathImagen.substr(0, lastSlash);
    string imageExtension = pathImagen.substr(pathImagen.size() - 3);

    ofstream reporte;
    reporte.open(pathDot, ios::out);

    if (reporte.fail()) {
        string comando1 = "mkdir -p \"" + folderPathOfImage + "\"";
        system(comando1.c_str());
    }
    reporte.close();

    reporte.open(pathDot);

    reporte << "digraph G {\n"
            << "node [shape=plaintext] \n"
            << "nodo [ \n"
            << " label =< \n"
            << "<table border=\"0\" cellborder=\"1\" cellspacing=\"0\">"
            << "<tr> <td bgcolor=\"#30A6BB\">Nombre</td> <td bgcolor=\"#30A6BB\"> Valor </td> </tr>";

    // Empiezo a leer el superbloque
    fread(&sb_aux, sizeof(SuperBloque), 1, archivo);

    reporte << "<tr>\n";
    reporte << "<td bgcolor=\"#89D8E7\"> s_inodes_count </td>";
    reporte << "<td bgcolor=\"#C7F6FF\">" << sb_aux.inodes_count << " </td> \n";
    reporte << "</tr>";

    reporte << "<tr>\n";
    reporte << "<td bgcolor=\"#89D8E7\"> s_blocks_count </td>";
    reporte << "<td bgcolor=\"#C7F6FF\">" << sb_aux.blocks_count << " </td> \n";
    reporte << "</tr>";

    reporte << "<tr>\n";
    reporte << "<td bgcolor=\"#89D8E7\"> s_free_blocks_count </td>";
    reporte << "<td bgcolor=\"#C7F6FF\">" << sb_aux.free_blocks_count << " </td> \n";
    reporte << "</tr>";

    reporte << "<tr>\n";
    reporte << "<td bgcolor=\"#89D8E7\"> s_free_inodes_count </td>";
    reporte << "<td bgcolor=\"#C7F6FF\">" << sb_aux.free_inodes_count << " </td> \n";
    reporte << "</tr>";

    reporte << "<tr>\n";
    reporte << "<td bgcolor=\"#89D8E7\"> s_mtime </td>";
    reporte << "<td bgcolor=\"#C7F6FF\">" << sb_aux.mtime << " </td> \n";
    reporte << "</tr>";

    reporte << "<tr>\n";
    reporte << "<td bgcolor=\"#89D8E7\"> s_umtime </td>";
    reporte << "<td bgcolor=\"#C7F6FF\">" << sb_aux.umtime << " </td> \n";
    reporte << "</tr>";

    reporte << "<tr>\n";
    reporte << "<td bgcolor=\"#89D8E7\"> s_mnt_count </td>";
    reporte << "<td bgcolor=\"#C7F6FF\">" << sb_aux.mnt_count << " </td> \n";
    reporte << "</tr>";

    reporte << "<tr>\n";
    reporte << "<td bgcolor=\"#89D8E7\"> s_magic </td>";
    reporte << "<td bgcolor=\"#C7F6FF\">" << sb_aux.magic << " </td> \n";
    reporte << "</tr>";

    reporte << "<tr>\n";
    reporte << "<td bgcolor=\"#89D8E7\"> s_inode_size </td>";
    reporte << "<td bgcolor=\"#C7F6FF\">" << sb_aux.inode_size << " </td> \n";
    reporte << "</tr>";

    reporte << "<tr>\n";
    reporte << "<td bgcolor=\"#89D8E7\"> s_block_size </td>";
    reporte << "<td bgcolor=\"#C7F6FF\">" << sb_aux.block_size << " </td> \n";
    reporte << "</tr>";

    reporte << "<tr>\n";
    reporte << "<td bgcolor=\"#89D8E7\"> s_first_ino </td>";
    reporte << "<td bgcolor=\"#C7F6FF\">" << sb_aux.first_ino << " </td> \n";
    reporte << "</tr>";

    reporte << "<tr>\n";
    reporte << "<td bgcolor=\"#89D8E7\"> s_first_blo </td>";
    reporte << "<td bgcolor=\"#C7F6FF\">" << sb_aux.first_blo << " </td> \n";
    reporte << "</tr>";

    reporte << "<tr>\n";
    reporte << "<td bgcolor=\"#89D8E7\"> s_bm_inode_start </td>";
    reporte << "<td bgcolor=\"#C7F6FF\">" << sb_aux.bm_inode_start << " </td> \n";
    reporte << "</tr>";

    reporte << "<tr>\n";
    reporte << "<td bgcolor=\"#89D8E7\"> s_bm_block_start </td>";
    reporte << "<td bgcolor=\"#C7F6FF\">" << sb_aux.bm_block_start << " </td> \n";
    reporte << "</tr>";

    reporte << "<tr>\n";
    reporte << "<td bgcolor=\"#89D8E7\"> s_inode_start </td>";
    reporte << "<td bgcolor=\"#C7F6FF\">" << sb_aux.inode_start << " </td> \n";
    reporte << "</tr>";

    reporte << "<tr>\n";
    reporte << "<td bgcolor=\"#89D8E7\"> s_block_start </td>";
    reporte << "<td bgcolor=\"#C7F6FF\">" << sb_aux.block_start << " </td> \n";
    reporte << "</tr>";

    reporte << "</table>\n";
    reporte << ">\n";
    reporte << "];\n";
    reporte << "}";


    reporte.close();
    fclose(archivo);

    string comando2 = "dot -T" + imageExtension + " \"" + pathDot + "\" -o \"" + pathImagen + "\"";
    system(comando2.c_str());

    string comandoOpen = "xdg-open \"" + pathImagen + "\"";
    system(comandoOpen.c_str());

    cout << endl << " *** Correcto: Se genero el reporte tipo MBR en - " << pathDot << endl << endl;



}


void Rep::reporteMBR(MBR discoEditar, string pathImagen, string pathDisco) {

    string hexColores[] = {"#88e3a0", "#e39688", "#e3c588", "#88e3d2", "#b488e3"};
    string codigoParticiones = "";
    string codigoExtendidas = "";
    int contExtendidas = 0;

    FILE *bfilel = fopen(pathDisco.c_str(), "rb+");
    if (bfilel != NULL) {
        rewind(bfilel);
        fread(&discoEditar, sizeof(MBR), 1, bfilel);
        for (int i = 0; i < 4; i++) {
            if(discoEditar.particiones[i].status != '0'){

                codigoParticiones += "       <tr>"
                                     "           <td bgcolor='" + hexColores[i] + "'><b>part" + to_string(i) +"_status</b></td>\n"
                                     "           <td bgcolor='" + hexColores[i] +"'>" + discoEditar.particiones[i].status + "</td>\n"
                                     "      </tr>\n"
                                     "       <tr>\n"
                                     "           <td bgcolor='" + hexColores[i] + "'><b>part" + to_string(i) +"_name</b></td>\n"
                                     "           <td bgcolor='" + hexColores[i] +"'>" + discoEditar.particiones[i].name + "</td>\n"
                                     "      </tr>\n"
                                     "       <tr>\n"
                                     "           <td bgcolor='" + hexColores[i] + "'><b>part" + to_string(i) +"_fit</b></td>\n"
                                     "           <td bgcolor='" + hexColores[i] +"'>" + discoEditar.particiones[i].fit + "</td>\n"
                                     "      </tr>\n"
                                     "       <tr>\n"
                                     "           <td bgcolor='" + hexColores[i] + "'><b>part" + to_string(i) +"_start</b></td>\n"
                                     "           <td bgcolor='" + hexColores[i] +"'>" + to_string(discoEditar.particiones[i].start) + "</td>\n"
                                     "      </tr>"
                                     "       <tr>\n"
                                     "           <td bgcolor='" + hexColores[i] + "'><b>part" + to_string(i) +"_size</b></td>\n"
                                     "           <td bgcolor='" + hexColores[i] +"'>" + to_string(discoEditar.particiones[i].size) + "</td>\n"
                                     "      </tr>"
                                     "       <tr>\n"
                                     "           <td bgcolor='" + hexColores[i] + "'><b>part" + to_string(i) +"_type</b></td>\n"
                                     "           <td bgcolor='" + hexColores[i] +"'>" + discoEditar.particiones[i].type + "</td>\n"
                                     "      </tr>\n";

                if (discoEditar.particiones[i].type == 'e') {
                    EBR logicaR;
                    fseek(bfilel, discoEditar.particiones[i].start, SEEK_SET);
                    fread(&logicaR, sizeof(EBR), 1, bfilel);

                    while(logicaR.next != -1){
                        codigoExtendidas += "ext" + to_string(contExtendidas++) +" [\n"
                                        "   shape=plaintext\n"
                                        "   label=<\n"
                                        "   <table border='1' cellborder='1'>\n"
                                        "       <tr>\n"
                                        "           <td bgcolor='" + hexColores[4] +"'><b>part_name</b></td>\n"
                                        "           <td bgcolor='" + hexColores[4] +"'>" + logicaR.name +"</td>\n"
                                        "       </tr>"
                                        "       <tr>\n"
                                        "           <td bgcolor='" + hexColores[4] +"'><b>part_fit</b></td>\n"
                                        "           <td bgcolor='" + hexColores[4] +"'>" + logicaR.fit +"</td>\n"
                                        "       </tr>"
                                        "       <tr>\n"
                                        "           <td bgcolor='" + hexColores[4] +"'><b>part_start</b></td>\n"
                                        "           <td bgcolor='" + hexColores[4] +"'>" + to_string(logicaR.part_start) +"</td>\n"
                                        "       </tr>"
                                        "       <tr>\n"
                                        "           <td bgcolor='" + hexColores[4] +"'><b>part_size</b></td>\n"
                                        "           <td bgcolor='" + hexColores[4] +"'>" + to_string(logicaR.size) +"</td>\n"
                                        "       </tr>"
                                        "       <tr>\n"
                                        "           <td bgcolor='" + hexColores[4] +"'><b>part_next</b></td>\n"
                                        "           <td bgcolor='" + hexColores[4] +"'>" + to_string(logicaR.next) +"</td>\n"
                                        "       </tr>"
                                        "       <tr>\n"
                                        "           <td bgcolor='" + hexColores[4] +"'><b>part_status</b></td>\n"
                                        "           <td bgcolor='" + hexColores[4] +"'>" + logicaR.status +"</td>\n"
                                        "       </tr>"
                                        "   </table>"
                                        ">]\n";

                        fseek(bfilel, logicaR.next, SEEK_SET);
                        fread(&logicaR, sizeof(EBR), 1, bfilel);
                    }


                    if (contExtendidas > 0) {
                        string rankSame = "";
                        for (int i = 0; i < contExtendidas; i++) {
                            if (i < contExtendidas - 1) {
                                codigoExtendidas += "   ext" + to_string(i) + " -> ext" + to_string(i + 1) + ";\n";
                            }
                            rankSame += "ext" + to_string(i) + " ";
                        }
                        rankSame = "    {rank=same; " + rankSame + "}\n";
                        codigoExtendidas += rankSame;
                    }

                }
            }
        }
        fclose(bfilel);

        // CODIGO MBR
        string codigoInterno = "";
        string size = to_string(discoEditar.tamano);
        string date(discoEditar.fecha_creacion);
        string firma = to_string(discoEditar.disk_signature);

        string codigoMBR = "       <tr>\n"
                           "           <td><b>MBR_Tamanio</b></td>\n"
                           "           <td>" + size + "</td>\n"
                           "       </tr>\n"
                           "       <tr>\n"
                           "           <td><b>MBR_Fecha_Creacion</b></td>\n"
                           "           <td>" + date + "</td>\n"
                           "       </tr>\n"
                           "       <tr>\n"
                           "           <td><b>MBR_Disk_Signature</b></td>\n"
                           "           <td>" + firma + "</td>\n"
                           "       </tr>\n";

        string codigo = "digraph  {\n"
                        "parent [\n"
                        "   shape=plaintext\n"
                        "   label=<\n"
                        "   <table border='1' cellborder='1'>\n"
                        "       <tr>\n"
                        "           <td><b>Propiedad</b></td>\n"
                        "           <td><b>Valor</b></td>\n"
                        "       </tr>"
                        +       codigoMBR + "\n"
                        +       codigoParticiones +
                        "   </table>"
                        ">];"
                        + codigoExtendidas +
                        "}\n";
        string pathDot = pathImagen.substr(0, pathImagen.size() - 4) + ".dot";
        size_t lastSlash = pathImagen.find_last_of('/');
        string folderPathOfImage = pathImagen.substr(0, lastSlash);
        string imageExtension = pathImagen.substr(pathImagen.size() - 3);


        ofstream dotFile;
        dotFile.open(pathDot);

        if (dotFile.fail()) {
            string comando1 = "mkdir -p \"" + folderPathOfImage + "\"";
            system(comando1.c_str());
        }
        dotFile.close();

        dotFile.open(pathDot);
        dotFile << codigo;
        dotFile.close();

        string comando2 = "dot -T" + imageExtension + " \"" + pathDot + "\" -o \"" + pathImagen + "\"";
        system(comando2.c_str());

        string comandoOpen = "xdg-open \"" + pathImagen + "\"";
        system(comandoOpen.c_str());

        cout << endl << " *** Correcto: Se genero el reporte tipo MBR en - " << pathDot << endl << endl;

    } else {
        cout << endl << " *** No se pueden mostrar las particiones por path incorrecto *** " << endl << endl;
    }

}

void Rep::reporteDisco(MBR discoEditar, string pathImagen, string pathDisco) {
    string codigoInterno = "";
    string size = to_string(discoEditar.tamano);
    string date(discoEditar.fecha_creacion);
    string firma = to_string(discoEditar.disk_signature);
    string fit = "";
    int acumulado = 0;

    string codigoParticiones = "";
    string codigoLogicas = "";

    for (int i = 0; i < 4; i++) {
        if (discoEditar.particiones[i].status == '1') {
            string name(discoEditar.particiones[i].name);
            string Estado = "";
            Estado.push_back(discoEditar.particiones[i].status);
            string type = "";
            type.push_back(discoEditar.particiones[i].type);
            int porcentaje = ((discoEditar.particiones[i].size * 100) / discoEditar.tamano);
            acumulado = acumulado + porcentaje;
            string porcentajeString = to_string(porcentaje);

            if (type == "p") {
                codigoParticiones = codigoParticiones + "           <td rowspan='2'>" + name + "<br/>" + type + "<br/>" + porcentajeString  + "%</td>\n";
            } else if (type == "e") {
                int inicioLogicas = discoEditar.particiones[i].start;
                int contLogicas = 0;
                EBR logicaI;
                FILE *bfilel = fopen(pathDisco.c_str(), "rb+");
                fseek(bfilel, inicioLogicas, SEEK_SET);
                fread(&logicaI, sizeof(EBR), 1, bfilel);
                while(logicaI.next != -1){
                    if(logicaI.status != '0'){
                        codigoLogicas += "                          <td border='1'>EBR</td>\n";
                        codigoLogicas += "                          <td border='1'>Logica<br/>" + (string) logicaI.name  + "</td>\n";
                        contLogicas++;
                    }
                    fseek(bfilel, logicaI.next, SEEK_SET);
                    fread(&logicaI, sizeof(EBR), 1, bfilel);
                }
                fclose(bfilel);

                codigoParticiones += "              <td>\n"
                                     "                  <table border='0'>\n"
                                     "                      <tr><td border='1' colspan='"+ to_string(contLogicas * 2) + "' >(Extendida)<br/>" + name + "<br/>" +porcentajeString +"%</td></tr>\n"
                                     "                      <tr>\n"
                                     + codigoLogicas +
                                     "                      </tr>\n"
                                     "                  </table>\n"
                                     "              </td>\n";
            }
        }
    }

    codigoLogicas = "";
    int libre = 100 - acumulado;
    string libreString = to_string(libre);
    if(acumulado<100){
        codigoParticiones = codigoParticiones + "           <td rowspan='2'>Libre<br/>" + libreString + "%</td>\n";

    }

    string codigo = "digraph  {\n"
                    "parent [\n"
                    "   shape=plaintext\n"
                    "   label=<\n"
                    "   <table border='1' cellborder='1'>\n"
                    "       <tr>\n"
                    "           <td rowspan='2'>MBR</td>\n"
                    + codigoParticiones +
                    "       </tr>\n"
                    "   </table>"
                    ">];"
                    "}\n";

    string pathDot = pathImagen.substr(0, pathImagen.size() - 4) + ".dot";
    size_t lastSlash = pathImagen.find_last_of('/');
    string folderPathOfImage = pathImagen.substr(0, lastSlash);
    string imageExtension = pathImagen.substr(pathImagen.size() - 3);

    // cout << codigo << endl;

    ofstream dotFile;
    dotFile.open(pathDot);

    if (dotFile.fail()) {
        string comando1 = "mkdir -p \"" + folderPathOfImage + "\"";
        //cout << endl << comando1 << endl;
        system(comando1.c_str());
    }
    dotFile.close();

    dotFile.open(pathDot);
    dotFile << codigo;
    dotFile.close();

    string comando2 = "dot -T" + imageExtension + " \"" + pathDot + "\" -o \"" + pathImagen + "\"";
    system(comando2.c_str());

    string comandoOpen = "xdg-open \"" + pathImagen + "\"";
    system(comandoOpen.c_str());

    cout << endl << " *** Correcto: Se genero el reporte tipo Disk en - " << pathDot << endl << endl;
}

#endif //VACASJUNIO_MIA_PROYECTO1_REP_H
