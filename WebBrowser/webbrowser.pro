QT += core gui webenginewidgets webengine webchannel network multimedia widgets

CONFIG += c++17

TARGET = WebBrowser
TEMPLATE = app

SOURCES += \
    main.cpp \
    browserwindow.cpp

HEADERS += \
    browserwindow.h

RESOURCES += \
    resources.qrc


