#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <memory>

class QGraphicsScene;
class QGraphicsView;
class QTimer;
class QGraphicsRectItem;
class QKeyEvent;

class Map;
class PlayerTank;
class InputSystem;
class Bullet;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

private:
    // View / Scene
    QGraphicsScene* m_scene = nullptr;
    QGraphicsView* m_view = nullptr;

    // Model
    std::unique_ptr<Map> m_map;
    std::unique_ptr<PlayerTank> m_player;
    std::unique_ptr<InputSystem> m_input;
    std::unique_ptr<Bullet> m_bullet;

    // View (player)
    QGraphicsRectItem* m_playerItem = nullptr;
    QGraphicsRectItem* m_bulletItem = nullptr;

    // Timer
    QTimer* m_timer = nullptr;
};

#endif // MAINWINDOW_H
