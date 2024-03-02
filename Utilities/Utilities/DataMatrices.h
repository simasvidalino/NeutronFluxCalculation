#pragma once

#include <iostream>
#include <string>
#include <fstream>         //trabalhar com arquivo
#include <ctime>           //para medir o tempo de execução
#include <cmath>           //para usar a função pow
#include <sstream>         //converter std::string para double, usar ostd::stringstream para o titulo de saída
#include <vector>          //para acessar funcoes como eraser e shrink_to_fit() no vector
#include <iomanip>         //para mostrar maior numero de casas decimais na tela
#include "ProjectStructs.h"
#include "VariablesUsed.h"

#include <memory>
#include <optional>

class BuildMatrices
{

public:
    enum DataOriginType
    {
        eTextFileData,
        eUserInterfaceDataAndTextFile
    };

    BuildMatrices();

    BuildMatrices(dados_entrada *newDatricesDD_Data, ProjectData *newProjectInterfaceData);

    void calculateAbsorptionRate(dados_entrada *data, CalculatedData *output);

    std::unique_ptr<dados_entrada> copyProjectDataToRawPointers(ProjectData &proj);

    void resizeDDOutputValues(CalculatedData &output, dados_entrada &data);

    std::unique_ptr<CalculatedCrossSectionMatrices> calculateCrossSectionMatrices(std::unique_ptr<dados_entrada> data,
                                                                                  std::unique_ptr<CalculatedData> output);

    void run(int buildType, std::string file, dados_entrada &valor);

    void alocate(dados_entrada &valor);

    int getEnergyGroupQtt() const;
    void setEnergyGroupQtt(int newEnergyGroupQtt);

    int getQuadratureOrder() const;
    void setQuadratureOrder(int newQuadratureOrder);

    int getStopOrder() const;
    void setStopOrder(int newStopOrder);

    int getAnisotropyOrder() const;
    void setAnisotropyOrder(int newAnisotropyOrder);

    int getRegionsNumber() const;
    void setRegionsNumber(int newRegionsNumber);

    int getMaterialNumber() const;
    void setMaterialNumber(int newMaterialNumber);

    int getIterationNumber() const;
    void setIterationNumber(int newIterationNumber);

    void setProjValue(dados_entrada *newProjValue);

    dados_entrada *getMatricesDDData() const;
    void setMatricesDDData(dados_entrada *newMatricesDDData);

private:
    void allocateMatrices(dados_entrada &valor);
    void allocateMatricesWithUserInterfaceData(dados_entrada &valor);

    std::vector<std::vector<long double> > calculateAbsorptionCrossSectionMatrix(dados_entrada *data,
                                                                                 std::vector<std::vector<long double>> scatCrossSection);
    std::vector<std::vector<long double> > calculateAverageNeutronFluxPerRegion(dados_entrada *data);

    std::vector<std::vector<long double> > calculateScatteringCrossSectionMatrix(dados_entrada *data);

    void copyVectorToRawPointer(std::vector<double> &mVector, double *&buffer);
    void copyRegionVectorToRowPointers(std::array<RegionData, 10> &regionArray, int regionNumber, dados_entrada* data);

    void buildCrossSectionMatrices(dados_entrada* data);

    void calculateLegendreMatrix(dados_entrada *data);

    void calculateDataMatrices(dados_entrada *data);

    std::vector<double> saveFileDataInVector();

    std::string fileName;
    int energyGroupQtt  = 0;
    int quadratureOrder = 0;
    int stopOrder       = 0;
    int anisotropyOrder = 0;
    int regionsNumber   = 0;
    int materialNumber  = 0;
    int iterationNumber = 0;
    int zoneNumber      = 0;
    int periodicity     = 0;
    int leftBCType      = 0;
    int rightBCType     = 0;

    dados_entrada *matricesDD_Data;
    ProjectData *projectInterfaceData;
};

//void construir_dados(std::string arquivotxt, dados_entrada &valor)
//{
//    double j;
//    std::string k;
//    std::ifstream dados_txt;
//    std::vector <double> vetor;
//    dados_txt.open(arquivotxt.c_str());


//    if(!dados_txt)
//    {
//        std::cout<<"\nFALHA NA ABERTURA DO ARQUIVO...\n";
//    }
//    else
//    {

//        while(dados_txt >> k)
//        {  //lê todo o arquivo
//            //            dados_txt >> k;

//            if(k[0] == '/'){ //pega somente linhas com texto (primeira palavra)
//                dados_txt.ignore(1000,'\n');
//            }else
//            {
//                std::stringstream(k) >> j;
//                vetor.push_back(j);
//            }
//        }
//    }

//    std::cout<<"Vector size "<<vetor.size()<<std::endl;

//    dados_txt.close(); //fecha o arquivo txt

//    /********************************************************************************************************

//                           Distribuição de dados em matrizes

//*********************************************************************************************************/

//    std::cout<<"PAssou do close"<<std::endl;
//    vetor.erase(vetor.begin(),vetor.begin()+9);
//    static int i = 0;

//    //Ordem da Quadratura GL
//    valor.n = vetor[i];
//    i++;

//    //Ordem de parada
//    valor.ordem_parada = vetor[i];
//    i++;

//    //Maximo Numero de Iteracoes
//    valor.iteracao = vetor[i];
//    i++;

//    //Grupos de Energia
//    valor.G = vetor[i];
//    i++;

//    //Grau da Anisotropia do Espalhamento (L)
//    valor.L = vetor[i];
//    i++;

//    //NR e NZ
//    valor.n_R = vetor[i]; i++;

//    valor.n_Z = vetor[i]; i++;

