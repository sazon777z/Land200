#line 1 "C:\\Users\\JZX\\Documents\\Arduino\\Land_Cruiser\\WebContent.h"
#ifndef WEB_CONTENT_H
#define WEB_CONTENT_H

#include <Arduino.h>

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="ru">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Land Cruiser 200 Control Deck</title>
    <link rel="preconnect" href="https://fonts.googleapis.com">
    <link rel="preconnect" href="https://fonts.gstatic.com" crossorigin>
    <link href="https://fonts.googleapis.com/css2?family=Manrope:wght@400;500;700;800&family=Space+Grotesk:wght@500;700&display=swap" rel="stylesheet">
    <link rel="stylesheet" href="style.css">
</head>
<body>
    <div class="ambient ambient-top"></div>
    <div class="ambient ambient-bottom"></div>

    <main class="dashboard">
        <section class="panel hero-panel span-12">
            <div class="hero-copy">
                <p class="eyebrow">Toyota Land Cruiser 200</p>
                <h1>Control Deck</h1>
                <p class="hero-text">Новый центр управления временем, будильником и подсветкой модели.</p>
            </div>

            <div class="hero-side">
                <div class="status-pill" id="connectionStatus">
                    <span class="status-dot"></span>
                    <span id="connectionLabel">Подключение...</span>
                </div>
                <p class="hero-tip">Двойное нажатие физической кнопки на корпусе переключает будильник.</p>
            </div>
        </section>

        <section class="panel stats-panel span-12">
            <div class="stat-card stat-time">
                <span class="stat-label">Время</span>
                <span class="stat-value stat-time-value" id="timeValue">--:--:--</span>
            </div>
            <div class="stat-card">
                <span class="stat-label">Температура</span>
                <span class="stat-value" id="tempValue">--&deg;C</span>
            </div>
            <div class="stat-card">
                <span class="stat-label">Погода</span>
                <span class="stat-value" id="weatherValue">--</span>
            </div>
            <div class="stat-card">
                <span class="stat-label">Город</span>
                <span class="stat-value" id="cityValue">--</span>
            </div>
        </section>

        <section class="panel alarm-panel span-7">
            <div class="panel-top">
                <div>
                    <p class="panel-kicker">Будильник</p>
                    <h2>Утренний сценарий</h2>
                    <p class="panel-text" id="alarmSummary">Будильник выключен. Включите тумблер или используйте двойное нажатие кнопки.</p>
                </div>

                <div class="alarm-toggle-box">
                    <span class="alarm-badge" id="savedAlarmBadge">Выключен</span>
                    <label class="switch" for="alarmToggle" aria-label="Включить будильник">
                        <input type="checkbox" id="alarmToggle">
                        <span class="switch-track">
                            <span class="switch-knob"></span>
                        </span>
                    </label>
                </div>
            </div>

            <div class="alarm-grid">
                <div class="field">
                    <label for="alarmTime">Время</label>
                    <input type="time" id="alarmTime" data-dirty="false">
                </div>

                <div class="field">
                    <label for="alarmSound">Звук</label>
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

                <div class="field span-2">
                    <div class="field-line">
                        <label for="alarmVolumeRange">Громкость</label>
                        <span class="range-value" id="alarmVolumeValue">20</span>
                    </div>
                    <input type="range" id="alarmVolumeRange" min="0" max="30" value="20" data-dirty="false">
                </div>

                <div class="field">
                    <label for="alarmCarEff">Эффект фар</label>
                    <select id="alarmCarEff" data-dirty="false">
                        <option value="0">Выкл</option>
                        <option value="1">Мигание фар</option>
                        <option value="2">Аварийка</option>
                        <option value="3">Полиция</option>
                    </select>
                </div>

                <div class="field">
                    <label for="alarmLedEff">Эффект днища</label>
                    <select id="alarmLedEff" data-dirty="false">
                        <option value="0">Оранжевый</option>
                        <option value="1">Радуга</option>
                        <option value="4">Полиция</option>
                        <option value="5">Строб</option>
                        <option value="2">Статичный цвет</option>
                    </select>
                </div>
            </div>
            <div class="alarm-note">
                <span class="note-mark">BTN</span>
                <span>Двойное нажатие кнопки на устройстве быстро включает или выключает будильник.</span>
            </div>

            <div class="button-row">
                <button class="btn btn-primary" type="button" id="saveAlarmBtn">Сохранить настройки</button>
                <button class="btn btn-secondary" type="button" id="testSoundBtn">Тест звука</button>
                <button class="btn btn-danger" type="button" id="stopSoundBtn">Стоп</button>
            </div>
        </section>

        <section class="panel scene-panel span-5">
            <p class="panel-kicker">Статус</p>
            <h2>Быстрый обзор</h2>

            <div class="status-stack">
                <div class="mini-card">
                    <span class="mini-label">Состояние будильника</span>
                    <strong id="alarmStateValue">Выключен</strong>
                </div>
                <div class="mini-card">
                    <span class="mini-label">Сетевой режим</span>
                    <strong id="wifiModeLabel">Подключение...</strong>
                </div>
                <div class="mini-card">
                    <span class="mini-label">Комментарий</span>
                    <strong id="alarmHelperText">Сохраните время и включите тумблер.</strong>
                </div>
            </div>

            <div class="scene-copy">
                <h3>Что нового</h3>
                <p>Интерфейс собран заново: крупный статус, быстрый тумблер будильника, понятные блоки настроек и отдельный раздел света.</p>
            </div>
        </section>

        <section class="panel lights-panel span-8">
            <div class="panel-top compact">
                <div>
                    <p class="panel-kicker">Подсветка</p>
                    <h2>Световой контур</h2>
                </div>
            </div>

            <div class="field">
                <label>Эффект днища</label>
                <div class="effect-grid" id="ledEffectButtons">
                    <button class="effect-btn" type="button" data-effect="0" style="--accent:#9aa7b2;">
                        <span class="effect-title">Выкл</span>
                        <span class="effect-meta">темный режим</span>
                    </button>
                    <button class="effect-btn" type="button" data-effect="1" style="--accent:#00c2ff;">
                        <span class="effect-title">Радуга</span>
                        <span class="effect-meta">поток цвета</span>
                    </button>
                    <button class="effect-btn" type="button" data-effect="2" style="--accent:#35d39d;">
                        <span class="effect-title">Цвет</span>
                        <span class="effect-meta">фиксированный тон</span>
                    </button>
                    <button class="effect-btn" type="button" data-effect="3" style="--accent:#ff8a3d;">
                        <span class="effect-title">Дыхание</span>
                        <span class="effect-meta">мягкая пульсация</span>
                    </button>
                    <button class="effect-btn" type="button" data-effect="4" style="--accent:#ff5d5d;">
                        <span class="effect-title">Полиция</span>
                        <span class="effect-meta">красный и синий</span>
                    </button>
                    <button class="effect-btn" type="button" data-effect="5" style="--accent:#ffd166;">
                        <span class="effect-title">Строб</span>
                        <span class="effect-meta">частые вспышки</span>
                    </button>
                    <button class="effect-btn" type="button" data-effect="6" style="--accent:#4cc9f0;">
                        <span class="effect-title">Затухание</span>
                        <span class="effect-meta">плавный цикл</span>
                    </button>
                    <button class="effect-btn" type="button" data-effect="7" style="--accent:#80ed99;">
                        <span class="effect-title">Блеск</span>
                        <span class="effect-meta">искры</span>
                    </button>
                    <button class="effect-btn" type="button" data-effect="8" style="--accent:#f77f00;">
                        <span class="effect-title">Сканер</span>
                        <span class="effect-meta">движущаяся полоса</span>
                    </button>
                </div>
            </div>

            <div class="lights-grid">
                <div class="field">
                    <label for="ledColor">Цвет</label>
                    <input type="color" id="ledColor" value="#00d2ff">
                </div>

                <div class="field">
                    <div class="field-line">
                        <label for="ledBrightRange">Яркость</label>
                        <span class="range-value" id="ledBrightValue">150</span>
                    </div>
                    <input type="range" id="ledBrightRange" min="0" max="255" value="150">
                </div>

                <div class="field">
                    <div class="field-line">
                        <label for="ledSpeedRange">Скорость</label>
                        <span class="range-value" id="ledSpeedValue">30</span>
                    </div>
                    <input type="range" id="ledSpeedRange" min="1" max="100" value="30">
                </div>
            </div>

            <div class="field">
                <label>Свет модели</label>
                <div class="car-actions">
                    <button class="btn btn-ghost" type="button" id="btnLowBeam">Ближний</button>
                    <button class="btn btn-ghost" type="button" id="btnParking">Габариты</button>
                    <button class="btn btn-ghost" type="button" id="btnTurnLeft">Левый</button>
                    <button class="btn btn-ghost" type="button" id="btnTurnRight">Правый</button>
                    <button class="btn btn-ghost danger-ghost" type="button" id="btnHazard">Аварийка</button>
                </div>
            </div>
        </section>
        <section class="panel settings-panel span-4">
            <div class="settings-group">
                <p class="panel-kicker">Локализация</p>
                <h2>Время и город</h2>

                <div class="field">
                    <label for="timeZone">Часовой пояс UTC</label>
                    <input type="number" id="timeZone" min="-12" max="14" value="5">
                </div>

                <div class="field">
                    <label for="weatherCity">Город для погоды</label>
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
                        <option value="Yekaterinburg">Екатеринбург</option>
                        <option value="Chelyabinsk">Челябинск</option>
                        <option value="Tyumen">Тюмень</option>
                        <option value="Omsk">Омск</option>
                        <option value="Moscow">Москва</option>
                        <option value="Saint Petersburg">Санкт-Петербург</option>
                        <option value="Novosibirsk">Новосибирск</option>
                        <option value="Sochi">Сочи</option>
                    </select>
                </div>

                <button class="btn btn-primary" type="button" id="saveLocBtn">Сохранить локализацию</button>
            </div>

            <div class="settings-divider"></div>

            <div class="settings-group">
                <p class="panel-kicker">Wi-Fi</p>
                <h2>Сеть</h2>

                <div id="wifiConnectForm">
                    <div class="field">
                        <label for="wifiSsid">SSID</label>
                        <input type="text" id="wifiSsid" placeholder="Имя сети">
                    </div>

                    <div class="field">
                        <label for="wifiPass">Пароль</label>
                        <input type="password" id="wifiPass" placeholder="Пароль">
                    </div>

                    <button class="btn btn-primary" type="button" id="saveWifiBtn">Сохранить и перезагрузить</button>
                </div>

                <div id="wifiConnectedInfo" class="wifi-info" hidden>
                    <p>Устройство уже подключено к сети. Можно очистить сохраненные данные и вернуться в режим точки доступа.</p>
                    <button class="btn btn-secondary" type="button" id="resetWifiBtn">Сбросить Wi-Fi</button>
                </div>
            </div>

            <div class="settings-divider"></div>

            <div class="settings-group">
                <p class="panel-kicker">Система</p>
                <h2>Экран и перезапуск</h2>

                <div class="field">
                    <div class="field-line">
                        <label for="dispBrightRange">Яркость экрана</label>
                        <span class="range-value" id="dispBrightValue">255</span>
                    </div>
                    <input type="range" id="dispBrightRange" min="0" max="255" value="255">
                </div>

                <button class="btn btn-danger" type="button" id="rebootBtn">Перезагрузить систему</button>
            </div>
        </section>
    </main>

    <div class="toast" id="toast" aria-live="polite"></div>

    <script src="script.js"></script>
