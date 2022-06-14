//
// Created by gtcaps on 7/06/22.
//

#ifndef VACASJUNIO_MIA_PROYECTO1_CONSOLA_H
#define VACASJUNIO_MIA_PROYECTO1_CONSOLA_H


#include <vector>
#include <string>
#include "Mount.h"

using namespace std;

class Consola {
public:
    Consola();
    void ejecutar();
private:
    bool ejecutarComando(string comando);


    // comandos
    bool mkdisk(string comando);
    bool rmdisk(string comando);
    bool fdisk(string comando);
    bool mount(string comando);
    bool umount(string comando);
    bool mkfs(string comando);
    bool mkfile(string comando);
    bool script(string comando);
    bool exec(string comando);
    bool rm(string comando);
    bool mkdir(string comando);
    bool cp(string comando);
    bool ls(string comando);
    bool pause(string comando);
    bool loss(string comando);
    bool rep(string comando);

    Mount montaje;

    string toLowerCase(string text);

    vector<string> getAtributtes(string comando);

    vector<string> split(string data);

    bool isNumber(string text);

};

#endif //VACASJUNIO_MIA_PROYECTO1_CONSOLA_H
