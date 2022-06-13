#ifndef VACASJUNIO_MIA_PROYECTO1_FDISK_H
#define VACASJUNIO_MIA_PROYECTO1_FDISK_H

#include <string>
#include "Structs.h"

using namespace std;

class Fdisk {
public:
    Fdisk();
    bool administrarParticion(int size, string path, string name, string unit, string type, string fit, string del, string add, string mov);
    bool deleteParticion(string del, string path, string name);
    bool addParticion(string add, string unit, string name, string path);
    bool createParticion(int size, string unit, string path, string name, string fit, string type);
    bool createParticionLogica(Particion, string path);
    void showParticiones(string path);
};

bool Fdisk::createParticion(int size, string unit, string path, string name, string fit, string type) {
    // Fit: BF, FF, WF. Default:(WF)   -  Type: P, E, L.   Default: (P)   -   Unit: B, K, M.  Default: (K)

    Particion particion;
}

#endif //VACASJUNIO_MIA_PROYECTO1_FDISK_H