</body>
</html>
)rawliteral";

const char style_css[] PROGMEM = R"rawliteral(
:root {
    color-scheme: light;
    --bg: #f4efe7;
    --bg-soft: #fbf8f3;
    --panel: rgba(255, 251, 245, 0.88);
    --panel-strong: rgba(255, 248, 238, 0.96);
    --line: rgba(80, 53, 30, 0.12);
    --line-strong: rgba(80, 53, 30, 0.2);
    --text: #2c221b;
    --muted: #746557;
    --accent: #db6d2a;
    --accent-strong: #b94d15;
    --accent-soft: rgba(219, 109, 42, 0.14);
    --ok: #2d8f5b;
    --ok-soft: rgba(45, 143, 91, 0.12);
    --danger: #d64933;
    --danger-soft: rgba(214, 73, 51, 0.12);
    --shadow: 0 24px 60px rgba(83, 56, 35, 0.14);
    --radius-xl: 28px;
    --radius-lg: 22px;
    --radius-md: 16px;
    --radius-sm: 12px;
}

* {
    box-sizing: border-box;
}

html, body {
    margin: 0;
    min-height: 100%;
}

body {
    position: relative;
    overflow-x: hidden;
    background:
        radial-gradient(circle at top left, rgba(255, 184, 120, 0.38), transparent 32%),
        radial-gradient(circle at 85% 10%, rgba(255, 229, 193, 0.7), transparent 24%),
        linear-gradient(160deg, #f6f1e7 0%, #efe7da 48%, #f8f4ed 100%);
    color: var(--text);
    font-family: 'Manrope', sans-serif;
}

.ambient {
    position: fixed;
    inset: auto;
    width: 420px;
    height: 420px;
    border-radius: 50%;
    pointer-events: none;
    filter: blur(18px);
    opacity: 0.58;
}

.ambient-top {
    top: -160px;
    right: -60px;
    background: radial-gradient(circle, rgba(219, 109, 42, 0.22) 0%, rgba(219, 109, 42, 0) 70%);
}

.ambient-bottom {
    left: -160px;
    bottom: -180px;
    background: radial-gradient(circle, rgba(38, 115, 84, 0.18) 0%, rgba(38, 115, 84, 0) 70%);
}

.dashboard {
    position: relative;
    z-index: 1;
    display: grid;
    grid-template-columns: repeat(12, minmax(0, 1fr));
    gap: 18px;
    width: min(1380px, calc(100% - 32px));
    margin: 24px auto 40px;
}

.span-12 { grid-column: span 12; }
.span-8 { grid-column: span 8; }
.span-7 { grid-column: span 7; }
.span-5 { grid-column: span 5; }
.span-4 { grid-column: span 4; }

.panel {
    position: relative;
    padding: 24px;
    border: 1px solid var(--line);
    border-radius: var(--radius-xl);
    background: linear-gradient(180deg, var(--panel-strong), var(--panel));
    box-shadow: var(--shadow);
    backdrop-filter: blur(14px);
}

.hero-panel {
    display: flex;
    align-items: end;
    justify-content: space-between;
    gap: 24px;
    padding: 28px;
    overflow: hidden;
}

.hero-panel::after {
    content: '';
    position: absolute;
    right: -40px;
    bottom: -70px;
    width: 280px;
    height: 180px;
    border-radius: 140px;
    background: linear-gradient(135deg, rgba(219, 109, 42, 0.18), rgba(255, 255, 255, 0));
    transform: rotate(-10deg);
}

.hero-copy,
.hero-side {
    position: relative;
    z-index: 1;
}

.eyebrow,
.panel-kicker {
    margin: 0 0 10px;
    font-size: 12px;
    font-weight: 800;
    letter-spacing: 0.22em;
    text-transform: uppercase;
    color: var(--accent-strong);
}

.hero-copy h1,
.panel h2,
.scene-copy h3 {
    margin: 0;
    font-family: 'Space Grotesk', sans-serif;
    letter-spacing: -0.03em;
}

.hero-copy h1 {
    font-size: clamp(34px, 5vw, 56px);
}

.hero-text,
.panel-text,
.hero-tip,
.scene-copy p,
.wifi-info p {
    margin: 10px 0 0;
    color: var(--muted);
    line-height: 1.55;
}

.hero-side {
    max-width: 320px;
}

.status-pill {
    display: inline-flex;
    align-items: center;
    gap: 10px;
    padding: 10px 16px;
    border-radius: 999px;
    background: rgba(44, 34, 27, 0.08);
    color: var(--text);
    font-weight: 700;
}

.status-pill.online {
    background: var(--ok-soft);
    color: #1f6f45;
}

.status-pill.ap {
    background: var(--accent-soft);
    color: var(--accent-strong);
}

.status-pill.offline {
    background: rgba(80, 53, 30, 0.08);
    color: var(--muted);
}

.status-dot {
    width: 10px;
    height: 10px;
    border-radius: 50%;
    background: currentColor;
}

.stats-panel {
    display: grid;
    grid-template-columns: 2fr 1fr 1fr 1fr;
    gap: 14px;
    padding: 14px;
    background: transparent;
    border: none;
    box-shadow: none;
}

.stat-card {
    display: flex;
    flex-direction: column;
    justify-content: space-between;
    min-height: 132px;
    padding: 22px;
    border: 1px solid var(--line);
    border-radius: var(--radius-lg);
    background: linear-gradient(180deg, rgba(255, 251, 244, 0.94), rgba(255, 247, 236, 0.88));
    box-shadow: 0 16px 34px rgba(83, 56, 35, 0.08);
}

.stat-time {
    background: linear-gradient(135deg, rgba(255, 244, 222, 0.98), rgba(255, 232, 201, 0.92));
}

.stat-label,
.mini-label,
.field label {
    font-size: 13px;
    font-weight: 700;
    color: var(--muted);
}

.stat-value {
    font-family: 'Space Grotesk', sans-serif;
    font-size: clamp(24px, 3vw, 34px);
    font-weight: 700;
    letter-spacing: -0.04em;
}

.stat-time-value {
    font-size: clamp(34px, 5vw, 54px);
}

.panel-top {
    display: flex;
    align-items: start;
    justify-content: space-between;
    gap: 18px;
    margin-bottom: 22px;
}

.panel-top.compact {
    margin-bottom: 18px;
}

.alarm-toggle-box {
    display: flex;
    flex-direction: column;
    align-items: flex-end;
    gap: 14px;
}

.alarm-badge {
    display: inline-flex;
    align-items: center;
    justify-content: center;
    min-width: 110px;
    padding: 8px 14px;
    border-radius: 999px;
    background: rgba(44, 34, 27, 0.08);
    color: var(--muted);
    font-weight: 800;
}

.alarm-badge.on {
    background: var(--accent);
    color: #fff7f0;
}
.switch {
    display: inline-flex;
    cursor: pointer;
}

.switch input {
    position: absolute;
    opacity: 0;
    pointer-events: none;
}

.switch-track {
    position: relative;
    width: 78px;
    height: 42px;
    border-radius: 999px;
    background: rgba(44, 34, 27, 0.12);
    transition: background 0.2s ease;
}

.switch-knob {
    position: absolute;
    top: 5px;
    left: 5px;
    width: 32px;
    height: 32px;
    border-radius: 50%;
    background: #fff;
    box-shadow: 0 8px 18px rgba(44, 34, 27, 0.18);
    transition: transform 0.2s ease;
}

.switch input:checked + .switch-track {
    background: linear-gradient(135deg, var(--accent), #f09551);
}

.switch input:checked + .switch-track .switch-knob {
    transform: translateX(36px);
}

.switch input:disabled + .switch-track {
    opacity: 0.55;
}

.alarm-grid,
.lights-grid {
    display: grid;
    grid-template-columns: repeat(2, minmax(0, 1fr));
    gap: 16px;
}

.field {
    display: flex;
    flex-direction: column;
    gap: 10px;
}

.field.span-2 {
    grid-column: span 2;
}

.field-line {
    display: flex;
    align-items: center;
    justify-content: space-between;
    gap: 10px;
}

input,
select,
button {
    font: inherit;
}

input[type="time"],
input[type="number"],
input[type="text"],
input[type="password"],
select {
    width: 100%;
    min-height: 50px;
    padding: 0 16px;
    border: 1px solid var(--line-strong);
    border-radius: var(--radius-md);
    background: rgba(255, 255, 255, 0.72);
    color: var(--text);
    outline: none;
    transition: border-color 0.2s ease, box-shadow 0.2s ease, background 0.2s ease;
}

input[type="color"] {
    width: 100%;
    height: 52px;
    padding: 6px;
    border: 1px solid var(--line-strong);
    border-radius: var(--radius-md);
    background: rgba(255, 255, 255, 0.72);
    cursor: pointer;
}

input:focus,
select:focus {
    border-color: rgba(219, 109, 42, 0.55);
    box-shadow: 0 0 0 4px rgba(219, 109, 42, 0.12);
    background: rgba(255, 255, 255, 0.96);
}

input[type="range"] {
    width: 100%;
    accent-color: var(--accent);
}

.range-value {
    display: inline-flex;
    align-items: center;
    justify-content: center;
    min-width: 44px;
    padding: 5px 10px;
    border-radius: 999px;
    background: rgba(44, 34, 27, 0.08);
    color: var(--text);
    font-size: 13px;
    font-weight: 800;
}

.alarm-note {
    display: flex;
    align-items: center;
    gap: 12px;
    margin-top: 18px;
    padding: 14px 16px;
    border: 1px solid rgba(219, 109, 42, 0.18);
    border-radius: var(--radius-md);
    background: linear-gradient(135deg, rgba(255, 238, 216, 0.78), rgba(255, 246, 233, 0.92));
    color: var(--text);
}

.note-mark {
    display: inline-flex;
    align-items: center;
    justify-content: center;
    min-width: 42px;
    height: 42px;
    border-radius: 14px;
    background: rgba(219, 109, 42, 0.14);
    color: var(--accent-strong);
    font-family: 'Space Grotesk', sans-serif;
    font-weight: 700;
}

.button-row,
.car-actions {
    display: flex;
    flex-wrap: wrap;
    gap: 12px;
    margin-top: 18px;
}

.btn {
    display: inline-flex;
    align-items: center;
    justify-content: center;
    min-height: 48px;
    padding: 0 18px;
    border: 1px solid transparent;
    border-radius: 999px;
    cursor: pointer;
    transition: transform 0.18s ease, box-shadow 0.18s ease, border-color 0.18s ease, background 0.18s ease;
}

.btn:hover {
    transform: translateY(-1px);
}

.btn-primary {
    background: linear-gradient(135deg, var(--accent), #f09351);
    color: #fff;
    box-shadow: 0 12px 24px rgba(219, 109, 42, 0.24);
}

.btn-secondary {
    background: rgba(44, 34, 27, 0.08);
    color: var(--text);
    border-color: rgba(44, 34, 27, 0.08);
}

.btn-danger {
    background: var(--danger);
    color: #fff;
    box-shadow: 0 12px 24px rgba(214, 73, 51, 0.2);
}

.btn-ghost {
    background: rgba(44, 34, 27, 0.05);
    color: var(--text);
    border-color: var(--line);
}

.btn-ghost.active {
    background: var(--accent-soft);
    border-color: rgba(219, 109, 42, 0.4);
    color: var(--accent-strong);
}

.danger-ghost.active {
    background: var(--danger-soft);
    border-color: rgba(214, 73, 51, 0.3);
    color: var(--danger);
}

.status-stack {
    display: grid;
    gap: 12px;
}

.mini-card {
    display: flex;
    flex-direction: column;
    gap: 8px;
    padding: 18px;
    border: 1px solid var(--line);
    border-radius: var(--radius-lg);
    background: rgba(255, 255, 255, 0.56);
}

.mini-card strong {
    font-family: 'Space Grotesk', sans-serif;
    font-size: 20px;
}

.scene-copy {
    margin-top: 22px;
    padding-top: 20px;
    border-top: 1px solid var(--line);
}
.effect-grid {
    display: grid;
    grid-template-columns: repeat(3, minmax(0, 1fr));
    gap: 12px;
}

.effect-btn {
    display: flex;
    flex-direction: column;
    align-items: flex-start;
    gap: 6px;
    min-height: 108px;
    padding: 16px;
    border: 1px solid transparent;
    border-radius: var(--radius-lg);
    background:
        linear-gradient(180deg, rgba(255, 255, 255, 0.78), rgba(255, 255, 255, 0.58));
    box-shadow: inset 0 0 0 1px rgba(255, 255, 255, 0.35);
    cursor: pointer;
    transition: transform 0.18s ease, border-color 0.18s ease, box-shadow 0.18s ease;
}

.effect-btn::before {
    content: '';
    width: 44px;
    height: 8px;
    border-radius: 999px;
    background: var(--accent);
    opacity: 0.9;
}

.effect-btn.active {
    border-color: var(--accent);
    box-shadow: 0 18px 30px rgba(219, 109, 42, 0.14);
    transform: translateY(-2px);
}

.effect-btn .effect-title {
    font-family: 'Space Grotesk', sans-serif;
    font-size: 18px;
    font-weight: 700;
}

.effect-btn .effect-meta {
    color: var(--muted);
    font-size: 13px;
}

.settings-panel {
    display: flex;
    flex-direction: column;
    gap: 4px;
}

.settings-group h2 {
    margin-bottom: 18px;
}

.settings-divider {
    height: 1px;
    margin: 10px 0 6px;
    background: var(--line);
}

.wifi-info {
    display: grid;
    gap: 12px;
}

.toast {
    position: fixed;
    left: 50%;
    bottom: 24px;
    transform: translateX(-50%) translateY(20px);
    min-width: 220px;
    max-width: min(90vw, 420px);
    padding: 14px 18px;
    border-radius: 999px;
    background: rgba(44, 34, 27, 0.92);
    color: #fff;
    text-align: center;
    opacity: 0;
    pointer-events: none;
    transition: opacity 0.22s ease, transform 0.22s ease;
    box-shadow: 0 20px 40px rgba(44, 34, 27, 0.28);
}

.toast.show {
    opacity: 1;
    transform: translateX(-50%) translateY(0);
}

.toast.error {
    background: rgba(214, 73, 51, 0.95);
}

@media (max-width: 1120px) {
    .span-8,
    .span-7,
    .span-5,
    .span-4 {
        grid-column: span 12;
    }

    .hero-panel {
        flex-direction: column;
        align-items: flex-start;
    }

    .stats-panel {
        grid-template-columns: repeat(2, minmax(0, 1fr));
    }
}

@media (max-width: 760px) {
    .dashboard {
        width: min(100% - 20px, 1380px);
        margin-top: 14px;
        gap: 14px;
    }

    .panel {
        padding: 18px;
        border-radius: 24px;
    }

    .stats-panel,
    .alarm-grid,
    .lights-grid,
    .effect-grid {
        grid-template-columns: 1fr;
    }

    .field.span-2 {
        grid-column: span 1;
    }

    .panel-top {
        flex-direction: column;
    }

    .alarm-toggle-box {
        align-items: flex-start;
    }

    .button-row .btn,
    .car-actions .btn {
        width: 100%;
    }
}
)rawliteral";

const char script_js[] PROGMEM = R"rawliteral(
const ALARM_FIELD_IDS = ['alarmTime', 'alarmSound', 'alarmCarEff', 'alarmLedEff', 'alarmVolumeRange'];

let selectedLedEffect = Number(localStorage.getItem('selectedLedEffect') || 0);
let toastTimer = null;
let frontOn = false;
let rearOn = false;
let currentTS = 0;

document.addEventListener('DOMContentLoaded', () => {
    initDirtyTracking();
    initEffectButtons();
    bindUi();
    syncAllRangeLabels();
    updateVisuals();
    fetchStatus();
    setInterval(fetchStatus, 5000);
});

function bindUi() {
    bind('saveAlarmBtn', 'click', saveAlarm);
    bind('testSoundBtn', 'click', testSound);
    bind('stopSoundBtn', 'click', stopSound);
    bind('saveLocBtn', 'click', saveLoc);
    bind('saveWifiBtn', 'click', saveWifi);
    bind('resetWifiBtn', 'click', resetWifi);
    bind('rebootBtn', 'click', rebootSystem);
    bind('alarmToggle', 'change', onAlarmToggleChange);
    bind('alarmVolumeRange', 'input', (event) => {
        updateRangeLabel('alarmVolumeRange', 'alarmVolumeValue');
        setAlarmVolume(event.target.value);
    });
    bind('ledBrightRange', 'input', (event) => {
        updateRangeLabel('ledBrightRange', 'ledBrightValue');
        setLedBrightness(event.target.value);
    });
    bind('ledSpeedRange', 'input', (event) => {
        updateRangeLabel('ledSpeedRange', 'ledSpeedValue');
        setLedSpeed(event.target.value);
    });
    bind('dispBrightRange', 'input', (event) => {
        updateRangeLabel('dispBrightRange', 'dispBrightValue');
        setDisplayBrightness(event.target.value);
    });
    bind('ledColor', 'change', (event) => setLedColor(event.target.value));
    bind('btnLowBeam', 'click', toggleLowBeam);
    bind('btnParking', 'click', toggleParking);
    bind('btnTurnLeft', 'click', () => toggleTurn(1));
    bind('btnTurnRight', 'click', () => toggleTurn(2));
    bind('btnHazard', 'click', () => toggleTurn(3));
}

function bind(id, eventName, handler) {
    const node = document.getElementById(id);
    if (node) {
        node.addEventListener(eventName, handler);
    }
}

function initDirtyTracking() {
    ALARM_FIELD_IDS.forEach((id) => {
        const node = document.getElementById(id);
        if (!node) return;
        const markDirty = () => {
            node.dataset.dirty = 'true';
        };
        node.addEventListener('input', markDirty);
        node.addEventListener('change', markDirty);
    });
}

function initEffectButtons() {
    document.querySelectorAll('.effect-btn[data-effect]').forEach((button) => {
        button.addEventListener('click', () => {
            const effect = Number(button.dataset.effect);
            if (Number.isNaN(effect)) return;
            selectedLedEffect = effect;
            localStorage.setItem('selectedLedEffect', String(effect));
            setActiveEffectButton(effect);
            setLedEffect(effect);
        });
    });

    setActiveEffectButton(selectedLedEffect);
}

function setActiveEffectButton(effect) {
    document.querySelectorAll('.effect-btn[data-effect]').forEach((button) => {
        const isActive = Number(button.dataset.effect) === Number(effect);
        button.classList.toggle('active', isActive);
        button.setAttribute('aria-pressed', isActive ? 'true' : 'false');
    });
}

function updateField(id, value) {
    const node = document.getElementById(id);
    if (!node) return;
    if (document.activeElement === node) return;
    if (node.dataset.dirty === 'true') return;
    if (value === undefined || value === null) return;
    node.value = String(value);
    if (node.type === 'range') {
        updateRangeLabel(id, id === 'alarmVolumeRange' ? 'alarmVolumeValue' :
            id === 'ledBrightRange' ? 'ledBrightValue' :
            id === 'ledSpeedRange' ? 'ledSpeedValue' :
            id === 'dispBrightRange' ? 'dispBrightValue' : '');
    }
}

function resetDirty(ids) {
    ids.forEach((id) => {
        const node = document.getElementById(id);
        if (node) node.dataset.dirty = 'false';
    });
}

function syncAllRangeLabels() {
    updateRangeLabel('alarmVolumeRange', 'alarmVolumeValue');
    updateRangeLabel('ledBrightRange', 'ledBrightValue');
    updateRangeLabel('ledSpeedRange', 'ledSpeedValue');
    updateRangeLabel('dispBrightRange', 'dispBrightValue');
}

function updateRangeLabel(inputId, labelId) {
    if (!labelId) return;
    const input = document.getElementById(inputId);
    const label = document.getElementById(labelId);
    if (input && label) {
        label.textContent = input.value;
    }
}

async function fetchStatus() {
    try {
        const response = await fetch('/api/status');
        if (!response.ok) {
            throw new Error('status');
        }

        const data = await response.json();
        renderStatus(data);
    } catch (error) {
        const status = document.getElementById('connectionStatus');
        const label = document.getElementById('connectionLabel');
        if (status) status.className = 'status-pill offline';
        if (label) label.textContent = 'Нет связи';
    }
}
function renderStatus(data) {
    text('timeValue', data.time || '--:--:--');
    text('tempValue', `${data.temp || '--'}\u00B0C`);
    text('weatherValue', translateCondition(data.condition));
    text('cityValue', translateCity(data.city));

    const isApMode = Boolean(data.is_ap);
    const status = document.getElementById('connectionStatus');
    const label = document.getElementById('connectionLabel');
    if (status && label) {
        if (isApMode) {
            status.className = 'status-pill ap';
            label.textContent = 'Режим точки доступа';
        } else {
            status.className = 'status-pill online';
            label.textContent = 'Онлайн';
        }
    }

    updateAlarmCard(data);

    updateField('alarmTime', data.alarm_time);
    updateField('alarmSound', data.alarm_sound);
    updateField('alarmCarEff', data.alarm_car_eff);
    updateField('alarmLedEff', data.alarm_led_eff);
    updateField('alarmVolumeRange', data.alarm_volume);
    updateField('timeZone', data.tz);
    updateField('weatherCity', data.city);

    if (data.led_effect !== undefined && data.led_effect !== null) {
        selectedLedEffect = Number(data.led_effect);
        if (!Number.isNaN(selectedLedEffect)) {
            localStorage.setItem('selectedLedEffect', String(selectedLedEffect));
            setActiveEffectButton(selectedLedEffect);
        }
    }

    const wifiForm = document.getElementById('wifiConnectForm');
    const wifiInfo = document.getElementById('wifiConnectedInfo');
    if (wifiForm && wifiInfo) {
        wifiForm.hidden = !isApMode;
        wifiInfo.hidden = isApMode;
    }

    text('wifiModeLabel', isApMode ? 'Точка доступа' : 'Домашняя сеть');
}

function updateAlarmCard(data) {
    const enabled = Boolean(data.alarm_enabled);
    const alarmTime = data.alarm_time || '--:--';
    const toggle = document.getElementById('alarmToggle');
    const badge = document.getElementById('savedAlarmBadge');

    if (toggle && toggle.dataset.busy !== 'true' && document.activeElement !== toggle) {
        toggle.checked = enabled;
    }

    if (badge) {
        badge.textContent = enabled ? alarmTime : 'Выключен';
        badge.classList.toggle('on', enabled);
    }

    text('alarmStateValue', enabled ? `Включен на ${alarmTime}` : 'Выключен');
    text('alarmHelperText', enabled
        ? 'Будильник активен. Двойное нажатие кнопки отключит его.'
        : 'Будильник не активен. Двойное нажатие кнопки включит его.');
    text('alarmSummary', enabled
        ? `Будильник включен на ${alarmTime}. Можно быстро отключить тумблером или двойным нажатием кнопки.`
        : 'Будильник выключен. Включите тумблер или используйте двойное нажатие кнопки.');
}

function text(id, value) {
    const node = document.getElementById(id);
    if (node) {
        node.textContent = value;
    }
}

async function onAlarmToggleChange(event) {
    await setAlarmEnabled(event.target.checked);
}

async function setAlarmEnabled(enabled) {
    const toggle = document.getElementById('alarmToggle');
    if (!toggle) return;

    toggle.dataset.busy = 'true';
    toggle.disabled = true;

    try {
        const response = await fetch('/api/alarm/enabled', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ enabled })
        });

        if (!response.ok) {
            throw new Error('toggle');
        }

        showToast(enabled ? 'Будильник включен' : 'Будильник выключен');
    } catch (error) {
        toggle.checked = !enabled;
        showToast('Не удалось изменить состояние будильника', true);
    } finally {
        toggle.disabled = false;
        toggle.dataset.busy = 'false';
        fetchStatus();
    }
}

