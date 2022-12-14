#include "stdafx.h"
#include "prgui.h"

using namespace System;
using namespace System::IO;
using namespace System::Windows;
using namespace System::Windows::Controls;
using namespace System::Windows::Interop;
using namespace System::Windows::Media;
using namespace System::Windows::Markup;
using namespace System::Windows::Threading;
using namespace System::Runtime;
using namespace System::Runtime::InteropServices;
using namespace System::Reflection;
using namespace System::Collections::Generic;



HINSTANCE g_hInstance;

#pragma unmanaged
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	g_hInstance = hModule;
    return TRUE;
}
#pragma managed


LRESULT WINAPI WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{

	switch(message)
	{
	case WM_CREATE:
		{
			CREATESTRUCT *cs = (CREATESTRUCT *)lParam;
			GUI *pgui = (GUI *)cs->lpCreateParams;
			::SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pgui);

			System::Windows::Interop::HwndSourceParameters^ sourceParams = gcnew System::Windows::Interop::HwndSourceParameters("sp");
			sourceParams->ParentWindow = System::IntPtr(hwnd);
			sourceParams->WindowStyle = WS_VISIBLE | WS_CHILD;
		    System::Windows::Interop::HwndSource^ source = gcnew System::Windows::Interop::HwndSource(*sourceParams);
			source->RootVisual = pgui->MGUI->Control;

			pgui->MGUI->HS = source;
		}
		break;

	case WM_SIZE:
		{
			GUI *pgui = (GUI *)::GetWindowLongPtr(hwnd, GWLP_USERDATA);
			pgui->MGUI->Control->Width = LOWORD(lParam);
			pgui->MGUI->Control->Height = HIWORD(lParam);
			return TRUE;
		}
		break;

	case WM_DESTROY:
		{
			GUI *pgui = (GUI *)::GetWindowLongPtr(hwnd, GWLP_USERDATA);
		}
		break;

	case WM_SETFOCUS:
		{
			GUI *pgui = (GUI *)::GetWindowLongPtr(hwnd, GWLP_USERDATA);
			pgui->MGUI->Control->Focus();
		}
		break;

	case WM_KILLFOCUS:
		{
			GUI *pgui = (GUI *)::GetWindowLongPtr(hwnd, GWLP_USERDATA);
			pgui->MGUI->Control->KillFocus();
		}
		break;

	case WM_ACTIVATE:
		{
			GUI *pgui = (GUI *)::GetWindowLongPtr(hwnd, GWLP_USERDATA);

			int x = LOWORD(wParam);
			if (x == WA_INACTIVE)
				pgui->MGUI->Control->WindowDeactivated();
			
		}
		break;

	case WM_COMMAND:
		{
			GUI *pgui = (GUI *)::GetWindowLongPtr(hwnd, GWLP_USERDATA);

			int id = LOWORD(wParam);
			switch(id)
			{
			case ID_EDIT_UNDO:
				pgui->MGUI->pEditorPattern->actionStack.Undo(pgui->MGUI->pEditorPattern);
				return 0;
				break;
			case ID_EDIT_REDO:
				pgui->MGUI->pEditorPattern->actionStack.Redo(pgui->MGUI->pEditorPattern);
				return 0;
				break;
			}

		}
		break;

	default:
		return DefWindowProc (hwnd, message, wParam, lParam);
		break;

	}
	
	return 0;
}
     
ref class AssemblyResolver
{
public:
	static System::Reflection::Assembly^ ResolveEventHandler(System::Object^ sender, System::ResolveEventArgs^ args)
	{
		AssemblyName ^aname = gcnew AssemblyName(args->Name);

		char fn[_MAX_PATH];
		::GetModuleFileName((HMODULE)g_hInstance, fn, _MAX_PATH);

		System::String ^path = gcnew System::String(fn);
		path = System::IO::Path::GetDirectoryName(path);
		path += "\\" + aname->Name + ".dll";

		if (!System::IO::File::Exists(path))
			return nullptr;

		System::Reflection::Assembly ^ass = System::Reflection::Assembly::LoadFile(path);

		return ass;
	}

};

void SetResolveEventHandler()
{
	System::AppDomain^ currentDomain = System::AppDomain::CurrentDomain;
	currentDomain->AssemblyResolve += gcnew System::ResolveEventHandler(AssemblyResolver::ResolveEventHandler);
}

void CreateGUI(GUI &gui, HWND parent, NativeData *pnd)
{

	try
	{

		gui.MGUI = gcnew ManagedGUI(pnd);
	}
	catch (Exception ^e)
	{
		System::Windows::MessageBox::Show(e->Message, "Pianoroll");
		return;
	}

	static int count = 0;

	if (!count++)
	{
		WNDCLASS windowClass;
		windowClass.style = 0;
		windowClass.lpfnWndProc = WindowProc;
		windowClass.cbClsExtra = 0;
		windowClass.cbWndExtra = 0;
		windowClass.hInstance = (HINSTANCE)g_hInstance;
		windowClass.hIcon = 0;
		windowClass.hCursor = 0;
		windowClass.hbrBackground = 0;
		windowClass.lpszMenuName = 0;
		windowClass.lpszClassName = "pianorollwndcls";
		RegisterClass (&windowClass);
	}

	try
	{
		gui.Window = CreateWindowEx (0, "pianorollwndcls", "pianorollwnd", WS_CHILD | WS_VISIBLE,0, 0, 1, (int)gui.MGUI->Control->Height, parent, NULL, (HINSTANCE)g_hInstance, &gui);

	}
	catch (System::Exception^ e)
	{
		System::Windows::MessageBox::Show(e->Message, "Pianoroll");
	}

}