import QtQuick 2.4
import Ubuntu.Components 1.3
import Ubuntu.Components.Popups 1.3
import NewsBlur 0.1
import U1db 1.0 as U1db

Page {
    id: root

    property string folderPath
	property bool itemsInit: false
	property string  queuedPushPath: ""
	property variant queuedPushProps
	property bool pageComplete: false
	property string folderTitle: "NewsBlur"

	header: PageHeader {
		id: header
		title: root.folderTitle
		flickable: listview

		trailingActionBar {
			actions: [
				Action {
					id: showLoginDialog
					visible: header.title == "NewsBlur"
					text: "Login"
					iconName: "account"
					onTriggered: PopupUtils.open(loginDialog)
				}
			]

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

	Component {
		id: loginDialog
		Dialog {
			id: loginDialogInstance
			title: "Login"

			TextField {
				id: usernameField
				placeholderText: "Username"
			}
			TextField {
				id: passwordField
				placeholderText: "Password"
				echoMode: TextInput.Password
			}
			Button {
				text: "Save"
				onClicked: {
					settingsDatabase.putDoc({"username": usernameField.text, "password": passwordField.text}, 'login-info')
					NewsBlur.login(usernameField.text, passwordField.text)
					PopupUtils.close(loginDialogInstance)
				}
			}
			Button {
				text: "Cancel"
				onClicked: PopupUtils.close(loginDialogInstance)
			}
		}
	}

	Connections {
		target: root.pageStack
		onPrimaryPageChanged: {
			if (itemsInit && root.pageStack.currentPage == root) {
				console.log("Clearing saved feed '" + childOpened.contents["childSelected"] + "' on '" + header.title + "'")
				settingsDatabase.putDoc({"childSelected": "None"}, childOpened.docId)
			}
		}
	}

    UbuntuListView {
		id: listview

        anchors.fill: parent
        Feeds {
			id: feedModel
            filterPath: folderPath
        }

		model: SortFilterModel {
			id: sortedFeedModel
			model: feedModel
			sort.property: "feedtitle"
			sortCaseSensitivity: Qt. CaseInsensitive
			filter.property: "unreadstr"
			filter.pattern: /someunread/
		}

        delegate: ListItem {
			ListItemLayout {
				title.text: feedtitle
				title.elide: Text.ElideRight

				ProgressionSlot {}
			}

            onClicked: {
				settingsDatabase.putDoc({"childSelected": feedtitle}, childOpened.docId)

                if (isFolder) {
                    root.pageStack.addPageToCurrentColumn(root, Qt.resolvedUrl("FoldersListPage.qml"), {
						folderPath: root.folderPath + '/' + feedtitle,
						folderTitle: feedtitle
					})
                } else {
                    console.log("Clicking on feed list '" + feedId + "': " + feedtitle);
                    root.pageStack.addPageToCurrentColumn(root, Qt.resolvedUrl("FeedListPage.qml"), {feedId: feedId, feedTitle: feedtitle})
                }
            }
			
			Component.onCompleted: {
				itemsInit = true
				if (childOpened.contents["childSelected"] == feedtitle) {
					console.log("Setting up queued push")
					if (isFolder) {
						queuedPushPath = Qt.resolvedUrl("FoldersListPage.qml")
						queuedPushProps = {
							folderPath: root.folderPath + '/' + feedtitle,
							folderTitle: feedtitle
						}
					} else {
						queuedPushPath = Qt.resolvedUrl("FeedListPage.qml")
						queuedPushProps = {feedId: feedId, feedTitle: feedtitle}
					}

					if (pageComplete) {
						root.pageStack.addPageToCurrentColumn(root, queuedPushPath, queuedPushProps)
					}
				}
			}
        }
    }

	Timer {
		id: queueTimer
		interval: 100
		onTriggered: {
			root.pageStack.addPageToCurrentColumn(root, queuedPushPath, queuedPushProps)
		}
	}

	Component.onCompleted: {
		console.log("Resolving up queued push")
		if (queuedPushPath != "") {
			queueTimer.start()
		}
		pageComplete = true
	}
}
