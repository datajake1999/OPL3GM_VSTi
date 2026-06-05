/*
OPL3GM VSTi Editor
Copyright (C) 2021-2026  Datajake

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
#include <tchar.h>
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
#define KEYNAME _T("SOFTWARE\\Datajake\\OPL3GM")
#define PROJPAGE _T("https://github.com/datajake1999/OPL3GM_VSTi")

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
		TCHAR text[MAX_PATH];
		char ansi[MAX_PATH];
		ZeroMemory(text, sizeof(text));
		ZeroMemory(ansi, sizeof(ansi));
		if (GetDlgItemText(hWnd, IDC_PRESETNAME, text, MAX_PATH))
		{
#ifdef UNICODE
			WideCharToMultiByte(CP_ACP, 0, text, -1, ansi, MAX_PATH, NULL, NULL);
#else
			strncpy(ansi, text, MAX_PATH);
#endif
			effect->setProgramName (ansi);
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
		TCHAR text[MAX_PATH];
		ZeroMemory(text, sizeof(text));
		if (GetDlgItemText(hWnd, IDC_OPLRATE, text, MAX_PATH))
		{
			VstInt32 rate = _ttoi(text);
			if (rate != (VstInt32)effect->getSampleRate ())
			{
				effect->setInternalRate (rate);
			}
			_stprintf(text, _T("%d"), effect->getInternalRate ());
			SetDlgItemText(hWnd, IDC_OPLRATE, text);
			return TRUE;
		}
	}
	return FALSE;
}

static void UpdateMeters(HWND hWnd, EditorState* state, OPL3GM* effect, BOOL IdleCall)
{
	if (hWnd && state && effect)
	{
		if (IdleCall && IsDlgButtonChecked(hWnd, IDC_FREEZE))
		{
			return;
		}
		TCHAR text[MAX_PATH];
		ZeroMemory(text, sizeof(text));
		VstInt32 numvoices = effect->getActiveVoices ();
		_stprintf(text, _T("%d/18"), numvoices);
		SetDlgItemText(hWnd, IDC_VOICECOUNT, text);
		float vu = effect->getVu ();
		TCHAR vustr[kVstMaxParamStrLen*2];
		char ansi[kVstMaxParamStrLen*2];
		ZeroMemory(vustr, sizeof(vustr));
		ZeroMemory(ansi, sizeof(ansi));
		effect->dB2string (vu, ansi, (kVstMaxParamStrLen*2)-1);
#ifdef UNICODE
		MultiByteToWideChar(CP_ACP, 0, ansi, -1, vustr, kVstMaxParamStrLen*2);
#else
		strncpy(vustr, ansi, kVstMaxParamStrLen*2);
#endif
		_stprintf(text, _T("%s dB"), vustr);
		SetDlgItemText(hWnd, IDC_VU, text);
		double cpu = effect->getCPULoad ();
		_stprintf(text, _T("%lf %%"), cpu);
		SetDlgItemText(hWnd, IDC_CPU, text);
		if (numvoices > 15)
		{
			state->VoiceAlert = true;
		}
		if (vu > 1)
		{
			state->VUAlert = true;
		}
		if (cpu > 50)
		{
			state->CPUAlert = true;
		}
		InvalidateRect(GetDlgItem(hWnd, IDC_VOICECOUNT), NULL, FALSE);
		UpdateWindow(GetDlgItem(hWnd, IDC_VOICECOUNT));
		InvalidateRect(GetDlgItem(hWnd, IDC_VU), NULL, FALSE);
		UpdateWindow(GetDlgItem(hWnd, IDC_VU));
		InvalidateRect(GetDlgItem(hWnd, IDC_CPU), NULL, FALSE);
		UpdateWindow(GetDlgItem(hWnd, IDC_CPU));
	}
}

static BOOL UpdateMeterColor(HWND hWnd, WPARAM wParam, LPARAM lParam, EditorState* state)
{
	if (hWnd && wParam && lParam && state)
	{
		if ((HWND)lParam == GetDlgItem(hWnd, IDC_VOICECOUNT))
		{
			HDC hDC = (HDC)wParam;
			SetBkMode(hDC, TRANSPARENT);
			if (state->VoiceAlert)
			{
				SetTextColor(hDC, RGB(255, 0, 0));
				state->VoiceAlert = false;
			}
			else
			{
				SetTextColor(hDC, RGB(0, 0, 0));
			}
			return (BOOL)GetStockObject(WHITE_BRUSH);
		}
		else if ((HWND)lParam == GetDlgItem(hWnd, IDC_VU))
		{
			HDC hDC = (HDC)wParam;
			SetBkMode(hDC, TRANSPARENT);
			if (state->VUAlert)
			{
				SetTextColor(hDC, RGB(255, 0, 0));
				state->VUAlert = false;
			}
			else
			{
				SetTextColor(hDC, RGB(0, 0, 0));
			}
			return (BOOL)GetStockObject(WHITE_BRUSH);
		}
		else if ((HWND)lParam == GetDlgItem(hWnd, IDC_CPU))
		{
			HDC hDC = (HDC)wParam;
			SetBkMode(hDC, TRANSPARENT);
			if (state->CPUAlert)
			{
				SetTextColor(hDC, RGB(255, 0, 0));
				state->CPUAlert = false;
			}
			else
			{
				SetTextColor(hDC, RGB(0, 0, 0));
			}
			return (BOOL)GetStockObject(WHITE_BRUSH);
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
		TCHAR text[MAX_PATH];
		ZeroMemory(text, sizeof(text));
		for (VstInt32 i = 0; i < sizeof(rates)/sizeof(int); i++)
		{
			_stprintf(text, _T("%d"), rates[i]);
			SendDlgItemMessage(hWnd, IDC_OPLRATE, CB_INSERTSTRING, i, (LPARAM)text);
		}
		return TRUE;
	}
	return FALSE;
}

static BOOL RefreshDialog(HWND hWnd, EditorState* state, OPL3GM* effect)
{
	if (hWnd && state && effect)
	{
		float ParamValue;
		TCHAR text[MAX_PATH];
		char ansi[MAX_PATH];
		ZeroMemory(text, sizeof(text));
		ZeroMemory(ansi, sizeof(ansi));
		SendDlgItemMessage(hWnd, IDC_PRESET, CB_RESETCONTENT, 0, 0);
		for (VstInt32 i = 0; i < kNumPrograms; i++)
		{
			effect->getProgramNameIndexed (-1, i, ansi);
#ifdef UNICODE
			MultiByteToWideChar(CP_ACP, 0, ansi, -1, text, MAX_PATH);
#else
			strncpy(text, ansi, MAX_PATH);
#endif
			SendDlgItemMessage(hWnd, IDC_PRESET, CB_INSERTSTRING, i, (LPARAM)text);
		}
		SendDlgItemMessage(hWnd, IDC_PRESET, CB_SETCURSEL, effect->getProgram (), 0);
		effect->getProgramName (ansi);
#ifdef UNICODE
		MultiByteToWideChar(CP_ACP, 0, ansi, -1, text, MAX_PATH);
#else
		strncpy(text, ansi, MAX_PATH);
#endif
		SetDlgItemText(hWnd, IDC_PRESETNAME, text);
		ParamValue = effect->getParameter (kVolume)*100;
		SendDlgItemMessage(hWnd, IDC_VOLUME, TBM_SETPOS, TRUE, (LPARAM)ParamValue);
		effect->getParameterDisplay (kVolume, ansi);
#ifdef UNICODE
		MultiByteToWideChar(CP_ACP, 0, ansi, -1, text, MAX_PATH);
#else
		strncpy(text, ansi, MAX_PATH);
#endif
		SetDlgItemText(hWnd, IDC_VOLDISP1, text);
		effect->getParameterLabel (kVolume, ansi);
#ifdef UNICODE
		MultiByteToWideChar(CP_ACP, 0, ansi, -1, text, MAX_PATH);
#else
		strncpy(text, ansi, MAX_PATH);
#endif
		SetDlgItemText(hWnd, IDC_VOLDISP2, text);
		ParamValue = effect->getParameter (kTranspose)*25;
		SendDlgItemMessage(hWnd, IDC_TRANSPOSE, TBM_SETPOS, TRUE, (LPARAM)ParamValue);
		effect->getParameterDisplay (kTranspose, ansi);
#ifdef UNICODE
		MultiByteToWideChar(CP_ACP, 0, ansi, -1, text, MAX_PATH);
#else
		strncpy(text, ansi, MAX_PATH);
#endif
		SetDlgItemText(hWnd, IDC_TRANDISP1, text);
		effect->getParameterLabel (kTranspose, ansi);
#ifdef UNICODE
		MultiByteToWideChar(CP_ACP, 0, ansi, -1, text, MAX_PATH);
#else
		strncpy(text, ansi, MAX_PATH);
#endif
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
		_stprintf(text, _T("%d"), effect->getInternalRate ());
		SetDlgItemText(hWnd, IDC_OPLRATE, text);
		effect->getBankName (ansi, MAX_PATH);
#ifdef UNICODE
		MultiByteToWideChar(CP_ACP, 0, ansi, -1, text, MAX_PATH);
#else
		strncpy(text, ansi, MAX_PATH);
#endif
		SetDlgItemText(hWnd, IDC_CURBANK, text);
		if (effect->getFreezeMeters ())
		{
			CheckDlgButton(hWnd, IDC_FREEZE, BST_CHECKED);
		}
		else
		{
			CheckDlgButton(hWnd, IDC_FREEZE, BST_UNCHECKED);
		}
		if (effect->getHideParameters ())
		{
			CheckDlgButton(hWnd, IDC_HIDEDISP, BST_CHECKED);
			ShowWindow(GetDlgItem(hWnd, IDC_VOLDISP1), SW_HIDE);
			ShowWindow(GetDlgItem(hWnd, IDC_VOLDISP2), SW_HIDE);
			ShowWindow(GetDlgItem(hWnd, IDC_TRANDISP1), SW_HIDE);
			ShowWindow(GetDlgItem(hWnd, IDC_TRANDISP2), SW_HIDE);
		}
		else
		{
			CheckDlgButton(hWnd, IDC_HIDEDISP, BST_UNCHECKED);
			ShowWindow(GetDlgItem(hWnd, IDC_VOLDISP1), SW_SHOW);
			ShowWindow(GetDlgItem(hWnd, IDC_VOLDISP2), SW_SHOW);
			ShowWindow(GetDlgItem(hWnd, IDC_TRANDISP1), SW_SHOW);
			ShowWindow(GetDlgItem(hWnd, IDC_TRANDISP2), SW_SHOW);
		}
		UpdateMeters(hWnd, state, effect, FALSE);
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
		TCHAR caption[MAX_PATH];
		TCHAR text[MAX_PATH];
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
		TCHAR filename[MAX_PATH];
		TCHAR title[MAX_PATH];
		TCHAR filter[MAX_PATH];
		TCHAR caption[MAX_PATH];
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
			ofn.lpstrDefExt = _T("TMB");
		}
		else if (!strcmp(synthname, "Doom OPL3"))
		{
			ofn.nFilterIndex = 3;
			ofn.lpstrDefExt = _T("OP2");
		}
		else if (!strcmp(synthname, "Windows 9x OPL3"))
		{
			TCHAR text[MAX_PATH];
			ZeroMemory(text, sizeof(text));
			LoadString((HINSTANCE)hInstance, IDS_W9XCAP, caption, MAX_PATH);
			LoadString((HINSTANCE)hInstance, IDS_W9XTXT, text, MAX_PATH);
			MessageBox(hWnd, text, caption, MB_ICONEXCLAMATION);
			return FALSE;
		}
		HKEY hKey;
		DWORD expstyle = 1;
		DWORD help = 0;
		TCHAR directory[MAX_PATH];
		ZeroMemory(directory, sizeof(directory));
		if (RegOpenKeyEx(HKEY_CURRENT_USER, KEYNAME, 0, KEY_READ, &hKey) == ERROR_SUCCESS)
		{
			DWORD type = REG_DWORD;
			DWORD len = sizeof(DWORD);
			RegQueryValueEx(hKey, _T("ExplorerStyle"), NULL, &type, (LPBYTE)&expstyle, &len);
			RegQueryValueEx(hKey, _T("HelpButton"), NULL, &type, (LPBYTE)&help, &len);
			type = REG_SZ;
			len = sizeof(directory);
			if (RegQueryValueEx(hKey, _T("ApogeePatchDir"), NULL, &type, (LPBYTE)directory, &len) == ERROR_SUCCESS && !strcmp(synthname, "Apogee OPL3"))
			{
				ofn.lpstrInitialDir = directory;
			}
			else if (RegQueryValueEx(hKey, _T("DoomPatchDir"), NULL, &type, (LPBYTE)directory, &len) == ERROR_SUCCESS && !strcmp(synthname, "Doom OPL3"))
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
			char ansi_filename[MAX_PATH];
			char ansi_title[MAX_PATH];
			ZeroMemory(ansi_filename, sizeof(ansi_filename));
			ZeroMemory(ansi_title, sizeof(ansi_title));
#ifdef UNICODE
			WideCharToMultiByte(CP_ACP, 0, filename, -1, ansi_filename, MAX_PATH, NULL, NULL);
			WideCharToMultiByte(CP_ACP, 0, title, -1, ansi_title, MAX_PATH, NULL, NULL);
#else
			strncpy(ansi_filename, filename, MAX_PATH);
			strncpy(ansi_title, title, MAX_PATH);
#endif
			if (!effect->loadInstruments (ansi_filename, ansi_title))
			{
				TCHAR temp[MAX_PATH];
				TCHAR text[MAX_PATH];
				ZeroMemory(temp, sizeof(temp));
				ZeroMemory(text, sizeof(text));
				LoadString((HINSTANCE)hInstance, IDS_FAILCAP, caption, MAX_PATH);
				LoadString((HINSTANCE)hInstance, IDS_FAILTXT, temp, MAX_PATH);
				_stprintf(text, temp, filename);
				MessageBox(hWnd, text, caption, MB_ICONERROR);
				return FALSE;
			}
			SetDlgItemText(hWnd, IDC_CURBANK, title);
			return TRUE;
		}
	}
	return FALSE;
}

static BOOL LoadInstrumentBankDragDrop(HWND hWnd, WPARAM wParam, OPL3GM* effect)
{
	HDROP hDrop = (HDROP)wParam;
	if (hWnd && effect && hDrop)
	{
		char synthname[kVstMaxEffectNameLen];
		ZeroMemory(synthname, sizeof(synthname));
		effect->getEffectName (synthname);
		if (!strcmp(synthname, "Windows 9x OPL3"))
		{
			TCHAR caption[MAX_PATH];
			TCHAR text[MAX_PATH];
			ZeroMemory(caption, sizeof(caption));
			ZeroMemory(text, sizeof(text));
			LoadString((HINSTANCE)hInstance, IDS_W9XCAP, caption, MAX_PATH);
			LoadString((HINSTANCE)hInstance, IDS_W9XTXT, text, MAX_PATH);
			MessageBox(hWnd, text, caption, MB_ICONEXCLAMATION);
			DragFinish(hDrop);
			return FALSE;
		}
		TCHAR filename[MAX_PATH];
		TCHAR title[MAX_PATH];
		ZeroMemory(filename, sizeof(filename));
		ZeroMemory(title, sizeof(title));
		if (DragQueryFile(hDrop, 0, filename, MAX_PATH))
		{
			GetFileTitle(filename, title, MAX_PATH);
			char ansi_filename[MAX_PATH];
			char ansi_title[MAX_PATH];
			ZeroMemory(ansi_filename, sizeof(ansi_filename));
			ZeroMemory(ansi_title, sizeof(ansi_title));
#ifdef UNICODE
			WideCharToMultiByte(CP_ACP, 0, filename, -1, ansi_filename, MAX_PATH, NULL, NULL);
			WideCharToMultiByte(CP_ACP, 0, title, -1, ansi_title, MAX_PATH, NULL, NULL);
#else
			strncpy(ansi_filename, filename, MAX_PATH);
			strncpy(ansi_title, title, MAX_PATH);
#endif
			if (!effect->loadInstruments (ansi_filename, ansi_title))
			{
				TCHAR caption[MAX_PATH];
				TCHAR temp[MAX_PATH];
				TCHAR text[MAX_PATH];
				ZeroMemory(caption, sizeof(caption));
				ZeroMemory(temp, sizeof(temp));
				ZeroMemory(text, sizeof(text));
				LoadString((HINSTANCE)hInstance, IDS_FAILCAP, caption, MAX_PATH);
				LoadString((HINSTANCE)hInstance, IDS_FAILTXT, temp, MAX_PATH);
				_stprintf(text, temp, filename);
				MessageBox(hWnd, text, caption, MB_ICONERROR);
				DragFinish(hDrop);
				return FALSE;
			}
			SetDlgItemText(hWnd, IDC_CURBANK, title);
			DragFinish(hDrop);
			return TRUE;
		}
	}
	if (hDrop)
	{
		DragFinish(hDrop);
	}
	return FALSE;
}

static BOOL AboutBox(HWND hWnd)
{
	if (hWnd)
	{
		TCHAR caption[MAX_PATH];
		TCHAR text[MAX_PATH];
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
		TCHAR caption[MAX_PATH];
		TCHAR temp[MAX_PATH];
		TCHAR text[MAX_PATH];
		ZeroMemory(caption, sizeof(caption));
		ZeroMemory(temp, sizeof(temp));
		ZeroMemory(text, sizeof(text));
		LoadString((HINSTANCE)hInstance, IDS_STATSCAP, caption, MAX_PATH);
		LoadString((HINSTANCE)hInstance, IDS_STATSTXT, temp, MAX_PATH);
		_stprintf(text, temp, (VstInt32)effect->getSampleRate (), effect->getInternalRate (), effect->getBlockSize (), g_useCount);
		MessageBox(hWnd, text, caption, MB_ICONINFORMATION);
		return TRUE;
	}
	return FALSE;
}

static BOOL HostInfoBox(HWND hWnd, OPL3GM* effect)
{
	if (hWnd && effect)
	{
		TCHAR caption[MAX_PATH];
		TCHAR temp[MAX_PATH];
		TCHAR text[MAX_PATH];
		TCHAR VendorString[kVstMaxVendorStrLen];
		TCHAR ProductString[kVstMaxProductStrLen];
		ZeroMemory(caption, sizeof(caption));
		ZeroMemory(temp, sizeof(temp));
		ZeroMemory(text, sizeof(text));
		ZeroMemory(VendorString, sizeof(VendorString));
		ZeroMemory(ProductString, sizeof(ProductString));
		HostInfo* hi = effect->getHostInfo ();
		LoadString((HINSTANCE)hInstance, IDS_HOSTCAP, caption, MAX_PATH);
		LoadString((HINSTANCE)hInstance, IDS_HOSTTXT, temp, MAX_PATH);
#ifdef UNICODE
		MultiByteToWideChar(CP_ACP, 0, hi->VendorString, -1, VendorString, kVstMaxVendorStrLen);
		MultiByteToWideChar(CP_ACP, 0, hi->ProductString, -1, ProductString, kVstMaxProductStrLen);
#else
		strncpy(VendorString, hi->VendorString, kVstMaxVendorStrLen);
		strncpy(ProductString, hi->ProductString, kVstMaxProductStrLen);
#endif
		_stprintf(text, temp, ProductString, hi->VendorVersion/1000.0f, VendorString, hi->MasterVersion/1000.0f);
		if (!_tcscmp(VendorString, _T("Cockos")))
		{
			TCHAR approve[32];
			ZeroMemory(approve, sizeof(approve));
			LoadString((HINSTANCE)hInstance, IDS_REAPPROVE, approve, sizeof(approve)/sizeof(TCHAR));
			_tcsncat(text, approve, sizeof(approve)/sizeof(TCHAR));
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
		TCHAR caption[MAX_PATH];
		TCHAR text[MAX_PATH];
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
	EditorState* state = (EditorState*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
#else
	EditorState* state = (EditorState*)GetWindowLong(hWnd, GWL_USERDATA);
#endif
	OPL3GM* effect = NULL;
	if (state)
	{
		if (state->Effect)
		{
			effect = (OPL3GM*)state->Effect;
		}
	}
	switch (message)
	{
	case WM_INITDIALOG:
		return InitDialog(hWnd);
	case WM_HSCROLL:
	case WM_VSCROLL:
		return Scroll(hWnd, lParam, effect);
	case WM_CTLCOLORSTATIC:
		return UpdateMeterColor(hWnd, wParam, lParam, state);
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
			return RefreshDialog(hWnd, state, effect);
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
				effect->initializeSettings (true);
				RefreshDialog(hWnd, state, effect);
				return TRUE;
			}
			else
			{
				return FALSE;
			}
		case IDC_FREEZE:
			if (effect)
			{
				if (IsDlgButtonChecked(hWnd, IDC_FREEZE))
				{
					effect->setFreezeMeters (true);
				}
				else
				{
					effect->setFreezeMeters (false);
				}
				RefreshDialog(hWnd, state, effect);
				return TRUE;
			}
			else
			{
				return FALSE;
			}
		case IDC_HIDEDISP:
			if (effect)
			{
				if (IsDlgButtonChecked(hWnd, IDC_HIDEDISP))
				{
					effect->setHideParameters (true);
				}
				else
				{
					effect->setHideParameters (false);
				}
				RefreshDialog(hWnd, state, effect);
				return TRUE;
			}
			else
			{
				return FALSE;
			}
		case IDC_MIXER:
			if (effect)
			{
				DialogBoxParam((HINSTANCE)hInstance, MAKEINTRESOURCE(IDD_MIXER), hWnd, (DLGPROC)MixerProc, (LPARAM)effect);
				return TRUE;
			}
			else
			{
				return FALSE;
			}
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
		ev.flags = kVstMidiEventIsRealtime;
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
		KeyboardEvent(info->Effect, 0x90, info->Channel, (12*info->Octave)+note, info->Velocity);
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
		KeyboardEvent(info->Effect, 0x80, info->Channel, (12*info->Octave)+note, info->Velocity);
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
	TCHAR text[MAX_PATH];
	ZeroMemory(text, sizeof(text));
	VstInt32 returnValue = -1;
	switch (wParam)
	{
	case 0x41:	//a,c
		_stprintf(text, _T("C"));
		returnValue = 0;
		break;
	case 0x53:	//s,d
		_stprintf(text, _T("D"));
		returnValue = 2;
		break;
	case 0x44:	//d,e
		_stprintf(text, _T("E"));
		returnValue = 4;
		break;
	case 0x46:	//f,f
		_stprintf(text, _T("F"));
		returnValue = 5;
		break;
	case 0x47:	//g,g
		_stprintf(text, _T("G"));
		returnValue = 7;
		break;
	case 0x48:	//h,a
		_stprintf(text, _T("A"));
		returnValue = 9;
		break;
	case 0x4a:	//j,b
		_stprintf(text, _T("B"));
		returnValue = 11;
		break;
	case 0x4b:	//k,c
		_stprintf(text, _T("C"));
		returnValue = 12;
		break;
	case 0x4c:	//l,d
		_stprintf(text, _T("D"));
		returnValue = 14;
		break;
	case VK_OEM_1:	//e
		_stprintf(text, _T("E"));
		returnValue = 16;
		break;
	case VK_OEM_7:	//f
		_stprintf(text, _T("F"));
		returnValue = 17;
		break;
	case 0x51:	//q,c#
		_stprintf(text, _T("C#"));
		returnValue = 1;
		break;
	case 0x57:	//w,d#
		_stprintf(text, _T("D#"));
		returnValue = 3;
		break;
	case 0x45:	//e,f#
		_stprintf(text, _T("F#"));
		returnValue = 6;
		break;
	case 0x52:	//r,g#
		_stprintf(text, _T("G#"));
		returnValue = 8;
		break;
	case 0x54:	//t,a#
		_stprintf(text, _T("A#"));
		returnValue = 10;
		break;
	case 0x59:	//y,c#
		_stprintf(text, _T("C#"));
		returnValue = 13;
		break;
	case 0x55:	//u,d#
		_stprintf(text, _T("D#"));
		returnValue = 15;
		break;
	case 0x49:	//i,f#
		_stprintf(text, _T("F#"));
		returnValue = 18;
		break;
	case 0x4f:	//o,g#
		_stprintf(text, _T("G#"));
		returnValue = 20;
		break;
	case 0x50:	//p,a#
		_stprintf(text, _T("A#"));
		returnValue = 22;
		break;
	case VK_OEM_4:	//c#
		_stprintf(text, _T("C#"));
		returnValue = 25;
		break;
	case VK_OEM_6:	//d#
		_stprintf(text, _T("D#"));
		returnValue = 27;
		break;
	}
	if (hWnd && returnValue >= 0)
	{
		SetWindowText(hWnd, text);
	}
	return returnValue;
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
	if (!info)
	{
		return TRUE;
	}
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
	TCHAR text[MAX_PATH];
	char ansi[MAX_PATH];
	ZeroMemory(text, sizeof(text));
	ZeroMemory(ansi, sizeof(ansi));
	switch (wParam)
	{
	case 0x5a:	//z
		info->Velocity = 10;
		_stprintf(text, _T("Velocity %d"), info->Velocity);
		SetWindowText(hWnd, text);
		return FALSE;
	case 0x58:	//x
		info->Velocity = 20;
		_stprintf(text, _T("Velocity %d"), info->Velocity);
		SetWindowText(hWnd, text);
		return FALSE;
	case 0x43:	//c
		info->Velocity = 30;
		_stprintf(text, _T("Velocity %d"), info->Velocity);
		SetWindowText(hWnd, text);
		return FALSE;
	case 0x56:	//v
		info->Velocity = 40;
		_stprintf(text, _T("Velocity %d"), info->Velocity);
		SetWindowText(hWnd, text);
		return FALSE;
	case 0x42:	//b
		info->Velocity = 50;
		_stprintf(text, _T("Velocity %d"), info->Velocity);
		SetWindowText(hWnd, text);
		return FALSE;
	case 0x4e:	//n
		info->Velocity = 60;
		_stprintf(text, _T("Velocity %d"), info->Velocity);
		SetWindowText(hWnd, text);
		return FALSE;
	case 0x4d:	//m
		info->Velocity = 70;
		_stprintf(text, _T("Velocity %d"), info->Velocity);
		SetWindowText(hWnd, text);
		return FALSE;
	case VK_OEM_COMMA:
		info->Velocity = 80;
		_stprintf(text, _T("Velocity %d"), info->Velocity);
		SetWindowText(hWnd, text);
		return FALSE;
	case VK_OEM_PERIOD:
		info->Velocity = 90;
		_stprintf(text, _T("Velocity %d"), info->Velocity);
		SetWindowText(hWnd, text);
		return FALSE;
	case VK_OEM_2:
		info->Velocity = 100;
		_stprintf(text, _T("Velocity %d"), info->Velocity);
		SetWindowText(hWnd, text);
		return FALSE;
	case 0x30:	//0
		info->Octave = 0;
		_stprintf(text, _T("Octave %d"), info->Octave);
		SetWindowText(hWnd, text);
		return FALSE;
	case 0x31:	//1
		info->Octave = 1;
		_stprintf(text, _T("Octave %d"), info->Octave);
		SetWindowText(hWnd, text);
		return FALSE;
	case 0x32:	//2
		info->Octave = 2;
		_stprintf(text, _T("Octave %d"), info->Octave);
		SetWindowText(hWnd, text);
		return FALSE;
	case 0x33:	//3
		info->Octave = 3;
		_stprintf(text, _T("Octave %d"), info->Octave);
		SetWindowText(hWnd, text);
		return FALSE;
	case 0x34:	//4
		info->Octave = 4;
		_stprintf(text, _T("Octave %d"), info->Octave);
		SetWindowText(hWnd, text);
		return FALSE;
	case 0x35:	//5
		info->Octave = 5;
		_stprintf(text, _T("Octave %d"), info->Octave);
		SetWindowText(hWnd, text);
		return FALSE;
	case 0x36:	//6
		info->Octave = 6;
		_stprintf(text, _T("Octave %d"), info->Octave);
		SetWindowText(hWnd, text);
		return FALSE;
	case 0x37:	//7
		info->Octave = 7;
		_stprintf(text, _T("Octave %d"), info->Octave);
		SetWindowText(hWnd, text);
		return FALSE;
	case 0x38:	//8
		info->Octave = 8;
		_stprintf(text, _T("Octave %d"), info->Octave);
		SetWindowText(hWnd, text);
		return FALSE;
	case 0x39:	//9
		info->Octave = 9;
		_stprintf(text, _T("Octave %d"), info->Octave);
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
		_stprintf(text, _T("Octave %d"), info->Octave);
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
		_stprintf(text, _T("Octave %d"), info->Octave);
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
		_stprintf(text, _T("Velocity %d"), info->Velocity);
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
		_stprintf(text, _T("Velocity %d"), info->Velocity);
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
			strncpy(ansi, GmDrumSets[0], MAX_PATH);
		}
		else
		{
			strncpy(ansi, GmNames[info->Program], MAX_PATH);
		}
#ifdef UNICODE
		MultiByteToWideChar(CP_ACP, 0, ansi, -1, text, MAX_PATH);
#else
		strncpy(text, ansi, MAX_PATH);
#endif
		SetWindowText(hWnd, text);
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
			strncpy(ansi, GmDrumSets[0], MAX_PATH);
		}
		else
		{
			strncpy(ansi, GmNames[info->Program], MAX_PATH);
		}
#ifdef UNICODE
		MultiByteToWideChar(CP_ACP, 0, ansi, -1, text, MAX_PATH);
#else
		strncpy(text, ansi, MAX_PATH);
#endif
		SetWindowText(hWnd, text);
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
		_stprintf(text, _T("Channel %d"), info->Channel+1);
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
		_stprintf(text, _T("Channel %d"), info->Channel+1);
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
		SetWindowText(hWnd, _T("Synth refreshed"));
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
		SetWindowText(hWnd, _T("Keyboard reset"));
		return FALSE;
	}
	return TRUE;
}

static BOOL KeyUp(WPARAM wParam, KeyboardInfo* info)
{
	if (!info)
	{
		return TRUE;
	}
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
		SetWindowText(hWnd, _T(""));
		return 0;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

Editor::Editor (AudioEffect* effect)
: AEffEditor (effect)
{
	memset(&vstrect, 0, sizeof(vstrect));
	dlg = NULL;
	dirty = false;
	memset(&state, 0, sizeof(state));
	memset(&keyboard, 0, sizeof(keyboard));
	keyboard.Octave = 4;
	keyboard.Velocity = 127;
	keyboard.BendMSB = 64;
	if (effect)
	{
		effect->setEditor (this);
		state.Effect = (AudioEffectX*)effect;
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
		else
		{
			KeyboardClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
		}
		KeyboardClass.lpszClassName = _T(CLASSNAME);
		RegisterClass(&KeyboardClass);
	}
}

Editor::~Editor ()
{
	g_useCount--;
	if (g_useCount == 0)
	{
		UnregisterClass(_T(CLASSNAME), (HINSTANCE)hInstance);
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
		TCHAR caption[MAX_PATH];
		TCHAR text[MAX_PATH];
		char ansi[MAX_PATH];
		ZeroMemory(caption, sizeof(caption));
		ZeroMemory(text, sizeof(text));
		ZeroMemory(ansi, sizeof(ansi));
		if (effect)
		{
			if (((OPL3GM*)effect)->getErrorText (ansi))
			{
				LoadString((HINSTANCE)hInstance, IDS_FAILCAP, caption, MAX_PATH);
#ifdef UNICODE
				MultiByteToWideChar(CP_ACP, 0, ansi, -1, text, MAX_PATH);
#else
				strncpy(text, ansi, MAX_PATH);
#endif
				MessageBox((HWND)dlg, text, caption, MB_ICONERROR);
			}
			((AudioEffectX*)effect)->getEffectName (ansi);
#ifdef UNICODE
			MultiByteToWideChar(CP_ACP, 0, ansi, -1, text, MAX_PATH);
#else
			strncpy(text, ansi, MAX_PATH);
#endif
			SetWindowText((HWND)dlg, text);
#ifdef _WIN64
			SetWindowLongPtr((HWND)dlg, GWLP_USERDATA, (LONG_PTR)&state);
#else
			SetWindowLong((HWND)dlg, GWL_USERDATA, (LONG)&state);
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
		RefreshDialog((HWND)dlg, &state, (OPL3GM*)effect);
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
	if (dirty)
	{
		RefreshDialog((HWND)dlg, &state, (OPL3GM*)effect);
		dirty = false;
	}
	UpdateMeters((HWND)dlg, &state, (OPL3GM*)effect, TRUE);
}

void Editor::refresh ()
{
	dirty = true;
}
