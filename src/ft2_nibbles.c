/* directly ported from FT2 source code (except some routines) */

#include <stdint.h>
#include <stdio.h>
#include <math.h> /* round() */
#include "ft2_header.h"
#include "ft2_keyboard.h"
#include "ft2_config.h"
#include "ft2_video.h"
#include "ft2_gui.h"
#include "ft2_help.h"
#include "ft2_pattern_ed.h"
#include "ft2_gfxdata.h"

#define NI_MAXLEVEL 30

static const char *NI_HelpText[] =
{
    "Player 1 uses cursor keys to control movement.",
    "Player 2 uses the following keys:",
    "",
    "                  (W=Up)",
    "  (A=Left) (S=Down) (D=Right)",
    "",
    "The \"Wrap\" option controls whether it's possible to walk through",
    "the screen edges or not. Turn it on and use your brain to get",
    "the maximum out of this feature.",
    "The \"Surround\" option turns nibbles into a completely different",
    "game. Don't change this option during play! (You'll see why)",
    "We wish you many hours of fun playing this game."
};
#define NIBBLES_HELP_LINES (sizeof (NI_HelpText) / sizeof (char *))

typedef struct
{
    int16_t antal;
    uint8_t data[8];
} nibbleBufferTyp;

typedef struct
{
    uint8_t x, y;
} nibbleCrd;

static const char nibblesCheatCode1[] = "skip", nibblesCheatCode2[] = "triton";
static char nibblesCheatBuffer[16], tempPlayer1Name[22], tempPlayer2Name[22];

const char convHexTable2[10] = { 7, 8, 9, 10, 11, 12, 13, 16, 17, 18 };
static const uint8_t NI_Speeds[4] = { 12, 8, 6, 4 };
static uint8_t NI_CheatIndex, NI_EternalLives, NI_CurSpeed, NI_CurTick60Hz, NI_CurSpeed60Hz, NI_Screen[51][23], NI_Level;
static uint8_t p1Died, p2Died, p1GameOver, p2GameOver;
static int16_t NI_P1Dir, NI_P2Dir, NI_P1Len, NI_P2Len, NI_Number, NI_NumberX, NI_NumberY, NI_P1NoRens, NI_P2NoRens;
static uint16_t NI_P1Lives, NI_P2Lives;
static int32_t NI_P1Score, NI_P2Score;
static nibbleCrd NI_P1[256], NI_P2[256];
static nibbleBufferTyp nibblesBuffer[2];

static void nibblesTextOutClipped(uint16_t x, uint16_t y, uint8_t paletteIndex, char *textPtr, uint16_t clipX)
{
    char ch;
    uint16_t i, currX;

    MY_ASSERT(textPtr != NULL)

    currX = x;
    for (i = 0; i < 22; ++i)
    {
        if (textPtr[i] == '\0')
            break;

        ch = relocateChars(textPtr[i], FONT_TYPE1);
        charOutClipped(currX, y, paletteIndex, ch, clipX);

        currX += font1Widths[(uint32_t)(ch)];
        if (currX >= clipX)
            break;
    }
}

static void nibblesTextOutShadowClipped(uint16_t x, uint16_t y, uint8_t paletteIndex, uint8_t shadowPaletteIndex, char *textPtr, uint16_t clipX)
{
    /* clipping is done in nibblesTextOutClipped() */
    nibblesTextOutClipped(x + 1, y + 1, shadowPaletteIndex, textPtr, clipX); /* shadow */
    nibblesTextOutClipped(x + 0, y + 0, paletteIndex,       textPtr, clipX); /* foreground */
}

static void redrawNibblesScreen(void)
{
    uint8_t x, y, c;
    int16_t xs, ys;

    if (!editor.NI_Play)
        return;

    for (x = 0; x < 51; ++x)
    {
        for (y = 0; y < 23; ++y)
        {
            xs = 152 + (x * 8);
            ys = 7   + (y * 7);

            c = NI_Screen[x][y];
            if (c < 16)
            {
                if (config.NI_Grid)
                {
                    fillRect(xs + 0, ys + 0, 8 - 0, 7 - 0, PAL_BUTTON2);
                    fillRect(xs + 1, ys + 1, 8 - 1, 7 - 1, c);
                }
                else
                {
                    fillRect(xs, ys, 8, 7, c);
                }
            }
            else
            {
                charOutFast(xs + 2, ys, PAL_FORGRND, convHexTable2[NI_Number]);
            }
        }
    }

    /* fix wrongly rendered grid */
    if (config.NI_Grid)
    {
        vLine(560,   7, 161, PAL_BUTTON2);
        hLine(152, 168, 409, PAL_BUTTON2);
    }
    else
    {
        /* if we turned grid off, clear lines */
        vLine(560,   7, 161, PAL_BCKGRND);
        hLine(152, 168, 409, PAL_BCKGRND);
    }
}

