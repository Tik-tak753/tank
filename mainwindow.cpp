#include "mainwindow.h"

#include <QGraphicsRectItem>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QTimer>
#include <QPoint>
#include <QPointF>
#include <QSize>
#include <utility>
#include <QBrush>
#include <QColor>
#include <QPen>
#include <QSizeF>
#include <QKeyEvent>

#include "core/GameRules.h"
#include "gameplay/Direction.h"
#include "gameplay/PlayerTank.h"
#include "systems/InputSystem.h"
#include "utils/Constants.h"
#include "world/LevelLoader.h"
#include "world/Map.h"
#include "world/Tile.h"

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
     * Map
     * ===================== */

    LevelLoader loader;
    GameRules rules;
    rules.setMapSize(QSize(GRID_WIDTH, GRID_HEIGHT));
    LevelData level = loader.loadDefaultLevel(rules);

    m_map = std::move(level.map);

    // Малюємо карту один раз
    for (int y = 0; y < m_map->size().height(); ++y) {
        for (int x = 0; x < m_map->size().width(); ++x) {
            const Tile tile = m_map->tile(QPoint(x, y));
            if (tile.type == TileType::Empty)
                continue;

            QColor color = Qt::gray;
            if (tile.type == TileType::Brick)
                color = QColor(193, 68, 14);
            if (tile.type == TileType::Steel)
                color = QColor(160, 160, 160);
            if (tile.type == TileType::Base)
                color = QColor(230, 230, 0);

            QPointF pos(x * TILE_SIZE, y * TILE_SIZE);
            m_scene->addRect(QRectF(pos, QSizeF(TILE_SIZE, TILE_SIZE)), QPen(Qt::NoPen), QBrush(color));
        }
    }

    /* =====================
     * PlayerTank (MODEL)
     * ===================== */

    m_input = std::make_unique<InputSystem>();

    m_player = std::make_unique<PlayerTank>(level.playerSpawn);
    m_player->setInput(m_input.get());
    m_player->setMap(m_map.get());

    /* =====================
     * Player QGraphicsItem (VIEW)
     * ===================== */

    const QPoint spawn = m_player->cell();
    const QRectF rect(QPointF(spawn) * TILE_SIZE, QSizeF(TILE_SIZE, TILE_SIZE));
    m_playerItem = m_scene->addRect(rect, QPen(Qt::black), QBrush(QColor(40, 160, 32)));

    /* =====================
     * Timer / GameLoop
     * ===================== */

    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, [this]() {
        const int deltaMs = 16; // ~60 FPS
        m_player->updateWithDelta(deltaMs);

        const QPoint cell = m_player->cell();
        const QPointF pixelPos = QPointF(cell) * TILE_SIZE;
        m_playerItem->setRect(QRectF(pixelPos, QSizeF(TILE_SIZE, TILE_SIZE)));
    });
    m_timer->start(16);
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

    if (m_input && m_input->handleKeyPress(event->key())) {
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

    if (m_input && m_input->handleKeyRelease(event->key())) {
        event->accept();
        return;
    }

    QMainWindow::keyReleaseEvent(event);
}
