#include "DataMatrices.h"
#include "GausLegendreQuadrature.h"
#include "LegendrePolynomial.h"

#include <algorithm>
#include <filesystem>
#include <numeric>      // std::accumulate
#include <stdexcept>

BuildMatrices::BuildMatrices()
{

}

BuildMatrices::BuildMatrices(dados_entrada *newDatricesDD_Data,
                             ProjectData *newProjectInterfaceData)
    : matricesDD_Data(newDatricesDD_Data),
      projectInterfaceData(newProjectInterfaceData)
{
};

std::unique_ptr<dados_entrada> BuildMatrices::copyProjectDataToRawPointers(ProjectData &proj)
{
    auto data = std::make_unique<dados_entrada>();
    fileName  = proj.scateringFilePath;
    std::filesystem::directory_entry entry{fileName};

    if ( (true == fileName.empty() )
        || (false == entry.exists()) )
    {
        throw std::invalid_argument("Error: Material Data File issue. \nYou need to set a Cross Section File.");
    }

    data->G   = proj.energyGroup;
    data->L   = proj.legendreOrder;
    data->n   = proj.quadratureOrder;
    data->n_R = proj.regionNumber;
    data->n_Z = proj.zoneNumber;

    data->tipo_ce = static_cast<int>(proj.leftBoundaryConditionsType);
    data->tipo_cd = static_cast<int>(proj.rightBoundaryConditionsType);
    data->ordem_parada  = proj.stopOrder;
    data->periodicidade = proj.periodicity;
    data->iteracao      = proj.maximumIterationsNumber;

    std::vector<double> bc;
    for (int iIndex = 0; iIndex < data->G; ++iIndex)
        bc.push_back(0.0);

    if (proj.bcLeft.has_value())
        copyVectorToRawPointer(proj.bcLeft.value(), data->cceg);
    else
    {
        copyVectorToRawPointer(bc, data->cceg);
    }

    if (proj.bcRight.has_value())
        copyVectorToRawPointer(proj.bcRight.value(), data->ccdg);
    else
    {
        copyVectorToRawPointer(bc, data->ccdg);
    }

    copyRegionVectorToRowPointers(proj.regionArray, proj.regionNumber, data.get());

    buildCrossSectionMatrices(data.get());

    return data;
}

CalculatedCrossSectionMatrices BuildMatrices::calculateCrossSectionMatrices(dados_entrada *data)
{
    CalculatedCrossSectionMatrices out;

    auto sigmaScattering = calculateScatteringCrossSectionMatrix(data);
    out.scatteringCrossSection.swap(sigmaScattering);

    auto sigmaAbsorption = calculateAbsorptionCrossSectionMatrix(data, out.scatteringCrossSection);
    out.absorptionCrossSection.swap(sigmaAbsorption);

    return out;
}

void BuildMatrices::calculateAbsorptionRate(dados_entrada *data,
                                            CalculatedData *output)
{
    std::vector<std::vector<long double>> absorptionRate;

    if (output->matrices.absorptionCrossSection.empty())
        throw std::invalid_argument("Error: Absorption Cross Section Matrix is empty");

    if (output->averageNeutronFluxPerRegion.empty())
        throw std::invalid_argument("Error: Average Neutron Flux Per Region Matrix is empty");

    for (int rIndex = 0; rIndex < data->n_R; ++rIndex)
    {
        std::vector<long double> absorptionRateByGroup;
        for (int gIndex = 0; gIndex < data->G; ++gIndex)
        {
            auto zIndex = data->Map_R[rIndex] - 1;

            if (zIndex < output->matrices.absorptionCrossSection.size())
            {
                auto sigmaAbsor = output->matrices.absorptionCrossSection[zIndex][gIndex] *
                        output->averageNeutronFluxPerRegion[zIndex][gIndex];

                absorptionRateByGroup.push_back(sigmaAbsor);
            }
        }

        absorptionRate.push_back(absorptionRateByGroup);
    }

    output->absorptionRate.swap(absorptionRate);
}


