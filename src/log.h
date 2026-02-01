#pragma once

#include <iostream>

inline bool g_debug_log_enabled = false;

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
#define LOG_WARN(message) \
    do { if (g_debug_log_enabled) std::cerr << ANSI_YELLOW "[!] " << message << ANSI_RESET << std::endl; } while(0)

// 成功ログ
#define LOG_SUCCESS(message) \
    do { if (g_debug_log_enabled) std::cerr << ANSI_GREEN << "[o] " << message << ANSI_RESET << std::endl; } while(0)

// 情報ログ
#define LOG_INFO(message) \
    do { if (g_debug_log_enabled) std::cerr << "[-] " << message << std::endl; } while(0)

// デバッグログ
#define LOG_DEBUG(message) \
    do { if (g_debug_log_enabled) std::cerr << ANSI_BLUE "[d] " << message << ANSI_RESET << std::endl; } while(0)
