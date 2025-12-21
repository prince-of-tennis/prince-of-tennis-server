# パケット受信のnetwork層への移行

## 変更の目的

main関数で直接行っていたパケット受信処理をnetwork層の関数に移行し、責務を明確に分離しました。

## 変更内容

### 1. 新しい関数の追加 (`network.h` / `network.cpp`)

```cpp
int network_receive_packet(TCPsocket client_socket, Packet *packet);
```

**機能:**
- Packet構造体を受信する専用関数
- 構造体のゼロクリア
- 確実な全データ受信
- パケット検証（type/sizeのバリデーション）
- デバッグログ出力
- エラーハンドリング

**実装の特徴:**

1. **自動ゼロクリア**
   ```cpp
   memset(packet, 0, sizeof(Packet));
   ```

2. **確実な受信**
   - `network_receive()`を使用して全データを確実に受信

3. **パケット検証**
   - typeの範囲チェック（> 100で不正と判定）
   - sizeの範囲チェック（> PACKET_MAX_SIZE で不正）
   - 不正なパケットは16進数ダンプを出力

4. **エラーハンドリング**
   - nullptr チェック
   - 部分受信の検出
   - 切断の検出

### 2. main.cppの変更

**変更前:**
```cpp
// 各クライアントからのデータ受信
for (int i = 0; i < MAX_CLIENTS; i++)
{
    if (players[i].connected && connections[i].socket && SDLNet_SocketReady(connections[i].socket))
    {
        Packet packet;
        memset(&packet, 0, sizeof(Packet));
        int size = network_receive(connections[i].socket, &packet, sizeof(Packet));

        if (size <= 0)
        {
            SDLNet_TCP_DelSocket(socket_set, connections[i].socket);
            network_close_client(&players[i], &connections[i]);
            continue;
        }

        // パケットサイズ検証
        if (packet.size > PACKET_MAX_SIZE)
        {
            LOG_WARN("不正なパケットサイズ: " << packet.size
                    << " (最大: " << PACKET_MAX_SIZE << " バイト, type=" << packet.type << ")");

            // デバッグ: パケットの最初の16バイトを16進数で出力
            #ifdef DEBUG
            std::cerr << "パケット内容(最初の16バイト): ";
            uint8_t* raw = (uint8_t*)&packet;
            for (int j = 0; j < 16 && j < sizeof(Packet); j++) {
                char buf[4];
                snprintf(buf, sizeof(buf), "%02x ", raw[j]);
                std::cerr << buf;
            }
            std::cerr << std::endl;
            #endif

            continue;
        }

        // パケットの種類をチェック
        PacketType pkt_type = (PacketType)packet.type;
        if (pkt_type == PACKET_TYPE_PLAYER_INPUT)
        {
            // ... 処理
        }
    }
}
```

**変更後:**
```cpp
// 各クライアントからのデータ受信
for (int i = 0; i < MAX_CLIENTS; i++)
{
    if (players[i].connected && connections[i].socket && SDLNet_SocketReady(connections[i].socket))
    {
        Packet packet;
        int size = network_receive_packet(connections[i].socket, &packet);

        if (size <= 0)
        {
            SDLNet_TCP_DelSocket(socket_set, connections[i].socket);
            network_close_client(&players[i], &connections[i]);
            continue;
        }

        // パケットの種類をチェック
        PacketType pkt_type = (PacketType)packet.type;
        if (pkt_type == PACKET_TYPE_PLAYER_INPUT)
        {
            // ... 処理
        }
    }
}
```

## メリット

### 1. **責務の分離**
- main関数: ゲームロジックに集中
- network層: 通信処理を担当

### 2. **コードの簡潔化**
- main関数から約30行のコードを削減
- パケット検証ロジックをnetwork層に集約

### 3. **再利用性の向上**
- `network_receive_packet()`は他の箇所でも使用可能
- 共通のパケット受信処理を統一

### 4. **保守性の向上**
- パケット検証ロジックの変更がnetwork層だけで完結
- デバッグログの出力箇所が明確

### 5. **エラーハンドリングの統一**
- network層で一貫したエラー処理
- main関数側は結果だけを判定すればよい

### 6. **テスト容易性**
- network層の関数を独立してテスト可能
- モックを使ったテストが容易

## 削減された処理（main.cppから削除）

1. ✅ `memset(&packet, 0, sizeof(Packet))` - network_receive_packet()内で実行
2. ✅ パケットサイズ検証 - network_receive_packet()内で実行
3. ✅ 16進数ダンプ - network_receive_packet()内で実行
4. ✅ `sizeof(Packet)` の明示的指定 - 不要

## 関数の使い方

```cpp
// パケット受信
Packet packet;
int result = network_receive_packet(client_socket, &packet);

if (result <= 0) {
    // エラーまたは切断
    return;
}

// パケット処理（検証済み）
switch ((PacketType)packet.type) {
    case PACKET_TYPE_PLAYER_INPUT:
        // 処理
        break;
    // ...
}
```

## network層の関数一覧（更新後）

### 送信
- `network_send_packet()` - Packet構造体を送信
- `network_broadcast()` - 全クライアントにPacketを送信

### 受信
- `network_receive()` - 汎用的なバイト列受信（内部使用）
- `network_receive_packet()` - Packet構造体を受信（検証付き）

### 接続管理
- `network_init_server()` - サーバー初期化
- `network_accept_client()` - クライアント接続受付
- `network_close_client()` - クライアント切断
- `network_shutdown_server()` - サーバー終了

## ビルド結果

✅ コンパイル成功（警告のみ、エラーなし）

## 今後の改善案

1. **パケット処理のコールバック化**
   ```cpp
   typedef void (*PacketHandler)(int client_id, const Packet *packet);
   void network_process_packets(PacketHandler handler);
   ```

2. **非同期受信**
   - 受信スレッドを分離
   - キューで受信パケットを管理

3. **パケット統計**
   - 受信パケット数
   - 不正パケット数
   - 帯域幅の監視

