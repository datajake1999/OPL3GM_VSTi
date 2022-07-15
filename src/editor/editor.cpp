/*
OPL3GM Editor
Copyright (C) 2021-2022  Datajake

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
#include "../OPL3GM.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>
#include <commdlg.h>
#include <dlgs.h>
#include <shellapi.h>
#include "../../res/resource.h"

extern void* hInstance;

static bool SetPreset(HWND hWnd, AudioEffectX* effect)
{
	if (hWnd && effect)
	{
		effect->setProgram (SendDlgItemMessage(hWnd, IDC_PRESET, CB_GETCURSEL, 0, 0));
		return TRUE;
	}
	return FALSE;
}

static bool SetPresetName(HWND hWnd, AudioEffectX* effect)
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

static void SetParameterValue(AudioEffectX* effect, VstInt32 index, float value)
{
	if (effect)
	{
		effect->beginEdit (index);
		effect->setParameterAutomated (index, value);
		effect->endEdit (index);
	}
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
		return TRUE;
	}
	return FALSE;
}

static BOOL RefreshDialog(HWND hWnd, AudioEffectX* effect)
{
	if (hWnd && effect)
	{
		float ParamValue;
		char text[MAX_PATH];
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
		SendDlgItemMessage(hWnd, IDC_VOLUME, TBM_SETPOS, TRUE, (long)ParamValue);
		effect->getParameterDisplay (kVolume, text);
		SetDlgItemText(hWnd, IDC_VOLDISP1, text);
		effect->getParameterLabel (kVolume, text);
		SetDlgItemText(hWnd, IDC_VOLDISP2, text);
		ParamValue = effect->getParameter (kTranspose)*25;
		SendDlgItemMessage(hWnd, IDC_TRANSPOSE, TBM_SETPOS, TRUE, (long)ParamValue);
		effect->getParameterDisplay (kTranspose, text);
		SetDlgItemText(hWnd, IDC_TRANDISP1, text);
		effect->getParameterLabel (kTranspose, text);
		SetDlgItemText(hWnd, IDC_TRANDISP2, text);
		ParamValue = effect->getParameter (kVolumeDisplay);
		if (ParamValue >= 0.5)
		{
			SendDlgItemMessage(hWnd, IDC_DISPLAY, BM_SETCHECK, BST_CHECKED, 0);
		}
		else
		{
			SendDlgItemMessage(hWnd, IDC_DISPLAY, BM_SETCHECK, BST_UNCHECKED, 0);
		}
		ParamValue = effect->getParameter (kEmulator);
		if (ParamValue >= 0.5)
		{
			SendDlgItemMessage(hWnd, IDC_NUKED, BM_SETCHECK, BST_CHECKED, 0);
		}
		else
		{
			SendDlgItemMessage(hWnd, IDC_NUKED, BM_SETCHECK, BST_UNCHECKED, 0);
		}
		ParamValue = effect->getParameter (kDCBlock);
		if (ParamValue >= 0.5)
		{
			SendDlgItemMessage(hWnd, IDC_DC, BM_SETCHECK, BST_CHECKED, 0);
		}
		else
		{
			SendDlgItemMessage(hWnd, IDC_DC, BM_SETCHECK, BST_UNCHECKED, 0);
		}
		ParamValue = effect->getParameter (kPushMidi);
		if (ParamValue >= 0.5)
		{
			SendDlgItemMessage(hWnd, IDC_QUEUE, BM_SETCHECK, BST_CHECKED, 0);
		}
		else
		{
			SendDlgItemMessage(hWnd, IDC_QUEUE, BM_SETCHECK, BST_UNCHECKED, 0);
		}
		if (((OPL3GM*)effect)->getBypass ())
		{
			SendDlgItemMessage(hWnd, IDC_BYPASS, BM_SETCHECK, BST_CHECKED, 0);
		}
		else
		{
			SendDlgItemMessage(hWnd, IDC_BYPASS, BM_SETCHECK, BST_UNCHECKED, 0);
		}
		((OPL3GM*)effect)->getBankName (text, MAX_PATH);
		SetDlgItemText(hWnd, IDC_CURBANK, text);
		return TRUE;
	}
	return FALSE;
}

static BOOL ProcessScrollParameter(HWND hWnd, LPARAM lParam, AudioEffectX* effect)
{
	if (hWnd && effect)
	{
		LPARAM VolumeHandle = (LPARAM)GetDlgItem(hWnd, IDC_VOLUME);
		LPARAM TransposeHandle = (LPARAM)GetDlgItem(hWnd, IDC_TRANSPOSE);
		float ParamValue;
		if (lParam == VolumeHandle)
		{
			ParamValue = SendDlgItemMessage(hWnd, IDC_VOLUME, TBM_GETPOS, 0, 0)/100.0f;
			SetParameterValue(effect, kVolume, ParamValue);
			return TRUE;
		}
		else if (lParam == TransposeHandle)
		{
			ParamValue = SendDlgItemMessage(hWnd, IDC_TRANSPOSE, TBM_GETPOS, 0, 0)/25.0f;
			SetParameterValue(effect, kTranspose, ParamValue);
			return TRUE;
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
		LoadString((HINSTANCE)hInstance, IDS_HELPCAP, caption, MAX_PATH);
		LoadString((HINSTANCE)hInstance, IDS_HELPTXT, text, MAX_PATH);
		MessageBox(hWnd, text, caption, MB_ICONINFORMATION);
		return 1;
	}
	return 0;
}

static UINT WINAPI HookProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
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
		switch (((OFNOTIFY *)lParam)->hdr.code)
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
		LoadString((HINSTANCE)hInstance, IDS_FILEFLT, filter, MAX_PATH);
		LoadString((HINSTANCE)hInstance, IDS_FILECAP, caption, MAX_PATH);
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = hWnd;
		ofn.lpstrFilter = filter;
		ofn.nFilterIndex = 1;
		ofn.lpstrFile = filename;
		ofn.nMaxFile = MAX_PATH;
		ofn.lpstrFileTitle = title;
		ofn.nMaxFileTitle = MAX_PATH;
		ofn.lpstrTitle = caption;
		ofn.Flags = OFN_ENABLEHOOK | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_LONGNAMES;
#ifdef OFN_ENABLESIZING
		ofn.Flags |= OFN_ENABLESIZING;
#endif
#ifdef OFN_DONTADDTORECENT
		ofn.Flags |= OFN_DONTADDTORECENT;
#endif
		char synthname[kVstMaxEffectNameLen];
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
		if (RegOpenKeyEx(HKEY_CURRENT_USER, "SOFTWARE\\Datajake\\OPL3GM", 0, KEY_READ, &hKey) == ERROR_SUCCESS)
		{
			DWORD type = REG_DWORD;
			DWORD len = sizeof(DWORD);
			RegQueryValueEx(hKey, "ExplorerStyle", NULL, &type, (LPBYTE)&expstyle, &len);
			RegQueryValueEx(hKey, "HelpButton", NULL, &type, (LPBYTE)&help, &len);
			len = sizeof(directory);
			if (RegQueryValueEx(hKey, "ApogeePatchDir", NULL, NULL, (LPBYTE)directory, &len) == ERROR_SUCCESS && !strcmp(synthname, "Apogee OPL3"))
			{
				ofn.lpstrInitialDir = directory;
			}
			else if (RegQueryValueEx(hKey, "DoomPatchDir", NULL, NULL, (LPBYTE)directory, &len) == ERROR_SUCCESS && !strcmp(synthname, "Doom OPL3"))
			{
				ofn.lpstrInitialDir = directory;
			}
			RegCloseKey(hKey);
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
			ofn.lpfnHook = (LPOFNHOOKPROC)HookProc;
		}
		if (GetOpenFileName(&ofn))
		{
			if (!effect->loadInstruments (ofn.lpstrFile, ofn.lpstrFileTitle))
			{
				char temp[MAX_PATH];
				char text[MAX_PATH];
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
		effect->getEffectName (synthname);
		if (!strcmp(synthname, "Windows 9x OPL3"))
		{
			char caption[MAX_PATH];
			char text[MAX_PATH];
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
		LoadString((HINSTANCE)hInstance, IDS_ABOUTCAP, caption, MAX_PATH);
		LoadString((HINSTANCE)hInstance, IDS_ABOUTTXT, text, MAX_PATH);
		MessageBox(hWnd, text, caption, MB_ICONINFORMATION);
		return TRUE;
	}
	return FALSE;
}

static BOOL StatisticsBox(HWND hWnd, AudioEffectX* effect)
{
	if (hWnd && effect)
	{
		char caption[MAX_PATH];
		char temp[MAX_PATH];
		char text[MAX_PATH];
		LoadString((HINSTANCE)hInstance, IDS_STATSCAP, caption, MAX_PATH);
		LoadString((HINSTANCE)hInstance, IDS_STATSTXT, temp, MAX_PATH);
		sprintf(text, temp, (VstInt32)effect->getSampleRate (), effect->getBlockSize ());
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
		HostInfo *hi = effect->getHostInfo ();
		LoadString((HINSTANCE)hInstance, IDS_HOSTCAP, caption, MAX_PATH);
		LoadString((HINSTANCE)hInstance, IDS_HOSTTXT, temp, MAX_PATH);
		sprintf(text, temp, hi->ProductString, hi->VendorVersion/1000.0f, hi->VendorString, hi->MasterVersion/1000.0f);
		if (!strcmp(hi->VendorString, "Cockos"))
		{
			char approve[32];
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
		LoadString((HINSTANCE)hInstance, IDS_WEBCAP, caption, MAX_PATH);
		LoadString((HINSTANCE)hInstance, IDS_WEBTXT, text, MAX_PATH);
		if (MessageBox(hWnd, text, caption, MB_ICONQUESTION | MB_YESNO) == IDYES)
		{
			ShellExecute(hWnd, NULL, "https://github.com/datajake1999/OPL3GM_VSTi", NULL, NULL, 0);
			return TRUE;
		}
	}
	return FALSE;
}

static BOOL WINAPI DialogProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
#ifdef _WIN64
	AudioEffectX* effect = (AudioEffectX*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
#else
	AudioEffectX* effect = (AudioEffectX*)GetWindowLong   (hWnd, GWL_USERDATA );
#endif
	switch (message)
	{
	case WM_INITDIALOG:
		return InitDialog(hWnd);
	case WM_HSCROLL:
	case WM_VSCROLL:
		return ProcessScrollParameter(hWnd, lParam, effect);
	case WM_DROPFILES:
		return LoadInstrumentBankDragDrop(hWnd, wParam, (OPL3GM*)effect);
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
			if (SendDlgItemMessage(hWnd, IDC_DISPLAY, BM_GETCHECK, 0, 0))
			{
				SetParameterValue(effect, kVolumeDisplay, 1);
			}
			else
			{
				SetParameterValue(effect, kVolumeDisplay, 0);
			}
			return TRUE;
		case IDC_NUKED:
			if (SendDlgItemMessage(hWnd, IDC_NUKED, BM_GETCHECK, 0, 0))
			{
				SetParameterValue(effect, kEmulator, 1);
			}
			else
			{
				SetParameterValue(effect, kEmulator, 0);
			}
			return TRUE;
		case IDC_DC:
			if (SendDlgItemMessage(hWnd, IDC_DC, BM_GETCHECK, 0, 0))
			{
				SetParameterValue(effect, kDCBlock, 1);
			}
			else
			{
				SetParameterValue(effect, kDCBlock, 0);
			}
			return TRUE;
		case IDC_QUEUE:
			if (SendDlgItemMessage(hWnd, IDC_QUEUE, BM_GETCHECK, 0, 0))
			{
				SetParameterValue(effect, kPushMidi, 1);
			}
			else
			{
				SetParameterValue(effect, kPushMidi, 0);
			}
			return TRUE;
		case IDC_BYPASS:
			if (SendDlgItemMessage(hWnd, IDC_BYPASS, BM_GETCHECK, 0, 0))
			{
				effect->setBypass (true);
			}
			else
			{
				effect->setBypass (false);
			}
			return TRUE;
		case IDC_REFRESH:
			return RefreshDialog(hWnd, effect);
		case IDC_LOAD:
			return LoadInstrumentBank(hWnd, (OPL3GM*)effect);
		case IDC_PANIC:
			if (effect)
			{
				effect->suspend ();
				return TRUE;
			}
		case IDC_RESET:
			if (effect)
			{
				effect->resume ();
				return TRUE;
			}
		case IDC_ABOUT:
			return AboutBox(hWnd);
		case IDC_STATS:
			return StatisticsBox(hWnd, effect);
		case IDC_HOSTINFO:
			return HostInfoBox(hWnd, (OPL3GM*)effect);
		case IDC_PROJPAGE:
			return ProjectPage(hWnd);
		}
	}
	return FALSE;
}

Editor::Editor (AudioEffect* effect)
: AEffEditor (effect)
{
	memset(&vstrect, 0, sizeof(vstrect));
	dlg = NULL;
	if (effect)
	{
		effect->setEditor (this);
	}
	InitCommonControls();
}

Editor::~Editor ()
{
}

bool Editor::getRect (ERect** rect)
{
	if (dlg)
	{
		RECT wndrect;
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
	AEffEditor::open (ptr);
	dlg = CreateDialog((HINSTANCE)hInstance, MAKEINTRESOURCE(IDD_DIALOG), (HWND)systemWindow, (DLGPROC)DialogProc);
	if (dlg)
	{
#ifdef _WIN64
		SetWindowLongPtr((HWND)dlg, GWLP_USERDATA, (LONG_PTR)effect);
#else
		SetWindowLong((HWND)dlg, GWL_USERDATA, (LONG)effect);
#endif
		RefreshDialog((HWND)dlg, (AudioEffectX*)effect);
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
		char text[MAX_PATH];
		int numvoices = ((OPL3GM*)effect)->getActiveVoices ();
		sprintf(text, "%d/18", numvoices);
		SetDlgItemText((HWND)dlg, IDC_VOICECOUNT, text);
		float vu = ((OPL3GM*)effect)->getVu ();
		char vustr[kVstMaxParamStrLen*2];
		effect->dB2string (vu, vustr, (kVstMaxParamStrLen*2)-1);
		sprintf(text, "%s dB", vustr);
		SetDlgItemText((HWND)dlg, IDC_VU, text);
		COLORREF color = 0;
		HDC hDc = GetDC(GetDlgItem((HWND)dlg, IDC_VOICECOUNT));
		if (hDc)
		{
			if (numvoices > 15)
			{
				color = RGB(255, 0, 0);
			}
			SetTextColor(hDc, color);
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
		}
	}
}

bool Editor::onKeyDown (VstKeyCode& keyCode)
{
	return false;
}

bool Editor::onKeyUp (VstKeyCode& keyCode)
{
	return false;
}

void Editor::refreshParameters ()
{
	RefreshDialog((HWND)dlg, (AudioEffectX*)effect);
}
