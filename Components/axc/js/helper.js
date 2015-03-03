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

