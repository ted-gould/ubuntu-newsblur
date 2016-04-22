import QtQuick 2.4
import Ubuntu.Components 1.3
import Ubuntu.Components.Popups 1.3
import NewsBlur 0.1
import U1db 1.0 as U1db

Page {
    id: root

    property int feedId
	property bool itemsInit: false
	property string title: "NewsBlur"

	header: PageHeader {
		id: header
		title: root.title

		trailingActionBar {
			actions: [
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
					onTriggered: PopupUtils.open(clearDialog)
				}
			]
		}

		flickable: listview
	}

	Component {
		id: clearDialog
		Dialog {
			id: clearDialogInstance
			title: "Mark Feed Read?"

			Button {
				text: "Mark Read"
				onClicked: {
					PopupUtils.close(clearDialogInstance)
					stories.markFeedRead()
				}
			}
			Button {
				text: "Cancel"
				onClicked: PopupUtils.close(clearDialogInstance)
			}
		}
	}

	U1db.Document {
		id: childOpened
		database: settingsDatabase
		docId: 'child-opened-' + header.title.replace(/\W/g, '-')
		create: true
		defaults: {
			"childSelected": "None"
		}
	}

	Connections {
		target: root.pageStack
		onCurrentPageChanged: {
			if (itemsInit && root.pageStack.currentPage == root) {
				console.log("Clearing saved feed '" + childOpened.contents["childSelected"] + "' on '" + header.title + "'")
				settingsDatabase.putDoc({"childSelected": "None"}, childOpened.docId)
			}
		}
	}

    UbuntuListView {
		id: listview

        anchors.fill: parent

        model: Stories {
			id: stories
            feedId: root.feedId
        }

        delegate: ListItem {
			id: storyitem

			ListItemLayout {
				title.text: storytitle
				title.elide: Text.ElideRight
				title.font.bold: !read

				UbuntuShape {
					id: ushape
					height: storyitem.height * 0.75
					width: storyitem.height * 0.75

					visible: false

					SlotsLayout.position: SlotsLayout.Trailing
					source: Image {
						id: image
						source: imageurl
						onStatusChanged: if (image.status == Image.Ready) ushape.visible = (imageurl != "")
					}
				}

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
