#include <stdio.h> // sprintf()
#include "ft2_header.h"
#include "ft2_gui.h"
#include "ft2_pattern_ed.h"
#include "ft2_gfxdata.h"
#include "ft2_video.h"

// ported from original FT2 code

#define NUM_STARS 512
#define ABOUT_SCREEN_W 626
#define ABOUT_SCREEN_H 167
#define FT2_LOGO_W 449
#define FT2_LOGO_H 75
#define ABOUT_TEXT_W 349
#define ABOUT_TEXT_H 29

typedef struct
{
	int16_t x, y, z;
} vector_t;

typedef struct
{
	uint16_t x, y, z;
} rotate_t;

typedef struct
{
	vector_t x, y, z;
} matrix_t;

extern const uint16_t sinusTables[256 * 5]; // defined at the bottom of this file

static const uint8_t starColConv[24] = { 2,2,2,2,2,2,2,2, 2,2,2,1,1,1,3,3, 3,3,3,3,3,3,3,3 };
static const int16_t *sin32767 = (const int16_t *)sinusTables, *cos32767 = (const int16_t *)&sinusTables[256];
static int16_t hastighet;
static int32_t lastStarScreenPos[NUM_STARS];
static uint32_t randSeed;
static vector_t starcrd[NUM_STARS];
static rotate_t star_a;
static matrix_t starmat;

void seedAboutScreenRandom(uint32_t newseed)
{
	randSeed = newseed;
}

static inline int32_t random32(int32_t l)
{
	int32_t r;

	randSeed *= 134775813;
	randSeed += 1;

	r = (int32_t)(((int64_t)(randSeed) * l) >> 32);
	return r;
}

static void fixaMatris(rotate_t a, matrix_t *mat)
{
	int16_t sa, sb, sc, ca, cb, cc;

	sa = sin32767[a.x >> 6]; sb = sin32767[a.y >> 6]; sc = sin32767[a.z >> 6];
	ca = cos32767[a.x >> 6]; cb = cos32767[a.y >> 6]; cc = cos32767[a.z >> 6];

	mat->x.x = ((ca * cc) >> 16) + (((sc * ((sa * sb) >> 16)) >> 16) << 1);
	mat->y.x =  (sa * cb) >> 16;
	mat->z.x = (((cc * ((sa * sb) >> 16)) >> 16) << 1) - ((ca * sc) >> 16);

	mat->x.y = (((sc * ((ca * sb) >> 16)) >> 16) << 1) - ((sa * cc) >> 16);
	mat->y.y =  (ca * cb) >> 16;
	mat->z.y = ((sa * sc) >> 16) + (((cc * ((ca * sb) >> 16)) >> 16) << 1);

	mat->x.z = (cb * sc) >> 16;
	mat->y.z =   0 - (sb >> 1);
	mat->z.z = (cb * cc) >> 16;
}

static inline int32_t sqr(int32_t x)
{
	return x * x;
}

static void aboutInit(void)
{
	uint8_t type;
	int16_t i;
	int32_t r, n, w, h;
	double ww;

	type = (uint8_t)random32(4);
	switch (type)
	{
		case 0:
		{
			hastighet = 309;
			for (i = 0; i < NUM_STARS; i++)
			{
				starcrd[i].z = (int16_t)random32(0xFFFF) - 0x8000;
				starcrd[i].y = (int16_t)random32(0xFFFF) - 0x8000;
				starcrd[i].x = (int16_t)random32(0xFFFF) - 0x8000;
			}
		}
		break;

		case 1:
		{
			hastighet = 0;
			for (i = 0; i < NUM_STARS; i++)
			{
				if (i < (NUM_STARS / 4))
				{
					starcrd[i].z = (int16_t)random32(0xFFFF) - 0x8000;
					starcrd[i].y = (int16_t)random32(0xFFFF) - 0x8000;
					starcrd[i].x = (int16_t)random32(0xFFFF) - 0x8000;
				}
				else
				{
					r = random32(30000);
					n = random32(5);
					w = ((2 * random32(2)) - 1) * sqr(random32(1000));
					ww = (((M_PI * 2.0) / 5.0) * n) + (r / 12000.0) + (w / 3000000.0);
					h = ((sqr(r) / 30000) * (random32(10000) - 5000)) / 12000;

					starcrd[i].x = (int16_t)trunc(r * cos(ww));
					starcrd[i].y = (int16_t)trunc(r * sin(ww));
					starcrd[i].z = (int16_t)h;
				}
			}
		}
		break;

		case 2:
		case 3:
		{
			hastighet = 0;
			for (i = 0; i < NUM_STARS; i++)
			{
				r = (int32_t)round(sqrt(random32(500) * 500));
				w = random32(3000);
				h = cos32767[(((w * 8) + r) / 16) & 1023] / 4;

				starcrd[i].z = (int16_t)((cos32767[w & 1023] * (w + r)) / 3500);
				starcrd[i].y = (int16_t)((sin32767[w & 1023] * (w + r)) / 3500);
				starcrd[i].x = (int16_t)((h * r) / 500);
			}
		}
		break;

		default:
			break;
	}

	for (i = 0; i < NUM_STARS; i++)
		lastStarScreenPos[i] = -1;
}