async function saveAlarm() {
    const time = value('alarmTime');
    const sound = Number(value('alarmSound'));
    const carEff = Number(value('alarmCarEff'));
    const ledEff = Number(value('alarmLedEff'));
    const volume = Number(value('alarmVolumeRange'));
    const enabled = checked('alarmToggle');

    if (!time) {
        showToast('Сначала выберите время будильника', true);
        return;
    }

    try {
        const response = await fetch('/api/alarm/set', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({
                time,
                sound,
                carEff,
                ledEff,
                volume,
                enabled
            })
        });

        if (!response.ok) {
            throw new Error('save-alarm');
        }

        resetDirty(ALARM_FIELD_IDS);
        showToast(enabled ? 'Будильник сохранен' : 'Настройки сохранены, будильник выключен');
        fetchStatus();
    } catch (error) {
        showToast('Ошибка при сохранении будильника', true);
    }
}
async function saveWifi() {
    const ssid = value('wifiSsid').trim();
    const pass = value('wifiPass');

    if (!ssid) {
        showToast('Введите имя сети Wi-Fi', true);
        return;
    }

    if (!confirm('Сохранить Wi-Fi и перезагрузить устройство?')) {
        return;
    }

    try {
        const response = await fetch('/api/settings/wifi', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ ssid, pass })
        });

        if (!response.ok) {
            throw new Error('wifi');
        }

        showToast('Настройки Wi-Fi сохранены. Перезагрузка...');
        setTimeout(() => location.reload(), 5000);
    } catch (error) {
        showToast('Ошибка при сохранении Wi-Fi', true);
    }
}