static void nibblesAddBuffer(int16_t nr, uint8_t typ)
{
    nibbleBufferTyp *n;

    n = &nibblesBuffer[nr];
    if (n->antal < 8)
    {
        n->data[n->antal] = typ;
        n->antal++;
    }
}

static uint8_t nibblesBufferFull(int16_t nr)
{
    return (nibblesBuffer[nr].antal > 0);
}

static int16_t nibblesGetBuffer(int16_t nr)
{
    int16_t dataOut;
    nibbleBufferTyp *n;

    n = &nibblesBuffer[nr];
    if (n->antal > 0)
    {
        dataOut = n->data[0];
        memmove(&n->data[0], &n->data[1], 7);
        n->antal--;

        return (dataOut);
    }

    return (-1);
}

static void nibblesGetLevel(int16_t nr)
{
    int16_t readX, readY, x, y;

    readX = 1 + ((51 + 2) * (nr % 10));
    readY = 1 + ((23 + 2) * (nr / 10));

    for (x = 0; x < 51; ++x)
    {
        for (y = 0; y < 23; ++y)
            NI_Screen[x][y] = nibblesStages[((readY + y) * 530) + (readX + x)];
    }
}

static void nibblesCreateLevel(int16_t nr)
{
    uint8_t c;
    int16_t i, x, y, x1, y1, x2, y2;

    if (nr >= NI_MAXLEVEL)
        nr  = NI_MAXLEVEL - 1;

    nibblesGetLevel(nr);

    x1 = 0; x2 = 0;
    y1 = 0; y2 = 0;

    for (y = 0; y < 23; ++y)
    {
        for (x = 0; x < 51; ++x)
        {
            if ((NI_Screen[x][y] == 1) || (NI_Screen[x][y] == 3))
            {
                c = NI_Screen[x][y];

                if (c == 3)
                {
                    x1 = x;
                    y1 = y;
                }

                if (c == 1)
                {
                    x2 = x;
                    y2 = y;
                }

                NI_Screen[x][y] = 0;
            }
        }
    }

    x = (51 + 2) * (nr % 10);
    y = (23 + 2) * (nr / 10);

    NI_P1Dir = nibblesStages[(y * 530) + (x + 1)];
    NI_P2Dir = nibblesStages[(y * 530) + (x + 0)];

    NI_P1Len = 5;
    NI_P2Len = 5;
    NI_P1NoRens = 0;
    NI_P2NoRens = 0;
    NI_Number = 0;
    nibblesBuffer[0].antal = 0;
    nibblesBuffer[1].antal = 0;

    for (i = 0; i < 256; ++i)
    {
        NI_P1[i].x = (uint8_t)(x1);
        NI_P1[i].y = (uint8_t)(y1);
        NI_P2[i].x = (uint8_t)(x2);
        NI_P2[i].y = (uint8_t)(y2);
    }
}

static void nibbleWriteLevelSprite(int16_t xOut, int16_t yOut, int16_t nr)
{
    uint8_t *src;
    uint16_t x, y, readX, readY;
    uint32_t *dst;

    readX = (51 + 2) * (nr % 10);
    readY = (23 + 2) * (nr / 10);

    src = (uint8_t *)(&nibblesStages[(readY * 530) + readX]);
    dst = &video.frameBuffer[(yOut * SCREEN_W) + xOut];

    for (y = 0; y < (23 + 2); ++y)
    {
        for (x = 0; x < (51 + 2); ++x)
            *dst++ = video.palette[*src++]; /* this is safe, data never exceeds palette length */

        src += (530      - (51 + 2));
        dst += (SCREEN_W - (51 + 2));
    }

    /* overwrite start position pixels */
    video.frameBuffer[(yOut * SCREEN_W) + (xOut + 0)] = video.palette[PAL_FORGRND];
    video.frameBuffer[(yOut * SCREEN_W) + (xOut + 1)] = video.palette[PAL_FORGRND];
}

void nibblesHighScore(void)
{
    int16_t i;

    if (editor.NI_Play)
    {
        sysReqQueue(SR_NIB_NO_HIGHS);
        return;
    }

    clearRect(152, 7, 409, 162);

    textBigOut(160, 10, PAL_FORGRND, "Fasttracker Nibbles Highscore");
    for (i = 0; i < 5; ++i)
    {
        nibblesTextOutShadowClipped(160, 42 + (26 * i), PAL_FORGRND, PAL_DSKTOP2, &config.NI_HighScore[i].name[1], 160 + 70);
        hexOutShadow(160 + 76, 42 + (26 * i), PAL_FORGRND, PAL_DSKTOP2, config.NI_HighScore[i].score, 8);
        nibbleWriteLevelSprite(160 + 136, (42 - 9) + (26 * i), config.NI_HighScore[i].level);

        nibblesTextOutShadowClipped(360, 42 + (26 * i), PAL_FORGRND, PAL_DSKTOP2, &config.NI_HighScore[i + 5].name[1], 360 + 70);
        hexOutShadow(360 + 76, 42 + (26 * i), PAL_FORGRND, PAL_DSKTOP2, config.NI_HighScore[i + 5].score, 8);
        nibbleWriteLevelSprite(360 + 136, (42 - 9) + (26 * i), config.NI_HighScore[i + 5].level);
    }
}

