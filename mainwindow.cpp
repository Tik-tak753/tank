#include "MainWindow.h"

#include <QGraphicsScene>
#include <QTimer>
#include"model/botagent.h"
#include "view/WorldView.h"
#include "model/TileMap.h"
#include "model/Agent.h"
#include "view/AgentItem.h"
#include "ai/PathFinder.h"
#include "utils/Constants.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    /* =====================
     * Сцена / вікно перегляду
     * ===================== */

    scene = new QGraphicsScene(this);

    view = new WorldView(scene, this);
    setCentralWidget(view);
    resize(800, 600);

    /* =====================
     * Карта
     * ===================== */

    map = new TileMap(GRID_WIDTH, GRID_HEIGHT);

    // тестові стіни
    map->setWall(QPoint(3,3), true);
    map->setWall(QPoint(3,4), true);
    map->setWall(QPoint(3,5), true);
    map->setWall(QPoint(4,5), true);
    map->setWall(QPoint(5,5), true);

    view->drawMap(*map, TILE_SIZE);

    /* =====================
     * Агент (модель)
     * ===================== */

    QPoint start(1, 1);
    QPoint goal(10, 10);

    agent = new BotAgent(start.x(), start.y(), map);

    QList<QPoint> path = PathFinder::findPath(*map, start, goal);
    agent->setPath(path);

    /* =====================
     * AgentItem (відображення)
     * ===================== */

    agentItem = new AgentItem(agent);
    scene->addItem(agentItem);

    /* =====================
     * Таймер
     * ===================== */

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [this]() {
        agent->update();
        agentItem->syncFromAgent();
    });
    timer->start(300);
}

MainWindow::~MainWindow()
{
    // Qt автоматично знищить дочірні QObject
}
