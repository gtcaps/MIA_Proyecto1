#ifndef VACASJUNIO_MIA_PROYECTO1_REP_H
#define VACASJUNIO_MIA_PROYECTO1_REP_H

#include <iostream>
#include <string>
#include <cstring>
#include <fstream>
#include "Mount.h"
#include "Structs.h"

using namespace std;

class Rep {
public:
    Rep();
    bool crearReporte(string path, string name, string id, Mount montaje);
    void reporteDisco(MBR discoEditar, string pathImagen, string pathDisco);
    void reporteMBR(MBR discoEditar, string pathImagen, string pathDisco);
    void getDatosID(string id, Mount montaje, string * path, int *inicioParticion, int * sizePart, string * nombrePart, int * error);
};

Rep::Rep() {}

void Rep::getDatosID(string id, Mount montaje, string * path, int *inicioParticion, int * sizePart, string *nombrePart, int * error){
    //Obtener particion y su size.
    char letra = id.c_str()[2];
    char numero = id.c_str()[3];
    bool existePath = false;
    int numeroInt = (int)numero - 48;
    string nombreParticion = "";

    char pathDisco[100] = "";

    for (int i = 0; i < 26; i++) {
        if (montaje.discos[i].letra == letra && montaje.discos[i].particiones[numeroInt-1].estado == 1) {
            strcpy(pathDisco, montaje.discos[i].path);
            nombreParticion = montaje.discos[i].particiones[numeroInt-1].name;
            existePath = true;
            break;
        }
    }

    if (!existePath) {
        cout << endl << " *** Error: id no existe, path no existente ***" << endl << endl;
        *error = 1;
        return;
    }

    int sizeParticion = -1;
    int startParticion = -1;

    FILE *bfile1 = fopen(pathDisco, "rb+");
    if(bfile1 != NULL){
        rewind(bfile1);
        MBR discoLeer;
        fread(&discoLeer, sizeof(MBR), 1, bfile1);
        for(int i = 0; i < 4; i++) {
            if(discoLeer.particiones[i].name == nombreParticion){
                sizeParticion = discoLeer.particiones[i].size;
                startParticion = discoLeer.particiones[i].start;
            }
        }

        fclose(bfile1);
    }else{
        cout << endl << " *** Error: Path no existente *** " << endl << endl;
        *error = 1;
        return;
    }

    *nombrePart = string(nombreParticion);
    *error = 0;
    *path = string(pathDisco);
    *inicioParticion = startParticion;
    *sizePart = sizeParticion;
}


bool Rep::crearReporte(string path, string name, string id, Mount montaje) {


    string pathD= "";
    int sizeParticion;
    int startParticion;
    int error = 0;
    string nombreParticion;

    getDatosID(id, montaje, &pathD, &startParticion, &sizeParticion, &nombreParticion, &error);


    if(error == 1){
        return false;
    }

    MBR discoEditar;
    FILE *bfile2 = fopen(pathD.c_str(), "rb+");
    if (bfile2 != NULL) {
        rewind(bfile2);
        fread(&discoEditar, sizeof(discoEditar), 1, bfile2);
    } else {
        cout << endl <<  " *** Error. Path no existente, disco no existente *** " << endl << endl;
        return false;
    }
    fclose(bfile2);

    if (name == "mbr") {
        reporteMBR(discoEditar, path, pathD);
    } else if (name == "disk") {
        reporteDisco(discoEditar, path, pathD);
    }

    return true;
}

