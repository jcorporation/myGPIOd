// Global variables
// BSN modal instances
const modalGPIOinfoInit = BSN.Modal.getInstance(document.getElementById('modalGPIOinfo'));
const modalGPIOsetInit = BSN.Modal.getInstance(document.getElementById('modalGPIOset'));
const modalGPIOblinkInit = BSN.Modal.getInstance(document.getElementById('modalGPIOblink'));

const icons = {
    'info': '<svg xmlns="http://www.w3.org/2000/svg" enable-background="new 0 0 24 24" height="24px" viewBox="0 0 24 24" width="24px" fill="#ffffff"><g><path d="M0,0h24v24H0V0z" fill="none"/><path d="M11,7h2v2h-2V7z M11,11h2v6h-2V11z M12,2C6.48,2,2,6.48,2,12s4.48,10,10,10s10-4.48,10-10S17.52,2,12,2z M12,20 c-4.41,0-8-3.59-8-8s3.59-8,8-8s8,3.59,8,8S16.41,20,12,20z"/></g></svg>',
    'blink': '<svg xmlns="http://www.w3.org/2000/svg" height="24px" viewBox="0 0 24 24" width="24px" fill="#ffffff"><path d="M0 0h24v24H0z" fill="none"/><path d="M3 2v12h3v9l7-12H9l4-9H3zm16 0h-2l-3.2 9h1.9l.7-2h3.2l.7 2h1.9L19 2zm-2.15 5.65L18 4l1.15 3.65h-2.3z"/></svg>',
    'set': '<svg xmlns="http://www.w3.org/2000/svg" enable-background="new 0 0 24 24" height="24px" viewBox="0 0 24 24" width="24px" fill="#ffffff"><g><path d="M0,0h24v24H0V0z" fill="none"/><path d="M19.14,12.94c0.04-0.3,0.06-0.61,0.06-0.94c0-0.32-0.02-0.64-0.07-0.94l2.03-1.58c0.18-0.14,0.23-0.41,0.12-0.61 l-1.92-3.32c-0.12-0.22-0.37-0.29-0.59-0.22l-2.39,0.96c-0.5-0.38-1.03-0.7-1.62-0.94L14.4,2.81c-0.04-0.24-0.24-0.41-0.48-0.41 h-3.84c-0.24,0-0.43,0.17-0.47,0.41L9.25,5.35C8.66,5.59,8.12,5.92,7.63,6.29L5.24,5.33c-0.22-0.08-0.47,0-0.59,0.22L2.74,8.87 C2.62,9.08,2.66,9.34,2.86,9.48l2.03,1.58C4.84,11.36,4.8,11.69,4.8,12s0.02,0.64,0.07,0.94l-2.03,1.58 c-0.18,0.14-0.23,0.41-0.12,0.61l1.92,3.32c0.12,0.22,0.37,0.29,0.59,0.22l2.39-0.96c0.5,0.38,1.03,0.7,1.62,0.94l0.36,2.54 c0.05,0.24,0.24,0.41,0.48,0.41h3.84c0.24,0,0.44-0.17,0.47-0.41l0.36-2.54c0.59-0.24,1.13-0.56,1.62-0.94l2.39,0.96 c0.22,0.08,0.47,0,0.59-0.22l1.92-3.32c0.12-0.22,0.07-0.47-0.12-0.61L19.14,12.94z M12,15.6c-1.98,0-3.6-1.62-3.6-3.6 s1.62-3.6,3.6-3.6s3.6,1.62,3.6,3.6S13.98,15.6,12,15.6z"/></g></svg>',
    'toggle': '<svg xmlns="http://www.w3.org/2000/svg" enable-background="new 0 0 24 24" height="24px" viewBox="0 0 24 24" width="24px" fill="#ffffff"><rect fill="none" height="24" width="24"/><path d="M8.5,8.62v6.76L5.12,12L8.5,8.62 M10,5l-7,7l7,7V5L10,5z M14,5v14l7-7L14,5z"/></svg>',
    'update': '<svg xmlns="http://www.w3.org/2000/svg" height="24px" viewBox="0 0 24 24" width="24px" fill="#ffffff"><path d="M0 0h24v24H0z" fill="none"/><path d="M17.65 6.35C16.2 4.9 14.21 4 12 4c-4.42 0-7.99 3.58-7.99 8s3.57 8 7.99 8c3.73 0 6.84-2.55 7.73-6h-2.08c-.82 2.33-3.04 4-5.65 4-3.31 0-6-2.69-6-6s2.69-6 6-6c1.66 0 3.14.69 4.22 1.78L13 11h7V4l-2.35 2.35z"/></svg>',
    'updating': '<svg xmlns="http://www.w3.org/2000/svg" height="24px" viewBox="0 0 24 24" width="24px" fill="#ffffff"><path d="M0 0h24v24H0z" fill="none"/><path d="M12 6v3l4-4-4-4v3c-4.42 0-8 3.58-8 8 0 1.57.46 3.03 1.24 4.26L6.7 14.8c-.45-.83-.7-1.79-.7-2.8 0-3.31 2.69-6 6-6zm6.76 1.74L17.3 9.2c.44.84.7 1.79.7 2.8 0 3.31-2.69 6-6 6v-3l-4 4 4 4v-3c4.42 0 8-3.58 8-8 0-1.57-.46-3.03-1.24-4.26z"/></svg>'
};

