// 1. Эти инклуды и дефайны должны быть САМЫМИ ПЕРВЫМИ в файле
#ifdef _MSC_VER
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

// Опционально: заменяем стандартный new на отладочный для точного указания строк
#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
#define new DBG_NEW
#endif

#include "mainwindow.h"

#include <QApplication>



int main(int argc, char *argv[])
{
#ifdef _MSC_VER
    // Включаем автоматический отчет об утечках в панель "Вывод приложения" при закрытии
    _CrtSetDbgFlag(_CRT_DBG_ALLOC_MEM_DF | _CRT_DBG_LEAK_CHECK_DF);

    // Если хотите направить вывод в консоль, раскомментируйте:
    // _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
    // _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDOUT);
#endif

    QApplication a(argc, argv);
    MainWindow w;
    w.showFullScreen();
    return a.exec();
}
