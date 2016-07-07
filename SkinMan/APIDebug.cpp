#include "stdafx.h"
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#include <iostream>
#include <memory>
#include <commctrl.h>
#include <commdlg.h>
#include <shellapi.h>
#include <shlwapi.h>
#include <shlobj.h>
#include <locale.h>
#include <process.h>
#include <malloc.h>

#include <tlhelp32.h>

#define APIDEBUG 0

#if APIDEBUG==1
VOID ReplaceIAT(LPCSTR pszCalleeModName, PROC pfnCurrent, PROC pfnNew, HMODULE hmodCaller) {
	PIMAGE_DOS_HEADER mz = reinterpret_cast<PIMAGE_DOS_HEADER>(hmodCaller);
	if(mz->e_magic != IMAGE_DOS_SIGNATURE) return;
	PIMAGE_NT_HEADERS pe = reinterpret_cast<PIMAGE_NT_HEADERS>(
                           static_cast<UCHAR*>(static_cast<void*>(hmodCaller)) + mz->e_lfanew); 
	if(pe->Signature != IMAGE_NT_SIGNATURE) return;
	PIMAGE_IMPORT_DESCRIPTOR im = reinterpret_cast<PIMAGE_IMPORT_DESCRIPTOR>(
	static_cast<UCHAR*>(static_cast<void*>(hmodCaller)) + 
		pe->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress); 
	if (im == NULL) return;
	for(; im->Name; ++im) { 
		LPCSTR pszModName = reinterpret_cast<LPCSTR>(static_cast<UCHAR*>(static_cast<void*>(hmodCaller)) + im->Name);
		if (!stricmp(pszModName, pszCalleeModName)) break; 
	} 
	if(im->Name == 0) return; 
	PIMAGE_THUNK_DATA thunk = reinterpret_cast<PIMAGE_THUNK_DATA>(static_cast<UCHAR*>(static_cast<void*>(hmodCaller)) + im->FirstThunk);
	for(; thunk->u1.Function; ++thunk) {
		PROC* ppfn = (PROC*) &thunk->u1.Function;
		bool found = *ppfn == pfnCurrent;
		if(found) { 
			DWORD dwOrgProtect;
			::VirtualProtect(ppfn, sizeof(pfnNew), PAGE_READWRITE, &dwOrgProtect);
			memcpy(ppfn, &pfnNew, sizeof(DWORD_PTR)); 
			::VirtualProtect(ppfn, sizeof(pfnNew), dwOrgProtect , &dwOrgProtect);
			break; 
		} 
	} 
}
VOID ReplaceIATEntryInAllMods(LPCSTR pszCalleeModName, PROC pfnCurrent, PROC pfnNew) {
	HANDLE m_Snap = INVALID_HANDLE_VALUE;
	MODULEENTRY32 me = { sizeof(me) };
	m_Snap = ::CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, ::GetCurrentProcessId());
	if(m_Snap != INVALID_HANDLE_VALUE) {
		if(::Module32First(m_Snap, &me)) {
			bool error = false; 
			do {
				ReplaceIAT(pszCalleeModName, pfnCurrent, pfnNew, me.hModule);
			} while (::Module32Next(m_Snap, &me)); 
		}
		::CloseHandle(m_Snap); 
	}
}
class DebHandleList {
public:
	void *handle;
	DebHandleList *next;
	bool fError;
	DebHandleList(void) {
		handle=0;
		next=0;
		fError=false;
	}
	~DebHandleList(void) {
	}
	void Add(void *h,bool fErr=false) {
		DebHandleList *p=this;
		while(p->next)
			p=p->next;
		p->next=new DebHandleList();
		p->next->handle=h;
		fError=fErr;
	}
	bool Del(void *h) {
		DebHandleList *p,*pOld;
		pOld=p=this;
		while(p->next) {
			p=p->next;
			if(p->handle==h&&p->fError==false) {
				pOld->next=p->next;
				delete p;
				return true;
			}
			pOld=p;
		}
		Add(h,true);
		return false;
	}
};
class DebTable {
public:
	DebHandleList *hl;
	int iCount;
	int iCountMax;
	bool fError;
	DebTable(void) {
		iCount=iCountMax=0;
		hl=new DebHandleList();
		fError=false;
	}
	void Add(void *h) {
		hl->Add(h);
	}
	void Del(void *h) {
		if(hl->Del(h)==false)
			fError=true;
	}
};
DebTable *dtImageList;
PROC pfnImageListCreate, pfnImageListDestroy;
DebTable *dtDCCreate;
PROC pfnDeleteDC,pfnCreateCompatibleDC,pfnCreateDC;
DebTable *dtDCGet;
PROC pfnGetDC,pfnReleaseDC;

