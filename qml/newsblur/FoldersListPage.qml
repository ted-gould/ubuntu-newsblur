import QtQuick 2.0
import Ubuntu.Components 0.1
import Ubuntu.Components.ListItems 0.1
import NewsBlur 0.1

Page {
    id: root
    title: "NewsBlur"

    property string folderPath

    ListView {
        anchors.fill: parent
        model: Feeds {
            filterPath: folderPath
        }

        delegate: Standard {
            text: title
            progression: true

            onClicked: {
                if (isFolder) {
                    pageStack.push(Qt.resolvedUrl("FoldersListPage.qml"), {folderPath: root.folderPath + '/' + title, title: title})
                } else {
                    pageStack.push(Qt.resolvedUrl("FeedListPage.qml"), {feedId: id, title: title})
				}
            }
        }
    }

}
