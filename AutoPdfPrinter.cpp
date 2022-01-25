/**
MIT License

Copyright(c) 2022 Keith Paterson

Permission is hereby granted, free of charge, to any person obtaining a copy
of this softwareand associated documentation files(the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions :

The above copyright noticeand this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#define WIN32_LEAN_AND_MEAN
#include <iostream>
#include <vector>
#include <windows.h>


void sendEnter() {
    std::cout << "Sending Enter" << std::endl;
    INPUT inputs[2] = {};
    ZeroMemory(inputs, sizeof(inputs));

    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.wVk = VK_RETURN;

    inputs[1].type = INPUT_KEYBOARD;
    inputs[1].ki.wVk = VK_RETURN;
    inputs[1].ki.dwFlags = KEYEVENTF_KEYUP;


    UINT uSent = SendInput(ARRAYSIZE(inputs), inputs, sizeof(INPUT));
    if (uSent != ARRAYSIZE(inputs))
    {
        std::cerr << "SendInput failed: 0x%x\n" << HRESULT_FROM_WIN32(GetLastError()) << std::endl;
    }
}

/*
Creates an INPUT structure for Scancode (Unicode)
*/

INPUT createScanCodeEvent(WORD scancode, bool isDown)
{
    INPUT input = {};
    input.type = INPUT_KEYBOARD;
    input.ki.wVk = 0;
    input.ki.wScan = scancode;
    input.ki.dwFlags = (isDown ? 0 : KEYEVENTF_KEYUP) | KEYEVENTF_SCANCODE;
    input.ki.time = 0;
    input.ki.dwExtraInfo = 0;
    return input;
}

int main(int argc, char* argv[])
{
    if (argc != 4) {
        std::cout << "Usage : AutoPdfPrinter secs_to_wait save_delay filename" << std::endl;
        std::cout << "secs_to_wait Time to wait for the appearance of the print dialog" << std::endl;
        std::cout << "save_delay Time to wait for the appearance of the save as dialog" << std::endl;
        std::cout << " filename the filenameand path to be used" << std::endl;
        return 0;
    }
    wchar_t printDlgTitle[1024];
    HMODULE h = LoadLibraryEx(L"comdlg32.dll", NULL, LOAD_LIBRARY_AS_IMAGE_RESOURCE); // the dll I know the associated .mui has the id
    LoadString(h, 1124, printDlgTitle, ARRAYSIZE(printDlgTitle));
    FreeLibrary(h);

    wchar_t saveAsDlgTitle[1024];
    h = LoadLibraryEx(L"winspool.drv", NULL, LOAD_LIBRARY_AS_IMAGE_RESOURCE); // the dll I know the associated .mui has the id
    LoadString(h, 505, saveAsDlgTitle, ARRAYSIZE(saveAsDlgTitle));
    FreeLibrary(h);

    std::wcout << "AutoPdfPrinter 0.2 Searching : "  << printDlgTitle << " and " << saveAsDlgTitle << std::endl;

    HWND hwnd = nullptr;

    int wait = atoi(argv[1]);
    while (--wait > 0 && !(hwnd = FindWindowW(0, printDlgTitle))) {
        Sleep(1000);
    }

    if (!hwnd) {
        std::cout << "Print Dialog not found : " << hwnd << std::endl;
        return 2;
    }
    std::cout << "Print : " << hwnd << std::endl;

    SetActiveWindow(hwnd);
    SetForegroundWindow(hwnd);
    sendEnter();
    HWND saveHwnd = nullptr;

    wait = atoi(argv[2]);
    while (--wait > 0 && !(saveHwnd = FindWindowW(0, saveAsDlgTitle))) {
        Sleep(1000);
    }

    saveHwnd = FindWindowW(0, saveAsDlgTitle);
    if (!saveHwnd) {
        std::wcout << "File Save Dialog not found : " << saveHwnd << std::endl;
        return 2;
    }
    SetActiveWindow(saveHwnd);
    SetForegroundWindow(saveHwnd);
    std::cout << "Sending " << argv[3] << " to " << saveHwnd << strlen(argv[3]) << std::endl;

    HKL kbl = GetKeyboardLayout(0);


    for (int i = 0; i < strlen(argv[3]);i++) {
        int keyCode = VkKeyScanExA(argv[3][i], kbl) & 0xff;

        int modifier = (VkKeyScanExA(argv[3][i], kbl) & 0xff00)>>8;

        if (modifier) {
            std::vector<INPUT> keystroke;
            if (modifier & 1) {
                keystroke.push_back(createScanCodeEvent(0x2a, true));
            }
            if (modifier & 2) {
                keystroke.push_back(createScanCodeEvent(0x1d, true));
            }
            if (modifier & 4) {
                keystroke.push_back(createScanCodeEvent(0x38, true));
            }
            SendInput(keystroke.size(), keystroke.data(), sizeof(INPUT));
        }

        INPUT saveinputs[2];
        ZeroMemory(saveinputs, sizeof(saveinputs));
        saveinputs[0].type = INPUT_KEYBOARD;
        saveinputs[0].ki.wVk = keyCode;
        saveinputs[1].type = INPUT_KEYBOARD;
        saveinputs[1].ki.wVk = keyCode;
        saveinputs[1].ki.dwFlags = KEYEVENTF_KEYUP;
        UINT uSent = SendInput(ARRAYSIZE(saveinputs), saveinputs, sizeof(INPUT));
        if (uSent != 2)
        {
            std::cerr << "SendInput failed: 0x%x\n" << HRESULT_FROM_WIN32(GetLastError()) << std::endl;
        }
        if (modifier) {
            std::vector<INPUT> keystroke;
            if (modifier & 4) {
                keystroke.push_back(createScanCodeEvent(0x38, false));
            }
            if (modifier & 2) {
                keystroke.push_back(createScanCodeEvent(0x1d, false));
            }
            if (modifier & 1) {
                keystroke.push_back(createScanCodeEvent(0x2a, false));
            }
            SendInput(keystroke.size(), keystroke.data(), sizeof(INPUT));
        }
    }

    sendEnter();
    std::cout << "End" << std::endl;
}

