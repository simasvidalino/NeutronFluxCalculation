#include "DataMatrices.h"
#include "GausLegendreQuadrature.h"
#include "LegendrePolynomial.h"
#include "get_GQ.h"

#include <algorithm>
#include <filesystem>
#include <numeric>
#include <stdexcept>

#include <QCoreApplication>
#include <QFile>
#include <QTemporaryFile>
#include <QTextStream>

#define SCALAR_NEUTRON_FLUX_FILE_NAME "Scalar_Flux"
#define ABSORPTION_NEUTRON_RATE_FILE_NAME "Absorption_Neutron_Rate"
#define DATA_VISUALIZATION_PERIODICITY 1

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

void BuildMatrices::calculateAbsorptionRateData(dados_entrada *data, CalculatedData *DDResult)
{
    calculateAbsorptionRatePerNode(data, DDResult);
    calculateAverageAbsorptionRatePerRegion(data, DDResult);
    calculateIntegratedAbsorptionRatePerRegion(data, DDResult);
    calculateIntegratedAbsorptionRatePerZone(data, DDResult);
    calculateTotalAbsorptionRatePerGroupPerRegion(data, DDResult);
    calculateTotalAbsorptionRatePerRegion(data, DDResult);
}

void BuildMatrices::calculateScalarNeutronFluxData(dados_entrada *data, CalculatedData *DDResult)
{
    calculateAverageNeutronFluxPerGroupPerRegion(data, DDResult);
    calculateIntegratedNeutronFluxPerGroupPerRegion(data,DDResult);
    calculateTotalNeutronFluxPerRegion(data, DDResult);
    calculateTotalNeutronFluxPerGroupPerRegion(data, DDResult);
}

