//
// Created by gtcaps on 7/06/22.
//
#include <iostream>
#include <string>
#include "Consola.h"

#include "Mkdisk.h"
#include "Rmdisk.h"
#include "Fdisk.h"
#include "Mount.h"

using namespace std;

Consola::Consola() {

}

void Consola::ejecutar() {
    string comando;

    while (true) {
        cout << "> ";
        getline(cin, comando);

        // Salir de la consola
        if (comando == "exit") {
            break;
        }

        if (comando.starts_with("/*") || comando.empty()) {
            continue;
        }

        ejecutarComando(toLowerCase(comando));

    }
}

bool Consola::ejecutarComando(string comando) {


    // MKDISK ================
    if (comando.starts_with("mkdisk")) {
        return mkdisk(comando);
    }

    // RMDISK ================
    if (comando.starts_with("rmdisk")) {
        return rmdisk(comando);
    }

    // FDISK ================
    if (comando.starts_with("fdisk")) {
        return fdisk(comando);
    }

    // MOUNT ================
    if (comando.starts_with("mount")) {
        return mount(comando);
    }

    cout << "comando: invalido" << endl;

    return false;
}

bool Consola::mkdisk(string comando) {
    vector<string> v = getAtributtes(comando);
    string path = "", name= "";
    int size = 0;

    for (auto it: v) {
        vector<string> s = split(it);

        if (s.at(0) == "size") {
            if (!isNumber(s.at(1))) {
                cout << endl << " *** El size debe de ser un numero mayor a 0 ***" << endl << endl;
                return false;
            }

            size = stoi(s.at(1));

            if (size <= 0 || (size % 8 != 0) ) {
                cout << endl << " *** El size debe de ser multiplo de 8 y mayor a 0 ***" << endl << endl;
                return false;
            }

        }

        if (s.at(0) == "path") {
            path = s.at(1);
        }

        if (s.at(0) == "name") {
            name = s.at(1);
        }

    }

    if (size == 0 || path.empty() || name.empty()) {
        cout << endl << " *** Parametros obligatorios: size, path, name *** " << endl << endl;
        return false;
    }

    Mkdisk mkdisk;
    return mkdisk.crearDisco(size, path, name);
}

bool Consola::rmdisk(string comando) {

    vector<string> v = getAtributtes(comando);
    string path = "";

    for (auto it: v) {
        vector<string> s = split(it);

        if (s.at(0) == "path") {
            path = s.at(1);
        }
    }

    if (path.empty() ) {
        cout << endl << " *** Parametros obligatorios: path *** " << endl << endl;
        return false;
    }

    Rmdisk rmdisk;
    return rmdisk.eliminarDisco(path);

}

bool Consola::fdisk(string comando) {

    vector<string> v = getAtributtes(comando);
    int size = 0;
    string path = "", name = "", unit = "", type = "", fit = "", del = "", add = "", mov = "";

    for (auto it: v) {
        vector<string> s = split(it);

        if (s.at(0) == "size") {
            if (!isNumber(s.at(1)) || s.at(1) == "0") {
                cout << endl << " *** El size debe de ser un numero mayor a 0 ***" << endl << endl;
                return false;
            }

            size = stoi(s.at(1));
            continue;
        }

        if (s.at(0) == "path") {
            path = s.at(1);
            continue;
        }

        if (s.at(0) == "name") {
            name = s.at(1);
            continue;
        }

        if (s.at(0) == "unit") {
            unit = s.at(1);
            continue;
        }

        if (s.at(0) == "type") {
            type = s.at(1);
            continue;
        }

        if (s.at(0) == "fit") {
            fit = s.at(1);
            continue;
        }

        if (s.at(0) == "delete") {
            del = s.at(1);
            continue;
        }

        if (s.at(0) == "add") {
            add = s.at(1);
            continue;
        }

        if (s.at(0) == "mov") {
            mov = s.at(1);
            continue;
        }
    }

    // Parametros obligatorios
    if (size == 0 || path.empty() || name.empty()) {
        cout << endl << " *** Parametros obligatorios: size, path, name *** " << endl << endl;
        return false;
    }

    // Unidades
    if (unit == "") {
        unit  = "k";
    }

    if (type == "") {
        type = "p";
    }

    if (fit == "") {
        fit = "wf";
    }

    Fdisk fdisk;
    return fdisk.administrarParticion(size, path, name, unit, type, fit, del, add, mov);

}

bool Consola::mount(string comando)  {

    vector<string> v = getAtributtes(comando);
    string path = "", name = "";

    for (auto it: v) {
        vector<string> s = split(it);
        cout << it << endl;

        if (s.at(0) == "path") {
            path = s.at(1);
        }

        if (s.at(0) == "name") {
            name = s.at(1);
        }
    }

    if (path.empty() || name.empty()) {
        cout << endl << " *** Parametros obligatorios: path y name *** " << endl << endl;
        return false;
    }


    return montaje.montarParticion(path, name);

}

string Consola::toLowerCase(string text) {
    for_each(text.begin(), text.end(), [](char& c) {
        c = tolower(c);
    });

    return text;
}

vector<string> Consola::getAtributtes(string comando) {

    vector<string> atributos;

    string atributo = "";
    string valor = "";
    for (int i = 0; i < comando.length(); i++) {
        char c = comando[i];
        if (c == '$' || c == '@') {
            for (int j = i + 1; j < comando.length(); j++) {
                c = comando [j];
                if (c == '=') {
                    i = j;
                    break;
                }

                atributo += c;
            }
        }

        if (c == '>') {

            bool comilla = false;

            for (int j = i + 1; j < comando.length(); j++) {
                c = comando [j];

                if (c == '"') {
                    comilla = !comilla;
                    continue;
                }


                if (c == ' ' && !comilla) {
                    i = j;
                    break;
                }

                valor += c;
            }

            atributos.insert(atributos.end(), atributo + "-" + valor);
            atributo = "";
            valor = "";

        }

    }

    return atributos;
}


vector<string> Consola::split(string data) {
    vector<string> s;
    size_t found = data.find("-");

    s.insert(s.end(), data.substr(0, found));
    s.insert(s.end(), data.substr(found + 1));

    return s;
}

bool Consola::isNumber(string text) {

    for (char const &c: text) {
        if (isdigit(c) == 0) {
            return false;
        }
    }

    return true;
}