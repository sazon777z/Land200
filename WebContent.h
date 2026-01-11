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
                </div>
            </section>

            <!-- Alarm Settings -->
            <section class="glass-card">
                <h2>Будильник</h2>
                <div class="control-group">
                    <label for="alarmTime">Установить время</label>
                    <input type="time" id="alarmTime">
                </div>
                <div class="control-group">
                    <label for="alarmSound">Звук</label>
                    <select id="alarmSound">
                        <option value="1">Запуск V8</option>
                        <option value="2">Двигатель V6</option>
                        <option value="3">Сигнал</option>
                        <option value="4">Бип тревоги</option>
                    </select>
                </div>
                <div class="button-group">
                    <button class="btn btn-primary" id="saveAlarmBtn">Сохранить</button>
                    <button class="btn btn-secondary" id="testSoundBtn">Тест звука</button>
                </div>
            </section>

            <!-- System Settings -->
            <section class="glass-card">
                <h2>Настройка WiFi</h2>
                <div class="control-group">
                    <label>SSID (Имя сети)</label>
                    <input type="text" id="wifiSsid" placeholder="Название вашей сети">
                </div>
                <div class="control-group">
                    <label>Пароль</label>
                    <input type="password" id="wifiPass" placeholder="Ваш пароль">
                </div>
                <button class="btn btn-primary" id="saveWifiBtn">Сохранить и перезагрузить</button>
            </section>

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
                    </select>
                </div>
                <button class="btn btn-primary" id="saveLocBtn">Сохранить настройки</button>
            </section>

            <!-- Underglow Lights -->
            <section class="glass-card">
                <h2>Подсветка днища</h2>
                <div class="control-group">
                    <label>Эффект</label>
                    <select id="ledEffect">
                        <option value="0">Выкл</option>
                        <option value="1" selected>Радуга</option>
                        <option value="2">Статичный цвет</option>
                        <option value="3">Дыхание</option>
                        <option value="4">Полиция</option>
                        <option value="5">Стробоскоп</option>
                        <option value="6">Затухание</option>
                        <option value="7">Искры</option>
                        <option value="8">Бегущий огонь</option>
                    </select>
                </div>
                <div class="control-group">
                    <label>Основной цвет</label>
                    <input type="color" id="ledColor" value="#00d2ff">
                </div>
                <div class="control-group">
                    <label>Яркость LED</label>
                    <input type="range" id="ledBrightRange" min="0" max="255" value="150">
                </div>
                <div class="control-group">
                    <label>Скорость эффектов</label>
                    <input type="range" id="ledSpeedRange" min="1" max="100" value="50">
                </div>
            </section>

            <!-- Vehicle Configuration -->
            <section class="glass-card">
                <h2>Конфигурация авто</h2>
                
                <div class="car-schematic">
                    <div id="visual-f-l" class="light-node f-left">Ф</div>
                    <div id="visual-f-r" class="light-node f-right">Ф</div>
                    <div id="visual-r-l" class="light-node r-left">З</div>
                    <div id="visual-r-r" class="light-node r-right">З</div>
                </div>

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

            <section class="glass-card">
                <h2>Дисплей</h2>
                <div class="control-group">
                    <label>Яркость экрана</label>
                    <input type="range" id="dispBrightRange" min="0" max="255" value="255">
                </div>
                <button class="btn btn-danger" id="rebootBtn">Перезагрузить систему</button>
            </section>
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

