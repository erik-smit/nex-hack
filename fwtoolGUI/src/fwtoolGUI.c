/*
 *  Sony NEX camera firmware toolbox GUI
 *
 *  Copyright (C) 2012-2013, nex-hack project
 *
 *  This file "fwtoolGUI" is part of fwtool (http://www.nex-hack.info)
 *
 *  All rights reserved.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <stdio.h>
#include <windows.h>
#include <shlobj.h>
#include "../res/resource.h"

#include "fwd_pack.h"

/*----------------- The window attributes -----------------*/
const char g_szClassName[] = "fwtoolWindowClass";
const char g_szAppTitle[]  = "fwtool for Sony NEX and Alpha";
#define APP_WINDOW_WIDTH    800
#define APP_WINDOW_HEIGHT   620

/*----------------- The input and output -----------------*/
char g_szDecInputFileName [MAX_PATH+4] = {'\0'};
char g_szDecOutputDirName [MAX_PATH+4] = {'\0'};
char g_szEncInputDirName  [MAX_PATH+4] = {'\0'};
char g_szEncOutputFileName[MAX_PATH+4] = {'\0'};

BOOL SelectOpenFile(char *pszFileName, const HWND hwnd);
BOOL SelectSaveFile(char *pszFileName, const HWND hwnd);
BOOL SelectOpenDir(char *pszDirName, const HWND hwnd);
BOOL SelectSaveDir(char *pszDirName, const HWND hwnd);

char g_szWorkingDirName[MAX_PATH+4] = {'\0'};

/*----------------- The declaration of GUI components -----------------*/
void CreateWndContent(HWND wParent);
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
void HandleButtonMouseLeftDown(HWND hwnd, WPARAM wParam);
void HandleButtonClick(HWND hwnd, WPARAM wParam);


/** \brief The Window Procedure. This function is called by the Windows function DispatchMessage()
 *
 * \param hwnd HWND             Handle of window which received this message
 * \param msg UINT              The message
 * \param wParam WPARAM         Extra information
 * \param lParam LPARAM         Extra information
 * \return LRESULT CALLBACK
 *
 */
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	// handle the messages
    switch(msg)
    {
        case WM_COMMAND:
        {
            HandleButtonClick(hwnd, wParam);
            // Handle the buttons event
            break;
        }

        case WM_LBUTTONDOWN:
        {
            // Handle mouse left button down events
            HandleButtonMouseLeftDown(hwnd, wParam);
            break;
        }
        case WM_CLOSE:
        {
            DestroyWindow(hwnd);
            break;
        }
        case WM_DESTROY:
        {
			// send a WM_QUIT to the message queue
            PostQuitMessage(0);
            break;
        }
        case WM_CREATE:
        {
            CreateWndContent(hwnd);
            break;
        }
        default:
        {
			// for messages that we don't deal with
            return DefWindowProc(hwnd, msg, wParam, lParam);
        }
    }

    return 0;
}