std::vector<std::vector<long double>> BuildMatrices::calculateAverageNeutronFluxPerRegion(dados_entrada *data)
{
    std::vector<std::vector<long double>> averageNeutronFlux;
    long double sum = 0.0;
    int nodeLeft = 0;
    int numberOfGroups = data->G;

    for (int regionIndex = 0; regionIndex < data->n_R; ++regionIndex)
    {
        std::vector<long double> averageNeutronFluxByGroup;
        int nodeQttPerRegion  = data->n_nodos[regionIndex];
        double regionSize = data->TAM[regionIndex];
        int nodeRight = nodeQttPerRegion + nodeLeft;

        for (int gIndex = 0; gIndex < numberOfGroups; ++gIndex)
        {
            sum = 0;

            for (int nodeIndex = nodeLeft; nodeIndex < nodeRight; ++nodeIndex)
            {
                auto value = data->FLUXO_ESCALAR[gIndex][nodeIndex];
                sum += value;
            }

            auto average = (sum * regionSize );
            averageNeutronFluxByGroup.push_back(average);
        }

        nodeLeft = nodeRight;
        averageNeutronFlux.push_back(averageNeutronFluxByGroup);
    }

    return averageNeutronFlux;
}

std::vector<std::vector<long double> > BuildMatrices::calculateScatteringCrossSectionMatrix(dados_entrada *data)
{
    std::vector<std::vector<long double>> sigmaScattering;

    //The total scattering cross section for a group
    //g is the sum of all scattering cross sections for that group,
    //regardless of target group g line and order l. However, only isotropic scattering is considered,
    //which is the largest contribution to scattering in many applications, i.e., l=0.
    for (int zIndex = 0; zIndex < data->n_Z; ++zIndex)
    {
        std::vector<long double> scattering_g;

        for (int gIndex = 0; gIndex < data->G; ++gIndex)
        {
            std::vector<long double> scatggline; //l = 0
            for (int gLineIndex = 0; gLineIndex < data->G; ++gLineIndex)
            {
                double value = data->s_s[gIndex][gLineIndex][zIndex][0];
                std::cout<<"g"<<gIndex<<"g'"<<gLineIndex<<" = "<<value<<std::endl;
                scatggline.push_back(value);
            }

            auto sum = std::accumulate(scatggline.begin(), scatggline.end(), 0.0L);
            std::cout<<"sum so"<<gIndex<<" "<<sum<<std::endl;
            scattering_g.push_back(sum);

            std::for_each(scattering_g.begin(), scattering_g.end(), [&](double value){
                std::cout<<value<<" ";
            });
        }

        sigmaScattering.push_back(scattering_g);
    }

    return sigmaScattering;
}

void BuildMatrices::run(int buildType, std::string file, dados_entrada &valor)
{
    fileName = file;

    switch (buildType)
    {
    case DataOriginType::eUserInterfaceDataAndTextFile:
        allocateMatricesWithUserInterfaceData(valor);
        break;

    case DataOriginType::eTextFileData:
        allocateMatrices(valor);
        break;

    default:
        break;
    }
}

