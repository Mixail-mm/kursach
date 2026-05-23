@echo off
chcp 65001
cd /d "%~dp0"
set PATH=C:\Program Files\JetBrains\CLion 2025.3.4\bin\cmake\win\x64\bin;C:\Program Files\JetBrains\CLion 2025.3.4\bin\mingw\bin;%PATH%

echo [1/5] Проверка входных данных...
if not exist "variant_21\ref.bin" (
    echo [ОШИБКА] Не найдена папка variant_21 или ref.bin
    pause & exit /b 1
)

echo [2/5] Конфигурация CMake...
cmake -S . -B cmake-build-debug -DCMAKE_BUILD_TYPE=Debug || goto :err

echo [3/5] Сборка...
cmake --build cmake-build-debug || goto :err

echo [4/5] Запуск noise_analyzer...
cmake-build-debug\noise_analyzer.exe || goto :err

echo [5/5] Запуск circle_generator...
cmake-build-debug\circle_generator.exe || goto :err

echo.
echo [УСПЕХ] Все программы выполнены. Файлы в папке output/
pause
exit /b 0

:err
echo.
echo [ОШИБКА] Процесс прерван. Проверьте вывод выше.
pause
exit /b 1