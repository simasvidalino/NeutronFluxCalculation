// InterfaceConfig.cpp
#include "InterFaceDefinitions.h"

namespace Interface {
QPalette getDarkPalette()
{
    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, QColor(30, 30, 30));           // Dark background color
    darkPalette.setColor(QPalette::WindowText, QColor(212, 212, 212));    // Light text color
    darkPalette.setColor(QPalette::Highlight, QColor(38, 79, 120));       // Highlight color (when pressed)
    darkPalette.setColor(QPalette::HighlightedText, QColor(212, 212, 212)); // Text color when highlighted
    darkPalette.setColor(QPalette::Base, QColor(51, 51, 51));             // Dark base color
    darkPalette.setColor(QPalette::AlternateBase, QColor(46, 46, 46));    // Alternative base color
    darkPalette.setColor(QPalette::Text, QColor(212, 212, 212));          // Default text color
    darkPalette.setColor(QPalette::Button, QColor(51, 51, 51));          // Button color
    darkPalette.setColor(QPalette::ButtonText, QColor(212, 212, 212));    // Text color for buttons
    darkPalette.setColor(QPalette::Link, QColor(85, 167, 255));

    return darkPalette;
}

QPalette getLightPalette()
{
    QPalette lightPalette;
    lightPalette.setColor(QPalette::Window, QColor(255, 255, 255));      // Light background color
    lightPalette.setColor(QPalette::WindowText, QColor(0, 0, 0));         // Dark text color
    lightPalette.setColor(QPalette::Highlight, QColor(173, 216, 230));    // Highlight color (when pressed)
    lightPalette.setColor(QPalette::HighlightedText, QColor(0, 0, 0));    // Text color when highlighted
    lightPalette.setColor(QPalette::Base, QColor(245, 245, 245));         // Light gray base color
    lightPalette.setColor(QPalette::AlternateBase, QColor(235, 235, 235)); // Alternate base color
    lightPalette.setColor(QPalette::Text, QColor(0, 0, 0));               // Default text color
    lightPalette.setColor(QPalette::Button, QColor(200, 200, 200));       // Button color
    lightPalette.setColor(QPalette::ButtonText, QColor(0, 0, 0));        // Button text color
    lightPalette.setColor(QPalette::Link, QColor(0, 122, 255));           // Apple Blue link color

    return lightPalette;
}

const char* getDefaultZoneString()
{
    const char* str = "UNKNOWN";
    return str;
}

const char *getAboutApp()
{
    const char* str = "<p><strong>Welcome to NeutronFlowPro</strong></p>"
                      "<p><strong>Version:</strong> 1.0.0</p>"
                      "<p><strong>Description:</strong></p>"
                      "<div align='justify'>NeutronFlowPro is a didactic and user-friendly tool for calculating neutron scalar flux "
                      "using the Diamond Difference Method for neutron transport problems. "
                      "This application provides an intuitive interface that allows users to input data, run "
                      "simulations, and visualize results in an effective manner.</p>"
                      "<p><strong>Key Features:</strong></div>"
                      "<ul>"
                      "<li><strong>Diamond Difference Method (SN) for neutron scalar flux calculation.</strong></li>"
                      "<li><strong>Interactive and user-friendly graphical interface.</strong></li>"
                      "<li><strong>Visualization of results through intuitive charts.</strong></li>"
                      "<li><strong>Parameter customization to meet user-specific needs.</strong></li>"
                      "</ul>"
                      "<p><strong>How to Use:</strong></p>"
                      "<ol>"
                      "<li><strong>Enter the necessary data for the simulation.</strong></li>"
                      "<li><strong>Configure simulation options according to your preferences.</strong></li>"
                      "<li><strong>Run the simulation and wait for the results.</strong></li>"
                      "<li><strong>Explore generated charts for visual analysis.</strong></li>"
                      "</ol>"
                      "<div align='justify'>Thank you for choosing NeutronFlowPro. If you have any questions or feedback, "
                      "please contact us at <strong>andreia.vidalino@grad.iprj.uerj.br</strong>.</div>"
                      "<p>Enjoy your experience with NeutronFlowPro!</p>";


    return str;

}

const char *getIAEAAdress()
{
    const char* iaeaPath = "<a href=\"https://www-nds.iaea.org/exfor/endf.htm\">IAEA</a>";

    return iaeaPath;
}

const char *getWindowTitle()
{
    return "Neutron Scalar Flux Calculator";
}

const int getMaxRegionQtt()
{
    return 10;
}

const char* getDefaultProjectName()
{
    return ":/Default_Project/Resources/Default_Project.json";
}

const char *getCrossSessionDataToolTip()
{
    return "Use this button to input total cross-section and group-to-group scattering data in a format that the program can interpret.";
}

const char *getToolTipForNodes()
{
    return "Defines the number of nodes for this region.\nToo few nodes may reduce accuracy, while too many may slow down the simulation.";
}

}
