#ifndef __SDL1_WRAPPER_H
#define __SDL1_WRAPPER_H

#include <SDL.h>
#define _USE_MATH_DEFINES
#include <math.h>
#define AUDIO_F32 10000000

#define SDL_Keysym			SDL_keysym
#define SDLK_KP_1			SDLK_KP1
#define SDLK_KP_2			SDLK_KP2
#define SDLK_KP_3			SDLK_KP3
#define SDLK_KP_4			SDLK_KP4
#define SDLK_KP_5			SDLK_KP5
#define SDLK_KP_6			SDLK_KP6
#define SDLK_KP_7			SDLK_KP7
#define SDLK_KP_8			SDLK_KP8
#define SDLK_KP_9			SDLK_KP9
#define SDLK_KP_0			SDLK_KP0
#define SDLK_PRINTSCREEN	SDLK_PRINT
#define SDLK_SCROLLLOCK		SDLK_SCROLLOCK
#define SDLK_NUMLOCKCLEAR	SDLK_NUMLOCK
#define SDLK_KP_COMMA		SDLK_COMMA
#define SDLK_LGUI			SDLK_LSUPER
#define SDLK_RGUI			SDLK_RSUPER
#define SDL_SCANCODE_LSHIFT	SDLK_LSHIFT
#define SDL_SCANCODE_RSHIFT	SDLK_RSHIFT
#define SDL_Keycode			SDLKey

#ifdef __AMIGA__
#define D_SDL_MODES (SDL_SWSURFACE | SDL_FULLSCREEN)
#define D_BPP 8
#else
#define D_SDL_MODES SDL_SWSURFACE
#define D_BPP 16
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*
 klaus
 make sdl1_wrapper.h & .cpp
 redefine sdl2 datatypes with sdl1 structs,
 redefine sdl2 functions in sdl1
 and link the include to everywhere needed.


 needed datatypes:
 ----------------
 SDL_Window
 SDL_Renderer
 SDL_Texture
 SDL_WindowUserData
 SDL_WindowShaper
 SDL_DisplayMode
 SDL_HitTest


 needed functions:
 -----------------
 SDL_WarpMouseInWindow(window, X, Y);
 SDL_RenderGetViewport(*renderer, SDL_Rect );
 SDL_RenderGetScale(*renderer, float, NULL );
 SDL_Log(SDL_GetError());
 SDL_SetWindowTitle(window, const *char);
 */
#define SDL_MAX_SINT32  ((Sint32)0x7FFFFFFF)    /* 2147483647 */
#define SDL_MIN_SINT32  ((Sint32)(~0x7FFFFFFF)) /* -2147483648 */
#define SDL_INIT_HAPTIC         0x00001000u

#define SDL_RENDER_SCALE_QUALITY  0

#define SDL_HINT_RENDER_SCALE_QUALITY       "SDL_RENDER_SCALE_QUALITY"

#define SDL_WINDOWPOS_UNDEFINED_MASK    0x1FFF0000u

#define SDL_WINDOWPOS_UNDEFINED_DISPLAY(X)  (SDL_WINDOWPOS_UNDEFINED_MASK|(X))

#define SDL_WINDOWPOS_UNDEFINED         SDL_WINDOWPOS_UNDEFINED_DISPLAY(0)

/* Define a four character code as a Uint32 */
#define SDL_FOURCC(A, B, C, D) \
    ((SDL_static_cast(Uint32, SDL_static_cast(Uint8, (A))) << 0) | \
     (SDL_static_cast(Uint32, SDL_static_cast(Uint8, (B))) << 8) | \
     (SDL_static_cast(Uint32, SDL_static_cast(Uint8, (C))) << 16) | \
     (SDL_static_cast(Uint32, SDL_static_cast(Uint8, (D))) << 24))

#define SDL_DEFINE_PIXELFOURCC(A, B, C, D) SDL_FOURCC(A, B, C, D)

#define SDL_DEFINE_PIXELFORMAT(type, order, layout, bits, bytes) \
    ((1 << 28) | ((type) << 24) | ((order) << 20) | ((layout) << 16) | \
     ((bits) << 8) | ((bytes) << 0))

#define SDL_zero(x) SDL_memset(&(x), 0, sizeof((x)))

#define SDL_TEXTINPUTEVENT_TEXT_SIZE (32)

#define SDL_Renderer SDL_Surface
#define SDL_Texture SDL_Surface

//klaus
// #define SDL_QueueAudio SDL_QueueAudio_REAL
#define SDL_QueueAudio 0

typedef enum
{
	SDL_SYSTEM_CURSOR_ARROW,     /**< Arrow */
	SDL_SYSTEM_CURSOR_IBEAM,     /**< I-beam */
	SDL_SYSTEM_CURSOR_WAIT,      /**< Wait */
	SDL_SYSTEM_CURSOR_CROSSHAIR, /**< Crosshair */
	SDL_SYSTEM_CURSOR_WAITARROW, /**< Small wait cursor (or Wait if not available) */
	SDL_SYSTEM_CURSOR_SIZENWSE,  /**< Double arrow pointing northwest and southeast */
	SDL_SYSTEM_CURSOR_SIZENESW,  /**< Double arrow pointing northeast and southwest */
	SDL_SYSTEM_CURSOR_SIZEWE,    /**< Double arrow pointing west and east */
	SDL_SYSTEM_CURSOR_SIZENS,    /**< Double arrow pointing north and south */
	SDL_SYSTEM_CURSOR_SIZEALL,   /**< Four pointed arrow pointing north, south, east, and west */
	SDL_SYSTEM_CURSOR_NO,        /**< Slashed circle or crossbones */
	SDL_SYSTEM_CURSOR_HAND,      /**< Hand */
	SDL_NUM_SYSTEM_CURSORS
} SDL_SystemCursor;

#define SDL_AUDIO_ALLOW_FREQUENCY_CHANGE    0x00000001
#define SDL_AUDIO_ALLOW_FORMAT_CHANGE       0x00000002
#define SDL_AUDIO_ALLOW_CHANNELS_CHANGE     0x00000004
#define SDL_AUDIO_ALLOW_SAMPLES_CHANGE      0x00000008
#define SDL_AUDIO_ALLOW_ANY_CHANGE          (SDL_AUDIO_ALLOW_FREQUENCY_CHANGE|SDL_AUDIO_ALLOW_FORMAT_CHANGE|SDL_AUDIO_ALLOW_CHANNELS_CHANGE|SDL_AUDIO_ALLOW_SAMPLES_CHANGE)
typedef SDLMod SDL_Keymod;

