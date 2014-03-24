
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
    qDebug() << "feeds updated";
	m_storyData = entriesData.toHash();
	/* TODO: Parse data */
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
    case RoleId:
        return m_list.at(index.row()).id;
    case RoleTitle:
        return m_list.at(index.row()).title;
    }

    return QVariant();
}

QHash<int, QByteArray> Stories::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles.insert(RoleTitle, "title");
	roles.insert(RoleId, "id");
    return roles;
}

int Stories::feedId() const
{
    return m_feedId;
}

void Stories::setFeedId(int feedId)
{
    if (m_feedId != feedId) {
        m_feedId = feedId;
        emit feedIdChanged();
        refresh();
    }
}