void BuildMatrices::alocate(dados_entrada &valor)
{
    //    //Ordem da Quadratura GL
    //    valor.n = quadratureOrder;

    //    //Ordem de parada
    //    valor.ordem_parada = stopOrder;

    //    //Maximo Numero de Iteracoes
    //    valor.iteracao = iterationNumber;

    //    //Grupos de Energia
    //    valor.G = energyGroupQtt;

    //    //Grau da Anisotropia do Espalhamento (L)
    //    valor.L = anisotropyOrder;

    //    //NR e NZ
    //    valor.n_R = regionsNumber;

    //    valor.n_Z = zoneNumber;

    //    //Periodicidade
    //    valor.periodicidade = periodicity;

    //    //Tipo de Condicoes de Contorno (Esq. Dir) (1-Prescrita. 2-Reflexiva)
    //    valor.tipo_ce = leftBCType;
    //    valor.tipo_cd = rightBCType;

    //    std::cout<<"Main data ordem da quadratura "<< valor.n
    //            << "\nordem de parada " << valor.ordem_parada
    //            <<"\nOrdem de iteracao " << valor.iteracao
    //           << "\nGropu de energia "<<valor.G
    //           << "\n valor.L "<<valor.L
    //           <<"\nNumero de zonas"<<valor.n_Z<<std::endl;

    //    //Tamanho de cada Regiao
    //    valor.TAM = new double [valor.n_R];
    //    for(int j = 0; j<valor.n_R; j++)
    //    {
    //        valor.TAM[j] = 0;
    //    }

    //    //Nodos por Regiao
    //    valor.n_nodos = new short int [valor.n_R];
    //    for(int j = 0; j<valor.n_R; j++)
    //    {
    //        valor.n_nodos[j] = 0;
    //    }

    //    //Mapeamento
    //    valor.Map_R = new short int [valor.n_R];
    //    for(int j = 0; j<valor.n_R; j++)
    //    {
    //        valor.Map_R[j] = 0;
    //    }

    //    /*********************************************************************************************************

    //                                    Matrizes

    //*********************************************************************************************************/

    //    valor.fonte_g = new double*[valor.G];        //Fonte Fisica
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
    //    for(int h = 0; h<valor.n_Z; h++)
    //    {
    //        for(int j = 0; j<valor.G; j++)
    //        {
    //            valor.s_t[j][h] = 0;
    //        }
    //        for(int k = 0; k<valor.L+1; k++)
    //        {
    //            for(int m = 0; m<valor.G;m++)
    //            {
    //                for(int n = 0; n<valor.G; n ++)
    //                {
    //                    valor.s_s[m][n][h][k] = 0;
    //                }
    //            }}}

    //    //Valor da condicao de contorno prescrita (Esq. Dir)
    //    valor.cceg = new double [valor.G];
    //    valor.ccdg = new double [valor.G];

    //    for(int j = 0; j < valor.G; j++){
    //        valor.cceg[j] = 0;
    //        valor.ccdg[j] = 0;
    //    }

    //    //Fonte Fisica
    //    for(int j = 0; j < valor.G; j++)
    //    {
    //        for(int k = 0; k<valor.n_R; k++)
    //        {
    //            valor.fonte_g[j][k] = 0;

    //        }
    //    }

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

    //    for(int g = 0; g < valor.G; g++)
    //    {
    //        valor.FLUXO_ANGULAR[g] = new long double*[(valor.NODOSX) + 1];
    //        valor.smgi[g] = new long double*[valor.NODOSX];
    //        valor.FLUXO_ESCALAR[g] = new long double[(valor.NODOSX) + 1];

    //        for(int o = 0; o <= valor.NODOSX; o++)
    //        {
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
    //    for(int n = 0; n < valor.n; n ++)
    //    {
    //        valor.Mat_Legendre[n] = new double [valor.L + 1];
    //    }

    //    setlocale(LC_ALL,"portuguese");

    //    for(int n = 0; n < valor.n; n ++)
    //    {
    //        Legendre::Pn(valor.L, valor.mi[n], valor.legendre_n);
    //        for(int l = 0; l < valor.L+1;l++)
    //        {
    //            valor.Mat_Legendre[n][l] = valor.legendre_n[l];
    //        }
    //    }
}

