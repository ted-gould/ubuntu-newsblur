import QtQuick 2.0
import Ubuntu.Components 0.1
import Ubuntu.Components.ListItems 0.1
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
