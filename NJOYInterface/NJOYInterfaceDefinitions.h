#pragma once

//Save format

#include <string>
enum SaveFormat
{
    jsonFormat,
    txtFormat,
    datFormat,
};

enum Tests
{
    eTest1,
    eTest2,
};
//Screen
const int pixmapWizardWidth = 640;

//Help
const char NJOYManualPath[] = ":/Help/njoy21.pdf";

const char appTitle[] = "NJOY21 Interface";

//Wizard

//Introduction
const char introductionTitle[] = "<h1>Introduction</h1>";

const char introductionText[] = "<p>NJOY21 Interface will help you "
                                "fill in all the initial information necessary for NJOY to work.</p>";
//Choosing Isotope

const char iaeaPath[] = "<a href=\"https://www-nds.iaea.org/exfor/endf.htm\">IAEA</a>";
const char nndcPath[] = "<a href=\"https://www.nndc.bnl.gov/endf/extended.html\">NNDC</a>";
const char oecdPath[] = "<a href=\"https://www.oecd-nea.org/dbdata/jeff\">NNDC</a>";

const char choosingIsotopesTitle[] = "<h1>Choosing Isotopes</h1>";

const char choosingIsotopesSubTitle[] = "<h1>Please, fill in the details below</h1>";

const char choosingIsotopesText[] = "<p>You need a ENDF type format file.</p><p>You can get one from your computer or access IEAA Database</p>";

const char choosingIsotopesNextButtoTooltipOk[] = "<p>On the next page you will choose NJOY modules</p>";

const char choosingIsotopesNextButtoTooltip[] = "<p>Choose a valid ENDF file to proceed</p>";

//Generals Parameters
const char generalParametersTitle[] = "Choosing General Parameters";
const char temperature[] = "296";
const char reaction[] = "N,TOT;N,EL;N,NON";

//Choosing modules
const char choosingModulesText[] = "Choosing Modules";

//Broadr
const char broadrName[] = "Broadr";

const char labelComment[] = "Comment: ";

//Moder
const char moderName[] = "Moder";
const char moderTapeInputDefault[] = "20";
const char moderTapeOutputDefault[] = "-21";

const char reconrEnterDataDefault[] = "";

//Unresr
const char unresrName[] = "Unresr";
const char sigma0[] = "10000000000";

//Groupr
const char grouprName[] = "Groupr";
const char fileAndSectionToBeProcessedExplanation[] = "The relevant reactions for calculating neutral transport are in the files Mfd = 3 "
                                                    "(cross sections) and Mfd = 6 (neutron-neutron scattering matrix).";

//Reconr
const char reconrName[] = "Reconr";
const char reconrCommentDefault[] = "Ex. pendf tape for";
const int maxMatNumber = 10000;
const int matNumberExample = 1306;
const double maxPrecision = 2;
const char matNumberDefault[] = "525"; //Boro -> B_010
const int decimals = 3;
const double precisionDefault = .005;
const double precisionStep = .001;
//tooltips
const char issSmoothToolTip[] = "Set ismooth to to enable sqrt(e) smoothing for mf6 "
                                "cm emission spectra at low energies and for histogram "
                                "delayed neutron spectra at low energies.";
const char fehiToolTip[] = "Must be in the resolved resonance range";
