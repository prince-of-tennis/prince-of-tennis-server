# Prince of Tennis Server - シーケンス図

このドキュメントは、テニスゲームサーバーの動作フローを示すシーケンス図です。

## ゲーム全体のシーケンス図

```mermaid
sequenceDiagram
    participant Server as サーバー
    participant Client1 as クライアント1
    participant Client2 as クライアント2
    participant GameState as ゲーム状態
    participant PhaseManager as フェーズ管理

    Note over Server: サーバー起動
    Server->>Server: network_init_server(5000)
    Server->>GameState: init_game()
    Server->>PhaseManager: init_phase_manager()
    Note over PhaseManager: GAME_PHASE_WAIT_FOR_MATCH

    Note over Server: クライアント接続待ち
    Server->>Server: wait_for_clients()
    Client1->>Server: 接続
    Server->>Client1: 接続受付 (slot 0)
    Client2->>Server: 接続
    Server->>Client2: 接続受付 (slot 1)
    
    Note over Server: 必要人数(2人)が揃った
    Server->>PhaseManager: update_game_phase(MATCH_COMPLETE)
    Note over PhaseManager: GAME_PHASE_MATCH_COMPLETE<br/>3秒待機

    Note over PhaseManager: タイマー経過後
    PhaseManager->>PhaseManager: update_game_phase(START_GAME)
    Note over PhaseManager: GAME_PHASE_START_GAME<br/>サーブ待機状態

    Server->>Client1: PACKET_TYPE_GAME_PHASE(START_GAME)
    Server->>Client2: PACKET_TYPE_GAME_PHASE(START_GAME)

    Note over Client1: サーブ動作
    Client1->>Server: PACKET_TYPE_PLAYER_INPUT(swing=true)
    Server->>GameState: apply_player_input(player_id=0)
    GameState->>GameState: handle_racket_hit()
    GameState->>PhaseManager: update_game_phase(IN_RALLY)
    Note over PhaseManager: GAME_PHASE_IN_RALLY<br/>ラリー中

    Server->>Client1: PACKET_TYPE_GAME_PHASE(IN_RALLY)
    Server->>Client2: PACKET_TYPE_GAME_PHASE(IN_RALLY)

    loop ラリー中
        Server->>Server: update_ball(dt)
        Server->>Client1: PACKET_TYPE_BALL_STATE
        Server->>Client2: PACKET_TYPE_BALL_STATE
        
        Client2->>Server: PACKET_TYPE_PLAYER_INPUT(move/swing)
        Server->>GameState: apply_player_input(player_id=1)
        
        Server->>Client1: PACKET_TYPE_PLAYER_STATE
        Server->>Client2: PACKET_TYPE_PLAYER_STATE
        
        Server->>Server: handle_bounce()
        
        alt ボールが1回バウンド＆アウト
            Note over Server: 打った側のミス
            Server->>GameState: add_point(winner_id)
            Server->>PhaseManager: update_game_phase(POINT_SCORED)
        else ボールが2回バウンド
            Note over Server: 打った側の得点
            Server->>GameState: add_point(winner_id)
            Server->>PhaseManager: update_game_phase(POINT_SCORED)
        end
    end

    Note over PhaseManager: GAME_PHASE_POINT_SCORED<br/>3秒待機

    Server->>Client1: PACKET_TYPE_GAME_PHASE(POINT_SCORED)
    Server->>Client2: PACKET_TYPE_GAME_PHASE(POINT_SCORED)
    Server->>Client1: PACKET_TYPE_SCORE_UPDATE
    Server->>Client2: PACKET_TYPE_SCORE_UPDATE

    alt 試合継続
        PhaseManager->>PhaseManager: update_game_phase(START_GAME)
        Note over PhaseManager: 次のサーブへ
    else 試合終了
        Server->>GameState: match_finished()
        PhaseManager->>PhaseManager: update_game_phase(GAME_FINISHED)
        Note over PhaseManager: GAME_PHASE_GAME_FINISHED<br/>5秒待機
        Server->>Client1: PACKET_TYPE_GAME_PHASE(GAME_FINISHED)
        Server->>Client2: PACKET_TYPE_GAME_PHASE(GAME_FINISHED)
        Note over Server: 5秒後にサーバー終了
        Server->>Server: exit(0)
    end
```

## 主要なコンポーネント

### ゲームフェーズ (GamePhase)

1. **GAME_PHASE_WAIT_FOR_MATCH**: プレイヤー接続待ち
2. **GAME_PHASE_MATCH_COMPLETE**: マッチング完了（3秒待機）
3. **GAME_PHASE_START_GAME**: サーブ待機状態
4. **GAME_PHASE_IN_RALLY**: ラリー中
5. **GAME_PHASE_POINT_SCORED**: 得点後（3秒待機）
6. **GAME_PHASE_GAME_FINISHED**: 試合終了（5秒待機後サーバー終了）

### パケットタイプ

- **PACKET_TYPE_PLAYER_INPUT**: クライアントからの入力（移動、スイング）
- **PACKET_TYPE_BALL_STATE**: ボールの座標情報
- **PACKET_TYPE_PLAYER_STATE**: プレイヤーの座標情報
- **PACKET_TYPE_GAME_PHASE**: ゲームフェーズの変更通知
- **PACKET_TYPE_SCORE_UPDATE**: スコア更新通知

### 得点判定ロジック

ラリー中 (GAME_PHASE_IN_RALLY) のみ判定が行われます：

1. **1回バウンド＆コートアウト**: 打った側のミス → 相手の得点
2. **2回バウンド（ツーバウンド）**: 返せなかった → 打った側の得点

### タイマー設定

- **TIME_MATCH_COMPLETE**: 2秒 - マッチング完了後の待機時間
- **TIME_AFTER_POINT**: 3秒 - 得点後から次のサーブまでの時間
- **TIME_GAME_FINISHED**: 5秒 - 試合終了後のサーバーシャットダウンまでの時間