#define KMOD_LGUI KMOD_LMETA
#define KMOD_RGUI KMOD_RMETA

#define KMOD_CTRL   (KMOD_LCTRL|KMOD_RCTRL)
#define KMOD_SHIFT  (KMOD_LSHIFT|KMOD_RSHIFT)
#define KMOD_ALT    (KMOD_LALT|KMOD_RALT)
#define KMOD_GUI    (KMOD_LGUI|KMOD_RGUI)
//typedef Sint32 SDL_Keycode;
typedef Uint32 SDL_AudioDeviceID;

typedef enum
{
	SDL_SCANCODE_UNKNOWN = 0,
	SDL_SCANCODE_A = 4,
	SDL_SCANCODE_B = 5,
	SDL_SCANCODE_C = 6,
	SDL_SCANCODE_D = 7,
	SDL_SCANCODE_E = 8,
	SDL_SCANCODE_F = 9,
	SDL_SCANCODE_G = 10,
	SDL_SCANCODE_H = 11,
	SDL_SCANCODE_I = 12,
	SDL_SCANCODE_J = 13,
	SDL_SCANCODE_K = 14,
	SDL_SCANCODE_L = 15,
	SDL_SCANCODE_M = 16,
	SDL_SCANCODE_N = 17,
	SDL_SCANCODE_O = 18,
	SDL_SCANCODE_P = 19,
	SDL_SCANCODE_Q = 20,
	SDL_SCANCODE_R = 21,
	SDL_SCANCODE_S = 22,
	SDL_SCANCODE_T = 23,
	SDL_SCANCODE_U = 24,
	SDL_SCANCODE_V = 25,
	SDL_SCANCODE_W = 26,
	SDL_SCANCODE_X = 27,
	SDL_SCANCODE_Y = 28,
	SDL_SCANCODE_Z = 29,

	SDL_SCANCODE_1 = 30,
	SDL_SCANCODE_2 = 31,
	SDL_SCANCODE_3 = 32,
	SDL_SCANCODE_4 = 33,
	SDL_SCANCODE_5 = 34,
	SDL_SCANCODE_6 = 35,
	SDL_SCANCODE_7 = 36,
	SDL_SCANCODE_8 = 37,
	SDL_SCANCODE_9 = 38,
	SDL_SCANCODE_0 = 39,

	SDL_SCANCODE_RETURN = 40,
	SDL_SCANCODE_ESCAPE = 41,
	SDL_SCANCODE_BACKSPACE = 42,
	SDL_SCANCODE_TAB = 43,
	SDL_SCANCODE_SPACE = 44,

	SDL_SCANCODE_MINUS = 45,
	SDL_SCANCODE_EQUALS = 46,
	SDL_SCANCODE_LEFTBRACKET = 47,
	SDL_SCANCODE_RIGHTBRACKET = 48,
	SDL_SCANCODE_BACKSLASH = 49, /**< Located at the lower left of the return
                                  *   key on ISO keyboards and at the right end
                                  *   of the QWERTY row on ANSI keyboards.
                                  *   Produces REVERSE SOLIDUS (backslash) and
                                  *   VERTICAL LINE in a US layout, REVERSE
                                  *   SOLIDUS and VERTICAL LINE in a UK Mac
                                  *   layout, NUMBER SIGN and TILDE in a UK
                                  *   Windows layout, DOLLAR SIGN and POUND SIGN
                                  *   in a Swiss German layout, NUMBER SIGN and
                                  *   APOSTROPHE in a German layout, GRAVE
                                  *   ACCENT and POUND SIGN in a French Mac
                                  *   layout, and ASTERISK and MICRO SIGN in a
                                  *   French Windows layout.
                                  */
	SDL_SCANCODE_NONUSHASH = 50, /**< ISO USB keyboards actually use this code
                                  *   instead of 49 for the same key, but all
                                  *   OSes I've seen treat the two codes
                                  *   identically. So, as an implementor, unless
                                  *   your keyboard generates both of those
                                  *   codes and your OS treats them differently,
                                  *   you should generate SDL_SCANCODE_BACKSLASH
                                  *   instead of this code. As a user, you
                                  *   should not rely on this code because SDL
                                  *   will never generate it with most (all?)
                                  *   keyboards.
                                  */
	SDL_SCANCODE_SEMICOLON = 51,
	SDL_SCANCODE_APOSTROPHE = 52,
	SDL_SCANCODE_GRAVE = 53, /**< Located in the top left corner (on both ANSI
                              *   and ISO keyboards). Produces GRAVE ACCENT and
                              *   TILDE in a US Windows layout and in US and UK
                              *   Mac layouts on ANSI keyboards, GRAVE ACCENT
                              *   and NOT SIGN in a UK Windows layout, SECTION
                              *   SIGN and PLUS-MINUS SIGN in US and UK Mac
                              *   layouts on ISO keyboards, SECTION SIGN and
                              *   DEGREE SIGN in a Swiss German layout (Mac:
                              *   only on ISO keyboards), CIRCUMFLEX ACCENT and
                              *   DEGREE SIGN in a German layout (Mac: only on
                              *   ISO keyboards), SUPERSCRIPT TWO and TILDE in a
                              *   French Windows layout, COMMERCIAL AT and
                              *   NUMBER SIGN in a French Mac layout on ISO
                              *   keyboards, and LESS-THAN SIGN and GREATER-THAN
                              *   SIGN in a Swiss German, German, or French Mac
                              *   layout on ANSI keyboards.
                              */
	SDL_SCANCODE_COMMA = 54,
	SDL_SCANCODE_PERIOD = 55,
	SDL_SCANCODE_SLASH = 56,

	SDL_SCANCODE_CAPSLOCK = 57,

	SDL_SCANCODE_F1 = 58,
	SDL_SCANCODE_F2 = 59,
	SDL_SCANCODE_F3 = 60,
	SDL_SCANCODE_F4 = 61,
	SDL_SCANCODE_F5 = 62,
	SDL_SCANCODE_F6 = 63,
	SDL_SCANCODE_F7 = 64,
	SDL_SCANCODE_F8 = 65,
	SDL_SCANCODE_F9 = 66,
	SDL_SCANCODE_F10 = 67,
	SDL_SCANCODE_F11 = 68,
	SDL_SCANCODE_F12 = 69,

	SDL_SCANCODE_PRINTSCREEN = 70,
	SDL_SCANCODE_SCROLLLOCK = 71,
	SDL_SCANCODE_PAUSE = 72,
	SDL_SCANCODE_INSERT = 73, /**< insert on PC, help on some Mac keyboards (but
                                   does send code 73, not 117) */
	SDL_SCANCODE_HOME = 74,
	SDL_SCANCODE_PAGEUP = 75,
	SDL_SCANCODE_DELETE = 76,
	SDL_SCANCODE_END = 77,
	SDL_SCANCODE_PAGEDOWN = 78,
	SDL_SCANCODE_RIGHT = 79,
	SDL_SCANCODE_LEFT = 80,
	SDL_SCANCODE_DOWN = 81,
	SDL_SCANCODE_UP = 82,

	SDL_SCANCODE_NUMLOCKCLEAR = 83, /**< num lock on PC, clear on Mac keyboards
                                     */
	SDL_SCANCODE_KP_DIVIDE = 84,
	SDL_SCANCODE_KP_MULTIPLY = 85,
	SDL_SCANCODE_KP_MINUS = 86,
	SDL_SCANCODE_KP_PLUS = 87,
	SDL_SCANCODE_KP_ENTER = 88,
	SDL_SCANCODE_KP_1 = 89,
	SDL_SCANCODE_KP_2 = 90,
	SDL_SCANCODE_KP_3 = 91,
	SDL_SCANCODE_KP_4 = 92,
	SDL_SCANCODE_KP_5 = 93,
	SDL_SCANCODE_KP_6 = 94,
	SDL_SCANCODE_KP_7 = 95,
	SDL_SCANCODE_KP_8 = 96,
	SDL_SCANCODE_KP_9 = 97,
	SDL_SCANCODE_KP_0 = 98,
	SDL_SCANCODE_KP_PERIOD = 99,

	SDL_SCANCODE_NONUSBACKSLASH = 100, /**< This is the additional key that ISO
                                        *   keyboards have over ANSI ones,
                                        *   located between left shift and Y.
                                        *   Produces GRAVE ACCENT and TILDE in a
                                        *   US or UK Mac layout, REVERSE SOLIDUS
                                        *   (backslash) and VERTICAL LINE in a
                                        *   US or UK Windows layout, and
                                        *   LESS-THAN SIGN and GREATER-THAN SIGN
                                        *   in a Swiss German, German, or French
                                        *   layout. */
	SDL_SCANCODE_APPLICATION = 101, /**< windows contextual menu, compose */
	SDL_SCANCODE_POWER = 102, /**< The USB document says this is a status flag,
                               *   not a physical key - but some Mac keyboards
                               *   do have a power key. */
	SDL_SCANCODE_KP_EQUALS = 103,
	SDL_SCANCODE_F13 = 104,
	SDL_SCANCODE_F14 = 105,
	SDL_SCANCODE_F15 = 106,
	SDL_SCANCODE_F16 = 107,
	SDL_SCANCODE_F17 = 108,
	SDL_SCANCODE_F18 = 109,
	SDL_SCANCODE_F19 = 110,
	SDL_SCANCODE_F20 = 111,
	SDL_SCANCODE_F21 = 112,
	SDL_SCANCODE_F22 = 113,
	SDL_SCANCODE_F23 = 114,
	SDL_SCANCODE_F24 = 115,
	SDL_SCANCODE_EXECUTE = 116,
	SDL_SCANCODE_HELP = 117,
	SDL_SCANCODE_MENU = 118,
	SDL_SCANCODE_SELECT = 119,
	SDL_SCANCODE_STOP = 120,
	SDL_SCANCODE_AGAIN = 121,   /**< redo */
	SDL_SCANCODE_UNDO = 122,
	SDL_SCANCODE_CUT = 123,
	SDL_SCANCODE_COPY = 124,
	SDL_SCANCODE_PASTE = 125,
	SDL_SCANCODE_FIND = 126,
	SDL_SCANCODE_MUTE = 127,
	SDL_SCANCODE_VOLUMEUP = 128,
	SDL_SCANCODE_VOLUMEDOWN = 129,
/* not sure whether there's a reason to enable these */
/*     SDL_SCANCODE_LOCKINGCAPSLOCK = 130,  */
/*     SDL_SCANCODE_LOCKINGNUMLOCK = 131, */
/*     SDL_SCANCODE_LOCKINGSCROLLLOCK = 132, */
			SDL_SCANCODE_KP_COMMA = 133,
	SDL_SCANCODE_KP_EQUALSAS400 = 134,

	SDL_SCANCODE_INTERNATIONAL1 = 135, /**< used on Asian keyboards, see
                                            footnotes in USB doc */
	SDL_SCANCODE_INTERNATIONAL2 = 136,
	SDL_SCANCODE_INTERNATIONAL3 = 137, /**< Yen */
	SDL_SCANCODE_INTERNATIONAL4 = 138,
	SDL_SCANCODE_INTERNATIONAL5 = 139,
	SDL_SCANCODE_INTERNATIONAL6 = 140,
	SDL_SCANCODE_INTERNATIONAL7 = 141,
	SDL_SCANCODE_INTERNATIONAL8 = 142,
	SDL_SCANCODE_INTERNATIONAL9 = 143,
	SDL_SCANCODE_LANG1 = 144, /**< Hangul/English toggle */
	SDL_SCANCODE_LANG2 = 145, /**< Hanja conversion */
	SDL_SCANCODE_LANG3 = 146, /**< Katakana */
	SDL_SCANCODE_LANG4 = 147, /**< Hiragana */
	SDL_SCANCODE_LANG5 = 148, /**< Zenkaku/Hankaku */
	SDL_SCANCODE_LANG6 = 149, /**< reserved */
	SDL_SCANCODE_LANG7 = 150, /**< reserved */
	SDL_SCANCODE_LANG8 = 151, /**< reserved */
	SDL_SCANCODE_LANG9 = 152, /**< reserved */

	SDL_SCANCODE_ALTERASE = 153, /**< Erase-Eaze */
	SDL_SCANCODE_SYSREQ = 154,
	SDL_SCANCODE_CANCEL = 155,
	SDL_SCANCODE_CLEAR = 156,
	SDL_SCANCODE_PRIOR = 157,
	SDL_SCANCODE_RETURN2 = 158,
	SDL_SCANCODE_SEPARATOR = 159,
	SDL_SCANCODE_OUT = 160,
	SDL_SCANCODE_OPER = 161,
	SDL_SCANCODE_CLEARAGAIN = 162,
	SDL_SCANCODE_CRSEL = 163,
	SDL_SCANCODE_EXSEL = 164,

	SDL_SCANCODE_KP_00 = 176,
	SDL_SCANCODE_KP_000 = 177,
	SDL_SCANCODE_THOUSANDSSEPARATOR = 178,
	SDL_SCANCODE_DECIMALSEPARATOR = 179,
	SDL_SCANCODE_CURRENCYUNIT = 180,
	SDL_SCANCODE_CURRENCYSUBUNIT = 181,
	SDL_SCANCODE_KP_LEFTPAREN = 182,
	SDL_SCANCODE_KP_RIGHTPAREN = 183,
	SDL_SCANCODE_KP_LEFTBRACE = 184,
	SDL_SCANCODE_KP_RIGHTBRACE = 185,
	SDL_SCANCODE_KP_TAB = 186,
	SDL_SCANCODE_KP_BACKSPACE = 187,
	SDL_SCANCODE_KP_A = 188,
	SDL_SCANCODE_KP_B = 189,
	SDL_SCANCODE_KP_C = 190,
	SDL_SCANCODE_KP_D = 191,
	SDL_SCANCODE_KP_E = 192,
	SDL_SCANCODE_KP_F = 193,
	SDL_SCANCODE_KP_XOR = 194,
	SDL_SCANCODE_KP_POWER = 195,
	SDL_SCANCODE_KP_PERCENT = 196,
	SDL_SCANCODE_KP_LESS = 197,
	SDL_SCANCODE_KP_GREATER = 198,
	SDL_SCANCODE_KP_AMPERSAND = 199,
	SDL_SCANCODE_KP_DBLAMPERSAND = 200,
	SDL_SCANCODE_KP_VERTICALBAR = 201,
	SDL_SCANCODE_KP_DBLVERTICALBAR = 202,
	SDL_SCANCODE_KP_COLON = 203,
	SDL_SCANCODE_KP_HASH = 204,
	SDL_SCANCODE_KP_SPACE = 205,
	SDL_SCANCODE_KP_AT = 206,
	SDL_SCANCODE_KP_EXCLAM = 207,
	SDL_SCANCODE_KP_MEMSTORE = 208,
	SDL_SCANCODE_KP_MEMRECALL = 209,
	SDL_SCANCODE_KP_MEMCLEAR = 210,
	SDL_SCANCODE_KP_MEMADD = 211,
	SDL_SCANCODE_KP_MEMSUBTRACT = 212,
	SDL_SCANCODE_KP_MEMMULTIPLY = 213,
	SDL_SCANCODE_KP_MEMDIVIDE = 214,
	SDL_SCANCODE_KP_PLUSMINUS = 215,
	SDL_SCANCODE_KP_CLEAR = 216,
	SDL_SCANCODE_KP_CLEARENTRY = 217,
	SDL_SCANCODE_KP_BINARY = 218,
	SDL_SCANCODE_KP_OCTAL = 219,
	SDL_SCANCODE_KP_DECIMAL = 220,
	SDL_SCANCODE_KP_HEXADECIMAL = 221,

	SDL_SCANCODE_LCTRL = 224,
//	SDL_SCANCODE_LSHIFT = 225,
	SDL_SCANCODE_LALT = 226, /**< alt, option */
	SDL_SCANCODE_LGUI = 227, /**< windows, command (apple), meta */
	SDL_SCANCODE_RCTRL = 228,
//	SDL_SCANCODE_RSHIFT = 229,
	SDL_SCANCODE_RALT = 230, /**< alt gr, option */
	SDL_SCANCODE_RGUI = 231, /**< windows, command (apple), meta */

	SDL_SCANCODE_MODE = 257,    /**< I'm not sure if this is really not covered
                                 *   by any of the above, but since there's a
                                 *   special KMOD_MODE for it I'm adding it here
                                 */

	/* @} *//* Usage page 0x07 */

	/**
	 *  \name Usage page 0x0C
	 *
	 *  These values are mapped from usage page 0x0C (USB consumer page).
	 */
	/* @{ */

			SDL_SCANCODE_AUDIONEXT = 258,
	SDL_SCANCODE_AUDIOPREV = 259,
	SDL_SCANCODE_AUDIOSTOP = 260,
	SDL_SCANCODE_AUDIOPLAY = 261,
	SDL_SCANCODE_AUDIOMUTE = 262,
	SDL_SCANCODE_MEDIASELECT = 263,
	SDL_SCANCODE_WWW = 264,
	SDL_SCANCODE_MAIL = 265,
	SDL_SCANCODE_CALCULATOR = 266,
	SDL_SCANCODE_COMPUTER = 267,
	SDL_SCANCODE_AC_SEARCH = 268,
	SDL_SCANCODE_AC_HOME = 269,
	SDL_SCANCODE_AC_BACK = 270,
	SDL_SCANCODE_AC_FORWARD = 271,
	SDL_SCANCODE_AC_STOP = 272,
	SDL_SCANCODE_AC_REFRESH = 273,
	SDL_SCANCODE_AC_BOOKMARKS = 274,



			SDL_SCANCODE_BRIGHTNESSDOWN = 275,
	SDL_SCANCODE_BRIGHTNESSUP = 276,
	SDL_SCANCODE_DISPLAYSWITCH = 277, /* *< display mirroring/dual display
                                           switch, video mode switch */
	SDL_SCANCODE_KBDILLUMTOGGLE = 278,
	SDL_SCANCODE_KBDILLUMDOWN = 279,
	SDL_SCANCODE_KBDILLUMUP = 280,
	SDL_SCANCODE_EJECT = 281,
	SDL_SCANCODE_SLEEP = 282,

	SDL_SCANCODE_APP1 = 283,
	SDL_SCANCODE_APP2 = 284,

	/* @} *//* Walther keys */

	/**
	 *  \name Usage page 0x0C (additional media keys)
	 *
	 *  These values are mapped from usage page 0x0C (USB consumer page).
	 */
	/* @{ */

			SDL_SCANCODE_AUDIOREWIND = 285,
	SDL_SCANCODE_AUDIOFASTFORWARD = 286,

	/* @} *//* Usage page 0x0C (additional media keys) */

	/* Add any other keys here. */

			SDL_NUM_SCANCODES = 512 /**< not a key, just marks the number of scancodes
                                 for array bounds */
} SDL_Scancode;