static void realStars(void)
{
	uint8_t col;
	int16_t x, y, z, xx, xy, xz, yx, yy, yz, zx, zy, zz;
	int32_t screenBufferPos;
	vector_t *star;

	xx = starmat.x.x; xy = starmat.x.y; xz = starmat.x.z;
	yx = starmat.y.x; yy = starmat.y.y; yz = starmat.y.z;
	zx = starmat.z.x; zy = starmat.z.y; zz = starmat.z.z;

	for (int16_t i = 0; i < NUM_STARS; i++)
	{
		// erase last star pixel
		screenBufferPos = lastStarScreenPos[i];
		if (screenBufferPos >= 0)
		{
			if (!(video.frameBuffer[screenBufferPos] & 0xFF000000))
				video.frameBuffer[screenBufferPos] = video.palette[PAL_BCKGRND];

			lastStarScreenPos[i] = -1;
		}

		star = &starcrd[i];
		star->z += hastighet;

		z = (((xz * star->x) >> 16) + ((yz * star->y) >> 16) + ((zz * star->z) >> 16)) + 9000;
		if (z <= 100)
			continue;

		y = ((xy * star->x) >> 16) + ((yy * star->y) >> 16) + ((zy * star->z) >> 16);
		y = (int16_t)((y << 7) / z) + 84;
		if ((uint16_t)y >= (173 - 6))
			continue;

		x = ((xx * star->x) >> 16) + ((yx * star->y) >> 16) + ((zx * star->z) >> 16);
		x = (int16_t)((((x >> 2) + x) << 7) / z) + (320 - 8);
		if ((uint16_t)x >= (640 - 16))
			continue;

		// render star pixel if the pixel under it is the background
		screenBufferPos = ((y + 4) * SCREEN_W) + (x + 4);
		if ((video.frameBuffer[screenBufferPos] >> 24) == PAL_BCKGRND)
		{
			col = ((uint8_t)~(z >> 8) >> 3) - (22 - 8);
			if (col < 24)
			{
				video.frameBuffer[screenBufferPos] = video.palette[starColConv[col]] & 0xFFFFFF;
				lastStarScreenPos[i] = screenBufferPos;
			}
		}
	}
}

void aboutFrame(void)
{
	star_a.x += (3 * 64);
	star_a.y += (2 * 64);
	star_a.z -= (1 * 64);

	fixaMatris(star_a, &starmat);

	realStars();
}

