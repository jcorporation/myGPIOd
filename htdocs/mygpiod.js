let socket = null;

// BSN modal instances
const modalGPIOinfoInit = BSN.Modal.getInstance(document.getElementById('modalGPIOinfo'));
const modalGPIOsetInit = BSN.Modal.getInstance(document.getElementById('modalGPIOset'));
const modalGPIOblinkInit = BSN.Modal.getInstance(document.getElementById('modalGPIOblink'));

// Calculates the uri for websocket and http requests.
function getUri(proto) {
    const protocol = proto === 'ws'
        ? window.location.protocol === 'https:'
            ? 'wss:'
            : 'ws:'
        : window.location.protocol;
    return protocol + '//' + window.location.hostname +
        (window.location.port !== '' ? ':' + window.location.port : '');
}

// Sets the error message in the footer.
function setError(msg) {
    document.getElementById('lastError').textContent = msg;
}

// Connect to the websocket and set event listeners.
function socketConnect() {
    document.getElementById('websocketState').textContent = 'Connecting';
    if (socket != null) {
        socket.onclose = function() {};
        socket.close();
        socket = null;
    }
    socket = new WebSocket(getUri('ws') + '/ws');
    socket.onopen = function() {
        document.getElementById('websocketState').textContent = 'Connected';
    }
    socket.onclose = function() {
        socket = null;
        document.getElementById('websocketState').textContent = 'Disconnected';
    }
    socket.onmessage = function(msg) {
        parseWebsocketMsg(msg.data);
    }
}

// Parses the websocket message.
function parseWebsocketMsg(data) {
    let obj;
    try {
        obj = JSON.parse(data);
        document.getElementById('websocketState').textContent = 'Connected';
    }
    catch(error) {
        document.getElementById('websocketState').textContent = 'Unable to parse event';
        console.error(error);
        return;
    }
    // update GPIO list
    updateGPIOvalue(obj.gpio);
    // append event
    const tr = document.createElement('tr');
    const tdGPIO = document.createElement('td');
    tdGPIO.textContent = obj.gpio;
    const tdEvent = document.createElement('td');
    tdEvent.textContent = obj.event;
    const tdTimestamp = document.createElement('td');
    tdTimestamp.textContent = obj.ts_ms;
    tr.appendChild(tdGPIO);
    tr.appendChild(tdEvent);
    tr.appendChild(tdTimestamp);
    const eventsEl = document.getElementById('events');
    eventsEl.prepend(tr);
    // enforce event list size
    if (eventsEl.childElementCount > 10) {
        eventsEl.removeChild(eventsEl.lastChild);
    }
}

// Clears the events table.
function clearEvents() {
    document.getElementById('events').textContent = '';
}

// Gets and updates the value of a GPIO.
function updateGPIOvalue(gpio) {
    const tr = document.getElementById('gpio' + gpio);
    httpRequest('GET', '/api/gpio/' + gpio, null, function(data) {
        tr.childNodes[2].textContent = data.value;
    });
}

// Executes the refresh GPIO value action.
function refreshGPIO(event) {
    const gpio = event.target.closest('tr').data.gpio;
    updateGPIOvalue(gpio);
}

// Executes the toggle GPIO action.
function toggleGPIO(event) {
    const body = JSON.stringify({
        "action": "gpiotoggle"
    });
    const gpio = event.target.closest('tr').data.gpio;
    httpRequest('POST', '/api/gpio/' + gpio, body, null);
}

// Shows the modal for the set GPIO action.
function showModalSetGPIO(event) {
    const gpio = event.target.closest('tr').data.gpio;
    document.getElementById('modalGPIOsetGPIO').value = gpio;
    modalGPIOsetInit.show();
}

// Executes the set GPIO action.
function setGPIO() {
    const gpio = document.getElementById('modalGPIOsetGPIO').value;
    const valueEl = document.getElementById('modalGPIOsetValue');
    const value = valueEl.options[valueEl.selectedIndex].value;
    const body = JSON.stringify({
        "action": "gpioset",
        "value": value
    });
    httpRequest('POST', '/api/gpio/' + gpio, body, null);
}

// Shows the modal for the blink GPIO action.
function showModalBlinkGPIO(event) {
    const gpio = event.target.closest('tr').data.gpio;
    document.getElementById('modalGPIOblinkGPIO').value = gpio;
    modalGPIOblinkInit.show();
}

