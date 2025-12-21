# network_send()のPacket構造体専用化

## 変更の目的

`void*`を使った汎用的な`network_send()`を、型安全な`network_send_packet()`に変更しました。

## 変更内容

### 1. 関数シグネチャの変更 (`network.h`)

**変更前:**
```cpp
int network_send(TCPsocket client_socket, const void *data, int size);
```

**変更後:**
```cpp
int network_send_packet(TCPsocket client_socket, const Packet *packet);
```

### 2. 実装の改善 (`network.cpp`)

**変更前:**
```cpp
int network_send(TCPsocket client_socket, const void *data, int size)
{
    int result = SDLNet_TCP_Send(client_socket, data, size);
    if (result < size)
    {
        LOG_ERROR("送信失敗: " << SDLNet_GetError());
    }
    return result;
}
```

**変更後:**
```cpp
int network_send_packet(TCPsocket client_socket, const Packet *packet)
{
    if (!packet)
    {
        LOG_ERROR("パケットがnullptrです");
        return -1;
    }
    
    if (!client_socket)
    {
        LOG_ERROR("ソケットがnullptrです");
        return -1;
    }
    
    // Packet構造体全体を送信
    int total_sent = 0;
    int packet_size = sizeof(Packet);
    const uint8_t* data = (const uint8_t*)packet;
    
    while (total_sent < packet_size)
    {
        int sent = SDLNet_TCP_Send(client_socket, data + total_sent, packet_size - total_sent);
        
        if (sent <= 0)
        {
            LOG_ERROR("送信失敗: " << SDLNet_GetError() 
                     << " (" << total_sent << " / " << packet_size << " バイト送信済み)");
            return sent;
        }
        
        total_sent += sent;
        LOG_DEBUG("送信: " << sent << " バイト (合計: " << total_sent << " / " << packet_size << ")");
    }
    
    LOG_DEBUG("パケット送信完了: type=" << packet->type << ", size=" << packet->size 
             << " (" << total_sent << " バイト)");
    
    return total_sent;
}
```

### 3. network_broadcast()の修正

**変更前:**
```cpp
network_send(connections[i].socket, &packet, sizeof(Packet))
```

**変更後:**
```cpp
network_send_packet(connections[i].socket, &packet)
```

## メリット

### 1. **型安全性の向上**
- `void*`を使わないため、誤った型のデータを送信するミスを防止
- コンパイル時に型チェックが行われる

### 2. **サイズ指定の削除**
- `sizeof(Packet)`を毎回指定する必要がなくなり、ミスを防止
- 関数内で自動的に正しいサイズを使用

### 3. **確実な送信**
- TCP送信が部分的にしか完了しない場合を考慮
- ループで全データを送信するまで継続

### 4. **nullチェックの追加**
- パケットとソケットのnullチェックを実施
- より堅牢なエラーハンドリング

### 5. **詳細なログ出力**
- 送信進捗をデバッグログで確認可能
- エラー時に送信済みバイト数を表示

### 6. **パケット内容のログ**
- 送信完了時にtype/sizeを表示
- デバッグが容易に

## 使用例

```cpp
// Packet構造体を作成
Packet packet;
memset(&packet, 0, sizeof(Packet));
packet.type = PACKET_TYPE_PLAYER_STATE;
packet.size = sizeof(Player);
memcpy(packet.data, &player, sizeof(Player));

// 送信（サイズ指定不要）
int result = network_send_packet(client_socket, &packet);
if (result < 0) {
    // エラー処理
}
```

## 影響範囲

- `network_broadcast()`: 自動的に新しい関数を使用
- その他の送信処理: すべて`network_send_packet()`経由でPacket構造体を送信

## 今後の拡張案

1. **送信キュー**: 非同期送信のためのキュー機構
2. **圧縮**: データサイズが大きい場合の圧縮オプション
3. **暗号化**: セキュリティのための暗号化レイヤー
4. **再送制御**: 送信失敗時の自動リトライ

