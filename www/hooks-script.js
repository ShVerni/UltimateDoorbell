document.addEventListener("DOMContentLoaded", () => {
    getSettings();
    document.getElementById("add-hook").onclick = addHook;
    document.getElementById("update").onclick = update;
});

function getSettings() {
    let xhr = new XMLHttpRequest();
    xhr.responseType = 'json';
    xhr.open('GET', '/webhookSettings');
    xhr.onload = function () {
        if (this.status != 200) {
            document.getElementById('message').innerHTML = 'ERROR!';
        } else {
            let response = xhr.response;
            console.log(response);
            if (response != null) {
                if (response.enable) {
                    document.getElementById("enable").checked = true;
                } else {
                    document.getElementById("enable").checked = false;
                }
                if (response.webhooks.length > 0) {
                    let list = document.getElementById("hook-list");
                    list.innerHTML = "";
                    for (let i = 0; i < response.webhooks.length; i++) {
                        let first = true;
                        let params = "";
                        for (let [key, value] of Object.entries(response.webhooks[i].parameters)) {
                            if (first) {
                                params += key + ":" + value;
                                first = false;
                            } else {
                                params += "," + key + ":" + value;
                            }
                        }
                        list.innerHTML += `
                        <tr class="file" data-url="` + response.webhooks[i].url + `" data-params="` + params + `" data-method="` + response.webhooks[i].method + `">
                            <td>` + response.webhooks[i].url + `</td>
                            <td>` + params + `</td>
                            <td>` + (response.webhooks[i].method === 0 ? "GET" : "POST") + `</td>
                            <td class="delete" onclick="deleteHook(this)">Delete</td>
                        </tr>`;
                    }
                }
            }
        }
    };
    xhr.send();
}

// Add webhook
function addHook() {
    document.getElementById("hook-list").innerHTML += `
    <tr class="file" data-url="` + document.getElementById("url").value + `" data-params="` + document.getElementById("parameters").value + `" data-method="` + document.getElementById("method").value + `">
        <td>` + document.getElementById("url").value + `</td>
        <td>` + document.getElementById("parameters").value + `</td>
        <td>` + (document.getElementById("method").value === "0" ? "GET" : "POST") + `</td>
        <td class="delete" onclick="deleteHook(this)">Delete</td>
    </tr>`;
    sendSettings(buildSettingsString(), "Webhook added!");
    document.getElementById("webhook").reset();
}

// Delete hook
function deleteHook(element) {
    if (window.confirm("Delete this webhook?")) {
        element.parentNode.remove();
        sendSettings(buildSettingsString(), "Hook deleted");
    }
}

// Updates settings on the server
function update() {
    sendSettings(buildSettingsString(), "Settings updated!")
}

// Build a JSON settings string from the current settings
function buildSettingsString() {
    let settings = {
        enable: document.getElementById("enable").checked,
        webhooks: []
    };

    let selected = document.querySelectorAll('.file');
    if (selected.length > 0) {
        for (let i = 0; i < selected.length; i++) {
            let webhook = {
                url: selected[i].getAttribute('data-url'),
                parameters: {},
                method: selected[i].getAttribute('data-method')
            }
            const params = selected[i].getAttribute('data-params').split(",");
            if (params.length > 0) {
                for (let j = 0; j < params.length; j++) {
                    webhook.parameters[params[j].substring(0, params[j].indexOf(':'))] = params[j].substring(params[j].indexOf(':') + 1);
                }
            }
            settings.webhooks.push(webhook);
        }
    }
    return JSON.stringify(settings);
}

// Send webhook setting to the server
function sendSettings(settings, success) {
    let xhr = new XMLHttpRequest(), data = new FormData();
    data.append('settings', settings);
    xhr.open('POST', '/webhookSettings');
    xhr.onload = function () {
        if (this.status != 200) {
            document.getElementById('message').innerHTML = 'ERROR!';
        } else {
            document.getElementById('message').innerHTML = success;
        }
    };
    xhr.send(data);  
}