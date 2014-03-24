#include "newsblurconnection.h"
#include "feeds.h"
#include "stories.h"

#include "qtquick2applicationviewer.h"

#include <QtGui/QGuiApplication>
#include <QtQml/qqml.h>

static QObject* connectionInstance(QQmlEngine* /* engine */, QJSEngine* /* scriptEngine */)
{
    return NewsBlurConnection::instance();
}


int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    qmlRegisterSingletonType<NewsBlurConnection>("NewsBlur", 0, 1, "NewsBlur", connectionInstance);
    qmlRegisterType<Feeds>("NewsBlur", 0, 1, "Feeds");
    qmlRegisterType<Stories>("NewsBlur", 0, 1, "Stories");

    QtQuick2ApplicationViewer viewer;
    viewer.setMainQmlFile(QStringLiteral("qml/newsblur/main.qml"));
    viewer.showExpanded();

    return app.exec();
}
