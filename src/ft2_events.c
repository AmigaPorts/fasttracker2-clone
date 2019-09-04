/* for finding memory leaks in debug mode with Visual Studio */
#if defined _DEBUG && defined _MSC_VER
#include <crtdbg.h>
#endif

#ifdef _WIN32
#define WIN32_MEAN_AND_LEAN
#include <windows.h>
#include <SDL2/SDL_syswm.h>
#else
#include <limits.h> /* UINT_MAX etc */
#include <signal.h>
#include <unistd.h> /* chdir() */
#endif
#include <stdio.h>
#include <sys/stat.h>
#include "ft2_header.h"
#include "ft2_config.h"
#include "ft2_diskop.h"
#include "ft2_module_loader.h"
#include "ft2_module_saver.h"
#include "ft2_sample_loader.h"
#include "ft2_mouse.h"
#include "ft2_midi.h"
#include "ft2_video.h"
#include "ft2_trim.h"
#include "ft2_inst_ed.h"
#include "ft2_sampling.h"
#include "ft2_textboxes.h"
#include "ft2_sysreqs.h"
#include "ft2_keyboard.h"
#include "ft2_sample_ed.h"
#include "ft2_sample_ed_features.h"

#define CRASH_TEXT "Oh no!\nThe Fasttracker II clone has crashed...\n\nA backup .xm was hopefully " \
                   "saved to the current module directory.\n\nPlease report this to 8bitbubsy " \
                   "(IRC or olav.sorensen@live.no).\nTry to mention what you did before the crash happened."

static bool backupMadeAfterCrash;

#ifdef _WIN32
#define SYSMSG_FILE_ARG (WM_USER + 1)
#define ARGV_SHARED_MEM_MAX_LEN ((MAX_PATH * 2) + 2)
#define SHARED_HWND_NAME TEXT("Local\\FT2CloneHwnd")
#define SHARED_FILENAME TEXT("Local\\FT2CloneFilename")
static HWND hWnd;
static HANDLE oneInstHandle, hMapFile;
static LPCTSTR sharedMemBuf;

/* used for Windows usleep() implementation */
static NTSTATUS (__stdcall *NtDelayExecution)(BOOL Alertable, PLARGE_INTEGER DelayInterval);
#endif

static void handleInput(void);

/* usleep() implementation for Windows */
#ifdef _WIN32
void usleep(uint32_t usec)
{
    LARGE_INTEGER lpDueTime;

    if (NtDelayExecution == NULL)
    {
        /* NtDelayExecution() is not available (shouldn't happen), use regular sleep() */
        Sleep((uint32_t)((usec / 1000.0) + 0.5));
    }
    else
    {
        /* this prevents a 64-bit MUL (will not overflow with typical values anyway) */
        lpDueTime.HighPart = 0xFFFFFFFF;
        lpDueTime.LowPart  = (DWORD)(-10 * (int32_t)(usec));

        NtDelayExecution(false, &lpDueTime);
    }
}

void setupWin32Usleep(void)
{
    NtDelayExecution = (NTSTATUS (__stdcall *)(BOOL, PLARGE_INTEGER))(GetProcAddress(GetModuleHandle("ntdll.dll"), "NtDelayExecution"));
    timeBeginPeriod(0); /* enter highest timer resolution */
}

void freeWin32Usleep(void)
{
    timeEndPeriod(0); /* exit highest timer resolution */
}
#endif

void readInput(void)
{
    readMouseXY();
    readKeyModifiers();
    setSyncedReplayerVars();
    handleInput();
}

void handleThreadEvents(void)
{
    if (okBoxData.active)
    {
        okBoxData.returnData = okBox(okBoxData.typ, okBoxData.headline, okBoxData.text);
        okBoxData.active = false;
    }
}

