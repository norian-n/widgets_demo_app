QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++20

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

LIBS += -L../../../../../src -legdb
INCLUDEPATH += ../../../src

SOURCES += \
    ../../../src/qtinterface/egQtInterface.cpp \
    EgGraphWidget.cpp \
    EgLinkWidget.cpp \
    EgNodeWidget.cpp \
    EgSettingsForm.cpp \
    NodeForm.cpp \
    TemplatesDragWidget.cpp \
    main.cpp \
    EgGraphForm.cpp

HEADERS += \
    ../../../src/qtinterface/egQtInterface.h \
    EgGraphForm.h \
    EgGraphWidget.h \
    EgLinkWidget.h \
    EgNodeWidget.h \
    EgSettingsForm.h \
    NodeForm.h \
    TemplatesDragWidget.h

FORMS += \
    EgGraphForm.ui \
    EgSettingsForm.ui \
    NodeForm.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
