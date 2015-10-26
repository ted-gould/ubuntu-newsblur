import QtQuick 2.4
import Ubuntu.Components 1.3
import Ubuntu.Components.ListItems 1.3
import Ubuntu.Components.Popups 1.3
import NewsBlur 0.1
import U1db 1.0 as U1db

Page {
    id: root
    title: "NewsBlur"

    property string folderPath
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

	head.actions: [
		Action {
			id: showLoginDialog
			visible: root.title == "NewsBlur"
			text: "Login"
			iconName: "account"
			onTriggered: PopupUtils.open(loginDialog)
		}
	]

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
        model: Feeds {
			id: feedModel
            filterPath: folderPath

        }

        delegate: Standard {
            text: title
            progression: true

            onClicked: {
				settingsDatabase.putDoc({"childSelected": title}, childOpened.docId)

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
				itemsInit = true
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
