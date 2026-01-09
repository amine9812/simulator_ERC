#include <QApplication>

#include "app/AppWindow.h"

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);
  hadak::AppWindow window;
  window.show();
  return app.exec();
}
