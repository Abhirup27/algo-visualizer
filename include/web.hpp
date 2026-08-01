#ifndef WEB_H
#define WEB_H

#include "raylib.h"
#include "utils.hpp"
#if defined(PLATFORM_WEB)
#include <emscripten/console.h>
#include <emscripten/em_js.h>
#include <emscripten/em_macros.h>
#include <emscripten/em_types.h>
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#ifdef __cplusplus
extern "C" {
#endif

EMSCRIPTEN_KEEPALIVE void js_register_algorithms(const char *);
// void toggle_console(void) __attribute__((used))
// __attribute__((visibility("default")));
void close_window(void);
void print_float(float string);
void print_string(const char *);

void set_mode(int, int);

char *list_all_files(const char *);

void print_console(const char *);
void print_console_float(float);
void onPreloadSuccess(const char *);
void onPreloadError(const char *);

void list_files();

#ifdef __cplusplus
}
#endif

#endif
#endif
