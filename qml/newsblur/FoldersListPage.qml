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

        delegate: Subtitled {
            text: title
            progression: isFolder

            onClicked: {
                if (isFolder) {
                    pageStack.push(Qt.resolvedUrl("FoldersListPage.qml"), {folderPath: root.folderPath + '/' + title})
                }
            }
        }
    }

}
