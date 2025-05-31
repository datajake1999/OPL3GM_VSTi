/*
OPL3GM Editor
Copyright (C) 2021-2024  Datajake

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "editor.h"
#include "OPL3GM.h"
#include "gmnames.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>
#include <commdlg.h>
#include <dlgs.h>
#include <shellapi.h>
#include "../res/resource.h"
#ifndef VK_OEM_1
#define VK_OEM_1          0xBA   // ';:' for US
#endif
#ifndef VK_OEM_2
#define VK_OEM_2          0xBF   // '/?' for US
#endif
#ifndef VK_OEM_3
#define VK_OEM_3          0xC0   // '`~' for US
#endif
#ifndef VK_OEM_4
#define VK_OEM_4          0xDB  //  '[{' for US
#endif
#ifndef VK_OEM_5
#define VK_OEM_5          0xDC  //  '\|' for US
#endif
#ifndef VK_OEM_6
#define VK_OEM_6          0xDD  //  ']}' for US
#endif
#ifndef VK_OEM_7
#define VK_OEM_7          0xDE  //  ''"' for US
#endif
#ifndef VK_OEM_COMMA
#define VK_OEM_COMMA      0xBC   // ',' any country
#endif
#ifndef VK_OEM_PERIOD
#define VK_OEM_PERIOD     0xBE   // '.' any country
#endif
#ifndef VK_OEM_PLUS
#define VK_OEM_PLUS       0xBB   // '+' any country
#endif
#ifndef VK_OEM_MINUS
#define VK_OEM_MINUS      0xBD   // '-' any country
#endif
#define KEYWASDOWN (1 << 30)
#define KEYNAME "SOFTWARE\\Datajake\\OPL3GM"
#define PROJPAGE "https://github.com/datajake1999/OPL3GM_VSTi"

static VstInt32 g_useCount = 0;
static HBRUSH hBrush = NULL;
extern void* hInstance;

static const VstInt32 rates[] =
{
	1000,1500,
	2000,3000,
	4000,6000,
	8000,
	11025,12000,
	12429,
	16000,
	22050,24000,
	24858,
	32000,
	44100,48000,
	49716,
	64000,
	88200,96000,
	99432,
	128000,
	176400,192000,
	198864,
	256000,
	352800,384000,
	397728,
	512000,
	705600,768000,
	795456
};

static BOOL SetPreset(HWND hWnd, AudioEffectX* effect)
{
	if (hWnd && effect)
	{
		effect->setProgram (SendDlgItemMessage(hWnd, IDC_PRESET, CB_GETCURSEL, 0, 0));
		return TRUE;
	}
	return FALSE;
}

static BOOL SetPresetName(HWND hWnd, AudioEffectX* effect)
{
	if (hWnd && effect)
	{
		char text[MAX_PATH];
		ZeroMemory(text, sizeof(text));
		if (GetDlgItemText(hWnd, IDC_PRESETNAME, text, MAX_PATH))
		{
			effect->setProgramName (text);
			return TRUE;
		}
	}
	return FALSE;
}

static BOOL SetParameterValue(AudioEffectX* effect, VstInt32 index, float value)
{
	if (effect)
	{
		effect->beginEdit (index);
		effect->setParameterAutomated (index, value);
		effect->endEdit (index);
		return TRUE;
	}
	return FALSE;
}

static BOOL SetBypassState(HWND hWnd, AudioEffectX* effect)
{
	if (hWnd && effect)
	{
		if (IsDlgButtonChecked(hWnd, IDC_BYPASS))
		{
			effect->setBypass (true);
		}
		else
		{
			effect->setBypass (false);
		}
		return TRUE;
	}
	return FALSE;
}

static BOOL SetOPLRate(HWND hWnd, OPL3GM* effect)
{
	if (hWnd && effect)
	{
		char text[MAX_PATH];
		ZeroMemory(text, sizeof(text));
		if (GetDlgItemText(hWnd, IDC_OPLRATE, text, MAX_PATH))
		{
			VstInt32 rate = atoi(text);
			if (rate != (VstInt32)effect->getSampleRate ())
			{
				effect->setInternalRate (rate);
			}
			sprintf(text, "%d", effect->getInternalRate ());
			SetDlgItemText(hWnd, IDC_OPLRATE, text);
			return TRUE;
		}
	}
	return FALSE;
}

static BOOL InitDialog(HWND hWnd)
{
	if (hWnd)
	{
		HICON hIcon = LoadIcon((HINSTANCE)hInstance, MAKEINTRESOURCE(IDI_ICON1));
		if (hIcon)
		{
			SendMessage(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
		}
		SendDlgItemMessage(hWnd, IDC_PRESETNAME, EM_LIMITTEXT, kVstMaxProgNameLen-1, 0);
		SendDlgItemMessage(hWnd, IDC_VOLUME, TBM_SETRANGE, 0, MAKELONG(0, 100));
		SendDlgItemMessage(hWnd, IDC_VOLUME, TBM_SETPAGESIZE, 0, 10);
		SendDlgItemMessage(hWnd, IDC_TRANSPOSE, TBM_SETRANGE, 0, MAKELONG(0, 25));
		SendDlgItemMessage(hWnd, IDC_TRANSPOSE, TBM_SETPAGESIZE, 0, 2);
		char text[MAX_PATH];
		ZeroMemory(text, sizeof(text));
		for (VstInt32 i = 0; i < sizeof(rates) / sizeof(int); i++)
		{
			sprintf(text, "%d", rates[i]);
			SendDlgItemMessage(hWnd, IDC_OPLRATE, CB_INSERTSTRING, i, (LPARAM)text);
		}
		return TRUE;
	}
	return FALSE;
}

static BOOL RefreshDialog(HWND hWnd, OPL3GM* effect)
{
	if (hWnd && effect)
	{
		float ParamValue;
		char text[MAX_PATH];
		ZeroMemory(text, sizeof(text));
		SendDlgItemMessage(hWnd, IDC_PRESET, CB_RESETCONTENT, 0, 0);
		for (VstInt32 i = 0; i < kNumPrograms; i++)
		{
			effect->getProgramNameIndexed (-1, i, text);
			SendDlgItemMessage(hWnd, IDC_PRESET, CB_INSERTSTRING, i, (LPARAM)text);
		}
		SendDlgItemMessage(hWnd, IDC_PRESET, CB_SETCURSEL, effect->getProgram (), 0);
		effect->getProgramName (text);
		SetDlgItemText(hWnd, IDC_PRESETNAME, text);
		ParamValue = effect->getParameter (kVolume)*100;
		SendDlgItemMessage(hWnd, IDC_VOLUME, TBM_SETPOS, TRUE, (LPARAM)ParamValue);
		effect->getParameterDisplay (kVolume, text);
		SetDlgItemText(hWnd, IDC_VOLDISP1, text);
		effect->getParameterLabel (kVolume, text);
		SetDlgItemText(hWnd, IDC_VOLDISP2, text);
		ParamValue = effect->getParameter (kTranspose)*25;
		SendDlgItemMessage(hWnd, IDC_TRANSPOSE, TBM_SETPOS, TRUE, (LPARAM)ParamValue);
		effect->getParameterDisplay (kTranspose, text);
		SetDlgItemText(hWnd, IDC_TRANDISP1, text);
		effect->getParameterLabel (kTranspose, text);
		SetDlgItemText(hWnd, IDC_TRANDISP2, text);
		ParamValue = effect->getParameter (kVolumeDisplay);
		if (ParamValue >= 0.5)
		{
			CheckDlgButton(hWnd, IDC_DISPLAY, BST_CHECKED);
		}
		else
		{
			CheckDlgButton(hWnd, IDC_DISPLAY, BST_UNCHECKED);
		}
		ParamValue = effect->getParameter (kEmulator);
		if (ParamValue >= 0.5)
		{
			CheckDlgButton(hWnd, IDC_NUKED, BST_CHECKED);
		}
		else
		{
			CheckDlgButton(hWnd, IDC_NUKED, BST_UNCHECKED);
		}
		ParamValue = effect->getParameter (kHQResample);
		if (ParamValue >= 0.5)
		{
			CheckDlgButton(hWnd, IDC_RESAMPLE, BST_CHECKED);
			EnableWindow(GetDlgItem(hWnd, IDC_OPLRATE), TRUE);
		}
		else
		{
			CheckDlgButton(hWnd, IDC_RESAMPLE, BST_UNCHECKED);
			EnableWindow(GetDlgItem(hWnd, IDC_OPLRATE), FALSE);
		}
		ParamValue = effect->getParameter (kDCBlock);
		if (ParamValue >= 0.5)
		{
			CheckDlgButton(hWnd, IDC_DC, BST_CHECKED);
		}
		else
		{
			CheckDlgButton(hWnd, IDC_DC, BST_UNCHECKED);
		}
		ParamValue = effect->getParameter (kNoiseGate);
		if (ParamValue >= 0.5)
		{
			CheckDlgButton(hWnd, IDC_GATE, BST_CHECKED);
		}
		else
		{
			CheckDlgButton(hWnd, IDC_GATE, BST_UNCHECKED);
		}
		ParamValue = effect->getParameter (kPushMidi);
		if (ParamValue >= 0.5)
		{
			CheckDlgButton(hWnd, IDC_QUEUE, BST_CHECKED);
		}
		else
		{
			CheckDlgButton(hWnd, IDC_QUEUE, BST_UNCHECKED);
		}
		if (effect->getBypass ())
		{
			CheckDlgButton(hWnd, IDC_BYPASS, BST_CHECKED);
		}
		else
		{
			CheckDlgButton(hWnd, IDC_BYPASS, BST_UNCHECKED);
		}
		sprintf(text, "%d", effect->getInternalRate ());
		SetDlgItemText(hWnd, IDC_OPLRATE, text);
		effect->getBankName (text, MAX_PATH);
		SetDlgItemText(hWnd, IDC_CURBANK, text);
		return TRUE;
	}
	return FALSE;
}

static BOOL Scroll(HWND hWnd, LPARAM lParam, AudioEffectX* effect)
{
	if (hWnd && effect)
	{
		LPARAM VolumeHandle = (LPARAM)GetDlgItem(hWnd, IDC_VOLUME);
		LPARAM TransposeHandle = (LPARAM)GetDlgItem(hWnd, IDC_TRANSPOSE);
		float ParamValue;
		if (lParam == VolumeHandle)
		{
			ParamValue = SendDlgItemMessage(hWnd, IDC_VOLUME, TBM_GETPOS, 0, 0)/100.0f;
			return SetParameterValue(effect, kVolume, ParamValue);
		}
		else if (lParam == TransposeHandle)
		{
			ParamValue = SendDlgItemMessage(hWnd, IDC_TRANSPOSE, TBM_GETPOS, 0, 0)/25.0f;
			return SetParameterValue(effect, kTranspose, ParamValue);
		}
	}
	return FALSE;
}

static UINT HelpBox(HWND hWnd)
{
	if (hWnd)
	{
		char caption[MAX_PATH];
		char text[MAX_PATH];
		ZeroMemory(caption, sizeof(caption));
		ZeroMemory(text, sizeof(text));
		LoadString((HINSTANCE)hInstance, IDS_HELPCAP, caption, MAX_PATH);
		LoadString((HINSTANCE)hInstance, IDS_HELPTXT, text, MAX_PATH);
		MessageBox(hWnd, text, caption, MB_ICONINFORMATION);
		return 1;
	}
	return 0;
}

static UINT WINAPI OldHookProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case pshHelp:
			return HelpBox(hWnd);
		}
	}
	return 0;
}

static UINT WINAPI ExplorerHookProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_NOTIFY:
		switch (((OFNOTIFY*)lParam)->hdr.code)
		{
		case CDN_HELP:
			return HelpBox(hWnd);
		}
	}
	return 0;
}

static BOOL LoadInstrumentBank(HWND hWnd, OPL3GM* effect)
{
	if (hWnd && effect)
	{
		OPENFILENAME ofn;
		char filename[MAX_PATH];
		char title[MAX_PATH];
		char filter[MAX_PATH];
		char caption[MAX_PATH];
		ZeroMemory(&ofn, sizeof(ofn));
		ZeroMemory(filename, sizeof(filename));
		ZeroMemory(title, sizeof(title));
		ZeroMemory(filter, sizeof(filter));
		ZeroMemory(caption, sizeof(caption));
		LoadString((HINSTANCE)hInstance, IDS_FILEFLT, filter, MAX_PATH);
		LoadString((HINSTANCE)hInstance, IDS_FILECAP, caption, MAX_PATH);
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = hWnd;
		ofn.hInstance = (HINSTANCE)hInstance;
		ofn.lpstrFilter = filter;
		ofn.nFilterIndex = 1;
		ofn.lpstrFile = filename;
		ofn.nMaxFile = MAX_PATH;
		ofn.lpstrFileTitle = title;
		ofn.nMaxFileTitle = MAX_PATH;
		ofn.lpstrTitle = caption;
		ofn.Flags = OFN_ENABLEHOOK | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_LONGNAMES | OFN_NOCHANGEDIR;
#ifdef OFN_ENABLESIZING
		ofn.Flags |= OFN_ENABLESIZING;
#endif
#ifdef OFN_DONTADDTORECENT
		ofn.Flags |= OFN_DONTADDTORECENT;
#endif
		char synthname[kVstMaxEffectNameLen];
		ZeroMemory(synthname, sizeof(synthname));
		effect->getEffectName (synthname);
		if (!strcmp(synthname, "Apogee OPL3"))
		{
			ofn.nFilterIndex = 2;
			ofn.lpstrDefExt = "TMB";
		}
		else if (!strcmp(synthname, "Doom OPL3"))
		{
			ofn.nFilterIndex = 3;
			ofn.lpstrDefExt = "OP2";
		}
		else if (!strcmp(synthname, "Windows 9x OPL3"))
		{
			char text[MAX_PATH];
			ZeroMemory(text, sizeof(text));
			LoadString((HINSTANCE)hInstance, IDS_W9XCAP, caption, MAX_PATH);
			LoadString((HINSTANCE)hInstance, IDS_W9XTXT, text, MAX_PATH);
			MessageBox(hWnd, text, caption, MB_ICONEXCLAMATION);
			return FALSE;
		}
		HKEY hKey;
		DWORD expstyle = 1;
		DWORD help = 0;
		char directory[MAX_PATH];
		ZeroMemory(directory, sizeof(directory));
		if (RegOpenKeyEx(HKEY_CURRENT_USER, KEYNAME, 0, KEY_READ, &hKey) == ERROR_SUCCESS)
		{
			DWORD type = REG_DWORD;
			DWORD len = sizeof(DWORD);
			RegQueryValueEx(hKey, "ExplorerStyle", NULL, &type, (LPBYTE)&expstyle, &len);
			RegQueryValueEx(hKey, "HelpButton", NULL, &type, (LPBYTE)&help, &len);
			type = REG_SZ;
			len = sizeof(directory);
			if (RegQueryValueEx(hKey, "ApogeePatchDir", NULL, &type, (LPBYTE)directory, &len) == ERROR_SUCCESS && !strcmp(synthname, "Apogee OPL3"))
			{
				ofn.lpstrInitialDir = directory;
			}
			else if (RegQueryValueEx(hKey, "DoomPatchDir", NULL, &type, (LPBYTE)directory, &len) == ERROR_SUCCESS && !strcmp(synthname, "Doom OPL3"))
			{
				ofn.lpstrInitialDir = directory;
			}
			RegCloseKey(hKey);
			hKey = NULL;
		}
		if (expstyle == 1)
		{
			ofn.Flags |= OFN_EXPLORER;
		}
		else if (expstyle >= 2)
		{
			srand(GetTickCount());
			if (rand() & 1)
			{
				ofn.Flags |= OFN_EXPLORER;
			}
		}
		if (help)
		{
			ofn.Flags |= OFN_SHOWHELP;
		}
		if (ofn.Flags & OFN_EXPLORER)
		{
			ofn.lpfnHook = (LPOFNHOOKPROC)ExplorerHookProc;
		}
		else
		{
			ofn.lpfnHook = (LPOFNHOOKPROC)OldHookProc;
		}
		if (GetOpenFileName(&ofn))
		{
			if (!effect->loadInstruments (ofn.lpstrFile, ofn.lpstrFileTitle))
			{
				char temp[MAX_PATH];
				char text[MAX_PATH];
				ZeroMemory(temp, sizeof(temp));
				ZeroMemory(text, sizeof(text));
				LoadString((HINSTANCE)hInstance, IDS_FAILCAP, caption, MAX_PATH);
				LoadString((HINSTANCE)hInstance, IDS_FAILTXT, temp, MAX_PATH);
				sprintf(text, temp, ofn.lpstrFile);
				MessageBox(hWnd, text, caption, MB_ICONERROR);
				return FALSE;
			}
			SetDlgItemText(hWnd, IDC_CURBANK, ofn.lpstrFileTitle);
			return TRUE;
		}
	}
	return FALSE;
}

static BOOL LoadInstrumentBankDragDrop(HWND hWnd, WPARAM wParam, OPL3GM* effect)
{
	HDROP hDrop = (HDROP)wParam;
	if (hWnd && effect)
	{
		char synthname[kVstMaxEffectNameLen];
		ZeroMemory(synthname, sizeof(synthname));
		effect->getEffectName (synthname);
		if (!strcmp(synthname, "Windows 9x OPL3"))
		{
			char caption[MAX_PATH];
			char text[MAX_PATH];
			ZeroMemory(caption, sizeof(caption));
			ZeroMemory(text, sizeof(text));
			LoadString((HINSTANCE)hInstance, IDS_W9XCAP, caption, MAX_PATH);
			LoadString((HINSTANCE)hInstance, IDS_W9XTXT, text, MAX_PATH);
			MessageBox(hWnd, text, caption, MB_ICONEXCLAMATION);
			DragFinish(hDrop);
			return FALSE;
		}
		char filename[MAX_PATH];
		ZeroMemory(filename, sizeof(filename));
		if (DragQueryFile(hDrop, 0, filename, MAX_PATH))
		{
			if (!effect->loadInstruments (filename, filename))
			{
				char caption[MAX_PATH];
				char temp[MAX_PATH];
				char text[MAX_PATH];
				ZeroMemory(caption, sizeof(caption));
				ZeroMemory(temp, sizeof(temp));
				ZeroMemory(text, sizeof(text));
				LoadString((HINSTANCE)hInstance, IDS_FAILCAP, caption, MAX_PATH);
				LoadString((HINSTANCE)hInstance, IDS_FAILTXT, temp, MAX_PATH);
				sprintf(text, temp, filename);
				MessageBox(hWnd, text, caption, MB_ICONERROR);
				DragFinish(hDrop);
				return FALSE;
			}
			SetDlgItemText(hWnd, IDC_CURBANK, filename);
			DragFinish(hDrop);
			return TRUE;
		}
	}
	DragFinish(hDrop);
	return FALSE;
}

static BOOL AboutBox(HWND hWnd)
{
	if (hWnd)
	{
		char caption[MAX_PATH];
		char text[MAX_PATH];
		ZeroMemory(caption, sizeof(caption));
		ZeroMemory(text, sizeof(text));
		LoadString((HINSTANCE)hInstance, IDS_ABOUTCAP, caption, MAX_PATH);
		LoadString((HINSTANCE)hInstance, IDS_ABOUTTXT, text, MAX_PATH);
		MessageBox(hWnd, text, caption, MB_ICONINFORMATION);
		return TRUE;
	}
	return FALSE;
}

static BOOL StatisticsBox(HWND hWnd, OPL3GM* effect)
{
	if (hWnd && effect)
	{
		char caption[MAX_PATH];
		char temp[MAX_PATH];
		char text[MAX_PATH];
		ZeroMemory(caption, sizeof(caption));
		ZeroMemory(temp, sizeof(temp));
		ZeroMemory(text, sizeof(text));
		LoadString((HINSTANCE)hInstance, IDS_STATSCAP, caption, MAX_PATH);
		LoadString((HINSTANCE)hInstance, IDS_STATSTXT, temp, MAX_PATH);
		sprintf(text, temp, (VstInt32)effect->getSampleRate (), effect->getInternalRate (), effect->getBlockSize (), g_useCount);
		MessageBox(hWnd, text, caption, MB_ICONINFORMATION);
		return TRUE;
	}
	return FALSE;
}

static BOOL HostInfoBox(HWND hWnd, OPL3GM* effect)
{
	if (hWnd && effect)
	{
		char caption[MAX_PATH];
		char temp[MAX_PATH];
		char text[MAX_PATH];
		ZeroMemory(caption, sizeof(caption));
		ZeroMemory(temp, sizeof(temp));
		ZeroMemory(text, sizeof(text));
		HostInfo* hi = effect->getHostInfo ();
		LoadString((HINSTANCE)hInstance, IDS_HOSTCAP, caption, MAX_PATH);
		LoadString((HINSTANCE)hInstance, IDS_HOSTTXT, temp, MAX_PATH);
		sprintf(text, temp, hi->ProductString, hi->VendorVersion/1000.0f, hi->VendorString, hi->MasterVersion/1000.0f);
		if (!strcmp(hi->VendorString, "Cockos"))
		{
			char approve[32];
			ZeroMemory(approve, sizeof(approve));
			LoadString((HINSTANCE)hInstance, IDS_REAPPROVE, approve, sizeof(approve));
			strncat(text, approve, sizeof(approve));
		}
		MessageBox(hWnd, text, caption, MB_ICONINFORMATION);
		return TRUE;
	}
	return FALSE;
}

static BOOL ProjectPage(HWND hWnd)
{
	if (hWnd)
	{
		char caption[MAX_PATH];
		char text[MAX_PATH];
		ZeroMemory(caption, sizeof(caption));
		ZeroMemory(text, sizeof(text));
		LoadString((HINSTANCE)hInstance, IDS_WEBCAP, caption, MAX_PATH);
		LoadString((HINSTANCE)hInstance, IDS_WEBTXT, text, MAX_PATH);
		if (MessageBox(hWnd, text, caption, MB_ICONQUESTION | MB_YESNO) == IDYES)
		{
			ShellExecute(hWnd, NULL, PROJPAGE, NULL, NULL, 0);
			return TRUE;
		}
	}
	return FALSE;
}

static BOOL WINAPI MixerProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
#ifdef _WIN64
	OPL3GM* effect = (OPL3GM*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
#else
	OPL3GM* effect = (OPL3GM*)GetWindowLong(hWnd, GWL_USERDATA);
#endif
	VstInt32 i;
	switch (message)
	{
	case WM_INITDIALOG:
		effect = (OPL3GM*)lParam;
		for (i = 0; i < 16; i++)
		{
			if (effect->isChannelEnabled (i))
			{
				CheckDlgButton(hWnd, IDC_CHAN01+i, BST_CHECKED);
			}
		}
#ifdef _WIN64
		SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)lParam);
#else
		SetWindowLong(hWnd, GWL_USERDATA, (LONG)lParam);
#endif
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_CHAN01:
			if (IsDlgButtonChecked(hWnd, IDC_CHAN01))
			{
				effect->enableChannel (0, true);
			}
			else
			{
				effect->enableChannel (0, false);
			}
			return TRUE;
		case IDC_CHAN02:
			if (IsDlgButtonChecked(hWnd, IDC_CHAN02))
			{
				effect->enableChannel (1, true);
			}
			else
			{
				effect->enableChannel (1, false);
			}
			return TRUE;
		case IDC_CHAN03:
			if (IsDlgButtonChecked(hWnd, IDC_CHAN03))
			{
				effect->enableChannel (2, true);
			}
			else
			{
				effect->enableChannel (2, false);
			}
			return TRUE;
		case IDC_CHAN04:
			if (IsDlgButtonChecked(hWnd, IDC_CHAN04))
			{
				effect->enableChannel (3, true);
			}
			else
			{
				effect->enableChannel (3, false);
			}
			return TRUE;
		case IDC_CHAN05:
			if (IsDlgButtonChecked(hWnd, IDC_CHAN05))
			{
				effect->enableChannel (4, true);
			}
			else
			{
				effect->enableChannel (4, false);
			}
			return TRUE;
		case IDC_CHAN06:
			if (IsDlgButtonChecked(hWnd, IDC_CHAN06))
			{
				effect->enableChannel (5, true);
			}
			else
			{
				effect->enableChannel (5, false);
			}
			return TRUE;
		case IDC_CHAN07:
			if (IsDlgButtonChecked(hWnd, IDC_CHAN07))
			{
				effect->enableChannel (6, true);
			}
			else
			{
				effect->enableChannel (6, false);
			}
			return TRUE;
		case IDC_CHAN08:
			if (IsDlgButtonChecked(hWnd, IDC_CHAN08))
			{
				effect->enableChannel (7, true);
			}
			else
			{
				effect->enableChannel (7, false);
			}
			return TRUE;
		case IDC_CHAN09:
			if (IsDlgButtonChecked(hWnd, IDC_CHAN09))
			{
				effect->enableChannel (8, true);
			}
			else
			{
				effect->enableChannel (8, false);
			}
			return TRUE;
		case IDC_CHAN10:
			if (IsDlgButtonChecked(hWnd, IDC_CHAN10))
			{
				effect->enableChannel (9, true);
			}
			else
			{
				effect->enableChannel (9, false);
			}
			return TRUE;
		case IDC_CHAN11:
			if (IsDlgButtonChecked(hWnd, IDC_CHAN11))
			{
				effect->enableChannel (10, true);
			}
			else
			{
				effect->enableChannel (10, false);
			}
			return TRUE;
		case IDC_CHAN12:
			if (IsDlgButtonChecked(hWnd, IDC_CHAN12))
			{
				effect->enableChannel (11, true);
			}
			else
			{
				effect->enableChannel (11, false);
			}
			return TRUE;
		case IDC_CHAN13:
			if (IsDlgButtonChecked(hWnd, IDC_CHAN13))
			{
				effect->enableChannel (12, true);
			}
			else
			{
				effect->enableChannel (12, false);
			}
			return TRUE;
		case IDC_CHAN14:
			if (IsDlgButtonChecked(hWnd, IDC_CHAN14))
			{
				effect->enableChannel (13, true);
			}
			else
			{
				effect->enableChannel (13, false);
			}
			return TRUE;
		case IDC_CHAN15:
			if (IsDlgButtonChecked(hWnd, IDC_CHAN15))
			{
				effect->enableChannel (14, true);
			}
			else
			{
				effect->enableChannel (14, false);
			}
			return TRUE;
		case IDC_CHAN16:
			if (IsDlgButtonChecked(hWnd, IDC_CHAN16))
			{
				effect->enableChannel (15, true);
			}
			else
			{
				effect->enableChannel (15, false);
			}
			return TRUE;
		case IDC_ALL:
			for (i = 0; i < 16; i++)
			{
				effect->enableChannel (i, true);
				CheckDlgButton(hWnd, IDC_CHAN01+i, BST_CHECKED);
			}
			return TRUE;
		case IDC_NONE:
			for (i = 0; i < 16; i++)
			{
				effect->enableChannel (i, false);
				CheckDlgButton(hWnd, IDC_CHAN01+i, BST_UNCHECKED);
			}
			return TRUE;
		case IDCANCEL:
		case IDC_CLOSE:
			EndDialog(hWnd, FALSE);
			return TRUE;
		}
	}
	return FALSE;
}

static BOOL WINAPI DialogProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
#ifdef _WIN64
	OPL3GM* effect = (OPL3GM*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
#else
	OPL3GM* effect = (OPL3GM*)GetWindowLong(hWnd, GWL_USERDATA);
#endif
	switch (message)
	{
	case WM_INITDIALOG:
		return InitDialog(hWnd);
	case WM_HSCROLL:
	case WM_VSCROLL:
		return Scroll(hWnd, lParam, effect);
	case WM_DROPFILES:
		return LoadInstrumentBankDragDrop(hWnd, wParam, effect);
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_PRESET:
			switch (HIWORD(wParam))
			{
			case CBN_SELCHANGE:
				return SetPreset(hWnd, effect);
			default:
				return FALSE;
			}
		case IDC_PRESETNAME:
			switch (HIWORD(wParam))
			{
			case EN_KILLFOCUS:
				return SetPresetName(hWnd, effect);
			default:
				return FALSE;
			}
		case IDC_DISPLAY:
			if (IsDlgButtonChecked(hWnd, IDC_DISPLAY))
			{
				return SetParameterValue(effect, kVolumeDisplay, 1);
			}
			else
			{
				return SetParameterValue(effect, kVolumeDisplay, 0);
			}
		case IDC_NUKED:
			if (IsDlgButtonChecked(hWnd, IDC_NUKED))
			{
				return SetParameterValue(effect, kEmulator, 1);
			}
			else
			{
				return SetParameterValue(effect, kEmulator, 0);
			}
		case IDC_RESAMPLE:
			if (IsDlgButtonChecked(hWnd, IDC_RESAMPLE))
			{
				EnableWindow(GetDlgItem(hWnd, IDC_OPLRATE), TRUE);
				return SetParameterValue(effect, kHQResample, 1);
			}
			else
			{
				EnableWindow(GetDlgItem(hWnd, IDC_OPLRATE), FALSE);
				return SetParameterValue(effect, kHQResample, 0);
			}
		case IDC_DC:
			if (IsDlgButtonChecked(hWnd, IDC_DC))
			{
				return SetParameterValue(effect, kDCBlock, 1);
			}
			else
			{
				return SetParameterValue(effect, kDCBlock, 0);
			}
		case IDC_GATE:
			if (IsDlgButtonChecked(hWnd, IDC_GATE))
			{
				return SetParameterValue(effect, kNoiseGate, 1);
			}
			else
			{
				return SetParameterValue(effect, kNoiseGate, 0);
			}
		case IDC_QUEUE:
			if (IsDlgButtonChecked(hWnd, IDC_QUEUE))
			{
				return SetParameterValue(effect, kPushMidi, 1);
			}
			else
			{
				return SetParameterValue(effect, kPushMidi, 0);
			}
		case IDC_BYPASS:
			return SetBypassState(hWnd, effect);
		case IDC_OPLRATE:
			switch (HIWORD(wParam))
			{
			case CBN_KILLFOCUS:
				return SetOPLRate(hWnd, effect);
			default:
				return FALSE;
			}
		case IDC_REFRESH:
			return RefreshDialog(hWnd, effect);
		case IDC_LOAD:
			return LoadInstrumentBank(hWnd, effect);
		case IDC_PANIC:
			if (effect)
			{
				effect->suspend ();
				return TRUE;
			}
			else
			{
				return FALSE;
			}
		case IDC_RESET:
			if (effect)
			{
				effect->resume ();
				return TRUE;
			}
			else
			{
				return FALSE;
			}
		case IDC_ABOUT:
			return AboutBox(hWnd);
		case IDC_STATS:
			return StatisticsBox(hWnd, effect);
		case IDC_HOSTINFO:
			return HostInfoBox(hWnd, effect);
		case IDC_HARDRESET:
			if (effect)
			{
				effect->hardReset ();
				return TRUE;
			}
			else
			{
				return FALSE;
			}
		case IDC_FORGET:
			if (effect)
			{
				CheckDlgButton(hWnd, IDC_FREEZE, BST_UNCHECKED);
				CheckDlgButton(hWnd, IDC_HIDEDISP, BST_UNCHECKED);
				ShowWindow(GetDlgItem(hWnd, IDC_VOLDISP1), SW_SHOW);
				ShowWindow(GetDlgItem(hWnd, IDC_VOLDISP2), SW_SHOW);
				ShowWindow(GetDlgItem(hWnd, IDC_TRANDISP1), SW_SHOW);
				ShowWindow(GetDlgItem(hWnd, IDC_TRANDISP2), SW_SHOW);
				effect->initializeSettings (true);
				RefreshDialog(hWnd, effect);
				return TRUE;
			}
			else
			{
				return FALSE;
			}
		case IDC_HIDEDISP:
			if (IsDlgButtonChecked(hWnd, IDC_HIDEDISP))
			{
				ShowWindow(GetDlgItem(hWnd, IDC_VOLDISP1), SW_HIDE);
				ShowWindow(GetDlgItem(hWnd, IDC_VOLDISP2), SW_HIDE);
				ShowWindow(GetDlgItem(hWnd, IDC_TRANDISP1), SW_HIDE);
				ShowWindow(GetDlgItem(hWnd, IDC_TRANDISP2), SW_HIDE);
			}
			else
			{
				ShowWindow(GetDlgItem(hWnd, IDC_VOLDISP1), SW_SHOW);
				ShowWindow(GetDlgItem(hWnd, IDC_VOLDISP2), SW_SHOW);
				ShowWindow(GetDlgItem(hWnd, IDC_TRANDISP1), SW_SHOW);
				ShowWindow(GetDlgItem(hWnd, IDC_TRANDISP2), SW_SHOW);
			}
			return TRUE;
		case IDC_MIXER:
			DialogBoxParam((HINSTANCE)hInstance, MAKEINTRESOURCE(IDD_MIXER), hWnd, (DLGPROC)MixerProc, (LPARAM)effect);
			return TRUE;
		case IDC_PROJPAGE:
			return ProjectPage(hWnd);
		}
	}
	return FALSE;
}

static void KeyboardEvent(AudioEffectX* effect, VstInt32 status, VstInt32 channel, VstInt32 data1, VstInt32 data2)
{
	if (effect)
	{
		VstMidiEvent ev;
		VstEvents evs;
		memset(&ev, 0, sizeof(ev));
		memset(&evs, 0, sizeof(evs));
		ev.type = kVstMidiType;
		ev.byteSize = sizeof(VstMidiEvent);
		ev.midiData[0] = (char)(status | channel);
		if (data1 > 127)
		{
			data1 = 127;
		}
		else if (data1 < 0)
		{
			data1 = 0;
		}
		ev.midiData[1] = (char)data1;
		if (data2 > 127)
		{
			data2 = 127;
		}
		else if (data2 < 0)
		{
			data2 = 0;
		}
		ev.midiData[2] = (char)data2;
		evs.numEvents = 1;
		evs.events[0] = (VstEvent*)&ev;
		effect->processEvents (&evs);
	}
}

static void KeyboardNoteOn(KeyboardInfo* info, VstInt32 note)
{
	if (note == -1)
	{
		return;
	}
	if (info)
	{
		KeyboardEvent(info->Effect, 0x90, info->Channel, (12 * info->Octave) + note, info->Velocity);
	}
}

static void KeyboardNoteOff(KeyboardInfo* info, VstInt32 note)
{
	if (note == -1)
	{
		return;
	}
	if (info)
	{
		KeyboardEvent(info->Effect, 0x80, info->Channel, (12 * info->Octave) + note, info->Velocity);
	}
}

static void KeyboardControlChange(KeyboardInfo* info, VstInt32 type, VstInt32 data)
{
	if (info)
	{
		KeyboardEvent(info->Effect, 0xb0, info->Channel, type, data);
	}
}

static void KeyboardProgramChange(KeyboardInfo* info)
{
	if (info)
	{
		KeyboardEvent(info->Effect, 0xc0, info->Channel, info->Program, 0);
	}
}

static void KeyboardPitchBend(KeyboardInfo* info)
{
	if (info)
	{
		KeyboardEvent(info->Effect, 0xe0, info->Channel, info->BendLSB, info->BendMSB);
	}
}

static VstInt32 char2note(HWND hWnd, WPARAM wParam)
{
	switch (wParam)
	{
	case 0x41:	//a,c
		SetWindowText(hWnd, "C");
		return 0;
	case 0x53:	//s,d
		SetWindowText(hWnd, "D");
		return 2;
	case 0x44:	//d,e
		SetWindowText(hWnd, "E");
		return 4;
	case 0x46:	//f,f
		SetWindowText(hWnd, "F");
		return 5;
	case 0x47:	//g,g
		SetWindowText(hWnd, "G");
		return 7;
	case 0x48:	//h,a
		SetWindowText(hWnd, "A");
		return 9;
	case 0x4a:	//j,b
		SetWindowText(hWnd, "B");
		return 11;
	case 0x4b:	//k,c
		SetWindowText(hWnd, "C");
		return 12;
	case 0x4c:	//l,d
		SetWindowText(hWnd, "D");
		return 14;
	case VK_OEM_1:	//e
		SetWindowText(hWnd, "E");
		return 16;
	case VK_OEM_7:	//f
		SetWindowText(hWnd, "F");
		return 17;
	case 0x51:	//q,c#
		SetWindowText(hWnd, "C#");
		return 1;
	case 0x57:	//w,d#
		SetWindowText(hWnd, "D#");
		return 3;
	case 0x45:	//e,f#
		SetWindowText(hWnd, "F#");
		return 6;
	case 0x52:	//r,g#
		SetWindowText(hWnd, "G#");
		return 8;
	case 0x54:	//t,a#
		SetWindowText(hWnd, "A#");
		return 10;
	case 0x59:	//y,c#
		SetWindowText(hWnd, "C#");
		return 13;
	case 0x55:	//u,d#
		SetWindowText(hWnd, "D#");
		return 15;
	case 0x49:	//i,f#
		SetWindowText(hWnd, "F#");
		return 18;
	case 0x4f:	//o,g#
		SetWindowText(hWnd, "G#");
		return 20;
	case 0x50:	//p,a#
		SetWindowText(hWnd, "A#");
		return 22;
	case VK_OEM_4:	//c#
		SetWindowText(hWnd, "C#");
		return 25;
	case VK_OEM_6:	//d#
		SetWindowText(hWnd, "D#");
		return 27;
	}
	return -1;
}

static BOOL KeepNotes(WPARAM wParam)
{
	switch (wParam)
	{
	case VK_OEM_PLUS:
		return TRUE;
	case VK_OEM_MINUS:
		return TRUE;
	case VK_OEM_5:
		return TRUE;
	case VK_OEM_3:
		return TRUE;
	case VK_SHIFT:
		return TRUE;
	}
	return FALSE;
}

static BOOL KeyDown(HWND hWnd, WPARAM wParam, LPARAM lParam, KeyboardInfo* info)
{
	VstInt32 note = char2note(hWnd, wParam);
	if (note >= 0)
	{
		if (!(lParam & KEYWASDOWN))
		{
			KeyboardNoteOn(info, note);
			return FALSE;
		}
		return TRUE;
	}
	if (!KeepNotes(wParam))
	{
		KeyboardControlChange(info, 0x40, 0);
		KeyboardControlChange(info, 0x7b, 0);
	}
	char text[MAX_PATH];
	ZeroMemory(text, sizeof(text));
	switch (wParam)
	{
	case 0x5a:	//z
		info->Velocity = 10;
		sprintf(text, "Velocity %d", info->Velocity);
		SetWindowText(hWnd, text);
		return FALSE;
	case 0x58:	//x
		info->Velocity = 20;
		sprintf(text, "Velocity %d", info->Velocity);
		SetWindowText(hWnd, text);
		return FALSE;
	case 0x43:	//c
		info->Velocity = 30;
		sprintf(text, "Velocity %d", info->Velocity);
		SetWindowText(hWnd, text);
		return FALSE;
	case 0x56:	//v
		info->Velocity = 40;
		sprintf(text, "Velocity %d", info->Velocity);
		SetWindowText(hWnd, text);
		return FALSE;
	case 0x42:	//b
		info->Velocity = 50;
		sprintf(text, "Velocity %d", info->Velocity);
		SetWindowText(hWnd, text);
		return FALSE;
	case 0x4e:	//n
		info->Velocity = 60;
		sprintf(text, "Velocity %d", info->Velocity);
		SetWindowText(hWnd, text);
		return FALSE;
	case 0x4d:	//m
		info->Velocity = 70;
		sprintf(text, "Velocity %d", info->Velocity);
		SetWindowText(hWnd, text);
		return FALSE;
	case VK_OEM_COMMA:
		info->Velocity = 80;
		sprintf(text, "Velocity %d", info->Velocity);
		SetWindowText(hWnd, text);
		return FALSE;
	case VK_OEM_PERIOD:
		info->Velocity = 90;
		sprintf(text, "Velocity %d", info->Velocity);
		SetWindowText(hWnd, text);
		return FALSE;
	case VK_OEM_2:
		info->Velocity = 100;
		sprintf(text, "Velocity %d", info->Velocity);
		SetWindowText(hWnd, text);
		return FALSE;
	case 0x30:	//0
		info->Octave = 0;
		sprintf(text, "Octave %d", info->Octave);
		SetWindowText(hWnd, text);
		return FALSE;
	case 0x31:	//1
		info->Octave = 1;
		sprintf(text, "Octave %d", info->Octave);
		SetWindowText(hWnd, text);
		return FALSE;
	case 0x32:	//2
		info->Octave = 2;
		sprintf(text, "Octave %d", info->Octave);
		SetWindowText(hWnd, text);
		return FALSE;
	case 0x33:	//3
		info->Octave = 3;
		sprintf(text, "Octave %d", info->Octave);
		SetWindowText(hWnd, text);
		return FALSE;
	case 0x34:	//4
		info->Octave = 4;
		sprintf(text, "Octave %d", info->Octave);
		SetWindowText(hWnd, text);
		return FALSE;
	case 0x35:	//5
		info->Octave = 5;
		sprintf(text, "Octave %d", info->Octave);
		SetWindowText(hWnd, text);
		return FALSE;
	case 0x36:	//6
		info->Octave = 6;
		sprintf(text, "Octave %d", info->Octave);
		SetWindowText(hWnd, text);
		return FALSE;
	case 0x37:	//7
		info->Octave = 7;
		sprintf(text, "Octave %d", info->Octave);
		SetWindowText(hWnd, text);
		return FALSE;
	case 0x38:	//8
		info->Octave = 8;
		sprintf(text, "Octave %d", info->Octave);
		SetWindowText(hWnd, text);
		return FALSE;
	case 0x39:	//9
		info->Octave = 9;
		sprintf(text, "Octave %d", info->Octave);
		SetWindowText(hWnd, text);
		return FALSE;
	case VK_OEM_PLUS:
		info->BendMSB++;
		if (info->BendMSB > 127)
		{
			info->BendMSB = 127;
			MessageBeep(MB_OK);
			return TRUE;
		}
		KeyboardPitchBend(info);
		return FALSE;
	case VK_OEM_MINUS:
		info->BendMSB--;
		if (info->BendMSB < 0)
		{
			info->BendMSB = 0;
			MessageBeep(MB_OK);
			return TRUE;
		}
		KeyboardPitchBend(info);
		return FALSE;
	case VK_OEM_5:
		info->BendLSB++;
		if (info->BendLSB > 127)
		{
			info->BendLSB = 127;
			MessageBeep(MB_OK);
			return TRUE;
		}
		KeyboardPitchBend(info);
		return FALSE;
	case VK_OEM_3:
		info->BendLSB--;
		if (info->BendLSB < 0)
		{
			info->BendLSB = 0;
			MessageBeep(MB_OK);
			return TRUE;
		}
		KeyboardPitchBend(info);
		return FALSE;
	case VK_RIGHT:
		info->Octave++;
		if (info->Octave > 10)
		{
			info->Octave = 10;
			MessageBeep(MB_OK);
			return TRUE;
		}
		sprintf(text, "Octave %d", info->Octave);
		SetWindowText(hWnd, text);
		return FALSE;
	case VK_LEFT:
		info->Octave--;
		if (info->Octave < 0)
		{
			info->Octave = 0;
			MessageBeep(MB_OK);
			return TRUE;
		}
		sprintf(text, "Octave %d", info->Octave);
		SetWindowText(hWnd, text);
		return FALSE;
	case VK_UP:
		info->Velocity++;
		if (info->Velocity > 127)
		{
			info->Velocity = 127;
			MessageBeep(MB_OK);
			return TRUE;
		}
		sprintf(text, "Velocity %d", info->Velocity);
		SetWindowText(hWnd, text);
		return FALSE;
	case VK_DOWN:
		info->Velocity--;
		if (info->Velocity < 0)
		{
			info->Velocity = 0;
			MessageBeep(MB_OK);
			return TRUE;
		}
		sprintf(text, "Velocity %d", info->Velocity);
		SetWindowText(hWnd, text);
		return FALSE;
	case VK_NEXT:
		info->Program++;
		if (info->Program > 127)
		{
			info->Program = 127;
			MessageBeep(MB_OK);
			return TRUE;
		}
		if (info->Channel == 9)
		{
			SetWindowText(hWnd, GmDrumSets[0]);
		}
		else
		{
			SetWindowText(hWnd, GmNames[info->Program]);
		}
		KeyboardProgramChange(info);
		return FALSE;
	case VK_PRIOR:
		info->Program--;
		if (info->Program < 0)
		{
			info->Program = 0;
			MessageBeep(MB_OK);
			return TRUE;
		}
		if (info->Channel == 9)
		{
			SetWindowText(hWnd, GmDrumSets[0]);
		}
		else
		{
			SetWindowText(hWnd, GmNames[info->Program]);
		}
		KeyboardProgramChange(info);
		return FALSE;
	case VK_END:
		info->Channel++;
		if (info->Channel > 15)
		{
			info->Channel = 15;
			MessageBeep(MB_OK);
			return TRUE;
		}
		sprintf(text, "Channel %d", info->Channel+1);
		SetWindowText(hWnd, text);
		return FALSE;
	case VK_HOME:
		info->Channel--;
		if (info->Channel < 0)
		{
			info->Channel = 0;
			MessageBeep(MB_OK);
			return TRUE;
		}
		sprintf(text, "Channel %d", info->Channel+1);
		SetWindowText(hWnd, text);
		return FALSE;
	case VK_SHIFT:
		if (!(lParam & KEYWASDOWN))
		{
			KeyboardControlChange(info, 0x40, 127);
			return FALSE;
		}
		return TRUE;
	case VK_BACK:
		KeyboardProgramChange(info);
		KeyboardPitchBend(info);
		SetWindowText(hWnd, "Synth refreshed");
		return FALSE;
	case VK_SPACE:
		info->Channel = 0;
		info->Octave = 4;
		info->Velocity = 127;
		info->Program = 0;
		info->BendMSB = 64;
		info->BendLSB = 0;
		KeyboardProgramChange(info);
		KeyboardPitchBend(info);
		SetWindowText(hWnd, "Keyboard reset");
		return FALSE;
	}
	return TRUE;
}

static BOOL KeyUp(WPARAM wParam, KeyboardInfo* info)
{
	VstInt32 note = char2note(NULL, wParam);
	if (note >= 0)
	{
		KeyboardNoteOff(info, note);
		return FALSE;
	}
	switch (wParam)
	{
	case VK_SHIFT:
		KeyboardControlChange(info, 0x40, 0);
		return FALSE;
	}
	return TRUE;
}

static LRESULT WINAPI KeyboardProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
#ifdef _WIN64
	KeyboardInfo* info = (KeyboardInfo*)GetWindowLongPtr(hWnd, 0);
#else
	KeyboardInfo* info = (KeyboardInfo*)GetWindowLong(hWnd, 0);
#endif
	switch (message)
	{
	case WM_GETDLGCODE:
		return DLGC_WANTCHARS | DLGC_WANTARROWS;
	case WM_KEYDOWN:
		return KeyDown(hWnd, wParam, lParam, info);
	case WM_KEYUP:
		return KeyUp(wParam, info);
	case WM_KILLFOCUS:
		KeyboardControlChange(info, 0x40, 0);
		KeyboardControlChange(info, 0x7b, 0);
		SetWindowText(hWnd, "");
		return 0;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

Editor::Editor (AudioEffect* effect)
: AEffEditor (effect)
{
	memset(&vstrect, 0, sizeof(vstrect));
	dlg = NULL;
	memset(&keyboard, 0, sizeof(keyboard));
	keyboard.Octave = 4;
	keyboard.Velocity = 127;
	keyboard.BendMSB = 64;
	if (effect)
	{
		effect->setEditor (this);
		keyboard.Effect = (AudioEffectX*)effect;
	}
	InitCommonControls();
	g_useCount++;
	if (g_useCount == 1)
	{
		WNDCLASS KeyboardClass;
		ZeroMemory(&KeyboardClass, sizeof(KeyboardClass));
		KeyboardClass.style = CS_HREDRAW | CS_VREDRAW;
		KeyboardClass.lpfnWndProc = (WNDPROC)KeyboardProc;
		KeyboardClass.cbWndExtra = sizeof(KeyboardInfo*);
		KeyboardClass.hInstance = (HINSTANCE)hInstance;
		KeyboardClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
		KeyboardClass.hCursor = LoadCursor(NULL, IDC_ARROW);
		HBITMAP hBitmap = LoadBitmap((HINSTANCE)hInstance, MAKEINTRESOURCE(IDB_BITMAP1));
		if (hBitmap)
		{
			hBrush = CreatePatternBrush(hBitmap);
			if (hBrush)
			{
				KeyboardClass.hbrBackground = hBrush;
			}
			else
			{
				KeyboardClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
			}
		}
		KeyboardClass.lpszClassName = CLASSNAME;
		RegisterClass(&KeyboardClass);
	}
}

Editor::~Editor ()
{
	g_useCount--;
	if (g_useCount == 0)
	{
		UnregisterClass(CLASSNAME, (HINSTANCE)hInstance);
		if (hBrush)
		{
			DeleteObject(hBrush);
			hBrush = NULL;
		}
	}
}

bool Editor::getRect (ERect** rect)
{
	if (dlg && rect)
	{
		RECT wndrect;
		ZeroMemory(&wndrect, sizeof(wndrect));
		if (GetWindowRect((HWND)dlg, &wndrect))
		{
			vstrect.top = (VstInt16)wndrect.top;
			vstrect.left = (VstInt16)wndrect.left;
			vstrect.bottom = (VstInt16)wndrect.bottom;
			vstrect.right = (VstInt16)wndrect.right;
			*rect = &vstrect;
			return true;
		}
	}
	return false;
}

bool Editor::open (void* ptr)
{
	if (!ptr)
	{
		return false;
	}
	AEffEditor::open (ptr);
	dlg = CreateDialog((HINSTANCE)hInstance, MAKEINTRESOURCE(IDD_DIALOG), (HWND)systemWindow, (DLGPROC)DialogProc);
	if (dlg)
	{
		char caption[MAX_PATH];
		char text[MAX_PATH];
		char synthname[kVstMaxEffectNameLen];
		ZeroMemory(caption, sizeof(caption));
		ZeroMemory(text, sizeof(text));
		ZeroMemory(synthname, sizeof(synthname));
		if (effect)
		{
			if (((OPL3GM*)effect)->getErrorText (text))
			{
				LoadString((HINSTANCE)hInstance, IDS_FAILCAP, caption, MAX_PATH);
				MessageBox((HWND)dlg, text, caption, MB_ICONERROR);
			}
			((AudioEffectX*)effect)->getEffectName (synthname);
			SetWindowText((HWND)dlg, synthname);
#ifdef _WIN64
			SetWindowLongPtr((HWND)dlg, GWLP_USERDATA, (LONG_PTR)effect);
#else
			SetWindowLong((HWND)dlg, GWL_USERDATA, (LONG)effect);
#endif
		}
		HWND kbdwin = GetDlgItem((HWND)dlg, IDC_KEYBOARD);
		if (kbdwin)
		{
#ifdef _WIN64
			SetWindowLongPtr(kbdwin, 0, (LONG_PTR)&keyboard);
#else
			SetWindowLong(kbdwin, 0, (LONG)&keyboard);
#endif
		}
		RefreshDialog((HWND)dlg, (OPL3GM*)effect);
		DragAcceptFiles((HWND)dlg, TRUE);
		ShowWindow((HWND)dlg, SW_SHOW);
		UpdateWindow((HWND)dlg);
		return true;
	}
	return false;
}

void Editor::close ()
{
	if (dlg)
	{
		DestroyWindow((HWND)dlg);
		dlg = NULL;
	}
	AEffEditor::close ();
}

void Editor::idle ()
{
	if (effect && dlg)
	{
		if (IsDlgButtonChecked((HWND)dlg, IDC_FREEZE))
		{
			return;
		}
		char text[MAX_PATH];
		ZeroMemory(text, sizeof(text));
		VstInt32 numvoices = ((OPL3GM*)effect)->getActiveVoices ();
		sprintf(text, "%d/18", numvoices);
		SetDlgItemText((HWND)dlg, IDC_VOICECOUNT, text);
		float vu = ((OPL3GM*)effect)->getVu ();
		char vustr[kVstMaxParamStrLen*2];
		ZeroMemory(vustr, sizeof(vustr));
		effect->dB2string (vu, vustr, (kVstMaxParamStrLen*2)-1);
		sprintf(text, "%s dB", vustr);
		SetDlgItemText((HWND)dlg, IDC_VU, text);
		double cpu = ((OPL3GM*)effect)->getCPULoad ();
		sprintf(text, "%lf %%", cpu);
		SetDlgItemText((HWND)dlg, IDC_CPU, text);
		COLORREF color = 0;
		HDC hDc = GetDC(GetDlgItem((HWND)dlg, IDC_VOICECOUNT));
		if (hDc)
		{
			if (numvoices > 15)
			{
				color = RGB(255, 0, 0);
			}
			SetTextColor(hDc, color);
			ReleaseDC(GetDlgItem((HWND)dlg, IDC_VOICECOUNT), hDc);
			hDc = NULL;
		}
		color = 0;
		hDc = GetDC(GetDlgItem((HWND)dlg, IDC_VU));
		if (hDc)
		{
			if (vu > 1)
			{
				color = RGB(255, 0, 0);
			}
			SetTextColor(hDc, color);
			ReleaseDC(GetDlgItem((HWND)dlg, IDC_VU), hDc);
			hDc = NULL;
		}
		color = 0;
		hDc = GetDC(GetDlgItem((HWND)dlg, IDC_CPU));
		if (hDc)
		{
			if (cpu > 50)
			{
				color = RGB(255, 0, 0);
			}
			SetTextColor(hDc, color);
			ReleaseDC(GetDlgItem((HWND)dlg, IDC_CPU), hDc);
			hDc = NULL;
		}
	}
}

void Editor::refresh ()
{
	RefreshDialog((HWND)dlg, (OPL3GM*)effect);
}