static void setNibbleDot(uint8_t x, uint8_t y, uint8_t c)
{
    uint16_t xs, ys;

    xs = 152 + (x * 8);
    ys = 7   + (y * 7);

    if (config.NI_Grid)
    {
        fillRect(xs + 0, ys + 0, 8 - 0, 7 - 0, PAL_BUTTON2);
        fillRect(xs + 1, ys + 1, 8 - 1, 7 - 1, c);
    }
    else
    {
        fillRect(xs, ys, 8, 7, c);
    }

    NI_Screen[x][y] = c;
}

static void nibblesGenNewNumber(void)
{
    int16_t x, y, xs, ys;

    while (true)
    {
        x = rand() % 51;
        y = rand() % 23;

        if ((NI_Screen[x][y] == 0) && (NI_Screen[x][y + 1] == 0))
        {
            NI_Number++;
            NI_Screen[x][y] = (uint8_t)(16 + NI_Number);
            NI_NumberX = x;
            NI_NumberY = y;

            xs = 152 + (x * 8);
            ys = 7   + (y * 7);

            if (config.NI_Grid)
            {
                fillRect(xs + 0, ys + 0, 8 - 0, 7 - 0, PAL_BUTTON2);
                fillRect(xs + 1, ys + 1, 8 - 1, 7 - 1, PAL_BCKGRND);
            }
            else
            {
                fillRect(xs, ys, 8, 7, PAL_BCKGRND);
            }

            charOutFast((x * 8) + 154, (y * 7) + 7, PAL_FORGRND, convHexTable2[NI_Number]);

            break;
        }
    }
}

static void newNibblesGame(void)
{
    nibblesCreateLevel(NI_Level);
    redrawNibblesScreen();

    setNibbleDot(NI_P1[0].x, NI_P1[0].y, 6);
    if (config.NI_AntPlayers == 1)
        setNibbleDot(NI_P2[0].x, NI_P2[0].y, 7);

    if (!config.NI_Surround)
        nibblesGenNewNumber();
}

void nibblesPlayerDiedOK(void) /* called from sys. req. */
{
    hideSystemRequest();

    p1Died = false;
    p2Died = false;

    if (!p1GameOver && !p2GameOver)
        newNibblesGame();
}

static uint8_t nibblesInvalid(int16_t x, int16_t y, int16_t d)
{
    if (!config.NI_Wrap)
    {
        if (((x == 0) && (d == 0)) || ((x == 50) && (d == 2)) ||
            ((y == 0) && (d == 3)) || ((y == 22) && (d == 1)))
        {
            return (true);
        }
    }

    MY_ASSERT((x >= 0) && (x < 51) && (y >= 0) && (y < 23))
    return (((NI_Screen[x][y] >= 1) && (NI_Screen[x][y] <= 15)));
}

static void drawScoresLives(void)
{
    /* player 1 */
    fillRect(89, 27, 55, 8, PAL_DESKTOP);
    hexOut(89, 27, PAL_FORGRND, NI_P1Score, 8);

    MY_ASSERT(NI_P1Lives < 100)

    fillRect(131, 39, 13, 8, PAL_DESKTOP);
    charOutFast(131 + (0 * 7), 39, PAL_FORGRND, '0' + ((uint8_t)(NI_P1Lives) / 10));
    charOutFast(131 + (1 * 7), 39, PAL_FORGRND, '0' + ((uint8_t)(NI_P1Lives) % 10));

    /* player 2 */
    fillRect(89, 75, 55, 8, PAL_DESKTOP);
    hexOut(89, 75, PAL_FORGRND, NI_P2Score, 8);

    MY_ASSERT(NI_P2Lives < 100)

    fillRect(131, 87, 13, 8, PAL_DESKTOP);
    charOutFast(131 + (0 * 7), 87, PAL_FORGRND, '0' + ((uint8_t)(NI_P2Lives) / 10));
    charOutFast(131 + (1 * 7), 87, PAL_FORGRND, '0' + ((uint8_t)(NI_P2Lives) % 10));
}

