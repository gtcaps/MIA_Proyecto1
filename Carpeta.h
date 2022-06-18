//
// Created by gtcaps on 16/06/22.
//

#ifndef VACASJUNIO_MIA_PROYECTO1_CARPETA_H
#define VACASJUNIO_MIA_PROYECTO1_CARPETA_H
#include <iostream>
#include <string>
using namespace std;

class Carpeta {
private:
    string nombreCarpeta;

public:
    Carpeta();

    void setNombreCarpeta(string nombreCarpeta);
    string getNombreCarpeta();
};


#endif //VACASJUNIO_MIA_PROYECTO1_CARPETA_H
