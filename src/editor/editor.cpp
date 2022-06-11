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
#include <windows.h>
#include <commctrl.h>
#include <commdlg.h>
#include "../../res/resource.h"

extern void* hInstance;

void SetParameterValue(AudioEffectX* effect, VstInt32 index, float value)
{
	if (effect)
	{
		effect->beginEdit (index);
		effect->setParameterAutomated (index, value);
		effect->endEdit (index);
	}
}

BOOL InitDialog(HWND hWnd)
{
	if (hWnd)
	{
		HICON hIcon = LoadIcon((HINSTANCE)hInstance, MAKEINTRESOURCE(IDI_ICON1));
		if (hIcon)
		{
			SendMessage(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
		}
		SendDlgItemMessage(hWnd, IDC_VOLUME, TBM_SETRANGE, 0, MAKELONG(0, 100));
		SendDlgItemMessage(hWnd, IDC_VOLUME, TBM_SETPAGESIZE, 0, 10);
		SendDlgItemMessage(hWnd, IDC_TRANSPOSE, TBM_SETRANGE, 0, MAKELONG(0, 25));
		SendDlgItemMessage(hWnd, IDC_TRANSPOSE, TBM_SETPAGESIZE, 0, 2);
		return TRUE;
	}
	return FALSE;
}

BOOL RefreshDialog(HWND hWnd, AudioEffectX* effect)
{
	if (hWnd && effect)
	{
		float ParamValue;
		char text[MAX_PATH];
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
		return TRUE;
	}
	return FALSE;
}

BOOL ProcessScrollParameter(HWND hWnd, LPARAM lParam, AudioEffectX* effect)
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

BOOL LoadInstrumentBank(HWND hWnd, OPL3GM* effect)
{
	if (hWnd && effect)
	{
		OPENFILENAME ofn;
		char filename[MAX_PATH];
		char filter[MAX_PATH];
		char caption[MAX_PATH];
		ZeroMemory(&ofn, sizeof(ofn));
		ZeroMemory(filename, sizeof(filename));
		LoadString((HINSTANCE)hInstance, IDS_FILEFLT, filter, MAX_PATH);
		LoadString((HINSTANCE)hInstance, IDS_FILECAP, caption, MAX_PATH);
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = hWnd;
		ofn.lpstrFilter = filter;
		ofn.nFilterIndex =1;
		ofn.lpstrFile = filename;
		ofn.nMaxFile = sizeof(filename);
		ofn.lpstrTitle = caption;
		ofn.Flags = OFN_ENABLEHOOK | OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
		if (GetOpenFileName(&ofn))
		{
			effect->loadInstruments (ofn.lpstrFile);
			return TRUE;
		}
	}
	return FALSE;
}

BOOL AboutBox(HWND hWnd)
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

BOOL StatisticsBox(HWND hWnd, AudioEffectX* effect)
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

BOOL WINAPI DialogProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
#ifdef _WIN64
	AudioEffectX* effect = (AudioEffectX*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
#else
	AudioEffectX* effect = (AudioEffectX*)GetWindowLong   (hWnd, GWL_USERDATA );
#endif
	switch (message)
	{
	case WM_HSCROLL:
	case WM_VSCROLL:
		return ProcessScrollParameter(hWnd, lParam, effect);
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
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
		case IDC_PROJPAGE:
			ShellExecute(hWnd, NULL, "https://github.com/datajake1999/OPL3GM_VSTi", NULL, NULL, 0);
			return TRUE;
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
		InitDialog((HWND)dlg);
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
