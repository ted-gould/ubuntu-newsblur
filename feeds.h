#ifndef FEEDS_H
#define FEEDS_H

#include <QAbstractListModel>
#include <QQmlParserStatus>

#include "newsblurconnection.h"

class Entry {
public:
    int id;
    QString title;
    bool isFolder;
	int unread;
};

class Feeds : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(QString folderName READ folderName WRITE setFolderName NOTIFY folderNameChanged)

public:
    enum Roles {
        RoleId,
        RoleTitle,
        RoleIsFolder,
		RoleUnread,
		RoleUnreadStr
    };

    explicit Feeds(QObject *parent = 0);

    int rowCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QHash<int, QByteArray> roleNames() const;

    QString folderName () const;
    void folderNamePath(const QString &folderName);
	void setFolderName(const QString &folderName);

protected:
    void componentComplete();
    void classBegin() {}

signals:
    void folderNameChanged();

private slots:
    void refresh();
    void feedsUpdated();
    void storyRead(int feedId, const QString &hash);
    void feedReset(int feedId);


private:
    QList<Entry> m_list;
    QString m_folderName;

	static bool entryCompare (const Entry& a, const Entry& b);
	void entriesFromFolders();
	void entriesFromFolder(const NewsBlurConnection::Folder &folder);
};

#endif // FEEDS_H
