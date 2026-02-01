function loadData() {
    fetch('/api/data')
        .then(res => res.json())
        .then(data => {
            document.getElementById('ssidCount').innerText = data.ssidCount;
            document.getElementById('uptime').innerText = data.uptime;
            document.getElementById('clientCount').innerText = data.clientCount;
            document.getElementById('wpa2').checked = data.wpa2;
            document.getElementById('appendSpaces').checked = data.appendSpaces;
            document.getElementById('beaconInterval').value = data.beaconInterval;
            document.getElementById('ssidList').value = data.ssids;
        })
        .catch(err => console.error('Error loading data:', err));
}

function saveConfig() {
    const data = {
        wpa2: document.getElementById('wpa2').checked,
        appendSpaces: document.getElementById('appendSpaces').checked,
        beaconInterval: parseInt(document.getElementById('beaconInterval').value)
    };

    const formData = new FormData();
    formData.append('data', JSON.stringify(data));

    fetch('/api/save_config', { method: 'POST', body: formData })
        .then(res => res.text())
        .then(txt => {
            alert(txt);
            loadData();
        })
        .catch(err => alert('Error saving config: ' + err));
}

function saveSSIDs() {
    const ssids = document.getElementById('ssidList').value;
    const formData = new FormData();
    formData.append('ssids', ssids);

    fetch('/api/save_ssids', { method: 'POST', body: formData })
        .then(res => res.text())
        .then(txt => {
            alert(txt);
            loadData();
        })
        .catch(err => alert('Error saving SSIDs: ' + err));
}

function reboot() {
    if (confirm('Reboot now?')) {
        fetch('/api/reboot', { method: 'POST' });
        alert('Rebooting...');
    }
}

// Initial Load
document.addEventListener('DOMContentLoaded', loadData);
