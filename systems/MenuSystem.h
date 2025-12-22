#ifndef MENUSYSTEM_H
#define MENUSYSTEM_H

#include <QRectF>
#include <QString>
#include <functional>

#include "core/GameState.h"

class Game;
class InputSystem;
class QGraphicsScene;
class QGraphicsTextItem;
class QKeyEvent;

enum class MenuState {
    None,
    MainMenu,
    PauseMenu,
    GameOverMenu
};

class MenuSystem
{
public:
    MenuSystem();

    void setGame(Game* game);
    void setInputSystem(InputSystem* input);
    void setScene(QGraphicsScene* scene);
    void setExitCallback(std::function<void()> callback);

    MenuState state() const { return m_state; }
    bool blocksGameplay() const;
    bool handleInput(QKeyEvent& event);
    void syncWithGameState(const GameState& state);
    void renderMenus();
    void showMainMenu();

private:
    void startGame();
    void pauseGame();
    void resumeGame();
    void restartLevel();
    void returnToMainMenu();
    void enterGameOverMenu(bool victory);
    void clearInput();

    void ensureOverlay(QGraphicsTextItem*& item, const QString& text);
    void hideOverlay(QGraphicsTextItem* item);
    void centerOverlay(QGraphicsTextItem* item) const;
    QRectF sceneRect() const;
    void clearGameOverOverlay();
    void updateMenuOverlays();

    MenuState m_state = MenuState::MainMenu;
    Game* m_game = nullptr;
    InputSystem* m_input = nullptr;
    QGraphicsScene* m_scene = nullptr;
    QGraphicsTextItem* m_mainMenuItem = nullptr;
    QGraphicsTextItem* m_pauseMenuItem = nullptr;
    QGraphicsTextItem* m_gameOverItem = nullptr;
    bool m_victory = false;
    std::function<void()> m_exitCallback;
};

#endif // MENUSYSTEM_H
