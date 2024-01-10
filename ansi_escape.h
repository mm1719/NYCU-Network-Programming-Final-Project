#pragma once

#include <stdio.h>

enum Colors {
    RESET_COLOR,
    BLACK_TXT = 30,
    RED_TXT=31,
    GREEN_TXT=32,
    YELLOW_TXT=33,
    BLUE_TXT=34,
    MAGENTA_TXT=35,
    CYAN_TXT=36,
    WHITE_TXT=37,

    BLACK_BKG = 40,
    RED_BKG=41,
    GREEN_BKG=42,
    YELLOW_BKG=43,
    BLUE_BKG=44,
    MAGENTA_BKG=45,
    CYAN_BKG=46,
    WHITE_BKG=47    
};

enum ClearCodes {
    CLEAR_FROM_CURSOR_TO_END=0,
    CLEAR_FROM_CURSOR_TO_BEGIN=1,
    CLEAR_ALL=2
};

void setupConsole(void);
void restoreConsole(void);
void getCursorPosition(int *row, int *col);

static inline void setTextColor(int code) {
    printf("\x1b[%dm", code);
}

static inline void setTextColorBright(int code) {
    printf("\x1b[%d;1m", code);
}

static inline void setBackgroundColor(int code) {
    printf("\x1b[%dm", code);
}

static inline void setBackgroundColorBright(int code) {
    printf("\x1b[%d;1m", code);
}

static inline void resetColor(void) {
    printf("\x1b[%dm", RESET_COLOR);
}

static inline void clearScreen(void) {
    printf("\x1b[%dJ", CLEAR_ALL);
}

static inline void clearScreenToBottom(void) {
    printf("\x1b[%dJ", CLEAR_FROM_CURSOR_TO_END);
}

static inline void clearScreenToTop(void) {
    printf("\x1b[%dJ", CLEAR_FROM_CURSOR_TO_BEGIN);
}

static inline void clearLine(void) {
    printf("\x1b[%dK", CLEAR_ALL);
}

static inline void clearLineToRight(void) {
    printf("\x1b[%dK", CLEAR_FROM_CURSOR_TO_END);
}

static inline void clearLineToLeft(void) {
    printf("\x1b[%dK", CLEAR_FROM_CURSOR_TO_BEGIN);
}

static inline void moveUp(int positions) {
    printf("\x1b[%dA", positions);
}

static inline void moveDown(int positions) {
    printf("\x1b[%dB", positions);
}

static inline void moveRight(int positions) {
    printf("\x1b[%dC", positions);
}

static inline void moveLeft(int positions) {
    printf("\x1b[%dD", positions);
}

static inline void moveTo(int row, int col) {
    printf("\x1b[%d;%df", row, col);
}

static inline void saveCursorPosition(void) {
    printf("\x1b%d", 7);
}

static inline void restoreCursorPosition(void) {
    printf("\x1b%d", 8);
}

static inline void set_scr() {		// set screen to 80 * 25 color mode
	printf("\x1B[=3h");
};