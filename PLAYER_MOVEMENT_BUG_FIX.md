# プレイヤーが勝手に右に動く問題の調査と修正

## 問題の症状

**プレイヤーが何も操作していないのに、勝手に右（X軸正方向）に動いていく**

## 原因の分析

### 1. **bool型のプラットフォーム依存性**

**問題:**
```cpp
struct PlayerInput
{
    bool right;
    bool left;
    bool front;
    bool back;
    bool swing;
};
```

- `bool`型のサイズはC++標準で保証されていない
- 通常は1バイトだが、コンパイラやプラットフォームによって異なる可能性
- ネットワーク通信では固定サイズの型を使うべき

### 2. **構造体パディングの問題**

**問題:**
- `PlayerInput`に`__attribute__((packed))`がなかった
- コンパイラが構造体メンバー間にパディングを挿入する可能性
- サーバーとクライアントで構造体サイズが異なる可能性

### 3. **初期化されていないデータ**

**問題:**
- クライアントがパケットを送信する際、データ部分が適切にゼロクリアされていない可能性
- ガベージ値（不定値）が`right = true`として解釈される

### 4. **Packet構造体のtypeフィールド**

**問題:**
```cpp
struct Packet
{
    PacketType type;  // enum型（サイズ不定）
    uint32_t size;
    uint8_t data[PACKET_MAX_SIZE];
};
```

- `PacketType`は`enum`型でサイズが不定
- `uint32_t`として扱うべき

## 修正内容

### 1. **bool型をuint8_tに変更**

**変更前:**
```cpp
struct PlayerInput
{
    bool right;
    bool left;
    bool front;
    bool back;
    bool swing;
};
```

**変更後:**
```cpp
struct PlayerInput
{
    uint8_t right;  // 0 or 1
    uint8_t left;   // 0 or 1
    uint8_t front;  // 0 or 1
    uint8_t back;   // 0 or 1
    uint8_t swing;  // 0 or 1
} __attribute__((packed));
```

**メリット:**
- プラットフォーム非依存（常に1バイト）
- サーバーとクライアントでサイズが一致
- ネットワーク通信で安全

### 2. **Packet構造体のtype フィールドを修正**

**変更前:**
```cpp
struct Packet
{
    PacketType type;  // enum型
    // ...
};
```

**変更後:**
```cpp
struct Packet
{
    uint32_t type;  // 固定4バイト
    // ...
} __attribute__((packed));
```

### 3. **デバッグログの追加**

**追加した項目:**
```cpp
// 起動時
LOG_DEBUG("sizeof(PlayerInput) = " << sizeof(PlayerInput) << " バイト");

// パケット受信時
LOG_DEBUG("クライアント " << i << " 入力: right=" << input.right 
         << ", left=" << input.left << ", front=" << input.front 
         << ", back=" << input.back << ", swing=" << input.swing);

// 毎フレーム
LOG_DEBUG("プレイヤー0座標: (" << state.players[0].point.x << ", "
          << state.players[0].point.y << ", " << state.players[0].point.z << ")");
LOG_DEBUG("プレイヤー1座標: (" << state.players[1].point.x << ", "
          << state.players[1].point.y << ", " << state.players[1].point.z << ")");
```

## 検証方法

### 1. **構造体サイズの確認**

サーバー起動時のログを確認：
```
[-] === 構造体サイズ情報 ===
[-] sizeof(Packet) = 4104 バイト
[-] sizeof(Player) = 52 バイト
[-] sizeof(Point3d) = 12 バイト
[-] sizeof(PlayerInput) = 5 バイト
[-] PACKET_MAX_SIZE = 4096 バイト
```

**期待される値:**
- `sizeof(PlayerInput) = 5` バイト（uint8_t × 5）

### 2. **入力ログの確認**

何も操作していない場合：
```
[-] クライアント 0 入力: right=0, left=0, front=0, back=0, swing=0
```

もし`right=1`などが表示される場合：
- クライアント側の実装に問題がある
- パケットのデータ部分が適切にゼロクリアされていない

### 3. **座標の確認**

プレイヤーが動いている場合：
```
[-] プレイヤー0座標: (0.0, 0.0, 24.9674)  # 初期位置
[-] プレイヤー0座標: (0.08, 0.0, 24.9674) # 右に移動！
[-] プレイヤー0座標: (0.16, 0.0, 24.9674) # さらに右に移動！
```

## クライアント側で必要な対応

クライアント側でも同じ修正が必要です：

### 1. **PlayerInput構造体の修正**

```cpp
// クライアント側のplayer_input.h
struct PlayerInput
{
    uint8_t right;
    uint8_t left;
    uint8_t front;
    uint8_t back;
    uint8_t swing;
} __attribute__((packed));
```

### 2. **パケット送信前のゼロクリア**

```cpp
// パケット作成
Packet packet;
memset(&packet, 0, sizeof(Packet));  // 重要！全体をゼロクリア
packet.type = PACKET_TYPE_PLAYER_INPUT;
packet.size = sizeof(PlayerInput);

// 入力データをコピー
PlayerInput input = {0};  // まずゼロ初期化
input.right = (is_key_pressed(KEY_RIGHT)) ? 1 : 0;
input.left = (is_key_pressed(KEY_LEFT)) ? 1 : 0;
// ... 他の入力

memcpy(packet.data, &input, sizeof(PlayerInput));

// 送信
network_send(&packet);
```

### 3. **Packet構造体の修正**

```cpp
// クライアント側のpacket.h
struct Packet
{
    uint32_t type;  // bool や enum ではなく uint32_t
    uint32_t size;
    uint8_t data[PACKET_MAX_SIZE];
} __attribute__((packed));
```

## トラブルシューティング

### 問題: まだプレイヤーが勝手に動く

**原因の可能性:**
1. クライアント側の修正が完了していない
2. パケットのデータ部分が初期化されていない
3. ネットワークバッファにゴミデータが残っている

**対処法:**
1. クライアント側も同じ修正を適用
2. パケット送信前に`memset(&packet, 0, sizeof(Packet))`を確実に実行
3. サーバーとクライアントを再起動

### 問題: 入力が反応しない

**原因の可能性:**
1. `uint8_t`を`bool`として扱っている
2. 0/1以外の値が入っている

**対処法:**
```cpp
// 入力判定時
if (input.right != 0)  // 0以外なら true
{
    // 右に移動
}
```

## まとめ

### 修正前の問題
- ❌ `bool`型でプラットフォーム依存
- ❌ 構造体パディングで予期しないサイズ
- ❌ ガベージ値が`right = true`として解釈

### 修正後
- ✅ `uint8_t`でプラットフォーム非依存
- ✅ `__attribute__((packed))`でパディング無効化
- ✅ デバッグログで問題を追跡可能
- ✅ サーバーとクライアントで構造体サイズが一致

## 注意事項

**クライアント側も同じ修正を適用しないと、通信が正常に動作しません！**

1. `PlayerInput`を`uint8_t`に変更
2. `Packet`の`type`を`uint32_t`に変更
3. すべての構造体に`__attribute__((packed))`を追加
4. パケット送信前に`memset`でゼロクリア