typedef enum
{
    SDL_RENDERER_SOFTWARE = 0x00000001,         /**< The renderer is a software fallback */
    SDL_RENDERER_ACCELERATED = 0x00000002,      /**< The renderer uses hardware acceleration */
    SDL_RENDERER_PRESENTVSYNC = 0x00000004,     /**< Present is synchronized with the refresh rate */
    SDL_RENDERER_TARGETTEXTURE = 0x00000008     /**< The renderer supports rendering to texture */
} SDL_RendererFlags;

#define SDL_WINDOWEVENT 0x200
#define SDL_TEXTINPUT 0x303

typedef enum
{
	SDL_WINDOWEVENT_NONE,           /**< Never used */
	SDL_WINDOWEVENT_SHOWN,          /**< Window has been shown */
	SDL_WINDOWEVENT_HIDDEN,         /**< Window has been hidden */
	SDL_WINDOWEVENT_EXPOSED,        /**< Window has been exposed and should be
                                         redrawn */
	SDL_WINDOWEVENT_MOVED,          /**< Window has been moved to data1, data2
                                     */
	SDL_WINDOWEVENT_RESIZED,        /**< Window has been resized to data1xdata2 */
	SDL_WINDOWEVENT_SIZE_CHANGED,   /**< The window size has changed, either as
                                         a result of an API call or through the
                                         system or user changing the window size. */
	SDL_WINDOWEVENT_MINIMIZED,      /**< Window has been minimized */
	SDL_WINDOWEVENT_MAXIMIZED,      /**< Window has been maximized */
	SDL_WINDOWEVENT_RESTORED,       /**< Window has been restored to normal size
                                         and position */
	SDL_WINDOWEVENT_ENTER,          /**< Window has gained mouse focus */
	SDL_WINDOWEVENT_LEAVE,          /**< Window has lost mouse focus */
	SDL_WINDOWEVENT_FOCUS_GAINED,   /**< Window has gained keyboard focus */
	SDL_WINDOWEVENT_FOCUS_LOST,     /**< Window has lost keyboard focus */
	SDL_WINDOWEVENT_CLOSE,          /**< The window manager requests that the window be closed */
	SDL_WINDOWEVENT_TAKE_FOCUS,     /**< Window is being offered a focus (should SetWindowInputFocus() on itself or a subwindow, or ignore) */
	SDL_WINDOWEVENT_HIT_TEST        /**< Window had a hit test that wasn't SDL_HITTEST_NORMAL. */
} SDL_WindowEventID;

