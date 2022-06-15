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
    void reporteMBR(MBR discoEditar, string path);
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
        reporteMBR(discoEditar, path);
    } else if (name == "disk") {
        reporteDisco(discoEditar, path, pathD);
    }

    return true;
}

void Rep::reporteMBR(MBR discoEditar, string path) {
    string codigoInterno = "";
    string size = to_string(discoEditar.tamano);
    string date(discoEditar.fecha_creacion);
    string firma = to_string(discoEditar.disk_signature);
    string fit = "";

    codigoInterno = "<TR>\n"
                    "<TD><B>MBR_Tamanio</B></TD>\n"
                    "<TD>" +
                    size + "</TD>\n"
                           "</TR>\n"
                           "<TR>\n"
                           "<TD><B>MBR_Fecha_Creacion</B></TD>\n"
                           "<TD>" +
                    date + "</TD>\n"
                           "</TR>\n"
                           "<TR>\n"
                           "<TD><B>MBR_Disk_Signature</B></TD>\n"
                           "<TD>" +
                    firma + "</TD>\n"
                            "</TR>\n"
                            "<TR>\n"
                            "<TD><B>MBR_Disk_Fit</B></TD>\n"
                            "<TD>" +
                    fit + "</TD>\n"
                          "</TR>\n";

    string codigoParticiones = "";
    for (int i = 0; i < 4; i++) {
        if (discoEditar.particiones[i].status == '1')
        {
            string size = to_string(discoEditar.particiones[i].size);
            string name(discoEditar.particiones[i].name);
            string Estado = "";
            Estado.push_back(discoEditar.particiones[i].status);
            string fit = "";
            fit.push_back(discoEditar.particiones[i].fit);
            string type = "";
            type.push_back(discoEditar.particiones[i].type);
            string part_start = to_string(discoEditar.particiones[i].start);
            string indice = to_string((i + 1));

            codigoParticiones = codigoParticiones +
                                "<TR>\n"
                                "<TD><B>part_Estado_" +
                                indice + "</B></TD>\n"
                                         "<TD>" +
                                Estado + "</TD>\n"
                                         "</TR>\n"
                                         "<TR>\n"
                                         "<TD><B>part_Type_" +
                                indice + "</B></TD>\n"
                                         "<TD>" +
                                type + "</TD>\n"
                                       "</TR>\n"
                                       "<TR>\n"
                                       "<TD><B>part_Fit_" +
                                indice + "</B></TD>\n"
                                         "<TD>" +
                                fit + "</TD>\n"
                                      "</TR>\n"
                                      "<TR>\n"
                                      "<TD><B>part_Start_" +
                                indice + "</B></TD>\n"
                                         "<TD>" +
                                part_start + "</TD>\n"
                                             "</TR>\n"
                                             "<TR>\n"
                                             "<TD><B>part_Name_" +
                                indice + "</B></TD>\n"
                                         "<TD>" +
                                name + "</TD>\n"
                                       "</TR>\n";
        }

    }

    string codigo = "digraph  {\n"
                    "graph[ratio = fill];\n"
                    " node [label=\"\n\", fontsize=15, shape=plaintext];\n"
                    "graph [bb=\"0,0,352,154\"];\n"
                    "arset [label=<\n"
                    " <TABLE ALIGN=\"LEFT\">\n"
                    "<TR>\n"
                    " <TD><B>Nombre</B></TD>\n"
                    "<TD><B> Valor </B></TD>\n"
                    "</TR>\n"
                    + codigoInterno +
                    codigoParticiones +
                    "</TABLE>\n"
                    ">, ];\n"
                    "}";

    string path1 = path;
    string pathPng = path1.substr(0, path1.size() - 4);
    pathPng = pathPng + ".png";

    FILE *validar = fopen(path1.c_str(), "r");
    if (validar != NULL) {
        ofstream outfile(path1);
        outfile << codigo.c_str() << endl;
        outfile.close();
        string comando = "dot -Tpng " + path1 + " -o " + pathPng;

        system(comando.c_str());
        fclose(validar);
    } else {
        string comando1 = "mkdir -p \"" + path + "\"";
        string comando2 = "rmdir \"" + path + "\"";
        system(comando1.c_str());
        system(comando2.c_str());

        ofstream outfile(path1);
        outfile << codigo.c_str() << endl;
        outfile.close();
        string comando = "dot -Tpng " + path1 + " -o " + pathPng;
        system(comando.c_str());
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
