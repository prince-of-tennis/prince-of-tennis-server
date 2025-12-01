# 1. 基本設定
# -----------------
cmake_minimum_required(VERSION 3.16)
project(SuperDimensionalTennisServer CXX)

# C++17規格を必須にする
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# SDLの読み込み
find_package(SDL2 REQUIRED)

# SDL_netの読み込み
find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2_NET REQUIRED SDL2_net)

# 3. ソースファイルの定義
# -----------------
file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.c")

# 4. 実行ファイルの定義
# -----------------
add_executable(server ${SOURCES})

# 5. インクルードパスの指定 (★修正箇所)
# -----------------
target_include_directories(server PRIVATE
    "src" ,
    "libs/common/src" ,
    ${SDL2_NET_INCLUDE_DIRS} # PkgConfigで見つけたパス
)

target_link_libraries(server PRIVATE

    # --- システムライブラリ ---
    ${SDL2_LIBRARIES} # SDL2::SDL2 の代わりにこの変数を使う
    ${SDL2_NET_LIBRARIES} # SDL2_net
)
