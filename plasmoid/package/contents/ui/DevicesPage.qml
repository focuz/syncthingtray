import QtQuick 2.3
import QtQuick.Layouts 1.1
import QtQml.Models 2.2
import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.extras 2.0 as PlasmaExtras

Item {
    property alias view: deviceView

    anchors.fill: parent
    objectName: "DevicesPage"

    PlasmaExtras.ScrollArea {
        anchors.fill: parent

        TopLevelView {
            id: deviceView
            model: plasmoid.nativeInterface.devModel

            delegate: TopLevelItem {
                id: item
                property alias resumePauseButton: resumePauseButton

                ColumnLayout {
                    width: parent.width
                    spacing: 0

                    RowLayout {
                        Layout.fillWidth: true

                        PlasmaCore.IconItem {
                            Layout.preferredWidth: units.iconSizes.small
                            Layout.preferredHeight: units.iconSizes.small
                            anchors.verticalCenter: parent.verticalCenter
                            source: statusIcon
                        }
                        PlasmaComponents.Label {
                            Layout.fillWidth: true
                            anchors.verticalCenter: parent.verticalCenter
                            elide: Text.ElideRight
                            text: name
                        }
                        RowLayout {
                            id: toolButtonsLayout
                            spacing: 0
                            PlasmaComponents.Label {
                                height: implicitHeight
                                text: statusString
                                color: statusColor ? statusColor : PlasmaCore.ColorScope.textColor
                                anchors.verticalCenter: parent.verticalCenter
                            }
                            Item {
                                width: 3
                            }
                            TinyButton {
                                id: resumePauseButton
                                iconSource: paused ? "media-playback-start" : "media-playback-pause"
                                tooltip: paused ? qsTr("Resume") : qsTr("Pause")
                                onClicked: {
                                    paused ? plasmoid.nativeInterface.connection.resumeDevice(
                                                 [devId]) : plasmoid.nativeInterface.connection.pauseDevice(
                                                 [devId])
                                }
                            }
                        }
                    }

                    DetailView {
                        id: detailsView
                        visible: item.expanded
                        Layout.fillWidth: true

                        model: DelegateModel {
                            model: plasmoid.nativeInterface.devModel
                            rootIndex: detailsView.model.modelIndex(index)
                            delegate: DetailItem {
                            }
                        }
                    }
                }
            }
        }
    }
}
