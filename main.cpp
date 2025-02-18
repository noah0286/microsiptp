#include <QApplication>
#include <QFile>
 #include <QLatin1String>

#include "managermainwindow.h"

QString readStyleSheet(QString path) {
    QFile file(path);
    file.open(QFile::ReadOnly);
    return QLatin1String(file.readAll());
}

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    qSetMessagePattern("[%{time yyyyMMdd h:mm:ss.zzz t} %{if-debug}D%{endif}%{if-info}I%{endif}%{if-warning}W%{endif}%{if-critical}C%{endif}%{if-fatal}F%{endif}] %{function}:%{line} - %{message}");

    QIcon appIcon(":/image/telephone.png");
    app.setWindowIcon(appIcon);
    app.setStyleSheet(readStyleSheet(":/qss/theme_dark_medical/dark_medical.qss"));

    ManagerMainWindow m;
    return app.exec();
}
