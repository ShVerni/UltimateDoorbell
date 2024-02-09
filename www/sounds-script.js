var vol_slider;
var vol_display;
document.addEventListener("DOMContentLoaded", () => {
    getFileList();
    getSettings();
    document.getElementById("update").onclick = updateSettings;
    vol_slider = document.getElementById("volume");
    vol_display = document.getElementById("volume-val");
    vol_display .innerHTML = vol_slider.value;
    vol_slider.oninput = function () {
        vol_display.innerHTML = this.value;
    }
});

function getSettings() {
    let xhr = new XMLHttpRequest();
    xhr.responseType = 'json';
    xhr.open('GET', '/audioSettings');
    xhr.onload = function () {
        if (this.status != 200) {
            document.getElementById('message').innerHTML = 'ERROR!';
        } else {
            let response = xhr.response;
            console.log(response);
            if (response != null) {
                vol_slider.value = response.volume.toString();
                vol_display .innerHTML = response.volume;
                if (response.files.length > 0) {
                    for (let i = 0; i < response.files.length; i++) {
                        document.querySelector('.sound-selector[data-name="' + response.files[i] +'"]').checked = true;
                    }
                }
            }
        }
    };
    xhr.send();
}

function updateSettings() {
    let settings = {
        volume: document.getElementById("volume").value,
        files: []
    };

    let selected = document.querySelectorAll('.sound-selector:checked');
    if (selected.length > 0) {
        for (let i = 0; i < selected.length; i++) {
            settings.files.push(selected[i].getAttribute('data-name'));
        }
    }
    sendSettings(JSON.stringify(settings));
}

function sendSettings(settings) {
    let xhr = new XMLHttpRequest(), data = new FormData();
    data.append('settings', settings);
    xhr.open('POST', '/audioSettings');
    xhr.onload = function () {
        if (this.status != 200) {
            document.getElementById('message').innerHTML = 'ERROR!';
        } else {
            document.getElementById('message').innerHTML = 'Settings updated!';
        }
    };
    xhr.send(data);  
}

function playSound(path) {    
    let xhr = new XMLHttpRequest(), data = new FormData();
    data.append('sound', path);
    xhr.open('POST', '/ring');
    xhr.onload = function () {
        if (this.status != 200) {
            document.getElementById('message').innerHTML = 'ERROR!';
        } else {
            document.getElementById('message').innerHTML = 'Playing file';
        }
    };
    xhr.send(data);    
}

function getFileList() {
    let xhr = new XMLHttpRequest();
    xhr.responseType = 'json';
    xhr.open('GET', '/list?path=/chimes');
    xhr.onload = function () {
        if (this.status != 200) {
            document.getElementById('message').innerHTML = 'ERROR!';
        } else {
            let response = xhr.response;
            console.log(response);
            if (response != null) {
                let list = document.getElementById("file-list");
                for (let i = 0; i < response.files.length; i++)
                {
                    list.innerHTML += `
                        <tr class="file">
                            <td><input class="sound-selector" data-name=` + response.files[i] + ` type="checkbox"></td>
                            <td>` + response.files[i].substring(response.files[i].lastIndexOf("/") + 1) + `</td>
                            <td class="download" onclick="playSound('` + response.files[i] + `')">Play</td>
                        </tr>`;
                }
            }
        }
    };
    xhr.send();
}