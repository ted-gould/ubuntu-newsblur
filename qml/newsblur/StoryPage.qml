import QtQuick 2.4
import Ubuntu.Components 1.3
import Ubuntu.Components.ListItems 1.3
import Ubuntu.Components.Popups 1.3
import NewsBlur 0.1
import Ubuntu.Web 0.2

Page {
	id: story
	
	property int feedId
	property string storyTitle
	property string storyContent
	property string storyLink
	property string storyHash

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
				// Far right
				Action {
					id: openExternally
					text: "Open in Browser"
					iconName: "external-link"
					onTriggered: Qt.openUrlExternally(storyLink)
				},
				Action {
					id: openInternally
					text: "Open Here"
					iconName: "down"
					onTriggered: webview.url = storyLink
				},
				Action {
					id: shareNewsblur
					text: "Share on Newsblur"
					iconName: "share" // TODO: We need a different icon
					onTriggered: PopupUtils.open(shareDialog)
				}
			]
		}
	}

	Component {
		id: shareDialog
		Dialog {
			id: shareDialogInstance
			title: "Share Story"

			TextArea {
				id: shareComment
				placeholderText: "Comment"
			}
			Button {
				text: "Share"
				onClicked: {
					PopupUtils.close(shareDialogInstance)
					NewsBlur.shareStory(story.feedId, story.storyLink, shareComment.text)
				}
			}
			Button {
				text: "Cancel"
				onClicked: PopupUtils.close(shareDialogInstance)
			}
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
