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
        "Abraham Linksys",
        "Benjamin FrankLAN",
        "Martin Router King",
        "LAN Solo",
        "House LANister",
        "Winternet Is Coming",
        "The Promised LAN"
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
        "Credit Card Collector",
        "Keylogger Active",
        "Trojan Downloader",
        "Ransomware Node 7",
        "Dark Web Access Point"
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
        "Doge Network Much Secure",
        "Never Gonna Give You Up",
        "This Is Fine Network",
        "Surprised Pikachu WiFi",
        "One Does Not Simply WiFi"
    ],
    pop_culture: [
        "Hogwarts WiFi",
        "Stark Industries Guest",
        "Wayne Manor Network",
        "Batcave Secure",
        "The Upside Down WiFi",
        "Wakanda Forever Net",
        "Avengers Tower Guest",
        "Daily Planet WiFi",
        "Los Pollos Hermanos",
        "Dunder Mifflin WiFi",
        "Paddy's Pub Free WiFi",
        "Central Perk Network",
        "Pawnee Parks Dept",
        "Schrute Farms B&B"
    ],
    trolling: [
        "Connecting...",
        "Searching...",
        "Free WiFi (Slow)",
        "WiFi Password: password",
        "Shout PASSWORD for WiFi",
        "Click Here for Free WiFi",
        "This Network Is Cursed",
        "You Shall Not Pass",
        "Lag Generator 3000",
        "Buffering Forever",
        "1 Bar of Signal",
        "Disconnecting Soon",
        "Try Another Network",
        "Not The WiFi You Want"
    ]
};

// Random name generators
const randomWords = {
    adjectives: ['Free', 'Fast', 'Secure', 'Premium', 'Super', 'Ultra', 'Mega', 'Turbo', 'Pro', 'VIP', 'Best', 'New'],
    nouns: ['WiFi', 'Internet', 'Network', 'Hotspot', 'Connection', 'Access', 'Link', 'Signal'],
    places: ['Cafe', 'Hotel', 'Airport', 'Library', 'Office', 'Guest', 'Public', 'Home', 'Shop', 'Mall'],
    suffixes: ['5G', 'Plus', 'Max', 'Lite', 'Go', 'Now', '2.4GHz', '5GHz', 'Fast', 'Secure']
};

