
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
    refresh();
}

void Stories::entriesFetched(const QVariant &entriesData)
{
    beginResetModel();

    m_list.clear();

    qDebug() << "feeds updated";
    m_storyData = entriesData.toHash()["stories"].toList();

    foreach (const QVariant &story, m_storyData) {
        QVariantMap storymap = story.toMap();
        StoryEntry entry;

        entry.content = storymap["story_content"].toString();
        entry.link = storymap["story_permalink"].toString();
        entry.title = storymap["story_title"].toString();
        entry.hash = storymap["story_hash"].toString();
        entry.timestamp = storymap["story_data"].toString();

        m_list.append(entry);
    }

    endResetModel();
}

void Stories::refresh() {
    NewsBlurConnection::instance()->feedEntries(m_feedId);
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
        emit feedIdChanged();
        refresh();
    }
}
