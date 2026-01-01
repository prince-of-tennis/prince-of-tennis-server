#include <csignal>
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
    ctx.running = &g_running;

    // サーバー初期化
    if (!server_initialize(&ctx))
    {
        LOG_ERROR("サーバー初期化失敗");
        return 1;
    }

    // クライアント接続待機
    if (!server_wait_for_clients(&ctx))
    {
        server_cleanup(&ctx);
        return g_running ? 1 : 0;
    }

    // メインループ実行
    server_run_main_loop(&ctx);

    // クリーンアップ
    server_cleanup(&ctx);

    LOG_SUCCESS("サーバーを正常終了しました");
    return 0;
}