function generateRandomSSIDs() {
    const ssids = [];
    for (let i = 0; i < 25; i++) {
        const type = Math.floor(Math.random() * 4);
        let ssid;
        if (type === 0) {
            ssid = randomWords.adjectives[Math.floor(Math.random() * randomWords.adjectives.length)] + ' ' +
                randomWords.nouns[Math.floor(Math.random() * randomWords.nouns.length)];
        } else if (type === 1) {
            ssid = randomWords.places[Math.floor(Math.random() * randomWords.places.length)] + ' ' +
                randomWords.nouns[Math.floor(Math.random() * randomWords.nouns.length)];
        } else if (type === 2) {
            ssid = randomWords.nouns[Math.floor(Math.random() * randomWords.nouns.length)] + ' ' +
                randomWords.suffixes[Math.floor(Math.random() * randomWords.suffixes.length)];
        } else {
            ssid = randomWords.adjectives[Math.floor(Math.random() * randomWords.adjectives.length)] + ' ' +
                randomWords.places[Math.floor(Math.random() * randomWords.places.length)] + ' WiFi';
        }
        if (Math.random() > 0.6) ssid += ' ' + Math.floor(Math.random() * 100);
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

// Apply saved theme
function applySavedTheme() {
    const saved = localStorage.getItem('theme');
    if (saved === 'light') {
        document.body.classList.add('light-mode');
        document.getElementById('themeBtn').innerText = '🌙 Dark Mode';
    }
}

function toggleRedirectInputs() {
    const isDisabled = document.getElementById('disableButton').checked;
    const urlInput = document.getElementById('redirectUrl');
    const delayInput = document.getElementById('autoRedirectDelay');

    urlInput.disabled = isDisabled;
    delayInput.disabled = isDisabled;

    // Optional: Add visual opacity if standard disabled isn't enough
    urlInput.style.opacity = isDisabled ? '0.5' : '1';
    delayInput.style.opacity = isDisabled ? '0.5' : '1';
}

function loadData() {
    fetch('/api/data')
        .then(res => res.json())
        .then(data => {
            document.getElementById('ssidCount').innerText = data.ssidCount;
            document.getElementById('uptime').innerText = data.uptime;
            if (data.freeHeap) {
                document.getElementById('freeHeap').innerText = Math.round(data.freeHeap / 1024) + 'KB';
            }
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
            document.getElementById('disableButton').checked = data.disableButton || false;
            document.getElementById('autoRedirectDelay').value = data.autoRedirectDelay || 0;
            document.getElementById('apName').value = data.apName || 'Free WiFi';
            document.getElementById('hideAP').checked = data.hideAP || false;
            // Advanced settings
            document.getElementById('wifiChannel').value = data.wifiChannel || 1;
            document.getElementById('randomizeMAC').checked = data.randomizeMAC || false;
            document.getElementById('useCustomPortal').checked = data.useCustomPortal || false;
            // Credentials
            document.getElementById('adminUser').value = data.adminUser || 'admin';
            document.getElementById('adminPass').value = '';

            // Set initial state of inputs
            toggleRedirectInputs();
        })
        .catch(err => console.error('Error loading data:', err));

    refreshClients();
    loadPortalHTML();
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
        disableButton: document.getElementById('disableButton').checked,
        autoRedirectDelay: parseInt(document.getElementById('autoRedirectDelay').value),
        apName: document.getElementById('apName').value,
        hideAP: document.getElementById('hideAP').checked,
        wifiChannel: parseInt(document.getElementById('wifiChannel').value),
        randomizeMAC: document.getElementById('randomizeMAC').checked,
        useCustomPortal: document.getElementById('useCustomPortal').checked
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

// ===== Portal HTML Editor =====

function loadPortalHTML() {
    fetch('/api/portal_html')
        .then(res => res.text())
        .then(html => {
            const el = document.getElementById('portalHtml');
            el.value = html;
            updatePreview(html);
        })
        .catch(err => console.error('Error loading portal HTML:', err));
}

function updatePreview(html) {
    const frame = document.getElementById('previewFrame');
    // We use srcdoc for immediate preview without saving
    frame.srcdoc = html || '<h3>Preview Area</h3><p>Type above to see changes...</p>';
}

// Live Preview Listener
document.getElementById('portalHtml').addEventListener('input', function (e) {
    updatePreview(e.target.value);
});

// Disable Button Listener
document.getElementById('disableButton').addEventListener('change', toggleRedirectInputs);

function loadDefaultPortalHTML() {
    // Provide a sample snippet for the new "Current Content" model
    const sample =
        '<div class="logo">⚡</div>\n' +
        '<h1>Your Custom Ad Here</h1>\n' +
        '<p class="desc">This is a sample layout. You can add images, text, or anything else here!</p>\n' +
        '<div class="f">\n' +
        '    <div><span>🔥</span> Great Deal 1</div>\n' +
        '    <div><span>⭐</span> Great Deal 2</div>\n' +
        '</div>';
    document.getElementById('portalHtml').value = sample;
}

function savePortalHTML() {
    const html = document.getElementById('portalHtml').value;

    if (html.length > 8192) {
        alert('HTML is too large! Max 8KB.');
        return;
    }

    const formData = new FormData();
    formData.append('html', html);

    fetch('/api/save_portal_html', { method: 'POST', body: formData })
        .then(res => res.text())
        .then(txt => alert(txt))
        .catch(err => alert('Error saving portal HTML: ' + err));
}

function resetPortalHTML() {
    if (!confirm('Reset to default portal HTML? Your custom HTML will be deleted.')) {
        return;
    }

    fetch('/api/reset_portal_html', { method: 'POST' })
        .then(res => res.text())
        .then(txt => {
            alert(txt);
            loadPortalHTML();
        })
        .catch(err => alert('Error resetting portal HTML: ' + err));
}
