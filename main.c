#include <stdbool.h>
#include <stdatomic.h>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <winternl.h>

// created via random.org
#define MUTEX "InputSwitch_w3BYxWnv8rnOgoP6RsNE"

NTSYSAPI
NTSTATUS
NTAPI
RtlGetVersion(
    _Out_ PRTL_OSVERSIONINFOW lpVersionInformation
);

#ifdef _DEBUG
NTSYSAPI
ULONG
DbgPrintEx(
    ULONG ComponentId,
    ULONG Level,
    PCSTR Format,
    ...
);

#define DebugMsg(format, ...) DbgPrintEx(-1, 0, (format), ##__VA_ARGS__)
#define DebugMsgIf(cond, format, ...) if (cond) DebugMsg((format), ##__VA_ARGS__)
#else
#define DebugMsg(format, ...)
#define DebugMsgIf(cond, format, ...)
#endif

static HHOOK hHook = NULL;
static atomic_bool hooked = false;

static LRESULT CALLBACK LowLevelKeyboardProc(
    _In_ int nCode,
    _In_ WPARAM kbMsgId,
    _In_ LPARAM pEvent
) {
  const KBDLLHOOKSTRUCT *hook = (LPKBDLLHOOKSTRUCT) pEvent;
  DebugMsg("Captured: nCode=%d, kbMsgId=%llx, vkCode=%lx", nCode, kbMsgId, hook->vkCode);
  if (nCode == HC_ACTION && kbMsgId == WM_KEYDOWN && hook->vkCode == VK_SPACE) {
    const SHORT ctrl = GetKeyState(VK_CONTROL);
    const SHORT lWin = GetKeyState(VK_LWIN);
    const SHORT rWin = GetKeyState(VK_RWIN);
    DebugMsg("Captured: ctrl=%hd, lWin=%hd, rWin=%hd", ctrl, lWin, rWin);
    if (ctrl < 0 && lWin >= 0 && rWin >= 0) {
      const LPARAM extraInfo = GetMessageExtraInfo();
      INPUT input[1] = {
          {
              .type = INPUT_KEYBOARD,
              .ki.wVk = VK_LWIN,
              .ki.dwExtraInfo = extraInfo
          }
      };
      hooked = true;
#ifdef _DEBUG
      const UINT r = SendInput(_countof(input), input, sizeof(INPUT));
      DebugMsg("SendInput: %u", r);
      DebugMsgIf(r == 0, "SendInput Error: %x", GetLastError());
#else
      SendInput(_countof(input), input, sizeof(INPUT));
#endif
    }
  } else if (hooked && nCode == HC_ACTION && kbMsgId == WM_KEYUP &&
             (hook->vkCode == VK_CONTROL ||
              hook->vkCode == VK_LCONTROL ||
              hook->vkCode == VK_RCONTROL)) {
    const LPARAM extraInfo = GetMessageExtraInfo();
    INPUT input[1] = {
        {
            .type = INPUT_KEYBOARD,
            .ki.wVk = VK_LWIN,
            .ki.dwFlags = KEYEVENTF_KEYUP,
            .ki.dwExtraInfo = extraInfo
        }
    };
#ifdef _DEBUG
    const UINT r = SendInput(_countof(input), input, sizeof(INPUT));
    DebugMsg("SendInput: %u", r);
    DebugMsgIf(r == 0, "SendInput Error: %x", GetLastError());
#else
    SendInput(_countof(input), input, sizeof(INPUT));
#endif
  }
  return CallNextHookEx(hHook, nCode, kbMsgId, pEvent);
}

static inline bool RtlIsWindows8OrGreater() {
  RTL_OSVERSIONINFOW versionInfo;
  if (NT_SUCCESS(RtlGetVersion(&versionInfo))) {
    return versionInfo.dwMajorVersion > 6 ||
    (versionInfo.dwMajorVersion == 6 && versionInfo.dwMinorVersion >= 2);
  } else {
    return false;
  }
}

#ifdef _NO_START_FILES
void WinMainCRTStartup() {
#else
int WinMain(
      HINSTANCE hInstance,
      HINSTANCE hPrevInstance,
      LPSTR lpCmdLine,
      int nCmdShow
      ) {
#endif
  if (!RtlIsWindows8OrGreater()) {
#ifdef _NO_START_FILES
    ExitProcess(1);
#else
    return 1;
#endif
  }
  HANDLE mutex = CreateMutexA(NULL, FALSE, MUTEX);
  if (mutex == NULL || GetLastError() == ERROR_ALREADY_EXISTS) {
    ReleaseMutex(mutex);
#ifdef _NO_START_FILES
    ExitProcess(2);
#else
    return 2;
#endif
  }
  hHook = SetWindowsHookExW(WH_KEYBOARD_LL, LowLevelKeyboardProc, NULL, 0);

  MSG msg = {0};
  // Main message loop:
  while (GetMessageW(&msg, NULL, 0, 0)) {
    TranslateMessage(&msg);
    DispatchMessageW(&msg);
  }
  UnhookWindowsHookEx(hHook);
  ReleaseMutex(mutex);
#ifdef _NO_START_FILES
  ExitProcess(0);
#else
  return 0;
#endif
}