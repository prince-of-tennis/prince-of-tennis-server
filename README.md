# prince-of-tennis-server
サーバ側のリポジトリ
## 実行方法
以下のコマンドを１回だけ実行すると初期設定が完了する。  
**これは１回だけでいい。**
```bash
cmake -B build
```

ビルドする時は以下のコマンドでビルドできる。
```bash
cmake --build build
```
すると`./build/server`が生成されるので
```bash
./build/server
```
で実行できる。

また、Bashスクリプトを用意したのでそれを使用してもいい。
ビルドは
```sh
./build.sh
```
実行は
```sh
./run.sh
```
また、ビルドと実行を続けて実行するには
```sh
./build_run.sh
```

## 環境
- **OS**: Ubuntu 20.04 LTS(VMWare or 電産室)
- **使用言語**: C
- **ライブラリ**: SDL2