// Executes the blink GPIO action.
function blinkGPIO() {
    const gpio = document.getElementById('modalGPIOblinkGPIO').value;
    const timeout = Number(document.getElementById('modalGPIOblinkTimeout').value)
    const interval = Number(document.getElementById('modalGPIOblinkInterval').value)
    const body = JSON.stringify({
        "action": "gpioblink",
        "timeout": timeout,
        "interval": interval
    });
    httpRequest('POST', '/api/gpio/' + gpio, body, null);
}

// Gets the details of a GPIO and displays it in a modal.
function infoGPIO(event) {
    const gpioInfoEl = document.getElementById('modalGPIOinfoList');
    gpioInfoEl.textContent = '';
    const gpio = event.target.closest('tr').data.gpio;
    httpRequest('OPTIONS', '/api/gpio/' + gpio, null, function(data) {
        const keys = Object.keys(data.data);
        for (const key of keys) {
            const tr = document.createElement('tr');
            const td1 = document.createElement('td');
            td1.textContent = key;
            tr.appendChild(td1);
            const td2 = document.createElement('td');
            td2.textContent = data.data[key];
            tr.appendChild(td2);
            gpioInfoEl.appendChild(tr);
        }
        modalGPIOinfoInit.show();
    });
}

// Creates an action button for GPIOs.
function createActionLink(icon, title, callback) {
    const a = document.createElement('a');
    a.innerHTML = icon;
    a.href = '#';
    a.title = title;
    a.classList.add('me-2','btn','btn-sm','btn-secondary');
    a.addEventListener('click', function(event) {
        event.preventDefault();
        callback(event);
    }, false);
    return a;
}

// Returns the action buttons for GPIOs.
function getGPIOactions(direction) {
    const td = document.createElement('td');
    td.appendChild(createActionLink('&#x1F6C8', 'Info', infoGPIO));
    td.appendChild(createActionLink('&#x1F5D8', 'Refresh', refreshGPIO));
    if (direction === 'out') {
        td.appendChild(createActionLink('&#x25e9', 'Toggle', toggleGPIO));
        td.appendChild(createActionLink('&#x2713', 'Set', showModalSetGPIO));
        td.appendChild(createActionLink('&#x2605', 'Blink', showModalBlinkGPIO));
    }
    return td;
}

// Gets the list of GPIOs and populates the GPIO table.
function getGPIOs() {
    const gpiosEl = document.getElementById('gpios');
    gpiosEl.textContent = '';
    httpRequest('GET', '/api/gpio', null, function(data) {
        for (let i = 0; i < data.entries; i++) {
            const tr = document.createElement('tr');
            for (const k of ['gpio', 'direction', 'value']) {
                const td = document.createElement('td');
                td.textContent = data.data[i][k];
                tr.appendChild(td);
            }
            tr.data = data.data[i];
            tr.appendChild(getGPIOactions(data.data[i].direction));
            tr.setAttribute('id', 'gpio' + data.data[i].gpio);
            gpiosEl.appendChild(tr);
        }
    });
}

// Makes an http request and calls the callback function on success.
async function httpRequest(method, path, body, callback) {
    const uri = getUri('http') + path;
    let response = null;
    try {
        response = await fetch(uri, {
            method: method,
            mode: 'same-origin',
            credentials: 'same-origin',
            cache: 'no-store',
            redirect: 'follow',
            body: body
        });
    }
    catch(error) {
        setError('REST API error for ' + method + ' ' + uri);
        console.error(error);
        return;
    }
    let data = null;
    try {
        data = await response.json();
    }
    catch(error) {
        setError('Can not parse response from ' + uri);
        console.error(error);
    }
    if (data.error) {
        setError(data.error);
    }
    else {
        if (callback !== null) {
            callback(data);
        }
        setError('OK');
    }
}

// Main
socketConnect();
getGPIOs()

// Refresh button event listeners
document.getElementById('websocketReconnect').addEventListener('click', function(event) {
    event.preventDefault();
    socketConnect();
}, false);

document.getElementById('clearEvents').addEventListener('click', function(event) {
    event.preventDefault();
    clearEvents();
}, false);

document.getElementById('gpioRefresh').addEventListener('click', function(event) {
    event.preventDefault();
    getGPIOs();
}, false);

// Add event listeners for the buttons in modals
document.getElementById('modalGPIOblinkSet').addEventListener('click', function(event) {
    blinkGPIO();
    modalGPIOblinkInit.hide();
}, false);

document.getElementById('modalGPIOsetSet').addEventListener('click', function(event) {
    setGPIO();
    modalGPIOsetInit.hide();
}, false);
