#include "newsblurconnection.h"

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QDebug>
#include <QUrlQuery>

NewsBlurConnection* NewsBlurConnection::s_instance = 0;

NewsBlurConnection::NewsBlurConnection(QObject *parent, QString baseurl) :
    QObject(parent),
    m_nam(new QNetworkAccessManager(this)),
    m_authenticated(false),
    m_baseurl(baseurl)
{
}

NewsBlurConnection *NewsBlurConnection::instance()
{
    if (!s_instance) {
        s_instance = new NewsBlurConnection();
    }
    return s_instance;
}

bool NewsBlurConnection::authenticated() const
{
    return m_authenticated;
}

QString NewsBlurConnection::username() const
{
    return m_username;
}


QString NewsBlurConnection::error() const
{
    return m_error;
}

void NewsBlurConnection::setError(const QString &error)
{
	if (m_error != error) {
		m_error = error;
		errorChanged();
	}
}

void NewsBlurConnection::login(const QString &username, const QString &password)
{
    if (m_username != username) {
        m_username = username;
        emit usernameChanged();
    }

    if (!m_username.isEmpty() && !m_authenticated) {
        QNetworkRequest request;
        request.setUrl(QUrl(m_baseurl + "/api/login"));
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

		QUrlQuery query = QUrlQuery();
		query.addQueryItem("username", m_username);
        if (!password.isEmpty()) {
			query.addQueryItem("password", password);
		}

        QByteArray postData;
		postData.append(query.query());

        // qDebug() << "posting" << postData << "to" << request.url();
        QNetworkReply *reply = m_nam->post(request, postData);
        connect(reply, &QNetworkReply::finished, this, &NewsBlurConnection::loggedIn);
    }
}

void NewsBlurConnection::createUser(const QString &username, const QString &email, const QString &password)
{
    QNetworkRequest request;
    request.setUrl(QUrl(m_baseurl + "/api/signup"));

    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

	QUrlQuery query = QUrlQuery();
	query.addQueryItem("username", m_username);
	query.addQueryItem("email", email);
	if (!password.isEmpty()) {
		query.addQueryItem("password", password);
	}

	QByteArray postData;
	postData.append(query.query());

    QNetworkReply *reply = m_nam->post(request, postData);
    connect(reply, &QNetworkReply::finished, this, &NewsBlurConnection::userCreated);
}

void NewsBlurConnection::userCreated()
{
    QNetworkReply *reply = static_cast<QNetworkReply*>(sender());
    qDebug() << "user created" << reply->errorString();
}

void NewsBlurConnection::loggedIn()
{
    QNetworkReply *reply = static_cast<QNetworkReply*>(sender());
    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "Failed to log in" << reply->errorString();
        m_authenticated = false;
        emit authenticatedChanged();

		setError("Networking issue: " + reply->errorString());
        return;
    }

    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(reply->readAll(), &error);
    if (error.error != QJsonParseError::NoError) {
        qWarning() << "Failed to parse login response" << error.errorString();
        m_authenticated = false;
        emit authenticatedChanged();

		setError("Unable to authenticate");
        return;
    }

    if (jsonDoc.toVariant().toMap().value("authenticated").toBool()) {
        qDebug() << "logged in!";
        m_authenticated = true;
    }
    fetchFeeds();
    emit authenticatedChanged();
}

void NewsBlurConnection::fetchFeeds()
{
    qDebug() << "fetching feeds...";
    QNetworkRequest request;
    request.setUrl(QUrl(m_baseurl + "/reader/feeds"));

    QNetworkReply *reply = m_nam->get(request);
    connect(reply, &QNetworkReply::finished, this, &NewsBlurConnection::feedsFetched);
}

void NewsBlurConnection::feedsFetched()
{
    qDebug() << "got feeds reply";
    QNetworkReply *reply = static_cast<QNetworkReply*>(sender());
    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "Error fetching feeds" << reply->errorString();
        return;
    }

    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(reply->readAll(), &error);
    if (error.error != QJsonParseError::NoError) {
        qWarning() << "Error parsing feeds" << error.errorString();
        return;
    }

	m_feedsData = jsonDoc.toVariant();

    emit feedsUpdated(m_feedsData);
}

QVariant NewsBlurConnection::feedsData() const
{
    return m_feedsData;
}

void NewsBlurConnection::feedEntries(int feedId, int page)
{
    qDebug() << "getting results for feed " << feedId;

    QNetworkRequest request;
    QUrl url(m_baseurl + QString("/reader/feed/%1").arg(feedId));

	QUrlQuery query;
	query.addQueryItem("page", QString::number(page));
	url.setQuery(query);

    qDebug() << "feed url: " << url;
    request.setUrl(url);

    QNetworkReply *reply = m_nam->get(request);
    connect(reply, &QNetworkReply::finished, this, &NewsBlurConnection::feedEntriesFetched);
}