static void nibblesDecLives(int16_t l1, int16_t l2)
{
    if (!NI_EternalLives)
    {
        NI_P1Lives -= l1;
        NI_P2Lives -= l2;
    }

    drawScoresLives();

    if ((l1 + l2) == 2)
    {
        p1Died = true;
        p2Died = true;
    }
    else
    {
        if (l2 == 0)
            p1Died = true;
        else
            p2Died = true;
    }

    if (NI_P1Lives == 0) p1GameOver = true;
    if (NI_P2Lives == 0) p2GameOver = true;

    if (p1Died && p2Died)
    {
        sysReqQueue(SR_NIB_BOTH_PLAYERS_DIED);
    }
    else
    {
        if (p1Died)
            sysReqQueue(SR_NIB_PLAYER1_DIED);
        else if (p2Died)
            sysReqQueue(SR_NIB_PLAYER2_DIED);
    }

    if (p1GameOver || p2GameOver)
        sysReqQueue(SR_NIB_GAME_OVER);
}

static void nibblesEraseNumber(void)
{
    if (!config.NI_Surround)
        setNibbleDot((uint8_t)(NI_NumberX), (uint8_t)(NI_NumberY), 0);
}

static void nibblesNewLevel(void)
{
    sprintf(editor.ui.nibblesLvlText, "Level %d finished!", NI_Level + 1);
    sysReqQueue(SR_NIB_STAGE_FINISHED);
}

void nibblesLevelFinishedOK(void) /* called from sys. req. */
{
    hideSystemRequest();

    /* cast to int16_t to simulate a bug in FT2 */
    NI_P1Score += (0x10000 + (int16_t)((12 - NI_CurSpeed) * 0x2000));
    if (config.NI_AntPlayers == 1)
        NI_P2Score += 0x10000;

    NI_Level++;

    if (NI_P1Lives < 99)
        NI_P1Lives++;

    if (config.NI_AntPlayers == 1)
    {
        if (NI_P2Lives < 99)
            NI_P2Lives++;
    }

    NI_Number = 0;
    nibblesCreateLevel(NI_Level);
    redrawNibblesScreen();

    nibblesGenNewNumber();
}

