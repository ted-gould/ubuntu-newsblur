#ifndef NEWSBLURCONNECTION_H
#define NEWSBLURCONNECTION_H

#include <QObject>
#include <QVariant>

class QNetworkAccessManager;

class NewsBlurConnection : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString username READ username NOTIFY usernameChanged)
    Q_PROPERTY(bool authenticated READ authenticated NOTIFY authenticatedChanged)

public:
    static NewsBlurConnection *instance();

    QString username() const;
    void setUsername(const QString &username);

    bool authenticated() const;

    QVariant feedsData() const;

public slots:
    void createUser(const QString &username, const QString &email, const QString &password = QString());
    void login(const QString &username, const QString &password = QString());
    void fetchFeeds();

private slots:
    void userCreated();
    void loggedIn();
    void feedsFetched();

private:
    explicit NewsBlurConnection(QObject *parent = 0);
    static NewsBlurConnection *s_instance;

signals:
    void usernameChanged();
    void authenticatedChanged();
    void feedsUpdated(const QVariant &feedsData);

private:
    QNetworkAccessManager *m_nam;
    bool m_authenticated;
    QString m_username;
    QVariant m_feedsData;

};

#endif // NEWSBLURCONNECTION_H
