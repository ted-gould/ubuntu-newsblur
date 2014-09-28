#ifndef FEEDS_H
#define FEEDS_H

#include <QAbstractListModel>
#include <QQmlParserStatus>


class Entry {
public:
    int id;
    QString title;
    bool isFolder;
};

class Feeds : public QAbstractListModel, public QQmlParserStatus
{
    Q_OBJECT

    Q_PROPERTY(QString filterPath READ filterPath WRITE setFilterPath NOTIFY filterPathChanged)

public:
    enum Roles {
        RoleId,
        RoleTitle,
        RoleIsFolder
    };

    explicit Feeds(QObject *parent = 0);

    int rowCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QHash<int, QByteArray> roleNames() const;

    QString filterPath() const;
    void setFilterPath(const QString &filterPath);

protected:
    void componentComplete();
    void classBegin() {}

signals:
    void filterPathChanged();

private slots:
    void refresh();
    void feedsUpdated(const QVariant &feedsData);

private:
    QVariantList findNode(const QVariantList &folders, const QString &path);
    void updateFromFolderNode(const QVariantList &folderNode);

private:
    QList<Entry> m_list;
    QString m_filterPath;
};

#endif // FEEDS_H
