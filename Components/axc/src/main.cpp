#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <xmllistmodel.h>
#include <QtDebug>
#include <QIcon>
#include <QtQuick>
#include "highlighter.h"
#include "string_encoder.h"
#include "Conventions.h"
#include <memory>

void passConventionsTo(QQmlContext *context) {
    AE::conv_map m;
    AE::fillConventions(m);
    for (auto iter = m.begin(); iter != m.end(); iter++) {
        context->setContextProperty((*iter).first, QVariant::fromValue(QString((*iter).second)));
    }
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("utf-8"));

    XMLListModel *model = new XMLListModel();
    String_encoder *enc = new String_encoder();

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("xml_model", model);
    engine.rootContext()->setContextProperty("encoder", enc);
    //Add conventions to js context
    passConventionsTo(engine.rootContext());

    engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));

    QList<QObject*> rol = engine.rootObjects();
    QObject *ro = rol.first();
    QQuickTextDocument *td = ro->findChild<QQuickTextDocument*>("TDHighlighted");
    if (!td) {
        qDebug() << "could not find TDHighlighted";
    } else {
        new Highlighter(td->textDocument());
    }

    return app.exec();
}
