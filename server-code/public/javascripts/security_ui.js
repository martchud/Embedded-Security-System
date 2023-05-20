"use strict";

// consts
const UPDATE_DELAY_MS = 1000;
const SecuritySystem_CMD = 'SecuritySystemCMD';


var socket = io.connect();

$(document).ready(function() {

    window.setInterval(function() {socket.emit(SecuritySystem_CMD, 'getStatus');}, UPDATE_DELAY_MS);

    $('#disarm-btn').click(function(){
        var pwd =  document.getElementById('password-text').value;
        if(pwd){
            console.log("password input = ", pwd);
            sendBeatBoxCommand({cmd:'disarm', val:pwd});
        }
    });
    $('#arm-btn').click(function(){
        var pwd =  document.getElementById('password-text').value;
        if(pwd){
            sendBeatBoxCommand({cmd:'arm', val:pwd}); 
        }
    });
    $('#shutdown-btn').click(function(){
        sendBeatBoxCommand({cmd:'stop'});
    });
    $('#password-reset-btn').click(function(){
        console.log("reset password");
        var pwd =  document.getElementById('pw1').value;
        var new_pwd =  document.getElementById('pw2').value;
        sendBeatBoxCommand({cmd:'reset', val:pwd, val2:new_pwd}); 
    });
   

    socket.on('getStatus', function(result) {
        console.log("status cmd response: ", result);
        var statusTxt = document.getElementById('status-text');
        if(statusTxt){
            if(statusTxt.val != result){
                setStatus(statusTxt, result);
            }
        }
	});

    socket.on('arm', function(result) {
        if(result == "incorrect_password"){
            $('#password-modal').modal('show')
        }else if(result == "success"){
            console.log("Sucessfully armed system!");
            sendBeatBoxCommand({cmd:'getStatus'});
        }else{
            console.log("ERROR: arm response not handled: ", result);
        }
	});

    socket.on('disarm', function(result) {
        if(result == "incorrect_password"){
            $('#password-modal').modal('show')
        }else if(result == "success"){
            console.log("Sucessfully disarmed system!");
            sendBeatBoxCommand({cmd:'getStatus'});
        }else{
            console.log("ERROR: disarm response not handled: ", result);
        }
	});

    socket.on('reset', function(result) {
        if(result == "incorrect_password"){
            $('#password-modal').modal('show')
        }else if(result == "success"){
            console.log("Sucessfully disarmed system!");
        }else{
            console.log("ERROR: reset response not handled: ", result);
        }
	});
});

function sendBeatBoxCommand(data){
    console.log("sending: ", data.cmd);
    socket.emit(SecuritySystem_CMD, data.cmd, data.val, data.val2);
}

function setStatus(statusTxt, state){
    switch(state){
        case "0":
            enbaleCMDButtons(); // enable any disabled buttons
            statusTxt.innerHTML = "Disarmed";
            statusTxt.style.color = "black";
            $('#disarm-btn').prop('disabled', true);
            break;
        case "1":
            enbaleCMDButtons(); // enable any disabled buttons
            statusTxt.innerHTML ="Armed";
            statusTxt.style.color = "green";
            $('#arm-btn').prop('disabled', true);
            break;
        case "2": // keep status the same
            break;
        case "3":
            statusTxt.innerHTML = "INTRUDER DETECTED!";
            statusTxt.style.color = "red";
            break;
        default:
            console.log("ERROR: unaught status cmd response: ", state);
            new_state = "ERR"
            break;
    }

}

function enbaleCMDButtons(){
    $('#arm-btn').prop('disabled', false);
    $('#disarm-btn').prop('disabled', false);
    $('#shutdown-btn').prop('disabled', false);
}
