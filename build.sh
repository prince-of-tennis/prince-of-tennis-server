#!/bin/bash

# ビルド
rm -rf build/
cmake -B build
cmake --build build