typedef struct SDL_WindowEvent
{
	Uint32 type;        /**< ::SDL_WINDOWEVENT */
	Uint32 timestamp;   /**< In milliseconds, populated using SDL_GetTicks() */
	Uint32 windowID;    /**< The associated window */
	Uint8 event;        /**< ::SDL_WindowEventID */
	Uint8 padding1;
	Uint8 padding2;
	Uint8 padding3;
	Sint32 data1;       /**< event dependent data */
	Sint32 data2;       /**< event dependent data */
} SDL_WindowEvent;

typedef enum
{
    /* !!! FIXME: change this to name = (1<<x). */
    SDL_WINDOW_FULLSCREEN = 0x00000001,         /**< fullscreen window */
    SDL_WINDOW_OPENGL = 0x00000002,             /**< window usable with OpenGL context */
    SDL_WINDOW_SHOWN = 0x00000004,              /**< window is visible */
    SDL_WINDOW_HIDDEN = 0x00000008,             /**< window is not visible */
    SDL_WINDOW_BORDERLESS = 0x00000010,         /**< no window decoration */
    SDL_WINDOW_RESIZABLE = 0x00000020,          /**< window can be resized */
    SDL_WINDOW_MINIMIZED = 0x00000040,          /**< window is minimized */
    SDL_WINDOW_MAXIMIZED = 0x00000080,          /**< window is maximized */
    SDL_WINDOW_INPUT_GRABBED = 0x00000100,      /**< window has grabbed input focus */
    SDL_WINDOW_INPUT_FOCUS = 0x00000200,        /**< window has input focus */
    SDL_WINDOW_MOUSE_FOCUS = 0x00000400,        /**< window has mouse focus */
    SDL_WINDOW_FULLSCREEN_DESKTOP = ( SDL_WINDOW_FULLSCREEN | 0x00001000 ),
    SDL_WINDOW_FOREIGN = 0x00000800,            /**< window not created by SDL */
    SDL_WINDOW_ALLOW_HIGHDPI = 0x00002000,      /**< window should be created in high-DPI mode if supported.
                                                     On macOS NSHighResolutionCapable must be set true in the
                                                     application's Info.plist for this to have any effect. */
    SDL_WINDOW_MOUSE_CAPTURE = 0x00004000,      /**< window has mouse captured (unrelated to INPUT_GRABBED) */
    SDL_WINDOW_ALWAYS_ON_TOP = 0x00008000,      /**< window should always be above others */
    SDL_WINDOW_SKIP_TASKBAR  = 0x00010000,      /**< window should not be added to the taskbar */
    SDL_WINDOW_UTILITY       = 0x00020000,      /**< window should be treated as a utility window */
    SDL_WINDOW_TOOLTIP       = 0x00040000,      /**< window should be treated as a tooltip */
    SDL_WINDOW_POPUP_MENU    = 0x00080000,      /**< window should be treated as a popup menu */
    SDL_WINDOW_VULKAN        = 0x10000000       /**< window usable for Vulkan surface */
} SDL_WindowFlags;

