#include "commissioning_control.h"
#include "../util/utils.h"
#include <signal.h>
#include <stdint.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/resource.h>
#endif
static bool active;
static uint64_t deadline;
static volatile sig_atomic_t cancelled;
static void (*old_int)(int);
static void (*old_term)(int);
static void on_signal(int sig) { (void)sig; cancelled = 1; }
#ifdef _WIN32
static BOOL WINAPI on_console(DWORD event) {
    if (event == CTRL_C_EVENT || event == CTRL_BREAK_EVENT || event == CTRL_CLOSE_EVENT) {
        cancelled = 1;
        return TRUE;
    }
    return FALSE;
}
#endif
void commissioning_begin(int timeout_ms) {
    active = true; cancelled = 0;
    deadline = get_timestamp_ns() + (uint64_t)timeout_ms * 1000000ULL;
    old_int = signal(SIGINT, on_signal); old_term = signal(SIGTERM, on_signal);
#ifdef _WIN32
    SetConsoleCtrlHandler(on_console, TRUE);
#else
    struct rlimit limit = {0, 0};
    setrlimit(RLIMIT_CORE, &limit);
#endif
}
void commissioning_end(void) {
#ifdef _WIN32
    SetConsoleCtrlHandler(on_console, FALSE);
#endif
    signal(SIGINT, old_int); signal(SIGTERM, old_term);
    active = false;
}
bool commissioning_active(void) { return active; }
bool commissioning_cancelled(void) { return cancelled != 0; }
int commissioning_remaining_ms(void) {
    if (!active) return -1;
    uint64_t now = get_timestamp_ns();
    if (cancelled || now >= deadline) return 0;
    return (int)((deadline - now + 999999ULL) / 1000000ULL);
}
