#pragma once
#include "Arduino.h"

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="pt-br">
<head>
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Drone Comunication</title>
    <style>
        h3, h2 {
            text-align: center;
        }
        .c1 div {
            display: flex;
            align-items: center;
            margin: 0.5rem;
        }
        span {
            display: inline-block;
            width: max(5%, 40px);
            /* margin-inline: 5px; */
            text-align: center;
        }
        input {
            width: calc(100% - max(10%, 80px));
        }
    </style>
</head>
<body>
    <h2>disconnected</h2>
    <div data-num="0" class="c1">
        <h3>Pitch and Roll</h3>
        <div>
            <span>Kp</span>
            <input type="range" min="5" max="30" step="0.001" >
            <span >17.5</span>
        </div>
        <div>
            <span>Ki</span>
            <input type="range" min="0" max="5" step="0.001" >
            <span >2.5</span>
        </div>
        <div>
            <span>Kd</span>
            <input type="range" min="0" max="20" step="0.001" >
            <span >10</span>
        </div>
    </div>
    <div data-num="1" class="c1">
        <h3>Yaw</h3>
        <div>
            <span>Kp</span>
            <input type="range" min="5" max="30" step="0.001" >
            <span >17.5</span>
        </div>
        <div>
            <span>Ki</span>
            <input type="range" min="0" max="5" step="0.001" >
            <span >2.5</span>
        </div>
        <div>
            <span>Kd</span>
            <input type="range" min="0" max="20" step="0.001" >
            <span >10</span>
        </div>
    </div>
</body>
<script>
    var ws;
    const h2 = document.querySelector('h2');
    function initWebSocket() {
        console.log('connecting')
        ws = new WebSocket(`ws://${window.location.host}/ws`);
        ws.onopen    = _ => {
            h2.innerHTML = 'connected';
        };
        ws.onclose   = _ => {
            h2.innerHTML = 'disconnected';
            setTimeout(initWebSocket, 2000);
        };
        ws.onmessage = evt => {
            const a = evt.data.split(';');
            const e = document.querySelectorAll('input')[parseInt(a[0])]
            e.value = a[1];
            e.parentElement.children[2].innerHTML = a[1];
        };
    }

    aux = {Kp: 0, Ki: 1, Kd: 2};
    for (const input of document.querySelectorAll('input')) {
        input.addEventListener('change', evt => {
            const e = evt.target;
            const p1 = e.parentElement;
            const num = parseInt(p1.parentElement.getAttribute('data-num'));
            p1.children[2].innerHTML = e.value;
            ws.send(
                String(3*num + aux[p1.children[0].innerHTML]) + ';' +
                String(e.value)
            );
        })
    }

    initWebSocket();
</script>
</html>
)rawliteral";