typedef enum
{
    SDL_MESSAGEBOX_ERROR        = 0x00000010,   /**< error dialog */
    SDL_MESSAGEBOX_WARNING      = 0x00000020,   /**< warning dialog */
    SDL_MESSAGEBOX_INFORMATION  = 0x00000040    /**< informational dialog */
} SDL_MessageBoxFlags;

typedef enum
{
    SDL_TEXTUREACCESS_STATIC,    /**< Changes rarely, not lockable */
    SDL_TEXTUREACCESS_STREAMING, /**< Changes frequently, lockable */
    SDL_TEXTUREACCESS_TARGET     /**< Texture can be used as a render target */
} SDL_TextureAccess;

enum
{
    SDL_PIXELTYPE_UNKNOWN,
    SDL_PIXELTYPE_INDEX1,
    SDL_PIXELTYPE_INDEX4,
    SDL_PIXELTYPE_INDEX8,
    SDL_PIXELTYPE_PACKED8,
    SDL_PIXELTYPE_PACKED16,
    SDL_PIXELTYPE_PACKED32,
    SDL_PIXELTYPE_ARRAYU8,
    SDL_PIXELTYPE_ARRAYU16,
    SDL_PIXELTYPE_ARRAYU32,
    SDL_PIXELTYPE_ARRAYF16,
    SDL_PIXELTYPE_ARRAYF32
};

