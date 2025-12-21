# 通信が途中で止まる問題の修正

## 問題の症状

**途中で急に通信しなくなる**

## 原因の分析

### 1. **物理更新・送信処理が条件分岐の中にあった**

**問題のコード:**
```cpp
while (g_running != 0)
{
    int ready_count = SDLNet_CheckSockets(socket_set, 0);
    
    if (ready_count > 0)  // ← この中に全処理が入っていた
    {
        // 受信処理
        // ...
        
        // 物理更新 ← ここが実行されない！
        update_ball(&state.ball, dt);
        
        // パケット送信 ← ここも実行されない！
        network_broadcast(...);
    }
}
```

**問題点:**
- `ready_count > 0`（受信データがある場合）のみ処理が実行される
- クライアントからの入力がない場合、物理更新もパケット送信も行われない
- → **通信が止まる**

### 2. **受信・送信処理が無限ループでブロッキング**

**問題のコード:**
```cpp
int network_receive(TCPsocket client_socket, void *buffer, int size)
{
    while (total_received < size)  // ← 無限ループ
    {
        int received = SDLNet_TCP_Recv(...);
        // 全データが来るまで永久に待つ
    }
}
```

**問題点:**
- データが完全に受信できるまで永久にブロック
- ネットワークの遅延や切断でサーバーが停止
- CPU使用率100%

### 3. **タイムアウトが0ms**

```cpp
SDLNet_CheckSockets(socket_set, 0);  // ← 0ms = ノンブロッキング
```

**問題点:**
- CPU使用率が100%になる
- ビジーウェイト（無駄なループ）

## 修正内容

### 1. **物理更新・送信処理を条件外に移動**

**修正後:**
```cpp
while (g_running != 0)
{
    int ready_count = SDLNet_CheckSockets(socket_set, 10);  // 10msタイムアウト
    
    // ソケットが準備できている場合のみ通信処理
    if (ready_count > 0)
    {
        // 受信処理のみ
    }
    
    // 物理更新（毎フレーム実行）
    update_ball(&state.ball, dt);
    
    // パケット送信（毎フレーム実行）
    network_broadcast(...);
}
```

**変更点:**
- 物理更新とパケット送信を`if`ブロックの外に移動
- 受信データの有無に関わらず、毎フレーム実行される
- タイムアウトを10msに設定してCPU負荷を軽減

### 2. **受信・送信にタイムアウト機構を追加**

**修正後:**
```cpp
int network_receive(TCPsocket client_socket, void *buffer, int size)
{
    int max_attempts = 100;  // 最大試行回数
    int attempts = 0;
    
    while (total_received < size && attempts < max_attempts)
    {
        int received = SDLNet_TCP_Recv(...);
        
        if (received <= 0)
            return received;
        
        total_received += received;
        attempts++;
        
        // まだデータが必要な場合は少し待機
        if (total_received < size)
        {
            SDL_Delay(1);  // 1ms待機
        }
    }
    
    if (attempts >= max_attempts)
    {
        LOG_ERROR("受信タイムアウト");
        return -1;
    }
    
    return total_received;
}
```

**変更点:**
- 最大試行回数を設定（無限ループ防止）
- 各試行間に1ms待機（CPU負荷軽減）
- タイムアウト時はエラーログを出力して-1を返す

**送信処理も同様に修正:**
```cpp
int network_send_packet(TCPsocket client_socket, const Packet *packet)
{
    int max_attempts = 100;
    int attempts = 0;
    
    while (total_sent < packet_size && attempts < max_attempts)
    {
        int sent = SDLNet_TCP_Send(...);
        
        if (sent <= 0)
            return sent;
        
        total_sent += sent;
        attempts++;
        
        if (total_sent < packet_size)
        {
            SDL_Delay(1);
        }
    }
    
    if (attempts >= max_attempts)
    {
        LOG_ERROR("送信タイムアウト");
        return -1;
    }
    
    return total_sent;
}
```

### 3. **PacketTypeの検証を改善**

**追加:**
```cpp
// packet_type.h
enum PacketType
{
    PACKET_TYPE_NONE,
    // ... 他のタイプ
    PACKET_TYPE_MAX  // 最大値（検証用）
};
```

**使用:**
```cpp
if (packet->type >= PACKET_TYPE_MAX)
{
    LOG_WARN("不正なパケットタイプ: " << packet->type);
    return -1;
}
```

## 修正の効果

### Before（修正前）
- ❌ クライアントからの入力がないと通信が止まる
- ❌ 受信・送信で永久にブロックする可能性
- ❌ CPU使用率100%
- ❌ ネットワーク遅延で停止

### After（修正後）
- ✅ クライアントの入力に関わらず、常に物理更新・送信を実行
- ✅ タイムアウト機構で永久ブロックを防止
- ✅ CPU使用率を軽減（10ms待機）
- ✅ ネットワーク遅延に対応

## タイムアウト設定の調整

現在の設定:
- `SDLNet_CheckSockets()`: 10ms
- `network_receive()`: 最大100試行 × 1ms = 100ms
- `network_send_packet()`: 最大100試行 × 1ms = 100ms

**60FPS (16ms/frame)の場合:**
- 10ms待機 → 約62.5 FPS
- ネットワーク処理が間に合わない場合、FPSは低下する

**調整方法:**
```cpp
// より高速な応答が必要な場合
SDLNet_CheckSockets(socket_set, 5);  // 5ms

// より安定した動作が必要な場合
SDLNet_CheckSockets(socket_set, 16);  // 16ms (60FPS相当)
```

## デバッグ方法

### ログの確認

**正常な場合:**
```
[-] 受信: 1032 バイト (合計: 1032 / 1032, 試行: 1)
[-] パケット受信完了: type=1, size=24
[-] 送信: 1032 バイト (合計: 1032 / 1032, 試行: 1)
[-] パケット送信完了: type=2, size=12
```

**タイムアウトが発生している場合:**
```
[!] 部分的な受信: 512 / 1032 バイト (試行回数: 50)
[x] 受信タイムアウト: 512 / 1032 バイト受信済み
```

**不正なパケットの場合:**
```
[!] 不正なパケットタイプ: 3724758048 (最大: 7)
パケットヘッダーの16進数: de 00 ff 20 ff ff 00 00 ...
```

## 今後の改善案

1. **適応的タイムアウト**
   - ネットワーク状況に応じて動的に調整

2. **送信キュー**
   - 送信失敗時のリトライ機構

3. **ハートビート**
   - 定期的なpingパケットで接続確認

4. **エラーリカバリ**
   - 一時的な通信エラーからの自動復旧

5. **統計情報**
   - パケットロス率、遅延時間の監視

