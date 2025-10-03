#include <QApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include "browserwindow.h"

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    
    QApplication app(argc, argv);
    app.setApplicationName("WebBrowser");
    app.setApplicationVersion("1.0");
    app.setOrganizationName("YourCompany");
    
    QCommandLineParser parser;
    parser.setApplicationDescription("A modern web browser built with Qt");
    parser.addHelpOption();
    parser.addVersionOption();
    
    QCommandLineOption urlOption(
        QStringList() << "u" << "url",
        "Open specified URL on startup",
        "url",
        "https://www.google.com"
    );
    parser.addOption(urlOption);
    parser.process(app);
    
    BrowserWindow window;
    QString initialUrl = parser.value(urlOption);
    window.navigateTo(initialUrl);
    
    window.show();
    return app.exec();
}


