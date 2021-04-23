const socket = io();

const data = {
    tempActual: 45.89,
    tempIdeal: 23,
    heaterStatus: 1,
    fanStatus: 0,
    opStatus: 0
}

var vm = new Vue({
    el: "#container",
    data
});

// Reducir la temperatura ideal
function tempIdealLower() {
    data.tempIdeal -= 1;
    sendData();
}

// Aumentar la temperatura ideal
function tempIdealRise() {
    data.tempIdeal += 1;
    sendData();
}

// Cambiar el estado de operación
function changeOpStatus(){
    data.opStatus = data.opStatus == 0 ? 1 : 0;
    sendData();
}

// Prender o apagar el ventilador
function changeFanStatus(){
    data.fanStatus = data.fanStatus == 0 ? 1 : 0;
    sendData();
}

// Prender o apagar el calentador
function changeHeaterStatus(){
    data.heaterStatus = data.heaterStatus == 0 ? 1 : 0;
    sendData();
}

// Separamos los valores recibidos 
function splitString(stringToSplit) {
    let array = stringToSplit.split(",");
    data.tempActual = array[0];
    data.tempIdeal = parseFloat(array[1]);
    data.heaterStatus = array[2];
    data.fanStatus = array[3];
    data.opStatus = array[4];
}

// Se envian los datos al backEnd en cada cambio a ellos
function sendData() {
    let str = `${data.tempActual},${data.tempIdeal},${data.heaterStatus},${data.fanStatus},${data.opStatus}`;
    console.log(str);
    socket.emit('controlData', str);
}

// Se reciben los datos en el frontEnd
socket.on('controlData', (data) => {
    splitString(data);
});