void moveNibblePlayers(void)
{
    int16_t i, j;

    if (editor.ui.systemRequestShown)
        return;

    NI_CurTick60Hz--;
    if (NI_CurTick60Hz != 0)
        return;

    if (nibblesBufferFull(0))
    {
        switch (nibblesGetBuffer(0))
        {
            case 0: { if (NI_P1Dir != 2) NI_P1Dir = 0; } break;
            case 1: { if (NI_P1Dir != 3) NI_P1Dir = 1; } break;
            case 2: { if (NI_P1Dir != 0) NI_P1Dir = 2; } break;
            case 3: { if (NI_P1Dir != 1) NI_P1Dir = 3; } break;
            default: break;
        }
    }

    if (nibblesBufferFull(1))
    {
        switch (nibblesGetBuffer(1))
        {
            case 0: { if (NI_P2Dir != 2) NI_P2Dir = 0; } break;
            case 1: { if (NI_P2Dir != 3) NI_P2Dir = 1; } break;
            case 2: { if (NI_P2Dir != 0) NI_P2Dir = 2; } break;
            case 3: { if (NI_P2Dir != 1) NI_P2Dir = 3; } break;
            default: break;
        }
    }

    memmove(&NI_P1[1], &NI_P1[0], 255 * sizeof (nibbleCrd));
    if (config.NI_AntPlayers == 1)
        memmove(&NI_P2[1], &NI_P2[0], 255 * sizeof (nibbleCrd));

    switch (NI_P1Dir)
    {
        case 0: NI_P1[0].x++; break;
        case 1: NI_P1[0].y--; break;
        case 2: NI_P1[0].x--; break;
        case 3: NI_P1[0].y++; break;
        default: break;
    }

    if (config.NI_AntPlayers == 1)
    {
        switch (NI_P2Dir)
        {
            case 0: NI_P2[0].x++; break;
            case 1: NI_P2[0].y--; break;
            case 2: NI_P2[0].x--; break;
            case 3: NI_P2[0].y++; break;
            default: break;
        }
    }

    if (NI_P1[0].x == 255) NI_P1[0].x = 50;
    if (NI_P2[0].x == 255) NI_P2[0].x = 50;
    if (NI_P1[0].y == 255) NI_P1[0].y = 22;
    if (NI_P2[0].y == 255) NI_P2[0].y = 22;

    NI_P1[0].x %= 51;
    NI_P1[0].y %= 23;
    NI_P2[0].x %= 51;
    NI_P2[0].y %= 23;

    if (config.NI_AntPlayers == 1)
    {
        if (nibblesInvalid(NI_P1[0].x, NI_P1[0].y, NI_P1Dir) &&
            nibblesInvalid(NI_P2[0].x, NI_P2[0].y, NI_P2Dir))
        {
            nibblesDecLives(1, 1); goto NoMove;
        }
        else if (nibblesInvalid(NI_P1[0].x, NI_P1[0].y, NI_P1Dir))
        {
            nibblesDecLives(1, 0); goto NoMove;
        }
        else if (nibblesInvalid(NI_P2[0].x, NI_P2[0].y, NI_P2Dir))
        {
            nibblesDecLives(0, 1); goto NoMove;
        }
        else if ((NI_P1[0].x == NI_P2[0].x) && (NI_P1[0].y == NI_P2[0].y))
        {
            nibblesDecLives(1, 1); goto NoMove;
        }
    }
    else
    {
        if (nibblesInvalid(NI_P1[0].x, NI_P1[0].y, NI_P1Dir))
        {
            nibblesDecLives(1, 0);
            goto NoMove;
        }
    }

    j = 0;
    i = NI_Screen[NI_P1[0].x][NI_P1[0].y];
    if (i >= 16)
    {
        NI_P1Score += ((i & 15) * 999 * (NI_Level + 1));
        nibblesEraseNumber(); j = 1;
        NI_P1NoRens = NI_P1Len / 2;
    }

    if (config.NI_AntPlayers == 1)
    {
        i = NI_Screen[NI_P2[0].x][NI_P2[0].y];
        if (i >= 16)
        {
            NI_P2Score += ((i & 15) * 999 * (NI_Level + 1));
            nibblesEraseNumber(); j = 1;
            NI_P2NoRens = NI_P2Len / 2;
        }
    }

    NI_P1Score -= 17;
    if (config.NI_AntPlayers == 1)
        NI_P2Score -= 17;

    if (NI_P1Score < 0) NI_P1Score = 0;
    if (NI_P2Score < 0) NI_P2Score = 0;

    if (!config.NI_Surround)
    {
        if ((NI_P1NoRens > 0) && (NI_P1Len < 255))
        {
            NI_P1NoRens--;
            NI_P1Len++;
        }
        else
        {
            setNibbleDot(NI_P1[NI_P1Len].x, NI_P1[NI_P1Len].y, 0);
        }

        if (config.NI_AntPlayers == 1)
        {
            if ((NI_P2NoRens > 0) && (NI_P2Len < 255))
            {
                NI_P2NoRens--;
                NI_P2Len++;
            }
            else
            {
                setNibbleDot(NI_P2[NI_P2Len].x, NI_P2[NI_P2Len].y, 0);
            }
        }
    }

    setNibbleDot(NI_P1[0].x, NI_P1[0].y, 6);
    if (config.NI_AntPlayers == 1)
        setNibbleDot(NI_P2[0].x, NI_P2[0].y, 5);

    if ((j == 1) && !config.NI_Surround)
    {
        if (NI_Number == 9)
        {
            nibblesNewLevel();
            NI_CurTick60Hz = NI_CurSpeed60Hz;
            return;
        }

        nibblesGenNewNumber();
    }

NoMove:
    NI_CurTick60Hz = NI_CurSpeed60Hz;
    drawScoresLives();
}