void Rep::reporteMBR(MBR discoEditar, string pathImagen, string pathDisco) {

    string hexColores[] = {"#88e3a0", "#e39688", "#e3c588", "#88e3d2", "#b488e3"};
    string codigoParticiones = "";
    string codigoExtendidas = "";
    int contExtendidas = 0;

    FILE *bfilel = fopen(pathDisco.c_str(), "rb+");
    if (bfilel != NULL) {
        rewind(bfilel);
        fread(&discoEditar, sizeof(MBR), 1, bfilel);
        for (int i = 0; i < 4; i++) {
            if(discoEditar.particiones[i].status != '0'){

                codigoParticiones += "       <tr>"
                                     "           <td bgcolor='" + hexColores[i] + "'><b>part" + to_string(i) +"_status</b></td>\n"
                                     "           <td bgcolor='" + hexColores[i] +"'>" + discoEditar.particiones[i].status + "</td>\n"
                                     "      </tr>\n"
                                     "       <tr>\n"
                                     "           <td bgcolor='" + hexColores[i] + "'><b>part" + to_string(i) +"_name</b></td>\n"
                                     "           <td bgcolor='" + hexColores[i] +"'>" + discoEditar.particiones[i].name + "</td>\n"
                                     "      </tr>\n"
                                     "       <tr>\n"
                                     "           <td bgcolor='" + hexColores[i] + "'><b>part" + to_string(i) +"_fit</b></td>\n"
                                     "           <td bgcolor='" + hexColores[i] +"'>" + discoEditar.particiones[i].fit + "</td>\n"
                                     "      </tr>\n"
                                     "       <tr>\n"
                                     "           <td bgcolor='" + hexColores[i] + "'><b>part" + to_string(i) +"_start</b></td>\n"
                                     "           <td bgcolor='" + hexColores[i] +"'>" + to_string(discoEditar.particiones[i].start) + "</td>\n"
                                     "      </tr>"
                                     "       <tr>\n"
                                     "           <td bgcolor='" + hexColores[i] + "'><b>part" + to_string(i) +"_size</b></td>\n"
                                     "           <td bgcolor='" + hexColores[i] +"'>" + to_string(discoEditar.particiones[i].size) + "</td>\n"
                                     "      </tr>"
                                     "       <tr>\n"
                                     "           <td bgcolor='" + hexColores[i] + "'><b>part" + to_string(i) +"_type</b></td>\n"
                                     "           <td bgcolor='" + hexColores[i] +"'>" + discoEditar.particiones[i].type + "</td>\n"
                                     "      </tr>\n";

                if (discoEditar.particiones[i].type == 'e') {
                    EBR logicaR;
                    fseek(bfilel, discoEditar.particiones[i].start, SEEK_SET);
                    fread(&logicaR, sizeof(EBR), 1, bfilel);

                    while(logicaR.next != -1){
                        codigoExtendidas += "ext" + to_string(contExtendidas++) +" [\n"
                                        "   shape=plaintext\n"
                                        "   label=<\n"
                                        "   <table border='1' cellborder='1'>\n"
                                        "       <tr>\n"
                                        "           <td bgcolor='" + hexColores[4] +"'><b>part_name</b></td>\n"
                                        "           <td bgcolor='" + hexColores[4] +"'>" + logicaR.name +"</td>\n"
                                        "       </tr>"
                                        "       <tr>\n"
                                        "           <td bgcolor='" + hexColores[4] +"'><b>part_fit</b></td>\n"
                                        "           <td bgcolor='" + hexColores[4] +"'>" + logicaR.fit +"</td>\n"
                                        "       </tr>"
                                        "       <tr>\n"
                                        "           <td bgcolor='" + hexColores[4] +"'><b>part_start</b></td>\n"
                                        "           <td bgcolor='" + hexColores[4] +"'>" + to_string(logicaR.part_start) +"</td>\n"
                                        "       </tr>"
                                        "       <tr>\n"
                                        "           <td bgcolor='" + hexColores[4] +"'><b>part_size</b></td>\n"
                                        "           <td bgcolor='" + hexColores[4] +"'>" + to_string(logicaR.size) +"</td>\n"
                                        "       </tr>"
                                        "       <tr>\n"
                                        "           <td bgcolor='" + hexColores[4] +"'><b>part_next</b></td>\n"
                                        "           <td bgcolor='" + hexColores[4] +"'>" + to_string(logicaR.next) +"</td>\n"
                                        "       </tr>"
                                        "       <tr>\n"
                                        "           <td bgcolor='" + hexColores[4] +"'><b>part_status</b></td>\n"
                                        "           <td bgcolor='" + hexColores[4] +"'>" + logicaR.status +"</td>\n"
                                        "       </tr>"
                                        "   </table>"
                                        ">]\n";

                        fseek(bfilel, logicaR.next, SEEK_SET);
                        fread(&logicaR, sizeof(EBR), 1, bfilel);
                    }


                    if (contExtendidas > 0) {
                        string rankSame = "";
                        for (int i = 0; i < contExtendidas; i++) {
                            if (i < contExtendidas - 1) {
                                codigoExtendidas += "   ext" + to_string(i) + " -> ext" + to_string(i + 1) + ";\n";
                            }
                            rankSame += "ext" + to_string(i) + " ";
                        }
                        rankSame = "    {rank=same; " + rankSame + "}\n";
                        codigoExtendidas += rankSame;
                    }

                }
            }
        }
        fclose(bfilel);

        // CODIGO MBR
        string codigoInterno = "";
        string size = to_string(discoEditar.tamano);
        string date(discoEditar.fecha_creacion);
        string firma = to_string(discoEditar.disk_signature);

        string codigoMBR = "       <tr>\n"
                           "           <td><b>MBR_Tamanio</b></td>\n"
                           "           <td>" + size + "</td>\n"
                           "       </tr>\n"
                           "       <tr>\n"
                           "           <td><b>MBR_Fecha_Creacion</b></td>\n"
                           "           <td>" + date + "</td>\n"
                           "       </tr>\n"
                           "       <tr>\n"
                           "           <td><b>MBR_Disk_Signature</b></td>\n"
                           "           <td>" + firma + "</td>\n"
                           "       </tr>\n";

        string codigo = "digraph  {\n"
                        "parent [\n"
                        "   shape=plaintext\n"
                        "   label=<\n"
                        "   <table border='1' cellborder='1'>\n"
                        "       <tr>\n"
                        "           <td><b>Propiedad</b></td>\n"
                        "           <td><b>Valor</b></td>\n"
                        "       </tr>"
                        +       codigoMBR + "\n"
                        +       codigoParticiones +
                        "   </table>"
                        ">];"
                        + codigoExtendidas +
                        "}\n";
        string pathDot = pathImagen.substr(0, pathImagen.size() - 4) + ".dot";
        size_t lastSlash = pathImagen.find_last_of('/');
        string folderPathOfImage = pathImagen.substr(0, lastSlash);
        string imageExtension = pathImagen.substr(pathImagen.size() - 3);


        ofstream dotFile;
        dotFile.open(pathDot);

        if (dotFile.fail()) {
            string comando1 = "mkdir -p \"" + folderPathOfImage + "\"";
            system(comando1.c_str());
        }
        dotFile.close();

        dotFile.open(pathDot);
        dotFile << codigo;
        dotFile.close();

        string comando2 = "dot -T" + imageExtension + " \"" + pathDot + "\" -o \"" + pathImagen + "\"";
        system(comando2.c_str());

        string comandoOpen = "xdg-open \"" + pathImagen + "\"";
        system(comandoOpen.c_str());

        cout << endl << " *** Correcto: Se genero el reporte tipo MBR en - " << pathDot << endl << endl;

    } else {
        cout << endl << " *** No se pueden mostrar las particiones por path incorrecto *** " << endl << endl;
    }

}