void showAboutScreen(void) // called once when About screen is opened
{
	const char *infoString = "Clone by Olav \"8bitbubsy\" S\025rensen - https://16-bits.org";
	const char *extraInfo = "SDL1.2/BigEndian version by Marlon Beijer @ AmigaDev.com";
	char betaText[32];
	uint16_t x, y;

	if (editor.ui.extended)
		exitPatternEditorExtended();

	hideTopScreen();

	drawFramework(0, 0, 632, 173, FRAMEWORK_TYPE1);
	drawFramework(2, 2, 628, 169, FRAMEWORK_TYPE2);

	showPushButton(PB_EXIT_ABOUT);

	blit32(91, 31, ft2Logo, FT2_LOGO_W, FT2_LOGO_H);
    blit(146, 113, aboutText, ABOUT_TEXT_W, ABOUT_TEXT_H);

	x = 5 + (SCREEN_W - textWidth(infoString)) / 2;
	y = 148;
	textOut(x, y, PAL_FORGRND, infoString);

	x = 5 + (SCREEN_W - textWidth(extraInfo)) / 2;
	y = 160;
	textOut(x, y, PAL_FORGRND, extraInfo);


	sprintf(betaText, "beta v.%d", BETA_VERSION);
	x = (3 + ABOUT_SCREEN_W) - (textWidth(betaText) + 3);
	y = (3 + ABOUT_SCREEN_H) - ((FONT1_CHAR_H - 2) + 2);
	textOut(x, y, PAL_FORGRND, betaText);

	aboutInit();

	editor.ui.aboutScreenShown = true;
}

void hideAboutScreen(void)
{
	hidePushButton(PB_EXIT_ABOUT);
	editor.ui.aboutScreenShown = false;
}

void exitAboutScreen(void)
{
	hideAboutScreen();
	showTopScreen(true);
}

