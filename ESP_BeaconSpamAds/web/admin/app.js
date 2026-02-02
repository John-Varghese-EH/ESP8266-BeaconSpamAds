// Admin Panel JavaScript

// SSID Preset Packs
const presets = {
    funny: [
        "FBI Surveillance Van #42",
        "Definitely Not a Virus",
        "Pretty Fly for a WiFi",
        "Mom Use This One",
        "No Free WiFi Here",
        "Get Your Own WiFi",
        "Nacho WiFi",
        "The LAN Before Time",
        "It Burns When IP",
        "Drop It Like Its Hotspot",
        "Silence of the LANs",
        "LAN of Milk and Honey",
        "John Wilkes Bluetooth",
        "Abraham Linksys"
    ],
    scary: [
        "Virus Distribution Center",
        "FBI Surveillance Van",
        "NSA Listening Post",
        "POLICE_STAKEOUT_VAN",
        "DEA Surveillance #4128",
        "CIA Black Site",
        "Hack Me If You Can",
        "MALWARE_INJECTION_POINT",
        "Your Data Is Ours",
        "We Can See You",
        "Identity Theft Network",
        "Credit Card Collector"
    ],
    memes: [
        "Hide Yo Kids Hide Yo WiFi",
        "Yell POTATO for Password",
        "Loading...",
        "Error 404 WiFi Not Found",
        "WiFi Name Here",
        "Tell My WiFi Love Her",
        "Bill Wi the Science Fi",
        "Martin Router King Jr",
        "The Password Is Password",
        "8Hz WAN IP",
        "Ermahgerd Werfer",
        "Doge Network Much Secure"
    ]
};

// Random name generators
const randomWords = {
    adjectives: ['Free', 'Fast', 'Secure', 'Premium', 'Super', 'Ultra', 'Mega', 'Turbo', 'Pro', 'VIP'],
    nouns: ['WiFi', 'Internet', 'Network', 'Hotspot', 'Connection', 'Access', 'Link'],
    places: ['Cafe', 'Hotel', 'Airport', 'Library', 'Office', 'Guest', 'Public', 'Home'],
    suffixes: ['5G', 'Plus', 'Max', 'Lite', 'Go', 'Now', '2.4GHz', '5GHz']
};

function generateRandomSSIDs() {
    const ssids = [];
    for (let i = 0; i < 20; i++) {
        const type = Math.floor(Math.random() * 3);
        let ssid;
        if (type === 0) {
            ssid = randomWords.adjectives[Math.floor(Math.random() * randomWords.adjectives.length)] + ' ' +
                randomWords.nouns[Math.floor(Math.random() * randomWords.nouns.length)];
        } else if (type === 1) {
            ssid = randomWords.places[Math.floor(Math.random() * randomWords.places.length)] + ' ' +
                randomWords.nouns[Math.floor(Math.random() * randomWords.nouns.length)];
        } else {
            ssid = randomWords.nouns[Math.floor(Math.random() * randomWords.nouns.length)] + ' ' +
                randomWords.suffixes[Math.floor(Math.random() * randomWords.suffixes.length)];
        }
        if (Math.random() > 0.5) ssid += ' ' + Math.floor(Math.random() * 100);
        ssids.push(ssid);
    }
    document.getElementById('ssidList').value = ssids.join('\n');
}

function loadPreset(name) {
    if (presets[name]) {
        document.getElementById('ssidList').value = presets[name].join('\n');
    }
}

// Theme toggle
function toggleTheme() {
    const body = document.body;
    const isDark = body.classList.toggle('light-mode');
    localStorage.setItem('theme', isDark ? 'light' : 'dark');
    document.getElementById('themeBtn').innerText = isDark ? '🌙 Dark Mode' : '☀️ Light Mode';
}

// Apply saved theme
function applySavedTheme() {
    const saved = localStorage.getItem('theme');
    if (saved === 'light') {
        document.body.classList.add('light-mode');
        document.getElementById('themeBtn').innerText = '🌙 Dark Mode';
    }
}

