@echo off
setlocal

:: Настройки путей (автоматически найдены агентом)
set MKLITTLEFS="C:\Users\JZX\AppData\Local\Arduino15\packages\esp32\tools\mklittlefs\4.0.2-db0513a\mklittlefs.exe"
set ESPTOOL="C:\Users\JZX\AppData\Local\Arduino15\packages\esp32\tools\esptool_py\5.1.0\esptool.exe"

:: Пути проекта
set PROJECT_DIR=%~dp0
set DATA_DIR=%PROJECT_DIR%data
set IMAGE_FILE=%PROJECT_DIR%littlefs.bin

:: Настройки ESP32 (проверьте COM порт!)
set /p COM_PORT="Enter COM port (e.g. COM3): "
set BAUD_RATE=921600

:: Параметры файловой системы (According to partition dump: Size: 0x160000)
set PARTITION_SIZE=1441792
:: Адрес начала раздела
set PARTITION_OFFSET=0x290000

echo.
echo ==========================================
echo Building LittleFS Image...
echo Data Dir: %DATA_DIR%
echo Image File: %IMAGE_FILE%
echo ==========================================

%MKLITTLEFS% -c "%DATA_DIR%" -p 256 -b 4096 -s %PARTITION_SIZE% "%IMAGE_FILE%"

if %ERRORLEVEL% NEQ 0 (
    echo Error building image!
    pause
    exit /b %ERRORLEVEL%
)

echo.
echo ==========================================
echo Uploading to ESP32...
echo Port: %COM_PORT%
echo Speed: %BAUD_RATE%
echo Offset: %PARTITION_OFFSET%
echo ==========================================
echo NOTE: Hold BOOT button if upload doesn't start automatically.

%ESPTOOL% --chip esp32c3 --port %COM_PORT% --baud %BAUD_RATE% --before default_reset --after hard_reset write_flash %PARTITION_OFFSET% "%IMAGE_FILE%"

if %ERRORLEVEL% NEQ 0 (
    echo Error uploading!
    pause
    exit /b %ERRORLEVEL%
)

echo.
echo ==========================================
echo Done! Files uploaded successfully.
echo You can close this window.
echo ==========================================
pause