void BuildMatrices::allocateMatrices(dados_entrada &valor)
{
    auto vector = saveFileDataInVector();

    if (vector.size() == 0)
        throw std::invalid_argument("Error: There is no data to build matrices");

    /********************************************************************************************************

                           Distribuição de dados em matrizes

*********************************************************************************************************/

    std::cout<<"PAssou do close"<<std::endl;
    vector.erase(vector.begin(),vector.begin() + 9);
    int i = 0;

    //Ordem da Quadratura GL
    valor.n = vector[i];
    i++;

    //Ordem de parada
    valor.ordem_parada = vector[i];
    i++;

    //Maximo Numero de Iteracoes
    valor.iteracao = vector[i];
    i++;

    //Grupos de Energia
    valor.G = vector[i];
    i++;

    //Grau da Anisotropia do Espalhamento (L)
    valor.L = vector[i];
    i++;

    //NR e NZ
    valor.n_R = vector[i]; i++;

    valor.n_Z = vector[i]; i++;

    std::cout<<"Main data ordem da quadratura "<< valor.n
            << "\nordem de parada " << valor.ordem_parada
            <<"\nOrdem de iteracao " << valor.iteracao
           << "\nGropu de energia "<<valor.G
           << "\n valor.L "<<valor.L
           <<"\nNumero de zonas"<<valor.n_Z
          <<"\nNumero de Região"<<valor.n_R
         <<std::endl;


    //Tamanho de cada Regiao
    // valor.TAM = new double [valor.n_R];
    for(int j = 0; j<valor.n_R; j++){
        // valor.TAM[j] = vector[i];
        i++;
    }

    //Nodos por Regiao
    valor.n_nodos = new short int [valor.n_R];
    for(int j = 0; j<valor.n_R; j++){
        valor.n_nodos[j] = vector[i];
        i++;
    }

    //Periodicidade
    valor.periodicidade = vector[i];
    i++;

    //Mapeamento
    valor.Map_R = new short int [valor.n_R];
    for(int j = 0; j<valor.n_R; j++){
        valor.Map_R[j] = vector[i];
        i++;
    }

    vector.erase(vector.begin(),vector.begin()+i);
    vector.shrink_to_fit() ;
    i = 0;

    /*********************************************************************************************************

                                    Matrizes

*********************************************************************************************************/

    valor.fonte_g = new double*[valor.G];  //Fonte Fisica
    valor.s_t = new long double*[valor.G];      //Sigma total
    valor.s_s = new long double***[valor.G];    //Sigma Espalhamento

    for(int j = 0; j<valor.G; j++){
        valor.fonte_g[j] = new double [valor.n_R];
        valor.s_s[j] = new long double**[valor.G];
        valor.s_t[j] = new long double[valor.n_Z];

        for(int k = 0; k<valor.G; k++){
            valor.s_s[j][k] = new long double *[valor.n_Z];

            for(int l = 0; l<valor.n_Z;l++){
                valor.s_s[j][k][l] = new long double [valor.L+1];}
        }
    }
    /**********************************************************************************************************/

    //Sigma total e Sigma de espalhamento
    for(int h = 0; h<valor.n_Z; h++){
        for(int j = 0; j<valor.G; j++){
            valor.s_t[j][h] = vector[i];
            i++;
        }
        for(int k = 0; k<valor.L+1; k++){
            for(int m = 0; m<valor.G;m++){
                for(int n = 0; n<valor.G; n ++){
                    valor.s_s[m][n][h][k] = vector[i];
                    i++;
                }
            }}}

    //Tipo de Condicoes de Contorno (Esq. Dir) (1-Prescrita. 2-Reflexiva)
    valor.tipo_ce = vector[i];i++;                valor.tipo_cd = vector[i]; i++;

    //Valor da condicao de contorno prescrita (Esq. Dir)
    valor.cceg = new double [valor.G];
    valor.ccdg = new double [valor.G];

    for(int j = 0; j<valor.G; j++){
        valor.cceg[j] = vector[i];
        i++;
        valor.ccdg[j] = vector[i];
        i++;
    }

    //Fonte Fisica
    for(int j = 0; j<valor.G; j++){
        for(int k = 0; k<valor.n_R; k++){
            valor.fonte_g[j][k] = vector[i];
            i++;
        }
    }
    vector.clear();
    vector.shrink_to_fit() ;

    /*********************************************************************************************************

                                           Dados calculados

*********************************************************************************************************/
    valor.CONTX = new int[valor.n_R]; //nodos por regiao acumulados
    valor.PASSO = new long double [valor.n_R]; //modulo entre nodos
    // valor.TAM_TOTAL = 0;  //comprimento total de x
    valor.NODOSX = 0;  // numero total de nodos por regiao

    for(int j = 0; j<valor.n_R; j++){
        valor.CONTX[j] = valor.NODOSX+valor.n_nodos[j];
        // valor.PASSO[j] = valor.TAM[j]/valor.n_nodos[j];
        valor.NODOSX = valor.CONTX[j];
    }
    valor.w = new double [valor.n];
    valor.mi = new double [valor.n];

    //fluxo angular e fluxo escalar

    valor.FLUXO_ANGULAR = new long double**[valor.G];
    valor.smgi = new long double**[valor.G];
    valor.FLUXO_ESCALAR = new long double*[valor.G];

    for(int g = 0; g<valor.G;g++){
        valor.FLUXO_ANGULAR[g] = new long double*[(valor.NODOSX)+1];
        valor.smgi[g] = new long double*[valor.NODOSX];
        valor.FLUXO_ESCALAR[g] = new long double[(valor.NODOSX)+1];

        for(int o = 0; o <= valor.NODOSX;o++){
            valor.FLUXO_ANGULAR[g][o] = new long double[valor.n];
        }
        for(int o = 0; o<valor.NODOSX;o++){
            valor.smgi[g][o] = new long double[valor.n];
        }
    }


    //Matriz com os polinômios de Legendre
    legendre_set ( valor.n,valor.mi,valor.w);
    valor.Mat_Legendre = new double *[valor.n];
    valor.legendre_n = new double [valor.L+1];
    for(int n = 0; n<valor.n; n ++){
        valor.Mat_Legendre[n] = new double [valor.L + 1];
    }

    setlocale(LC_ALL,"portuguese");

    for(int n = 0; n<valor.n; n ++){
        Legendre::Pn(valor.L,valor.mi[n],valor.legendre_n);
        for(int l = 0; l<valor.L+1;l++){
            valor.Mat_Legendre[n][l] = valor.legendre_n[l];
        }
    }
}

