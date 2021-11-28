var mymap = L.map('osm-map').setView([0, 0], 1);

L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png', {
    attribution: '&copy; <a href="https://www.openstreetmap.org/copyright">OpenStreetMap</a> contributors'
}).addTo(mymap);

var layerGroup = L.layerGroup().addTo(mymap);

function addMarker(latitude, longitude) {
    layerGroup.clearLayers();
    L.marker([latitude, longitude]).addTo(layerGroup);
    mymap.setView([latitude, longitude], 15)
}