void showNibblesScreen(void)
{
    if (editor.ui.extended)
        exitPatternEditorExtended();

    hideTopScreen();
    editor.ui.nibblesShown = true;

    drawFramework(0,     0, 632,   3, FRAMEWORK_TYPE1);
    drawFramework(0,     3, 148,  49, FRAMEWORK_TYPE1);
    drawFramework(0,    52, 148,  49, FRAMEWORK_TYPE1);
    drawFramework(0,   101, 148,  72, FRAMEWORK_TYPE1);
    drawFramework(148,   3, 417, 170, FRAMEWORK_TYPE1);
    drawFramework(150,   5, 413, 166, FRAMEWORK_TYPE2);
    drawFramework(565,   3,  67, 170, FRAMEWORK_TYPE1);

    textBigOutShadow(4,   6,  PAL_FORGRND, PAL_DSKTOP2, "Player 1");
    textBigOutShadow(4,  55,  PAL_FORGRND, PAL_DSKTOP2, "Player 2");

    textOutShadow(4,  27,  PAL_FORGRND, PAL_DSKTOP2, "Score");
    textOutShadow(4,  75,  PAL_FORGRND, PAL_DSKTOP2, "Score");
    textOutShadow(4,  39,  PAL_FORGRND, PAL_DSKTOP2, "Lives");
    textOutShadow(4,  87,  PAL_FORGRND, PAL_DSKTOP2, "Lives");
    textOutShadow(18, 106, PAL_FORGRND, PAL_DSKTOP2, "1 Player");
    textOutShadow(18, 120, PAL_FORGRND, PAL_DSKTOP2, "2 Players");
    textOutShadow(20, 135, PAL_FORGRND, PAL_DSKTOP2, "Surround");
    textOutShadow(20, 148, PAL_FORGRND, PAL_DSKTOP2, "Grid");
    textOutShadow(20, 161, PAL_FORGRND, PAL_DSKTOP2, "Wrap");
    textOutShadow(80, 105, PAL_FORGRND, PAL_DSKTOP2, "Difficulty:");
    textOutShadow(93, 118, PAL_FORGRND, PAL_DSKTOP2, "Novice");
    textOutShadow(93, 132, PAL_FORGRND, PAL_DSKTOP2, "Average");
    textOutShadow(93, 146, PAL_FORGRND, PAL_DSKTOP2, "Pro");
    textOutShadow(93, 160, PAL_FORGRND, PAL_DSKTOP2, "Triton");

    drawScoresLives();

    blitFast(569, 7, nibblesLogo, 59, 91);

    showPushButton(PB_NIBBLES_PLAY);
    showPushButton(PB_NIBBLES_HELP);
    showPushButton(PB_NIBBLES_HIGHS);
    showPushButton(PB_NIBBLES_EXIT);

    checkBoxes[CB_NIBBLES_SURROUND].checked = config.NI_Surround ? true : false;
    checkBoxes[CB_NIBBLES_GRID].checked     = config.NI_Grid     ? true : false;
    checkBoxes[CB_NIBBLES_WRAP].checked     = config.NI_Wrap     ? true : false;
    showCheckBox(CB_NIBBLES_SURROUND);
    showCheckBox(CB_NIBBLES_GRID);
    showCheckBox(CB_NIBBLES_WRAP);

    uncheckRadioButtonGroup(RB_GROUP_NIBBLES_PLAYERS);
    if (config.NI_AntPlayers == 0)
        radioButtons[RB_NIBBLES_1PLAYER].state = RADIOBUTTON_CHECKED;
    else
        radioButtons[RB_NIBBLES_2PLAYERS].state = RADIOBUTTON_CHECKED;
    showRadioButtonGroup(RB_GROUP_NIBBLES_PLAYERS);

    uncheckRadioButtonGroup(RB_GROUP_NIBBLES_DIFFICULTY);
    switch (config.NI_Speed)
    {
        default:
        case 0: radioButtons[RB_NIBBLES_NOVICE].state  = RADIOBUTTON_CHECKED; break;
        case 1: radioButtons[RB_NIBBLES_AVERAGE].state = RADIOBUTTON_CHECKED; break;
        case 2: radioButtons[RB_NIBBLES_PRO].state     = RADIOBUTTON_CHECKED; break;
        case 3: radioButtons[RB_NIBBLES_MANIAC].state  = RADIOBUTTON_CHECKED; break;
    }
    showRadioButtonGroup(RB_GROUP_NIBBLES_DIFFICULTY);
}

void hideNibblesScreen(void)
{
    hidePushButton(PB_NIBBLES_PLAY);
    hidePushButton(PB_NIBBLES_HELP);
    hidePushButton(PB_NIBBLES_HIGHS);
    hidePushButton(PB_NIBBLES_EXIT);

    hideRadioButtonGroup(RB_GROUP_NIBBLES_PLAYERS);
    hideRadioButtonGroup(RB_GROUP_NIBBLES_DIFFICULTY);

    hideCheckBox(CB_NIBBLES_SURROUND);
    hideCheckBox(CB_NIBBLES_GRID);
    hideCheckBox(CB_NIBBLES_WRAP);

    editor.ui.nibblesShown = false;
}

void exitNibblesScreen(void)
{
    hideNibblesScreen();
    showTopScreen(true);
}

/* PUSH BUTTONS */

void nibblesPlay(void)
{
    if (editor.NI_Play)
    {
        sysReqQueue(SR_NIB_RESTART);
        return;
    }

    if (config.NI_Surround && (config.NI_AntPlayers == 0))
    {
        sysReqQueue(SR_NIB_SURR_1PLAYER);
        return;
    }

    MY_ASSERT(config.NI_Speed < 4)

    NI_CurSpeed = NI_Speeds[config.NI_Speed];

    /* adjust for 70Hz -> 60Hz frames */
    NI_CurSpeed60Hz = (uint8_t)(round(NI_CurSpeed * (VBLANK_HZ / 70.0)));

    /* adjust for 70Hz -> 60Hz frames */
    NI_CurTick60Hz = (uint8_t)(round(NI_Speeds[2] * (VBLANK_HZ / 70.0)));

    editor.NI_Play = true;
    NI_P1Score = 0;
    NI_P2Score = 0;
    NI_P1Lives = 5;
    NI_P2Lives = 5;
    NI_Level = 0;

    newNibblesGame();
}

