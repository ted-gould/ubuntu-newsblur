import QtQuick 2.4
import Ubuntu.Components 1.3
import Ubuntu.Components.ListItems 1.3
import NewsBlur 0.1
import U1db 1.0 as U1db

Page {
    id: root
    title: "NewsBlur"

    property int feedId
	property bool itemsInit: false

	U1db.Document {
		id: childOpened
		database: settingsDatabase
		docId: 'child-opened-' + title.replace(/\W/g, '-')
		create: true
		defaults: {
			"childSelected": "None"
		}
	}

	Connections {
		target: pageStack
		onCurrentPageChanged: {
			if (itemsInit && pageStack.currentPage == root) {
				console.log("Clearing saved feed '" + childOpened.contents["childSelected"] + "' on '" + root.title + "'")
				settingsDatabase.putDoc({"childSelected": "None"}, childOpened.docId)
			}
		}
	}

    ListView {
        anchors.fill: parent

        model: Stories {
            feedId: root.feedId
        }

        delegate: Standard {
            text: title
            progression: true

            onClicked: {
                 pageStack.push(Qt.resolvedUrl("StoryPage.qml"), {storyTitle: title, storyLink: link, storyContent: content});
		         NewsBlur.markStoryHashRead(hash);
            }

			Component.onCompleted: {
				itemsInit = true
				if (childOpened.contents["childSelected"] == title) {
					pageStack.push(Qt.resolvedUrl("StoryPage.qml"), {storyTitle: title, storyLink: link, storyContent: content});
				}
			}
        }
    }
}
