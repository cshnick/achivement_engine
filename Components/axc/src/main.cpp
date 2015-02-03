#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <xmllistmodel.h>
#include <QtDebug>
#include <QIcon>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    XMLListModel *model = new XMLListModel();

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("xml_model", model);
    engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));

    return app.exec();
}
