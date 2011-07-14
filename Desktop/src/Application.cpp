#include <QtGui/QApplication>

#include "MainWindowController.h"

int main(int argc, char **argv)
{
	// Create the Qt application
	QApplication app(argc, argv);

    QCoreApplication::setOrganizationName("SL Studios");
    QCoreApplication::setOrganizationDomain("SLStudios.org");
    QCoreApplication::setApplicationName("SLS-Desktop");
	
	// Create a main widget and start it up
	MainWindowController* mainWindowController = new MainWindowController();
	mainWindowController->showWidget();
	
	// Start up the application run loop
	return app.exec();
}
