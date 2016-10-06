
#include "stories.h"
#include "newsblurconnection.h"

#include <QStringList>
#include <QDebug>

Stories::Stories(QObject *parent) :
    QAbstractListModel(parent)
{
    connect(NewsBlurConnection::instance(), &NewsBlurConnection::entriesFetched, this, &Stories::entriesFetched);
    connect(NewsBlurConnection::instance(), &NewsBlurConnection::feedDecremented, this, &Stories::feedDecremented);
    connect(NewsBlurConnection::instance(), &NewsBlurConnection::feedReset, this, &Stories::feedReset);
    connect(NewsBlurConnection::instance(), &NewsBlurConnection::storyStarred, this, &Stories::storyStarred);
    connect(this, &Stories::pageUpdateStarted, this, &Stories::pageUpdateStart);
}

void Stories::componentComplete()
{
}

void Stories::feedDecremented(int feedId, const QString &hash) {
	if (m_feedId != feedId) {
		return;
	}

	QVector<int> roles;
	roles += RoleRead;

    for (int i = 0; i < m_list.count(); i++) {
        if (m_list[i].hash == hash) {
            m_list[i].read = true;
            emit dataChanged(createIndex(i, 0), createIndex(i, 0), roles);
        }
    }
}

void Stories::feedReset (int feedId) {
	if (m_feedId != feedId) {
		return;
	}

	refresh();
}

void Stories::entriesFetched(const QVariant &entriesData)
{
	if (entriesData.toMap()["feed_id"].toInt() != m_feedId) {
		qDebug() << "Feed Data Not for us. Expecting" << m_feedId << "but got" << entriesData.toMap()["feed_id"].toInt();
		return;
	}

    qDebug() << "feeds updated";
    QVariantList storyData = entriesData.toMap()["stories"].toList();

	beginInsertRows(QModelIndex(), m_list.count(), m_list.count() + storyData.count() - 1);

    foreach (const QVariant &story, storyData) {
		// qDebug() << "Story info: " << story;
        QVariantMap storymap = story.toMap();
        StoryEntry entry;

        entry.content = storymap["story_content"].toString();
        entry.link = storymap["story_permalink"].toString();
        entry.title = storymap["story_title"].toString();
        entry.hash = storymap["story_hash"].toString();
        entry.timestamp = storymap["story_data"].toString();
		entry.read = storymap["read_status"].toInt() != 0;
		entry.starred = false; /* TODO */

		QList<QVariant> imagelist = storymap["image_urls"].toList();
		if (imagelist.size() > 0)
			entry.imageurl = imagelist.first().toString();

		m_list.append(entry);
    }

    qDebug() << "Got" << m_list.count() << "stories for feed" << m_feedId;

	bool prev = m_storiesAvailable;
	m_storiesAvailable = storyData.count() != 0 || entriesData.toMap()["hidden_stories_removed"].toInt() != 0;
	if (prev != m_storiesAvailable)
		emit storiesAvailableChanged();

	m_requestInProgress = false;

	qDebug() << "Refresh complete" << (m_storiesAvailable ? ", more stories available" : "");

    endInsertRows();

	emit loadingChanged();
}

void Stories::refresh() {
    beginResetModel();

	m_storiesAvailable = true;
	m_list.clear();
	m_page = 1;

    NewsBlurConnection::instance()->feedEntries(m_feedId, m_page);
	m_requestInProgress = true;

    endResetModel();

	emit loadingChanged();
}

int Stories::rowCount(const QModelIndex & /*parent*/) const
{
    return m_list.count();
}

void Stories::pageUpdateStart (void)
{
	if (!m_requestInProgress) {
		NewsBlurConnection::instance()->feedEntries(m_feedId, ++m_page);
		m_requestInProgress = true;
	}
}

QVariant Stories::data(const QModelIndex &index, int role) const
{
	if (index.row() == m_list.count() - 1 && m_storiesAvailable) {
		emit pageUpdateStarted();
	}

    switch(role) {
    case RoleTitle:
        return m_list.at(index.row()).title;
    case RoleHash:
        return m_list.at(index.row()).hash;
    case RoleStarred:
        return m_list.at(index.row()).starred;
    case RoleContent:
        return m_list.at(index.row()).content;
    case RoleLink:
        return m_list.at(index.row()).link;
    case RoleTimestamp:
        return m_list.at(index.row()).timestamp;
    case RoleRead:
        return m_list.at(index.row()).read;
    case RoleImageUrl:
        return m_list.at(index.row()).imageurl;
    }

    return QVariant();
}

QHash<int, QByteArray> Stories::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles.insert(RoleTitle, "storytitle");
    roles.insert(RoleHash, "hash");
    roles.insert(RoleStarred, "starred");
    roles.insert(RoleContent, "content");
    roles.insert(RoleLink, "link");
    roles.insert(RoleTimestamp, "timestamp");
    roles.insert(RoleRead, "read");
    roles.insert(RoleImageUrl, "imageurl");
    return roles;
}

int Stories::feedId() const
{
    return m_feedId;
}

bool Stories::loading() const
{
	return m_requestInProgress && m_list.count() == 0;
}

bool Stories::storiesAvailable() const
{
    return m_storiesAvailable;
}

void Stories::setFeedId(int feedId)
{
    qDebug() << "Story model created for feed:"<< feedId;
    if (m_feedId != feedId) {
        m_feedId = feedId;
        emit feedIdChanged();
        refresh();
    }
}

void Stories::markStoryHashRead(const QString &hash)
{
	NewsBlurConnection::instance()->markStoryHashRead(hash, feedId());
}

void Stories::markFeedRead ()
{
	NewsBlurConnection::instance()->markFeedRead(feedId());
}

void Stories::markStoryHashStarred(const QString &hash)
{
	NewsBlurConnection::instance()->markStoryHashStarred(hash, feedId());
}


void Stories::storyStarred(int feedId, const QString &hash) {
	if (m_feedId != feedId) {
		return;
	}

	QVector<int> roles;
	roles += RoleStarred;

    for (int i = 0; i < m_list.count(); i++) {
        if (m_list[i].hash == hash) {
            m_list[i].starred = true;
            emit dataChanged(createIndex(i, 0), createIndex(i, 0), roles);
        }
    }
}
