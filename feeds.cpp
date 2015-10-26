#include "feeds.h"
#include "newsblurconnection.h"

#include <QStringList>
#include <QDebug>

Feeds::Feeds(QObject *parent) :
    QAbstractListModel(parent)
{
    connect(NewsBlurConnection::instance(), &NewsBlurConnection::feedsUpdated, this, &Feeds::feedsUpdated);
}

void Feeds::componentComplete()
{
    refresh();
}

void Feeds::feedsUpdated(const QVariant &feedsData)
{
    Q_UNUSED(feedsData)
    qDebug() << "feeds updated";
    refresh();
}

void Feeds::refresh() {

    qDebug() << "refreshing for folderNode" << m_filterPath;

    QVariantList folders = NewsBlurConnection::instance()->feedsData().toMap().value("folders").toList();
    QVariantList folderNode;
    if (m_filterPath.isEmpty()) {
        folderNode = folders;
    } else {
        folderNode = findNode(folders, m_filterPath);
    }
    updateFromFolderNode(folderNode);

}

QVariantList Feeds::findNode(const QVariantList &folders, const QString &path)
{
    QString currentFindDir = path;
    currentFindDir.remove(QRegExp("^/"));
    currentFindDir = currentFindDir.split('/').first();

    QString nextFindDir = path;
    nextFindDir.remove(QRegExp("^/"));
    if (nextFindDir.contains('/')) {
        nextFindDir.remove(QRegExp(".*/"));
    } else {
        nextFindDir.clear();
    }

    qDebug() << "searching for dir" << path << currentFindDir << nextFindDir;
    foreach (const QVariant &folderListEntry, folders) {
        bool canConvert;
        folderListEntry.toInt(&canConvert);
        if (!canConvert) {
            QString folderEntry = folderListEntry.toMap().keys().first();
            if (folderEntry == currentFindDir) {
                qDebug() << "got folder entry" << nextFindDir;
                if (nextFindDir.isEmpty()) {
                    return folderListEntry.toMap().values().first().toList();
                }
                return findNode(folderListEntry.toMap().values().first().toList(), nextFindDir);
            }
        }
    }

    return QVariantList();
}

bool EntryCompare (const Entry& a, const Entry& b)
{
	return a.title < b.title;
}

void Feeds::updateFromFolderNode(const QVariantList &folderNode)
{
    beginResetModel();

    m_list.clear();

    QVariantMap feeds = NewsBlurConnection::instance()->feedsData().toMap().value("feeds").toMap();
    foreach (const QVariant &folderListEntry, folderNode) {
        bool canConvert;
        int feedId = folderListEntry.toInt(&canConvert);
        if (canConvert) {
            Entry entry;
            entry.id = feedId;
            entry.title = feeds.value(QString::number(feedId)).toMap().value("feed_title").toString();
            entry.isFolder = false;
			entry.unread = feeds.value(QString::number(feedId)).toMap().value("nt").toInt();
            if (entry.title != "") {
                m_list.append(entry);
                qDebug() << "Feed (" << feedId << "):" << entry.title;
            } else {
                qDebug() << "Feed (" << feedId << "): Skipping empty title";
            }
        } else {
            Entry entry;
            entry.id = feedId;
            entry.title = folderListEntry.toMap().keys().first();
            entry.isFolder = true;
			entry.unread = 0;

            if (entry.title != "") {
                m_list.append(entry);
            } else {
                qDebug() << "Feed (" << feedId << "): Skipping empty title";
            }
        }
    }

	qSort(m_list.begin(), m_list.end(), EntryCompare);

    endResetModel();
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
    }

    return QVariant();
}

QHash<int, QByteArray> Feeds::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles.insert(RoleTitle, "title");
    roles.insert(RoleIsFolder, "isFolder");
    roles.insert(RoleId, "feedId");
    roles.insert(RoleUnread, "unread");
    return roles;
}

QString Feeds::filterPath() const
{
    return m_filterPath;
}

void Feeds::setFilterPath(const QString &filterPath)
{
    if (m_filterPath != filterPath) {
        m_filterPath = filterPath;
        emit filterPathChanged();
        refresh();
    }
}
