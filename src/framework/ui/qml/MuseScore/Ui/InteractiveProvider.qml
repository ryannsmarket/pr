/**
 * // SPDX-License-Identifier: GPL-3.0-only
 * // MuseScore-CLA-applies
 * //=============================================================================
 * //  MuseScore
 * //  Music Composition & Notation
 * //
 * //  Copyright (C) 2021 MuseScore BVBA and others
 * //
 * //  This program is free software: you can redistribute it and/or modify
 * //  it under the terms of the GNU General Public License version 3 as
 * //  published by the Free Software Foundation.
 * //
 * //  This program is distributed in the hope that it will be useful,
 * //  but WITHOUT ANY WARRANTY; without even the implied warranty of
 * //  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * //  GNU General Public License for more details.
 * //
 * //  You should have received a copy of the GNU General Public License
 * //  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * //============================================================================= **/
import QtQuick 2.7
import MuseScore.Ui 1.0

Item {

    id: root

    property var topParent: null
    property var provider: ui._interactiveProvider

    signal requestedDockPage(var uri, var params)

    anchors.fill: parent

    Rectangle {
        width: 100
        height: 100
        color: "#440000"
    }

    Connections {
        target: root.provider

        function onFireOpen(data) {

            var page = data.data()
            console.log("try open uri: " + data.value("uri") + ", page: " + JSON.stringify(page))
            if (!(page && (page.type === ContainerType.PrimaryPage || page.type === ContainerType.QmlDialog))) {
                data.setValue("ret", {errcode: 101 }) // ResolveFailed
                return;
            }

            if (page.type === ContainerType.PrimaryPage) {
                root.requestedDockPage(data.value("uri"), page.params)
                root.provider.onOpen(page.type, {})
                data.setValue("ret", {errcode: 0 })
                return;
            }

            if (page.type === ContainerType.QmlDialog) {

                var comp = Qt.createComponent("../../" + page.path);
                if (comp.status !== Component.Ready) {
                    console.log("[qml] failed create component: " + page.path + ", err: " + comp.errorString())
                    data.setValue("ret", {errcode: 102 }) // CreateFailed
                    return;
                }

                var obj = comp.createObject(root.topParent, page.params);
                obj.objectID = root.provider.objectID(obj)

                var ret = (obj.ret && obj.ret.errcode) ? obj.ret : {errcode: 0}
                data.setValue("ret", (obj.ret && obj.ret.errcode) ? obj.ret : {errcode: 0})
                data.setValue("objectID", obj.objectID)

                if (ret.errcode > 0) {
                    return;
                }

                obj.closed.connect(function() {
                    root.provider.onPopupClose(obj.objectID, obj.ret ? obj.ret : {errcode: 0})
                    obj.destroy()
                })

                root.provider.onOpen(page.type, obj.objectID)

                if (Boolean(data.value("sync")) && data.value("sync") === true) {
                    obj.exec()
                } else {
                    obj.show()
                }
            }
        }

        function onFireClose(objectID) {
            for(var i = 0; i < root.topParent.children.length; ++i) {
                if (root.topParent.children[i].objectID === objectID) {
                    root.topParent.children[i].hide()
                }
            }
        }
    }
}