async function saveLoc() {
    const timezone = Number(value('timeZone'));
    const city = value('weatherCity');

    try {
        const response = await fetch('/api/settings/loc', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ timezone, city })
        });

        if (!response.ok) {
            throw new Error('loc');
        }

        showToast('Локализация сохранена');
        fetchStatus();
    } catch (error) {
        showToast('Ошибка при сохранении локализации', true);
    }
}

async function resetWifi() {
    if (!confirm('Очистить настройки Wi-Fi и перейти в режим точки доступа?')) {
        return;
    }

    try {
        const response = await fetch('/api/settings/wifi_reset');
        if (!response.ok) {
            throw new Error('wifi-reset');
        }

        showToast('Wi-Fi очищен. Устройство перезагружается...');
        setTimeout(() => location.reload(), 5000);
    } catch (error) {
        showToast('Ошибка при сбросе Wi-Fi', true);
    }
}

function testSound() {
    const sound = value('alarmSound');
    fetch(`/api/sound/test?id=${encodeURIComponent(sound)}`);
    showToast('Тестовый звук отправлен');
}

function stopSound() {
    fetch('/api/sound/stop');
    showToast('Звук остановлен');
}

function setAlarmVolume(valueToSend) {
    fetch(`/api/settings/alarm_volume?val=${encodeURIComponent(valueToSend)}`);
}

