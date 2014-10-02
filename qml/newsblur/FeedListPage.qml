import QtQuick 2.0
import Ubuntu.Components 0.1
import Ubuntu.Components.ListItems 0.1
import NewsBlur 0.1

Page {
    id: root
    title: "NewsBlur"

    property int feedId

    ListView {
        anchors.fill: parent

        model: Stories {
            feedId: root.feedId
        }

        delegate: Subtitled {
            text: title
            progression: true

            onClicked: {
                 pageStack.push(Qt.resolvedUrl("StoryPage.qml"), {storyTitle: title, storyLink: link, storyContent: content});
            }
        }
    }

}