std::vector<double> BuildMatrices::saveFileDataInVector()
{
    double j;
    std::string k;
    std::ifstream file;
    std::vector <double> vector;
    file.open(fileName.c_str());

    if(!file)
    {
        std::cout<<"FILE OPENING FAILED\n";
        throw std::invalid_argument("Error: Material Data file opening failed");
    }
    else
    {

        while(file >> k)
        {  //lê todo o arquivo
            //            dados_txt >> k;

            if(k[0] == '/'){ //pega somente linhas com texto (primeira palavra)
                file.ignore(1000,'\n');
                std::cout<<k<<std::endl;
            }
            else
            {
                std::stringstream(k) >> j;
                vector.push_back(j);
            }
        }
    }

    std::cout<<"Vector size "<<vector.size()<<std::endl;

    file.close(); //fecha o arquivo txt

    return vector;
}

int BuildMatrices::getMaterialNumber() const
{
    return materialNumber;
}

void BuildMatrices::setMaterialNumber(int newMaterialNumber)
{
    materialNumber = newMaterialNumber;
}

int BuildMatrices::getRegionsNumber() const
{
    return regionsNumber;
}

void BuildMatrices::setRegionsNumber(int newRegionsNumber)
{
    regionsNumber = newRegionsNumber;
}

int BuildMatrices::getAnisotropyOrder() const
{
    return anisotropyOrder;
}

void BuildMatrices::setAnisotropyOrder(int newAnisotropyOrder)
{
    anisotropyOrder = newAnisotropyOrder;
}

int BuildMatrices::getStopOrder() const
{
    return stopOrder;
}

void BuildMatrices::setStopOrder(int newStopOrder)
{
    stopOrder = newStopOrder;
}

int BuildMatrices::getQuadratureOrder() const
{
    return quadratureOrder;
}

void BuildMatrices::setQuadratureOrder(int newQuadratureOrder)
{
    quadratureOrder = newQuadratureOrder;
}

int BuildMatrices::getEnergyGroupQtt() const
{
    return energyGroupQtt;
}

void BuildMatrices::setEnergyGroupQtt(int newEnergyGroupQtt)
{
    energyGroupQtt = newEnergyGroupQtt;
}


