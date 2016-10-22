#include "feeds.h"
#include "newsblurconnection.h"

#include <QStringList>
#include <QDebug>

Feeds::Feeds(QObject *parent) :
    QAbstractListModel(parent)
{
	m_nullfolder.name = "";

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
	qDebug() << "Feeds reset gotten by:" << m_folderName;
	QVector<int> roles;
	roles += RoleUnread;
	roles += RoleUnreadStr;

    for (int i = 0; i < m_list.count(); i++) {
		bool changed = false;

        if (m_list[i].id == feedId) {
            m_list[i].unread = 0;
			changed = true;
        } else if (m_list[i].isFolder) {
			const NewsBlurConnection::Folder &folder = findFolder(m_list[i].title);
			if (folder.name != "" && folderHasFeed(folder, feedId)) {
				m_list[i].unread = calculateFolderUnread(folder);
				changed = true;
			}
		}

		if (changed)
            emit dataChanged(createIndex(i, 0), createIndex(i, 0), roles);
    }
}

void Feeds::storyRead(int feedId, const QString &hash)
{
	QVector<int> roles;
	roles += RoleUnread;
	roles += RoleUnreadStr;

    for (int i = 0; i < m_list.count(); i++) {
		bool changed = false;

        if (m_list[i].id == feedId) {
            m_list[i].unread--;
			changed = true;
        } else if (m_list[i].isFolder) {
			const NewsBlurConnection::Folder & folder = findFolder(m_list[i].title);
			if (folder.name != "" && folderHasFeed(folder, feedId)) {
				m_list[i].unread = calculateFolderUnread(folder);
				changed = true;
			}
		}

		if (changed)
            emit dataChanged(createIndex(i, 0), createIndex(i, 0), roles);
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
		const NewsBlurConnection::Folder &folder = findFolder(m_folderName);
		
		if (folder.name != "") {
			entriesFromFolder(folder);	
		} else {
			qWarning() << "Unable to find folder:" << m_folderName;
		}
    }

	qSort(m_list.begin(), m_list.end(), entryCompare);

    endResetModel();
}

const NewsBlurConnection::Folder& Feeds::findFolder (const QString &name)
{
	foreach(const NewsBlurConnection::Folder &thisfolder, NewsBlurConnection::instance()->foldersData()) {
		if (thisfolder.name == name) {
			return thisfolder;
		}
	}

	return m_nullfolder;
}

int Feeds::calculateFolderUnread (const NewsBlurConnection::Folder &folder)
{
	qDebug() << "Calculating unread for: " << folder.name;

	int retval = 0;
	QHash<int, NewsBlurConnection::Feed> feeds = NewsBlurConnection::instance()->feedsData();

	foreach(int feed, folder.feeds) {
		retval += feeds[feed].unread;
	}

	return retval;
}

bool Feeds::folderHasFeed (const NewsBlurConnection::Folder &folder, int infeed)
{
	foreach(int feed, folder.feeds) {
		if (feed == infeed)
			return true;
	}
	return false;
}

void Feeds::entriesFromFolders()
{
	foreach(const NewsBlurConnection::Folder &folder, NewsBlurConnection::instance()->foldersData()) {
		Entry entry;

		entry.title = folder.name;
		entry.isFolder = true;
		entry.unread = 0;
		entry.id = 0;
		entry.unread = calculateFolderUnread(folder);

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
        if (entry.unread > 0) {
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