void NewsBlurConnection::feedEntriesFetched()
{
    qDebug() << "got feed entries reply";
    QNetworkReply *reply = static_cast<QNetworkReply*>(sender());
    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "Error fetching feed entries" << reply->errorString();
        return;
    }

    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(reply->readAll(), &error);
    if (error.error != QJsonParseError::NoError) {
        qWarning() << "Error parsing feed entries" << error.errorString();
        return;
    }

    QVariant feedEntriesData = jsonDoc.toVariant();

    emit entriesFetched(feedEntriesData);
}

void NewsBlurConnection::markStoryHashRead(const QString &hash, int feedId)
{
	/* Mark the story as read in the read count for the feed */
    emit feedDecremented(feedId, hash);

	/* Sent the request to the API to mark it as read on the server */
    QNetworkRequest request;
    QUrl url(m_baseurl + QString("/reader/mark_story_hashes_as_read"));

	QUrlQuery query;
	query.addQueryItem("story_hash", hash);
	url.setQuery(query);

    qDebug() << "feed url: " << url;
    request.setUrl(QUrl(url));
	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

	QByteArray data;
    QNetworkReply * reply = m_nam->post(request, data);
    connect(reply, &QNetworkReply::finished, this, &NewsBlurConnection::storyMarkedRead);

	m_storyReadId = feedId; /* This sucks */
	m_storyReadHash = hash; /* This sucks */
}

void NewsBlurConnection::storyMarkedRead()
{
    QNetworkReply *reply = static_cast<QNetworkReply*>(sender());
    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "Unable to mark read " << m_storyReadId << ", " << m_storyReadHash << ":" << reply->errorString();
        return;
    } else {
		qDebug() << "Story read: " << m_storyReadHash << " on feed " << m_storyReadId;
		emit storyRead(m_storyReadId, m_storyReadHash);
	}
}

void NewsBlurConnection::markStoryHashStarred(const QString &hash, int feedId)
{
	/* Sent the request to the API to mark it as starred on the server */
    QNetworkRequest request;
    QUrl url(m_baseurl + QString("/reader/mark_story_hashes_as_starred"));

	QUrlQuery query;
	query.addQueryItem("story_hash", hash);
	url.setQuery(query);

    qDebug() << "feed url: " << url;
    request.setUrl(url);
	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

	QByteArray data;
    QNetworkReply * reply = m_nam->post(request, data);
    connect(reply, &QNetworkReply::finished, this, &NewsBlurConnection::storyMarkedStarred);
	m_storyStarredId = feedId; /* This sucks */
	m_storyStarredHash = hash; /* This sucks */
}

void NewsBlurConnection::storyMarkedStarred()
{
    QNetworkReply *reply = static_cast<QNetworkReply*>(sender());
    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "Unable to reset feed" << m_feedResetId << ":" << reply->errorString();
        return;
    } else {
		qDebug() << "Story starred: " << m_storyStarredHash << " on feed " << m_storyStarredId;
		emit storyStarred(m_storyStarredId, m_storyStarredHash);
	}
}

void NewsBlurConnection::markFeedRead(int feedId)
{
    QNetworkRequest request;
    QUrl url(m_baseurl + QString("/reader/mark_feed_as_read"));

	QUrlQuery query;
	query.addQueryItem("feed_id", QString::number(feedId));
	url.setQuery(query);

    qDebug() << "feed url: " << url;
    request.setUrl(url);
	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

	QByteArray data;
    QNetworkReply* reply = m_nam->post(request, data);
    connect(reply, &QNetworkReply::finished, this, &NewsBlurConnection::feedMarkedRead);
	m_feedResetId = feedId; /* This sucks */
}

void NewsBlurConnection::feedMarkedRead ()
{
    QNetworkReply *reply = static_cast<QNetworkReply*>(sender());
    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "Unable to reset feed" << m_feedResetId << ":" << reply->errorString();
        return;
    } else {
		qDebug() << "Feed Reset: " << m_feedResetId;
		emit feedReset(m_feedResetId);
	}
}

void NewsBlurConnection::shareStory (int feedId, const QString& story, const QString& comments)
{
    QNetworkRequest request;
	QUrl url = QUrl(m_baseurl + "/social/share_story");

	QUrlQuery query = QUrlQuery();
	query.addQueryItem("feed_id", QString::number(feedId));
	query.addQueryItem("story_id", story);
	query.addQueryItem("comments", comments);

	QByteArray data;
	data.append(query.query());

    qDebug() << "share url: " << url;
    request.setUrl(url);
	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    QNetworkReply * reply = m_nam->post(request, data);
    connect(reply, &QNetworkReply::finished, this, &NewsBlurConnection::storyShared);
}

void NewsBlurConnection::storyShared (void)
{
    QNetworkReply *reply = static_cast<QNetworkReply*>(sender());
    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "Error sharing story" << reply->errorString();
        return;
    } else {
		qDebug() << "Story Shared";
	}

	return;
}
