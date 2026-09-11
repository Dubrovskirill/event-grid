#include <QFile>
#include <QtTest>

class QmlResourcesTest final : public QObject
{
    Q_OBJECT

private slots:
    void requiredScreensAreEmbedded();
};

void QmlResourcesTest::requiredScreensAreEmbedded()
{
    QVERIFY2(QFile::exists(QStringLiteral(":/qml/Main.qml")),
             "Main.qml must be embedded in the application resources");
    QVERIFY2(QFile::exists(QStringLiteral(":/qml/MapsScreen.qml")),
             "MapsScreen.qml must be embedded in the application resources");
    QVERIFY2(QFile::exists(QStringLiteral(":/qml/CreateMapScreen.qml")),
             "CreateMapScreen.qml must be embedded in the application resources");
}

QTEST_APPLESS_MAIN(QmlResourcesTest)

#include "qml_resources_test.moc"
