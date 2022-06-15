#include "Mount.h"
#include "Structs.h"
#include <cstring>


Mount::Mount() {}

bool Mount::montarParticion(string path, string name) {
    int inicioExtendida = 0;
    FILE *disco = fopen(path.c_str(), "rb+");

    if (disco == NULL) {
        cout << endl << " *** Disco no existente, disco path *** " << endl << endl;
        return false;
    } else {
        MBR discoParticion;
        bool existeParticion = false;
        rewind(disco);
        fread(&discoParticion, sizeof(MBR), 1, disco);

        for(int i = 0 ; i<4; i++){

            if(discoParticion.particiones[i].type == 'e'){
                inicioExtendida = discoParticion.particiones[i].start;
            }

            if(discoParticion.particiones[i].name == name){
                existeParticion = true;
            }
        }

        //VALIDAR SI ES UNA PARTICION LOGICA.
        if(!existeParticion && inicioExtendida !=0){
            EBR logicaM;
            fseek(disco, inicioExtendida, SEEK_SET);
            fread(&logicaM, sizeof(EBR), 1, disco);
            while(logicaM.status != '0' && logicaM.next != -1){
                if(logicaM.name == name){
                    existeParticion = true;
                }
                fseek(disco, logicaM.next, SEEK_SET);
                fread(&logicaM, sizeof(EBR), 1, disco);
            }
        }


        if(!existeParticion){
            cout << endl << " *** Nombre de particion no existente ***" << endl << endl;
            fclose(disco);
            return false;
        }
        fclose(disco);
    }

    bool existePath = false;
    int posicionPath = 0;
    for (int i = 0; i < 26; i++) {
        if (discos[i].path == path) {
            existePath = true;
            posicionPath = i;
        }
    }

    //SI NO EXISTE EL DISCO MONTADO, SE MONTA.
    if (!existePath) {
        for (int j = 0; j < 99; j++) {
            if (discos[j].estado == 0) {
                discos[j].estado = 1;
                discos[j].letra = j + 97;
                strcpy(discos[j].path, path.c_str());
                posicionPath = j;
                existePath = true;
                break;
            }
        }
    }

    //DISCO YA MONTADO, SOLO SE AGREGA LA PARTICION.
    if (existePath) {
        for (int j = 0; j < 99; j++) {
            if (discos[posicionPath].particiones[j].estado == 0)
            {
                discos[posicionPath].particiones[j].estado = 1;
                discos[posicionPath].particiones[j].numero = j + 1;
                strcpy(discos[posicionPath].particiones[j].name, name.c_str());
                cout << endl << " *** Particion montada exitosamente, id: vd " << discos[posicionPath].letra << discos[posicionPath].particiones[j].numero << " ***" << endl << endl;
                break;
            }
        }
    }


    //leerMontaje();
    return true;
}

bool Mount::desmontarParticion(string id) {
    char letra = id.c_str()[2];
    char numero = id.c_str()[3];
    bool desmontado = false;
    int numeroInt = (int) numero - 48;

    for (int i = 0; i < 26; i++) {
        if (discos[i].letra == letra) {
            for (int j = 0; j < 99; j++) {
                if (discos[i].particiones[j].numero == numeroInt) {
                    cout << endl << " *** Particion desmontada: vd"  << discos[i].letra << discos[i].particiones[j].numero << " ***" << endl << endl;
                    discos[i].particiones[j].estado = 0;
                    desmontado = true;
                }
            }
        }
    }

    if(!desmontado){
        cout << endl << " *** Error: id no existe, no se desmonto la particion ***" << endl <<endl;
        return false;
    }
    //leerMontaje();
    return true;
}

void Mount::leerMontaje() {
    cout << "<<<-------------------------- MONTAJES ---------------------->>>" << endl;
    for (int i = 0; i < 26; i++) {
        for (int j = 0; j < 99; j++) {
            if (discos[i].particiones[j].estado == 1) {
                cout << "vd" << discos[i].letra << discos[i].particiones[j].numero << " - " << discos[i].path << endl;
            }
        }
    }
    cout << endl;
}