void nibblesRestartYes(void) /* called from system request */
{
    hideSystemRequest();
    editor.NI_Play = false;

    nibblesPlay();
}

void nibblesHelp(void)
{
    uint8_t i;

    if (editor.NI_Play)
    {
        sysReqQueue(SR_NIB_NO_HELP);
        return;
    }

    clearRect(152, 7, 409, 162);

    textBigOut(160, 10, PAL_FORGRND, "Fasttracker Nibbles Help");
    for (i = 0; i < NIBBLES_HELP_LINES; ++i)
        textOut(160, 36 + (11 * i), PAL_BUTTONS, (char *)(NI_HelpText[i]));
}

void nibblesExit(void)
{
    if (editor.NI_Play)
    {
        sysReqQueue(SR_NIB_QUIT);
        return;
    }

    exitNibblesScreen();
}

void nibblesExit2(void) /* called from system request */
{
    hideSystemRequest();
    editor.NI_Play = false;
    exitNibblesScreen();
}

/* RADIO BUTTONS */

void nibblesSet1Player(void)
{
    config.NI_AntPlayers = 0;
    checkRadioButton(RB_NIBBLES_1PLAYER);
}

void nibblesSet2Players(void)
{
    config.NI_AntPlayers = 1;
    checkRadioButton(RB_NIBBLES_2PLAYERS);
}

void nibblesSetNovice(void)
{
    config.NI_Speed = 0;
    checkRadioButton(RB_NIBBLES_NOVICE);
}

void nibblesSetAverage(void)
{
    config.NI_Speed = 1;
    checkRadioButton(RB_NIBBLES_AVERAGE);
}

void nibblesSetPro(void)
{
    config.NI_Speed = 2;
    checkRadioButton(RB_NIBBLES_PRO);
}

void nibblesSetTriton(void)
{
    config.NI_Speed = 3;
    checkRadioButton(RB_NIBBLES_MANIAC);
}

/* CHECK BOXES */

void nibblesToggleSurround(void)
{
    config.NI_Surround ^= 1;
    checkBoxes[CB_NIBBLES_SURROUND].checked = config.NI_Surround ? true : false;
    showCheckBox(CB_NIBBLES_SURROUND);
}

void nibblesToggleGrid(void)
{
    config.NI_Grid ^= 1;
    checkBoxes[CB_NIBBLES_GRID].checked = config.NI_Grid ? true : false;
    showCheckBox(CB_NIBBLES_GRID);

    if (editor.NI_Play)
        redrawNibblesScreen();
}

void nibblesToggleWrap(void)
{
    config.NI_Wrap ^= 1;

    checkBoxes[CB_NIBBLES_WRAP].checked = config.NI_Wrap ? true : false;
    showCheckBox(CB_NIBBLES_WRAP);
}

void nibblesPlayer1NameOK(void)
{
    uint8_t showHighScoreScreen, player2HadHighScore;
    int16_t i, k;
    highScoreType *h;

    hideSystemRequest();

    player2HadHighScore = NI_P2Score > config.NI_HighScore[9].score;

    showHighScoreScreen = false;
    if ((config.NI_AntPlayers == 0) || !player2HadHighScore)
        showHighScoreScreen = true;

    i = 0;
    while (NI_P1Score <= config.NI_HighScore[i].score)
        i++;

    for (k = 8; k >= i; --k)
        memcpy(&config.NI_HighScore[k + 1], &config.NI_HighScore[k], sizeof (highScoreType));

    /* count name length */
    for (k = 0; k < 22; ++k)
    {
        if (tempPlayer1Name[k] == '\0')
            break;
    }

    h = &config.NI_HighScore[i];

    memset(h->name, 0, sizeof (h->name));
    memcpy(&h->name[1], tempPlayer1Name, k);
    h->name[0] = (char)(k);
    h->score = NI_P1Score;
    h->level = NI_Level;

    if (player2HadHighScore && (NI_P2Score <= config.NI_HighScore[9].score))
    {
        /* player 2 no longer has a highscore */

        hideSystemRequest(); /* close player 2 highscore system request */
        showHighScoreScreen = true;
    }

    if (showHighScoreScreen)
        nibblesHighScore();
}

void nibblesPlayer2NameOK(void)
{
    int16_t i, k;
    highScoreType *h;

    hideSystemRequest();

    if (NI_P2Score <= config.NI_HighScore[9].score)
        return;

    i = 0;
    while (NI_P2Score <= config.NI_HighScore[i].score)
        i++;

    for (k = 8; k >= i; --k)
        memcpy(&config.NI_HighScore[k + 1], &config.NI_HighScore[k], sizeof (highScoreType));

    /* count name length */
    for (k = 0; k < 22; ++k)
    {
        if (tempPlayer2Name[k] == '\0')
            break;
    }

    h = &config.NI_HighScore[i];

    memset(h->name, 0, sizeof (h->name));
    memcpy(&h->name[1], tempPlayer2Name, k);
    h->name[0] = (char)(k);
    h->score = NI_P2Score;
    h->level = NI_Level;

    nibblesHighScore();
}

