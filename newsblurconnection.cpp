#include "newsblurconnection.h"

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QDebug>

NewsBlurConnection* NewsBlurConnection::s_instance = 0;

NewsBlurConnection::NewsBlurConnection(QObject *parent) :
    QObject(parent),
    m_nam(new QNetworkAccessManager(this)),
    m_authenticated(false)
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

void NewsBlurConnection::login(const QString &username, const QString &password)
{
    if (m_username != username) {
        m_username = username;
        emit usernameChanged();
    }

    if (!m_username.isEmpty() && !m_authenticated) {
        QNetworkRequest request;
        request.setUrl(QUrl("https://newsblur.com/api/login"));
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

        QByteArray postData;
        postData.append(QString("username=%1").arg(m_username));
        if (!password.isEmpty()) {
            postData.append(QString("&password=%1").arg(password));
        }
        qDebug() << "posting" << postData << "to" << request.url();
        QNetworkReply *reply = m_nam->post(request, postData);
        connect(reply, &QNetworkReply::finished, this, &NewsBlurConnection::loggedIn);
    }
}

void NewsBlurConnection::createUser(const QString &username, const QString &email, const QString &password)
{
    QNetworkRequest request;
    request.setUrl(QUrl("https://newsblur.com/api/signup"));

    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    QByteArray postData;
    postData.append(QString("username=%1").arg(username));
    postData.append(QString("&email=%1").arg(email));
    if (!password.isEmpty()) {
        postData.append(QString("&password=%1").arg(password));
    }

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
        return;
    }

    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(reply->readAll(), &error);
    if (error.error != QJsonParseError::NoError) {
        qWarning() << "Failed to parse login response" << error.errorString();
        m_authenticated = false;
        emit authenticatedChanged();
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
    request.setUrl(QUrl("https://newsblur.com/reader/feeds"));

//    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

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
    QString url = QString("https://newsblur.com/reader/feed/%1?page=%2&read_filter=unread").arg(feedId).arg(page);
    qDebug() << "feed url: " << url;
    request.setUrl(QUrl(url));

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

void NewsBlurConnection::markStoryHashRead(const QString &hash)
{
    QNetworkRequest request;
    QString url = QString("https://newsblur.com/reader/mark_story_hashes_as_read?story_hash=%1").arg(hash);
    qDebug() << "feed url: " << url;
    request.setUrl(QUrl(url));
	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

	QByteArray data;
    m_nam->post(request, data);
}
