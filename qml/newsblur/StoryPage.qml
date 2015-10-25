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

	title: storyTitle

	WebView {
		anchors.fill: parent
		Component.onCompleted: loadHtml(story.storyContent)
	}
}