enum
{
    SDL_BITMAPORDER_NONE,
    SDL_BITMAPORDER_4321,
    SDL_BITMAPORDER_1234
};

enum
{
    SDL_PACKEDORDER_NONE,
    SDL_PACKEDORDER_XRGB,
    SDL_PACKEDORDER_RGBX,
    SDL_PACKEDORDER_ARGB,
    SDL_PACKEDORDER_RGBA,
    SDL_PACKEDORDER_XBGR,
    SDL_PACKEDORDER_BGRX,
    SDL_PACKEDORDER_ABGR,
    SDL_PACKEDORDER_BGRA
};

enum
{
    SDL_PACKEDLAYOUT_NONE,
    SDL_PACKEDLAYOUT_332,
    SDL_PACKEDLAYOUT_4444,
    SDL_PACKEDLAYOUT_1555,
    SDL_PACKEDLAYOUT_5551,
    SDL_PACKEDLAYOUT_565,
    SDL_PACKEDLAYOUT_8888,
    SDL_PACKEDLAYOUT_2101010,
    SDL_PACKEDLAYOUT_1010102
};

enum
{
    SDL_ARRAYORDER_NONE,
    SDL_ARRAYORDER_RGB,
    SDL_ARRAYORDER_RGBA,
    SDL_ARRAYORDER_ARGB,
    SDL_ARRAYORDER_BGR,
    SDL_ARRAYORDER_BGRA,
    SDL_ARRAYORDER_ABGR
};

enum
{
    SDL_PIXELFORMAT_UNKNOWN,
    SDL_PIXELFORMAT_INDEX1LSB =
        SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_INDEX1, SDL_BITMAPORDER_4321, 0,
                               1, 0),
    SDL_PIXELFORMAT_INDEX1MSB =
        SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_INDEX1, SDL_BITMAPORDER_1234, 0,
                               1, 0),
    SDL_PIXELFORMAT_INDEX4LSB =
        SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_INDEX4, SDL_BITMAPORDER_4321, 0,
                               4, 0),
    SDL_PIXELFORMAT_INDEX4MSB =
        SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_INDEX4, SDL_BITMAPORDER_1234, 0,
                               4, 0),
    SDL_PIXELFORMAT_INDEX8 =
        SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_INDEX8, 0, 0, 8, 1),
    SDL_PIXELFORMAT_RGB332 =
        SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED8, SDL_PACKEDORDER_XRGB,
                               SDL_PACKEDLAYOUT_332, 8, 1),
    SDL_PIXELFORMAT_RGB444 =
        SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED16, SDL_PACKEDORDER_XRGB,
                               SDL_PACKEDLAYOUT_4444, 12, 2),
    SDL_PIXELFORMAT_RGB555 =
        SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED16, SDL_PACKEDORDER_XRGB,
                               SDL_PACKEDLAYOUT_1555, 15, 2),
    SDL_PIXELFORMAT_BGR555 =
        SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED16, SDL_PACKEDORDER_XBGR,
                               SDL_PACKEDLAYOUT_1555, 15, 2),
    SDL_PIXELFORMAT_ARGB4444 =
        SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED16, SDL_PACKEDORDER_ARGB,
                               SDL_PACKEDLAYOUT_4444, 16, 2),
    SDL_PIXELFORMAT_RGBA4444 =
        SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED16, SDL_PACKEDORDER_RGBA,
                               SDL_PACKEDLAYOUT_4444, 16, 2),
    SDL_PIXELFORMAT_ABGR4444 =
        SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED16, SDL_PACKEDORDER_ABGR,
                               SDL_PACKEDLAYOUT_4444, 16, 2),
    SDL_PIXELFORMAT_BGRA4444 =
        SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED16, SDL_PACKEDORDER_BGRA,
                               SDL_PACKEDLAYOUT_4444, 16, 2),
    SDL_PIXELFORMAT_ARGB1555 =
        SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED16, SDL_PACKEDORDER_ARGB,
                               SDL_PACKEDLAYOUT_1555, 16, 2),
    SDL_PIXELFORMAT_RGBA5551 =
        SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED16, SDL_PACKEDORDER_RGBA,
                               SDL_PACKEDLAYOUT_5551, 16, 2),
    SDL_PIXELFORMAT_ABGR1555 =
        SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED16, SDL_PACKEDORDER_ABGR,
                               SDL_PACKEDLAYOUT_1555, 16, 2),
    SDL_PIXELFORMAT_BGRA5551 =
        SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED16, SDL_PACKEDORDER_BGRA,
                               SDL_PACKEDLAYOUT_5551, 16, 2),
    SDL_PIXELFORMAT_RGB565 =
        SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED16, SDL_PACKEDORDER_XRGB,
                               SDL_PACKEDLAYOUT_565, 16, 2),
    SDL_PIXELFORMAT_BGR565 =
        SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED16, SDL_PACKEDORDER_XBGR,
                               SDL_PACKEDLAYOUT_565, 16, 2),
    SDL_PIXELFORMAT_RGB24 =
        SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_ARRAYU8, SDL_ARRAYORDER_RGB, 0,
                               24, 3),
    SDL_PIXELFORMAT_BGR24 =
        SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_ARRAYU8, SDL_ARRAYORDER_BGR, 0,
                               24, 3),
    SDL_PIXELFORMAT_RGB888 =
        SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED32, SDL_PACKEDORDER_XRGB,
                               SDL_PACKEDLAYOUT_8888, 24, 4),
    SDL_PIXELFORMAT_RGBX8888 =
        SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED32, SDL_PACKEDORDER_RGBX,
                               SDL_PACKEDLAYOUT_8888, 24, 4),
    SDL_PIXELFORMAT_BGR888 =
        SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED32, SDL_PACKEDORDER_XBGR,
                               SDL_PACKEDLAYOUT_8888, 24, 4),
    SDL_PIXELFORMAT_BGRX8888 =
        SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED32, SDL_PACKEDORDER_BGRX,
                               SDL_PACKEDLAYOUT_8888, 24, 4),
    SDL_PIXELFORMAT_ARGB8888 =
        SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED32, SDL_PACKEDORDER_ARGB,
                               SDL_PACKEDLAYOUT_8888, 32, 4),
    SDL_PIXELFORMAT_RGBA8888 =
        SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED32, SDL_PACKEDORDER_RGBA,
                               SDL_PACKEDLAYOUT_8888, 32, 4),
    SDL_PIXELFORMAT_ABGR8888 =
        SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED32, SDL_PACKEDORDER_ABGR,
                               SDL_PACKEDLAYOUT_8888, 32, 4),
    SDL_PIXELFORMAT_BGRA8888 =
        SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED32, SDL_PACKEDORDER_BGRA,
                               SDL_PACKEDLAYOUT_8888, 32, 4),
    SDL_PIXELFORMAT_ARGB2101010 =
        SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED32, SDL_PACKEDORDER_ARGB,
                               SDL_PACKEDLAYOUT_2101010, 32, 4),

    /* Aliases for RGBA byte arrays of color data, for the current platform */
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    SDL_PIXELFORMAT_RGBA32 = SDL_PIXELFORMAT_RGBA8888,
    SDL_PIXELFORMAT_ARGB32 = SDL_PIXELFORMAT_ARGB8888,
    SDL_PIXELFORMAT_BGRA32 = SDL_PIXELFORMAT_BGRA8888,
    SDL_PIXELFORMAT_ABGR32 = SDL_PIXELFORMAT_ABGR8888,