void BuildMatrices::allocateMatricesWithUserInterfaceData(dados_entrada &valor)
{
    int i = 0;

    auto vector = saveFileDataInVector();

    std::cout<<"Main data ordem da quadratura "<< valor.n
            << "\nordem de parada " << valor.ordem_parada
            <<"\nOrdem de iteracao " << valor.iteracao
           << "\nGropu de energia "<<valor.G
           << "\n valor.L "<<valor.L
           <<"\nNumero de zonas"<<valor.n_Z<<std::endl;

    //Tamanho de cada Regiao
    // valor.TAM = new double [valor.n_R];

    //    for(int j = 0; j<valor.n_R; j++){
    //        valor.TAM[j] = vector[i];
    //        i++;
    //    }

    //Nodos por Regiao
    valor.n_nodos = new short int [valor.n_R];
    for(int j = 0; j<valor.n_R; j++){
        valor.n_nodos[j] = vector[i];
        i++;
    }

    //Periodicidade
    valor.periodicidade = vector[i];
    i++;

    //Mapeamento
    valor.Map_R = new short int [valor.n_R];
    for(int j = 0; j<valor.n_R; j++){
        valor.Map_R[j] = vector[i];
        i++;
    }

    vector.erase(vector.begin(),vector.begin()+i);
    vector.shrink_to_fit() ;
    i = 0;

    /*********************************************************************************************************

                                    Matrizes

*********************************************************************************************************/

    valor.fonte_g = new double*[valor.G];        //Fonte Fisica
    valor.s_t = new long double*[valor.G];      //Sigma total
    valor.s_s = new long double***[valor.G];    //Sigma Espalhamento

    for(int j = 0; j<valor.G; j++){
        valor.fonte_g[j] = new double [valor.n_R];
        valor.s_s[j] = new long double**[valor.G];
        valor.s_t[j] = new long double[valor.n_Z];

        for(int k = 0; k<valor.G; k++){
            valor.s_s[j][k] = new long double *[valor.n_Z];

            for(int l = 0; l<valor.n_Z;l++){
                valor.s_s[j][k][l] = new long double [valor.L+1];}
        }
    }
    /**********************************************************************************************************/

    //Sigma total e Sigma de espalhamento
    for(int h = 0; h<valor.n_Z; h++){
        for(int j = 0; j<valor.G; j++){
            valor.s_t[j][h] = vector[i];
            i++;
        }
        for(int k = 0; k<valor.L+1; k++){
            for(int m = 0; m<valor.G;m++){
                for(int n = 0; n<valor.G; n ++){
                    valor.s_s[m][n][h][k] = vector[i];
                    i++;
                }
            }}}

    //Tipo de Condicoes de Contorno (Esq. Dir) (1-Prescrita. 2-Reflexiva)
    valor.tipo_ce = vector[i];i++;                valor.tipo_cd = vector[i]; i++;

    //Valor da condicao de contorno prescrita (Esq. Dir)
    valor.cceg = new double [valor.G];
    valor.ccdg = new double [valor.G];

    for(int j = 0; j<valor.G; j++){
        valor.cceg[j] = vector[i];
        i++;
        valor.ccdg[j] = vector[i];
        i++;
    }

    //Fonte Fisica
    for(int j = 0; j<valor.G; j++){
        for(int k = 0; k<valor.n_R; k++){
            valor.fonte_g[j][k] = vector[i];
            i++;
        }
    }
    vector.clear();
    vector.shrink_to_fit() ;

    /*********************************************************************************************************

                                           Dados calculados

*********************************************************************************************************/
    valor.CONTX = new int[valor.n_R]; //nodos por regiao acumulados
    valor.PASSO = new long double [valor.n_R]; //modulo entre nodos
    valor.TAM_TOTAL = 0;  //comprimento total de x
    valor.NODOSX = 0;  // numero total de nodos por regiao

    for(int j = 0; j<valor.n_R; j++){
        valor.CONTX[j] = valor.NODOSX+valor.n_nodos[j];
        //valor.PASSO[j] = valor.TAM[j]/valor.n_nodos[j];
        valor.NODOSX = valor.CONTX[j];
    }
    valor.w = new double [valor.n];
    valor.mi = new double [valor.n];

    //fluxo angular e fluxo escalar

    valor.FLUXO_ANGULAR = new long double**[valor.G];
    valor.smgi = new long double**[valor.G];
    valor.FLUXO_ESCALAR = new long double*[valor.G];

    for(int g = 0; g<valor.G;g++){
        valor.FLUXO_ANGULAR[g] = new long double*[(valor.NODOSX)+1];
        valor.smgi[g] = new long double*[valor.NODOSX];
        valor.FLUXO_ESCALAR[g] = new long double[(valor.NODOSX)+1];

        for(int o = 0; o <= valor.NODOSX;o++){
            valor.FLUXO_ANGULAR[g][o] = new long double[valor.n];
        }
        for(int o = 0; o<valor.NODOSX;o++){
            valor.smgi[g][o] = new long double[valor.n];
        }
    }


    //Matriz com os polinômios de Legendre
    legendre_set ( valor.n,valor.mi,valor.w);
    valor.Mat_Legendre = new double *[valor.n];
    valor.legendre_n = new double [valor.L+1];
    for(int n = 0; n<valor.n; n ++){
        valor.Mat_Legendre[n] = new double [valor.L + 1];
    }

    setlocale(LC_ALL,"portuguese");

    for(int n = 0; n<valor.n; n ++){
        Legendre::Pn(valor.L,valor.mi[n],valor.legendre_n);
        for(int l = 0; l<valor.L+1;l++){
            valor.Mat_Legendre[n][l] = valor.legendre_n[l];
        }
    }
}