void nibblesGameOverOK(void) /* called from system request */
{
    uint8_t showHighScoreScreen;

    hideSystemRequest();

    editor.NI_Play = false;

    p1GameOver = true;
    p2GameOver = true;

    showHighScoreScreen = true;

    if (NI_P1Score > config.NI_HighScore[9].score)
    {
        memset(tempPlayer1Name, 0, sizeof (tempPlayer1Name));
        strcpy(tempPlayer1Name, "Unknown");
        setupTextBoxForSysReq(TB_NIB_PLAYER1_NAME, tempPlayer1Name, sizeof (tempPlayer1Name), true);
        sysReqQueue(SR_NIB_P1_NAME);
        showHighScoreScreen = false; /* will be done later */
    }

    if (config.NI_AntPlayers == 1)
    {
        if (NI_P2Score > config.NI_HighScore[9].score)
        {
            memset(tempPlayer2Name, 0, sizeof (tempPlayer2Name));
            strcpy(tempPlayer2Name, "Unknown");
            setupTextBoxForSysReq(TB_NIB_PLAYER2_NAME, tempPlayer2Name, sizeof (tempPlayer2Name), true);
            sysReqQueue(SR_NIB_P2_NAME);
            showHighScoreScreen = false; /* will be done later */
        }
    }

    if (showHighScoreScreen)
    {
        p1GameOver = false;
        p2GameOver = false;

        nibblesHighScore();
    }
}

/* GLOBAL FUNCTIONS */

void nibblesKeyAdministrator(SDL_Scancode scancode)
{
    if (scancode == SDL_SCANCODE_ESCAPE)
    {
        sysReqQueue(SR_NIB_QUIT);
        return;
    }

    switch (scancode)
    {
        /* player 1 */
        case SDL_SCANCODE_RIGHT: nibblesAddBuffer(0, 0); break;
        case SDL_SCANCODE_UP:    nibblesAddBuffer(0, 1); break;
        case SDL_SCANCODE_LEFT:  nibblesAddBuffer(0, 2); break;
        case SDL_SCANCODE_DOWN:  nibblesAddBuffer(0, 3); break;

        /* player 2 */
        case SDL_SCANCODE_D:     nibblesAddBuffer(1, 0); break;
        case SDL_SCANCODE_W:     nibblesAddBuffer(1, 1); break;
        case SDL_SCANCODE_A:     nibblesAddBuffer(1, 2); break;
        case SDL_SCANCODE_S:     nibblesAddBuffer(1, 3); break;

        default: break;
    }
}

uint8_t testNibblesCheatCodes(SDL_Keycode keycode)
{
    const char *codeStringPtr;
    uint8_t codeStringLen;

    /* nibbles cheat codes can only be typed in while holding down left SHIFT+CTRL+ALT */
    if (keyb.leftShiftPressed && keyb.leftCtrlPressed && keyb.leftAltPressed)
    {
        if (editor.ui.systemRequestShown)
            return (true); /* don't allow cheat input while a system request is shown */

        if (editor.NI_Play)
        {
            /* during game: "S", "K", "I", "P" (skip to next level) */
            codeStringPtr = nibblesCheatCode1;
            codeStringLen = sizeof (nibblesCheatCode1) - 1;
        }
        else
        {
            /* not during game: "T", "R", "I", "T", "O", "N" (enable infinite lives) */
            codeStringPtr = nibblesCheatCode2;
            codeStringLen = sizeof (nibblesCheatCode2) - 1;
        }

        nibblesCheatBuffer[NI_CheatIndex] = (char)(keycode);
        if (nibblesCheatBuffer[NI_CheatIndex] != codeStringPtr[NI_CheatIndex])
        {
            NI_CheatIndex = 0; /* start over again, one letter didn't match */
            return (true);
        }

        if (++NI_CheatIndex == codeStringLen) /* cheat code was successfully entered */
        {
            NI_CheatIndex = 0;

            if (editor.NI_Play)
            {
                nibblesNewLevel();
            }
            else
            {
                NI_EternalLives ^= 1;
                if (NI_EternalLives)
                    sysReqQueue(SR_NIB_CHEAT_ON);
                else
                    sysReqQueue(SR_NIB_CHEAT_OFF);
            }
        }

        return (true); /* SHIFT+CTRL+ALT held down, don't test other keys */
    }

    return (false); /* SHIFT+CTRL+ALT not held down, test other keys */
}

void pbNibbles(void)
{
    showNibblesScreen();
}