void handleEvents(void)
{
    /* called after MIDI has been initialized */
    if (midi.rescanDevicesFlag)
    {
        midi.rescanDevicesFlag = false;

        rescanMidiInputDevices();
        if (editor.ui.configScreenShown && (editor.currConfigScreen == CONFIG_SCREEN_MIDI_INPUT))
            drawMidiInputList();
    }

    if (editor.trimThreadWasDone)
    {
        editor.trimThreadWasDone = false;
        trimThreadDone();
    }

    if (editor.updateCurSmp)
    {
        editor.updateCurSmp = false;

        updateNewInstrument();
        updateNewSample();

        diskOpSetFilename(DISKOP_ITEM_SAMPLE, editor.tmpFilenameU);

        removeSampleIsLoadingFlag();
        setMouseBusy(false);
    }

    if (editor.updateCurInstr)
    {
        editor.updateCurInstr = false;

        updateNewInstrument();
        updateNewSample();

        diskOpSetFilename(DISKOP_ITEM_INSTR, editor.tmpInstrFilenameU);
        setMouseBusy(false);
    }

    /* some Disk Op. stuff */

    if (editor.diskOpReadDir)
    {
        editor.diskOpReadDir = false;
        startDiskOpFillThread();
    }

    if (editor.diskOpReadDone)
    {
        editor.diskOpReadDone = false;
        if (editor.ui.diskOpShown)
            diskOp_DrawDirectory();
    }

    handleLoadMusicEvents();

    if (editor.samplingAudioFlag) handleSamplingUpdates();
    if (editor.ui.setMouseBusy)   mouseAnimOn();
    if (editor.ui.setMouseIdle)   mouseAnimOff();

    if (editor.updateWindowTitle)
    {
        editor.updateWindowTitle = false;
        updateWindowTitle(false);
    }
}

/* Windows specific routines */
#ifdef _WIN32
static bool instanceAlreadyOpen(void)
{
    hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, SHARED_HWND_NAME);
    if (hMapFile != NULL)
        return (true); /* another instance is already open */

    /* no instance is open, let's created a shared memory file with hWnd in it */
    oneInstHandle = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof (HWND), SHARED_HWND_NAME);
    if (oneInstHandle != NULL)
    {
        sharedMemBuf = (LPTSTR)(MapViewOfFile(oneInstHandle, FILE_MAP_ALL_ACCESS, 0, 0, sizeof (HWND)));
        if (sharedMemBuf != NULL)
        {
            CopyMemory((PVOID)(sharedMemBuf), &video.hWnd, sizeof (HWND));
            UnmapViewOfFile(sharedMemBuf);
            sharedMemBuf = NULL;
        }
    }

    return (false);
}

bool handleSingleInstancing(int32_t argc, char **argv)
{
    SDL_SysWMinfo wmInfo;

    SDL_VERSION(&wmInfo.version);
    if (!SDL_GetWindowWMInfo(video.window, &wmInfo))
        return (false);

    video.hWnd = wmInfo.info.win.window;
    if (instanceAlreadyOpen())
    {
        if ((argc >= 2) && (argv[1][0] != '\0'))
        {
            sharedMemBuf = (LPTSTR)(MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof (HWND)));
            if (sharedMemBuf != NULL)
            {
                memcpy(&hWnd, sharedMemBuf, sizeof (HWND));
                UnmapViewOfFile(sharedMemBuf);
                sharedMemBuf = NULL;

                CloseHandle(hMapFile);
                hMapFile = NULL;

                hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, ARGV_SHARED_MEM_MAX_LEN, SHARED_FILENAME);
                if (hMapFile != NULL)
                {
                    sharedMemBuf = (LPTSTR)(MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, ARGV_SHARED_MEM_MAX_LEN));
                    if (sharedMemBuf != NULL)
                    {
                        strcpy((char *)(sharedMemBuf), argv[1]);
                        UnmapViewOfFile(sharedMemBuf);
                        sharedMemBuf = NULL;

                        SendMessage(hWnd, SYSMSG_FILE_ARG, 0, 0);
                        Sleep(80); /* wait a bit to make sure first instance received msg */

                        CloseHandle(hMapFile);
                        hMapFile = NULL;

                        return (true); /* quit instance now */
                    }
                }

                return (true);
            }

            CloseHandle(hMapFile);
            hMapFile = NULL;
        }
    }

    SDL_EventState(SDL_SYSWMEVENT, SDL_ENABLE);
    return (false);
}

