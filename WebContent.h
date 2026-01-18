#ifndef WEB_CONTENT_H
#define WEB_CONTENT_H

#include <Arduino.h>

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="ru">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>LC200 Smart Watch - Управление</title>
    <link rel="stylesheet" href="style.css">
    <link href="https://fonts.googleapis.com/css2?family=Outfit:wght@200;400;700&display=swap" rel="stylesheet">
</head>
<body>
    <div class="background-glow"></div>
    <div class="container">
        <header>
            <h1>LC200 Контроль</h1>
            <div class="status-indicator online" id="connectionStatus"></div>
        </header>

        <main>
            <nav class="tabs-nav">
                <button class="tab-btn active" onclick="switchTab('main')">Главное</button>
                <button class="tab-btn" onclick="switchTab('settings')">Настройки</button>
                <button class="tab-btn" onclick="switchTab('effects')">Подсветка</button>
            </nav>

            <div id="tab-main" class="tab-content active">
                <!-- Dashboard Card -->
                <section class="glass-card">
                    <h2>Панель состояния</h2>
                    <div class="info-grid">
                        <div class="info-item">
                            <span class="label">Время</span>
                            <span class="value" id="timeValue">--:--</span>
                        </div>
                        <div class="info-item">
                            <span class="label">Темп.</span>
                            <span class="value" id="tempValue">--&deg;C</span>
                        </div>
                        <div class="info-item">
                            <span class="label">Погода</span>
                            <span class="value" id="weatherValue">--</span>
                        </div>
                        <div class="info-item">
                            <span class="label">Город</span>
                            <span class="value" id="cityValue">--</span>
                        </div>
                    </div>
                </section>

                <!-- Alarm Settings -->
                <section class="glass-card">
                    <div style="display:flex; justify-content:space-between; align-items:center; margin-bottom:15px; border-bottom:1px solid var(--glass-border); padding-bottom:5px;">
                        <h2 style="margin:0; border:none; padding:0;">Будильник</h2>
                        <span id="savedAlarmBadge" class="label" style="background:rgba(255,255,255,0.1); padding:4px 8px; border-radius:12px; font-weight:700;">Выкл</span>
                    </div>
                    
                    <div class="control-group">
                        <label for="alarmTime">Установка времени</label>
                        <input type="time" id="alarmTime" data-dirty="false">
                    </div>
                    
                    <div class="info-grid" style="margin-bottom:15px;">
                        <div class="control-group" style="margin:0;">
                            <label>Звук</label>
                            <select id="alarmSound" data-dirty="false">
                                <option value="1">Звук 1</option>
                                <option value="2">Звук 2</option>
                                <option value="3">Звук 3</option>
                                <option value="4">Звук 4</option>
                                <option value="5">Звук 5</option>
                                <option value="6">Звук 6</option>
                                <option value="7">Звук 7</option>
                            </select>
                        </div>
                        <div class="control-group" style="margin:0;">
                            <label>Громкость</label>
                            <input type="range" id="alarmVolumeRange" min="0" max="30" value="20" style="padding:5px;" data-dirty="false">
                        </div>
                    </div>

                    <div class="control-group">
                        <label>Эффект фар при срабатывании</label>
                        <select id="alarmCarEff" data-dirty="false">
                            <option value="0">Выкл</option>
                            <option value="1">Моргание фар</option>
                            <option value="2">Аварийка</option>
                            <option value="3">Полиция</option>
                        </select>
                    </div>

                    <div class="control-group">
                        <label>Эффект подсветки при срабатывании</label>
                        <select id="alarmLedEff" data-dirty="false">
                            <option value="0">Выкл (Оранжевый)</option>
                            <option value="1">Радуга</option>
                            <option value="4">Полиция</option>
                            <option value="5">Стробоскоп</option>
                            <option value="2">Статичный цвет</option>
                        </select>
                    </div>

                    <div class="button-group">
                        <button class="btn btn-primary" id="saveAlarmBtn">Сохранить</button>
                        <button class="btn btn-secondary" id="testSoundBtn">Тест</button>
                        <button class="btn btn-danger" id="stopSoundBtn" style="flex:0.5;">Стоп</button>
                    </div>
                </section>

            </div>

            <div id="tab-settings" class="tab-content">
                <!-- Localization Settings -->
                <section class="glass-card">
                    <h2>Локализация</h2>
                    <div class="control-group">
                        <label>Часовой пояс (UTC)</label>
                        <input type="number" id="timeZone" placeholder="напр. 5 для GMT+5" value="5">
                    </div>
                    <div class="control-group">
                        <label>Город для погоды</label>
                        <select id="weatherCity">
                            <option value="Almaty">Алматы</option>
                            <option value="Astana">Астана</option>
                            <option value="Shymkent">Шымкент</option>
                            <option value="Aktobe">Актобе</option>
                            <option value="Karaganda">Караганда</option>
                            <option value="Taraz">Тараз</option>
                            <option value="Pavlodar">Павлодар</option>
                            <option value="Ust-Kamenogorsk">Усть-Каменогорск</option>
                            <option value="Semey">Семей</option>
                            <option value="Aktau">Актау</option>
                            <option value="Kostanay">Костанай</option>
                            <option value="Kyzylorda">Кызылорда</option>
                            <option value="Atyrau">Атырау</option>
                            <option value="Petropavl">Петропавловск</option>
                            <option value="Rudny">Рудный</option>
                        </select>
                    </div>
                    <button class="btn btn-primary" id="saveLocBtn">Сохранить локализацию</button>
                </section>

                <!-- Wi-Fi Settings -->
                <section class="glass-card" id="wifiSection">
                    <h2>Wi-Fi</h2>
                    <div id="wifiConnectForm">
                        <div class="control-group">
                            <label>SSID</label>
                            <input type="text" id="wifiSsid" placeholder="Имя сети">
                        </div>
                        <div class="control-group">
                            <label>Пароль</label>
                            <input type="password" id="wifiPass" placeholder="Пароль">
                        </div>
                        <button class="btn btn-primary" id="saveWifiBtn">Сохранить и перезагрузить</button>
                    </div>
                    <div id="wifiConnectedInfo" style="display:none;">
                        <p style="margin-bottom:15px; opacity:0.8;">Устройство подключено.</p>
                        <button class="btn btn-secondary" id="resetWifiBtn">Сбросить Wi-Fi</button>
                    </div>
                </section>

                <section class="glass-card">
                    <h2>Система</h2>
                    <div class="control-group">
                        <label>Яркость экрана</label>
                        <input type="range" id="dispBrightRange" min="0" max="255" value="255">
                    </div>
                    <button class="btn btn-danger" id="rebootBtn">Перезагрузить систему</button>
                </section>
            </div>

            <div id="tab-effects" class="tab-content">
                <!-- Underglow Lights -->
                </section>

                <!-- Vehicle Configuration (Moved) -->
                <section class="glass-card">
                    <h2>Конфигурация фар</h2>
                    <div class="control-group">
                        <label>Управление светом</label>
                        <div class="button-group">
                            <button class="btn btn-secondary" id="frontLightsBtn">Фары выкл</button>
                            <button class="btn btn-secondary" id="rearLightsBtn">Задние выкл</button>
                        </div>
                    </div>
                    <div class="control-group">
                        <label>Поворотники</label>
                        <select id="turnSignal">
                            <option value="0">Выкл</option>
                            <option value="1">Влево</option>
                            <option value="2">Вправо</option>
                            <option value="3">Аварийка</option>
                        </select>
                    </div>
                </section>
            </div>
        </main>
    </div>
    <script src="script.js"></script>
