#include <signal.h>
#include <unistd.h>

#include "log.h"
#include "core/server_init.h"
#include "core/server_loop.h"

// グローバル変数: Ctrl+C対応
// シグナルハンドラーから参照するため、グローバルに配置
volatile int g_running = 1;

// シグナルハンドラー
void signal_handler(int signum)
{
    if (signum == SIGINT)
    {
        // 非同期シグナル安全な処理のみ行う（LOG_* は使用しない）
        const char msg[] = "\nCtrl+C検出: サーバーを終了します...\n";
        write(STDERR_FILENO, msg, sizeof(msg) - 1);
        g_running = 0;
    }
}


int main(int argc, char *argv[])
{
    // シグナルハンドラーを設定
    signal(SIGINT, signal_handler);

    // サーバーコンテキスト初期化
    ServerContext ctx;

    // サーバー初期化
    if (!server_initialize(&ctx))
    {
        LOG_ERROR("サーバー初期化失敗");
        return 1;
    }

    // runningフラグを設定（server_initialize内のmemsetの後に設定する必要がある）
    ctx.running = &g_running;

    // メインループ（ゲーム終了後に再待機）
    while (g_running)
    {
        // クライアント接続待機
        if (!server_wait_for_clients(&ctx))
        {
            if (!g_running)
            {
                // Ctrl+C で中断された場合
                break;
            }
            // エラーの場合は終了
            server_cleanup(&ctx);
            return 1;
        }

        // メインループ実行
        server_run_main_loop(&ctx);

        // ゲーム終了後、Ctrl+C でなければリセットして再待機
        if (ctx.state.match_result_sent && ctx.state.phase == GAME_PHASE_GAME_FINISHED)
        {
            server_reset_for_new_game(&ctx);
            // running を1に戻す（server_run_main_loop で0になっている可能性）
            g_running = 1;
        }
    }

    // クリーンアップ
    server_cleanup(&ctx);

    LOG_SUCCESS("サーバーを正常終了しました");
    return 0;
}
