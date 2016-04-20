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
    Q_PROPERTY(QString error READ error NOTIFY errorChanged)

public:
    static NewsBlurConnection *instance();

    QString username() const;
    void setUsername(const QString &username);

    bool authenticated() const;

    QVariant feedsData() const;

    QString error() const;
    void setError(const QString &error);

public slots:
    void createUser(const QString &username, const QString &email, const QString &password = QString());
    void login(const QString &username, const QString &password = QString());
    void fetchFeeds();
	void feedEntries(int feedId, int page = 1);
	void markStoryHashRead (const QString &hash, int feedId);
	void markFeedRead (int feedId);

private slots:
    void userCreated();
    void loggedIn();
    void feedsFetched();
    void feedEntriesFetched();

private:
    explicit NewsBlurConnection(QObject *parent = 0);
    static NewsBlurConnection *s_instance;

signals:
    void usernameChanged();
    void authenticatedChanged();
    void feedsUpdated(const QVariant &feedsData);
    void entriesFetched(const QVariant &entriesData);
    void errorChanged();
    void feedDecremented(int feedId, const QString &hash);

private:
    QNetworkAccessManager *m_nam;
    bool m_authenticated;
    QString m_username;
    QVariant m_feedsData;
	QString m_error;

};

#endif // NEWSBLURCONNECTION_H
