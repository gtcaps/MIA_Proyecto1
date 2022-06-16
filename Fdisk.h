#ifndef VACASJUNIO_MIA_PROYECTO1_FDISK_H
#define VACASJUNIO_MIA_PROYECTO1_FDISK_H

#include <iostream>
#include <string>
#include <cstring>
#include "Structs.h"
#include "Consola.h"

using namespace std;

class Fdisk {
public:
    Fdisk();
    bool administrarParticion(int size, string path, string name, string unit, string type, string fit, string del, string add, string mov);
    bool deleteParticion(string del, string path, string name);
    bool addParticion(string add, string unit, string name, string path);
    bool createParticion(int size, string unit, string path, string name, string fit, string type);
    bool createParticionLogica(Particion particion, string path);
    void showParticiones(string path);
};

Fdisk::Fdisk() {}

bool Fdisk::administrarParticion(int size, string path, string name, string unit, string type, string fit, string del,
                                 string add, string mov) {

    if (add != "") {
        return addParticion(add, unit, name, path);
    } else if (del != "") {
        return deleteParticion(del, path, name);
    } else {
        return createParticion(size, unit, path, name, fit, type);
    }


}

bool Fdisk::createParticion(int size, string unit, string path, string name, string fit, string type) {
    // Fit: BF, FF, WF. Default:(WF)   -  Type: P, E, L.   Default: (P)   -   Unit: B, K, M.  Default: (K)

    Particion particion;

    // Asignar el Tamano de la particion
    if (unit == "m") {
        size = size * 1024 * 1024;
    } else if (unit == "k") {
        size = size * 1024;
    } else if (unit == "b") {
        size = size;
    } else {
        cout << endl << " *** Error en la unidad de la particion *** " << endl << endl;
        return false;
    }

    if (size <= 0) {
        cout << endl << " *** No se puede asignar el tamanio a la particion ***" << endl << endl;
        return false;
    }

    particion.status = '1';
    particion.fit = fit[0];
    strcpy(particion.name, name.c_str());
    particion.start = 0;
    particion.size = size;
    particion.type = type[0];

    // Tipo de Particion
    if (type == "p" || type == "e") {

    } else if (type == "l") {
        bool logicaCreada = createParticionLogica(particion, path);
        if (logicaCreada) {
            showParticiones(path);
        }
        return true;
    } else {
        cout << endl << " *** Error en el tipo de particion *** " << endl << endl;
    }

    // MBR
    MBR mbr;
    FILE* disco = fopen(path.c_str(), "rb+");
    if (disco != NULL) {
        rewind(disco);
        fread(&mbr, sizeof(mbr), 1, disco);
    } else {
        cout << endl << " *** Error el path del disco no existe *** " << endl << endl ;
    }
    fclose(disco);

    // Validar informacion del disco
    //cout << endl << "Disco Leido:" << endl << "Size: " << mbr.tamano << endl << "Date: " << mbr.fecha_creacion << endl << endl;

    // Editar Disco
    bool discoLleno = true;
    int conteoExtendida = 0;
    bool tamanoPasado = false;
    int part_startExtendida = 0;
    bool ultimaParticion = false;

    // Validar Particiones Extendidas
    for (int i = 0; i < 4; i++) {
        if (mbr.particiones[i].type == 'e') {
            conteoExtendida++;
        }
    }

    // Validar si el disco esta lleno
    for (int i = 0; i < 4; i++) {
        if (mbr.particiones[i].status == '0') {
            discoLleno = false;
            break;
        }
    }

    // Ordenar y Primer Ajuste (FF).
    for (int i = 0; i < 4; i++) {
        if (mbr.particiones[i].status != '0') {
            if (!(particion.type == 'e' && conteoExtendida > 0)) {
                //PRIMER AJUSTE
                if (i == 0) {
                    if (mbr.particiones[i].start - sizeof(MBR) >= particion.size) {
                        particion.start = sizeof(MBR);
                        break;
                    }
                } else {
                    if (mbr.particiones[i].start - (mbr.particiones[i - 1].start + mbr.particiones[i - 1].size) >= particion.size) {
                        particion.start = mbr.particiones[i - 1].start + mbr.particiones[i - 1].size;

                        break;
                    }
                }
            } else {
                cout << endl << " *** Error ya existe una particion extendida, no puedes crear otra." << endl << endl;
                tamanoPasado = true;
                return false;
            }
        } else {
            if (i == 0) {
                //PRIMERA POSICION
                particion.start = sizeof(MBR);
                break;
            } else {
                //ULTIMA POSICION
                ultimaParticion = true;
                break;
            }
        }
    }

    for (int i = 0; i < 4; i++) {
        if (mbr.particiones[i].status != '1') {
            if (ultimaParticion) {
                particion.start = mbr.particiones[i - 1].start + mbr.particiones[i - 1].size;
                if (mbr.tamano < (particion.start + particion.size)) {
                    tamanoPasado = true;
                }
            }

            mbr.particiones[i] = particion;

            //Es extendida, creo el EBR.
            if (particion.type == 'e') {
                part_startExtendida = particion.start;
            }
            break;
        }
    }

    //METODO DE ORDENAMIENTO BURBUJA.
    Particion aux;
    for (int i = 1; i < 4; i++) {
        for (int j = 0; j < 4 - i; j++) {
            if ((mbr.particiones[j].start > mbr.particiones[j + 1].start) && mbr.particiones[j + 1].status != '0') {
                aux = mbr.particiones[j + 1];
                mbr.particiones[j + 1] = mbr.particiones[j];
                mbr.particiones[j] = aux;
            }
        }
    }

    if (!discoLleno && !tamanoPasado) {
        FILE* bfile = fopen(path.c_str(), "rb+");
        if (bfile != NULL) {
            rewind(bfile);
            fwrite(&mbr, sizeof(MBR), 1, bfile);

            //Crea primer EBR
            if (part_startExtendida != 0) {
                rewind(bfile);
                EBR logica;
                logica.part_start = part_startExtendida;
                logica.status = '0';
                strcpy(logica.name, "NoName");
                fseek(bfile, logica.part_start, SEEK_SET);
                fwrite(&logica, sizeof(EBR), 1, bfile);
            }
        }
        fclose(bfile);
        showParticiones(path);
    } else {
        if (discoLleno) {
            cout << endl << " *** Error numero de particiones maximas creadas *** " << endl << endl;
            return false;
        } else {
            cout << endl << " *** Error tamanio de la particion sobre pasa el disco *** " << endl << endl;
            return false;
        }
    }

    return true;

}