/** \brief  The main entry for the GUI
 *
 * \param hInstance     HINSTANCE   Handle to the programs executable module (the .exe file in memory)
 * \param hPrevInstance HINSTANCE   Always NULL for Win32 programs
 * \param lpCmdLine     LPSTR       The command line arguments as a single string. NOT including the program name.
 * \param nCmdShow      int         An integer value which may be passed to ShowWindow()
 * \return
 *
 */
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    HWND hwnd;              // This is the handle for our window
    MSG  Msg;               // Here messages to the application are saved
    WNDCLASSEX wc;          // Data structure for the windowclass

    //Step 1: Registering the Window Class
    wc.cbSize        = sizeof(WNDCLASSEX);              // This must always be set to sizeof(WNDCLASSEX)
    wc.style         = 0;                               // Class Styles (CS_*), not to be confused with Window Styles (WS_*) This can usually be set to 0.
    wc.lpfnWndProc   = WndProc;                         // Pointer to the window procedure for this window class.
    wc.cbClsExtra    = 0;                               // Amount of extra data allocated for this class in memory. Usually 0.
    wc.cbWndExtra    = 0;                               // Amount of extra data allocated in memory per window of this type. Usually 0.
    wc.hInstance     = hInstance;                       // Handle to application instance (that we got in the first parameter of WinMain()).
    wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION); // Large (usually 32x32) icon shown when the user presses Alt+Tab.
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);     // Cursor that will be displayed over our window.
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW);        	// Background Brush to set the color of our window.
    wc.lpszMenuName  = NULL;                            // Name of a menu resource to use for the windows with this class.
    wc.lpszClassName = g_szClassName;                   // Name to identify the class with.
    wc.hIconSm       = LoadIcon(NULL, IDI_APPLICATION); // Small (usually 16x16) icon to show in the taskbar and in the top left corner of the window.

	// Register the window class, and if it fails quit the program
    if (!RegisterClassEx(&wc))
    {
        MessageBox(NULL, "Window Registration Failed!", "Error!",
                   MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    // Step 2: Creating the Window
    hwnd = CreateWindowEx(
               WS_EX_CLIENTEDGE,    // the extended windows style
               g_szClassName,       // this tells the system what kind of window to create
               g_szAppTitle,        // title of the window
               WS_OVERLAPPEDWINDOW, // Window Style parameter
               CW_USEDEFAULT,       // x
               CW_USEDEFAULT,       // y
               APP_WINDOW_WIDTH,    // Width
               APP_WINDOW_HEIGHT,   // Height
               NULL,                // Parent Window handle
               NULL,                // menu handle
               hInstance,           // application instance handle
               NULL);               // pointer to window creation data

    if (hwnd == NULL)
    {
        MessageBox(NULL, "Window Creation Failed!", "Error!",
                   MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    // Initialise variables
    GetModuleFileName(NULL, g_szWorkingDirName, sizeof(g_szWorkingDirName));
    strrchr(g_szWorkingDirName, '\\')[1] = '\0';

	// Make the window visible on the screen
    ShowWindow(hwnd, nCmdShow);
    // Update it to make it appears correctly
    UpdateWindow(hwnd);

    // Step 3: The Message Loop. It will run until GetMessage() returns 0
    while (GetMessage(&Msg, NULL, 0, 0) > 0)
    {
		// Translate virtual-key messages into character messages
        TranslateMessage(&Msg);
        // Send message to WindowProcedure
        DispatchMessage(&Msg);
    }

	// The program return-value is 0 - The value that PostQuitMessage() gave
    return Msg.wParam;
}


/** \brief
 *
 * \param hwnd HWND
 * \param wParam WPARAM
 * \return void
 *
 */
void HandleButtonMouseLeftDown(HWND hwnd, WPARAM wParam)
{
    switch (LOWORD(wParam))
    {
    case 0:
        {
            // MAX_PATH is defined in windows.h
            char szFileName[MAX_PATH];
            HINSTANCE hInstance = GetModuleHandle(NULL);

            GetModuleFileName(hInstance, szFileName, MAX_PATH);
            MessageBox(hwnd, szFileName, "This program is:", MB_OK | MB_ICONINFORMATION);
            break;
        }
    default:
        {
            ;
        }
    }
}

char* convert_name_slashes(char *buf)
{
	char *p = NULL;
	if (buf) {
		while((p = strrchr(buf, '\\'))) {
			*p = '/';
		}
	}
	return buf;
}


/** \brief
 *
 * \param hwnd HWND
 * \param wParam WPARAM
 * \return void
 *
 */
void HandleButtonClick(HWND hwnd, WPARAM wParam)
{
    switch (LOWORD(wParam))
    {
        case IDC_BUTTON_DEC_BROWSE_INPUT_FILE:
        {
            if (TRUE == SelectOpenFile(g_szDecInputFileName, hwnd))
            {
                // set the IDC_EDIT_DEC_INPUT content
                SetDlgItemText(hwnd, IDC_EDIT_DEC_INPUT, g_szDecInputFileName);

                // set the IDC_EDIT_DEC_OUTPUT_CONTENT if it's empty
                GetDlgItemText(hwnd, IDC_EDIT_DEC_OUTPUT, g_szDecOutputDirName, sizeof(g_szDecOutputDirName));
                if (strlen(g_szDecOutputDirName) == 0)
                {
                    // Will use the same path as the input
                    memcpy(g_szDecOutputDirName, g_szDecInputFileName, (strrchr(g_szDecInputFileName, '\\')-g_szDecInputFileName+1));
                    SetDlgItemText(hwnd, IDC_EDIT_DEC_OUTPUT, g_szDecOutputDirName);
                }

                // enable the decrypt button
                EnableWindow(GetDlgItem(hwnd, IDC_BUTTON_DEC), TRUE);
            }

            break;
        }
        case IDC_BUTTON_ENC_BROWSE_OUTPUT_FILE:
        {
            if (TRUE == SelectSaveFile(g_szEncOutputFileName, hwnd))
            {
                // set the IDC_EDIT_ENC_OUTPUT content
                SetDlgItemText(hwnd, IDC_EDIT_ENC_OUTPUT, g_szEncOutputFileName);
            }

            break;
        }
        case IDC_BUTTON_DEC_BROWSE_OUTPUT_DIR:
        {
            if (TRUE == SelectOpenDir(g_szDecOutputDirName, hwnd))
            {
                // set the IDC_EDIT_ENC_OUTPUT content
                if (strlen(g_szDecOutputDirName) > 0)
                {
                    SetDlgItemText(hwnd, IDC_EDIT_DEC_OUTPUT, g_szDecOutputDirName);
                }
            }

            break;
        }
        case IDC_BUTTON_DEC:
        {
            char *src_name  = NULL;
            char *dest_name = NULL;

            // Check the content of the input
            GetDlgItemText(hwnd, IDC_EDIT_DEC_INPUT, g_szDecInputFileName, sizeof(g_szDecInputFileName));
            if (strlen(g_szDecInputFileName) < 8)
            {
                return;
            }

            // Check the content of the output
            GetDlgItemText(hwnd, IDC_EDIT_DEC_OUTPUT, g_szDecOutputDirName, sizeof(g_szDecOutputDirName));
            if (strlen(g_szDecOutputDirName) == 0)
            {
                // Will use the same path as the input
                memcpy(g_szDecOutputDirName, g_szDecInputFileName, (strrchr(g_szDecInputFileName, '\\')-g_szDecInputFileName+1));
                SetDlgItemText(hwnd, IDC_EDIT_DEC_OUTPUT, g_szDecOutputDirName);
            }

            src_name = convert_name_slashes(g_szDecInputFileName);
            dest_name = convert_name_slashes(g_szDecOutputDirName);

            // Call the decryption function
            // Start a new working thread
            do_unpack(src_name, dest_name, 0);

            break;
        }
        case IDC_BUTTON_ENC_BROWSE_INPUT_DIR:
        {
            // Set the input directory
            if (TRUE == SelectOpenDir(g_szEncInputDirName, hwnd))
            {
                if (strlen(g_szEncInputDirName) > 0)
                {
                    // set the IDC_EDIT_ENC_OUTPUT content
                    SetDlgItemText(hwnd, IDC_EDIT_ENC_INPUT, g_szEncInputDirName);

                    // Enable encrypt button
                    EnableWindow(GetDlgItem(hwnd, IDC_BUTTON_ENC), TRUE);
                }
            }

            break;
        }
    }
}


/** \brief
 *
 * \param pszFileName char* buffer for file name
 * \param hwnd const HWND   owner window
 * \return BOOL
 *
 */
BOOL SelectOpenFile(char *pszFileName, const HWND hwnd)
{
    OPENFILENAME ofn;   // common dialog box structure
    //HANDLE hf;          // file handle

    // Initialize OPENFILENAME
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = pszFileName;
    // Set lpstrFile[0] to '\0' so that GetOpenFileName does not use the contents of szFile to initialize itself.
    ofn.lpstrFile[0] = '\0';
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFilter = "*.exe\0*.EXE\0All\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    // Display the Open dialog box.
    if (TRUE == GetOpenFileName(&ofn))
    {
        /*
        hf = CreateFile(ofn.lpstrFile,
                GENERIC_READ,
                0,
                (LPSECURITY_ATTRIBUTES) NULL,
                OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL,
                (HANDLE) NULL);
        */
        return TRUE;
    }

    return FALSE;
}


/** \brief
 *
 * \param pszFileName char* buffer for file name
 * \param hwnd const HWND   owner window
 * \return BOOL
 *
 */
BOOL SelectSaveFile(char *pszFileName, const HWND hwnd)
{
    OPENFILENAME ofn;   // common dialog box structure
    //HANDLE hf;          // file handle

    // Initialize OPENFILENAME
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = pszFileName;
    // Set lpstrFile[0] to '\0' so that GetOpenFileName does not use the contents of szFile to initialize itself.
    ofn.lpstrFile[0] = '\0';
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFilter = "*.exe\0*.EXE\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_SHOWHELP | OFN_OVERWRITEPROMPT;

    // Display the Open dialog box.
    if (TRUE == GetSaveFileName(&ofn))
    {
        strcat(pszFileName, ".exe");

        return TRUE;
    }

    return FALSE;
}


/** \brief
 *
 * \param pszDirName char*
 * \param hwnd const HWND
 * \return BOOL
 *
 */
BOOL SelectOpenDir(char *pszDirName, const HWND hwnd)
{
    BROWSEINFO bi;
    ITEMIDLIST *pidl;

    bi.hwndOwner = hwnd;
    bi.pidlRoot = NULL;
    bi.pszDisplayName = pszDirName;
    //bi.lpszTitle = "Select a directory";
    bi.lpszTitle = NULL;
    bi.ulFlags = BIF_RETURNONLYFSDIRS;
    bi.lpfn = NULL;
    bi.lParam = 0;
    bi.iImage = 0;

    // Display "Select Folder" dialogue box, Get the folder name and convert it into a ITEMLIST data structure.
    pidl = SHBrowseForFolder( &bi );
    if ( pidl == NULL )
        pszDirName[0] = 0;

    // Retrieve folder name from ITEMLIST structure
    if (!SHGetPathFromIDList( pidl, pszDirName ))
      pszDirName[0] = 0;

    return TRUE;
}

BOOL SelectSaveDir(char *pszDirName, const HWND hwnd)
{
    return TRUE;
}


/*----------------- The definition of GUI components -----------------*/
/** \brief Create the content in the window
 *
 * \param wParent HWND  The parent window
 * \return void
 *
 */
void CreateWndContent(HWND wParent)
{
    HWND wTmp           = NULL;     // A temp handler
    HINSTANCE hInstance = GetModuleHandle(NULL);

    HFONT h_font        = CreateFont(
                                -13, 0, 0, 0,
                                FW_NORMAL, 0, 0, 0,
                                ANSI_CHARSET,
                                OUT_DEFAULT_PRECIS,
                                CLIP_DEFAULT_PRECIS,
                                DEFAULT_QUALITY,
                                DEFAULT_PITCH | FF_DONTCARE,
                                "Arial");


    // The Decryption group
	wTmp = CreateWindowEx(WS_EX_LEFT, "Button", "Decryption",
                       BS_GROUPBOX | WS_CHILD | WS_VISIBLE,
                       10, 10, 520, 100, wParent, (HMENU) IDC_GROUP_DECRYPTION, hInstance, NULL);
	SendMessage(wTmp, WM_SETFONT, (WPARAM) h_font, TRUE);

	wTmp = CreateWindowEx(WS_EX_LEFT, "Static", "Input:",
                        0x50000300,
                        26, 34, 50, 24, wParent, (HMENU) IDC_STATIC_LABEL_DEC_INPUT, hInstance, NULL);
	SendMessage(wTmp, WM_SETFONT, (WPARAM) h_font, TRUE);

	wTmp = CreateWindowEx(WS_EX_LEFT, "Static", "Output:",
                        0x50000300,
                        26, 68, 50, 24, wParent, (HMENU) IDC_STATIC_LABEL_DEC_OUTPUT, hInstance, NULL);
	SendMessage(wTmp, WM_SETFONT, (WPARAM) h_font, TRUE);

	wTmp = CreateWindowEx(WS_EX_CLIENTEDGE, "Edit", "",
                        0x50010080,
                        74, 34, 280, 24, wParent, (HMENU) IDC_EDIT_DEC_INPUT, hInstance, NULL);
	SendMessage(wTmp, WM_SETFONT, (WPARAM) h_font, TRUE);
    wTmp = CreateWindowEx(WS_EX_CLIENTEDGE, "Edit", "",
                        0x50010080,
                        74, 68, 280, 24, wParent, (HMENU) IDC_EDIT_DEC_OUTPUT, hInstance, NULL);
	SendMessage(wTmp, WM_SETFONT, (WPARAM) h_font, TRUE);

	wTmp = CreateWindowEx(WS_EX_LEFT, "Button", "Browse",
                        BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE,
                        360, 34, 80, 24, wParent, (HMENU) IDC_BUTTON_DEC_BROWSE_INPUT_FILE, hInstance, NULL);
	SendMessage(wTmp, WM_SETFONT, (WPARAM) h_font, TRUE);
	wTmp = CreateWindowEx(WS_EX_LEFT, "Button", "Browse",
                        BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE,
                        360, 68, 80, 24, wParent, (HMENU) IDC_BUTTON_DEC_BROWSE_OUTPUT_DIR, hInstance, NULL);
	SendMessage(wTmp, WM_SETFONT, (WPARAM) h_font, TRUE);

	wTmp = CreateWindowEx(WS_EX_LEFT, "Button", "Decrypt!",
                        BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE | WS_DISABLED,
                        460, 34, 60, 60, wParent, (HMENU) IDC_BUTTON_DEC, hInstance, NULL);
	SendMessage(wTmp, WM_SETFONT, (WPARAM) h_font, TRUE);


    // The Encryption group
	wTmp = CreateWindowEx(WS_EX_LEFT, "Button", "Encryption",
                       BS_GROUPBOX | WS_CHILD | WS_VISIBLE,
                       10, 120, 520, 100, wParent, (HMENU) IDC_GROUP_ENCRYPTION, hInstance, NULL);
	SendMessage(wTmp, WM_SETFONT, (WPARAM) h_font, TRUE);

	wTmp = CreateWindowEx(WS_EX_LEFT, "Static", "Input:",
                        0x50000300,
                        26, 144, 50, 24, wParent, (HMENU) IDC_STATIC_LABEL_ENC_INPUT, hInstance, NULL);
	SendMessage(wTmp, WM_SETFONT, (WPARAM) h_font, TRUE);

	wTmp = CreateWindowEx(WS_EX_LEFT, "Static", "Output:",
                        0x50000300,
                        26, 178, 50, 24, wParent, (HMENU) IDC_STATIC_LABEL_ENC_OUTPUT, hInstance, NULL);
	SendMessage(wTmp, WM_SETFONT, (WPARAM) h_font, TRUE);

	wTmp = CreateWindowEx(WS_EX_CLIENTEDGE, "Edit", "",
                        0x50010080,
                        74, 144, 280, 24, wParent, (HMENU) IDC_EDIT_ENC_INPUT, hInstance, NULL);
	SendMessage(wTmp, WM_SETFONT, (WPARAM) h_font, TRUE);
    wTmp = CreateWindowEx(WS_EX_CLIENTEDGE, "Edit", "",
                        0x50010080,
                        74, 178, 280, 24, wParent, (HMENU) IDC_EDIT_ENC_OUTPUT, hInstance, NULL);
	SendMessage(wTmp, WM_SETFONT, (WPARAM) h_font, TRUE);

	wTmp = CreateWindowEx(WS_EX_LEFT, "Button", "Browse",
                        BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE,
                        360, 144, 80, 24, wParent, (HMENU) IDC_BUTTON_ENC_BROWSE_INPUT_DIR, hInstance, NULL);
	SendMessage(wTmp, WM_SETFONT, (WPARAM) h_font, TRUE);
	wTmp = CreateWindowEx(WS_EX_LEFT, "Button", "Browse",
                        BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE,
                        360, 178, 80, 24, wParent, (HMENU) IDC_BUTTON_ENC_BROWSE_OUTPUT_FILE, hInstance, NULL);
	SendMessage(wTmp, WM_SETFONT, (WPARAM) h_font, TRUE);

	wTmp = CreateWindowEx(WS_EX_LEFT, "Button", "Encrypt!",
                        BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE | WS_DISABLED,
                        460, 144, 60, 60, wParent, (HMENU) IDC_BUTTON_ENC, hInstance, NULL);
	SendMessage(wTmp, WM_SETFONT, (WPARAM) h_font, TRUE);


    // The Options group
	wTmp = CreateWindowEx(WS_EX_LEFT, "Button", "Options",
                        BS_GROUPBOX | WS_CHILD | WS_VISIBLE,
                        540, 10, 230, 560, wParent, (HMENU) IDC_GROUP_OPTIONS, hInstance, NULL);
	SendMessage(wTmp, WM_SETFONT, (WPARAM) h_font, TRUE);

	wTmp = CreateWindowEx(WS_EX_LEFT, "Static", "Language:",
                        BS_SOLID | WS_CHILD | WS_VISIBLE,
                        556, 34, 64, 24, wParent, (HMENU) IDC_STATIC_LABEL_OPT_LANGUAGE, hInstance, NULL);
	SendMessage(wTmp, WM_SETFONT, (WPARAM) h_font, TRUE);

	wTmp = CreateWindowEx(WS_EX_LEFT, "ComboBox", "",
                        0x50010203,
                        620, 34, 96, 23, wParent, (HMENU) IDC_COMBO_OPT_LANGUAGES, hInstance, NULL);
	SendMessage(wTmp, WM_SETFONT, (WPARAM) h_font, TRUE);

	wTmp = CreateWindowEx(WS_EX_LEFT, "Button", "Apply!",
                        BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE,
                        700, 540, 60, 24, wParent, (HMENU) IDC_BUTTON_OPT_APPLY, hInstance, NULL);
	SendMessage(wTmp, WM_SETFONT, (WPARAM) h_font, TRUE);


    // The Status group
	wTmp = CreateWindowEx(WS_EX_LEFT, "Button", "Status",
                        BS_GROUPBOX | WS_CHILD | WS_VISIBLE,
                        10, 230, 520, 340, wParent, (HMENU) IDC_GROUP_STATUS, hInstance, NULL);
	SendMessage(wTmp, WM_SETFONT, (WPARAM) h_font, TRUE);

	wTmp = CreateWindowEx(WS_EX_LEFT, "Static", "Progress:",
                        0x50000300,
                        20, 260, 60, 20, wParent, (HMENU) IDC_STATIC_LABEL_STA_PROGRESS, hInstance, NULL);
	SendMessage(wTmp, WM_SETFONT, (WPARAM) h_font, TRUE);

	wTmp = CreateWindowEx(WS_EX_LEFT, "msctls_progress32", "",
                        0x50000000,
                        80, 260, 440, 20, wParent, (HMENU) IDC_PROGRESS_BAR_STA, hInstance, NULL);
	SendMessage(wTmp, WM_SETFONT, (WPARAM) h_font, TRUE);
}