void Rep::reporteDisco(MBR discoEditar, string pathImagen, string pathDisco) {
    string codigoInterno = "";
    string size = to_string(discoEditar.tamano);
    string date(discoEditar.fecha_creacion);
    string firma = to_string(discoEditar.disk_signature);
    string fit = "";
    int acumulado = 0;

    string codigoParticiones = "";
    string codigoLogicas = "";

    for (int i = 0; i < 4; i++) {
        if (discoEditar.particiones[i].status == '1') {
            string name(discoEditar.particiones[i].name);
            string Estado = "";
            Estado.push_back(discoEditar.particiones[i].status);
            string type = "";
            type.push_back(discoEditar.particiones[i].type);
            int porcentaje = ((discoEditar.particiones[i].size * 100) / discoEditar.tamano);
            acumulado = acumulado + porcentaje;
            string porcentajeString = to_string(porcentaje);

            if (type == "p") {
                codigoParticiones = codigoParticiones + "           <td rowspan='2'>" + name + "<br/>" + type + "<br/>" + porcentajeString  + "%</td>\n";
            } else if (type == "e") {
                int inicioLogicas = discoEditar.particiones[i].start;
                int contLogicas = 0;
                EBR logicaI;
                FILE *bfilel = fopen(pathDisco.c_str(), "rb+");
                fseek(bfilel, inicioLogicas, SEEK_SET);
                fread(&logicaI, sizeof(EBR), 1, bfilel);
                while(logicaI.next != -1){
                    if(logicaI.status != '0'){
                        codigoLogicas += "                          <td border='1'>EBR</td>\n";
                        codigoLogicas += "                          <td border='1'>Logica<br/>" + (string) logicaI.name  + "</td>\n";
                        contLogicas++;
                    }
                    fseek(bfilel, logicaI.next, SEEK_SET);
                    fread(&logicaI, sizeof(EBR), 1, bfilel);
                }
                fclose(bfilel);

                codigoParticiones += "              <td>\n"
                                     "                  <table border='0'>\n"
                                     "                      <tr><td border='1' colspan='"+ to_string(contLogicas * 2) + "' >(Extendida)<br/>" + name + "<br/>" +porcentajeString +"%</td></tr>\n"
                                     "                      <tr>\n"
                                     + codigoLogicas +
                                     "                      </tr>\n"
                                     "                  </table>\n"
                                     "              </td>\n";
            }
        }
    }

    codigoLogicas = "";
    int libre = 100 - acumulado;
    string libreString = to_string(libre);
    if(acumulado<100){
        codigoParticiones = codigoParticiones + "           <td rowspan='2'>Libre<br/>" + libreString + "%</td>\n";

    }

    string codigo = "digraph  {\n"
                    "parent [\n"
                    "   shape=plaintext\n"
                    "   label=<\n"
                    "   <table border='1' cellborder='1'>\n"
                    "       <tr>\n"
                    "           <td rowspan='2'>MBR</td>\n"
                    + codigoParticiones +
                    "       </tr>\n"
                    "   </table>"
                    ">];"
                    "}\n";

    string pathDot = pathImagen.substr(0, pathImagen.size() - 4) + ".dot";
    size_t lastSlash = pathImagen.find_last_of('/');
    string folderPathOfImage = pathImagen.substr(0, lastSlash);
    string imageExtension = pathImagen.substr(pathImagen.size() - 3);

    // cout << codigo << endl;

    ofstream dotFile;
    dotFile.open(pathDot);

    if (dotFile.fail()) {
        string comando1 = "mkdir -p \"" + folderPathOfImage + "\"";
        //cout << endl << comando1 << endl;
        system(comando1.c_str());
    }
    dotFile.close();

    dotFile.open(pathDot);
    dotFile << codigo;
    dotFile.close();

    string comando2 = "dot -T" + imageExtension + " \"" + pathDot + "\" -o \"" + pathImagen + "\"";
    system(comando2.c_str());

    string comandoOpen = "xdg-open \"" + pathImagen + "\"";
    system(comandoOpen.c_str());

    cout << endl << " *** Correcto: Se genero el reporte tipo Disk en - " << pathDot << endl << endl;
}

#endif //VACASJUNIO_MIA_PROYECTO1_REP_H