std::vector<std::vector<long double> > BuildMatrices::
calculateAbsorptionCrossSectionMatrix(dados_entrada *data,
                                      std::vector<std::vector<long double> >& sigmaScattering)
{
    std::vector<std::vector<long double>> absorptionMatrix;

    if (data->s_t == nullptr || sigmaScattering.empty())
        throw std::invalid_argument("Error: calculating the neutron absorption matrix");

    for (int zIndex = 0; zIndex < data->n_Z; ++zIndex)
    {
        std::vector<long double> absorptionByGroup;

        for (int gIndex = 0; gIndex < data->G; ++gIndex)
        {
            auto sigmaScatteringByGroup = sigmaScattering[zIndex][gIndex];
            auto sigmaTotal             = data->s_t[gIndex][zIndex] ;

            auto sigmaAbsor = sigmaTotal - sigmaScatteringByGroup;
            absorptionByGroup.push_back(sigmaAbsor);
        }

        absorptionMatrix.push_back(absorptionByGroup);
    }

    return absorptionMatrix;
}

void BuildMatrices::copyVectorToRawPointer(std::vector<double> &mVector, double*& buffer)
{
    buffer = new double[mVector.size()];

    std::copy(mVector.begin(), mVector.end(), buffer);
}

void BuildMatrices::copyRegionVectorToRowPointers(std::array<RegionData, 10> &regionArray,
                                                  int regionNumber,
                                                  dados_entrada* data)
{
    data->n_nodos = new short int[regionNumber];
    data->TAM     = new double[regionNumber];
    data->Map_R   = new short int[regionNumber];
    data->PASSO   = new long double [regionNumber];
    data->CONTX   = new int[regionNumber];

    data->fonte_g = new double*[data->G];

    for(int j = 0; j < data->G; j++)
        data->fonte_g[j] = new double [regionNumber];

    for (int iIndex = 0; iIndex < regionNumber; ++iIndex)
    {
        auto region = regionArray[iIndex];

        data->n_nodos[iIndex] = region.node;
        data->TAM[iIndex]     = region.quote;
        data->Map_R[iIndex]   = region.zone;

        if (region.physicalSource.has_value())
        {
            for (int j = 0; j < data->G; j++)
            {
                data->fonte_g[j][iIndex] = region.physicalSource.value()[j];
            }
        }
        else
        {
            //Define zero to not set source
            for (int j = 0; j < data->G; j++)
            {
                data->fonte_g[j][iIndex] = 0.0;
            }
        }
    }
}