bool Fdisk::deleteParticion(string del, string path, string name) {

    //Full or Fast
    cout << "Seguro que deseas eliminar la particion? [s/n]" << endl;
    string respuesta;
    cin >> respuesta;

    int inicioExtendida = -1;

    if (respuesta == "s") {

        //ELIMINAR PARTICION LOGICA.
        EBR logicaD;
        FILE *bfile = fopen(path.c_str(), "rb+");
        if (bfile != NULL) {
            MBR discoEdit;
            rewind(bfile);
            fread(&discoEdit, sizeof(MBR), 1, bfile);
            for (int i = 0; i < 4; i++) {
                if (discoEdit.particiones[i].type == 'e') {
                    inicioExtendida = discoEdit.particiones[i].start;
                }
            }

            if(inicioExtendida != -1){
                fseek(bfile, inicioExtendida, SEEK_SET);
                fread(&logicaD, sizeof(EBR), 1, bfile);
                while(logicaD.next != -1){
                    if(strcmp(logicaD.name, name.c_str()) == 0){
                        logicaD.status = '0'; // Elimino la particion logica.
                        fseek(bfile, logicaD.part_start, SEEK_SET);
                        fwrite(&logicaD, sizeof(EBR), 1, bfile);
                        cout << endl << " *** Particion logica eliminada exitosamente ***" << endl << endl;
                        fclose(bfile);
                        return true;
                    }
                    fseek(bfile, logicaD.next, SEEK_SET);
                    fread(&logicaD, sizeof(EBR), 1, bfile);
                }
            }
            fclose(bfile);
        }else{
            cout << endl << " *** Path no existente en la eliminacion de particion *** " << endl << endl;
            return false;
        }


        //ELIMINACION DE PRIMARIAS Y EXTENDIDAS.
        if (del == "fast") {
            MBR discoEditar;
            FILE *bfile2 = fopen(path.c_str(), "rb+");
            if (bfile2 != NULL) {
                rewind(bfile2);
                fread(&discoEditar, sizeof(discoEditar), 1, bfile2);

                //ELIMINANDO FAST
                for (int i = 0; i < 4; i++) {
                    if (strcmp(discoEditar.particiones[i].name, name.c_str()) == 0)
                    {
                        discoEditar.particiones[i].status = '0';

                        cout << endl << " *** Particion eliminada correctamente./Fast *** " << endl  << endl;
                        int j = i;
                        Particion aux;
                        while (j != 3) {
                            if (discoEditar.particiones[j + 1].status != '0') {
                                aux = discoEditar.particiones[j];
                                discoEditar.particiones[j] = discoEditar.particiones[j + 1];
                                discoEditar.particiones[j + 1] = aux;
                            }
                            j++;
                        }
                        break;
                    }
                }

                rewind(bfile2);
                fwrite(&discoEditar, sizeof(MBR), 1, bfile2);
            } else {
                cout << endl <<  " *** Error: Path no existente en la eliminacion de particion ***" << endl << endl;
                return false;
            }
            fclose(bfile2);
        } else if (del == "full") {
            MBR discoEditar;
            FILE *bfile2 = fopen(path.c_str(), "rb+");
            if (bfile2 != NULL) {
                rewind(bfile2);
                fread(&discoEditar, sizeof(discoEditar), 1, bfile2);

                //ELIMINANDO FULL
                for (int i = 0; i < 4; i++) {
                    if (strcmp(discoEditar.particiones[i].name, name.c_str()) == 0) {
                        discoEditar.particiones[i].status = '0';
                        discoEditar.particiones[i].fit = ' ';
                        strcpy(discoEditar.particiones[i].name, "");
                        discoEditar.particiones[i].start = 0;
                        discoEditar.particiones[i].size = 0;
                        discoEditar.particiones[i].type = ' ';

                        cout << endl << " *** Particion eliminada correctamente. /Full *** " << endl << endl;
                        int j = i;
                        Particion aux;
                        while (j != 3)  { //ORDENAMIENTO
                            if (discoEditar.particiones[j + 1].status != '0') {
                                aux = discoEditar.particiones[j];
                                discoEditar.particiones[j] = discoEditar.particiones[j + 1];
                                discoEditar.particiones[j + 1] = aux;
                            }
                            j++;
                        }
                        break;
                    }
                }

                rewind(bfile2);
                fwrite(&discoEditar, sizeof(MBR), 1, bfile2);
            } else {
                cout << endl << " *** Error: Path no existente en la eliminacion de particion. ***" << endl << endl;
                return false;
            }
            fclose(bfile2);
        } else {
            cout << endl << " *** Error: Valor de delete es incorrecto. ***" << endl << endl;
            return false;
        }
    } else {
        cout << endl << " *** Comando cancelado *** " << endl << endl;
        return false;
    }

    return true;
}



