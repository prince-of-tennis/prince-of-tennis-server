# 1. 基本設定
# -----------------
cmake_minimum_required(VERSION 3.16)
project(SuperDimensionalTennisServer C)

# C99規格を必須にする
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# 3. ソースファイルの定義
# -----------------
file(GLOB_RECURSE SOURCES "src/*.c")

# 4. 実行ファイルの定義
# -----------------
add_executable(server ${SOURCES})

# 5. インクルードパスの指定 (★修正箇所)
# -----------------
target_include_directories(server PRIVATE
    "src"
)
