#include "Carpeta.h"
Carpeta::Carpeta() { }

void Carpeta::setNombreCarpeta(string nombreCarpeta) {
    this->nombreCarpeta = nombreCarpeta;
}

string Carpeta::getNombreCarpeta() {
    return this->nombreCarpeta;
}