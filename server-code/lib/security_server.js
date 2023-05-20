"use-strict";

// var socketio = require('socket.io');
var {Server} = require('socket.io');
var io;

var counter = 0;

var dgram = require('dgram');
const { defaultMaxListeners } = require('events');

exports.listen = function(server) {
	//io = socketio.listen(server);
    io = new Server(server);
	io.sockets.on('connection', function(socket) {
		handleCommand(socket);
	});
};

function handleCommand(socket) {
    var errorTimer = setTimeout(function() {
        console.log("UDP TIMEOUT!")
        socket.emit('TIMEOUT',
        "ERROR: No response from beat-box application. Is it running?");
    }, 2000);

    socket.on('SecuritySystemCMD', (arg, val, val2) => {
        var PORT = 12345;
        var HOST = '127.0.0.1';

        // create packet
        var data = arg;
        if(val){ data += " " + val; }
        if(val2){ data += " " + val2; }
        console.log("created packet:" + data);
        var buffer = new Buffer.from(data);

        // if(arg == 'status'){
        //     reply = counter.toString();
        //     socket.emit(arg, reply);
        //     counter = (counter+1)%4;
        // }
        // if(arg == 'arm'){
        //     socket.emit("invalid_password");
        // }

        // Send message
        var client = dgram.createSocket("udp4");
        client.send(buffer, 0, buffer.length, PORT, HOST, function(err, bytes){
            if(err)
                throw err;
            console.log('UDP message sent to ' + HOST + ':' + PORT);
            console.log('UDP message sent ' + data);
        });

        client.on('listening', function () {
            var address = client.address();
            console.log('UDP Client: lisenting on ' + address.address + ':' + address.port);
        });

        client.on('message', function(message, remote) {
            console.log('UPD Client: message Rx' + remote.address + ':' + remote.port + ' - ' + message);
            clearTimeout(errorTimer); 

            var reply = message.toString('utf8');

            socket.emit(arg, reply);

            client.close();

        });

        client.on('UDP Client: close', function() {
            console.log("closed");

        });

        client.on("UDP Client: error", function(err) {
            console.log("error: ", err);
        });
        
    
    });

    // clearTimeout(errorTimer); 
};

