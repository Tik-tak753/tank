#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QGraphicsScene;
class QTimer;

class WorldView;
class TileMap;

class Agent;
class AgentItem;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    // View / Scene
    QGraphicsScene* scene = nullptr;
    WorldView* view = nullptr;

    // Model
    TileMap* map = nullptr;
    Agent* agent = nullptr;

    // View (agent)
    AgentItem* agentItem = nullptr;

    // Timer
    QTimer* timer = nullptr;
};

#endif // MAINWINDOW_H
