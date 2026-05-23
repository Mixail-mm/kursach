@echo off
chcp 65001 >nul
setlocal enabledelayedexpansion

set "LINK_DIR=%TEMP%\_build_!RANDOM!"
mklink /J "!LINK_DIR!" . >nul 2>&1
if errorlevel 1 (
    echo [ОШИБКА] Не удалось создать временную ссылку. Убедитесь, что папка доступна.
    pause & exit /b 1
)
cd /d "!LINK_DIR!"

where gcc >nul 2>&1 && goto :cc_found


echo [ОШИБКА] Компилятор GCC не найден ни в PATH, ни в стандартных папках.
echo Добавьте папку bin вашего MinGW в системную переменную PATH Windows.
cd .. & rmdir "!LINK_DIR!"
pause & exit /b 1
:cc_found

if not exist "variant_21\ref.bin" (
    echo [ОШИБКА] Не найдена папка variant_21 или файл ref.bin
    cd .. & rmdir "!LINK_DIR!"
    pause & exit /b 1
)

:: Очистка старой сборки
if exist "cmake-build-debug" rmdir /s /q "cmake-build-debug"

echo [1/5] Проверка входных данных... OK
echo [2/5] Конфигурация CMake...
:: Принудительно используем MinGW Makefiles, чтобы не ушёл в nmake/VS
cmake -G "MinGW Makefiles" -S . -B cmake-build-debug -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=gcc  || goto :err

echo [3/5] Сборка...
cmake --build cmake-build-debug || goto :err

echo [4/5] Запуск noise_analyzer...
cmake-build-debug\noise_analyzer.exe || goto :err

echo [5/5] Запуск circle_generator...
cmake-build-debug\circle_generator.exe || goto :err

echo.
echo [УСПЕХ] Все программы выполнены. Результаты в output/
cd .. & rmdir "!LINK_DIR!"
pause
exit /b 0

:err
echo.
echo [ОШИБКА] Процесс прерван. Проверьте лог выше.
cd .. & rmdir "!LINK_DIR!"
pause
exit /b 1