std::unique_ptr<dados_entrada> BuildMatrices::copyProjectDataToRawPointers(ProjectData &proj, CrossSectionDataFilerParameters& fileParameter)
{
    auto data = std::make_unique<dados_entrada>();
    fileName  = proj.neutronMacroscopicCrossSectionsFilePath;
    std::filesystem::directory_entry entry{fileName};

    if (   ( true == fileName.empty() )
        || (    ( false == entry.exists() )
             && ( fileName != ":/Default_Project/Resources/Default_Project.txt") ) )
    {
        throw std::invalid_argument("Error: Material Data File issue. \nYou need to set a Cross Section File.");
    }

    dataVisualizationType = proj.dataVisualizationType;

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

    data->tipo_criterio_parada = proj.stoppingCriteriaType;

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
void BuildMatrices::calculateAverageAbsorptionRatePerRegion(dados_entrada *data, CalculatedData *DDResult)
{
    if (DDResult->matrices.absorptionCrossSection.empty())
        throw std::invalid_argument("Error: Absorption Cross Section Matrix is empty");

    if (DDResult->nodalScalarFlux.empty())
        throw std::invalid_argument("Error: Scalar Flux Matrix is empty");

    std::vector<std::vector<long double>> absorptionRate(data->n_R, std::vector<long double>(data->G, 0.0));

    for (int gIndex = 0; gIndex < data->G; ++gIndex)
    {
        int nodeIndex = 0;

        for (int rIndex = 0; rIndex < data->n_R; ++rIndex)
        {
            const int zIndex                 = data->Map_R[rIndex] - 1;
            const int nodesInRegion          = data->n_nodos[rIndex];
            const long double sigmaAbs       = DDResult->matrices.absorptionCrossSection[zIndex][gIndex];

            long double regionGroupSum = 0.0;

            for (int n = 0; n < nodesInRegion; ++n, ++nodeIndex)
            {
                long double flux = DDResult->cellAverageScalarFlux[gIndex][nodeIndex];
                regionGroupSum += flux;
            }

            absorptionRate[rIndex][gIndex] = ((sigmaAbs * regionGroupSum) / nodesInRegion);
        }
    }

    DDResult->averageAbsorptionRatePerRegion.swap(absorptionRate);
}

void BuildMatrices::calculateIntegratedAbsorptionRatePerRegion( dados_entrada *data, CalculatedData *DDResult )
{
    if (DDResult->matrices.absorptionCrossSection.empty())
    {
        throw std::invalid_argument("Error: Absorption Cross Section Matrix is empty");
    }

    if (DDResult->cellAverageScalarFlux.empty())
    {
        throw std::invalid_argument("Error: Average Scalar Flux Matrix in cell is empty");
    }

    std::vector<std::vector<long double>> absorptionRate(data->n_R, std::vector<long double>(data->G, 0.0L));
    int nodeIndex = 0;

    for (int rIndex = 0; rIndex < data->n_R; ++rIndex)
    {
        const long double step  = data->PASSO[rIndex];
        const int zIndex        = data->Map_R[rIndex] - 1;
        const int nodesInRegion = data->n_nodos[rIndex];

        for (int n = 0; n < nodesInRegion; ++n)
        {
            for (int gIndex = 0; gIndex < data->G; ++gIndex)
            {
                long double sigmaAbs    = DDResult->matrices.absorptionCrossSection[zIndex][gIndex];
                long double fluxCurrent = DDResult->cellAverageScalarFlux[gIndex][nodeIndex];
                absorptionRate[rIndex][gIndex] += sigmaAbs * fluxCurrent * step;
            }

            ++nodeIndex;
        }
    }

    DDResult->integratedAbsorptionRatePerGroupPerRegion.swap(absorptionRate);
}

void BuildMatrices::calculateIntegratedAbsorptionRatePerZone( dados_entrada *data, CalculatedData *DDResult)
{
    if (DDResult->integratedAbsorptionRatePerGroupPerRegion.empty())
    {
        throw std::invalid_argument("Error: integrated Absorption Rate Per Group Per Region Matrix is empty");
    }

    std::vector<long double> integratedAbsorptionRatePerRegion(data->n_R, 0.0L);

    for (int rIndex = 0; rIndex < data->n_R; ++rIndex)
    {
        for (int gIndex = 0; gIndex < data->G; ++gIndex)
        {
            integratedAbsorptionRatePerRegion[rIndex] += DDResult->integratedAbsorptionRatePerGroupPerRegion[rIndex][gIndex];
        }
    }

    std::vector<long double> sumPerZone(data->n_Z, 0.0L);

    for (int rIndex = 0; rIndex < data->n_R; ++rIndex)
    {
        const int zIndex = data->Map_R[rIndex] - 1;
        sumPerZone[zIndex] += integratedAbsorptionRatePerRegion[rIndex];
    }

    DDResult->integratedAbsorptionRatePerZone.swap(sumPerZone);
    DDResult->integratedAbsorptionRatePerRegion.swap(integratedAbsorptionRatePerRegion);
}

void BuildMatrices::calculateTotalAbsorptionRatePerGroupPerRegion(dados_entrada *data, CalculatedData *DDResult)
{
    if (DDResult->absorptionRatePerNode.empty())
        throw std::invalid_argument("Error: Absorption Rate Per Node Matrix is empty");

    std::vector<std::vector<long double>> absorptionRate(data->n_R, std::vector<long double>(data->G, 0.0L));
    int nodeIndex = 0;

    for (int r = 0; r < data->n_R; ++r)
    {
        for (int n = 0; n < data->n_nodos[r]; ++n)
        {
            for (int g = 0; g < data->G; ++g)
            {
                absorptionRate[r][g] += DDResult->absorptionRatePerNode[g][nodeIndex];
            }

            nodeIndex++;
        }
    }

    DDResult->totalAbsorptionRatePerGroupPerRegion.swap(absorptionRate);
}

void BuildMatrices::calculateAbsorptionRatePerNode(dados_entrada *data, CalculatedData *DDResult)
{
    if (DDResult->matrices.absorptionCrossSection.empty())
        throw std::invalid_argument("Error: Absorption Cross Section Matrix is empty");

    if (DDResult->nodalScalarFlux.empty())
        throw std::invalid_argument("Error: Nodal Scalar Flux Matrix is empty");

    std::vector<std::vector<long double>> absorptionRatePerNode(data->G, std::vector<long double>(data->NODOSX + 1, 0.0L));
    int nodeIndex = 0;

    for (int r = 0; r < data->n_R; ++r)
    {
        const int zone = data->Map_R[r] - 1;
        const int nodeRegion = data->n_nodos[r];

        for (int n = 0; n <= nodeRegion && data->NODOSX >= nodeIndex; ++n)
        {
            for (int g = 0; g < data->G; ++g)
            {
                absorptionRatePerNode[g][nodeIndex] = DDResult->matrices.absorptionCrossSection[zone][g]
                                                      * DDResult->nodalScalarFlux[g][nodeIndex];
            }

            nodeIndex++;
        }
    }

    DDResult->absorptionRatePerNode.swap(absorptionRatePerNode);
}

void BuildMatrices::calculateTotalAbsorptionRatePerRegion(dados_entrada *data, CalculatedData *DDResult)
{
    if (DDResult->absorptionRatePerNode.empty())
        throw std::invalid_argument("Error: Absorption Rate Per Node Matrix is empty");

    std::vector<long double> absorptionRate(data->n_R, 0.0L);

    int nodeIndex          = 0;
    long double regionFlux = 0.0L;

    for (int r = 0; r < data->n_R; ++r)
    {
        regionFlux = 0.0L;
        const int nodeRegion = data->n_nodos[r];

        for (int n = 0; n <= nodeRegion && data->NODOSX >= nodeIndex; ++n)
        {
            for (int g = 0; g < data->G; ++g)
            {
                regionFlux += DDResult->absorptionRatePerNode[g][nodeIndex];
            }

            nodeIndex++;
        }

        absorptionRate[r] = regionFlux;
    }

    DDResult->totalAbsorptionRatePerRegion.swap(absorptionRate);
}

void BuildMatrices::calculateAverageNeutronFluxPerGroupPerRegion(dados_entrada *data, CalculatedData *DDResult)
{
    if (DDResult->cellAverageScalarFlux.empty())
        throw std::invalid_argument("Error: cell Average Scalar Flux Matrix is empty");

    std::vector<std::vector<long double>> averageFlux(data->n_R, std::vector<long double>(data->G, 0.0L));

    int nodeIndex = 0;

    for (int r = 0; r < data->n_R; ++r)
    {
        const int nodesInRegion = data->n_nodos[r];

        for (int n = 0; n < nodesInRegion; ++n)
        {
            for (int g = 0; g < data->G; ++g)
            {
                averageFlux[r][g] += DDResult->cellAverageScalarFlux[g][nodeIndex];
            }

            nodeIndex++;
        }

        for (int g = 0; g < data->G; ++g)
        {
            averageFlux[r][g] /= nodesInRegion;
        }
    }

    DDResult->averageNeutronFluxPerGroupPerRegion.swap(averageFlux);
}

void BuildMatrices::calculateAverageNeutronFluxPerRegion(dados_entrada *data, CalculatedData *DDResult)
{

}

void BuildMatrices::calculateIntegratedNeutronFluxPerGroupPerRegion(dados_entrada *data, CalculatedData *DDResult)
{
    if (DDResult->cellAverageScalarFlux.empty())
        throw std::invalid_argument("Error: Cell Average Scalar Flux Matrix is empty");

    std::vector<std::vector<long double>> integratedFlux(data->n_R, std::vector<long double>(data->G, 0.0L));

    int nodeIndex = 0;

    for (int rIndex = 0; rIndex < data->n_R; ++rIndex)
    {
        const long double step = data->PASSO[rIndex];

        for (int n = 0; n < data->n_nodos[rIndex]; ++n)
        {            
            for (int gIndex = 0; gIndex < data->G; ++gIndex)
            {
                integratedFlux[rIndex][gIndex] += DDResult->cellAverageScalarFlux[gIndex][nodeIndex] * step;
            }

            ++nodeIndex;
        }

    }

    DDResult->integratedNeutronFluxPerGroupPerRegion.swap(integratedFlux);
}
//Cálculo inutil
void BuildMatrices::calculateTotalNeutronFluxPerGroupPerRegion(dados_entrada *data, CalculatedData *DDResult)
{
    if (DDResult->cellAverageScalarFlux.empty())
        throw std::invalid_argument("Error: Cell Average Scalar Flux Matrix is empty");

    std::vector<std::vector<long double>> flux(data->n_R, std::vector<long double>(data->G, 0.0L));
    int nodeIndex = 0;

    for (int r = 0; r < data->n_R; ++r)
    {
        for (int n = 0; n < data->n_nodos[r]; ++n)
        {
            for (int g = 0; g < data->G; ++g)
            {
                flux[r][g] += DDResult->cellAverageScalarFlux[g][nodeIndex];
            }

            nodeIndex++;
        }
    }

    DDResult->totalNeutronFluxPerGroupPerRegion.swap(flux);
}

void BuildMatrices::calculateTotalNeutronFluxPerRegion(dados_entrada *data, CalculatedData *DDResult)
{
    if (DDResult->averageNeutronFluxPerGroupPerRegion.empty())
        throw std::invalid_argument("Error: Average Scalar Flux per Group per Region Matrix is empty");

    std::vector<long double> fluxRegion(data->n_R, 0.0L);

    for (int r = 0; r < data->n_R; ++r)
    {
        long double flux = 0.0L;

        for (int g = 0; g < data->G; ++g)
        {
            flux += DDResult->averageNeutronFluxPerGroupPerRegion[r][g];
        }

        fluxRegion[r] = flux;
    }

    DDResult->totalNeutronFluxPerRegion.swap(fluxRegion);
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

    for (int zIndex = 0; zIndex < data->n_Z; ++zIndex)
    {
        std::vector<long double> scattering_g;

        for (int gIndex = 0; gIndex < data->G; ++gIndex)
        {
            long double sum = 0.0; // Initialize the sum for scattering cross-section for group gIndex

            for (int gLineIndex = 0; gLineIndex < data->G; ++gLineIndex) // Loop over source groups (g')
            {
                long double value = data->s_s[gLineIndex][gIndex][zIndex][0]; //only l = 0 metters

                if (value > 0.0L)
                {
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

    // std::cout<<"Main data ordem da quadratura "<< valor.n
    //           << "\nordem de parada " << valor.ordem_parada
    //           <<"\nOrdem de iteracao " << valor.iteracao
    //           << "\nGropu de energia "<<valor.G
    //           << "\n valor.L "<<valor.L
    //           <<"\nNumero de zonas"<<valor.n_Z
    //           <<"\nNumero de Região"<<valor.n_R
    //           <<std::endl;


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
    valor.PASSO = new long double [valor.n_R]; //modulo entre nodos
    // valor.TAM_TOTAL = 0;  //comprimento total de x
    valor.NODOSX = 0;  // numero total de nodos por regiao

    for (int j = 0; j < valor.n_R; j++)
    {
        valor.NODOSX   += valor.n_nodos[j];
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

    for (auto &v : vector)
    {
        if (v < 0.0)
        {
            std::cout << "Negative cross section value detected. It will be replaced with zero." << std::endl;
            //v = 0.0;
        }
    }

    file.close(); // Close the file

    return vector;
}

int BuildMatrices::getDataVisualizationType() const
{
    return dataVisualizationType;
}

void BuildMatrices::setDataVisualizationType(int newDataVisualizationType)
{
    dataVisualizationType = newDataVisualizationType;
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

std::vector<std::vector<long double>> BuildMatrices::calculateAbsorptionCrossSectionMatrix(dados_entrada *data,
                                                                                           std::vector<std::vector<long double>>& sigmaScattering)
{
    if (data->s_t == nullptr || sigmaScattering.empty())
    {
        throw std::invalid_argument(
            "Error: Invalid input data. Ensure sigmaTotal and sigmaScattering are correctly initialized.");
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

    // for (auto v:vector)
    //     std::cout<<v<< std::endl;

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
        //std::cout<<"\nZona "<<h<<std::endl;

        for (int j = 0; j < data->G; j++)
        {
            data->s_t[j][h] = vector[i];
            i++;
        }

        for (int k = 0; k < data->L + 1; k++)
        {
            //std::cout<<"\nLegendre "<<k<<std::endl;
            for (int m = 0; m < data->G; m++)
            {
                for (int n = 0; n<data->G; n ++)
                {
                    data->s_s[m][n][h][k] = vector[i];
                    //std::cout<<data->s_s[m][n][h][k] <<" "<<std::endl;

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

    // legendre_set ( data->n, data->mi, data->w);
    auto [miVec, wVec] = getQuadratureValues(data->n);

    for (int i = 0; i < data->n ; ++i)
    {
        data->mi[i] = miVec[i];
        data->w[i]  = wVec[i];
    }

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
    data->TAM_TOTAL = 0; //comprimento total de x
    data->NODOSX    = 0; // numero total de nodos por regiao

    for (int j = 0; j < data->n_R; j++)
    {
        data->PASSO[j] = data->TAM[j] / data->n_nodos[j];
        data->NODOSX   += data->n_nodos[j];
        data->TAM_TOTAL += data->TAM[j];
    }

    //fluxo angular e fluxo escalar
    data->FLUXO_ANGULAR          = new long double **[data->G];
    data->smgi                   = new long double **[data->G];
    data->FLUXO_ESCALAR          = new long double *[data->G];

    for (int g = 0; g < data->G; g++)
    {
        data->FLUXO_ANGULAR[g]          = new long double *[(data->NODOSX) + 1];
        data->smgi[g]                   = new long double *[data->NODOSX];
        data->FLUXO_ESCALAR[g]          = new long double[(data->NODOSX) + 1];

        for (int o = 0; o <= data->NODOSX; o++)
        {
            data->FLUXO_ANGULAR[g][o]          = new long double[data->n];
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

std::tuple<std::vector<long double>, std::vector<long double>> BuildMatrices::getQuadratureValues(int NWanted)
{
    std::vector<long double> mu_values;
    std::vector<long double> w_values;

    std::filesystem::path jsonPath(fileName);
    std::filesystem::path parentDir = jsonPath.parent_path();
    std::filesystem::path csvPath = parentDir / "Quadrature.csv";
    std::filesystem::path path = std::filesystem::u8path(csvPath.string());
    std::ifstream file(path);

    bool quadratureFound = false;

    if (!file.is_open())
    {
        auto quad = get_GQ(NWanted);

        saveQuadratureValueInCSV(quad, NWanted, path);

        return quad;
    }

    std::string line;

    while (std::getline(file, line))
    {
        line.erase(remove_if(line.begin(), line.end(), isspace), line.end());

        if (line.rfind("N=", 0) == 0)
        {
            int NRead = std::stoi(line.substr(2));

            if (NRead == NWanted)
            {
                std::cout <<"Recovering Data from File of Quadrature order = " << NWanted << std::endl;;

                for (int i = 0; i < NRead; ++i)
                {
                    if (std::getline(file, line))
                    {
                        std::istringstream iss(line);
                        double mu, w;
                        char comma;

                        if (iss >> mu >> comma >> w)
                        {
                            mu_values.push_back(mu);
                            w_values.push_back(w);
                        }
                    }
                }
                quadratureFound = true;
                break;
            }
            else
            {
                for (int i = 0; i < NRead; ++i)
                {
                    std::getline(file, line);
                }
            }
        }
    }

    if (false == quadratureFound)
    {
        std::tie(mu_values, w_values) = get_GQ(NWanted);
        saveQuadratureValueInCSV(std::make_tuple(mu_values, w_values), NWanted, path);
    }

    file.close();

    return std::make_tuple(mu_values, w_values);
}

void BuildMatrices::saveQuadratureValueInCSV(const std::tuple<std::vector<long double>, std::vector<long double>> &value,
                                             int NNew,
                                             std::filesystem::path filePath)
{
    std::ofstream file(filePath, std::ios::app);

    if (!file.is_open())
    {
        std::cerr << "Error opening file: " << filePath << std::endl;
        return;
    }

    file << "N=" << NNew << std::endl;

    const auto &mu_values = std::get<0>(value);
    const auto &w_values  = std::get<1>(value);

    if ((mu_values.size() != static_cast<size_t>(NNew)) || (w_values.size() != static_cast<size_t>(NNew)))
    {
        std::cerr << "Size of mu or w does not match NNew" << std::endl;
        file.close();
        return;
    }

    std::cout <<"Quadrature order = " << NNew << std::endl;;

    for (int i = 0; i < NNew; ++i)
    {
        std::cout << std::fixed << std::setprecision(30) << "Saving quadrature data: " << mu_values[i] << std::endl;
        file << std::setprecision(30) << mu_values[i] << "," << std::setprecision(30) << w_values[i] << std::endl;
    }

    file.close();
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
        copyResourceToDestination(oldPath, destinationPath.string());
    }

    return destinationPath.string();
}

void BuildMatrices::writeHTMLAbsorptionRateData(dados_entrada *DDValues, CalculatedData *DDResult)
{
    if (dataVisualizationType == DATA_VISUALIZATION_PERIODICITY)
    {
        writeHTMLAbsorptionRateFilePerPeridiocity(DDValues, DDResult);
    }
    else
    {
        writeHTMLAbsorptionRateFilePerRegionInterface(DDValues, DDResult);
    }
}

void BuildMatrices::writeHTMLScalarNeutronFluxData(dados_entrada *DDValues, CalculatedData *DDResult)
{
    if (dataVisualizationType == DATA_VISUALIZATION_PERIODICITY)
    {
        writeHTMLNeutronFluxFilePerPeridiocity(DDValues, DDResult);
    }
    else
    {
        writeHTMLNeutronFluxFilePerRegionInterface(DDValues, DDResult);
    }
}

void BuildMatrices::writeHTMLCrossSectionMatrices(dados_entrada *DDValues, CalculatedCrossSectionMatrices *matrices)
{
    writeHTMLAbsorptionCrossSectionFile(DDValues, matrices);
    writeHTMLScatteringCrossSectionFile(DDValues, matrices);
}

void BuildMatrices::writeTXTAbsorptionRateData(dados_entrada *DDValues, CalculatedData *DDResult)
{
    writeAbsorptionRateFile(DDValues, DDResult);
    writeIntegratedAbsorptionRatePerRegionFile(DDValues, DDResult);
    writeAverageAbsorptionRateFile(DDValues, DDResult);
}

void BuildMatrices::writeTXTScalarNeutronFluxData(dados_entrada *DDValues, CalculatedData *DDResult)
{
    writeNeutronFluxFile(DDValues, DDResult);
    writeAverageNeutronFluxPerRegion(DDValues, DDResult);
    writeintegratedNeutronFluxPerGroupPerRegion(DDValues, DDResult);
}

void BuildMatrices::writeHTMLNeutronFluxFilePerPeridiocity(dados_entrada *DDValues, CalculatedData *DDResult)
{
    auto title = computerFileNameHtm(DDValues, SCALAR_NEUTRON_FLUX_FILE_NAME);
    std::ofstream out(title);

    if (!out.is_open())
    {
        throw std::runtime_error("Error opening file: " + title.string());
    }

    DDResult->scalarFluxFile = title.string();

    const int precisionPos   = 2;
    const int precisionVal   = 15;
    std::ostringstream posFmt, valFmt;

    out << R"(<!doctype html>
            <html lang="en"><head><meta charset="utf-8">
            <style>
            html,body{margin:0;padding:16px;background:#121212;color:#ddd;font:14px/1.4 system-ui,Segoe UI,Arial,sans-serif}
            table{border-collapse:collapse;width:100%;margin-top:8px}
            th,td{border:1px solid #555;padding:6px 10px}
            th{text-align:center;background:#1f1f1f;color:#ddd}
            td.num{text-align:right;font-family:ui-monospace,Consolas,monospace}
            td.grp{text-align:center}
            caption{caption-side:top;text-align:left;margin:8px 0;font-weight:600}
            h2{margin-top:24px}
            hr{margin:24px 0;border:0;border-top:1px solid #444}
            </style>
            </head><body>)";

    // Title section: Outputs the main report header
    out << "<h1>Neutron Flux Report</h1>\n";

    // Preformatted block for iteration and time info
    out
        << "<pre style='white-space:pre-wrap;background:#1b1b1b;border:1px solid #444;"
           "padding:8px;border-radius:8px'>"
        << "Iteration Number: "
        << DDValues->iteracaoFinal
        << "\n"
        << "Time: "
        << std::fixed
        << std::setprecision(3)
        << DDValues->tempoFinalDeProcessamento
        << "s\n"
        << "</pre>\n"
        << "<hr/>\n";

    // Inline CSS for table styling: Ensures tables are responsive, full-width, and scrollable if needed
    // Added fixed row height (tr { height: 40px; }) for consistent row sizing
    // Added white-space: nowrap to prevent text wrapping, allowing columns to expand horizontally to the right
    out << "<style>"
           "table { width: 100%; border-collapse: collapse; margin-bottom: 20px; overflow-x: auto; display: block; }"
           "th, td { border: 1px solid #ddd; padding: 8px; text-align: right; white-space: nowrap; }" // No wrapping: Expands columns to the right instead of increasing height
           "tr { height: 40px; }" // Fixed row height for consistent layout
           "th { background-color: #f2f2f2; }"
           "caption { font-weight: bold; margin-bottom: 10px; }"
           ".num { text-align: right; }"
           ".grp { text-align: center; }"
           "</style>\n";

    // Calculate total region size by summing up all region sizes (used to limit the loop to the physical domain, e.g., 18 cm)
    double totalRegionSize = 0.0;
    for (int r = 0; r < DDValues->n_R; ++r)
    {
        totalRegionSize += DDValues->TAM[r];
    }

    out << "<h2>Nodal Scalar Flux </h2>\n";
    out << "<table><thead><tr><th>Position x (cm)</th>";
    for (size_t g = 0; g < DDValues->G; ++g)
    {
        out << "<th>G " << (g + 1) << "</th>"; // Shortened header to "G X" as per user change
    }
    out << "</tr></thead><tbody>\n";

    double t            = 0.0; // Starting position
    int nod             = 0;   // Starting node index
    int nodIncrement = static_cast<int>((static_cast<double>(DDValues->NODOSX) * DDValues->periodicidade)
                          / DDValues->TAM_TOTAL); // Proportional increment to skip nodes based on periodicity

    if (nodIncrement < 1)
    {
        QString warning = QString("Warning: Node increment is too small (may not advance nod correctly): nodIncrement = (%1 * %2) / %3 = %4."
                                  " Suggestion: Increase the number of node to make nodIncrement >=1.")
                              .arg(DDValues->NODOSX)
                              .arg(DDValues->periodicidade)
                              .arg(totalRegionSize)
                              .arg(nodIncrement);

        throw std::invalid_argument(warning.toStdString());
    }

    // Loop over positions while within total region size, incrementing by periodicity
    while (t <= totalRegionSize)
    {
        posFmt.str("");
        posFmt.clear();
        posFmt << std::fixed << std::setprecision(precisionPos) << t;

        out << "<tr><td class='num'>" << posFmt.str() << "</td>";

        for (size_t g = 0; g < DDValues->G; ++g)
        {
            valFmt.str("");
            valFmt.clear();
            valFmt << std::scientific << std::setprecision(precisionVal) << DDResult->nodalScalarFlux[g][nod];
            out << "<td class='num'>" << valFmt.str() << "</td>";
        }
        out << "</tr>\n";

        t += DDValues->periodicidade;          // Increment position by periodicity
        nod += nodIncrement; // Increment node index proportionally
    }

    out << "</tbody></table><hr/>\n";

    // // Cell-Average Scalar Flux section: Table with positions at cell centers, sampled by periodicity, groups as columns
    // out << "<h2>Cell-Average Scalar Flux at Cell Centers</h2>\n";
    // out << "<table><thead><tr><th>Cell Center x (cm)</th>";
    // for (size_t g = 0; g < DDValues->G; ++g)
    // {
    //     out << "<th>G " << (g + 1) << "</th>"; // Shortened header to "G X" as per user change
    // }
    // out << "</tr></thead><tbody>\n";

    // int cell = 0;
    // t = 0.0;
    // double next_sample_pos = 0.0;

    // for (size_t r = 0; r < DDValues->n_R; ++r)
    // {
    //     double region_dx = DDValues->PASSO[r];
    //     int nodes_in_region = DDValues->n_nodos[r];
    //     int cells_in_region = nodes_in_region;

    //     double region_start_pos = t;

    //     for (int node = 0; node < cells_in_region; ++node)
    //     {
    //         double cell_start = region_start_pos + node * region_dx;
    //         double cell_center = cell_start + (region_dx / 2.0);

    //         // Check if this cell center aligns with periodic sampling
    //         if (cell_center >= next_sample_pos - 1e-10)
    //         {
    //             double cell_end = cell_start + region_dx;

    //             posFmt.str("");
    //             posFmt.clear();
    //             posFmt << std::fixed << std::setprecision(precisionPos) << "["
    //                    << cell_start << "," << cell_end << "]";

    //             out << "<tr><td class='num'>" << posFmt.str() << "</td>";

    //             for (size_t g = 0; g < DDValues->G; ++g)
    //             {
    //                 valFmt.str("");
    //                 valFmt.clear();
    //                 valFmt << std::scientific << std::setprecision(precisionVal) << DDResult->cellAverageScalarFlux[g][cell];
    //                 out << "<td class='num'>" << valFmt.str() << "</td>";
    //             }
    //             out << "</tr>\n";

    //             next_sample_pos += DDValues->periodicidade;
    //         }

    //         cell++;
    //         if (cell > DDValues->NODOSX)
    //         {
    //             break;
    //         }
    //     }

    //     t += nodes_in_region * region_dx;
    // }

    // out << "</tbody></table><hr/>\n";

    // Average Neutron Flux per Region: Single table with regions as rows, groups as columns
    out << "<h2>Average Neutron Flux per Region</h2>\n";
    out << "<table><thead><tr><th>Region</th>";
    for (size_t g = 0; g < DDValues->G; ++g)
    {
        out << "<th>G " << (g + 1) << "</th>"; // Shortened header to "G X" as per user change
    }
    out << "</tr></thead><tbody>\n";

    // Loop over regions to fill the table
    for (size_t r = 0; r < DDValues->n_R; ++r)
    {
        out << "<tr><td class='grp'>Region " << (r + 1) << "</td>";
        for (size_t g = 0; g < DDValues->G; ++g)
        {
            valFmt.str("");
            valFmt.clear();
            valFmt << std::scientific << std::setprecision(precisionVal) << DDResult->averageNeutronFluxPerGroupPerRegion[r][g];
            out << "<td class='num'>" << valFmt.str() << "</td>";
        }
        out << "</tr>\n";
    }
    out << "</tbody></table><hr/>\n";

    // Integrated Neutron Flux per Region: Similar single table with regions as rows, groups as columns
    out << "<h2>Integrated Neutron Flux per Region</h2>\n";
    out << "<table><thead><tr><th>Region</th>";

    for (size_t g = 0; g < DDValues->G; ++g)
    {
        out << "<th>G " << (g + 1) << "</th>"; // Shortened header to "G X" as per user change
    }
    out << "</tr></thead><tbody>\n";

    // Loop over regions to fill the table
    for (size_t r = 0; r < DDValues->n_R; ++r)
    {
        out << "<tr><td class='grp'>Region " << (r + 1) << "</td>";
        for (size_t g = 0; g < DDValues->G; ++g)
        {
            valFmt.str("");
            valFmt.clear();
            valFmt << std::scientific << std::setprecision(precisionVal) << DDResult->integratedNeutronFluxPerGroupPerRegion[r][g];
            out << "<td class='num'>" << valFmt.str() << "</td>";
        }
        out << "</tr>\n";
    }
    out << "</tbody></table><hr/>\n";

    // Angular Flux section: Added here as per user request, with tables per group for angular flux
    // Loop over groups for angular flux
    for (size_t g = 0; g < DDValues->G; ++g)
    {
        out << "<h2>Angular Flux for Group " << (g + 1) << "</h2>\n";
        out << "<table><thead><tr><th>Position x (cm)</th>";

        // Headers for directions with shortened format "D X (mu=Y.YY)"
        for (int o = 0; o < DDValues->n; ++o)
        {
            // Assume DDValues->mi[o] holds the mu (cosine) values from quadrature; adjust if different
            double mu = DDValues->mi[o]; // Replace with actual mu access (e.g., from get_GQ())
            std::ostringstream muFmt;
            muFmt << std::fixed << std::setprecision(precisionPos) << mu;
            out << "<th>D " << (o + 1) << " (mu=" << muFmt.str() << ")</th>";
        }
        out << "</tr></thead><tbody>\n";

        // Sample nodes respecting periodicity
        t            = 0.0; // Reset starting position
        nod          = 0;   // Reset starting node index

        while (t <= totalRegionSize)
        {
            posFmt.str("");
            posFmt.clear();
            posFmt << std::fixed << std::setprecision(precisionPos) << t;

            out << "<tr><td class='num'>" << posFmt.str() << "</td>";
            for (int o = 0; o < DDValues->n; ++o)
            {
                valFmt.str("");
                valFmt.clear();
                valFmt
                    << std::scientific
                    << std::setprecision(precisionVal)
                    << DDValues->FLUXO_ANGULAR[g][nod][o]; // Access angular flux (note: using DDValues as per allocation)
                out << "<td class='num'>" << valFmt.str() << "</td>";
            }
            out << "</tr>\n";

            t += DDValues->periodicidade;          // Increment position by periodicity
            nod += nodIncrement; // Increment node index proportionally

            if (nod > DDValues->NODOSX)
            {
                break;
            }
        }

        out << "</tbody></table><hr/>\n";
    }

    // Close the HTML body
    out << "</body></html>";

    out.close();
}

void BuildMatrices::writeHTMLAbsorptionRateFilePerPeridiocity(dados_entrada *DDValues, CalculatedData *DDResult)
{
    if (DDResult->absorptionRatePerNode.empty())
        throw std::runtime_error("Absorption rate matrices are empty.");

    auto title = computerFileNameHtm(DDValues, ABSORPTION_NEUTRON_RATE_FILE_NAME);
    std::ofstream out(title);

    if (!out.is_open())
        throw std::runtime_error("Error opening file: " + title.string());

    DDResult->absorptionRateFile = title.string();

    const int precisionPos = 2;
    const int precisionVal = 15;
    std::ostringstream posFmt, valFmt;

    out << R"(<!doctype html>
            <html lang="en"><head><meta charset="utf-8">
            <style>
            html,body{margin:0;padding:16px;background:#121212;color:#ddd;font:14px/1.4 system-ui,Segoe UI,Arial,sans-serif}
            table{border-collapse:collapse;width:100%;margin-top:8px}
            th,td{border:1px solid #555;padding:6px 10px}
            th{text-align:center;background:#1f1f1f;color:#ddd}
            td.num{text-align:right;font-family:ui-monospace,Consolas,monospace}
            td.grp{text-align:center}
            caption{caption-side:top;text-align:left;margin:8px 0;font-weight:600}
            h2{margin-top:24px}
            hr{margin:24px 0;border:0;border-top:1px solid #444}
            </style>
            </head><body>)";

    // Title section: Outputs the main report header
    out << "<h1>Absorption Rate Report</h1>\n";

    // Inline CSS for table styling: Ensures tables are responsive, full-width, and scrollable if needed
    // Added fixed row height (tr { height: 40px; }) for consistent row sizing
    // Added white-space: nowrap to prevent text wrapping, allowing columns to expand horizontally to the right
    out << "<style>"
           "table { width: 100%; border-collapse: collapse; margin-bottom: 20px; overflow-x: auto; display: block; }"
           "th, td { border: 1px solid #ddd; padding: 8px; text-align: right; white-space: nowrap; }" // No wrapping: Expands columns to the right instead of increasing height
           "tr { height: 40px; }" // Fixed row height for consistent layout
           "th { background-color: #f2f2f2; }"
           "caption { font-weight: bold; margin-bottom: 10px; }"
           ".num { text-align: right; }"
           ".grp { text-align: center; }"
           "</style>\n";

    // Calculate total region size by summing up all region sizes (used to limit the loop to the physical domain, e.g., 18 cm)
    double totalRegionSize = 0.0;
    for (int r = 0; r < DDValues->n_R; ++r)
    {
        totalRegionSize += DDValues->TAM[r];
    }

    // Absorption Rate per Node section: Table with positions at mesh points, sampled by periodicity, groups as columns
    out << "<h2>Absorption Rate per Node</h2>\n";
    out << "<table><thead><tr><th>Position x (cm)</th>";
    for (size_t g = 0; g < DDValues->G; ++g)
    {
        out << "<th>G " << (g + 1) << "</th>"; // Shortened header to "G X" for compactness
    }
    out << "</tr></thead><tbody>\n";

    double t            = 0.0; // Starting position
    int nod             = 0;   // Starting node index
    int nodIncrement = static_cast<int>( ( (static_cast<double>(DDValues->NODOSX) * DDValues->periodicidade)
                          / DDValues->TAM_TOTAL) ); // Proportional increment to skip nodes based on periodicity

    if (nodIncrement < 1)
    {
        QString warning = QString("Warning: Node increment is too small (may not advance nod correctly): nodIncrement = (%1 * %2) / %3 = %4."
                                  " Suggestion: Increase the number of node to make nodIncrement >=1.")
                              .arg(DDValues->NODOSX)
                              .arg(DDValues->periodicidade)
                              .arg(totalRegionSize)
                              .arg(nodIncrement);

        throw std::invalid_argument(warning.toStdString());
    }

    // Loop over positions while within total region size, incrementing by periodicity
    while (t <= totalRegionSize)
    {
        posFmt.str("");
        posFmt.clear();
        posFmt << std::fixed << std::setprecision(precisionPos) << t;

        out << "<tr><td class='num'>" << posFmt.str() << "</td>";
        for (size_t g = 0; g < DDValues->G; ++g)
        {
            valFmt.str("");
            valFmt.clear();
            valFmt << std::scientific << std::setprecision(precisionVal) << DDResult->absorptionRatePerNode[g][nod];

            out << "<td class='num'>" << valFmt.str() << "</td>";
        }
        out << "</tr>\n";

        t += DDValues->periodicidade;          // Increment position by periodicity
        nod += nodIncrement; // Increment node index proportionally
    }

    out << "</tbody></table><hr/>\n";

    // Average Absorption Rate per Region: Single table with regions as rows, groups as columns for compactness
    out << "<h2>Average Absorption Rate per Region</h2>\n";
    out << "<table><thead><tr><th>Region</th>";
    for (size_t g = 0; g < DDValues->G; ++g)
    {
        out << "<th>G " << (g + 1) << "</th>"; // Shortened header to "G X" for compactness
    }
    out << "</tr></thead><tbody>\n";

    // Loop over regions to fill the table
    for (size_t r = 0; r < DDValues->n_R; ++r)
    {
        out << "<tr><td class='grp'>Region " << (r + 1) << "</td>";
        for (size_t g = 0; g < DDValues->G; ++g)
        {
            valFmt.str("");
            valFmt.clear();
            valFmt << std::scientific << std::setprecision(precisionVal) << DDResult->averageAbsorptionRatePerRegion[r][g];
            out << "<td class='num'>" << valFmt.str() << "</td>";
        }
        out << "</tr>\n";
    }
    out << "</tbody></table><hr/>\n";

    // Integrated Absorption Rate per Zone: Single table with zones as rows (not multi-group, so kept vertical but styled)
    out << "<h2>Integrated Absorption Rate per Zone</h2>\n";
    out << "<table><thead><tr><th>Zone</th><th>Integrated Absorption Rate</th></tr></thead><tbody>\n";

    // Loop over zones to fill the table
    for (size_t z = 0; z < DDValues->n_Z; ++z)
    {
        valFmt.str("");
        valFmt.clear();
        valFmt << std::scientific << std::setprecision(precisionVal) << DDResult->integratedAbsorptionRatePerZone[z];

        out << "<tr><td class='grp'>Zone " << (z + 1) << "</td><td class='num'>" << valFmt.str() << "</td></tr>\n";
    }
    out << "</tbody></table>\n";

    // Close the HTML body and the output stream
    out << "</body></html>";

    out.close();
}

void BuildMatrices::writeHTMLAbsorptionCrossSectionFile(dados_entrada *DDValues, CalculatedCrossSectionMatrices *DDResult)
{
    if (DDResult->absorptionCrossSection.empty())
    {
        throw std::runtime_error("Absorption Cross Section Matrix is empty.");
    }

    auto title = computerFileNameHtm(DDValues, "Absorption_Cross_Section");
    std::ofstream out(title);

    if (!out.is_open())
    {
        throw std::runtime_error("Error opening file: " + title.string());
    }

    DDResult->absorptionCrossSectionFile = title.string();

    const int precisionVal               = 6;

    out <<
        R"(<!doctype html>
            <html lang="en"><head><meta charset="utf-8">
            <style>
            html,body{margin:0;padding:16px;background:#121212;color:#ddd;font:14px/1.4 system-ui,Segoe UI,Arial,sans-serif}
            h1{margin:0 0 12px 0;font-size:22px;font-weight:700;color:#fff}
            section{margin:16px 0 24px}
            h2{margin:0 0 8px 0;font-size:18px;font-weight:700;color:#eee}
            table{border-collapse:collapse;width:100%;margin-top:6px}
            th,td{border:1px solid #555;padding:6px 10px}
            th{text-align:center;background:#1f1f1f;color:#ddd}
            td.grp{text-align:center}
            td.num{text-align:right;font-family:ui-monospace,Consolas,monospace}
            hr{border:none;border-top:1px solid #333;margin:16px 0}
            </style></head><body>
            <h2>Absorption Cross Section</h2>
            <hr>
            )";

    for (size_t z = 0; z < static_cast<size_t>(DDValues->n_Z); ++z)
    {
        out << "<section>\n";
        out << "<h2>Zone " << (z + 1) << "</h2>\n";
        out << "<table>\n<thead><tr><th>Energy Group</th><th>Absorption Cross-Section</th></tr></thead>\n<tbody>\n";

        for (size_t g = 0; g < static_cast<size_t>(DDValues->G); ++g)
        {
            std::ostringstream vfmt;
            vfmt << std::defaultfloat << std::scientific << std::setprecision(precisionVal) << DDResult->absorptionCrossSection[z][g];

            out << "<tr>"
                << "<td class='grp'>" << (g + 1) << "</td>"
                << "<td class='num'>" << vfmt.str() << "</td>"
                << "</tr>\n";
        }

        out << "</tbody>\n</table>\n</section>\n";
        if (z + 1 < static_cast<size_t>(DDValues->n_Z))
        {
            out << "<hr/>\n";
        }
    }

    out << "</body></html>";
}

void BuildMatrices::writeHTMLScatteringCrossSectionFile(dados_entrada *DDValues, CalculatedCrossSectionMatrices *DDResult)
{
    if (DDResult->scatteringCrossSection.empty())
    {
        throw std::runtime_error("Scattering Cross Section Matrix is empty.");
    }

    auto title = computerFileNameHtm(DDValues, "Scattering_Cross_Section");
    std::ofstream out(title);

    if (!out.is_open())
    {
        throw std::runtime_error("Error opening file: " + title.string());
    }

    DDResult->totalScatteringCrossSectionFile = title.string();

    const int precisionVal                    = 15;

    out <<
        R"(<!doctype html>
        <html lang="en"><head><meta charset="utf-8">
        <style>
        html,body{margin:0;padding:16px;background:#121212;color:#ddd;font:14px/1.4 system-ui,Segoe UI,Arial,sans-serif}
        h1{margin:0 0 12px 0;font-size:22px;font-weight:700;color:#fff}
        section{margin:16px 0 24px}
        h2{margin:0 0 8px 0;font-size:18px;font-weight:700;color:#eee}
        table{border-collapse:collapse;width:100%;margin-top:6px}
        th,td{border:1px solid #555;padding:6px 10px}
        th{text-align:center;background:#1f1f1f;color:#ddd}
        td.grp{text-align:center}
        td.num{text-align:right;font-family:ui-monospace,Consolas,monospace}
        hr{border:none;border-top:1px solid #333;margin:16px 0}
        </style></head><body>
        <h2>Scattering Cross Section</h2>
        <hr>
        )";

    for (size_t z = 0; z < static_cast<size_t>(DDValues->n_Z); ++z)
    {
        out << "<section>\n";
        out << "<h2>Zone " << (z + 1) << "</h2>\n";
        out << "<table>\n<thead><tr><th>Energy Group</th><th>Scattering Cross-Section</th></tr></thead>\n<tbody>\n";

        for (size_t g = 0; g < static_cast<size_t>(DDValues->G); ++g)
        {
            std::ostringstream vfmt;
            vfmt << std::scientific << std::setprecision(precisionVal) << DDResult->scatteringCrossSection[z][g];

            out << "<tr>"
                << "<td class='grp'>" << (g + 1) << "</td>"
                << "<td class='num'>" << vfmt.str() << "</td>"
                << "</tr>\n";
        }

        out << "</tbody>\n</table>\n</section>\n";
        if (z + 1 < static_cast<size_t>(DDValues->n_Z))
        {
            out << "<hr/>\n";
        }
    }

    out << "</body></html>";
}

void BuildMatrices::writeHTMLNeutronFluxFilePerRegionInterface( dados_entrada *DDValues, CalculatedData *DDResult)
{
    auto title = computerFileNameHtm(DDValues, SCALAR_NEUTRON_FLUX_FILE_NAME);
    std::ofstream out(title);

    if (!out.is_open())
    {
        throw std::runtime_error("Error opening file: " + title.string());
    }

    DDResult->scalarFluxFile = title.string();

    const int precisionPos   = 2;
    const int precisionVal   = 15;
    std::ostringstream posFmt, valFmt;

    out << R"(<!doctype html>
            <html lang="en"><head><meta charset="utf-8">
            <style>
            html,body{margin:0;padding:16px;background:#121212;color:#ddd;font:14px/1.4 system-ui,Segoe UI,Arial,sans-serif}
            table{border-collapse:collapse;width:100%;margin-top:8px}
            th,td{border:1px solid #555;padding:6px 10px}
            th{text-align:center;background:#1f1f1f;color:#ddd}
            td.num{text-align:right;font-family:ui-monospace,Consolas,monospace}
            td.grp{text-align:center}
            caption{caption-side:top;text-align:left;margin:8px 0;font-weight:600}
            h2{margin-top:24px}
            hr{margin:24px 0;border:0;border-top:1px solid #444}
            </style>
            </head><body>)";

    // Title section: Outputs the main report header
    out << "<h1>Neutron Flux Report</h1>\n";

    // Preformatted block for iteration and time info
    out
        << "<pre style='white-space:pre-wrap;background:#1b1b1b;border:1px solid #444;"
           "padding:8px;border-radius:8px'>"
        << "Iteration Number: "
        << DDValues->iteracaoFinal
        << "\n"
        << "Time: "
        << std::fixed
        << std::setprecision(3)
        << DDValues->tempoFinalDeProcessamento
        << "s\n"
        << "</pre>\n"
        << "<hr/>\n";

    // Inline CSS for table styling: Ensures tables are responsive, full-width, and scrollable if needed
    // Added fixed row height (tr { height: 40px; }) for consistent row sizing
    // Added white-space: nowrap to prevent text wrapping, allowing columns to expand horizontally to the right
    out << "<style>"
           "table { width: 100%; border-collapse: collapse; margin-bottom: 20px; overflow-x: auto; display: block; }"
           "th, td { border: 1px solid #ddd; padding: 8px; text-align: right; white-space: nowrap; }" // No wrapping: Expands columns to the right instead of increasing height
           "tr { height: 40px; }" // Fixed row height for consistent layout
           "th { background-color: #f2f2f2; }"
           "caption { font-weight: bold; margin-bottom: 10px; }"
           ".num { text-align: right; }"
           ".grp { text-align: center; }"
           "</style>\n";

    // Calculate total region size by summing up all region sizes (used to limit the loop to the physical domain, e.g., 18 cm)
    double totalRegionSize = 0.0;
    for (int r = 0; r < DDValues->n_R; ++r)
    {
        totalRegionSize += DDValues->TAM[r];
    }

    // Nodal Scalar Flux section: Table with positions at mesh points, sampled by periodicity, groups as columns
    out << "<h2>Nodal Scalar Flux</h2>\n";
    out << "<table><thead><tr><th>Position x (cm)</th>";
    for (size_t g = 0; g < DDValues->G; ++g)
    {
        out << "<th>G " << (g + 1) << "</th>"; // Shortened header to "G X" as per user change
    }
    out << "</tr></thead><tbody>\n";

    double t            = 0.0; // Starting position
    int nod             = 0;   // Starting node index
    std::vector<int> nodeIndices;
    std::vector<double> positions;

    determinePositionIncrement(nodeIndices, positions, DDValues);

    for ( int i = 0; i <= nodeIndices.size(); ++i)
    {
        posFmt.str("");
        posFmt.clear();
        posFmt << std::fixed << std::setprecision(precisionPos) << t;

        out << "<tr><td class='num'>" << posFmt.str() << "</td>";

        for (size_t g = 0; g < DDValues->G; ++g)
        {
            valFmt.str("");
            valFmt.clear();
            valFmt << std::scientific << std::setprecision(precisionVal) << DDResult->nodalScalarFlux[g][nod];
            out << "<td class='num'>" << valFmt.str() << "</td>";
        }
        out << "</tr>\n";

        t = positions[i];          // Increment position by periodicity
        nod = nodeIndices[i]; // Increment node index proportionally
    }

    out << "</tbody></table><hr/>\n";

    // Average Neutron Flux per Region: Single table with regions as rows, groups as columns
    out << "<h2>Average Neutron Flux per Region</h2>\n";
    out << "<table><thead><tr><th>Region</th>";
    for (size_t g = 0; g < DDValues->G; ++g)
    {
        out << "<th>G " << (g + 1) << "</th>"; // Shortened header to "G X" as per user change
    }
    out << "</tr></thead><tbody>\n";

    // Loop over regions to fill the table
    for (size_t r = 0; r < DDValues->n_R; ++r)
    {
        out << "<tr><td class='grp'>Region " << (r + 1) << "</td>";
        for (size_t g = 0; g < DDValues->G; ++g)
        {
            valFmt.str("");
            valFmt.clear();
            valFmt << std::scientific << std::setprecision(precisionVal) << DDResult->averageNeutronFluxPerGroupPerRegion[r][g];
            out << "<td class='num'>" << valFmt.str() << "</td>";
        }
        out << "</tr>\n";
    }
    out << "</tbody></table><hr/>\n";

    // Integrated Neutron Flux per Region: Similar single table with regions as rows, groups as columns
    out << "<h2>Integrated Neutron Flux per Region</h2>\n";
    out << "<table><thead><tr><th>Region</th>";

    for (size_t g = 0; g < DDValues->G; ++g)
    {
        out << "<th>G " << (g + 1) << "</th>"; // Shortened header to "G X" as per user change
    }
    out << "</tr></thead><tbody>\n";

    // Loop over regions to fill the table
    for (size_t r = 0; r < DDValues->n_R; ++r)
    {
        out << "<tr><td class='grp'>Region " << (r + 1) << "</td>";
        for (size_t g = 0; g < DDValues->G; ++g)
        {
            valFmt.str("");
            valFmt.clear();
            valFmt << std::scientific << std::setprecision(precisionVal)
                   << DDResult->integratedNeutronFluxPerGroupPerRegion[r][g];
            out << "<td class='num'>" << valFmt.str() << "</td>";
        }
        out << "</tr>\n";
    }
    out << "</tbody></table><hr/>\n";

    // Angular Flux section: Added here as per user request, with tables per group for angular flux
    // Loop over groups for angular flux
    for (size_t g = 0; g < DDValues->G; ++g)
    {
        out << "<h2>Angular Flux for Group " << (g + 1) << "</h2>\n";
        out << "<table><thead><tr><th>Position x (cm)</th>";

        // Headers for directions with shortened format "D X (mu=Y.YY)"
        for (int o = 0; o < DDValues->n; ++o)
        {
            // Assume DDValues->mi[o] holds the mu (cosine) values from quadrature; adjust if different
            double mu = DDValues->mi[o]; // Replace with actual mu access (e.g., from get_GQ())
            std::ostringstream muFmt;
            muFmt << std::fixed << std::setprecision(precisionPos) << mu;
            out << "<th>D " << (o + 1) << " (mu=" << muFmt.str() << ")</th>";
        }
        out << "</tr></thead><tbody>\n";

        // Sample nodes respecting periodicity
        t            = 0.0; // Reset starting position
        nod          = 0;   // Reset starting node index

        for ( int i = 0; i <= nodeIndices.size(); ++i)
        {
            posFmt.str("");
            posFmt.clear();
            posFmt << std::fixed << std::setprecision(precisionPos) << t;

            out << "<tr><td class='num'>" << posFmt.str() << "</td>";
            for (int o = 0; o < DDValues->n; ++o)
            {
                valFmt.str("");
                valFmt.clear();
                valFmt
                    << std::scientific
                    << std::setprecision(precisionVal)
                    << DDValues->FLUXO_ANGULAR[g][nod][o];
                out << "<td class='num'>" << valFmt.str() << "</td>";
            }
            out << "</tr>\n";

            t = positions[i];          // Increment position by periodicity
            nod = nodeIndices[i]; // Increment node index proportionally

            if (nod > DDValues->NODOSX)
            {
                break;
            }
        }

        out << "</tbody></table><hr/>\n";
    }

    // Close the HTML body
    out << "</body></html>";

    out.close();
}

void BuildMatrices::writeHTMLAbsorptionRateFilePerRegionInterface(dados_entrada *DDValues, CalculatedData *DDResult)
{
    if (DDResult->absorptionRatePerNode.empty())
        throw std::runtime_error("Absorption rate matrices are empty.");

    auto title = computerFileNameHtm(DDValues, ABSORPTION_NEUTRON_RATE_FILE_NAME);
    std::ofstream out(title);

    if (!out.is_open())
        throw std::runtime_error("Error opening file: " + title.string());

    DDResult->absorptionRateFile = title.string();

    const int precisionPos = 2;
    const int precisionVal = 15;
    std::ostringstream posFmt, valFmt;

    out << R"(<!doctype html>
            <html lang="en"><head><meta charset="utf-8">
            <style>
            html,body{margin:0;padding:16px;background:#121212;color:#ddd;font:14px/1.4 system-ui,Segoe UI,Arial,sans-serif}
            table{border-collapse:collapse;width:100%;margin-top:8px}
            th,td{border:1px solid #555;padding:6px 10px}
            th{text-align:center;background:#1f1f1f;color:#ddd}
            td.num{text-align:right;font-family:ui-monospace,Consolas,monospace}
            td.grp{text-align:center}
            caption{caption-side:top;text-align:left;margin:8px 0;font-weight:600}
            h2{margin-top:24px}
            hr{margin:24px 0;border:0;border-top:1px solid #444}
            </style>
            </head><body>)";

    // Title section: Outputs the main report header
    out << "<h1>Absorption Rate Report</h1>\n";

    // Inline CSS for table styling: Ensures tables are responsive, full-width, and scrollable if needed
    // Added fixed row height (tr { height: 40px; }) for consistent row sizing
    // Added white-space: nowrap to prevent text wrapping, allowing columns to expand horizontally to the right
    out << "<style>"
           "table { width: 100%; border-collapse: collapse; margin-bottom: 20px; overflow-x: auto; display: block; }"
           "th, td { border: 1px solid #ddd; padding: 8px; text-align: right; white-space: nowrap; }" // No wrapping: Expands columns to the right instead of increasing height
           "tr { height: 40px; }" // Fixed row height for consistent layout
           "th { background-color: #f2f2f2; }"
           "caption { font-weight: bold; margin-bottom: 10px; }"
           ".num { text-align: right; }"
           ".grp { text-align: center; }"
           "</style>\n";

    // Calculate total region size by summing up all region sizes (used to limit the loop to the physical domain, e.g., 18 cm)
    double totalRegionSize = 0.0;
    for (int r = 0; r < DDValues->n_R; ++r)
    {
        totalRegionSize += DDValues->TAM[r];
    }

    // Absorption Rate per Node section: Table with positions at mesh points, sampled by periodicity, groups as columns
    out << "<h2>Absorption Rate per Node</h2>\n";
    out << "<table><thead><tr><th>Position x (cm)</th>";
    for (size_t g = 0; g < DDValues->G; ++g)
    {
        out << "<th>G " << (g + 1) << "</th>"; // Shortened header to "G X" for compactness
    }
    out << "</tr></thead><tbody>\n";

    double t            = 0.0; // Starting position
    int nod             = 0;   // Starting node index
    std::vector<int> nodeIndices;
    std::vector<double> positions;

    determinePositionIncrement(nodeIndices, positions, DDValues);

    for ( int i = 0; i <= nodeIndices.size(); ++i)
    {
        posFmt.str("");
        posFmt.clear();
        posFmt << std::fixed << std::setprecision(precisionPos) << t;

        out << "<tr><td class='num'>" << posFmt.str() << "</td>";

        for (size_t g = 0; g < DDValues->G; ++g)
        {
            valFmt.str("");
            valFmt.clear();
            valFmt << std::scientific << std::setprecision(precisionVal) << DDResult->absorptionRatePerNode[g][nod];
            out << "<td class='num'>" << valFmt.str() << "</td>";
        }
        out << "</tr>\n";

        t = positions[i];          // Increment position by periodicity
        nod = nodeIndices[i]; // Increment node index proportionally
    }

    out << "</tbody></table><hr/>\n";

    // Average Absorption Rate per Region: Single table with regions as rows, groups as columns for compactness
    out << "<h2>Average Absorption Rate per Region</h2>\n";
    out << "<table><thead><tr><th>Region</th>";
    for (size_t g = 0; g < DDValues->G; ++g)
    {
        out << "<th>G " << (g + 1) << "</th>"; // Shortened header to "G X" for compactness
    }
    out << "</tr></thead><tbody>\n";

    // Loop over regions to fill the table
    for (size_t r = 0; r < DDValues->n_R; ++r)
    {
        out << "<tr><td class='grp'>Region " << (r + 1) << "</td>";
        for (size_t g = 0; g < DDValues->G; ++g)
        {
            valFmt.str("");
            valFmt.clear();
            valFmt << std::scientific << std::setprecision(precisionVal) << DDResult->averageAbsorptionRatePerRegion[r][g];
            out << "<td class='num'>" << valFmt.str() << "</td>";
        }
        out << "</tr>\n";
    }

    out << "</tbody></table><hr/>\n";

    // Integrated Absorption Rate per Zone: Single table with zones as rows (not multi-group, so kept vertical but styled)
    out << "<h2>Integrated Absorption Rate per Zone</h2>\n";
    out << "<table><thead><tr><th>Zone</th><th>Integrated Absorption Rate</th></tr></thead><tbody>\n";

    // Loop over zones to fill the table
    for (size_t z = 0; z < DDValues->n_Z; ++z)
    {
        valFmt.str("");
        valFmt.clear();
        valFmt << std::scientific << std::setprecision(precisionVal) << DDResult->integratedAbsorptionRatePerZone[z];

        out << "<tr><td class='grp'>Zone " << (z + 1) << "</td><td class='num'>" << valFmt.str() << "</td></tr>\n";
    }
    out << "</tbody></table>\n";

    // Close the HTML body and the output stream
    out << "</body></html>";

    out.close();
}

void BuildMatrices::determinePositionIncrement(std::vector<int> &nodeIndices,
                                               std::vector<double> &positions,
                                               dados_entrada *DDValues)
{
    int nod  = 0;
    double t = 0.0;

    for (int r = 0; r < DDValues->n_R; ++r)
    {
        t += DDValues->TAM[r];
        positions.push_back(t);
        nod += DDValues->n_nodos[r];
        nodeIndices.push_back(nod);
    }
}

void BuildMatrices::writeAbsorptionRateFile(dados_entrada *DDValues, CalculatedData *DDResult)
{
    if (DDResult->absorptionRatePerNode.empty())
    {
        throw std::runtime_error("absorptionRatePerNode is empty");
    }

    std::string fileName = computerFileName(DDValues, "Absorption_Rate_Report") + ".html";
    std::ofstream out(fileName);

    if (!out.is_open())
        throw std::runtime_error("Error opening file: " + fileName);

    const int precisionPos = 2;
    const int precisionVal = 15;
    std::ostringstream posFmt, valFmt;

    out << R"(<!doctype html>
            <html lang="en"><head><meta charset="utf-8">
            <style>
            html,body{margin:0;padding:16px;background:#121212;color:#ddd;font:14px/1.4 system-ui,Segoe UI,Arial,sans-serif}
            table{border-collapse:collapse;width:100%;margin-top:8px}
            th,td{border:1px solid #555;padding:6px 10px}
            th{text-align:center;background:#1f1f1f;color:#ddd}
            td.num{text-align:right;font-family:ui-monospace,Consolas,monospace}
            td.grp{text-align:center}
            caption{caption-side:top;text-align:left;margin:8px 0;font-weight:600}
            h2{margin-top:24px}
            hr{margin:24px 0;border:0;border-top:1px solid #444}
            </style>
            </head><body>)";

    out << "<h1>Absorption Rate Report</h1>\n";

    // Rate per node
    out << "<h2>Absorption Rate per Node</h2>";
    out << "<table><caption>Node-wise Absorption Rate</caption><thead><tr><th>Position x (cm)</th><th>Group</th><th>Absorption Rate</th></tr></thead><tbody>";

    double t = 0.0;
    int nodStep = 1;
    for (size_t nod = 0; nod <= DDValues->NODOSX; nod += nodStep)
    {
        for (size_t g = 0; g < DDValues->G; ++g)
        {
            posFmt.str(""); posFmt.clear();
            posFmt << std::fixed << std::setprecision(precisionPos) << t;

            valFmt.str(""); valFmt.clear();
            valFmt << std::scientific << std::setprecision(precisionVal)
                   << DDResult->absorptionRatePerNode[g][nod];

            out << "<tr>"
                << "<td class='num'>" << (g==0 ? posFmt.str() : "") << "</td>"
                << "<td class='grp'>" << (g+1) << "</td>"
                << "<td class='num'>" << valFmt.str() << "</td>"
                << "</tr>";
        }
        t += DDValues->periodicidade;
    }
    out << "</tbody></table><hr/>";

    // Média por região
    out << "<h2>Average Absorption Rate per Region</h2>";
    for (size_t r = 0; r < DDValues->n_R; ++r)
    {
        out << "<table><caption>Region " << (r+1) << "</caption><thead><tr><th>Group</th><th>Average "
                                                       "Absorption Rate</th></tr></thead><tbody>";
        for (size_t g = 0; g < DDValues->G; ++g)
        {
            valFmt.str(""); valFmt.clear();
            valFmt << std::scientific << std::setprecision(precisionVal)
                   << DDResult->averageAbsorptionRatePerRegion[r][g];

            out << "<tr><td class='grp'>" << (g+1) << "</td><td class='num'>" << valFmt.str() << "</td></tr>";
        }
        out << "</tbody></table><br/>";
    }

    // Integrada por zona
    out << "<h2>Integrated Absorption Rate per Zone</h2>";
    out << "<table><thead><tr><th>Zone</th><th>Integrated Absorption Rate</th></tr></thead><tbody>";
    for (size_t z = 0; z < DDValues->n_Z; ++z)
    {
        valFmt.str(""); valFmt.clear();
        valFmt << std::scientific << std::setprecision(precisionVal)
               << DDResult->integratedAbsorptionRatePerZone[z];

        out << "<tr><td class='grp'>Zone " << (z+1) << "</td><td class='num'>" << valFmt.str() << "</td></tr>";
    }
    out << "</tbody></table>";

    out << "</body></html>";
    out.close();
}

std::string BuildMatrices::computerFileName(dados_entrada* DDValues, std::string name)
{
    std::ostringstream title;
    std::string directory;

    std::filesystem::path filePath(fileName);

    if (QString(fileName.c_str()).contains(":/Default"))
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
          << "_Nod"          << DDValues->NODOSX;

    std::string finalTitle = title.str();

    return finalTitle;
}

std::filesystem::path BuildMatrices::computerFileNameHtm(dados_entrada *DDValues, std::string name)
{
    auto title = computerFileName(DDValues, name) + ".html";

    std::filesystem::path htmlPath = std::filesystem::u8path(title);

    return htmlPath;
}

void BuildMatrices::writeAverageNeutronFluxPerRegion(dados_entrada *DDValues, CalculatedData *DDResult)
{
    auto title = computerFileName(DDValues, SCALAR_NEUTRON_FLUX_FILE_NAME) + ".txt";
    std::ofstream outFile(title, std::ios::app);

    if (!outFile.is_open())
    {
        throw std::runtime_error("Error opening file: " + title);
    }

    DDResult->scalarFluxFile = title;

    // Iterate over zones
    for (size_t rIndex = 0; rIndex < DDValues->n_R; ++rIndex)
    {
        // Write zoneIndex as table title
        outFile << "-----------------------------------------------------------------------------------------" << std::endl;
        outFile << "Region " << rIndex + 1 << std::endl;

        // Write table headers
        outFile << std::left << std::setw(15) << "Group" << "Average Neutron Flux Per Region" << std::endl;

        for (size_t group = 0; group < DDValues->G; ++group)
        {
            outFile << std::left << std::setw(15) << group + 1
                    << DDResult->averageNeutronFluxPerGroupPerRegion[rIndex][group] << std::endl;
        }

        outFile << "-----------------------------------------------------------------------------------------" << std::endl;

        // Add a newline for separation between Region if there are multiple Regions
        if (rIndex < DDResult->averageNeutronFluxPerGroupPerRegion.size() - 1)
        {
            outFile << std::endl;
        }
    }

    outFile.close();
}

void BuildMatrices::writeintegratedNeutronFluxPerGroupPerRegion(dados_entrada *DDValues, CalculatedData *DDResult)
{
    auto title = computerFileName(DDValues, "Scalar_Flux") + ".txt";
    std::ofstream outFile(title);

    if (!outFile.is_open())
    {
        throw std::runtime_error("Error opening file: " + title);
    }

    DDResult->scalarFluxFile = title;

    // Iterate over zones
    for (size_t rIndex = 0; rIndex < DDValues->n_R; ++rIndex)
    {
        // Write zoneIndex as table title
        outFile << "-----------------------------------------------------------------------------------------" << std::endl;
        outFile << "Region " << rIndex + 1 << std::endl;

        // Write table headers
        outFile << std::left << std::setw(15) << "Group" << "Integrated Neutron Flux Per Region" << std::endl;

        for (size_t group = 0; group < DDValues->G; ++group)
        {
            outFile << std::left << std::setw(15) << group + 1
                    << DDResult->integratedNeutronFluxPerGroupPerRegion[rIndex][group] << std::endl;
        }

        outFile << "-----------------------------------------------------------------------------------------" << std::endl;

        // Add a newline for separation between Region if there are multiple Regions
        if (rIndex < DDResult->integratedNeutronFluxPerGroupPerRegion.size() - 1)
        {
            outFile << std::endl;
        }
    }

    outFile.close();
}

void BuildMatrices::writeIntegratedAbsorptionRatePerRegionFile(dados_entrada *DDValues, CalculatedData *DDResult)
{
    if (DDResult->integratedAbsorptionRatePerGroupPerRegion.empty())
    {
        throw std::runtime_error("Absorption Matrix is empty.");
    }

    auto title = computerFileName(DDValues, ABSORPTION_NEUTRON_RATE_FILE_NAME) + ".txt";
    std::ofstream outFile(title);

    if (!outFile.is_open())
    {
        throw std::runtime_error("Error opening file: " + title);
    }

    DDResult->absorptionRateFile = title;

    // Iterate over zones
    for (size_t rIndex = 0; rIndex < DDValues->n_R; ++rIndex)
    {
        // Write zoneIndex as table title
        outFile << "-----------------------------------------------------------------------------------------" << std::endl;
        outFile << "Region " << rIndex + 1 << std::endl;

        // Write table headers
        outFile << std::left << std::setw(15) << "Group" << "Integrated Absorption Rate Per Region" << std::endl;

        for (size_t group = 0; group < DDValues->G; ++group)
        {
            outFile << std::left << std::setw(15) << group + 1
                    << DDResult->integratedAbsorptionRatePerGroupPerRegion[rIndex][group] << std::endl;
        }

        outFile << "-----------------------------------------------------------------------------------------" << std::endl;

        // Add a newline for separation between Region if there are multiple Regions
        if (rIndex < DDResult->integratedAbsorptionRatePerGroupPerRegion.size() - 1)
        {
            outFile << std::endl;
        }
    }

    outFile.close();
}

void BuildMatrices::writeAverageAbsorptionRateFile(dados_entrada *DDValues, CalculatedData *DDResult)
{
    if (DDResult->averageAbsorptionRatePerRegion.empty())
    {
        throw std::runtime_error("Absorption Matrix is empty.");
    }

    auto title = computerFileName(DDValues, ABSORPTION_NEUTRON_RATE_FILE_NAME) + ".txt";
    std::ofstream outFile(title);

    if (!outFile.is_open())
    {
        throw std::runtime_error("Error opening file: " + title);
    }

    DDResult->absorptionRateFile = title;

    // Iterate over zones
    for (size_t rIndex = 0; rIndex < DDValues->n_R; ++rIndex)
    {
        // Write zoneIndex as table title
        outFile << "-----------------------------------------------------------------------------------------" << std::endl;
        outFile << "Region " << rIndex + 1 << std::endl;

        // Write table headers
        outFile << std::left << std::setw(15) << "Group" << "Average Absorption Rate" << std::endl;

        for (size_t group = 0; group < DDValues->G; ++group)
        {
            outFile << std::left << std::setw(15) << group + 1
                    << DDResult->averageAbsorptionRatePerRegion[rIndex][group] << std::endl;
        }

        outFile << "-----------------------------------------------------------------------------------------" << std::endl;

        // Add a newline for separation between Region if there are multiple Regions
        if (rIndex < DDResult->averageAbsorptionRatePerRegion.size() - 1)
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

    auto title = computerFileName(DDValues, "Absorption_Cross_Section") + ".txt";
    std::ofstream outFile(title);

    if (!outFile.is_open())
    {
        throw std::runtime_error("Error opening file: " + title);
    }

    DDResult->absorptionCrossSectionFile = title;

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

    auto title = computerFileName(DDValues, "Scalar_Flux") + ".txt";
    std::ofstream output(title);

    if (!output.is_open())
    {
        throw std::runtime_error( "Error opening file: " + title);
    }

    DDResult->scalarFluxFile = title;

    int colWidth = 25;
    int precision = 2;

    const int precisionScalar = 30;

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
                       << std::fixed << std::setprecision(precisionScalar) << DDValues->FLUXO_ESCALAR[g][nod] << std::endl;
            }
            else
            {
                output << std::left << std::setw(colWidth) << ""
                       << std::setw(colWidth) << g + 1
                       << std::fixed << std::setprecision(precisionScalar) << DDValues->FLUXO_ESCALAR[g][nod] << std::endl;
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

    auto title = computerFileName(DDValues, "Scattering_Cross_Section") + ".txt";
    std::ofstream output(title);

    if (!output.is_open())
    {
        throw std::runtime_error( "Error opening file: " + title);
    }

    DDResult->totalScatteringCrossSectionFile = title;

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

