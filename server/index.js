//---------- Server express ----------------------
const express = require('express');
const app = express();
// statci files
app.use(express.static(__dirname + '/public'));
// start server
const server = app.listen(3000, () => {
    console.log('server on port 3000')
})

// ------------ Serial Port ----------------------
const SerialPort = require('serialport');
const Readline = SerialPort.parsers.Readline;
const port = new SerialPort('COM4', {   // Es necesario colocar el puerto donde esta el Arduino, 
    baudRate: 9600
}, (err) => {                           // Cerrar IDE de arduino para no tener problemas (consola)
    if (err) {
        return console.log('Error on write: ', err.message)
    }
    console.log('Connection correct')
});
const parser = port.pipe(new Readline())
// -----------------------------------------------

// ------------ websockets ----------------------
const SocketIO = require('socket.io');
const io = SocketIO(server);
// -----------------------------------------------

// ------------ fs -------------------------------
const fs = require('fs');
const logger = fs.createWriteStream('data.csv', {
    flags: 'a'
});
// -----------------------------------------------

io.on('connection', (socket) => {
    console.log("websocket on");
    // Se reciben los datos del frontEnd en el backEnd
    socket.on('controlData', (data) => {
        port.write(data);   // Se ecriben los datos en el puerto serie 
        logger.write(Date() + ',' + data + '\n') // Se guardan los datos en el archivo csv
        io.emit('controlData', data);
    });
});

// Se reciben los datos en del puerto serie en el backEnd y se envian al frontEnd
parser.on('data', (data) => {
    io.emit('controlData', data);
    logger.write(Date() + ',' + data + '\n') // Se guardan los datos en el archivo csv
});