const uint16_t sinusTables[256 * 5] =
{
	0x0000,0x00C9,0x0192,0x025B,0x0324,0x03ED,0x04B6,0x057F,0x0648,0x0711,0x07D9,0x08A2,0x096A,0x0A33,0x0AFB,0x0BC3,
	0x0C8C,0x0D54,0x0E1C,0x0EE3,0x0FAB,0x1072,0x113A,0x1201,0x12C8,0x138F,0x1455,0x151C,0x15E2,0x16A8,0x176D,0x1833,
	0x18F8,0x19BD,0x1A82,0x1B47,0x1C0B,0x1CCF,0x1D93,0x1E56,0x1F19,0x1FDC,0x209F,0x2161,0x2223,0x22E5,0x23A6,0x2467,
	0x2527,0x25E8,0x26A8,0x2767,0x2826,0x28E5,0x29A3,0x2A61,0x2B1F,0x2BDC,0x2C98,0x2D55,0x2E10,0x2ECC,0x2F87,0x3041,
	0x30FB,0x31B5,0x326E,0x3326,0x33DE,0x3496,0x354D,0x3603,0x36B9,0x376F,0x3824,0x38D8,0x398C,0x3A3F,0x3AF2,0x3BA4,
	0x3C56,0x3D07,0x3DB7,0x3E67,0x3F16,0x3FC5,0x4073,0x4120,0x41CD,0x4279,0x4325,0x43D0,0x447A,0x4523,0x45CC,0x4674,
	0x471C,0x47C3,0x4869,0x490E,0x49B3,0x4A57,0x4AFA,0x4B9D,0x4C3F,0x4CE0,0x4D80,0x4E20,0x4EBF,0x4F5D,0x4FFA,0x5097,
	0x5133,0x51CE,0x5268,0x5301,0x539A,0x5432,0x54C9,0x555F,0x55F4,0x5689,0x571D,0x57AF,0x5841,0x58D3,0x5963,0x59F2,
	0x5A81,0x5B0F,0x5B9C,0x5C28,0x5CB3,0x5D3D,0x5DC6,0x5E4F,0x5ED6,0x5F5D,0x5FE2,0x6067,0x60EB,0x616E,0x61EF,0x6270,
	0x62F0,0x6370,0x63EE,0x646B,0x64E7,0x6562,0x65DC,0x6656,0x66CE,0x6745,0x67BB,0x6831,0x68A5,0x6918,0x698A,0x69FC,
	0x6A6C,0x6ADB,0x6B49,0x6BB6,0x6C22,0x6C8E,0x6CF8,0x6D60,0x6DC8,0x6E2F,0x6E95,0x6EFA,0x6F5D,0x6FC0,0x7021,0x7082,
	0x70E1,0x713F,0x719C,0x71F8,0x7253,0x72AD,0x7306,0x735E,0x73B4,0x740A,0x745E,0x74B1,0x7503,0x7554,0x75A4,0x75F2,
	0x7640,0x768C,0x76D7,0x7722,0x776A,0x77B2,0x77F9,0x783E,0x7883,0x78C6,0x7908,0x7949,0x7988,0x79C7,0x7A04,0x7A40,
	0x7A7B,0x7AB5,0x7AED,0x7B25,0x7B5B,0x7B90,0x7BC4,0x7BF7,0x7C28,0x7C58,0x7C87,0x7CB5,0x7CE2,0x7D0D,0x7D38,0x7D61,
	0x7D88,0x7DAF,0x7DD4,0x7DF9,0x7E1C,0x7E3D,0x7E5E,0x7E7D,0x7E9B,0x7EB8,0x7ED4,0x7EEE,0x7F08,0x7F20,0x7F36,0x7F4C,
	0x7F60,0x7F73,0x7F85,0x7F96,0x7FA5,0x7FB3,0x7FC0,0x7FCC,0x7FD7,0x7FE0,0x7FE8,0x7FEF,0x7FF4,0x7FF8,0x7FFC,0x7FFD,
	0x7FFE,0x7FFD,0x7FFC,0x7FF8,0x7FF4,0x7FEF,0x7FE8,0x7FE0,0x7FD7,0x7FCC,0x7FC0,0x7FB3,0x7FA5,0x7F96,0x7F85,0x7F73,
	0x7F60,0x7F4C,0x7F36,0x7F20,0x7F08,0x7EEE,0x7ED4,0x7EB8,0x7E9B,0x7E7D,0x7E5E,0x7E3D,0x7E1C,0x7DF9,0x7DD4,0x7DAF,
	0x7D88,0x7D61,0x7D38,0x7D0D,0x7CE2,0x7CB5,0x7C87,0x7C58,0x7C28,0x7BF7,0x7BC4,0x7B90,0x7B5B,0x7B25,0x7AED,0x7AB5,
	0x7A7B,0x7A40,0x7A04,0x79C7,0x7988,0x7949,0x7908,0x78C6,0x7883,0x783E,0x77F9,0x77B2,0x776A,0x7722,0x76D7,0x768C,
	0x7640,0x75F2,0x75A4,0x7554,0x7503,0x74B1,0x745E,0x740A,0x73B4,0x735E,0x7306,0x72AD,0x7253,0x71F8,0x719C,0x713F,
	0x70E1,0x7082,0x7021,0x6FC0,0x6F5D,0x6EFA,0x6E95,0x6E2F,0x6DC8,0x6D60,0x6CF8,0x6C8E,0x6C22,0x6BB6,0x6B49,0x6ADB,
	0x6A6C,0x69FC,0x698A,0x6918,0x68A5,0x6831,0x67BB,0x6745,0x66CE,0x6656,0x65DC,0x6562,0x64E7,0x646B,0x63EE,0x6370,
	0x62F0,0x6270,0x61EF,0x616E,0x60EB,0x6067,0x5FE2,0x5F5D,0x5ED6,0x5E4F,0x5DC6,0x5D3D,0x5CB3,0x5C28,0x5B9C,0x5B0F,
	0x5A81,0x59F2,0x5963,0x58D3,0x5841,0x57AF,0x571D,0x5689,0x55F4,0x555F,0x54C9,0x5432,0x539A,0x5301,0x5268,0x51CE,
	0x5133,0x5097,0x4FFA,0x4F5D,0x4EBF,0x4E20,0x4D80,0x4CE0,0x4C3F,0x4B9D,0x4AFA,0x4A57,0x49B3,0x490E,0x4869,0x47C3,
	0x471C,0x4674,0x45CC,0x4523,0x447A,0x43D0,0x4325,0x4279,0x41CD,0x4120,0x4073,0x3FC5,0x3F16,0x3E67,0x3DB7,0x3D07,
	0x3C56,0x3BA4,0x3AF2,0x3A3F,0x398C,0x38D8,0x3824,0x376F,0x36B9,0x3603,0x354D,0x3496,0x33DE,0x3326,0x326E,0x31B5,
	0x30FB,0x3041,0x2F87,0x2ECC,0x2E10,0x2D55,0x2C98,0x2BDC,0x2B1F,0x2A61,0x29A3,0x28E5,0x2826,0x2767,0x26A8,0x25E8,
	0x2527,0x2467,0x23A6,0x22E5,0x2223,0x2161,0x209F,0x1FDC,0x1F19,0x1E56,0x1D93,0x1CCF,0x1C0B,0x1B47,0x1A82,0x19BD,
	0x18F8,0x1833,0x176D,0x16A8,0x15E2,0x151C,0x1455,0x138F,0x12C8,0x1201,0x113A,0x1072,0x0FAB,0x0EE3,0x0E1C,0x0D54,
	0x0C8C,0x0BC3,0x0AFB,0x0A33,0x096A,0x08A2,0x07D9,0x0711,0x0648,0x057F,0x04B6,0x03ED,0x0324,0x025B,0x0192,0x00C9,
	0x0000,0xFF37,0xFE6E,0xFDA5,0xFCDC,0xFC13,0xFB4A,0xFA81,0xF9B8,0xF8EF,0xF827,0xF75E,0xF696,0xF5CD,0xF505,0xF43D,
	0xF374,0xF2AC,0xF1E4,0xF11D,0xF055,0xEF8E,0xEEC6,0xEDFF,0xED38,0xEC71,0xEBAB,0xEAE4,0xEA1E,0xE958,0xE893,0xE7CD,
	0xE708,0xE643,0xE57E,0xE4B9,0xE3F5,0xE331,0xE26D,0xE1AA,0xE0E7,0xE024,0xDF61,0xDE9F,0xDDDD,0xDD1B,0xDC5A,0xDB99,
	0xDAD9,0xDA18,0xD958,0xD899,0xD7DA,0xD71B,0xD65D,0xD59F,0xD4E1,0xD424,0xD368,0xD2AB,0xD1F0,0xD134,0xD079,0xCFBF,
	0xCF05,0xCE4B,0xCD92,0xCCDA,0xCC22,0xCB6A,0xCAB3,0xC9FD,0xC947,0xC891,0xC7DC,0xC728,0xC674,0xC5C1,0xC50E,0xC45C,
	0xC3AA,0xC2F9,0xC249,0xC199,0xC0EA,0xC03B,0xBF8D,0xBEE0,0xBE33,0xBD87,0xBCDB,0xBC30,0xBB86,0xBADD,0xBA34,0xB98C,
	0xB8E4,0xB83D,0xB797,0xB6F2,0xB64D,0xB5A9,0xB506,0xB463,0xB3C1,0xB320,0xB280,0xB1E0,0xB141,0xB0A3,0xB006,0xAF69,
	0xAECD,0xAE32,0xAD98,0xACFF,0xAC66,0xABCE,0xAB37,0xAAA1,0xAA0C,0xA977,0xA8E3,0xA851,0xA7BF,0xA72D,0xA69D,0xA60E,
	0xA57F,0xA4F1,0xA464,0xA3D8,0xA34D,0xA2C3,0xA23A,0xA1B1,0xA12A,0xA0A3,0xA01E,0x9F99,0x9F15,0x9E92,0x9E11,0x9D90,
	0x9D10,0x9C90,0x9C12,0x9B95,0x9B19,0x9A9E,0x9A24,0x99AA,0x9932,0x98BB,0x9845,0x97CF,0x975B,0x96E8,0x9676,0x9604,
	0x9594,0x9525,0x94B7,0x944A,0x93DE,0x9372,0x9308,0x92A0,0x9238,0x91D1,0x916B,0x9106,0x90A3,0x9040,0x8FDF,0x8F7E,
	0x8F1F,0x8EC1,0x8E64,0x8E08,0x8DAD,0x8D53,0x8CFA,0x8CA2,0x8C4C,0x8BF6,0x8BA2,0x8B4F,0x8AFD,0x8AAC,0x8A5C,0x8A0E,
	0x89C0,0x8974,0x8929,0x88DE,0x8896,0x884E,0x8807,0x87C2,0x877D,0x873A,0x86F8,0x86B7,0x8678,0x8639,0x85FC,0x85C0,
	0x8585,0x854B,0x8513,0x84DB,0x84A5,0x8470,0x843C,0x8409,0x83D8,0x83A8,0x8379,0x834B,0x831E,0x82F3,0x82C8,0x829F,
	0x8278,0x8251,0x822C,0x8207,0x81E4,0x81C3,0x81A2,0x8183,0x8165,0x8148,0x812C,0x8112,0x80F8,0x80E0,0x80CA,0x80B4,
	0x80A0,0x808D,0x807B,0x806A,0x805B,0x804D,0x8040,0x8034,0x8029,0x8020,0x8018,0x8011,0x800C,0x8008,0x8004,0x8003,
	0x8002,0x8003,0x8004,0x8008,0x800C,0x8011,0x8018,0x8020,0x8029,0x8034,0x8040,0x804D,0x805B,0x806A,0x807B,0x808D,
	0x80A0,0x80B4,0x80CA,0x80E0,0x80F8,0x8112,0x812C,0x8148,0x8165,0x8183,0x81A2,0x81C3,0x81E4,0x8207,0x822C,0x8251,
	0x8278,0x829F,0x82C8,0x82F3,0x831E,0x834B,0x8379,0x83A8,0x83D8,0x8409,0x843C,0x8470,0x84A5,0x84DB,0x8513,0x854B,
	0x8585,0x85C0,0x85FC,0x8639,0x8678,0x86B7,0x86F8,0x873A,0x877D,0x87C2,0x8807,0x884E,0x8896,0x88DE,0x8929,0x8974,
	0x89C0,0x8A0E,0x8A5C,0x8AAC,0x8AFD,0x8B4F,0x8BA2,0x8BF6,0x8C4C,0x8CA2,0x8CFA,0x8D53,0x8DAD,0x8E08,0x8E64,0x8EC1,
	0x8F1F,0x8F7E,0x8FDF,0x9040,0x90A3,0x9106,0x916B,0x91D1,0x9238,0x92A0,0x9308,0x9372,0x93DE,0x944A,0x94B7,0x9525,
	0x9594,0x9604,0x9676,0x96E8,0x975B,0x97CF,0x9845,0x98BB,0x9932,0x99AA,0x9A24,0x9A9E,0x9B19,0x9B95,0x9C12,0x9C90,
	0x9D10,0x9D90,0x9E11,0x9E92,0x9F15,0x9F99,0xA01E,0xA0A3,0xA12A,0xA1B1,0xA23A,0xA2C3,0xA34D,0xA3D8,0xA464,0xA4F1,
	0xA57F,0xA60E,0xA69D,0xA72D,0xA7BF,0xA851,0xA8E3,0xA977,0xAA0C,0xAAA1,0xAB37,0xABCE,0xAC66,0xACFF,0xAD98,0xAE32,
	0xAECD,0xAF69,0xB006,0xB0A3,0xB141,0xB1E0,0xB280,0xB320,0xB3C1,0xB463,0xB506,0xB5A9,0xB64D,0xB6F2,0xB797,0xB83D,
	0xB8E4,0xB98C,0xBA34,0xBADD,0xBB86,0xBC30,0xBCDB,0xBD87,0xBE33,0xBEE0,0xBF8D,0xC03B,0xC0EA,0xC199,0xC249,0xC2F9,
	0xC3AA,0xC45C,0xC50E,0xC5C1,0xC674,0xC728,0xC7DC,0xC891,0xC947,0xC9FD,0xCAB3,0xCB6A,0xCC22,0xCCDA,0xCD92,0xCE4B,
	0xCF05,0xCFBF,0xD079,0xD134,0xD1F0,0xD2AB,0xD368,0xD424,0xD4E1,0xD59F,0xD65D,0xD71B,0xD7DA,0xD899,0xD958,0xDA18,
	0xDAD9,0xDB99,0xDC5A,0xDD1B,0xDDDD,0xDE9F,0xDF61,0xE024,0xE0E7,0xE1AA,0xE26D,0xE331,0xE3F5,0xE4B9,0xE57E,0xE643,
	0xE708,0xE7CD,0xE893,0xE958,0xEA1E,0xEAE4,0xEBAB,0xEC71,0xED38,0xEDFF,0xEEC6,0xEF8E,0xF055,0xF11D,0xF1E4,0xF2AC,
	0xF374,0xF43D,0xF505,0xF5CD,0xF696,0xF75E,0xF827,0xF8EF,0xF9B8,0xFA81,0xFB4A,0xFC13,0xFCDC,0xFDA5,0xFE6E,0xFF37,
	0x0000,0x00C9,0x0192,0x025B,0x0324,0x03ED,0x04B6,0x057F,0x0648,0x0711,0x07D9,0x08A2,0x096A,0x0A33,0x0AFB,0x0BC3,
	0x0C8C,0x0D54,0x0E1C,0x0EE3,0x0FAB,0x1072,0x113A,0x1201,0x12C8,0x138F,0x1455,0x151C,0x15E2,0x16A8,0x176D,0x1833,
	0x18F8,0x19BD,0x1A82,0x1B47,0x1C0B,0x1CCF,0x1D93,0x1E56,0x1F19,0x1FDC,0x209F,0x2161,0x2223,0x22E5,0x23A6,0x2467,
	0x2527,0x25E8,0x26A8,0x2767,0x2826,0x28E5,0x29A3,0x2A61,0x2B1F,0x2BDC,0x2C98,0x2D55,0x2E10,0x2ECC,0x2F87,0x3041,
	0x30FB,0x31B5,0x326E,0x3326,0x33DE,0x3496,0x354D,0x3603,0x36B9,0x376F,0x3824,0x38D8,0x398C,0x3A3F,0x3AF2,0x3BA4,
	0x3C56,0x3D07,0x3DB7,0x3E67,0x3F16,0x3FC5,0x4073,0x4120,0x41CD,0x4279,0x4325,0x43D0,0x447A,0x4523,0x45CC,0x4674,
	0x471C,0x47C3,0x4869,0x490E,0x49B3,0x4A57,0x4AFA,0x4B9D,0x4C3F,0x4CE0,0x4D80,0x4E20,0x4EBF,0x4F5D,0x4FFA,0x5097,
	0x5133,0x51CE,0x5268,0x5301,0x539A,0x5432,0x54C9,0x555F,0x55F4,0x5689,0x571D,0x57AF,0x5841,0x58D3,0x5963,0x59F2,
	0x5A81,0x5B0F,0x5B9C,0x5C28,0x5CB3,0x5D3D,0x5DC6,0x5E4F,0x5ED6,0x5F5D,0x5FE2,0x6067,0x60EB,0x616E,0x61EF,0x6270,
	0x62F0,0x6370,0x63EE,0x646B,0x64E7,0x6562,0x65DC,0x6656,0x66CE,0x6745,0x67BB,0x6831,0x68A5,0x6918,0x698A,0x69FC,
	0x6A6C,0x6ADB,0x6B49,0x6BB6,0x6C22,0x6C8E,0x6CF8,0x6D60,0x6DC8,0x6E2F,0x6E95,0x6EFA,0x6F5D,0x6FC0,0x7021,0x7082,
	0x70E1,0x713F,0x719C,0x71F8,0x7253,0x72AD,0x7306,0x735E,0x73B4,0x740A,0x745E,0x74B1,0x7503,0x7554,0x75A4,0x75F2,
	0x7640,0x768C,0x76D7,0x7722,0x776A,0x77B2,0x77F9,0x783E,0x7883,0x78C6,0x7908,0x7949,0x7988,0x79C7,0x7A04,0x7A40,
	0x7A7B,0x7AB5,0x7AED,0x7B25,0x7B5B,0x7B90,0x7BC4,0x7BF7,0x7C28,0x7C58,0x7C87,0x7CB5,0x7CE2,0x7D0D,0x7D38,0x7D61,
	0x7D88,0x7DAF,0x7DD4,0x7DF9,0x7E1C,0x7E3D,0x7E5E,0x7E7D,0x7E9B,0x7EB8,0x7ED4,0x7EEE,0x7F08,0x7F20,0x7F36,0x7F4C,
	0x7F60,0x7F73,0x7F85,0x7F96,0x7FA5,0x7FB3,0x7FC0,0x7FCC,0x7FD7,0x7FE0,0x7FE8,0x7FEF,0x7FF4,0x7FF8,0x7FFC,0x7FFD
};
