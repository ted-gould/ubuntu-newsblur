
#include "stories.h"
#include "newsblurconnection.h"

#include <QStringList>
#include <QDebug>

Stories::Stories(QObject *parent) :
    QAbstractListModel(parent)
{
    connect(NewsBlurConnection::instance(), &NewsBlurConnection::entriesFetched, this, &Stories::entriesFetched);
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

		if (storymap["read_status"].toInt() == 0)
			m_list.append(entry);
    }

    qDebug() << "Got" << m_list.count() << "stories for feed" << m_feedId;

    endResetModel();

	if (storyData.count() != 0 || entriesData.toMap()["hidden_stories_removed"].toInt() != 0) {
		NewsBlurConnection::instance()->feedEntries(m_feedId, ++m_page);
	}
}

void Stories::refresh() {
    NewsBlurConnection::instance()->feedEntries(m_feedId, m_page);
}

int Stories::rowCount(const QModelIndex & /*parent*/) const
{
    return m_list.count();
}

QVariant Stories::data(const QModelIndex &index, int role) const
{
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
		m_list.clear();
		m_page = 1;
        emit feedIdChanged();
        refresh();
    }
}
