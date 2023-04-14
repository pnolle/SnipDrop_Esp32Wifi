var Socket;
document
  .getElementById('BTN_SEND_BACK')
  .addEventListener('click', button_send_back);

function init() {
  Socket = new WebSocket('ws://' + window.location.hostname + ':81/');
  Socket.onmessage = function (event) {
    processCommand(event);
  };
}

function button_send_back() {
  var light_details = {
    ledNum: Math.floor(Math.random() * 500),
    r: Math.floor(Math.random() * 255),
    g: Math.floor(Math.random() * 255),
    b: Math.floor(Math.random() * 255),
  };
  Socket.send(JSON.stringify(light_details));
}

function processCommand(event) {
  var obj = JSON.parse(event.data);
  document.getElementById('rand1').innerHTML = obj.rand1;
  document.getElementById('rand2').innerHTML = obj.rand2;
  console.log(obj.rand1);
  console.log(obj.rand2);
}

window.onload = function (event) {
  init();
};
