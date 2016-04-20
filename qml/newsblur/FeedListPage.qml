import QtQuick 2.4
import Ubuntu.Components 1.3
import NewsBlur 0.1
import U1db 1.0 as U1db

Page {
    id: root
    title: "NewsBlur"

    property int feedId
	property bool itemsInit: false

	head.actions: [
		Action {
			id: refreshFeed
			text: "Refresh"
			iconName: "view-refresh"
			onTriggered: stories.refresh()
		},
		Action {
			id: markRead
			text: "Mark Feed Read"
			iconName: "calendar-today"
			onTriggered: stories.markFeedRead()
		}
	]

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
		target: root.pageStack
		onCurrentPageChanged: {
			if (itemsInit && root.pageStack.currentPage == root) {
				console.log("Clearing saved feed '" + childOpened.contents["childSelected"] + "' on '" + root.title + "'")
				settingsDatabase.putDoc({"childSelected": "None"}, childOpened.docId)
			}
		}
	}

    UbuntuListView {
        anchors.fill: parent

        model: Stories {
			id: stories
            feedId: root.feedId
        }

        delegate: ListItem {
			ListItemLayout {
				title.text: storytitle
				title.elide: Text.ElideRight
				title.font.bold: !read

				ProgressionSlot {}
			}

            onClicked: {
                 root.pageStack.addPageToNextColumn(root, Qt.resolvedUrl("StoryPage.qml"), {storyTitle: storytitle, storyLink: link, storyContent: content});
				 if (!read)
					 stories.markStoryHashRead(hash);
            }

			Component.onCompleted: {
				itemsInit = true
				if (childOpened.contents["childSelected"] == storytitle) {
					root.pageStack.addPageToNextColumn(root, Qt.resolvedUrl("StoryPage.qml"), {storyTitle: storytitle, storyLink: link, storyContent: content});
				}
			}
        }

		footer: Column {
			Label {
				text: "More to come"
				visible: stories.storiesAvailable
			}
			Icon {
				visible: !stories.storiesAvailable
				source: Qt.resolvedUrl('newsblur-footer.png')
			}
		}
	}
}
