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

}
