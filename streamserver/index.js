/*
node.js server code:
  Receives the frames sent through a socket from capture.c and displays live stream of camera
  QR code scanning (compares against an existing QR text) for disarming the alarm
  Capatility to generate new QR code
  SMS functionality for convenience of saving a portable copy of QR to phone
Base code taken from: https://opencoursehub.cs.sfu.ca/bfraser/grav-cms/ensc351/links/files/2022-student-howtos/StreamingWebcamFromBeagleBoneToNodeJSServer.pdf
Modified by Jason for additional functionalities (QR code generation and scanner) and communication to frontend.
Additional libraries used: https://github.com/cozmo/jsQR and qrcode-reader; Twilio API
Sample QR code snippets: https://blog.logrocket.com/create-read-qr-codes-node-js/
*/

//Define and include required dependencies
const twilio = require('twilio');
const express = require('express');
const app = express();
const http = require('http');
const jsQR = require("jsqr");
const qrCodeReader = require('qrcode-reader');
const qrGenerator = require('qrcode');
const Jimp = require("jimp");
const fs = require("fs");
const server = http.createServer(app);
const { Server } = require("socket.io");
const io = new Server(server);
const startRouter = require('./routers/page.js');
const {SERVER_PORT: port = 3000} = process.env;
const child = require('child_process');
let last_time = 0;
let passcode;
app.use('/', startRouter);
var dgram = require('dgram');

