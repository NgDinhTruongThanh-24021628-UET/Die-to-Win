#pragma once

enum Color { PINK=0, BLUE, YELLOW, DARK, TOTAL_COLOR };
extern Color selectedColor;

enum Background { BLANK=0, STRIPE, TETRIS, TOTAL_BG };
extern Background selectedBG;

// Game settings
enum GameSetting {
    SETTING_BG=0,
    SETTING_COLOR,
    TOTAL_SETTING
};
extern GameSetting currentSetting;

// Game status
enum GameStatus {
    MENU=0,
    START,
    PLAYING,
    SETTINGS,
    CREDITS,
    WIN,
    RESTART,
    STATUS_COUNT,
    TEST
};
extern GameStatus currentStatus;
