document.addEventListener('DOMContentLoaded', () => {
    fetchStatus();
    setInterval(fetchStatus, 5000); // Poll every 5 seconds

    document.getElementById('saveAlarmBtn').addEventListener('click', saveAlarm);
    document.getElementById('saveWifiBtn').addEventListener('click', saveWifi);
    document.getElementById('testSoundBtn').addEventListener('click', testSound);
    document.getElementById('rebootBtn').addEventListener('click', rebootSystem);
    document.getElementById('brightnessRange').addEventListener('change', (e) => {
        setBrightness(e.target.value);
    });
});

async function fetchStatus() {
    try {
        const response = await fetch('/api/status');
        if (!response.ok) throw new Error('Network response was not ok');
        const data = await response.json();

        document.getElementById('timeValue').textContent = data.time;
        document.getElementById('tempValue').textContent = data.temp + '°C';
        document.getElementById('weatherValue').textContent = data.condition;

        document.getElementById('connectionStatus').classList.add('online');
    } catch (error) {
        console.error('Error fetching status:', error);
        document.getElementById('connectionStatus').classList.remove('online');
    }
}

async function saveAlarm() {
    const time = document.getElementById('alarmTime').value;
    const sound = document.getElementById('alarmSound').value;

    if (!time) return alert('Please select a time');

    try {
        await fetch('/api/alarm/set', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ time: time, sound: sound })
        });
        alert('Alarm saved!');
    } catch (e) {
        alert('Error saving alarm');
    }
}

async function saveWifi() {
    const ssid = document.getElementById('wifiSsid').value;
    const pass = document.getElementById('wifiPass').value;

    if (!ssid) return alert('SSID Required');

    if (confirm('Save WiFi and Reboot?')) {
        try {
            await fetch('/api/settings/wifi', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ ssid: ssid, pass: pass })
            });
            alert('Saved. Rebooting...');
            setTimeout(() => { location.reload(); }, 5000);
        } catch (e) {
            alert('Error saving WiFi');
        }
    }
}

async function testSound() {
    const sound = document.getElementById('alarmSound').value;
    fetch('/api/sound/test?id=' + sound);
}

async function setBrightness(val) {
    fetch('/api/settings/brightness?val=' + val);
}

async function rebootSystem() {
    if (confirm('Reboot system?')) {
        fetch('/api/system/reboot');
    }
}
