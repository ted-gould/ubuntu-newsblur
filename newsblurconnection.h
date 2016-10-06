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
	void markStoryHashStarred (const QString &hash, int feedId);
	void markFeedRead (int feedId);
	void shareStory (int feedId, const QString& story, const QString& comments);

private slots:
    void userCreated();
    void loggedIn();
    void feedsFetched();
    void feedEntriesFetched();
	void feedMarkedRead();
	void storyMarkedStarred();
	void storyMarkedRead();
	void storyShared();

private:
    explicit NewsBlurConnection(QObject *parent = 0, QString baseurl = "https://newsblur.com");
    static NewsBlurConnection *s_instance;

signals:
    void usernameChanged();
    void authenticatedChanged();
    void feedsUpdated(const QVariant &feedsData);
    void entriesFetched(const QVariant &entriesData);
    void errorChanged();
    void feedDecremented(int feedId, const QString &hash);
    void feedReset(int feedId);
	void storyStarred(int feedId, const QString &hash);
	void storyRead(int feedId, const QString &hash);

private:
    QNetworkAccessManager *m_nam;
    bool m_authenticated;
    QString m_baseurl;
    QString m_username;
    QVariant m_feedsData;
    QString m_error;
	int m_feedResetId;
	int m_storyStarredId;
	QString m_storyStarredHash;
	int m_storyReadId;
	QString m_storyReadHash;

};

#endif // NEWSBLURCONNECTION_H
