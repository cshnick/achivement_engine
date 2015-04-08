function loadAchievements(user, project) {
    //Request xml data on load
    var request = new XMLHttpRequest()
    request.open('POST', 'http://127.0.0.1:5555/AchievementListGet')
    request.setRequestHeader('content-type', 'text/xml;charset=utf-8')

    request.onreadystatechange = function () {
        if (request.readyState === XMLHttpRequest.DONE) {
            if (request.status === 200) {
                console.log("Get Request answer")
                xml_model.fromXml(request.responseText)
            } else {
                console.log("HTTP request failed", request.status)
            }
        }
    }
    request.send("user" + top_level.par_delimiter + user
                 + top_level.req_delimiter +
                 "project" + top_level.par_delimiter + project
                 )
}

function registerRemoved(dict) {
    if (dict[f_id]) {
        top_level.deletedElements.push(dict)
    }
}

function updateModel(ind) {
    if (ind === -1) return
    var dict = {}
    if (top_level.idLabel != "") {
        dict[f_id] = parseInt(top_level.idLabel)
    }
    dict[f_name] = block_name.text
    dict[f_description] = block_description.text
    dict[f_condition] = text_condition.text
    dict[f_type] = ach_type.checked

    xml_model.update(ind, dict)
}

function save_achievemets() {
    var str = xml_model.toXml()
    console.log("xml_model.toXml():\n" + str)
    var request = new XMLHttpRequest()
    request.open('POST', 'http://127.0.0.1:5555/AchievementListSend')
    request.setRequestHeader('Content-Type', 'text/xml;charset=utf-8')

    request.onreadystatechange = function () {
        if (request.readyState === XMLHttpRequest.DONE) {
            if (request.status === 200) {

                console.log("Reply from server: " + request.responseText)

                top_panel.header.reportHttp200("Сохранено...")
//                var ind = lview.currentIndex
//                xml_model.fromXml(request.responseText)
//                lview.currentIndex = ind
            } else {
                console.log("HTTP request failed", request.status)
                top_panel.header.reportHttpError(request.responseText)
            }
        }
    }

    var obj = top_level.deletedElements
    request.send(                            "content" + top_level.par_delimiter + str
                 + top_level.req_delimiter + "user"    + top_level.par_delimiter + top_level.user
                 + top_level.req_delimiter + "project" + top_level.par_delimiter + top_level.project
                 + top_level.req_delimiter + "removed" + top_level.par_delimiter + JSON.stringify(obj))
}

function empty(obj) {
    return Object.getOwnPropertyNames(obj).length === 0
}

var AE_TYPE_INSTANT = 0
var AE_TYPE_SESSION_WIDE = 1


