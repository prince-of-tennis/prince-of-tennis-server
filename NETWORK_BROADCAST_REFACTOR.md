# network_broadcast()のPacket構造体化

## 変更の目的

`network_broadcast()`を`void*`ではなく`Packet`構造体を直接受け取るように変更し、型安全性を向上させ、引数を減らしました。

## 変更内容

### 1. 関数シグネチャの変更 (`network.h`)

**変更前:**
```cpp
void network_broadcast(Player players[], ClientConnection connections[], 
                      PacketType type, const void *data, size_t data_size);
```

**変更後:**
```cpp
void network_broadcast(Player players[], ClientConnection connections[], 
                      const Packet *packet);
```

### 2. 実装の簡潔化 (`network.cpp`)

**変更前:**
```cpp
void network_broadcast(Player players[], ClientConnection connections[], 
                      PacketType type, const void *data, size_t data_size)
{
    Packet packet;
    memset(&packet, 0, sizeof(Packet));

    packet.type = (uint32_t)type;
    packet.size = (uint32_t)data_size;

    if (data != nullptr && data_size > 0)
    {
        if (data_size > sizeof(packet.data))
        {
            LOG_ERROR("パケットデータが大きすぎます: " << data_size << " バイト");
            return;
        }
        memcpy(packet.data, data, data_size);
    }

    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (players[i].connected && connections[i].socket)
        {
            if (network_send_packet(connections[i].socket, &packet) < (int)sizeof(Packet))
            {
                LOG_ERROR("クライアント " << i << " への送信失敗");
            }
        }
    }
}
```

**変更後:**
```cpp
void network_broadcast(Player players[], ClientConnection connections[], 
                      const Packet *packet)
{
    if (!packet)
    {
        LOG_ERROR("パケットがnullptrです");
        return;
    }

    // パケットサイズの検証
    if (packet->size > PACKET_MAX_SIZE)
    {
        LOG_ERROR("パケットデータが大きすぎます: " << packet->size 
                 << " バイト (最大: " << PACKET_MAX_SIZE << ")");
        return;
    }

    LOG_DEBUG("ブロードキャスト: type=" << packet->type << ", size=" << packet->size);

    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (players[i].connected && connections[i].socket)
        {
            if (network_send_packet(connections[i].socket, packet) < (int)sizeof(Packet))
            {
                LOG_ERROR("クライアント " << i << " への送信失敗");
            }
        }
    }
}
```

### 3. 呼び出し側の変更 (`main.cpp`)

#### ボール座標の送信

**変更前:**
```cpp
network_broadcast(players, connections, PACKET_TYPE_BALL_STATE, 
                 &state.ball.point, sizeof(Point3d));
```

**変更後:**
```cpp
Packet ball_packet;
memset(&ball_packet, 0, sizeof(Packet));
ball_packet.type = PACKET_TYPE_BALL_STATE;
ball_packet.size = sizeof(Point3d);
memcpy(ball_packet.data, &state.ball.point, sizeof(Point3d));
network_broadcast(players, connections, &ball_packet);
```

#### プレイヤー状態の送信

**変更前:**
```cpp
network_broadcast(players, connections, PACKET_TYPE_PLAYER_STATE,
                 &state.players[i], sizeof(Player));
```

**変更後:**
```cpp
Packet player_packet;
memset(&player_packet, 0, sizeof(Packet));
player_packet.type = PACKET_TYPE_PLAYER_STATE;
player_packet.size = sizeof(Player);
memcpy(player_packet.data, &state.players[i], sizeof(Player));
network_broadcast(players, connections, &player_packet);
```

#### スコアの送信

**変更前:**
```cpp
network_broadcast(players, connections, PACKET_TYPE_SCORE_UPDATE,
                 &s_packet, sizeof(ScorePacket));
```

**変更後:**
```cpp
Packet score_packet;
memset(&score_packet, 0, sizeof(Packet));
score_packet.type = PACKET_TYPE_SCORE_UPDATE;
score_packet.size = sizeof(ScorePacket);
memcpy(score_packet.data, &s_packet, sizeof(ScorePacket));
network_broadcast(players, connections, &score_packet);
```

#### ゲームフェーズの送信

**変更前:**
```cpp
network_broadcast(players, connections, PACKET_TYPE_GAME_PHASE,
                 &state.phase, sizeof(GamePhase));
```

**変更後:**
```cpp
Packet phase_packet;
memset(&phase_packet, 0, sizeof(Packet));
phase_packet.type = PACKET_TYPE_GAME_PHASE;
phase_packet.size = sizeof(GamePhase);
memcpy(phase_packet.data, &state.phase, sizeof(GamePhase));
network_broadcast(players, connections, &phase_packet);
```

## メリット

### 1. **引数の削減**
- 5引数 → 3引数
- `type`, `data`, `data_size`が不要になった

### 2. **型安全性の向上**
- `void*`を使わないため、誤った型のデータを渡すミスを防止
- コンパイル時に型チェック可能

### 3. **コードの明確性**
- 呼び出し側でパケット構造が明示的
- パケットの内容が一目で分かる

### 4. **nullチェックの追加**
- パケットのnullチェックを実施
- より堅牢なエラーハンドリング

### 5. **デバッグログの追加**
- ブロードキャスト時にtype/sizeをログ出力
- デバッグが容易に

### 6. **パケット検証の統一**
- `network_broadcast()`内でサイズ検証
- 呼び出し側で検証不要

## デメリットと対処

### デメリット: 呼び出し側のコードが増える

**対処法: ヘルパー関数の作成**

将来的に以下のようなヘルパー関数を追加することで、さらに簡潔に書けます：

```cpp
// ヘルパー関数の例
Packet create_packet(PacketType type, const void *data, size_t size)
{
    Packet packet;
    memset(&packet, 0, sizeof(Packet));
    packet.type = type;
    packet.size = size;
    if (data && size > 0) {
        memcpy(packet.data, data, size);
    }
    return packet;
}

// 使用例
Packet ball_packet = create_packet(PACKET_TYPE_BALL_STATE, 
                                   &state.ball.point, sizeof(Point3d));
network_broadcast(players, connections, &ball_packet);
```

## 影響範囲

- `network_broadcast()`: シグネチャと実装を変更
- `main.cpp`: 4箇所の呼び出しを変更
  - ボール座標送信
  - プレイヤー状態送信
  - スコア送信
  - ゲームフェーズ送信

## ビルド結果

✅ コンパイル成功（警告のみ、エラーなし）

## 今後の改善案

1. **パケット作成ヘルパー関数**: `create_packet()`を追加
2. **テンプレート関数**: 型安全なパケット作成（C++のみ）
3. **パケットビルダー**: Builder パターンの導入
4. **マクロの活用**: 定型的なパケット作成を簡潔に

```cpp
// マクロの例（better Cスタイル）
#define CREATE_PACKET(name, pkt_type, data_ptr, data_size) \
    Packet name; \
    memset(&name, 0, sizeof(Packet)); \
    name.type = pkt_type; \
    name.size = data_size; \
    memcpy(name.data, data_ptr, data_size)

// 使用例
CREATE_PACKET(ball_packet, PACKET_TYPE_BALL_STATE, 
              &state.ball.point, sizeof(Point3d));
network_broadcast(players, connections, &ball_packet);
```