#else
    SDL_PIXELFORMAT_RGBA32 = SDL_PIXELFORMAT_ABGR8888,
    SDL_PIXELFORMAT_ARGB32 = SDL_PIXELFORMAT_BGRA8888,
    SDL_PIXELFORMAT_BGRA32 = SDL_PIXELFORMAT_ARGB8888,
    SDL_PIXELFORMAT_ABGR32 = SDL_PIXELFORMAT_RGBA8888,
#endif

    SDL_PIXELFORMAT_YV12 =      /**< Planar mode: Y + V + U  (3 planes) */
        SDL_DEFINE_PIXELFOURCC('Y', 'V', '1', '2'),
    SDL_PIXELFORMAT_IYUV =      /**< Planar mode: Y + U + V  (3 planes) */
        SDL_DEFINE_PIXELFOURCC('I', 'Y', 'U', 'V'),
    SDL_PIXELFORMAT_YUY2 =      /**< Packed mode: Y0+U0+Y1+V0 (1 plane) */
        SDL_DEFINE_PIXELFOURCC('Y', 'U', 'Y', '2'),
    SDL_PIXELFORMAT_UYVY =      /**< Packed mode: U0+Y0+V0+Y1 (1 plane) */
        SDL_DEFINE_PIXELFOURCC('U', 'Y', 'V', 'Y'),
    SDL_PIXELFORMAT_YVYU =      /**< Packed mode: Y0+V0+Y1+U0 (1 plane) */
        SDL_DEFINE_PIXELFOURCC('Y', 'V', 'Y', 'U'),
    SDL_PIXELFORMAT_NV12 =      /**< Planar mode: Y + U/V interleaved  (2 planes) */
        SDL_DEFINE_PIXELFOURCC('N', 'V', '1', '2'),
    SDL_PIXELFORMAT_NV21 =      /**< Planar mode: Y + V/U interleaved  (2 planes) */
        SDL_DEFINE_PIXELFOURCC('N', 'V', '2', '1'),
    SDL_PIXELFORMAT_EXTERNAL_OES =      /**< Android video texture format */
        SDL_DEFINE_PIXELFOURCC('O', 'E', 'S', ' ')
};

typedef struct SDL_Window SDL_Window;

typedef struct SDL_Point
{
    int x;
    int y;
} SDL_Point;


// todo structs:
// ============

// sdl_displaymode struct is not finished
typedef struct
{
	Uint32 format;              /**< pixel format */
	int w;                      /**< width, in screen coordinates */
	int h;                      /**< height, in screen coordinates */
	int refresh_rate;           /**< refresh rate (or zero for unspecified) */
	void *driverdata;           /**< driver-specific data, initialize to 0 */
} SDL_DisplayMode;

typedef struct SDL_WindowUserData
{
    char *name;
    void *data;
    struct SDL_WindowUserData *next;
} SDL_WindowUserData;

typedef struct {
    SDL_Window*      win;
    const SDL_Point* area;
    void*            data;
} SDL_HitTest;


#define SDL_WINDOWPOS_CENTERED         0

Uint64 SDL_GetPerformanceCounter(void);
Uint64 SDL_GetPerformanceFrequency(void);
int SDL_GetNumAudioDevices(int iscapture);
const char * SDL_GetAudioDeviceName(int index, int iscapture);
typedef struct SDL_WindowShaper SDL_WindowShaper;
typedef struct SDL_ShapeDriver SDL_ShapeDriver;
typedef struct SDL_VideoDisplay SDL_VideoDisplay;
typedef struct SDL_VideoDevice SDL_VideoDevice;
typedef union {
    /** \brief A cutoff alpha value for binarization of the window shape's alpha channel. */
    Uint8 binarizationCutoff;
    SDL_Color colorKey;
} SDL_WindowShapeParams;
typedef enum {
    /** \brief The default mode, a binarized alpha cutoff of 1. */
    ShapeModeDefault,
    /** \brief A binarized alpha cutoff with a given integer value. */
    ShapeModeBinarizeAlpha,
    /** \brief A binarized alpha cutoff with a given integer value, but with the opposite comparison. */
    ShapeModeReverseBinarizeAlpha,
    /** \brief A color key is applied. */
    ShapeModeColorKey
} WindowShapeMode;

