function submitMessage() {
    setTimeout(function () {
        //document.location.reload(); 
        window.location.replace("/");
    }, 10000);
};

function indexStart() {
    getIndexStatus();
    getLog();
    setInterval(function () {
        getIndexStatus();
        getLog();
    }, 10000);
};

function settingsStart() {
    getSettings();
    getSettingsStatus();
    setInterval(function () {
        getSettingsStatus();
    }, 10000);
};

function getSettings() {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {
            let json = JSON.parse(this.responseText);

            document.getElementById("hostname").value = json.hostname;
            document.getElementById("casterHost").value = json.casterHost;
            document.getElementById("casterPort").value = json.casterPort;
            document.getElementById("ntrip_sName").value = json.ntrip_sName;
            document.getElementById("rtk_mntpnt").value = json.rtk_mntpnt;
            document.getElementById("rtk_mntpnt_pw").value = json.rtk_mntpnt_pw;
            document.getElementById("connectNtrip").checked = json.connectNtrip;
            document.getElementById("ecefX").value = json.ecefX;
            document.getElementById("ecefY").value = json.ecefY;
            document.getElementById("ecefZ").value = json.ecefZ;
            //console.log("Settings:", json);
        }
    };
    xhttp.open("GET", "settings", true);
    xhttp.send();
};

function getSettingsStatus() {

    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {
            let json = JSON.parse(this.responseText);

            if (json.ntripConnected == true) {
                document.getElementById("ntripBade").innerHTML = "OK";
                document.getElementById("ntripBade").classList.remove("badge-danger");
                document.getElementById("ntripBade").classList.add("badge-info");
            } else {
                document.getElementById("ntripBade").innerHTML = "X";
                document.getElementById("ntripBade").classList.remove("badge-info");
                document.getElementById("ntripBade").classList.add("badge-danger");
            }

            if (json.gpsConnected == true) {
                document.getElementById("sivBadge").innerHTML = json.gpsSiv;
                document.getElementById("sivBadge").classList.remove("badge-danger");
                document.getElementById("sivBadge").classList.add("badge-info");
            } else {
                document.getElementById("sivBadge").innerHTML = "X";
                document.getElementById("sivBadge").classList.remove("badge-info");
                document.getElementById("sivBadge").classList.add("badge-danger");
            }

            //console.log("Status: ", json);
        }
    };
    xhttp.open("GET", "/status", true);
    xhttp.send();
}

function getIndexStatus() {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {
            let json = JSON.parse(this.responseText);

            document.getElementById("upSince").innerHTML = json.gpsInitTime;
            document.getElementById("ntripBade").innerHTML = json.ntripConnected;

            if (json.ntripConnected == true) {
                document.getElementById("ntripBade").innerHTML = "OK";
                document.getElementById("ntripBade").classList.remove("badge-danger");
                document.getElementById("ntripBade").classList.add("badge-info");

                document.getElementById("ntripCard").innerHTML = json.ntripConnectedTime;

                document.getElementById("ntripKBSent").innerHTML = json.ntripServerKBSent;

            } else {
                document.getElementById("ntripBade").innerHTML = "X";
                document.getElementById("ntripBade").classList.remove("badge-info");
                document.getElementById("ntripBade").classList.add("badge-danger");

                document.getElementById("ntripCard").innerHTML = "--";
                document.getElementById("ntripKBSent").innerHTML = "--";
            }

            if (json.gpsConnected == true) {
                document.getElementById("sivBadge").innerHTML = json.gpsSiv;
                document.getElementById("sivBadge").classList.remove("badge-danger");
                document.getElementById("sivBadge").classList.add("badge-info");

                document.getElementById("sivCard").innerHTML = json.gpsSiv;
            } else {
                document.getElementById("sivBadge").innerHTML = "X";
                document.getElementById("sivBadge").classList.remove("badge-info");
                document.getElementById("sivBadge").classList.add("badge-danger");

                document.getElementById("sivCard").innerHTML = "--";
            }

            //console.log("Status: ", json);
            addMarker(json.gpsLatitude, json.gpsLongitude)
        }
    };
    xhttp.open("GET", "/status", true);
    xhttp.send();
};

function getLog() {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {
            let json = JSON.parse(this.responseText);
            //console.log(this.responseText);

            var tbodyRef = document.getElementById('logTable').getElementsByTagName('tbody')[0];

            // Delete all rows from table
            while (tbodyRef.rows.length > 0) {
                tbodyRef.deleteRow(0);
            }

            json.log.forEach(element => {
                let logTime = element[0];
                let logText = element[1];

                // Insert a row at the end of table
                var newRow = tbodyRef.insertRow();

                // add row color depending on INF or ERR
                if (logText.includes("INF")) {
                    newRow.classList.add("table-info");
                } else if (logText.includes("ERR")) {
                    newRow.classList.add("table-danger");
                }

                // Insert a cell at the end of the row
                var timeCell = newRow.insertCell();
                var infoCell = newRow.insertCell();

                // Append a text node to the cell
                var timeText = document.createTextNode(logTime);
                var infoText = document.createTextNode(logText);

                timeCell.appendChild(timeText);
                infoCell.appendChild(infoText);
            });

        }
    };
    xhttp.open("GET", "/log", true);
    xhttp.send();
};


