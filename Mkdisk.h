#ifndef VACASJUNIO_MIA_PROYECTO1_MKDISK_H
#define VACASJUNIO_MIA_PROYECTO1_MKDISK_H

#include "Structs.h"
#include <iostream>
#include <string.h>

using namespace std;

class Mkdisk {

public:
    Mkdisk();
    bool crearDisco(int size, string path, string name);

private:
    bool existeDisco(string path);

};


Mkdisk::Mkdisk() {}

bool Mkdisk::existeDisco(string path) {
    FILE *disk = fopen(path.c_str(), "rb");

    if (!disk) {
        return false;
    }

    fclose(disk);
    return true;
}

bool Mkdisk::crearDisco(int size, string path, string name) {

    // Validar que el nombre termine con la extensión .dsk
    if (!name.ends_with(".dsk")) {
        cout << endl << " *** El disco debe tener extensión .dsk ***" << endl << endl;
        return false;
    }

    string fullPath = path;
    if (fullPath.ends_with("/")) {
        fullPath += name;
    } else {
        fullPath += "/" + name;
    }

    if (existeDisco(fullPath)) {
        cout << endl << " *** El disco ya existe ***" << endl << endl;
        return false;
    }

    // Obtener Hora Creacion
    time_t t;
    struct tm *tm;
    char fechayhora[20];

    t = time(NULL);
    tm = localtime(&t);
    strftime(fechayhora, 20, "%d/%m/%Y %H:%M:%S", tm);

    size = size * 1000 * 1000; //Mb

    // Crear MBR del disco
    MBR mbr;
    mbr.tamano = size;
    strcpy(mbr.fecha_creacion, fechayhora);
    mbr.disk_signature = rand() % 1000;

    cout << endl << "Disco creado" << endl;
    cout << "Size: " << mbr.tamano / (1000 * 1000) << "MB" << endl;
    cout << "Fecha: " << mbr.fecha_creacion << endl;
    cout << "Signature: " << mbr.disk_signature  << endl;

    // Crear el disco y escribir el mbr dentro
    FILE* disco = fopen(fullPath.c_str(), "wb");
    if (disco != NULL) {
        fwrite("\0", 1, 1, disco);
        fseek(disco, size - 1, SEEK_SET);
        fwrite("\0", 1, 1, disco);
        rewind(disco);
        fwrite(&mbr, sizeof(MBR), 1, disco);

        cout << endl << "Disco Creado Exitosamente" << endl << endl;
    } else {
        // Crear la estructura de directorios
        string comando1 = "sudo mkdir -p \"" + path + "\"";
        string comando2 = "sudo chmod -R 777 \"" + path + "\"";
        system(comando1.c_str());
        system(comando2.c_str());


        disco = fopen(fullPath.c_str(), "wb");
        fwrite("\0", 1, 1, disco);
        fseek(disco, size - 1, SEEK_SET);
        fwrite("\0", 1, 1, disco);
        rewind(disco);
        fwrite(&mbr, sizeof(MBR), 1, disco);

        cout << endl << "Disco Creado Exitosamente" << endl << endl;
    }

    fclose(disco);

    return true;

}




#endif //VACASJUNIO_MIA_PROYECTO1_MKDISK_H