typedef struct SDL_WindowShapeMode {
    /** \brief The mode of these window-shape parameters. */
    WindowShapeMode mode;
    /** \brief Window-shape parameters. */
    SDL_WindowShapeParams parameters;
} SDL_WindowShapeMode;

typedef enum
{
    SDL_ORIENTATION_UNKNOWN,            /**< The display orientation can't be determined */
    SDL_ORIENTATION_LANDSCAPE,          /**< The display is in landscape mode, with the right side up, relative to portrait mode */
    SDL_ORIENTATION_LANDSCAPE_FLIPPED,  /**< The display is in landscape mode, with the left side up, relative to portrait mode */
    SDL_ORIENTATION_PORTRAIT,           /**< The display is in portrait mode */
    SDL_ORIENTATION_PORTRAIT_FLIPPED    /**< The display is in portrait mode, upside down */
} SDL_DisplayOrientation;

struct SDL_VideoDisplay
{
    char *name;
    int max_display_modes;
    int num_display_modes;
    SDL_DisplayMode *display_modes;
    SDL_DisplayMode desktop_mode;
    SDL_DisplayMode current_mode;
    SDL_DisplayOrientation orientation;

    SDL_Window *fullscreen_window;

    SDL_VideoDevice *device;

    void *driverdata;
};


// todo functions:
// ==============


void SDL_RenderGetViewport(SDL_Renderer* renderer, SDL_Rect* rect);
void SDL_RenderGetScale(SDL_Renderer* renderer, float* scaleX, float* scaleY);
void SDL_WarpMouseInWindow(SDL_Window* window, int x, int y);
void SDL_Log(const char* fmt, ...);
void SDL_SetWindowTitle(SDL_Window* window, const char* title);
char* SDL_GetPrefPath(const char* org, const char* app);
const Uint8* SDL_GetKeyboardState(int* numkeys);
typedef int (SDLCALL * SDL_ThreadFunction) (void *data);
//SDL_Thread* SDL_CreateThread(SDL_ThreadFunction fn, const char* name, void* data);
char* SDL_GetBasePath(void);
void SDL_EnableScreenSaver(void);
void SDL_DisableScreenSaver(void);
void SDL_RaiseWindow(SDL_Window* window);
int SDL_SetWindowInputFocus(SDL_Window* window);
SDL_bool SDL_SetHint(const char* name, const char* value);
void SDL_ShowWindow(SDL_Window* window);
int SDL_GetCurrentDisplayMode(int displayIndex, SDL_DisplayMode* mode);
void SDL_GetWindowPosition(SDL_Window* window,  int* x, int* y);
int SDL_ShowSimpleMessageBox(Uint32      flags, const char* title, const char* message, SDL_Window* window);
SDL_bool SDL_IsScreenSaverEnabled(void);
void SDL_HideWindow(SDL_Window* window);
SDL_Window* SDL_CreateWindow(const char* title, int x, int y, int w, int h, Uint32 flags);
void SDL_FreePalette(SDL_Palette* palette);
void SDL_DestroyTexture(SDL_Texture* texture);
void SDL_DestroyRenderer(SDL_Renderer* renderer);
void SDL_DestroyWindow(SDL_Window* window);
SDL_Palette* SDL_AllocPalette(int ncolors);
int SDL_GetRendererOutputSize(SDL_Renderer* renderer, int* w, int* h);
SDL_Surface* SDL_CreateRGBSurfaceWithFormat(Uint32 flags, int width, int height, int depth, Uint32 format);
void SDL_GetWindowSize(SDL_Window* window, int* w, int* h);
SDL_Surface* SDL_GetWindowSurface(SDL_Window* window);
int SDL_SetSurfacePalette(SDL_Surface* surface, SDL_Palette* palette);
int SDL_UpdateTexture(SDL_Texture* texture, const SDL_Rect* rect, const void* pixels, int pitch);
int SDL_RenderClear(SDL_Renderer* renderer);
int SDL_RenderCopy(SDL_Renderer*   renderer, SDL_Texture* texture, const SDL_Rect* srcrect, const SDL_Rect* dstrect);
void SDL_RenderPresent(SDL_Renderer* renderer);
int SDL_UpdateWindowSurface(SDL_Window* window);
SDL_Renderer* SDL_CreateRenderer(SDL_Window* window, int index, Uint32 flags);
SDL_Texture* SDL_CreateTexture(SDL_Renderer* renderer, Uint32 format, int access, int w, int h);
int SDL_RenderSetLogicalSize(SDL_Renderer* renderer, int w, int h);
int SDL_SetRenderDrawColor(SDL_Renderer* renderer,  Uint8 r, Uint8 g, Uint8 b, Uint8 a);
Uint32 SDL_GetWindowPixelFormat(SDL_Window* window);
SDL_Surface* SDL_ConvertSurfaceFormat(SDL_Surface* src, Uint32 pixel_format, Uint32 flags);
int SDL_BlitScaled(SDL_Surface* src, const SDL_Rect* srcrect, SDL_Surface* dst, SDL_Rect* dstrect);
void SDL_ClearQueuedAudio(SDL_AudioDeviceID dev);
void SDL_PauseAudioDevice(SDL_AudioDeviceID dev, int pause_on);
void SDL_UnlockAudioDevice(SDL_AudioDeviceID dev);

SDL_AudioDeviceID SDL_OpenAudioDevice(const char* device, int iscapture,
									  const SDL_AudioSpec* desired,
                                      SDL_AudioSpec*       obtained,
                                      int                  allowed_changes);

int SDL_SetPaletteColors(SDL_Palette* palette, const SDL_Color* colors, int firstcolor, int ncolors);
void SDL_CloseAudioDevice(SDL_AudioDeviceID dev);

SDL_Surface* SDL_CreateRGBSurfaceWithFormatFrom(void*  pixels,
                                                int    width,
                                                int    height,
                                                int    depth,
                                                int    pitch,
                                                Uint32 format);

void SDL_SetWindowPosition(SDL_Window* window, int x, int y);
void SDL_StopTextInput(void);
void SDL_StartTextInput(void);
SDL_bool SDL_IsTextInputActive(void);
char* SDL_GetClipboardText(void);
SDL_bool SDL_PointInRect(const SDL_Point* p, const SDL_Rect*  r);

#ifdef __cplusplus
}
#endif

#endif //  __SDL1_WRAPPER_H