bool Fdisk::addParticion(string add, string unit, string name, string path) {
    // Positive or negative number
    int addSize = stoi(add);

    if (unit == "m") {
        addSize = addSize * 1024 * 1024;
    } else if (unit == "k") {
        addSize = addSize * 1024;
    } else if (unit == "B" || unit == "b") {
    } else {
        cout << endl << " *** Error en la unidad de la particion *** " << endl << endl;
        return false;
    }

    MBR discoEditar;
    FILE *bfile2 = fopen(path.c_str(), "rb+");
    if (bfile2 != NULL) {
        rewind(bfile2);
        fread(&discoEditar, sizeof(MBR), 1, bfile2);

        for (int i = 0; i < 4; i++) {
            if (strcmp(discoEditar.particiones[i].name, name.c_str()) == 0) {
                if (((discoEditar.particiones[i].size + (addSize)) > 0)) {
                    if (discoEditar.particiones[i + 1].start != 0) {
                        if (((discoEditar.particiones[i].size + (addSize) + discoEditar.particiones[i].start) <=
                             discoEditar.particiones[i + 1].start)) {
                            discoEditar.particiones[i].size = discoEditar.particiones[i].size + (addSize);
                            cout << endl << " *** Size modificado correctamente *** " << endl << endl;
                            break;
                        } else {
                            cout << endl << " *** Error size incorrecta, sobrepasa limite de particiones *** " << endl << endl;
                        }
                    } else {
                        if ((discoEditar.particiones[i].size + (addSize) + discoEditar.particiones[i].start) <= discoEditar.tamano) {
                            discoEditar.particiones[i].size = discoEditar.particiones[i].size + (addSize);
                            cout << endl << " *** Size modificado correctamente *** " << endl << endl;
                            break;
                        } else {
                            cout << endl <<  " *** Error size incorrecta, sobrepasa limite de disco ***" << endl << endl;
                        }
                    }
                } else {
                    cout << "Error: Size menor a cero, no se puede reducir." << endl;
                }
            }
        }
        rewind(bfile2);
        fwrite(&discoEditar, sizeof(MBR), 1, bfile2);
    } else {
        cout << " *** Error Path no existente en la eliminacion de particion. *** " << endl << endl;
        return false;
    }
    fclose(bfile2);
    return true;
}


