import QtQuick 2.4
import Ubuntu.Components 1.3
import Ubuntu.Components.ListItems 1.3
import NewsBlur 0.1
import U1db 1.0 as U1db

Page {
    id: root
    title: "NewsBlur"

    property string folderPath

	U1db.Document {
		id: childOpened
		database: settingsDatabase
		docId: 'child-opened-' + title
		create: true
		defaults: {
			"childSelected": "None"
		}
	}

    ListView {
        anchors.fill: parent
        model: Feeds {
			id: feedModel
            filterPath: folderPath

        }

        delegate: Standard {
            text: title
            progression: true

            onClicked: {
				settingsDatabase.putDoc({"childSelected": title}, 'child-opened-' + root.title)

                if (isFolder) {
                    pageStack.push(Qt.resolvedUrl("FoldersListPage.qml"), {
						folderPath: root.folderPath + '/' + title,
						title: title
					})
                } else {
                    console.log("Clicking on feed list '" + feedId + "': " + title);
                    pageStack.push(Qt.resolvedUrl("FeedListPage.qml"), {feedId: feedId, title: title})
                }
            }
			
			Component.onCompleted: {
				if (childOpened.contents["childSelected"] == title) {
					if (isFolder) {
						pageStack.push(Qt.resolvedUrl("FoldersListPage.qml"), {
							folderPath: root.folderPath + '/' + title,
							title: title
						})
					} else {
						console.log("Clicking on feed list '" + feedId + "': " + title);
						pageStack.push(Qt.resolvedUrl("FeedListPage.qml"), {feedId: feedId, title: title})
					}
				}
			}
        }
    }

}