//    std::cout<<"Main data ordem da quadratura "<< valor.n
//            << "\nordem de parada " << valor.ordem_parada
//            <<"\nOrdem de iteracao " << valor.iteracao
//           << "\nGropu de energia "<<valor.G
//           << "\n valor.L "<<valor.L
//           <<"\nNumero de zonas"<<valor.n_Z<<std::endl;


//    //Tamanho de cada Regiao
//    valor.TAM = new double [valor.n_R];
//    for(int j = 0; j<valor.n_R; j++){
//        valor.TAM[j] = vetor[i];
//        i++;
//    }

//    //Nodos por Regiao
//    valor.n_nodos = new short int [valor.n_R];
//    for(int j = 0; j<valor.n_R; j++){
//        valor.n_nodos[j] = vetor[i];
//        i++;
//    }

//    //Periodicidade
//    valor.periodicidade = vetor[i];
//    i++;

//    //Mapeamento
//    valor.Map_R = new short int [valor.n_R];
//    for(int j = 0; j<valor.n_R; j++){
//        valor.Map_R[j] = vetor[i];
//        i++;
//    }

//    vetor.erase(vetor.begin(),vetor.begin()+i);
//    vetor.shrink_to_fit() ;
//    i = 0;

//    /*********************************************************************************************************

//                                    Matrizes

//*********************************************************************************************************/

//    valor.fonte_g = new double*[valor.G];  //Fonte Fisica
//    valor.s_t = new long double*[valor.G];      //Sigma total
//    valor.s_s = new long double***[valor.G];    //Sigma Espalhamento

//    for(int j = 0; j<valor.G; j++){
//        valor.fonte_g[j] = new double [valor.n_R];
//        valor.s_s[j] = new long double**[valor.G];
//        valor.s_t[j] = new long double[valor.n_Z];

//        for(int k = 0; k<valor.G; k++){
//            valor.s_s[j][k] = new long double *[valor.n_Z];

//            for(int l = 0; l<valor.n_Z;l++){
//                valor.s_s[j][k][l] = new long double [valor.L+1];}
//        }
//    }
//    /**********************************************************************************************************/

//    //Sigma total e Sigma de espalhamento
//    for(int h = 0; h<valor.n_Z; h++){
//        for(int j = 0; j<valor.G; j++){
//            valor.s_t[j][h] = vetor[i];
//            i++;
//        }
//        for(int k = 0; k<valor.L+1; k++){
//            for(int m = 0; m<valor.G;m++){
//                for(int n = 0; n<valor.G; n ++){
//                    valor.s_s[m][n][h][k] = vetor[i];
//                    i++;
//                }
//            }}}

//    //Tipo de Condicoes de Contorno (Esq. Dir) (1-Prescrita. 2-Reflexiva)
//    valor.tipo_ce = vetor[i];i++;                valor.tipo_cd = vetor[i]; i++;

//    //Valor da condicao de contorno prescrita (Esq. Dir)
//    valor.cceg = new double [valor.G];
//    valor.ccdg = new double [valor.G];

//    for(int j = 0; j<valor.G; j++){
//        valor.cceg[j] = vetor[i];
//        i++;
//        valor.ccdg[j] = vetor[i];
//        i++;
//    }

//    //Fonte Fisica
//    for(int j = 0; j<valor.G; j++){
//        for(int k = 0; k<valor.n_R; k++){
//            valor.fonte_g[j][k] = vetor[i];
//            i++;
//        }
//    }
//    vetor.clear();
//    vetor.shrink_to_fit() ;

//    /*********************************************************************************************************

//                                           Dados calculados

//*********************************************************************************************************/
//    valor.CONTX = new int[valor.n_R]; //nodos por regiao acumulados
//    valor.PASSO = new long double [valor.n_R]; //modulo entre nodos
//    valor.TAM_TOTAL = 0;  //comprimento total de x
//    valor.NODOSX = 0;  // numero total de nodos por regiao

//    for(int j = 0; j<valor.n_R; j++){
//        valor.CONTX[j] = valor.NODOSX+valor.n_nodos[j];
//        valor.PASSO[j] = valor.TAM[j]/valor.n_nodos[j];
//        valor.NODOSX = valor.CONTX[j];
//    }
//    valor.w = new double [valor.n];
//    valor.mi = new double [valor.n];

//    //fluxo angular e fluxo escalar

//    valor.FLUXO_ANGULAR = new long double**[valor.G];
//    valor.smgi = new long double**[valor.G];
//    valor.FLUXO_ESCALAR = new long double*[valor.G];

//    for(int g = 0; g < valor.G;g++){
//        valor.FLUXO_ANGULAR[g] = new long double*[(valor.NODOSX)+1];
//        valor.smgi[g] = new long double*[valor.NODOSX];
//        valor.FLUXO_ESCALAR[g] = new long double[(valor.NODOSX)+1];

//        for(int o = 0; o <= valor.NODOSX;o++){
//            valor.FLUXO_ANGULAR[g][o] = new long double[valor.n];
//        }
//        for(int o = 0; o<valor.NODOSX;o++){
//            valor.smgi[g][o] = new long double[valor.n];
//        }
//    }


//    //Matriz com os polinômios de Legendre
//    legendre_set ( valor.n,valor.mi,valor.w);
//    valor.Mat_Legendre = new double *[valor.n];
//    valor.legendre_n = new double [valor.L+1];
//    for(int n = 0; n<valor.n; n ++){
//        valor.Mat_Legendre[n] = new double [valor.L + 1];
//    }

//    setlocale(LC_ALL,"portuguese");

//    for(int n = 0; n<valor.n; n ++){
//        Legendre::Pn(valor.L,valor.mi[n],valor.legendre_n);
//        for(int l = 0; l<valor.L+1;l++){
//            valor.Mat_Legendre[n][l] = valor.legendre_n[l];
//        }
//    }

//    i = 0; //@TBD bug when use this function more than once
//}
