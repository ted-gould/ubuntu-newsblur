import QtQuick 2.4
import Ubuntu.Components 1.3
import Ubuntu.Components.ListItems 1.3
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

    Component.onCompleted: {
        pageStack.push(Qt.resolvedUrl("FoldersListPage.qml"))
        NewsBlur.login(loginInfo.contents["username"], loginInfo.contents["password"])
    }

    PageStack {
        id: pageStack
        anchors.fill: parent
    }
}
