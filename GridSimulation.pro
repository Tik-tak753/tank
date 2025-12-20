QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    ai/EnemyAI.cpp \
    ai/MovementController.cpp \
    ai/ShootingController.cpp \
    ai/pathfinder.cpp \
    core/Game.cpp \
    core/GameLoop.cpp \
    core/GameRules.cpp \
    core/GameState.cpp \
    gameplay/Bullet.cpp \
    gameplay/GameObject.cpp \
    gameplay/EnemyTank.cpp \
    gameplay/HealthSystem.cpp \
    gameplay/PlayerTank.cpp \
    gameplay/Tank.cpp \
    gameplay/WeaponSystem.cpp \
    main.cpp \
    mainwindow.cpp \
    model/agent.cpp \
    model/botagent.cpp \
    model/tilemap.cpp \
    rendering/Animation.cpp \
    rendering/Camera.cpp \
    rendering/Renderer.cpp \
    rendering/SpriteManager.cpp \
    systems/CollisionSystem.cpp \
    systems/InputSystem.cpp \
    systems/PhysicsSystem.cpp \
    systems/SoundSystem.cpp \
    world/Base.cpp \
    world/LevelLoader.cpp \
    world/Map.cpp \
    world/Tile.cpp \
    world/Wall.cpp \
    view/GridObject.cpp \
    view/agentitem.cpp \
    view/worldview.cpp

HEADERS += \
    ai/EnemyAI.h \
    ai/MovementController.h \
    ai/ShootingController.h \
    ai/pathfinder.h \
    core/Game.h \
    core/GameLoop.h \
    core/GameRules.h \
    core/GameState.h \
    enums/enums.h \
    gameplay/Bullet.h \
    gameplay/Direction.h \
    gameplay/GameObject.h \
    gameplay/EnemyTank.h \
    gameplay/HealthSystem.h \
    gameplay/PlayerTank.h \
    gameplay/Tank.h \
    gameplay/WeaponSystem.h \
    mainwindow.h \
    model/agent.h \
    model/botagent.h \
    model/tilemap.h \
    utils/Constants.h \
    rendering/Animation.h \
    rendering/Camera.h \
    rendering/Renderer.h \
    rendering/SpriteManager.h \
    systems/CollisionSystem.h \
    systems/InputSystem.h \
    systems/PhysicsSystem.h \
    systems/SoundSystem.h \
    world/Base.h \
    world/LevelLoader.h \
    world/Map.h \
    world/Tile.h \
    world/Wall.h \
    view/GridObject.h \
    view/agentitem.h \
    view/worldview.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

# Добавление CRT Debug для поиска утечек памяти (только для MSVC)
win32-msvc* {
    CONFIG(debug, debug|release) {
        DEFINES += _CRTDBG_MAP_ALLOC
        LIBS += -lOle32
    }
}
