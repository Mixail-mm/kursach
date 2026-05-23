cmake_minimum_required(VERSION 4.2)
project(kursach C)

set(CMAKE_C_STANDARD 11)

# Макрос передаётся в кавычках. В C сработает конкатенация: OUTPUT_DIR "/file.txt"
add_compile_definitions(OUTPUT_DIR="./output")

add_executable(noise_analyzer
        stats_lib.c
        noise_analyzer.c
)

add_executable(circle_generator
        stats_lib.c
        circle_geenrator.c
)