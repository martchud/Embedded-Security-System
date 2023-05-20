/*
Frontend code for displaying the stream and registering user interactions(buttons)
Base code taken from: https://opencoursehub.cs.sfu.ca/bfraser/grav-cms/ensc351/links/files/2022-student-howtos/StreamingWebcamFromBeagleBoneToNodeJSServer.pdf
*/
const socket = io();
socket.on("connect", (socket) => { //confirm connection with NodeJS server
console.log("Connected");
});
$( document ).ready(function() {
    window.setInterval(function() {getUpdatedQR("updateqr")}, 3000);
    window.setInterval(function() {removeBanner("remove")}, 3000);

    window.setInterval(function() {socket.emit('SecuritySystemCMD', "getStatus");}, 3000);

    $('#generateqr').click(function(){
		sendCommandToBackend("generateqr");
	});
    $('#saveqr').click(function(){
		sendQRToPhone("sendqr");
	});
    socket.on('updatedqr', function(result) {
        console.log("updating qr code");
        const canvas = $("#qrdisplay");
        const context = canvas[0].getContext('2d');
        var image = new Image();
		image.src = 'data:image/jpeg;base64,' + result.buffer;
        image.onload = function(){
            context.clearRect(0, 0, canvas.width, canvas.height);
            context.height = image.height;
            context.width = image.width;
            context.drawImage(image,0,0,320, 320);
            console.log("updated");
        }
	});
    socket.on('canvas', function(data) {
        const canvas = $("#videostream");
        const context = canvas[0].getContext('2d');
        const image = new Image();
        image.src = "data:image/jpeg;base64,"+data;
        image.onload = function(){
            context.height = image.height;
            context.width = image.width;
            context.drawImage(image,0,0,context.width, context.height);
    }
    });
    socket.on('deactivate', function(data) {
        $('#qr-box').show();
    });
});

function sendCommandToBackend(message) {
	socket.emit('UdpCommand', message);
    console.log("sent");
};

function getUpdatedQR(message) {
	socket.emit('UpdateQR', message);
};

function removeBanner(message) {
	$('#qr-box').hide();
};

function sendQRToPhone(message) {
	socket.emit('SendQRToPhone', message);
};
