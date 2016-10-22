#include "feeds.h"
#include "newsblurconnection.h"

#include <QStringList>
#include <QDebug>

Feeds::Feeds(QObject *parent) :
    QAbstractListModel(parent)
{
    connect(NewsBlurConnection::instance(), &NewsBlurConnection::feedsUpdated, this, &Feeds::feedsUpdated);
    connect(NewsBlurConnection::instance(), &NewsBlurConnection::feedReset, this, &Feeds::feedReset);
    connect(NewsBlurConnection::instance(), &NewsBlurConnection::storyRead, this, &Feeds::storyRead);
}

void Feeds::componentComplete()
{
    refresh();
}

void Feeds::feedsUpdated()
{
    qDebug() << "feeds updated";
    refresh();
}

void Feeds::feedReset(int feedId)
{
	QVector<int> roles;
	roles += RoleUnread;
	roles += RoleUnreadStr;

    for (int i = 0; i < m_list.count(); i++) {
        if (m_list[i].id == feedId) {
            m_list[i].unread = 0;
            emit dataChanged(createIndex(i, 0), createIndex(i, 0), roles);
        }
    }
}

void Feeds::storyRead(int feedId, const QString &hash)
{
	QVector<int> roles;
	roles += RoleUnread;
	roles += RoleUnreadStr;

    for (int i = 0; i < m_list.count(); i++) {
        if (m_list[i].id == feedId) {
            m_list[i].unread--;
            emit dataChanged(createIndex(i, 0), createIndex(i, 0), roles);
        }
    }
}

bool Feeds::entryCompare (const Entry& a, const Entry& b)
{
	return a.title < b.title;
}

void Feeds::refresh() {
    qDebug() << "refreshing for folder: " << folderName();

    beginResetModel();

    m_list.clear();

    if (folderName() == "") {
		entriesFromFolders();
	} else {
		bool found = false;

		foreach(const NewsBlurConnection::Folder &thisfolder, NewsBlurConnection::instance()->foldersData()) {
			if (thisfolder.name == m_folderName) {
				entriesFromFolder(thisfolder);	
				found = true;
				break;
			}
		}

		if (!found) {
			qWarning() << "Unable to find folder:" << m_folderName;
		}
    }

	qSort(m_list.begin(), m_list.end(), entryCompare);

    endResetModel();
}

void Feeds::entriesFromFolders()
{
	foreach(const NewsBlurConnection::Folder &folder, NewsBlurConnection::instance()->foldersData()) {
		Entry entry;

		entry.title = folder.name;
		entry.isFolder = true;
		entry.unread = 0; // TODO: Fix this

		m_list.append(entry);
	}
}

void Feeds::entriesFromFolder(const NewsBlurConnection::Folder &folder)
{
	QHash<int, NewsBlurConnection::Feed> feeds = NewsBlurConnection::instance()->feedsData();
	qDebug() << "Looking at folder: " << folder.name;

	foreach(int feed, folder.feeds) {
		Entry entry;

		entry.title = feeds[feed].name;
		entry.isFolder = false;
		entry.id = feeds[feed].id;
		entry.unread = feeds[feed].unread;

		m_list.append(entry);

		qDebug() << "\tfeed name" << feeds[feed].name;
	}
}

int Feeds::rowCount(const QModelIndex & /*parent*/) const
{
    return m_list.count();
}

QVariant Feeds::data(const QModelIndex &index, int role) const
{
    switch(role) {
    case RoleId:
        return m_list.at(index.row()).id;
    case RoleTitle:
        return m_list.at(index.row()).title;
    case RoleIsFolder:
        return m_list.at(index.row()).isFolder;
    case RoleUnread:
        return m_list.at(index.row()).unread;
    case RoleUnreadStr:
		const Entry &entry = m_list.at(index.row());
        if (entry.unread > 0 || entry.isFolder) {
			return "someunread";
		} else {
			return "noneunread";
		}
    }

    return QVariant();
}

QHash<int, QByteArray> Feeds::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles.insert(RoleTitle, "feedtitle");
    roles.insert(RoleIsFolder, "isFolder");
    roles.insert(RoleId, "feedId");
    roles.insert(RoleUnread, "unread");
    roles.insert(RoleUnreadStr, "unreadstr");
    return roles;
}

QString Feeds::folderName() const
{
    return m_folderName;
}

void Feeds::setFolderName(const QString &folderName)
{
	qDebug() << "Setting folder name:" << folderName;
    if (m_folderName != folderName) {
        m_folderName = folderName;
        emit folderNameChanged();
        refresh();
    }
}