void AddFunc(char *module,char *func,PROC *ppfn,PROC hook) {
	*ppfn=::GetProcAddress(GetModuleHandleA(module), func);
	ReplaceIATEntryInAllMods(module, *ppfn, hook);
}
HIMAGELIST WINAPI HookImageList_Create(int cx,int cy,UINT flags,int cInitial,int cGrow) {
	HIMAGELIST h;
	++dtImageList->iCount;
	dtImageList->iCountMax=max(dtImageList->iCountMax,dtImageList->iCount);
	h=(*(HIMAGELIST (WINAPI*)(int,int,UINT,int,int))pfnImageListCreate)(cx,cy,flags,cInitial,cGrow);
	dtImageList->Add(h);
	return h;
}
BOOL WINAPI HookImageList_Destroy(HIMAGELIST himl) {
	--dtImageList->iCount;
	dtImageList->Del(himl);
	return (*(BOOL (WINAPI*)(HIMAGELIST))pfnImageListDestroy)(himl);
}
HDC WINAPI HookCreateDC(LPCTSTR lpszDriver,LPCTSTR lpszDevice,LPCTSTR lpszOutput,CONST DEVMODE *lpInitData) {
	HDC h;
	++dtDCCreate->iCount;
	dtDCCreate->iCountMax=max(dtDCCreate->iCountMax,dtDCCreate->iCount);
	h=(*(HDC (WINAPI*)(LPCTSTR,LPCTSTR,LPCTSTR,CONST DEVMODE *))pfnCreateDC)(lpszDriver,lpszDevice,lpszOutput,lpInitData);
	dtDCCreate->Add(h);
	return h;
}
HDC WINAPI HookCreateCompatibleDC(HDC hDC) {
	HDC h;
	++dtDCCreate->iCount;
	dtDCCreate->iCountMax=max(dtDCCreate->iCountMax,dtDCCreate->iCount);
	h=(*(HDC (WINAPI*)(HDC))pfnCreateCompatibleDC)(hDC);
	dtDCCreate->Add(h);
	return h;
}
BOOL WINAPI HookDeleteDC(HDC hDC) {
	--dtDCCreate->iCount;
	dtDCCreate->Del(hDC);
	return (*(BOOL (WINAPI*)(HDC))pfnDeleteDC)(hDC);
}
HDC WINAPI HookGetDC(HWND hWnd) {
	HDC h;
	++dtDCGet->iCount;
	dtDCGet->iCountMax=max(dtDCGet->iCountMax,dtDCGet->iCount);
	h=(*(HDC (WINAPI*)(HWND))pfnGetDC)(hWnd);
	dtDCGet->Add(h);
	return h;
}
int WINAPI HookReleaseDC(HWND hWnd,HDC  hDC) {
	--dtDCGet->iCount;
	dtDCGet->Del(hDC);
	return (*(int (WINAPI*)(HDC))pfnReleaseDC)(hDC);
}

#endif


void _cdecl APIDebugEnd(void) {
#if APIDEBUG==1
	wchar_t str[300];
	wsprintf(str,L"ImageList:%d,%d,%d\nDC(Create):%d,%d,%d\nDC(Get):%d,%d,%d\n"
		,dtImageList->iCount,dtImageList->iCountMax,dtImageList->fError
		,dtDCCreate->iCount,dtDCCreate->iCountMax,dtDCCreate->fError
		,dtDCGet->iCount,dtDCGet->iCountMax,dtDCGet->fError);
	MessageBox(NULL,str,L"Debug",MB_OK);
#endif
}

void APIDebugStart(void) {
#if APIDEBUG==1
	dtDCCreate=new DebTable();
	dtDCGet=new DebTable();
	dtImageList=new DebTable();
	AddFunc("gdi32.dll","DeleteDC",&pfnDeleteDC,(PROC)HookDeleteDC);
	AddFunc("gdi32.dll","CreateDC",&pfnCreateDC,(PROC)HookCreateDC);
	AddFunc("gdi32.dll","CreateCompatibleDC",&pfnCreateCompatibleDC,(PROC)HookCreateCompatibleDC);
	AddFunc("gdi32.dll","GetDC",&pfnGetDC,(PROC)HookGetDC);
	AddFunc("gdi32.dll","ReleaseDC",&pfnReleaseDC,(PROC)HookReleaseDC);
	AddFunc("comctl32.dll","ImageList_Create",&pfnImageListCreate,(PROC)HookImageList_Create);
	AddFunc("comctl32.dll","ImageList_Destroy",&pfnImageListDestroy,(PROC)HookImageList_Destroy);
	atexit(APIDebugEnd);
#endif
}