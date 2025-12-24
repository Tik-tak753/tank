# Battle City Tank (Qt) — README (Українською)

## Опис
Battle City–подібна 2D гра на Qt/C++, де ви керуєте танком, знищуєте ворогів і захищаєте базу на сітковій карті.

## Основні можливості
- Класичний геймплей: поїздка по тайловій карті, стрільба та захист бази.
- Вбудований редактор рівнів прямо в застосунку.
- Система бонусів (зірка, шолом, годинник, граната) з тимчасовими ефектами.
- HUD із життями, рахунком, прогресом ворогів і статусом гравця.
- Підтримка кількох рівнів та завантаження карт із файлів.

## Керування
- **Гра:** WASD або стрілки — рух; Space/Shift/Enter — стрільба; P — пауза.
- **Редактор:** натисніть цифри 1–6 для вибору плитки (1 — порожньо, 2 — цегла, 3 — сталь, 4 — ліс, 5 — вода, 6 — лід); ЛКМ ставить плитку, ПКМ стирає; Ctrl+S — зберегти карту у файл; Ctrl+O/Ctrl+L — завантажити карту.

## Редактор рівнів
- Працює лише в режимі редагування (GameMode::Editing) та синхронізує зміни з поточною картою `Map`.
- Базова клітинка, старт гравця та точки спавну ворогів захищені від змін.
- Дає лише пряме встановлення тайлів (`Map::setTile`) без додаткової логіки чи скриптів.
- Підтримує імпорт/експорт простих текстових матриць чисел.

## Формат збереження рівнів
- Текстовий файл із розмірами карти в першому рядку (`width height`).
- Далі — матриця кодів плиток через пробіл: 0 — пусто, 1 — Brick, 2 — Steel, 3 — Forest, 4 — Water, 5 — Ice. База зберігається у коді правил (`GameRules::baseCell`) і не записується.

## Запуск і збірка
- Потрібні Qt 6 (Widgets), компілятор C++17 і MinGW (на Windows) або GCC/Clang (на Linux/macOS).
- Відкрийте `GridSimulation.pro` у Qt Creator або виконайте з консолі:
  - `qmake GridSimulation.pro`
  - `make` (або `mingw32-make` на Windows)
- Запустіть отриманий виконуваний файл; ресурси карт/спрайтів очікуються відносно каталогу проєкту.

## Структура проєкту
- `core/` — фасад `Game`, стан `GameState`, правила `GameRules`, ігровий цикл.
- `gameplay/` — танки, снаряди, бонуси, здоров'я та зброя.
- `world/` — карта `Map`, плитки `Tile`, база та завантаження рівнів.
- `systems/` — вхід, фізика, колізії та меню.
- `rendering/` — відтворення сцени, HUD, камери та спрайти.
- `LevelEditor.*` — простий тайловий редактор карт у межах додатка.
- `assets/` — спрайти, звуки й шаблони карт.

---

# Battle City Tank (Qt) — README (English)

## Description
A Battle City–style 2D Qt/C++ game where you drive a tank, destroy enemies, and defend your base on a grid map.

## Key Features
- Classic gameplay with tile-based movement, shooting, and base defense.
- Built-in level editor inside the application.
- Bonus system (star, helmet, clock, grenade) with temporary effects.
- HUD showing lives, score, enemy progress, and player status.
- Multiple levels with loading from external map files.

## Controls
- **Game:** WASD or arrow keys to move; Space/Shift/Enter to fire; P to pause.
- **Editor:** Press 1–6 to select tiles (1 = empty, 2 = brick, 3 = steel, 4 = forest, 5 = water, 6 = ice); LMB paints, RMB erases; Ctrl+S saves the map; Ctrl+O/Ctrl+L loads a map.

## Level Editor
- Active only in editing mode (`GameMode::Editing`) and writes directly into the current `Map`.
- Base cell, player spawn, and enemy spawn cells are protected from modification.
- Provides direct tile placement via `Map::setTile` without extra scripting.
- Supports importing/exporting simple numeric text matrices.

## Level Storage Format
- Text file with map size in the first line (`width height`).
- Followed by a matrix of tile codes separated by spaces: 0 = empty, 1 = brick, 2 = steel, 3 = forest, 4 = water, 5 = ice. The base position comes from `GameRules::baseCell` and is not stored in the file.

## Run/Build
- Requires Qt 6 (Widgets), a C++17 compiler, and MinGW on Windows (or GCC/Clang on Linux/macOS).
- Open `GridSimulation.pro` in Qt Creator or build from terminal:
  - `qmake GridSimulation.pro`
  - `make` (or `mingw32-make` on Windows)
- Run the produced executable; map/sprite assets are expected relative to the project directory.

## Project Structure
- `core/` — `Game` facade, `GameState`, `GameRules`, and the main loop.
- `gameplay/` — tanks, bullets, bonuses, health, and weapon systems.
- `world/` — `Map`, `Tile`, base entity, and level loading.
- `systems/` — input, physics, collision handling, and menu helpers.
- `rendering/` — scene rendering, HUD, camera, and sprites.
- `LevelEditor.*` — in-app tile editor for maps.
- `assets/` — sprites, sounds, and map templates.