/* Car Schematic */
.car-schematic {
    position: relative;
    width: 160px;
    height: 280px;
    background: rgba(255,255,255,0.05);
    border: 2px solid rgba(255,255,255,0.2);
    border-radius: 40px 40px 20px 20px;
    margin: 20px auto;
}
.light-node {
    position: absolute;
    width: 30px;
    height: 15px;
    background: rgba(255,255,255,0.1);
    border-radius: 4px;
    cursor: pointer;
    transition: all 0.3s;
    display: flex;
    align-items: center;
    justify-content: center;
    font-size: 10px;
    font-weight: bold;
}
.light-node.active-white { background: #fff; box-shadow: 0 0 15px #fff; color: #000; }
.light-node.active-red { background: #ff3b3b; box-shadow: 0 0 15px #ff3b3b; color: #fff; }
.light-node.active-amber { background: #ff6600; box-shadow: 0 0 15px #ff6600; color: #000; }

.f-left { top: 10px; left: 10px; border-radius: 10px 4px 4px 4px; }
.f-right { top: 10px; right: 10px; border-radius: 4px 10px 4px 4px; }
.r-left { bottom: 10px; left: 10px; }
.r-right { bottom: 10px; right: 10px; }

@keyframes blink { 
    0% { opacity: 1; } 
    50% { opacity: 0.3; } 
    100% { opacity: 1; } 
}
.blinking { animation: blink 0.5s infinite; }
)rawliteral";

const char script_js[] PROGMEM = R"rawliteral(
document.addEventListener('DOMContentLoaded', () => {
    fetchStatus();
    setInterval(fetchStatus, 5000);
    document.getElementById('saveAlarmBtn').addEventListener('click', saveAlarm);
    document.getElementById('saveWifiBtn').addEventListener('click', saveWifi);
    document.getElementById('saveLocBtn').addEventListener('click', saveLoc);
    document.getElementById('testSoundBtn').addEventListener('click', testSound);
    document.getElementById('rebootBtn').addEventListener('click', rebootSystem);
    document.getElementById('ledEffect').addEventListener('change', (e) => { setLedEffect(e.target.value); });
    document.getElementById('ledColor').addEventListener('input', (e) => { setLedColor(e.target.value); });
    document.getElementById('ledBrightRange').addEventListener('input', (e) => { setLedBrightness(e.target.value); });
    document.getElementById('ledSpeedRange').addEventListener('input', (e) => { setLedSpeed(e.target.value); });
    document.getElementById('dispBrightRange').addEventListener('input', (e) => { setDisplayBrightness(e.target.value); });
    document.getElementById('turnSignal').addEventListener('change', (e) => { setTurnSignal(e.target.value); });
    document.getElementById('frontLightsBtn').addEventListener('click', toggleFront);
    document.getElementById('rearLightsBtn').addEventListener('click', toggleRear);
    // Schematic clicks
    document.getElementById('visual-f-l').addEventListener('click', toggleFront);
    document.getElementById('visual-f-r').addEventListener('click', toggleFront);
    document.getElementById('visual-r-l').addEventListener('click', toggleRear);
    document.getElementById('visual-r-r').addEventListener('click', toggleRear);
});

let frontOn = false;
let rearOn = false;

function toggleFront() {
    frontOn = !frontOn;
    updateVisuals();
    fetch('/api/settings/car_light?front=' + (frontOn ? 1 : 0));
}

function toggleLeftFront() {
    // Single click on schematic can also toggle all front for simplicity in this version
    toggleFront();
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

    // Update schematic nodes
    const fl = document.getElementById('visual-f-l');
    const fr = document.getElementById('visual-f-r');
    const rl = document.getElementById('visual-r-l');
    const rr = document.getElementById('visual-r-r');

    fl.classList.toggle('active-white', frontOn);
    fr.classList.toggle('active-white', frontOn);
    rl.classList.toggle('active-red', rearOn);
    rr.classList.toggle('active-red', rearOn);

    // Turn signal visuals
    const ts = document.getElementById('turnSignal').value;
    fl.classList.remove('active-amber', 'blinking');
    fr.classList.remove('active-amber', 'blinking');
    rl.classList.remove('active-amber', 'blinking');
    rr.classList.remove('active-amber', 'blinking');

    if (ts == '1') { // Left
        fl.classList.add('active-amber', 'blinking');
        rl.classList.add('active-amber', 'blinking');
    } else if (ts == '2') { // Right
        fr.classList.add('active-amber', 'blinking');
        rr.classList.add('active-amber', 'blinking');
    } else if (ts == '3') { // Hazard
        fl.classList.add('active-amber', 'blinking');
        fr.classList.add('active-amber', 'blinking');
        rl.classList.add('active-amber', 'blinking');
        rr.classList.add('active-amber', 'blinking');
    }
}

async function fetchStatus() {
    try {
        const response = await fetch('/api/status');
        if (!response.ok) throw new Error('Ошибка сети');
        const data = await response.json();
        document.getElementById('timeValue').textContent = data.time;
        document.getElementById('tempValue').textContent = data.temp + '\u00B0C';
        document.getElementById('weatherValue').textContent = translateCondition(data.condition);
        document.getElementById('connectionStatus').className = 'status-indicator online';
        
        // Update inputs if not focused (simple sync)
        if(document.activeElement !== document.getElementById('timeZone') && data.tz) {
             document.getElementById('timeZone').value = data.tz;
        }
        if(document.activeElement !== document.getElementById('weatherCity') && data.city) {
             document.getElementById('weatherCity').value = data.city;
        }

    } catch (error) {
        document.getElementById('connectionStatus').className = 'status-indicator';
    }
}
function translateCondition(c) {
    const t = {'Clear':'Ясно','Clouds':'Облачно','Rain':'Дождь','Snow':'Снег','Thunderstorm':'Гроза','Drizzle':'Морось','Mist':'Туман','Fog':'Туман','Haze':'Дымка'};
    return t[c] || c;
}
async function saveAlarm() {
    const time = document.getElementById('alarmTime').value;
    const sound = document.getElementById('alarmSound').value;
    if (!time) return alert('Пожалуйста, выберите время');
    try {
        await fetch('/api/alarm/set', { method: 'POST', headers: { 'Content-Type': 'application/json' }, body: JSON.stringify({ time: time, sound: sound }) });
        alert('Будильник сохранен!');
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
async function testSound() {
    const sound = document.getElementById('alarmSound').value;
    fetch('/api/sound/test?id=' + sound);
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
    updateVisuals();
    fetch('/api/settings/turn_signal?mode=' + val);
}
async function rebootSystem() {
    if (confirm('Перезагрузить систему?')) { fetch('/api/system/reboot'); }
}
)rawliteral";

#endif