function loadData() {
    fetch('/api/data')
        .then(res => res.json())
        .then(data => {
            document.getElementById('ssidCount').innerText = data.ssidCount;
            document.getElementById('uptime').innerText = data.uptime;
            document.getElementById('clientCount').innerText = data.clientCount;
            document.getElementById('wpa2').checked = data.wpa2;
            document.getElementById('appendSpaces').checked = data.appendSpaces;
            document.getElementById('enableBLE').checked = data.enableBLE;
            document.getElementById('beaconInterval').value = data.beaconInterval;
            document.getElementById('ssidList').value = data.ssids;
            // Portal settings
            document.getElementById('redirectUrl').value = data.redirectUrl || '';
            document.getElementById('advertisingHeadline').value = data.advertisingHeadline || '';
            document.getElementById('advertisingDescription').value = data.advertisingDescription || '';
            document.getElementById('buttonText').value = data.buttonText || 'Connect';
            document.getElementById('autoRedirectDelay').value = data.autoRedirectDelay || 0;
            document.getElementById('apName').value = data.apName || 'Free WiFi';
            document.getElementById('hideAP').checked = data.hideAP || false;
            // Advanced settings
            document.getElementById('wifiChannel').value = data.wifiChannel || 1;
            document.getElementById('randomizeMAC').checked = data.randomizeMAC || false;
            // Credentials
            document.getElementById('adminUser').value = data.adminUser || 'admin';
            document.getElementById('adminPass').value = '';
        })
        .catch(err => console.error('Error loading data:', err));

    refreshClients();
}

function refreshClients() {
    fetch('/api/clients')
        .then(res => res.json())
        .then(data => {
            const el = document.getElementById('clientsList');
            if (data.clients && data.clients.length > 0) {
                let html = '<table style="width:100%;border-collapse:collapse;">';
                html += '<tr><th style="text-align:left;">MAC Address</th><th style="text-align:left;">IP Address</th></tr>';
                data.clients.forEach(c => {
                    html += `<tr><td>${c.mac}</td><td>${c.ip}</td></tr>`;
                });
                html += '</table>';
                el.innerHTML = html;
            } else {
                el.innerHTML = '<p style="color:var(--text-secondary);">No clients connected</p>';
            }
            document.getElementById('clientCount').innerText = data.total || 0;
        })
        .catch(err => {
            document.getElementById('clientsList').innerHTML = '<p style="color:#f85149;">Error loading clients</p>';
        });
}

function saveConfig() {
    const data = {
        wpa2: document.getElementById('wpa2').checked,
        appendSpaces: document.getElementById('appendSpaces').checked,
        enableBLE: document.getElementById('enableBLE').checked,
        beaconInterval: parseInt(document.getElementById('beaconInterval').value),
        redirectUrl: document.getElementById('redirectUrl').value,
        advertisingHeadline: document.getElementById('advertisingHeadline').value,
        advertisingDescription: document.getElementById('advertisingDescription').value,
        buttonText: document.getElementById('buttonText').value,
        autoRedirectDelay: parseInt(document.getElementById('autoRedirectDelay').value),
        apName: document.getElementById('apName').value,
        hideAP: document.getElementById('hideAP').checked,
        wifiChannel: parseInt(document.getElementById('wifiChannel').value),
        randomizeMAC: document.getElementById('randomizeMAC').checked
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

function saveCredentials() {
    const user = document.getElementById('adminUser').value;
    const pass = document.getElementById('adminPass').value;

    if (!user || !pass) {
        alert('Please enter both username and password');
        return;
    }

    const data = { adminUser: user, adminPass: pass };
    const formData = new FormData();
    formData.append('data', JSON.stringify(data));

    fetch('/api/save_config', { method: 'POST', body: formData })
        .then(res => res.text())
        .then(txt => {
            alert('Credentials updated! Please log in again.');
            window.location.href = '/admin';
        })
        .catch(err => alert('Error saving credentials: ' + err));
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

function exportConfig() {
    window.location.href = '/api/export';
}

function importConfig(event) {
    const file = event.target.files[0];
    if (!file) return;

    const reader = new FileReader();
    reader.onload = function (e) {
        const formData = new FormData();
        formData.append('data', e.target.result);

        fetch('/api/import', { method: 'POST', body: formData })
            .then(res => res.text())
            .then(txt => {
                alert(txt);
                loadData();
            })
            .catch(err => alert('Error importing config: ' + err));
    };
    reader.readAsText(file);
    event.target.value = ''; // Reset file input
}

function reboot() {
    if (confirm('Reboot the device?')) {
        fetch('/api/reboot', { method: 'POST' });
        alert('Rebooting... Please wait a few seconds.');
    }
}

// Initialize
document.addEventListener('DOMContentLoaded', function () {
    applySavedTheme();
    loadData();
});
