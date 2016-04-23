import QtQuick 2.4
import Ubuntu.Components 1.3
import Ubuntu.Components.ListItems 1.3
import NewsBlur 0.1
import Ubuntu.Web 0.2

Page {
	id: story
	
	property string storyTitle
	property string storyContent
	property string storyLink

	header: PageHeader {
		title: webview.canGoBack ? webview.title : storyTitle
		leadingActionBar {
			actions: [
				Action {
					id: webback
					text: "Back"
					iconName: "go-previous"
					onTriggered: {
						if (webview.canGoBack) {
							webview.goBack();
						} else {
							story.pageStack.removePages(story);
						}
					}
					visible: (webview.canGoBack && pagelayout.columns > 1) || pagelayout.columns == 1
				}
			]
		}
		trailingActionBar {
			actions: [
				Action {
					id: openExternally
					text: "Open"
					iconName: "external-link"
					onTriggered: Qt.openUrlExternally(storyLink)
				}
			]
		}
	}

	WebView {
		id: webview

		anchors.top: story.header.bottom
		anchors.bottom: story.bottom
		anchors.right: story.right
		anchors.left: story.left

		Component.onCompleted: {
			if (story.storyContent != "") {
				loadHtml(story.storyContent)
			} else {
				webview.url = storyLink
			}
		}

		incognito: true

		contextualActions: ActionList {
			Action {
				text: i18n.tr("Open link in browser")
				enabled: webview.contextModel && webview.contextModel.linkUrl.toString()
				onTriggered: Qt.openUrlExternally(webview.contextModel.linkUrl)
			}
		}
	}
}
