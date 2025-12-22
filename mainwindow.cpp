#include "mainwindow.h"

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QTimer>
#include <QPoint>
#include <QKeyEvent>
#include <QSize>
#include <QResizeEvent>
#include <QtGlobal>
#include <QMouseEvent>

#include "LevelEditor.h"
#include "core/Game.h"
#include "systems/InputSystem.h"
#include "systems/MenuSystem.h"
#include "rendering/Renderer.h"
#include "utils/Constants.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    /* =====================
     * Scene / View
     * ===================== */

    m_scene = new QGraphicsScene(this);

    m_view = new QGraphicsView(m_scene, this);
    m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setCentralWidget(m_view);
    resize(800, 600);
    setFocusPolicy(Qt::StrongFocus);
    m_view->setFocus();

    /* =====================
     * Game
     * ===================== */

    m_game = std::make_unique<Game>(this);
    m_input = std::make_unique<InputSystem>();

    GameRules& rules = m_game->rules();
    rules.setMapSize(QSize(GRID_WIDTH, GRID_HEIGHT));
    rules.setBaseCell(QPoint(GRID_WIDTH / 2, GRID_HEIGHT - 2));

    m_game->setInputSystem(m_input.get());

    m_menuSystem = std::make_unique<MenuSystem>();
    m_menuSystem->setGame(m_game.get());
    m_menuSystem->setInputSystem(m_input.get());
    m_menuSystem->setScene(m_scene);
    m_menuSystem->setExitCallback([this]() { close(); });
    m_menuSystem->showMainMenu();

    m_renderer = std::make_unique<Renderer>(m_scene);
    m_levelEditor = std::make_unique<LevelEditor>();
    m_levelEditor->setGame(m_game.get());
    m_levelEditor->setView(m_view);

    /* =====================
     * Timer / GameLoop
     * ===================== */

    m_frameTimer.start();
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, [this]() {
        const qint64 frameDeltaMs = m_frameTimer.restart();
        m_frameAccumulatorMs += frameDeltaMs;

        while (m_frameAccumulatorMs >= kFixedTickMs) {
            if (m_game && (!m_menuSystem || !m_menuSystem->blocksGameplay()))
                m_game->update(kFixedTickMs);
            m_frameAccumulatorMs -= kFixedTickMs;
        }

        if (m_menuSystem && m_game)
            m_menuSystem->syncWithGameState(m_game->state());

        const qreal alpha = static_cast<qreal>(m_frameAccumulatorMs) / static_cast<qreal>(kFixedTickMs);

        if (m_renderer)
            m_renderer->renderFrame(*m_game, alpha);

        if (m_menuSystem)
            m_menuSystem->renderMenus();
    });
    m_timer->start(kFixedTickMs);
}

MainWindow::~MainWindow()
{
    // Qt удалит QObject-детей автоматически
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->isAutoRepeat()) {
        QMainWindow::keyPressEvent(event);
        return;
    }

    if (m_menuSystem && m_menuSystem->handleInput(*event))
        return;

    if (m_menuSystem && m_menuSystem->blocksGameplay()) {
        QMainWindow::keyPressEvent(event);
        return;
    }

    const bool editing = m_game && m_game->state().gameMode() == GameMode::Editing;
    if (editing) {
        if (m_levelEditor && m_levelEditor->handleKeyPress(*event))
            return;
        QMainWindow::keyPressEvent(event);
        return;
    }

    if (m_input && m_input->handleKeyPress(event->key(), event->nativeScanCode())) {
        event->accept();
        return;
    }

    QMainWindow::keyPressEvent(event);
}

void MainWindow::keyReleaseEvent(QKeyEvent *event)
{
    if (event->isAutoRepeat()) {
        QMainWindow::keyReleaseEvent(event);
        return;
    }

    if (m_menuSystem && m_menuSystem->blocksGameplay()) {
        QMainWindow::keyReleaseEvent(event);
        return;
    }

    const bool editing = m_game && m_game->state().gameMode() == GameMode::Editing;
    if (editing) {
        QMainWindow::keyReleaseEvent(event);
        return;
    }

    if (m_input && m_input->handleKeyRelease(event->key(), event->nativeScanCode())) {
        event->accept();
        return;
    }

    QMainWindow::keyReleaseEvent(event);
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if (m_menuSystem && m_menuSystem->blocksGameplay()) {
        QMainWindow::mousePressEvent(event);
        return;
    }

    if (m_levelEditor && m_levelEditor->handleMousePress(*event))
        return;

    QMainWindow::mousePressEvent(event);
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);

    if (m_menuSystem)
        m_menuSystem->renderMenus();
}