// Functions
// Calculates the uri for websocket and http requests.
function getUri() {
    return window.location.protocol + '//' + window.location.hostname +
        (window.location.port !== '' ? ':' + window.location.port : '');
}

// Sets the error message in the footer.
function setError(msg) {
    document.getElementById('lastError').textContent = msg;
}

// Executes the update GPIO action.
function updateGPIOstate(event) {
    const target = event.target;
    target.innerHTML = icons.updating;
    const tr = event.target.closest('tr');
    const gpio = tr.data.gpio;
    httpRequest('GET', '/api/gpio/' + gpio, function(data) {
        tr.childNodes[2].textContent = data.value;
        setTimeout(function() {
            target.innerHTML = icons.update;
        }, 200);
    });
}

// Executes the toggle GPIO action.
function toggleGPIO(event) {
    const gpio = event.target.closest('tr').data.gpio;
    httpRequest('PATCH', '/api/gpio/' + gpio + '/toggle', getGPIOs);
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
    httpRequest('PATCH', '/api/gpio/' + gpio + '/set?value=' + value, getGPIOs);
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
    const timeout = document.getElementById('modalGPIOblinkTimeout').value;
    const interval = document.getElementById('modalGPIOblinkInterval').value;
    httpRequest('PATCH', '/api/gpio/' + gpio + '/blink?timeout=' + timeout + '&interval=' + interval, getGPIOs);
}

// Gets the details of a GPIO and displays it in a modal.
function infoGPIO(event) {
    const gpioInfoEl = document.getElementById('modalGPIOinfoList');
    gpioInfoEl.textContent = '';
    const gpio = event.target.closest('tr').data.gpio;
    httpRequest('OPTIONS', '/api/gpio/' + gpio, function(data) {
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
    td.appendChild(createActionLink(icons.info, 'Info', infoGPIO));
    td.appendChild(createActionLink(icons.update, 'Update state', updateGPIOstate));
    if (direction === 'out') {
        td.appendChild(createActionLink(icons.toggle, 'Toggle value', toggleGPIO));
        td.appendChild(createActionLink(icons.set, 'Set value', showModalSetGPIO));
        td.appendChild(createActionLink(icons.blink, 'Blink value', showModalBlinkGPIO));
    }
    return td;
}

// Gets the list of GPIOs and populates the GPIO table.
function getGPIOs() {
    document.getElementById('gpioUpdateAll').innerHTML = icons.updating;
    const gpiosEl = document.getElementById('gpios');
    const rows = gpiosEl.querySelectorAll('tr');
    httpRequest('GET', '/api/gpio', function(data) {
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
            if (rows[i]) {
                rows[i].replaceWith(tr);
            }
            else {
                gpiosEl.appendChild(tr);
            }
        }
        setTimeout(function() {
            document.getElementById('gpioUpdateAll').innerHTML = icons.update;
        }, 200);
    });
}

// Makes an http request and calls the callback function on success.
async function httpRequest(method, path, callback) {
    const uri = getUri() + path;
    let response = null;
    try {
        response = await fetch(uri, {
            method: method,
            mode: 'same-origin',
            credentials: 'same-origin',
            cache: 'no-store',
            redirect: 'follow'
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

// Init
// Update all button event listener
document.getElementById('gpioUpdateAll').addEventListener('click', function(event) {
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

// Fetch GPIO table
getGPIOs()
