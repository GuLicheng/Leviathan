#pragma once

enum State {
    STATE_ILLEGAL,
    STATE_INITIAL,
    STATE_INT_SIGN,
    STATE_INTEGER,
    STATE_POINT,
    STATE_POINT_WITHOUT_INT,
    STATE_EXP,
    STATE_EXP_SIGN,
    STATE_FRACTION,
    STATE_EXP_NUMBER
};

enum CharType {
    CHAR_ILLEGAL,
    CHAR_NUMBER,
    CHAR_EXP,
    CHAR_POINT,
    CHAR_SIGN
};

int map[10][5] = {
    {0 ,0 ,0 ,0 ,0 },
    {0 ,3 ,0 ,5 ,2 },
    {0 ,3 ,0 ,5 ,0 },
    {0 ,3 ,6 ,4 ,0 },
    {0 ,8 ,6 ,0 ,0 },
    {0 ,8 ,0 ,0 ,0 },
    {0 ,9 ,0 ,0 ,7 },
    {0 ,9 ,0 ,0 ,0 },
    {0 ,8 ,6 ,0 ,0 },
    {0 ,9 ,0 ,0 ,0 }
};
int end[10] = {0, 0, 0, 1, 1, 0, 0, 0, 1, 1};

enum CharType toCharType(char ch) {
    if ('0' <= ch && ch <= '9') {
        return CHAR_NUMBER;
    } else if (ch == 'e' || ch == 'E') {
        return CHAR_EXP;
    } else if (ch == '.') {
        return CHAR_POINT;
    } else if (ch == '+' || ch == '-') {
        return CHAR_SIGN;
    } else {
        return CHAR_ILLEGAL;
    }
}


void init() {
    // case 1
    map[STATE_INITIAL][CHAR_NUMBER] = STATE_INTEGER;
    map[STATE_INITIAL][CHAR_SIGN] = STATE_INT_SIGN;
    map[STATE_INITIAL][CHAR_POINT] = STATE_POINT_WITHOUT_INT;

    // case 2
    map[STATE_INT_SIGN][CHAR_NUMBER] = STATE_INTEGER;
    map[STATE_INT_SIGN][CHAR_POINT] = STATE_POINT_WITHOUT_INT;

    // case 3
    map[STATE_INTEGER][CHAR_NUMBER] = STATE_INTEGER;
    map[STATE_INTEGER][CHAR_EXP] = STATE_EXP;
    map[STATE_INTEGER][CHAR_POINT] = STATE_POINT;

    map[STATE_POINT][CHAR_NUMBER] = STATE_FRACTION;
    map[STATE_POINT][CHAR_EXP] = STATE_EXP;

    map[STATE_POINT_WITHOUT_INT][CHAR_NUMBER] = STATE_FRACTION;

    map[STATE_FRACTION][CHAR_NUMBER] = STATE_FRACTION;
    map[STATE_FRACTION][CHAR_EXP] = STATE_EXP;

    map[STATE_EXP][CHAR_NUMBER] = STATE_EXP_NUMBER;
    map[STATE_EXP][CHAR_SIGN] = STATE_EXP_SIGN;

    map[STATE_EXP_SIGN][CHAR_NUMBER] = STATE_EXP_NUMBER;

    map[STATE_EXP_NUMBER][CHAR_NUMBER] = STATE_EXP_NUMBER;


    end[STATE_INTEGER] = 1;
    end[STATE_POINT] = 1;
    end[STATE_FRACTION] = 1;
    end[STATE_EXP_NUMBER] = 1;
}

int isFloating(char* s) {
    // init();
    enum State state = STATE_INITIAL;
    for (char* p = s; *p; ++p) {
        enum CharType type = toCharType(*p);
        enum State next = map[state][type];
        if (next == STATE_ILLEGAL)
            return false;
        state = next;
    }
    return end[state];
}