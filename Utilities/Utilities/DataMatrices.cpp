#include "DataMatrices.h"
#include "GausLegendreQuadrature.h"
#include "LegendrePolynomial.h"

#include <algorithm>
#include <filesystem>
#include <numeric>
#include <stdexcept>

#include <QCoreApplication>
#include <QFile>
#include <QTemporaryFile>
#include <QTextStream>

BuildMatrices* BuildMatrices::m_ptr = nullptr;

BuildMatrices::BuildMatrices()
{

}

BuildMatrices *BuildMatrices::getInstance()
{
    if (nullptr == m_ptr)
    {
        m_ptr = new BuildMatrices;
    }

    return m_ptr;
}

void BuildMatrices::destroyInstance()
{
    if (nullptr != m_ptr)
    {
        delete m_ptr;
        m_ptr = nullptr;
    }
}

BuildMatrices::BuildMatrices(dados_entrada *newDatricesDD_Data,
                             ProjectData *newProjectInterfaceData)
    : matricesDD_Data(newDatricesDD_Data),
    projectInterfaceData(newProjectInterfaceData)
{
};

std::unique_ptr<dados_entrada> BuildMatrices::copyProjectDataToRawPointers(ProjectData &proj, CrossSectionDataFilerParameters& fileParameter)
{
    auto data = std::make_unique<dados_entrada>();
    fileName  = proj.NeutronMacroscopicCrossSectionsFilePath;
    std::filesystem::directory_entry entry{fileName};

    if (   ( true == fileName.empty() )
        || (    ( false == entry.exists() )
             && ( fileName != ":/Default_Project/Resources/Default_Projet.txt") ) )
    {
        throw std::invalid_argument("Error: Material Data File issue. \nYou need to set a Cross Section File.");
    }

    //Use the values from cross section data file
    data->n   = proj.quadratureOrder;
    data->n_R = proj.regionNumber;

    data->G   = fileParameter.numberOfEnergyGroup;
    data->L   = fileParameter.numberOfLegendre;
    data->n_Z = fileParameter.numberOfZones;

    data->tipo_ce = static_cast<int>(proj.leftBoundaryConditionsType);
    data->tipo_cd = static_cast<int>(proj.rightBoundaryConditionsType);
    data->ordem_parada  = proj.stopOrder;
    data->periodicidade = proj.periodicity;
    data->iteracao      = proj.maximumIterationsNumber;

    std::vector<double> bc;

    for (int iIndex = 0; iIndex < data->G; ++iIndex)
    {
        bc.push_back(0.0);
    }

    if (proj.bcLeft.has_value())
    {
        copyVectorToRawPointer(proj.bcLeft.value(), data->cceg);
    }
    else
    {
        copyVectorToRawPointer(bc, data->cceg);
    }

    if (proj.bcRight.has_value())
    {
        copyVectorToRawPointer(proj.bcRight.value(), data->ccdg);
    }
    else
    {
        copyVectorToRawPointer(bc, data->ccdg);
    }

    copyRegionVectorToRowPointers(proj.regionArray, proj.regionNumber, data.get());

    buildCrossSectionMatrices(data.get());

    //update values that the user chose
    data->G   = proj.energyGroup;
    data->L   = proj.legendreOrder;
    data->n_Z = proj.zoneNumber;

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

void BuildMatrices::calculateAbsorptionRatePerRegion(dados_entrada *data,
                                                     CalculatedData *DDResult)
{
    std::vector<std::vector<long double>> absorptionRate;

    if (DDResult->matrices.absorptionCrossSection.empty())
        throw std::invalid_argument("Error: Absorption Cross Section Matrix is empty");

    if (DDResult->averageNeutronFluxPerRegion.empty())
        throw std::invalid_argument("Error: Average Neutron Flux Per Region Matrix is empty");

    for (int rIndex = 0; rIndex < data->n_R; ++rIndex)
    {
        std::vector<long double> absorptionByGroup;

        for (int gIndex = 0; gIndex < data->G; ++gIndex)
        {
            auto zIndex = data->Map_R[rIndex] - 1;

            if (zIndex < DDResult->matrices.absorptionCrossSection.size())
            {
                auto sigmaAbsor = DDResult->matrices.absorptionCrossSection[zIndex][gIndex] *
                                  DDResult->averageNeutronFluxPerRegion[rIndex][gIndex];

                absorptionByGroup.push_back(sigmaAbsor);
            }
        }

        absorptionRate.push_back(absorptionByGroup);
    }

    DDResult->absorptionRate.swap(absorptionRate);
}

void BuildMatrices::calculateAbsorptionRatePerNode(dados_entrada *data, CalculatedData *DDResult)
{
    if (DDResult->matrices.absorptionCrossSection.empty())
        throw std::invalid_argument("Error: Absorption Cross Section Matrix is empty");

    if (data->FLUXO_ESCALAR == nullptr)
        throw std::invalid_argument("Error: Scalar Flux Matrix is not initialized");

    std::vector<std::vector<long double>> absorptionRatePerRegion;

    for (int rIndex = 0; rIndex < data->n_R; ++rIndex)
    {
        auto zIndex = data->Map_R[rIndex] - 1;

        for (int gIndex = 0; gIndex < data->G; ++gIndex)
        {
            std::vector<long double> absorptionByGroup;

            for (int nIndex = 0; nIndex <= data->NODOSX; ++nIndex)
            {
                long double sigmaAbsor = DDResult->matrices.absorptionCrossSection[zIndex][gIndex]  *
                                         data->FLUXO_ESCALAR[gIndex][nIndex];

                absorptionByGroup.push_back(sigmaAbsor);
            }

            absorptionRatePerRegion.push_back(absorptionByGroup);
        }
    }

    DDResult->absorptionRatePerNode.swap( absorptionRatePerRegion );
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
            sum = 0.0;

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

void BuildMatrices::copyResourceToDestination(const std::string &resourcePath, const std::string &destinationPath)
{
    QFile resourceFile(resourcePath.c_str());

    if (!resourceFile.open(QIODevice::ReadOnly))
    {
        qWarning() << "Failed to open the resource:" << resourcePath;
        return;
    }

    QTemporaryFile tempFile;

    if (!tempFile.open())
    {
        qWarning() << "Failed to create a temporary file.";
        return;
    }

    tempFile.write(resourceFile.readAll());
    tempFile.close();

    if (QFile::copy(tempFile.fileName(), destinationPath.c_str()))
    {
        qDebug() << "File copied to:" << destinationPath;
    }
    else
    {
        qWarning() << "Failed to copy the file to:" << destinationPath;
    }
}

std::vector<std::vector<long double>> BuildMatrices::calculateScatteringCrossSectionMatrix(dados_entrada *data)
{
    std::vector<std::vector<long double>> sigmaScattering;

    // Loop over all spatial zones
    for (int zIndex = 0; zIndex < data->n_Z; ++zIndex)
    {
        std::vector<long double> scattering_g;

        // Loop over destination energy groups (gIndex)
        for (int gIndex = 0; gIndex < data->G; ++gIndex)
        {
            long double sum = 0.0; // Initialize the sum for scattering cross-section for group gIndex

            // Accumulate scattering contributions directly
            for (int gLineIndex = 0; gLineIndex < data->G; ++gLineIndex) // Loop over source groups (g')
            {
                for (int lIndex = 0; lIndex < data->L + 1; ++lIndex) // Loop over Legendre moments
                {
                    double value = data->s_s[gLineIndex][gIndex][zIndex][lIndex];
                   // std::cout << "g" << gIndex << " g'" << gLineIndex
                              //<< " legendre = " << lIndex << " value = " << value << std::endl;

                    sum += value; // Add the current value to the total sum
                }
            }

            //std::cout << "sum for g " << gIndex << " = " << sum << std::endl;

            // Store the total sum for the current group gIndex
            scattering_g.push_back(sum);
        }

        // Store the scattering vector for the current spatial zone
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

void BuildMatrices::allocateMatrices(dados_entrada &valor)
{
    auto vector = saveFileDataInVector();

    if (vector.size() == 0)
        throw std::invalid_argument("Error: There is no data to build matrices");

 /********************************************************************************************************

                           Distribuição de dados em matrizes

*********************************************************************************************************/
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
    for (int j = 0; j<valor.n_R; j++){
        // valor.TAM[j] = vector[i];
        i++;
    }

    //Nodos por Regiao
    valor.n_nodos = new short int [valor.n_R];
    for (int j = 0; j<valor.n_R; j++){
        valor.n_nodos[j] = vector[i];
        i++;
    }

    //Periodicidade
    valor.periodicidade = vector[i];
    i++;

    //Mapeamento
    valor.Map_R = new short int [valor.n_R];
    for (int j = 0; j<valor.n_R; j++){
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

    for (int j = 0; j<valor.G; j++){
        valor.fonte_g[j] = new double [valor.n_R];
        valor.s_s[j] = new long double**[valor.G];
        valor.s_t[j] = new long double[valor.n_Z];

        for (int k = 0; k<valor.G; k++){
            valor.s_s[j][k] = new long double *[valor.n_Z];

            for (int l = 0; l<valor.n_Z;l++){
                valor.s_s[j][k][l] = new long double [valor.L+1];}
        }
    }
    /**********************************************************************************************************/

    //Sigma total e Sigma de espalhamento
    for (int h = 0; h<valor.n_Z; h++){
        for (int j = 0; j<valor.G; j++){
            valor.s_t[j][h] = vector[i];
            i++;
        }
        for (int k = 0; k<valor.L+1; k++){
            for (int m = 0; m<valor.G;m++){
                for (int n = 0; n<valor.G; n ++){
                    valor.s_s[m][n][h][k] = vector[i];
                    i++;
                }
            }}}

    //Tipo de Condicoes de Contorno (Esq. Dir) (1-Prescrita. 2-Reflexiva)
    valor.tipo_ce = vector[i];i++;                valor.tipo_cd = vector[i]; i++;

    //Valor da condicao de contorno prescrita (Esq. Dir)
    valor.cceg = new double [valor.G];
    valor.ccdg = new double [valor.G];

    for (int j = 0; j<valor.G; j++){
        valor.cceg[j] = vector[i];
        i++;
        valor.ccdg[j] = vector[i];
        i++;
    }

    //Fonte Fisica
    for (int j = 0; j<valor.G; j++){
        for (int k = 0; k<valor.n_R; k++){
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

    for (int j = 0; j<valor.n_R; j++){
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

    for (int g = 0; g<valor.G;g++){
        valor.FLUXO_ANGULAR[g] = new long double*[(valor.NODOSX)+1];
        valor.smgi[g] = new long double*[valor.NODOSX];
        valor.FLUXO_ESCALAR[g] = new long double[(valor.NODOSX)+1];

        for (int o = 0; o <= valor.NODOSX;o++){
            valor.FLUXO_ANGULAR[g][o] = new long double[valor.n];
        }
        for (int o = 0; o<valor.NODOSX;o++){
            valor.smgi[g][o] = new long double[valor.n];
        }
    }


    //Matriz com os polinômios de Legendre
    legendre_set ( valor.n,valor.mi,valor.w);
    valor.Mat_Legendre = new double *[valor.n];
    double* legendre_n = new double [valor.L+1];
    for (int n = 0; n<valor.n; n ++){
        valor.Mat_Legendre[n] = new double [valor.L + 1];
    }

    setlocale(LC_ALL,"portuguese");

    for (int n = 0; n<valor.n; n ++){
        Legendre::Pn(valor.L,valor.mi[n], legendre_n);
        for (int l = 0; l<valor.L+1;l++){
            valor.Mat_Legendre[n][l] = legendre_n[l];
        }
    }

    delete [] legendre_n;
}

std::vector<double> BuildMatrices::saveFileDataInVector()
{
    double j;
    std::string k;
    std::vector<double> vector;

    // Open the file using QFile
    QFile file(QString::fromStdString(fileName));

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        std::cerr << "FILE OPENING FAILED\n";
        throw std::invalid_argument("Error: Material Data file opening failed");
    }

    QTextStream in(&file);

    while (!in.atEnd())
    {
        QString line = in.readLine(); // Read the file line by line

        if (!line.isEmpty() && line[0] == '/') // Skip comment lines
        {
            std::cout << line.toStdString() << std::endl;
            continue;
        }

        // Convert the line to std::string and process
        std::string lineStr = line.toStdString();
        std::stringstream ss(lineStr);
        while (ss >> k) // Read words from the line
        {
            std::stringstream(k) >> j;
            vector.push_back(j);
        }
    }

    std::cout << "Vector size " << vector.size() << std::endl;

    file.close(); // Close the file

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

    //    for (int j = 0; j<valor.n_R; j++){
    //        valor.TAM[j] = vector[i];
    //        i++;
    //    }

    //Nodos por Regiao
    valor.n_nodos = new short int [valor.n_R];
    for (int j = 0; j<valor.n_R; j++){
        valor.n_nodos[j] = vector[i];
        i++;
    }

    //Periodicidade
    valor.periodicidade = vector[i];
    i++;

    //Mapeamento
    valor.Map_R = new short int [valor.n_R];
    for (int j = 0; j<valor.n_R; j++){
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

    for (int j = 0; j<valor.G; j++){
        valor.fonte_g[j] = new double [valor.n_R];
        valor.s_s[j] = new long double**[valor.G];
        valor.s_t[j] = new long double[valor.n_Z];

        for (int k = 0; k<valor.G; k++){
            valor.s_s[j][k] = new long double *[valor.n_Z];

            for (int l = 0; l<valor.n_Z;l++){
                valor.s_s[j][k][l] = new long double [valor.L+1];}
        }
    }
    /**********************************************************************************************************/

    //Sigma total e Sigma de espalhamento
    for (int h = 0; h<valor.n_Z; h++){
        for (int j = 0; j<valor.G; j++){
            valor.s_t[j][h] = vector[i];
            i++;
        }
        for (int k = 0; k<valor.L+1; k++){
            for (int m = 0; m<valor.G;m++){
                for (int n = 0; n<valor.G; n ++){
                    valor.s_s[m][n][h][k] = vector[i];
                    i++;
                }
            }}}

    //Tipo de Condicoes de Contorno (Esq. Dir) (1-Prescrita. 2-Reflexiva)
    valor.tipo_ce = vector[i];i++;                valor.tipo_cd = vector[i]; i++;

    //Valor da condicao de contorno prescrita (Esq. Dir)
    valor.cceg = new double [valor.G];
    valor.ccdg = new double [valor.G];

    for (int j = 0; j<valor.G; j++){
        valor.cceg[j] = vector[i];
        i++;
        valor.ccdg[j] = vector[i];
        i++;
    }

    //Fonte Fisica
    for (int j = 0; j<valor.G; j++){
        for (int k = 0; k<valor.n_R; k++){
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

    for (int j = 0; j<valor.n_R; j++){
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

    for (int g = 0; g<valor.G;g++){
        valor.FLUXO_ANGULAR[g] = new long double*[(valor.NODOSX)+1];
        valor.smgi[g] = new long double*[valor.NODOSX];
        valor.FLUXO_ESCALAR[g] = new long double[(valor.NODOSX)+1];

        for (int o = 0; o <= valor.NODOSX;o++){
            valor.FLUXO_ANGULAR[g][o] = new long double[valor.n];
        }
        for (int o = 0; o<valor.NODOSX;o++){
            valor.smgi[g][o] = new long double[valor.n];
        }
    }


    //Matriz com os polinômios de Legendre
    legendre_set ( valor.n,valor.mi,valor.w);
    valor.Mat_Legendre = new double *[valor.n];
    double* legendre_n = new double [valor.L+1];
    for (int n = 0; n<valor.n; n ++){
        valor.Mat_Legendre[n] = new double [valor.L + 1];
    }

    setlocale(LC_ALL,"portuguese");

    for (int n = 0; n<valor.n; n ++){
        Legendre::Pn(valor.L,valor.mi[n],legendre_n);
        for (int l = 0; l<valor.L+1;l++){
            valor.Mat_Legendre[n][l] = legendre_n[l];
        }
    }

    delete [] legendre_n;
}

std::vector<std::vector<long double>> BuildMatrices::calculateAbsorptionCrossSectionMatrix(dados_entrada *data,
                                                                                           std::vector<std::vector<long double>>& sigmaScattering)
{
    if (data->s_t == nullptr || sigmaScattering.empty())
    {
        throw std::invalid_argument("Error: Invalid input data. Ensure sigmaTotal and sigmaScattering are correctly initialized.");
    }

    std::vector<std::vector<long double>> absorptionMatrix;

    // Iterate over all spatial zones
    for (int zIndex = 0; zIndex < data->n_Z; ++zIndex)
    {
        std::vector<long double> absorptionByGroup;

        // Iterate over all energy groups
        for (int gIndex = 0; gIndex < data->G; ++gIndex)
        {
            // Retrieve total and scattering cross-sections
            long double sigmaTotal = data->s_t[gIndex][zIndex];
            long double sigmaScatteringByGroup = sigmaScattering[zIndex][gIndex];

            // Calculate absorption cross-section
            long double sigmaAbsor = sigmaTotal - sigmaScatteringByGroup;

            if (sigmaAbsor < 0.0L)
            {
                //throw std::invalid_argument(
                //"Error: Absorption cross-section is negative. Check your total and scattering cross-section data.");
            }

            // Store the result
            absorptionByGroup.push_back(sigmaAbsor);
        }

        // Add the absorption values for the current zone to the matrix
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

    for (int j = 0; j < data->G; j++)
    {
        data->fonte_g[j] = new double [regionNumber];
    }

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

    for (int j = 0; j < data->G; j++)
    {
        data->s_s[j] = new long double**[data->G];
        data->s_t[j] = new long double  [data->n_Z];

        for (int k = 0; k < data->G; k++)
        {
            data->s_s[j][k] = new long double *[data->n_Z];

            for (int l = 0; l < data->n_Z; l++){
                data->s_s[j][k][l] = new long double [data->L + 1];}
        }
    }

    /**********************************************************************************************************/
    //Total and Scattering cross section
    for (int h = 0; h < data->n_Z; h++)
    {
        std::cout<<"\nZona "<<h<<std::endl;

        for (int j = 0; j < data->G; j++)
        {
            data->s_t[j][h] = vector[i];
            i++;
        }

        for (int k = 0; k < data->L + 1; k++)
        {
            std::cout<<"\nLegendre "<<k<<std::endl;
            for (int m = 0; m < data->G; m++)
            {
                for (int n = 0; n<data->G; n ++)
                {
                    data->s_s[m][n][h][k] = vector[i];
                    std::cout<<data->s_s[m][n][h][k] <<" "<<std::endl;

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

    data->Mat_Legendre   = new double *[data->n];
    double* legendre_n   = new double [data->L + 1];

    for (int n = 0; n < data->n; n++)
        data->Mat_Legendre[n] = new double [data->L + 1];

    for (int n = 0; n < data->n; n++)
    {
        Legendre::Pn(data->L, data->mi[n], legendre_n);

        for (int l = 0; l < data->L + 1; l++)
        {
            data->Mat_Legendre[n][l] = legendre_n[l];
        }
    }

    delete [] legendre_n;
}

void BuildMatrices::calculateDataMatrices(dados_entrada *data)
{
    data->TAM_TOTAL = 0;  //comprimento total de x
    data->NODOSX    = 0;  // numero total de nodos por regiao

    for (int j = 0; j < data->n_R; j++)
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

    for (int g = 0; g < data->G; g++)
    {
        data->FLUXO_ANGULAR[g] = new long double*[(data->NODOSX) + 1];
        data->smgi[g]          = new long double*[data->NODOSX      ];
        data->FLUXO_ESCALAR[g] = new long double[(data->NODOSX)  + 1];

        data->FLUXO_ANGULAR_DIREITA[g]  = new long double*[(data->NODOSX) + 1];
        data->FLUXO_ANGULAR_ESQUERDA[g] = new long double*[(data->NODOSX) + 1];

        for (int o = 0; o <= data->NODOSX; o++)
        {
            data->FLUXO_ANGULAR[g][o] = new long double[data->n];

            data->FLUXO_ANGULAR_DIREITA[g][o]   = new long double[data->n];
            data->FLUXO_ANGULAR_ESQUERDA[g][o]  = new long double[data->n];
        }

        for (int o = 0; o < data->NODOSX; o++)
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

std::string BuildMatrices::saveMaterialData(std::string &finalPath, std::string &oldPath)
{
    namespace fs = std::filesystem;

    fs::path filePath(finalPath);
    fs::path scatteringFile(oldPath);
    fs::path destinationPath = filePath.parent_path() / (scatteringFile.stem().string() + " (copy)" + scatteringFile.extension().string());
    int counter = 0;

    while (fs::exists(destinationPath))
    {
        destinationPath = filePath.parent_path() / (scatteringFile.stem().string() + " (copy " + std::to_string(counter) + ")" + scatteringFile.extension().string());
        counter++;
    }

    if (oldPath.find(":/") == std::string::npos)
    {
        std::filesystem::copy(scatteringFile, destinationPath, std::filesystem::copy_options::overwrite_existing);

        qInfo() << "File was copied to:" << QString::fromStdString(destinationPath.string());
    }
    else
    {
        //The path is a r
        copyResourceToDestination(oldPath, destinationPath);
    }

    return destinationPath;
}

void BuildMatrices::writeAbsRatePerNode(dados_entrada *DDValues, CalculatedData *DDResult)
{
    if (DDResult->absorptionRatePerNode.empty())
    {
        throw std::runtime_error("absorptionRatePerNode is empty");
    }

    std::string titleStr = computerFileName(DDValues, "Absorption_Rate_Per_Node");
    std::ofstream output(titleStr);

    if (!output.is_open())
    {
        throw std::runtime_error( "Error opening file: " + titleStr);
    }

    DDResult->absorptionRatePerNodeFile = titleStr;

    int colWidth = 25;
    int precision = 2;

    //Write compile information
    output << std::string(colWidth * 3, '-') << std::endl;
    output << "Iteration Number: "
           << DDValues->iteracaoFinal<<"\nTime: "
           << DDValues->tempoFinalDeProcessamento<<"s\n";

    output << std::string(colWidth * 3, '-') << std::endl;

    // Write table headers
    output << std::left << std::setw(colWidth) << "Position x (cm)"
           << std::setw(colWidth) << "Group"
           << std::setw(colWidth) << "Absorption Rate Per Node" << std::endl;
    output << std::string(colWidth * 3, '-') << std::endl;

    double t = 0;
    int nod  = 0;
    double totalRegionSize = 0.0;

    for (int r = 0; r < DDValues->n_R; ++r)
        totalRegionSize += DDValues->TAM[r];

    while (t <= totalRegionSize)
    {
        for (int g = 0; g < DDValues->G; ++g)
        {
            if (g == 0)
            {
                output << std::left << std::setw(colWidth) << std::fixed << std::setprecision(precision) << t
                       << std::setw(colWidth) << g + 1
                       << std::fixed << std::setprecision(15) << DDResult->absorptionRatePerNode[g][nod] << std::endl;
            }
            else
            {
                output << std::left << std::setw(colWidth) << ""
                       << std::setw(colWidth) << g + 1
                       << std::fixed << std::setprecision(15) << DDResult->absorptionRatePerNode[g][nod] << std::endl;
            }
        }

        t += DDValues->periodicidade;
        nod += static_cast<int>((DDValues->NODOSX * DDValues->periodicidade) / DDValues->TAM_TOTAL);
        output << std::string(colWidth * 3, '-') << std::endl;
    }
}

std::string BuildMatrices::computerFileName(dados_entrada* DDValues, std::string name)
{
    std::ostringstream title;
    std::string titleStr;
    std::string directory;

    std::filesystem::path filePath(fileName);

    if (QString(fileName.c_str()).contains("Default"))
    {
        std::string binaryDir = QCoreApplication::applicationDirPath().toStdString();

        directory = binaryDir;
    }
    else
    {
        directory = filePath.parent_path().string();
    }

    title << directory       <<"/"
          << name
          << "_R"            << DDValues->n_R
          << "_G"            << DDValues->G
          << "_L"            << DDValues->L
          << "_N"            << DDValues->n
          << "_Nod"          << DDValues->NODOSX
          << ".txt";

    titleStr = title.str();

    return titleStr;
}

void BuildMatrices::writeAverageNeutronFluxPerRegion(dados_entrada *DDValues, CalculatedData *DDResult)
{
    std::string titleStr = computerFileName(DDValues, "Average_Neutron_Flux_Per_Region");
    std::ofstream outFile(titleStr);

    if (!outFile.is_open())
    {
        throw std::runtime_error("Error opening file: " + titleStr);
    }

    DDResult->averageNeutronFluxPerRegionFile = titleStr;

    // Iterate over zones
    for (size_t rIndex = 0; rIndex < DDValues->n_R; ++rIndex)
    {
        // Write zoneIndex as table title
        outFile << "-----------------------------------------------------------------------------------------" << std::endl;
        outFile << "Region " << rIndex + 1 << std::endl;

        // Write table headers
        outFile << std::left << std::setw(15) << "Group" << "Average Neutron Flux" << std::endl;

        for (size_t group = 0; group < DDValues->G; ++group)
        {
            outFile << std::left << std::setw(15) << group + 1
                    << DDResult->averageNeutronFluxPerRegion[rIndex][group] << std::endl;
        }

        outFile << "-----------------------------------------------------------------------------------------" << std::endl;

        // Add a newline for separation between Region if there are multiple Regions
        if (rIndex < DDResult->averageNeutronFluxPerRegion.size() - 1)
        {
            outFile << std::endl;
        }
    }

    outFile.close();
}

void BuildMatrices::writeAbsorptionRateFile(dados_entrada *DDValues, CalculatedData *DDResult)
{
    if (DDResult->absorptionRate.empty())
    {
        throw std::runtime_error("Absorption Matrix is empty.");
    }

    std::string titleStr = computerFileName(DDValues, "Absorption_Rate");
    std::ofstream outFile(titleStr);

    if (!outFile.is_open())
    {
        throw std::runtime_error("Error opening file: " + titleStr);
    }

    DDResult->absorptionRateFile = titleStr;

    // Iterate over zones
    for (size_t rIndex = 0; rIndex < DDValues->n_R; ++rIndex)
    {
        // Write zoneIndex as table title
        outFile << "-----------------------------------------------------------------------------------------" << std::endl;
        outFile << "Region " << rIndex + 1 << std::endl;

        // Write table headers
        outFile << std::left << std::setw(15) << "Group" << "Absorption Rate" << std::endl;

        for (size_t group = 0; group < DDValues->G; ++group)
        {
            outFile << std::left << std::setw(15) << group + 1
                    << DDResult->absorptionRate[rIndex][group] << std::endl;
        }

        outFile << "-----------------------------------------------------------------------------------------" << std::endl;

        // Add a newline for separation between Region if there are multiple Regions
        if (rIndex < DDResult->absorptionRate.size() - 1)
        {
            outFile << std::endl;
        }
    }

    outFile.close();
}

void BuildMatrices::writeAbsorptionCrossSectionFile(dados_entrada *DDValues, CalculatedCrossSectionMatrices *DDResult)
{
    if (DDResult->absorptionCrossSection.empty())
    {
        throw std::runtime_error("Absorption Cross Section Matrix is empty.");
    }

    auto absorptionCrossSection = DDResult->absorptionCrossSection;
    auto zoneNumber  = DDValues->n_Z;
    auto groupNumber = DDValues->G;

    std::string titleStr = computerFileName(DDValues, "Absorption_Cross_Section");
    std::ofstream outFile(titleStr);

    if (!outFile.is_open())
    {
        throw std::runtime_error("Error opening file: " + titleStr);
    }

    DDResult->absorptionCrossSectionFile = titleStr;

    // Iterate over zones
    for (size_t zoneIndex = 0; zoneIndex < zoneNumber; ++zoneIndex)
    {
        // Write zoneIndex as table title
        outFile << "Zone " << zoneIndex + 1 << std::endl;

        // Write table headers
        outFile << std::left << std::setw(15) << "Energy Group" << "Absorption Cross-Section" << std::endl;

        // Iterate over energy groups within each zone
        for (size_t group = 0; group < groupNumber; ++group)
        {
            // Write the energy group and corresponding absorption cross-section
            outFile << std::left << std::setw(15) << group + 1
                    << absorptionCrossSection[zoneIndex][group] << std::endl;
        }

        // Add a newline for separation between zones if there are multiple zones
        if (zoneIndex < DDResult->absorptionCrossSection.size() - 1)
        {
            outFile << std::endl;
        }
    }

    outFile.close();
}

void BuildMatrices::writeNeutronFluxFile(dados_entrada *DDValues, CalculatedData *DDResult)
{
    if (nullptr == DDValues->FLUXO_ESCALAR)
    {
        throw std::runtime_error("NeutronFlux Matrix is null.");
    }

    std::string titleStr = computerFileName(DDValues, "Scalar_Flux");
    std::ofstream output(titleStr);

    if (!output.is_open())
    {
        throw std::runtime_error( "Error opening file: " + titleStr);
    }

    DDResult->scalarFluxFile = titleStr;

    int colWidth = 25;
    int precision = 2;

    //Write compile information
    output << std::string(colWidth * 3, '-') << std::endl;
    output << "Iteration Number: "
             << DDValues->iteracaoFinal<<"\nTime: "
             << DDValues->tempoFinalDeProcessamento<<"s\n";

    output << std::string(colWidth * 3, '-') << std::endl;

    // Write table headers
    output << std::left << std::setw(colWidth) << "Position x (cm)"
             << std::setw(colWidth) << "Group"
             << std::setw(colWidth) << "Scalar Neutron Flux" << std::endl;
    output << std::string(colWidth * 3, '-') << std::endl;

    double t = 0;
    int nod  = 0;
    double totalRegionSize = 0.0;

    for (int r = 0; r < DDValues->n_R; ++r)
        totalRegionSize += DDValues->TAM[r];

    while (t <= totalRegionSize)
    {
        for (int g = 0; g < DDValues->G; ++g)
        {
            if (g == 0)
            {
                output << std::left << std::setw(colWidth) << std::fixed << std::setprecision(precision) << t
                         << std::setw(colWidth) << g + 1
                         << std::fixed << std::setprecision(15) << DDValues->FLUXO_ESCALAR[g][nod] << std::endl;
            }
            else
            {
                output << std::left << std::setw(colWidth) << ""
                         << std::setw(colWidth) << g + 1
                         << std::fixed << std::setprecision(15) << DDValues->FLUXO_ESCALAR[g][nod] << std::endl;
            }
        }

        t += DDValues->periodicidade;
        nod += static_cast<int>((DDValues->NODOSX * DDValues->periodicidade) / DDValues->TAM_TOTAL);
        output << std::string(colWidth * 3, '-') << std::endl;
    }

}

void BuildMatrices::writeScatteringCrossSectionFile(dados_entrada *DDValues, CalculatedCrossSectionMatrices *DDResult)
{
    if (DDResult->scatteringCrossSection.empty())
    {
        throw std::runtime_error("scattering Cross Section Matrix is empty.");
    }

    auto scttCrossSection = DDResult->scatteringCrossSection;
    auto zoneNumber       = DDValues->n_Z;
    auto groupNumber      = DDValues->G;

    std::string titleStr = computerFileName(DDValues, "Scattering_Cross_Section");
    std::ofstream output(titleStr);

    if (!output.is_open())
    {
        throw std::runtime_error( "Error opening file: " + titleStr);
    }

    DDResult->scatteringCrossSectionFile = titleStr;

    // Iterate over zones
    for (size_t zoneIndex = 0; zoneIndex < zoneNumber; ++zoneIndex)
    {
        // Write zoneIndex as table title
        output << "Zone " << zoneIndex << std::endl;
        // Write table headers
        output << std::left << std::setw(15) << "Energy Group" << "Scattering Cross-Section" << std::endl;

        // Iterate over energy groups within each zone
        for (size_t group = 0; group < groupNumber; ++group)
        {
            // Write the energy group and corresponding absorption cross-section
            output << std::left << std::setw(15) << group + 1 << scttCrossSection[zoneIndex][group] << std::endl;
        }

        // Add a newline for separation between zones if there are multiple zones
        if (zoneIndex < DDResult->absorptionCrossSection.size() - 1)
        {
            output << std::endl;
        }
    }

    output.close();
}