</body>
</html>
)rawliteral";

const char style_css[] PROGMEM = R"rawliteral(
:root {
    --glass-bg: rgba(255, 255, 255, 0.05);
    --glass-border: rgba(255, 255, 255, 0.1);
    --primary-color: #00d2ff;
    --text-color: #ffffff;
    --bg-color: #0f0f13;
}
* { box-sizing: border-box; margin: 0; padding: 0; }
body {
    font-family: 'Outfit', sans-serif;
    background-color: var(--bg-color);
    color: var(--text-color);
    min-height: 100vh;
    display: flex;
    justify-content: center;
    padding: 20px;
    overflow-x: hidden;
}
.background-glow {
    position: fixed; top: -50%; left: -50%; width: 200%; height: 200%;
    background: radial-gradient(circle at 50% 50%, rgba(0, 210, 255, 0.1), transparent 60%);
    z-index: -1; animation: pulse 10s infinite alternate;
}
@keyframes pulse { from { transform: scale(1); } to { transform: scale(1.1); } }
.container { width: 100%; max-width: 400px; }
header { display: flex; justify-content: space-between; align-items: center; margin-bottom: 20px; }
h1 { font-weight: 200; letter-spacing: 1px; }
.status-indicator { width: 10px; height: 10px; border-radius: 50%; background-color: #ff3b3b; box-shadow: 0 0 10px #ff3b3b; }
.status-indicator.online { background-color: #00ff88; box-shadow: 0 0 10px #00ff88; }
.glass-card {
    background: var(--glass-bg); backdrop-filter: blur(10px); -webkit-backdrop-filter: blur(10px);
    border: 1px solid var(--glass-border); border-radius: 16px; padding: 20px; margin-bottom: 20px;
    box-shadow: 0 8px 32px 0 rgba(0, 0, 0, 0.37);
}
h2 { font-size: 1.2rem; margin-bottom: 15px; border-bottom: 1px solid var(--glass-border); padding-bottom: 5px; color: var(--primary-color); }
.info-grid { display: grid; grid-template-columns: repeat(2, 1fr); gap: 15px; }
.info-item { display: flex; flex-direction: column; }
.label { font-size: 0.8rem; opacity: 0.7; }
.value { font-size: 1.2rem; font-weight: 700; }
.control-group { margin-bottom: 15px; }
label { display: block; margin-bottom: 5px; font-size: 0.9rem; }
input[type="time"], select, input[type="range"], input[type="text"], input[type="password"], input[type="number"] {
    width: 100%; background: rgba(0, 0, 0, 0.3); border: 1px solid var(--glass-border);
    color: white; padding: 10px; border-radius: 8px; font-family: inherit; outline: none;
}
.button-group { display: flex; gap: 10px; }
.btn { flex: 1; padding: 10px; border: none; border-radius: 8px; cursor: pointer; font-weight: 700; transition: transform 0.1s; }
.btn:active { transform: scale(0.95); }
.btn-primary { background: var(--primary-color); color: #000; }
.btn-secondary { background: transparent; border: 1px solid var(--primary-color); color: var(--primary-color); }
.btn-danger { background: rgba(255, 59, 59, 0.2); color: #ff3b3b; border: 1px solid #ff3b3b; width: 100%; }

/* Tab Navigation Styles */
.tabs-nav {
    display: flex;
    gap: 10px;
    margin-bottom: 20px;
    background: var(--glass-bg);
    padding: 5px;
    border-radius: 12px;
    border: 1px solid var(--glass-border);
}
.tab-btn {
    flex: 1;
    padding: 10px;
    border: none;
    background: transparent;
    color: var(--text-color);
    cursor: pointer;
    font-size: 0.9rem;
    font-weight: 400;
    border-radius: 8px;
    transition: all 0.3s;
    opacity: 0.6;
}
.tab-btn.active {
    background: var(--primary-color);
    color: #000;
    opacity: 1;
    font-weight: 700;
}
.tab-content {
    display: none;
}
.tab-content.active {
    display: block;
    animation: fadeIn 0.3s ease-in-out;
}
@keyframes fadeIn {
    from { opacity: 0; transform: translateY(10px); }
    to { opacity: 1; transform: translateY(0); }
}

)rawliteral";

const char script_js[] PROGMEM = R"rawliteral(
document.addEventListener('DOMContentLoaded', () => {
    // Restore active tab
    const activeTab = localStorage.getItem('activeTab') || 'main';
    switchTab(activeTab);

    // Track dirty state for alarm fields to prevent periodic sync from overwriting user input
    ['alarmTime', 'alarmSound', 'alarmCarEff', 'alarmLedEff', 'alarmVolumeRange'].forEach(id => {
        const el = document.getElementById(id);
        if (el) {
            const markDirty = () => { el.dataset.dirty = 'true'; };
            el.addEventListener('input', markDirty);
            el.addEventListener('change', markDirty);
        }
    });

    fetchStatus();
    setInterval(fetchStatus, 5000);
    document.getElementById('saveAlarmBtn').addEventListener('click', saveAlarm);
    document.getElementById('saveWifiBtn').addEventListener('click', saveWifi);
    document.getElementById('resetWifiBtn').addEventListener('click', resetWifi);
    document.getElementById('saveLocBtn').addEventListener('click', saveLoc);
    document.getElementById('testSoundBtn').addEventListener('click', testSound);
    document.getElementById('stopSoundBtn').addEventListener('click', stopSound);
    document.getElementById('rebootBtn').addEventListener('click', rebootSystem);
    document.getElementById('ledEffect').addEventListener('change', (e) => { setLedEffect(e.target.value); });
    document.getElementById('ledColor').addEventListener('change', (e) => { setLedColor(e.target.value); });
    document.getElementById('ledBrightRange').addEventListener('change', (e) => { setLedBrightness(e.target.value); });
    document.getElementById('ledSpeedRange').addEventListener('change', (e) => { setLedSpeed(e.target.value); });
    document.getElementById('dispBrightRange').addEventListener('change', (e) => { setDisplayBrightness(e.target.value); });
    document.getElementById('turnSignal').addEventListener('change', (e) => { setTurnSignal(e.target.value); });
    document.getElementById('frontLightsBtn').addEventListener('click', toggleFront);
    document.getElementById('rearLightsBtn').addEventListener('click', toggleRear);
});

function updateField(id, newVal) {
    const el = document.getElementById(id);
    if (!el) return;
    if (document.activeElement === el) return;
    if (el.dataset.dirty === 'true') return;
    if (newVal !== undefined && newVal !== null) {
        el.value = newVal;
    }
}

function resetDirty(ids) {
    ids.forEach(id => {
        const el = document.getElementById(id);
        if (el) el.dataset.dirty = 'false';
    });
}

function switchTab(tabId) {
    document.querySelectorAll('.tab-content').forEach(el => el.classList.remove('active'));
    document.querySelectorAll('.tab-btn').forEach(el => el.classList.remove('active'));
    
    document.getElementById('tab-' + tabId).classList.add('active');
    document.querySelector(`[onclick="switchTab('${tabId}')"]`).classList.add('active');
    
    localStorage.setItem('activeTab', tabId);
}

let frontOn = false;
let rearOn = false;

function toggleFront() {
    frontOn = !frontOn;
    updateVisuals();
    fetch('/api/settings/car_light?front=' + (frontOn ? 1 : 0));
}

function toggleRear() {
    rearOn = !rearOn;
    updateVisuals();
    fetch('/api/settings/car_light?rear=' + (rearOn ? 1 : 0));
}

function updateVisuals() {
    const btnF = document.getElementById('frontLightsBtn');
    const btnR = document.getElementById('rearLightsBtn');
    
    btnF.textContent = frontOn ? 'Фары вкл' : 'Фары выкл';
    btnF.className = frontOn ? 'btn btn-primary' : 'btn btn-secondary';
    
    btnR.textContent = rearOn ? 'Задние вкл' : 'Задние выкл';
    btnR.className = rearOn ? 'btn btn-primary' : 'btn btn-secondary';
}

async function fetchStatus() {
    try {
        const response = await fetch('/api/status');
        if (!response.ok) throw new Error('Ошибка сети');
        const data = await response.json();
        document.getElementById('timeValue').textContent = data.time;
        document.getElementById('tempValue').textContent = data.temp + '\u00B0C';
        document.getElementById('weatherValue').textContent = translateCondition(data.condition);
        document.getElementById('cityValue').textContent = translateCity(data.city);
        document.getElementById('connectionStatus').className = 'status-indicator online';
        
        // Update Alarm Badge and Time
        const badge = document.getElementById('savedAlarmBadge');
        if (data.alarm_enabled) {
            badge.textContent = data.alarm_time;
            badge.style.background = 'var(--primary-color)';
            badge.style.color = '#000';
        } else {
            badge.textContent = 'Выкл';
            badge.style.background = 'rgba(255,255,255,0.1)';
            badge.style.color = 'inherit';
        }

        // Update inputs if not focused and not modified by user
        updateField('alarmTime', data.alarm_time);
        updateField('alarmSound', data.alarm_sound);
        updateField('alarmCarEff', data.alarm_car_eff);
        updateField('alarmLedEff', data.alarm_led_eff);
        updateField('alarmVolumeRange', data.alarm_volume);
        
        updateField('timeZone', data.tz);
        updateField('weatherCity', data.city);

        // WiFi Status UI
        const wifiForm = document.getElementById('wifiConnectForm');
        const wifiInfo = document.getElementById('wifiConnectedInfo');
        if (data.is_ap) {
            wifiForm.style.display = 'block';
            wifiInfo.style.display = 'none';
        } else {
            wifiForm.style.display = 'none';
            wifiInfo.style.display = 'block';
        }

    } catch (error) {
        document.getElementById('connectionStatus').className = 'status-indicator';
    }
}
function translateCondition(c) {
    const t = {'Clear':'Ясно','Clouds':'Облачно','Rain':'Дождь','Snow':'Снег','Thunderstorm':'Гроза','Drizzle':'Морось','Mist':'Туман','Fog':'Туман','Haze':'Дымка'};
    return t[c] || c;
}
function translateCity(c) {
    const t = {
        'Almaty':'Алматы','Astana':'Астана','Shymkent':'Шымкент','Aktobe':'Актобе',
        'Karaganda':'Караганда','Taraz':'Тараз','Pavlodar':'Павлодар',
        'Ust-Kamenogorsk':'Усть-Каменогорск','Semey':'Семей','Aktau':'Актау',
        'Kostanay':'Костанай','Kyzylorda':'Кызылорда','Atyrau':'Атырау',
        'Petropavl':'Петропавловск','Rudny':'Рудный'
    };
    return t[c] || c;
}
async function saveAlarm() {
    const time = document.getElementById('alarmTime').value;
    const sound = document.getElementById('alarmSound').value;
    const carEff = document.getElementById('alarmCarEff').value;
    const ledEff = document.getElementById('alarmLedEff').value;
    const volume = document.getElementById('alarmVolumeRange').value;
    if (!time) return alert('Пожалуйста, выберите время');
    try {
        await fetch('/api/alarm/set', { 
            method: 'POST', 
            headers: { 'Content-Type': 'application/json' }, 
            body: JSON.stringify({ 
                time: time, 
                sound: Number(sound),
                carEff: Number(carEff),
                ledEff: Number(ledEff),
                volume: Number(volume),
                enabled: true
            }) 
        });
        alert('Будильник сохранен!');
        resetDirty(['alarmTime', 'alarmSound', 'alarmCarEff', 'alarmLedEff', 'alarmVolumeRange']);
        fetchStatus();
    } catch (e) { alert('Ошибка при сохранении будильника'); }
}
async function saveWifi() {
    const ssid = document.getElementById('wifiSsid').value;
    const pass = document.getElementById('wifiPass').value;
    if (!ssid) return alert('Нужно имя сети (SSID)');
    if (confirm('Сохранить WiFi и перезагрузить?')) {
        try {
            await fetch('/api/settings/wifi', { method: 'POST', headers: { 'Content-Type': 'application/json' }, body: JSON.stringify({ ssid: ssid, pass: pass }) });
            alert('Настройки сохранены. Перезагрузка...');
            setTimeout(() => { location.reload(); }, 5000);
        } catch (e) { alert('Ошибка при сохранении WiFi'); }
    }
}
async function saveLoc() {
    const tz = document.getElementById('timeZone').value;
    const city = document.getElementById('weatherCity').value;
    try {
        await fetch('/api/settings/loc', { method: 'POST', headers: { 'Content-Type': 'application/json' }, body: JSON.stringify({ timezone: tz, city: city }) });
        alert('Локализация сохранена!');
        fetchStatus(); // Refresh
    } catch (e) { alert('Ошибка при сохранении настроек'); }
}
async function resetWifi() {
    if (confirm('Очистить настройки WiFi и перейти в режим точки доступа?')) {
        try {
            await fetch('/api/settings/wifi_reset');
            alert('Настройки сброшены. Устройство перезагружается...');
            setTimeout(() => { location.reload(); }, 5000);
        } catch (e) { alert('Ошибка при сбросе WiFi'); }
    }
}
async function testSound() {
    const sound = document.getElementById('alarmSound').value;
    fetch('/api/sound/test?id=' + sound);
}
async function stopSound() {
    fetch('/api/sound/stop');
}
async function setAlarmVolume(val) {
    fetch('/api/settings/alarm_volume?val=' + val);
}
async function setLedBrightness(val) {
    fetch('/api/settings/led_bright?val=' + val);
}
async function setDisplayBrightness(val) {
    fetch('/api/settings/disp_bright?val=' + val);
}
async function setLedSpeed(val) {
    fetch('/api/settings/led_speed?val=' + val);
}
async function setLedEffect(val) {
    fetch('/api/settings/led?eff=' + val);
}
async function setLedColor(val) {
    fetch('/api/settings/led_color?hex=' + encodeURIComponent(val));
}
async function setTurnSignal(val) {
    fetch('/api/settings/turn_signal?mode=' + val);
}
async function rebootSystem() {
    if (confirm('Перезагрузить систему?')) { fetch('/api/system/reboot'); }
}
)rawliteral";

#endif