io.on('connection', (socket) => {
const accountSid = 'AC57e961348725cb92e7e6688459b3388e'; 
const authToken = '63bb9d6884b11c07ee5ff4fd0a0257ac';

const client = require('twilio')(accountSid, authToken);

  //Initialize web page: send current QR img to client, update passcode
  console.log('a user connected');
  fs.readFile('qrcode/qr.png', (err, data) => {
    if (err) throw err;
    socket.emit('updatedqr', { image: true, buffer: data.toString('base64') });
  });
  fs.readFile('qrcode/qr.txt', 'utf8', (err, data) => {
    if (err) throw err;
    passcode = data;
    console.log(data);
  });

  //Send QR code to phone
  socket.on('SendQRToPhone', function(data){
    console.log(passcode);
    client.messages
    .create({
    body: 'Your new QR Code:',
    to: '+16046527233',
    mediaUrl: ['https://qrcode.tec-it.com/API/QRCode?data='+passcode],
    from: '+15063153561',
  })
  .then((message) => console.log(message.sid));
  });

  //Generate a new QR code and save to local files
  socket.on('UdpCommand', function(data){
    console.log('UdpCommand from client: ' + data);
    //Generate a new random QR code 
    let newcode = Math.floor(Math.random() * 9999).toString();
    qrGenerator.toFile('qrcode/qr.png', newcode, {
      errorCorrectionLevel: 'H'
    }, function(err) {
      if (err) throw err;   
    });
    console.log('New QR code generated');
    fs.writeFile('qrcode/qr.txt', newcode, err => {
      if (err) throw err;
      else{
        passcode = newcode;
        console.log("New pass code = " + passcode.toString());
      }
    });
  });

  //Update QR code image on the web interface
  socket.on('UpdateQR', function(data){
    //Send it to frontend and display it
    fs.readFile('qrcode/qr.png', (err, updateddata) => {
      if (err) throw err;
        //console.log('Sending new QR code to frontend');
        socket.emit('updatedqr', { image: true, buffer: updateddata.toString('base64') });
    }); 
  });


  // handles commands to the UDP server on the bbg
  socket.on('SecuritySystemCMD', (arg, val, val2) => {
    var PORT = 12345;
    var HOST = '192.168.7.2';

    // create packet
    var data = arg;
    if(val){ data += " " + val; }
    if(val2){ data += " " + val2; }
    // console.log("created packet:" + data);
    var buffer = new Buffer.from(data);


    // Send message
    var client2 = dgram.createSocket("udp4");
    client2.send(buffer, 0, buffer.length, PORT, HOST, function(err, bytes){
        if(err)
            throw err;
        // console.log('UDP message sent to ' + HOST + ':' + PORT);
        // console.log('UDP message sent ' + data);
    });

    client2.on('listening', function () {
        var address = client2.address();
        // console.log('UDP Client: lisenting on ' + address.address + ':' + address.port);
    });

    client2.on('message', function(message, remote) {
        console.log('UPD Client: message Rx' + remote.address + ':' + remote.port + ' - ' + message);

        var reply = message.toString('utf8');

        if(arg == 'getStatus'){
          if(reply == "3"){
            limitedFunction(client, "INTRUDER ALERT! Your security system has detected suspicious activity");
          }
        }

        socket.emit(arg, reply);

        client2.close();

    });

    client2.on('UDP Client: close', function() {
        console.log("closed");

    });

    client2.on("UDP Client: error", function(err) {
        console.log("error: ", err);
    });
  });

    let ffmpeg = child.spawn("ffmpeg", [
    "-re",
    "-y",
    "-i",
    "udp://192.168.7.1:1234",
    "-preset",
    "ultrafast",
    "-f",
    "mjpeg",
    "pipe:1"
    ]);

    ffmpeg.on('error', function (err) {
      console.log(err);
      throw err;
    });

    ffmpeg.on('close', function (code) {
      console.log('ffmpeg exited with code ' + code);
    });

    ffmpeg.stderr.on('data', function(data) {
    // Don't remove this
    // Child Process hangs when stderr exceed certain memory
    });

    ffmpeg.stdout.on('data', function (data) {
      var frame = Buffer.from(data).toString('base64'); //convert raw data to string
      io.sockets.emit('canvas',frame); //send data to client
      if( (Date.now()/1000) - last_time > 1 ){
          //console.log("stream alive");
          last_time = (Date.now())/1000;
      fs.writeFileSync("jee.jpg", data);
      Jimp.read("jee.jpg", function (err, image) {
          //will print the error
          if (err) {
            console.log(err)
          } 
          //Convert the image into PNG format and save
          else {
            image.write("jee.png")
          }
        })
      // Read the image file
      fs.readFile('jee.png', (err, data) => {
      if (err){
        console.log(err);
        return;
      }
    
      // Create a Jimp image object from the data
      Jimp.read(data, (err, image) => {
        if (err){
          console.log(err);
          return;
        }
    
        // Get the image as a Uint8ClampedArray
        const imageData = image.bitmap.data;
    
        // Do something with the image data
        const code = jsQR(imageData, 800, 600);

      if (code) {
        console.log("Found QR code", code);
        if(code.data == passcode){
          console.log("QR code matches. Attempting to deactivate alarm:\n");
          var PORT = 12345;
          var HOST = '192.168.7.2';
          var buffer = new Buffer("QR");
      
          var client = dgram.createSocket('udp4');
          client.send(buffer, 0, buffer.length, PORT, HOST, function(err, bytes) {
            if (err) 
              throw err;
            console.log('UDP message sent to ' + HOST +':'+ PORT);
          });
      
          client.on('listening', function () {
            var address = client.address();
            console.log('UDP Client: listening on ' + address.address + ":" + address.port);
          });
      
          // Handle an incoming update message over the UDP from the local application.
          client.on('message', function (message, remote) {
            console.log("UDP Client: message Rx" + remote.address + ':' + remote.port +' - ' + message);
          });
          socket.emit('deactivate', 'true');
        }
      }
      });
    });
      }

    });
});

server.listen({ port }, () => {
  console.log(`🚀 Server ready at http://192.168.7.1:${port}`);
});


const limitedFunction = limitFunctionCall(sendText, 1000*60*5);

function limitFunctionCall(fn, intervalTime) {
  let lastExecutionTime = 0;

  return function(...args) {
    const currentTime = Date.now();
    if (currentTime - lastExecutionTime >= intervalTime) {
      fn.apply(this, args);
      lastExecutionTime = currentTime;
    }
  };
}

function sendText(client, message) {
  console.log("SENDING TEXT!");
  console.log(message);
  client.messages
    .create({
    body: message,
    to: '+16046527233',
    from: '+15063153561',
  })
}