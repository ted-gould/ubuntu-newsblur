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
	QString imageurl;
};

class Stories : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(int feedId READ feedId WRITE setFeedId NOTIFY feedIdChanged)
    Q_PROPERTY(bool storiesAvailable READ storiesAvailable NOTIFY storiesAvailableChanged)
    Q_PROPERTY(bool loading READ loading NOTIFY loadingChanged)

public:
    enum Roles {
        RoleTitle,
        RoleHash,
        RoleContent,
        RoleLink,
        RoleTimestamp,
		RoleRead,
		RoleImageUrl
    };

    explicit Stories(QObject *parent = 0);

    int rowCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QHash<int, QByteArray> roleNames() const;

    int feedId() const;
    void setFeedId(int feedId);

	bool storiesAvailable() const;
	bool loading() const;

protected:
    void componentComplete();
    void classBegin() {}

signals:
    void feedIdChanged();
	void pageUpdateStarted() const;
    void storiesAvailableChanged();
    void loadingChanged();

public slots:
	void markStoryHashRead(const QString &hash);
    void refresh();
	void markFeedRead();

private slots:
    void entriesFetched(const QVariant &entriesData);
	void pageUpdateStart();
	void feedDecremented(int feedId, const QString &hash);
	void feedReset(int feedId);

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