void BuildMatrices::buildCrossSectionMatrices(dados_entrada *data)
{
    data->s_t = new long double*[data->G];      //total Sigma
    data->s_s = new long double***[data->G];    //Scattering Sigma

    auto vector = saveFileDataInVector();

    for (auto v:vector)
        std::cout<<v<< std::endl;

    int i = 0;

    for(int j = 0; j < data->G; j++)
    {
        data->s_s[j] = new long double**[data->G];
        data->s_t[j] = new long double  [data->n_Z];

        for(int k = 0; k < data->G; k++)
        {
            data->s_s[j][k] = new long double *[data->n_Z];

            for(int l = 0; l < data->n_Z; l++){
                data->s_s[j][k][l] = new long double [data->L + 1];}
        }
    }

    /**********************************************************************************************************/
    //Total and Scattering cross section
    for(int h = 0; h < data->n_Z; h++)
    {
        for(int j = 0; j < data->G; j++)
        {
            data->s_t[j][h] = vector[i];
            i++;
        }

        for(int k = 0; k < data->L + 1; k++)
        {
            for(int m = 0; m < data->G; m++)
            {
                for(int n = 0; n<data->G; n ++)
                {
                    data->s_s[m][n][h][k] = vector[i];
                    i++;
                }
            }
        }
    }

    calculateDataMatrices(data);
}

void BuildMatrices::calculateLegendreMatrix(dados_entrada* data)
{
    data->w  = new double [data->n];
    data->mi = new double [data->n];

    //Matriz com os polinômios de Legendre
    legendre_set ( data->n, data->mi, data->w);

    data->Mat_Legendre = new double *[data->n];
    data->legendre_n   = new double [data->L + 1];

    for(int n = 0; n < data->n; n++)
        data->Mat_Legendre[n] = new double [data->L + 1];

    for(int n = 0; n < data->n; n++)
    {
        Legendre::Pn(data->L, data->mi[n], data->legendre_n);
        for(int l = 0; l < data->L + 1; l++)
        {
            data->Mat_Legendre[n][l] = data->legendre_n[l];
        }
    }
}

void BuildMatrices::calculateDataMatrices(dados_entrada *data)
{
    data->TAM_TOTAL = 0;  //comprimento total de x
    data->NODOSX    = 0;  // numero total de nodos por regiao

    for(int j = 0; j < data->n_R; j++)
    {
        data->CONTX[j] = data->NODOSX + data->n_nodos[j];
        data->PASSO[j] = data->TAM[j]/data->n_nodos[j];
        data->NODOSX   = data->CONTX[j];
        data->TAM_TOTAL +=  data->TAM[j];
    }

    //fluxo angular e fluxo escalar

    data->FLUXO_ANGULAR = new long double**[data->G];
    data->smgi          = new long double**[data->G];
    data->FLUXO_ESCALAR = new long double*[data->G];

    data->FLUXO_ANGULAR_DIREITA  = new long double**[data->G];
    data->FLUXO_ANGULAR_ESQUERDA = new long double**[data->G];

    for(int g = 0; g < data->G; g++)
    {
        data->FLUXO_ANGULAR[g] = new long double*[(data->NODOSX) + 1];
        data->smgi[g]          = new long double*[data->NODOSX      ];
        data->FLUXO_ESCALAR[g] = new long double[(data->NODOSX)  + 1];

        data->FLUXO_ANGULAR_DIREITA[g]  = new long double*[(data->NODOSX) + 1];
        data->FLUXO_ANGULAR_ESQUERDA[g] = new long double*[(data->NODOSX) + 1];

        for(int o = 0; o <= data->NODOSX; o++)
        {
            data->FLUXO_ANGULAR[g][o] = new long double[data->n];

            data->FLUXO_ANGULAR_DIREITA[g][o]   = new long double[data->n];
            data->FLUXO_ANGULAR_ESQUERDA[g][o]  = new long double[data->n];
        }

        for(int o = 0; o < data->NODOSX; o++)
        {
            data->smgi[g][o] = new long double[data->n];
        }
    }

    calculateLegendreMatrix(data);
}

int BuildMatrices::getIterationNumber() const
{
    return iterationNumber;
}

void BuildMatrices::setIterationNumber(int newIterationNumber)
{
    iterationNumber = newIterationNumber;
}
