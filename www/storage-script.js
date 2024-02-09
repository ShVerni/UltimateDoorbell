var uprog = {
    hBar: null,
    hPercent: null,
    hFile: null,
    init: () => {
        uprog.hBar = document.getElementById('up-bar');
        uprog.hPercent = document.getElementById('up-percent');
        uprog.hFile = document.getElementById('up-file');
        uprog.hFile.disabled = false;
    },
    update: (percent) => {
        percent = percent + '%';
        uprog.hBar.style.width = percent;
        uprog.hPercent.innerHTML = percent;
        if (percent == '100%') { uprog.hFile.disabled = false; }
    },
    upload: (destination) => {
        if (uprog.hFile.files.length == 0) {
            return;
        }
        let file = uprog.hFile.files[0];
        uprog.hFile.disabled = true;
        uprog.hFile.value = '';
        let xhr = new XMLHttpRequest(), data = new FormData();
        data.append('upfile', file);
        xhr.open('POST', destination);
        let percent = 0;
        xhr.upload.onloadstart = (evt) => { uprog.update(0); };
        xhr.upload.onloadend = (evt) => { uprog.update(100); };
        xhr.upload.onprogress = (evt) => {
            percent = Math.ceil((evt.loaded / evt.total) * 100);
            uprog.update(percent);
        };
        xhr.onload = function () {
            if (this.status != 202) {
                document.getElementById('message').innerHTML = 'ERROR!';
            } else {
                uprog.update(100);
                document.getElementById('message').innerHTML = 'File uploaded!';
                updateFileList();
            }
        };
        xhr.send(data);
    }
};

document.addEventListener("DOMContentLoaded", () => {
    uprog.init();

    updateFileList();

    document.getElementById("up-www").onclick = function () {
        uprog.upload('/upload-www');
    };

    document.getElementById("up-settings").onclick = function () {
        uprog.upload('/upload-settings');
    };

    document.getElementById("up-chimes").onclick = function () {
        uprog.upload('/upload-chimes');
    };
});

// Update the list of files displayed on this page
function updateFileList() {
    document.getElementById("file-list").innerHTML = "";
    //getFileList("/");
    getFileList("/chimes");
    getFileList("/settings");
    getFileList("/www")
}

// Delete file
function deleteFile(element) {
    let name = element.getAttribute('data-name');
    if (window.confirm("Delete this " + name + "?")) {
        let xhr = new XMLHttpRequest(), data = new FormData();
        data.append('path', name);
        xhr.open('POST', '/delete');
        xhr.onload = function () {
            if (this.status != 200) {
                document.getElementById('message').innerHTML = 'ERROR!';
            } else {
                document.getElementById('message').innerHTML = 'File deleted!';
                element.parentNode.remove();
            }
        };
        xhr.send(data);
    }
}

// Get list of files, add to DOM
function getFileList(path) {
    let xhr = new XMLHttpRequest();
    xhr.responseType = 'json';
    xhr.open('GET', '/list?path=' + path);
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
                        <td>` + response.files[i] + `</td>
                        <td class="download"><a href="/download?path=` + response.files[i] + `">Download</a>
                        <td class="delete" onclick="deleteFile(this)" data-name="` + response.files[i] + `">Delete</td>
                    </tr>`;
                }
            }
        }
    };
    xhr.send();
}