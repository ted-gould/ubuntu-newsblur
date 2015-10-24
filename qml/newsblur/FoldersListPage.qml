import QtQuick 2.0
import Ubuntu.Components 0.1
import Ubuntu.Components.ListItems 0.1
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
			"childSelected": 0
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
				console.log("Current selected: " + childOpened.contents["childSelected"])
				childOpened.contents["childSelected"] = feedId

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

	Component.onCompleted: {
		if (childOpened.childSelected != 0) {
			travelid = childOpened.childSelected
			for (i = 0; i < feedModel.rowCount(); i++) {
				if (feedModel[i].feedId == travelid) {
					if (feedModel[i].isFolder) {
						pageStack.push(Qt.resolvedUrl("FoldersListPage.qml"), {
							folderPath: root.folderPath + '/' + feedModel[i].title,
							title: feedModel[i].title
						})
					} else {
						console.log("Clicking on feed list '" + feedModel[i].feedId + "': " + feedModel[i].title);
						pageStack.push(Qt.resolvedUrl("FeedListPage.qml"), {feedId: feedModel[i].feedId, title: feedModel[i].title})
					}
					break;
				}
			}
		}
	}
}
