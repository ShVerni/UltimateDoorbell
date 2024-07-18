document.addEventListener("DOMContentLoaded", () => {
    document.getElementById("reboot").onclick = function () {
        let xhr = new XMLHttpRequest();
        xhr.open('PUT', '/reboot');
        xhr.onload = function () {
            if (this.status != 200) {
                document.getElementById('message').innerHTML = 'ERROR!';
            } else {
                document.getElementById('message').innerHTML = 'Success, rebooting!';
            }
        };
        xhr.send();
    };

    document.getElementById("reset").onclick = function () {
		if (confirm("Reset WiFi settings?") == true) {
			let xhr = new XMLHttpRequest();
			xhr.open('PUT', '/reset');
			xhr.onload = function () {
				if (this.response != 200) {
					document.getElementById('message').innerHTML = 'ERROR!';
				} else {
					document.getElementById('message').innerHTML = 'Success, rebooting!';
				}
			};
			xhr.send();
		}
    };
});