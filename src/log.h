#pragma once

#include <iostream>

#define DEBUG

#define ANSI_RESET "\033[0m"
#define ANSI_RED "\033[31m"
#define ANSI_GREEN "\033[32m"
#define ANSI_YELLOW "\033[33m"
#define ANSI_BLUE "\033[34m"
#define ANSI_BOLD "\033[1m"

// エラーログ (ファイル名と行番号付き)
#define LOG_ERROR(message)                                                                       \
    std::cerr << ANSI_BOLD ANSI_RED << "[x] (" << __FILE__ << ":" << __LINE__ << ") " << message \
              << ANSI_RESET << std::endl

// 警告ログ
#define LOG_WARN(message) std::cerr << ANSI_YELLOW "[!] " << message << ANSI_RESET << std::endl

// 成功ログ
#define LOG_SUCCESS(message) std::cerr << ANSI_GREEN << "[o] " << message << ANSI_RESET << std::endl

// 情報ログ (重要なイベント用)
#define LOG_INFO(message) std::cerr << "[-] " << message << ANSI_RESET << std::endl

#ifdef DEBUG
// デバッグログ (DEBUGが定義された時のみ有効、詳細情報用)
#define LOG_DEBUG(message) std::cerr << ANSI_BLUE "[d] " << message << ANSI_RESET << std::endl
#else
#define LOG_DEBUG(message)
#endif
