function ajaxGetHTML(webURL) {
    var url = webURL;
    var xmlhttp;
    try {
        xmlhttp = new ActiveXObject("Msxml2.XMLHTTP");
    } catch (e) {
        try {
            xmlhttp = new ActiveXObject("Microsoft.XMLHTTP");
        } catch (e) { }
    }

    if (!xmlhttp) xmlhttp = new XMLHttpRequest();
    xmlhttp.onreadystatechange = function () {
        if (xmlhttp.readyState == 4) {
            var s = xmlhttp.responseText;
            //s=s.replace(/</g,"&lt;");
            //s=s.replace(/>/g,"&gt;");
            document.write(s);
            xmlhttp = null;
        }

    }
    xmlhttp.open("GET", url, true);
    xmlhttp.send(null);
}