static void handleSysMsg(SDL_Event inputEvent)
{
    SDL_SysWMmsg *wmMsg;

    if (inputEvent.type == SDL_SYSWMEVENT)
    {
        wmMsg = inputEvent.syswm.msg;
        if ((wmMsg->subsystem == SDL_SYSWM_WINDOWS) && (wmMsg->msg.win.msg == SYSMSG_FILE_ARG))
        {
            hMapFile = OpenFileMapping(FILE_MAP_READ, FALSE, SHARED_FILENAME);
            if (hMapFile != NULL)
            {
                sharedMemBuf = (LPTSTR)(MapViewOfFile(hMapFile, FILE_MAP_READ, 0, 0, ARGV_SHARED_MEM_MAX_LEN));
                if (sharedMemBuf != NULL)
                {
                    editor.autoPlayOnDrop = true;
                    loadDroppedFile((char *)(sharedMemBuf), true);

                    UnmapViewOfFile(sharedMemBuf);
                    sharedMemBuf = NULL;

                    //SDL_RestoreWindow(video.window);
                }

                CloseHandle(hMapFile);
                hMapFile = NULL;
            }
        }
    }
}

void closeSingleInstancing(void)
{
    if (oneInstHandle != NULL)
    {
        CloseHandle(oneInstHandle);
        oneInstHandle = NULL;
    }
}

static LONG WINAPI exceptionHandler(EXCEPTION_POINTERS *ptr)
{
#define BACKUP_FILES_TO_TRY 1000
    char fileName[32];
    uint16_t i;
    UNICHAR *fileNameU;
    struct stat statBuffer;

    (void)(ptr);

    if (oneInstHandle != NULL)
        CloseHandle(oneInstHandle);

    if (!backupMadeAfterCrash)
    {
        if ((getDiskOpModPath() != NULL) && (UNICHAR_CHDIR(getDiskOpModPath()) == 0))
        {
            /* find a free filename */
            for (i = 1; i < 1000; ++i)
            {
                sprintf(fileName, "backup%03d.xm", i);
                if (stat(fileName, &statBuffer) != 0)
                    break; /* filename OK */
            }

            if (i != 1000)
            {
                fileNameU = cp437ToUnichar(fileName);
                if (fileNameU != NULL)
                {
                    saveXM(fileNameU);
                    free(fileNameU);
                }
            }
        }

        backupMadeAfterCrash = true; /* set this flag to prevent multiple backups from being saved at once */
        showErrorMsgBox(CRASH_TEXT);
    }

    return (EXCEPTION_CONTINUE_SEARCH);
}
#else
static void exceptionHandler(int32_t signal)
{
#define BACKUP_FILES_TO_TRY 1000
    char fileName[32];
    uint16_t i;
    UNICHAR *fileNameU;
    struct stat statBuffer;

    if (signal == 15)
        return;

    if (!backupMadeAfterCrash)
    {
        if ((getDiskOpModPath() != NULL) && (UNICHAR_CHDIR(getDiskOpModPath()) == 0))
        {
            /* find a free filename */
            for (i = 1; i < 1000; ++i)
            {
                sprintf(fileName, "backup%03d.xm", i);
                if (stat(fileName, &statBuffer) != 0)
                    break; /* filename OK */
            }

            if (i != 1000)
            {
                fileNameU = cp437ToUnichar(fileName);
                if (fileNameU != NULL)
                {
                    saveXM(fileNameU);
                    free(fileNameU);
                }
            }
        }

        backupMadeAfterCrash = true; /* set this flag to prevent multiple backups from being saved at once */
        showErrorMsgBox(CRASH_TEXT);
    }
}
#endif

void setupCrashHandler(void)
{
#ifndef _DEBUG
#ifdef _WIN32
    SetUnhandledExceptionFilter(exceptionHandler);
#else
    struct sigaction act;
    struct sigaction oldAct;

    memset(&act, 0, sizeof (act));
    act.sa_handler = exceptionHandler;
    act.sa_flags   = SA_RESETHAND;

    sigaction(SIGILL | SIGABRT | SIGFPE | SIGSEGV, &act, &oldAct);
    sigaction(SIGILL,  &act, &oldAct);
    sigaction(SIGABRT, &act, &oldAct);
    sigaction(SIGFPE,  &act, &oldAct);
    sigaction(SIGSEGV, &act, &oldAct);
#endif
#endif
}

