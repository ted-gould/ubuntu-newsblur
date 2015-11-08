
#include "stories.h"
#include "newsblurconnection.h"

#include <QStringList>
#include <QDebug>

Stories::Stories(QObject *parent) :
    QAbstractListModel(parent)
{
    connect(NewsBlurConnection::instance(), &NewsBlurConnection::entriesFetched, this, &Stories::entriesFetched);
    connect(this, &Stories::pageUpdateStarted, this, &Stories::pageUpdateStart);
}

void Stories::componentComplete()
{
}

void Stories::entriesFetched(const QVariant &entriesData)
{
	if (entriesData.toMap()["feed_id"].toInt() != m_feedId) {
		qDebug() << "Feed Data Not for us. Expecting" << m_feedId << "but got" << entriesData.toMap()["feed_id"].toInt();
		return;
	}

    beginResetModel();

    qDebug() << "feeds updated";
    QVariantList storyData = entriesData.toMap()["stories"].toList();

    foreach (const QVariant &story, storyData) {
        QVariantMap storymap = story.toMap();
        StoryEntry entry;

        entry.content = storymap["story_content"].toString();
        entry.link = storymap["story_permalink"].toString();
        entry.title = storymap["story_title"].toString();
        entry.hash = storymap["story_hash"].toString();
        entry.timestamp = storymap["story_data"].toString();
		entry.read = storymap["read_status"].toInt() == 0;

		m_list.append(entry);
    }

    qDebug() << "Got" << m_list.count() << "stories for feed" << m_feedId;

    endResetModel();

	m_storiesAvailable = storyData.count() != 0 || entriesData.toMap()["hidden_stories_removed"].toInt() != 0;
	m_requestInProgress = false;
}

void Stories::refresh() {
	m_storiesAvailable = true;
	m_list.clear();
	m_page = 1;

    NewsBlurConnection::instance()->feedEntries(m_feedId, m_page);
	m_requestInProgress = true;
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
    case RoleContent:
        return m_list.at(index.row()).content;
    case RoleLink:
        return m_list.at(index.row()).link;
    case RoleTimestamp:
        return m_list.at(index.row()).timestamp;
    case RoleRead:
        return m_list.at(index.row()).read;
    }

    return QVariant();
}

QHash<int, QByteArray> Stories::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles.insert(RoleTitle, "title");
    roles.insert(RoleHash, "hash");
    roles.insert(RoleContent, "content");
    roles.insert(RoleLink, "link");
    roles.insert(RoleTimestamp, "timestamp");
    roles.insert(RoleRead, "read");
    return roles;
}

int Stories::feedId() const
{
    return m_feedId;
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