function setLedBrightness(valueToSend) {
    fetch(`/api/settings/led_bright?val=${encodeURIComponent(valueToSend)}`);
}

function setDisplayBrightness(valueToSend) {
    fetch(`/api/settings/disp_bright?val=${encodeURIComponent(valueToSend)}`);
}

function setLedSpeed(valueToSend) {
    fetch(`/api/settings/led_speed?val=${encodeURIComponent(valueToSend)}`);
}

function setLedEffect(valueToSend) {
    fetch(`/api/settings/led?eff=${encodeURIComponent(valueToSend)}`);
}

function setLedColor(color) {
    fetch(`/api/settings/led_color?hex=${encodeURIComponent(color)}`);
}

function toggleLowBeam() {
    frontOn = !frontOn;
    rearOn = frontOn;
    updateVisuals();
    fetch(`/api/settings/car_light?front=${frontOn ? 1 : 0}&rear=${rearOn ? 1 : 0}`);
}

function toggleParking() {
    rearOn = !rearOn;
    if (!rearOn) frontOn = false;
    updateVisuals();
    fetch(`/api/settings/car_light?rear=${rearOn ? 1 : 0}`);
}

function toggleTurn(mode) {
    currentTS = currentTS === mode ? 0 : mode;
    updateVisuals();
    fetch(`/api/settings/turn_signal?mode=${currentTS}`);
}

