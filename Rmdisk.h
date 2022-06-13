#ifndef VACASJUNIO_MIA_PROYECTO1_RMDISK_H
#define VACASJUNIO_MIA_PROYECTO1_RMDISK_H

#include <iostream>

using namespace std;

class Rmdisk {
public:
    Rmdisk();
    bool eliminarDisco(string path);
};

Rmdisk::Rmdisk() {}

bool Rmdisk::eliminarDisco(string path) {

    if (remove(path.c_str()) == 0) {
        cout << endl << " *** Disco eliminado exitosamente ***" << endl << endl;
        return true;
    } else {
        cout << endl << " *** Disco no existente ***" << endl << endl;
        return false;
    }
}


#endif //VACASJUNIO_MIA_PROYECTO1_RMDISK_H
