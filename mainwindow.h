#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QElapsedTimer>
#include <memory>

class QGraphicsScene;
class QGraphicsView;
class QTimer;
class QKeyEvent;
class QMouseEvent;
class QResizeEvent;

class Game;
class InputSystem;
class MenuSystem;
class Renderer;
class LevelEditor;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    static constexpr int kFixedTickMs = 16;

    // View / Scene
    QGraphicsScene* m_scene = nullptr;
    QGraphicsView* m_view = nullptr;

    // Game
    std::unique_ptr<Game> m_game;
    std::unique_ptr<InputSystem> m_input;
    std::unique_ptr<MenuSystem> m_menuSystem;
    std::unique_ptr<Renderer> m_renderer;
    std::unique_ptr<LevelEditor> m_levelEditor;

    // Timer
    QTimer* m_timer = nullptr;
    qint64 m_frameAccumulatorMs = 0;
    QElapsedTimer m_frameTimer;
};

#endif // MAINWINDOW_H
