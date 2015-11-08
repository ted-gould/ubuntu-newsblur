#ifndef ENTRIES_H
#define ENTRIES_H

#include <QAbstractListModel>
#include <QQmlParserStatus>
#include <QDate>


class StoryEntry {
public:
    QString title;
    QString hash;
    QString content;
    QString link;
    QString timestamp;
	bool read;
};

class Stories : public QAbstractListModel, public QQmlParserStatus
{
    Q_OBJECT

    Q_PROPERTY(int feedId READ feedId WRITE setFeedId NOTIFY feedIdChanged)

public:
    enum Roles {
        RoleTitle,
        RoleHash,
        RoleContent,
        RoleLink,
        RoleTimestamp,
		RoleRead
    };

    explicit Stories(QObject *parent = 0);

    int rowCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QHash<int, QByteArray> roleNames() const;

    int feedId() const;
    void setFeedId(int feedId);

protected:
    void componentComplete();
    void classBegin() {}

signals:
    void feedIdChanged();
	void pageUpdateStarted() const;

private slots:
    void refresh();
    void entriesFetched(const QVariant &entriesData);
	void pageUpdateStart();

private:
    QVariantList findNode(const QVariantList &folders, const QString &path);
    void updateFromFolderNode(const QVariantList &folderNode);

private:
    QList<StoryEntry> m_list;
    int m_feedId;
	int m_page;
	bool m_storiesAvailable;
	bool m_requestInProgress;
};

#endif // ENTRIES_H
