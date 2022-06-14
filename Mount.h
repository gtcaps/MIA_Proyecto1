//
// Created by gtcaps on 14/06/22.
//

#ifndef VACASJUNIO_MIA_PROYECTO1_MOUNT_H
#define VACASJUNIO_MIA_PROYECTO1_MOUNT_H

#include <iostream>
#include <string>

using namespace std;

class Mount {
public:
    Mount();

    struct ParticionMontada {
        int numero = 0;
        int estado = 0;
        char name[16] = "";
    };

    struct DiscoMontado {
        char path[100] = "";
        char letra;
        int estado = 0;
        ParticionMontada particiones[99];
    };

    DiscoMontado discos[26];

    bool montarParticion(string path, string name);
    void leerMontaje();
    bool desmontarParticion(string id);
};


#endif //VACASJUNIO_MIA_PROYECTO1_MOUNT_H