function updateVisuals() {
    toggleButtonState('btnLowBeam', frontOn);
    toggleButtonState('btnParking', rearOn);
    toggleButtonState('btnTurnLeft', currentTS === 1);
    toggleButtonState('btnTurnRight', currentTS === 2);
    toggleButtonState('btnHazard', currentTS === 3);
}

function toggleButtonState(id, active) {
    const node = document.getElementById(id);
    if (node) {
        node.classList.toggle('active', active);
    }
}

async function rebootSystem() {
    if (!confirm('Перезагрузить систему?')) {
        return;
    }

    fetch('/api/system/reboot');
    showToast('Перезагрузка запущена');
}
function value(id) {
    const node = document.getElementById(id);
    return node ? node.value : '';
}

function checked(id) {
    const node = document.getElementById(id);
    return node ? node.checked : false;
}

function showToast(message, isError = false) {
    const toast = document.getElementById('toast');
    if (!toast) return;

    toast.textContent = message;
    toast.classList.toggle('error', isError);
    toast.classList.add('show');

    if (toastTimer) {
        clearTimeout(toastTimer);
    }

    toastTimer = setTimeout(() => {
        toast.classList.remove('show');
        toast.classList.remove('error');
    }, 2600);
}

function translateCondition(condition) {
    const map = {
        Clear: 'Ясно',
        Clouds: 'Облачно',
        Rain: 'Дождь',
        Snow: 'Снег',
        Thunderstorm: 'Гроза',
        Drizzle: 'Морось',
        Mist: 'Туман',
        Fog: 'Туман',
        Haze: 'Дымка'
    };

    return map[condition] || condition || '--';
}

function translateCity(city) {
    const map = {
        Almaty: 'Алматы',
        Astana: 'Астана',
        Shymkent: 'Шымкент',
        Aktobe: 'Актобе',
        Karaganda: 'Караганда',
        Taraz: 'Тараз',
        Pavlodar: 'Павлодар',
        'Ust-Kamenogorsk': 'Усть-Каменогорск',
        Semey: 'Семей',
        Aktau: 'Актау',
        Kostanay: 'Костанай',
        Kyzylorda: 'Кызылорда',
        Atyrau: 'Атырау',
        Petropavl: 'Петропавловск',
        Rudny: 'Рудный',
        Yekaterinburg: 'Екатеринбург',
        Chelyabinsk: 'Челябинск',
        Tyumen: 'Тюмень',
        Omsk: 'Омск',
        Moscow: 'Москва',
        'Saint Petersburg': 'Санкт-Петербург',
        Novosibirsk: 'Новосибирск',
        Sochi: 'Сочи'
    };

    return map[city] || city || '--';
}
)rawliteral";

#endif // WEB_CONTENT_H
