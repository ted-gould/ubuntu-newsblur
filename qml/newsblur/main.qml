import QtQuick 2.4
import Ubuntu.Components 1.3
import Ubuntu.Components.ListItems 1.3
import Ubuntu.Components.Popups 1.3
import NewsBlur 0.1
import U1db 1.0 as U1db

MainView {
    width: units.gu(40)
    height: units.gu(70)

    automaticOrientation: true
    applicationName: "cx.gould.ted.newsblur"

	U1db.Database {
		id: settingsDatabase
		path: "config-database"
	}

	U1db.Document {
		id: loginInfo
		database: settingsDatabase
		docId: 'login-info'
		create: true
		defaults: {
			"username": "testuser",
			"password": ""
		}
	}

	Connections {
		target: NewsBlur
		onErrorChanged: {
			console.log("NewBlur Error: " + NewsBlur.error)
			PopupUtils.open(errorDialog)
		}
	}

	Component {
		id: errorDialog
		Dialog {
			id: errorDialogInstance
			title: "Error"

			Text {
				text: NewsBlur.error
			}

			Button {
				text: "Close"
				onClicked: PopupUtils.close(errorDialogInstance)
			}
		}
	}

    Component.onCompleted: {
        NewsBlur.login(loginInfo.contents["username"], loginInfo.contents["password"])
    }

    AdaptivePageLayout {
        id: pagelayout
        anchors.fill: parent

		Component.onCompleted: {
			var component = Qt.createComponent("FoldersListPage.qml")
			if (component.status == Component.Ready)
				pagelayout.primaryPage = component.createObject(pagelayout)
		}
    }
}