bool Fdisk::createParticionLogica(Particion particion, string path) {

    EBR newLogica;

    newLogica.status = '1';
    newLogica.fit = particion.fit;
    strcpy(newLogica.name, particion.name);
    newLogica.next = -1;
    newLogica.size = particion.size;

    bool existeExtendida = false;
    int posExtendida = 0;
    MBR discoEditar;
    FILE *bfile2 = fopen(path.c_str(), "rb+");
    if (bfile2 != NULL) {
        rewind(bfile2);
        fread(&discoEditar, sizeof(MBR), 1, bfile2);

        for(int i = 0; i < 4; i++){
            if(discoEditar.particiones[i].type == 'e'){
                existeExtendida = true;
                posExtendida = i;
            }
        }

        if(existeExtendida){
            int posicion = discoEditar.particiones[posExtendida].start;
            EBR getLogica;
            fseek(bfile2, posicion, SEEK_SET);
            fread(&getLogica, sizeof(EBR), 1, bfile2);
            bool finalizar = false;
            do{
                if(getLogica.status == '0' && getLogica.next == -1){
                    newLogica.part_start = getLogica.part_start;
                    newLogica.next = newLogica.part_start + newLogica.size;
                    fseek(bfile2, newLogica.part_start, SEEK_SET);
                    fwrite(&newLogica, sizeof(EBR), 1, bfile2);
                    fseek(bfile2, newLogica.next, SEEK_SET);

                    EBR addLogic;
                    addLogic.part_start = newLogica.next;
                    fseek(bfile2, addLogic.part_start, SEEK_SET);
                    fwrite(&addLogic, sizeof(EBR), 1, bfile2);
                    finalizar = true;
                    cout << endl << " *** Particion logica creada exitosamente, " << newLogica.name << endl << endl;
                }
                fseek(bfile2, getLogica.next, SEEK_SET);
                fread(&getLogica, sizeof(EBR), 1, bfile2);
            } while(!finalizar);
        } else {
            cout << endl << " *** Error, no existe particion extendida *** " << endl << endl;
            return false;
        }
    } else {
        cout << endl << " *** Error, Path no existente en la creacion de particion logica *** " << endl << endl;
        return false;
    }
    fclose(bfile2);
    return true;
}

void Fdisk::showParticiones(string path) {
    MBR discoEditar;
    FILE *bfilel = fopen(path.c_str(), "rb+");
    if (bfilel != NULL) {
        rewind(bfilel);
        fread(&discoEditar, sizeof(MBR), 1, bfilel);
        cout << endl << "<<------------------------ PARTICIONES ---------------------------->>" << endl;
        for (int i = 0; i < 4; i++) {
            if(discoEditar.particiones[i].status != '0'){
                cout << "<< ------------------- " << i << " ------------------->>" << endl;
                cout << "Estado: " << discoEditar.particiones[i].status << endl;
                cout << "Nombre: " << discoEditar.particiones[i].name << endl;
                cout << "Fit: " << discoEditar.particiones[i].fit << endl;
                cout << "Part_start: " << discoEditar.particiones[i].start << endl;
                cout << "Size: " << discoEditar.particiones[i].size << endl;
                cout << "Type: " << discoEditar.particiones[i].type << endl;

                if (discoEditar.particiones[i].type == 'e') {
                    EBR logicaR;
                    fseek(bfilel, discoEditar.particiones[i].start, SEEK_SET);
                    fread(&logicaR, sizeof(EBR), 1, bfilel);

                    while(logicaR.next != -1){
                        cout << "<< -------------------- Particion Logica --------------------->> " << endl;
                        cout << "Nombre: " << logicaR.name << endl;
                        cout << "Fit: " << logicaR.fit << endl;
                        cout << "Part_start: " << logicaR.part_start << endl;
                        cout << "Size: " << logicaR.size << endl;
                        cout << "Part_next: " << logicaR.next << endl;
                        cout << "Estado: " << logicaR.status << endl;

                        fseek(bfilel, logicaR.next, SEEK_SET);
                        fread(&logicaR, sizeof(EBR), 1, bfilel);
                    }
                }
            }
        }
        fclose(bfilel);
    }else{
        cout << endl << " *** No se pueden mostrar las particiones por path incorrecto *** " << endl << endl;
    }
}

#endif //VACASJUNIO_MIA_PROYECTO1_FDISK_H