static void handleInput(void)
{
    char *inputText;
    uint8_t vibDepth;
    uint32_t eventType;
    SDL_Event inputEvent;
    SDL_Keycode key;

    if (!editor.busy)
        handleLastGUIObjectDown(); /* this should be handled before main input poll (on next frame) */

    while (SDL_PollEvent(&inputEvent))
    {
        if (editor.busy)
        {
            eventType = inputEvent.type;
            key = inputEvent.key.keysym.scancode;

            /* the Echo tool in Smp. Ed. can literally take forever if abused,
            ** let mouse buttons/ESC/SIGTERM force-stop it.
            */
            if ((eventType == SDL_MOUSEBUTTONDOWN) || (eventType == SDL_QUIT) ||
                ((eventType == SDL_KEYUP) && (key == SDL_SCANCODE_ESCAPE)))
            {
                handleEchoToolPanic();
            }

            /* let certain mouse buttons or keyboard keys stop certain events */
            if ((eventType == SDL_MOUSEBUTTONDOWN) || ((eventType == SDL_KEYDOWN) &&
                (key != SDL_SCANCODE_MUTE) &&
                (key != SDL_SCANCODE_AUDIOMUTE) &&
                (key != SDL_SCANCODE_VOLUMEDOWN) &&
                (key != SDL_SCANCODE_VOLUMEUP)))
            {
                /* only let keyboard keys interrupt audio sampling */
                if (editor.samplingAudioFlag && (eventType != SDL_MOUSEBUTTONDOWN))
                    stopSampling();

                editor.wavIsRendering = false;
            }

            continue; /* another thread is busy with something, drop input */
        }

#ifdef _WIN32
        handleSysMsg(inputEvent);
#endif
        /* text input when editing texts */
        if (inputEvent.type == SDL_TEXTINPUT)
        {
            if (editor.editTextFlag)
            {
                if (keyb.ignoreTextEditKey)
                {
                    keyb.ignoreTextEditKey = false;
                    continue;
                }

                inputText = utf8ToCp437(inputEvent.text.text, false);
                if (inputText != NULL)
                {
                    if (inputText[0] != '\0')
                        handleTextEditInputChar(inputText[0]);

                    free(inputText);
                }
            }
        }
        else if (inputEvent.type == SDL_MOUSEWHEEL)
        {
                 if (inputEvent.wheel.y > 0) mouseWheelHandler(MOUSE_WHEEL_UP);
            else if (inputEvent.wheel.y < 0) mouseWheelHandler(MOUSE_WHEEL_DOWN);
        }
        else if (inputEvent.type == SDL_DROPFILE)
        {
            editor.autoPlayOnDrop = false;
            loadDroppedFile(inputEvent.drop.file, true);
            SDL_free(inputEvent.drop.file);
        }
        else if (inputEvent.type == SDL_QUIT)
        {
            if (editor.ui.sysReqShown)
                continue;

            if (editor.editTextFlag)
                exitTextEditing();

            if (!song.isModified)
            {
                editor.throwExit = true;
            }
            else
            {
                if (!video.fullscreen)
                {
                    /* de-minimize window and set focus so that the user sees the message box */
                    SDL_RestoreWindow(video.window);
                    SDL_RaiseWindow(video.window);
                }

                if (quitBox(true) == 1)
                    editor.throwExit = true;
            }
        }
        else if (inputEvent.type == SDL_KEYUP)
        {
            keyUpHandler(inputEvent.key.keysym.scancode, inputEvent.key.keysym.sym);
        }
        else if (inputEvent.type == SDL_KEYDOWN)
        {
            keyDownHandler(inputEvent.key.keysym.scancode, inputEvent.key.keysym.sym, inputEvent.key.repeat);
        }
        else if (inputEvent.type == SDL_MOUSEBUTTONUP)
        {
            mouseButtonUpHandler(inputEvent.button.button);
        }
        else if (inputEvent.type == SDL_MOUSEBUTTONDOWN)
        {
            mouseButtonDownHandler(inputEvent.button.button);
        }

        if (editor.throwExit)
            editor.programRunning = false;
    }

    /* MIDI vibrato */
    vibDepth = (midi.currMIDIVibDepth >> 9) & 0x0F;
    if (vibDepth > 0)
        recordMIDIEffect(0x04, 0xA0 | vibDepth);
}
