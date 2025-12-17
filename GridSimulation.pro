QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    ai/pathfinder.cpp \
    main.cpp \
    mainwindow.cpp \
    model/agent.cpp \
    model/botagent.cpp \
    model/tilemap.cpp \
    simulation/world.cpp \
    view/GridObject.cpp \
    view/agentitem.cpp \
    view/worldview.cpp

HEADERS += \
    ai/pathfinder.h \
    mainwindow.h \
    model/agent.h \
    model/botagent.h \
    model/tilemap.h \
    simulation/world.h \
    utils/Constants.h \
    view/GridObject.h \
    view/agentitem.h \
    view/worldview.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
