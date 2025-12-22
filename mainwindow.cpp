#include "mainwindow.h"

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsTextItem>
#include <QFont>
#include <QTimer>
#include <QPoint>
#include <QKeyEvent>
#include <QSize>
#include <QColor>
#include <QtGlobal>

#include "core/Game.h"
#include "systems/InputSystem.h"
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
    m_game->enterMainMenu();

    m_renderer = std::make_unique<Renderer>(m_scene);

    /* =====================
     * Timer / GameLoop
     * ===================== */

    m_frameTimer.start();
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, [this]() {
        const qint64 frameDeltaMs = m_frameTimer.restart();
        m_frameAccumulatorMs += frameDeltaMs;

        while (m_frameAccumulatorMs >= kFixedTickMs) {
            m_game->update(kFixedTickMs);
            m_frameAccumulatorMs -= kFixedTickMs;
        }

        const qreal alpha = static_cast<qreal>(m_frameAccumulatorMs) / static_cast<qreal>(kFixedTickMs);

        if (m_renderer)
            m_renderer->renderFrame(*m_game, alpha);

        if (m_game && !m_gameOverItem && (m_game->state().isGameOver() || m_game->state().isVictory())) {
            const bool victory = m_game->state().isVictory();
            m_gameOverItem = new QGraphicsTextItem(victory ? QStringLiteral("STAGE CLEAR") : QStringLiteral("GAME OVER"));
            QFont font = m_gameOverItem->font();
            font.setPointSize(40);
            m_gameOverItem->setFont(font);
            m_gameOverItem->setDefaultTextColor(victory ? QColor(240, 240, 240) : Qt::red);
            m_gameOverItem->setZValue(1000);
            m_scene->addItem(m_gameOverItem);

            QRectF sceneRect = m_scene->sceneRect();
            if (!sceneRect.isValid() || sceneRect.isNull())
                sceneRect = m_scene->itemsBoundingRect();

            const QRectF textRect = m_gameOverItem->boundingRect();
            const QPointF centeredPos = sceneRect.center() - QPointF(textRect.width() / 2.0, textRect.height() / 2.0);
            m_gameOverItem->setPos(centeredPos);
        }

        updateMenuOverlays();
    });
    m_timer->start(kFixedTickMs);
}

MainWindow::~MainWindow()
{
    // Qt удалит QObject-детей автоматически
}

void MainWindow::startGame()
{
    if (!m_game)
        return;

    if (m_input)
        m_input->clear();

    m_game->startNewGame();
    clearGameOverOverlay();
}

void MainWindow::pauseGame()
{
    if (!m_game)
        return;

    if (m_input)
        m_input->clear();

    m_game->pause();
}

void MainWindow::resumeGame()
{
    if (!m_game)
        return;

    if (m_input)
        m_input->clear();

    m_game->resume();
}

void MainWindow::returnToMainMenu()
{
    if (!m_game)
        return;

    if (m_input)
        m_input->clear();

    m_game->enterMainMenu();
    clearGameOverOverlay();
}

void MainWindow::updateMenuOverlays()
{
    if (!m_scene)
        return;

    const GameMode mode = m_game ? m_game->state().gameMode() : GameMode::MainMenu;

    auto ensureOverlay = [&](QGraphicsTextItem*& item, const QString& text) {
        if (!item) {
            item = m_scene->addText(text);
            item->setDefaultTextColor(Qt::white);
            QFont font = item->font();
            font.setPointSize(28);
            item->setFont(font);
            item->setZValue(2000);
        }
        if (item->toPlainText() != text)
            item->setPlainText(text);
        item->setVisible(true);
    };

    auto hideOverlay = [](QGraphicsTextItem* item) {
        if (item)
            item->setVisible(false);
    };

    if (mode == GameMode::MainMenu) {
        ensureOverlay(m_mainMenuItem, QStringLiteral("BATTLE CITY\n\nStart Game - Enter\nExit - Esc"));
        hideOverlay(m_pauseMenuItem);
    } else if (mode == GameMode::Paused) {
        ensureOverlay(m_pauseMenuItem, QStringLiteral("PAUSED\n\nResume - Esc\nRestart Level - R\nMain Menu - M"));
        hideOverlay(m_mainMenuItem);
    } else {
        hideOverlay(m_mainMenuItem);
        hideOverlay(m_pauseMenuItem);
    }

    auto centerItem = [&](QGraphicsTextItem* item) {
        if (!item || !item->isVisible())
            return;

        QRectF sceneRect = m_scene->sceneRect();
        if ((!sceneRect.isValid() || sceneRect.isNull()) && !m_scene->views().isEmpty() && m_scene->views().first()->viewport())
            sceneRect = QRectF(QPointF(0.0, 0.0), QSizeF(m_scene->views().first()->viewport()->size()));
        if (!sceneRect.isValid() || sceneRect.isNull())
            sceneRect = m_scene->itemsBoundingRect();

        const QRectF textRect = item->boundingRect();
        const QPointF centeredPos = sceneRect.center() - QPointF(textRect.width() / 2.0, textRect.height() / 2.0);
        item->setPos(centeredPos);
    };

    centerItem(m_mainMenuItem);
    centerItem(m_pauseMenuItem);

    if (mode == GameMode::MainMenu && m_gameOverItem)
        clearGameOverOverlay();
}

void MainWindow::clearGameOverOverlay()
{
    if (m_gameOverItem) {
        m_scene->removeItem(m_gameOverItem);
        delete m_gameOverItem;
        m_gameOverItem = nullptr;
    }
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->isAutoRepeat()) {
        QMainWindow::keyPressEvent(event);
        return;
    }

    const GameMode mode = m_game ? m_game->state().gameMode() : GameMode::MainMenu;

    if (mode == GameMode::MainMenu) {
        if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter || event->key() == Qt::Key_Space) {
            startGame();
            event->accept();
            return;
        }

        if (event->key() == Qt::Key_Escape) {
            close();
            event->accept();
            return;
        }
    }

    if (mode == GameMode::Paused) {
        if (event->key() == Qt::Key_Escape) {
            resumeGame();
            event->accept();
            return;
        }
        if (event->key() == Qt::Key_R) {
            startGame();
            event->accept();
            return;
        }
        if (event->key() == Qt::Key_M) {
            returnToMainMenu();
            event->accept();
            return;
        }
    }

    if (mode == GameMode::GameOver) {
        if (event->key() == Qt::Key_R) {
            startGame();
            event->accept();
            return;
        }
        if (event->key() == Qt::Key_M) {
            returnToMainMenu();
            event->accept();
            return;
        }
    }

    if (mode == GameMode::Playing && event->key() == Qt::Key_Escape) {
        pauseGame();
        event->accept();
        return;
    }

    if (mode == GameMode::Playing && m_input && m_input->handleKeyPress(event->key())) {
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

    const GameMode mode = m_game ? m_game->state().gameMode() : GameMode::MainMenu;
    if (mode == GameMode::Playing && m_input && m_input->handleKeyRelease(event->key())) {
        event->accept();
        return;
    }

    QMainWindow::keyReleaseEvent(event);
}
