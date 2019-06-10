// SkinMan.cpp : Application Main
//
//  Copyright (c) 2007-2016 g200kg
//
#include "stdafx.h"

#include "SkinMan.h"
#define OEMRESOURCE

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#include <iostream>
#include <memory>

#include <commctrl.h>
#include <commdlg.h>
#include <htmlhelp.h>
#include <math.h>
#include <shellapi.h>
#include <shlwapi.h>
#include <shlobj.h>
#include <locale.h>
#include <process.h>
#include <malloc.h>
#include <gdiplus.h>
#include <direct.h>

//#include <tlhelp32.h>

#include "XFont.h"


//#define _CRTDBG_MAP_ALLOC
//#include <crtdbg.h>

#define VERSION 1000
#define VERSIONSUFFIX L""

#define MOVESENSITIVITY 3

#define WM_MOUSEWHEEL 0x20a
#define FLOATMAX (float)(1e30)
#define FLOATMIN (float)(-1e30)


#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "htmlhelp.lib")
#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "shlwapi.lib")

#define MAX_LOADSTRING 100
#define WM_USER_ENDEDIT (WM_APP+1)
#define WM_USER_STARTEDIT (WM_APP+2)
#define WM_USER_CTLCOLOR (WM_APP+3)
#define WM_USER_UPDATEPRIMITIVE (WM_APP+4)
#define PI (3.1415926535897932384626433832795f)



#pragma warning(disable:4996)

using namespace Gdiplus;
using namespace std::tr1;

DWORD _fastcall Blend(DWORD dw1,DWORD dw2,int a);
DWORD _fastcall Blend(DWORD dw1,DWORD dw2);
DWORD _fastcall Bright(DWORD dw,int iA,int iB);
int _fastcall GetLuminance(DWORD dw);

void CopyCoodinate(void);

// Globals
HWND hwndSplash;
HWND hwndMain;
wchar_t strInstanceID[16];
HINSTANCE hinstMain;							// Current instance
TCHAR szTitle[MAX_LOADSTRING];					// Titlebar text
TCHAR szWindowClass[MAX_LOADSTRING];			// Main window class
GdiplusStartupInput gdiSI;
ULONG_PTR           gdiToken;
HCURSOR hcurArrow;
HCURSOR hcurHand;
HCURSOR hcurRect;
HCURSOR hcurLines;
HCURSOR hcurEllipse;
HCURSOR hcurPolygon;
HCURSOR hcurText;
HCURSOR hcurShapeEdit;
HCURSOR hcurShapePlus;
HCURSOR hcurShapeClose;
HCURSOR hcurCurveEdit;
HCURSOR hcurCurvePlus;
HCURSOR hcurCurveClose;
HCURSOR hcurGradation;
HCURSOR hcurTrimming;
HCURSOR hcurMove;
HCURSOR hcurSizeNESW;
HCURSOR	hcurSizeNWSE;
HCURSOR	hcurSizeNS;
HCURSOR	hcurSizeWE;
HCURSOR hcurPipette;
HCURSOR hcurWait;
int iTransparentBackground;
LANGID lang;

typedef enum tool {tArrow=IDC_SELECT,tHand,tEdit,tRect,tEllipse,tPolygon,tText,tCurve,tShape,tLines,tGradation,tTrimming} tool;
typedef enum primitive {prtNone=-1,prtRect,prtEllipse,prtPolygon,prtText,prtShape,prtLines,prtImage,prtGroup} prim;
const wchar_t *FILEFILTEREXPORT=L"All Image(Depends on Ext)\0*.bmp;*.png;*.jpg;*.jpeg;*.gif\0BMP File (*.bmp) [24bpp]\0*.bmp\0BMP File (*.bmp) [32bpp with Alpha]\0*.bmp\0PNG File(*.png) [with Alpha]\0*.png\0PNG File(*.png) [Opaque Background]\0*.png\0JPG File(*.jpg/*.jpeg)\0*.jpg;*.jpeg\0GIF File(*.gif)\0*.gif\0All(*.*)\0*.*\0";

COLORREF colCust[24]={
	RGB(0,0,0),	RGB(0,0,128), RGB(0,128,0), RGB(0,128,128),
	RGB(128,0,0),RGB(128,0,128), RGB(128,128,0),RGB(128,128,128),
	RGB(220,220,220),RGB(0,0,255),RGB(0,255,0),RGB(0,255,255),
	RGB(255,0,0),RGB(255,0,255),RGB(255,255,0),RGB(255,255,255),
	RGB(64,64,64),RGB(64,64,64),RGB(64,64,64),RGB(64,64,64),
	RGB(64,64,64),RGB(64,64,64),RGB(64,64,64),RGB(64,64,64),
};

char *strTemplete="<OBJ>\\t%X,%Y,%W,%H,%NAME,\"%FILE\"\\r\\n</OBJ>";

// Functions
void Open(wchar_t *str,int iMode,bool bAsChild,bool bRec,int iRefresh);
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, wchar_t *strCmdLine, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

void Group(int iRefresh=1);
void UnGroup(int iRefresh=1);
float wstrtofloat(wchar_t *wstr,wchar_t **pwstr);
void GetXY(LPARAM lparam,int *px,int *py);
class Primitive;
class App;
class Lang;
class ColorTool;
class Splash;
class Debug;
App *theApp;
Lang *theLang;
Splash *theSplash;

#ifdef _DEBUG
void DebugMessage1(wchar_t *str) {
	OutputDebugString(str);
}
void DebugMessage2(wchar_t *str,int x) {
	wchar_t s[256];
	wsprintf(s,str,x);
	OutputDebugString(s);
}
void DebugMessage3(wchar_t *str,int x1,int x2) {
	wchar_t s[256];
	wsprintf(s,str,x1,x2);
	OutputDebugString(s);
}
#else
#define DebugMessage1(x)
#define DebugMessage2(x,y)
#define DebugMessage3(x,y,z)
#endif

void TCheck(Matrix *mx) {
#ifdef _DEBUG
	if(mx && mx->GetLastStatus()!=Ok) {
		float m[9];
		int i;
		for(i=0;i<9;++i)
			m[i]=0.f;
		mx->GetElements(m);
		MessageBox(0,L"T",L"T",MB_OK);
	}
#endif
}

class Splash {
public:
	HWND hwnd;
	bool bExit;
	Splash(HINSTANCE hinst) {
		bExit=false;
		hwnd=CreateDialog(hinst,(LPCTSTR)IDD_SPLASH,NULL,(DLGPROC)dlgproc);
		ShowWindow(hwnd,SW_NORMAL);
		UpdateWindow(hwnd);
	}
	~Splash(void) {
		DestroyWindow(hwnd);
	}
	static LRESULT CALLBACK dlgproc(HWND hwnd,UINT uMsg,WPARAM wparam,LPARAM lparam) {
		wchar_t str[80];
		switch(uMsg) {
		case WM_INITDIALOG:
			swprintf(str,L"%d.%03d%s",VERSION/1000,VERSION%1000,VERSIONSUFFIX);
			SetDlgItemText(hwnd,IDC_VER,str);
			break;
		}
		return FALSE;
	}
	void Message(wchar_t *str) {
		SetDlgItemText(hwnd,IDC_MSG1,str);
	}
};
class PrTabItem {
public:
	Primitive *pr;
	int x;
};
class PrivateProfileReader {
	WORD wID;
	DWORD dwLen;
	DWORD dwSection;
	bool bUnicode;
	wchar_t strLine[1024];
	int GetLineA(void) {
		char str[1024];
		char *p;
		p=fgets(str,1024,fp);
		setlocale(LC_CTYPE,"");
		mbstowcs(strLine,str,1024);
		if(p==NULL)
			return 0;
		if(strLine[0]=='[')
			return -1;
		return 1;
	}
	int GetLineW(void) {
		wchar_t *p;
		p=fgetws(strLine,1024,fp);
		if(p==NULL)
			return 0;
		if(strLine[0]=='[')
			return -1;
		return 1;
	}
	int GetLine(void) {
		if(bUnicode)
			return GetLineW();
		else
			return GetLineA();
	}
	bool FindSection(wchar_t *strSection) {
		wchar_t strSec[64];
		swprintf(strSec,L"[%s]",strSection);
		for(;;) {
			if(GetLine()==0)
				break;
			if(wcsstr(strLine,strSec)==strLine) {
				dwSection=ftell(fp);
				return true;
			}
			if(wcsstr(strLine,L"[")==strLine)
				break;
		}
		SeekHead();
		dwSection=0;
		for(;;) {
			if(GetLine()==0)
				return false;
			if(wcsstr(strLine,L"[End]")==strLine) {
				dwSection=ftell(fp);
				return false;
			}
			if(wcsstr(strLine,strSec)==strLine) {
				dwSection=ftell(fp);
				return true;
			}
		}
	}
public:
	FILE *fp;
	PrivateProfileReader(wchar_t *strFile) {
		fp=_wfopen(strFile,L"rb");
		if(fp==NULL)
			return;
		fread(&wID,1,2,fp);
		if(wID==0x4d42) { //'BM'
			fread(&dwLen,1,4,fp);
			bUnicode=true;
		}
		else if(wID==0x5089) { //0x89P
			DWORD dwChunkLen;
			DWORD dwChunkID;
			dwLen=8;
			for(;;) {
				fseek(fp,dwLen,SEEK_SET);
				fread(&dwChunkLen,1,4,fp);
				dwChunkLen=((dwChunkLen&0xff)<<24)|((dwChunkLen&0xff00)<<8)|((dwChunkLen&0xff0000)>>8)|((dwChunkLen&0xff000000)>>24);
				fread(&dwChunkID,1,4,fp);
				if(dwChunkID==0x74584574) {//tEXt
					dwLen+=16;
					break;
				}
				else {
					dwLen+=(dwChunkLen+12);	//len+id+crc
				}
			}
			bUnicode=false;
		}
		else if(wID==0x4d4b) { //'KM'
			fread(&dwLen,1,4,fp);
			bUnicode=true;
		}
		else {
			dwLen=0;
			bUnicode=true;
		}
	}
	~PrivateProfileReader(void) {
		if(fp)
			fclose(fp);
	}
	int Error(void) {
		if(fp==0)
			return 1;
		return 0;
	}
	void SeekHead(void) {
		fseek(fp,dwLen,SEEK_SET);
	}
	bool SetSection(wchar_t *strSection) {
		if(fp)
			return FindSection(strSection);
		return false;
	}
	int ReadInt(wchar_t *strKey,int iDefault) {
		wchar_t str[64];
		if(fp==NULL)
			return iDefault;
		swprintf(str,L"%s=",strKey);
		if(GetLine()>0) {
			if(wcsstr(strLine,str)==strLine) {
				return _wtoi(wcschr(strLine,'=')+1);
			}
		}
		fseek(fp,dwSection,SEEK_SET);
		for(;;) {
			if(GetLine()<=0)
				break;
			if(wcsstr(strLine,str)==strLine) {
				return _wtoi(wcschr(strLine,'=')+1);
			}
		}
		return iDefault;
	}
	float ReadFloat(wchar_t *strKey,float fDefault,bool *find=0) {
		wchar_t str[64];
		if(find)
			*find=false;
		if(fp==0)
			return fDefault;
		swprintf(str,L"%s=",strKey);
		if(GetLine()>0) {
			if(wcsstr(strLine,str)==strLine) {
				if(find)
					*find=true;
				return wstrtofloat(wcschr(strLine,'=')+1,NULL);
			}
		}
		fseek(fp,dwSection,SEEK_SET);
		for(;;) {
			if(GetLine()<=0)
				break;
			if(wcsstr(strLine,str)==strLine) {
				if(find)
					*find=true;
				return wstrtofloat(wcschr(strLine,'=')+1,NULL);
			}
		}
		return fDefault;
	}
	void ReadString(wchar_t *strKey,wchar_t *strDefault,wchar_t *strBuf,int iSize) {
		wchar_t str[64];
		wchar_t *p,*q;
		if(fp==0) {
			wcscpy(strBuf,strDefault);
			return;
		}
		swprintf(str,L"%s=",strKey);
		if(GetLine()>0) {
			if(wcsstr(strLine,str)==strLine) {
				p=wcschr(strLine,'=')+1;
				q=strBuf;
				while(*p>=' ')
					*q++=*p++;
				*q=0;
				return;
			}
		}
		fseek(fp,dwSection,SEEK_SET);
		for(;;) {
			if(GetLine()<=0)
				break;
			if(wcsstr(strLine,str)==strLine) {
				p=wcschr(strLine,'=')+1;
				q=strBuf;
				while(*p>=' ')
					*q++=*p++;
				*q=0;
				return;
			}
		}
		wcscpy(strBuf,strDefault);
	}
	void ReadNext(wchar_t *strDefault,wchar_t *strBuf,int Size) {
		wchar_t *p,*q;
		if(fp==0) {
			wcscpy(strBuf,strDefault);
			return;
		}
		if(GetLine()<=0) {
			wcscpy(strBuf,strDefault);
			return;
		}
		p=wcschr(strLine,'=')+1;
		q=strBuf;
		while(*p>=' ')
			*q++=*p++;
		*q=0;
		return;
	}
	void ExtractThumbnail(wchar_t *strFile) {
		BYTE bDat[1024];
		FindSection(L"End");
		FILE *fpFile;
		int iLen;
		fpFile=_wfopen(strFile,L"wb");
		if(fpFile==NULL)
			return;
		for(;;) {
			iLen=fread(bDat,1,1024,fp);
			if(iLen==0)
				break;
			fwrite(bDat,1,iLen,fpFile);
		}
		fclose(fpFile);
	}
	void ExtractFile(wchar_t *strKeyBase,wchar_t *strFile) {
		int j;
		BYTE bDat[256];
		wchar_t strD[520],*p;
		wchar_t strKey[64];
		int iCount;
		FILE *fp;
		fp=_wfopen(strFile,L"wb");
		if(fp==NULL)
			return;
		iCount=0;
		for(;;) {
			wsprintf(strKey,L"%s%d",strKeyBase,iCount);
			if(iCount==0)
				ReadString(strKey,L"",strD,520);
			else
				ReadNext(L"",strD,520);
			++iCount;
			if(*strD==0)
				break;
			j=0;
			p=strD;
			while((*p&0xc000)==0x8000) {
				bDat[j]=(*p&0x3fc0)>>6;
				bDat[j+1]=((*p&0x3f)<<2)|((*(p+1)&0x3000)>>12);
				bDat[j+2]=((*(p+1)&0x0ff0)>>4);
				bDat[j+3]=((*(p+1)&0x000f)<<4)|((*(p+2)&0x3c00)>>10);
				bDat[j+4]=((*(p+2)&0x03fc)>>2);
				bDat[j+5]=((*(p+2)&0x0003)<<6)|((*(p+3)&0x3f00)>>8);
				bDat[j+6]=*(p+3)&0xff;
				j+=7;
				p+=4;
			}
			while(*p) {
				bDat[j]=*p&0xff;
				++j;
				++p;
			}
			fwrite(bDat,1,j,fp);
			// xx00 0000 0011 1111
			// xx11 2222 2222 3333
			// xx33 3344 4444 4455
			// xx55 5555 6666 6666
		}
		fclose(fp);
	}
};
class StatusBar {
public:
	HWND hwnd;
	StatusBar(HWND hwndParent) {
		int iArray[2];
		hwnd=CreateWindowEx(0,STATUSCLASSNAME,L"",WS_VISIBLE|WS_CHILD|SBARS_SIZEGRIP,0,0,0,0,hwndParent,NULL,hinstMain,0);
		iArray[0]=250,iArray[1]=-1;
		SendMessage(hwnd,SB_SETPARTS,(WPARAM)2,(LPARAM)iArray);
	}
	void Display(wchar_t *wstr) {
		SendMessage(hwnd,SB_SETTEXT,(WPARAM)0,(LPARAM)wstr);
	}
	void Display2(wchar_t *wstr) {
		SendMessage(hwnd,SB_SETTEXT,(WPARAM)1,(LPARAM)wstr);
	}
};
class App {
	bool bEdit;
public:
	HINSTANCE hinst;
	HWND hwnd;
	HWND hwndColor;
	int iIconic;
	int iUndoLevel;
	int iCursorMode;
	int iDispGuide;
	StatusBar *statusbar;
	ColorTool *pal;
	RECT rcWin,rcTree,rcProp,rcColor;
	HICON hiconCopy;
	HBITMAP hbmpCopy;
	HBITMAP hbmpArrow,hbmpHand,hbmpRect,hbmpEllipse,hbmpPolygon,hbmpText,hbmpCurve,hbmpShape,hbmpLines,hbmpGradation,hbmpTrimming,hbmpImage,hbmpNewKnob,hbmpUndo,hbmpRedo;
	HBITMAP hbmpMirrorH,hbmpMirrorV,hbmpShapeEdit;
	HBITMAP hbmpEazel,hbmpZoomIn,hbmpZoom1,hbmpZoomOut,hbmpExportImage;
	HBITMAP hbmpFileNew,hbmpFileOpen,hbmpFileSave;
	HBITMAP hbmpMiter,hbmpBevel,hbmpRound,hbmpFlatCap,hbmpSquareCap,hbmpRoundCap,hbmpTriangleCap;
	HBITMAP hbmpAlignHorzLeft,hbmpAlignHorzCenter,hbmpAlignHorzRight;
	HBITMAP hbmpAlignVertTop,hbmpAlignVertCenter,hbmpAlignVertBottom;
	HBITMAP hbmpDistHorzLeft,hbmpDistHorzCenter,hbmpDistHorzRight;
	HBITMAP hbmpDistVertTop,hbmpDistVertCenter,hbmpDistVertBottom;
	int iAlwaysFront;
	int iDefaultGridX,iDefaultGridY;
	int iDefaultGridEnable,iDefaultGridDisp;
	wchar_t strCurrentProject[MAX_PATH];
	wchar_t strLatestBackup[MAX_PATH];
	wchar_t strModule[MAX_PATH];
	wchar_t strModuleDir[MAX_PATH];
	wchar_t strImageDir[MAX_PATH];
	wchar_t strDocDir[MAX_PATH];
	wchar_t strLibDir[MAX_PATH];
	wchar_t strPalDir[MAX_PATH];
	wchar_t strAppDataDir[MAX_PATH];
	wchar_t strTempDir[MAX_PATH];
	wchar_t strTempTextureDir[MAX_PATH];
	wchar_t strClipboard[MAX_PATH];
	wchar_t strClipboard2[MAX_PATH];
	wchar_t strThumbnail[MAX_PATH];
	wchar_t strDefaultKnob[MAX_PATH];
	wchar_t strIniFile[MAX_PATH];
	wchar_t strLangFile[MAX_PATH];
	wchar_t strTextureDir[MAX_PATH];
	App(HINSTANCE hinstInit,wchar_t *strCmdLine);
	~App(void);
	void SaveSetup(int);
	void LayoutWindow(int x,int y);
	void UpdateTitle(void);
	void UpdateColor(Primitive *pr,int i,DWORD rgb,bool bAdjust,bool bRefresh);
	void Edit(bool b=true);	
	bool IsEdit(void) {return bEdit;}
	void CheckMenu(void);
};


void APIDebugStart(void);
void APIDebugEnd(void);

const int MSG_NOTSAVED=(int)L"File Not Saved. Are you sure?";
const int MSG_REGISTERCOLOR=(int)L"Register color";
const int MSG_ENDSHAPEEDIT=(int)L"End Shape/Curve Edit";
const int MSG_DELPOINT=(int)L"Del Point";
const int MSG_CLOSESHAPE=(int)L"Close Shape/Curve";
const int MSG_MOVETOPARENT=(int)L"MoveTo Parent";
const int MSG_MOVETOBG=(int)L"MoveTo Background";
const int MSG_MOVETOFG=(int)L"MoveTo Foreground";
const int MSG_UNDO=(int)L"Undo (Ctrl-Z)";
const int MSG_REDO=(int)L"Redo (Ctrl-Y)";
const int MSG_ZOOMOUT=(int)L"Zoom Out (-)";
const int MSG_ZOOM1=(int)L"Zoom 1:1 (1)";
const int MSG_ZOOMIN=(int)L"Zoom In (+)";
const int MSG_ALIGNCENTER=(int) L"Align Center (F3)";
const int MSG_GRIDVISIBLE=(int)L"Grid Visible (#)";
const int MSG_GRIDENABLE=(int)L"Grid Enable (Alt for disable)";
const int MSG_DISPLAYTREE=(int)L"Display Tree Window (F5)";
const int MSG_DISPLAYPROP=(int)L"Display Properties Window (F6)";
const int MSG_DISPLAYCOLOR=(int)L"Display Color Window (F7)";
const int MSG_ARROWTOOL=(int)L"Arrow Tool (A): Select/Move Primitives";
const int MSG_HANDTOOL=(int)L"Hand Tool (H/Space): Scroll Screen";
const int MSG_RECTTOOL=(int)L"Rect Tool (R): Add Rectangle";
const int MSG_ELLIPSETOOL=(int)L"Ellipse Tool (E): Add Ellipse";
const int MSG_POLYGONTOOL=(int)L"Polygon Tool (P): Add Polygon";
const int MSG_TEXTTOOL=(int)L"Text Tool (T): Add Text";
const int MSG_CURVETOOL=(int)L"Curve Tool (C): Add Curved Line";
const int MSG_SHAPETOOL=(int)L"Shape Tool (S): Add Shape";
const int MSG_LINESTOOL=(int)L"Lines Tool (L): Add Multi-Lines";
const int MSG_GRADATIONTOOL=(int)L"Gradation Tool (G): Set Gradation/Specular Center";
const int MSG_TRIMMINGTOOL=(int)L"Trimming Tool (M): Trimming canvas size";
const int MSG_INSERTIMAGE=(int)L"Insert Image: Import Images/Knobs from file";
const int MSG_INSERTNEWKNOB=(int)L"Create New Knob by KnobMan";
const int MSG_DIALOGOPENPROJECT=(int)L"Open Project";
const int MSG_DIALOGSAVEPROJECTAS=(int)L"Save Project As";
const int MSG_DIALOGEXPORTIMAGE=(int)L"Export Image";
const int MSG_DIALOGIMPORTFROMLIB=(int)L"Import from Lib";
const int MSG_DIALOGEXPORTTOLIB=(int)L"Export to Lib";
struct tag_menutab {
	wchar_t* str;
	int n[5];
} menutab[]={
	L"File",{0,-1,},
	L"File-New",{0,ID_FILE_NEW,-2,},
	L"File-Open",{0,ID_FILE_OPEN,-2},
	L"File-Save",{0,ID_FILE_SAVE,-2},
	L"File-SaveAs",{0,ID_FILE_SAVEAS,-2},
	L"File-ImportLib",{0,ID_FILE_IMPORT,-2},
	L"File-ExportLib",{0,ID_FILE_EXPORT,-2},
	L"File-ExportImage",{0,ID_FILE_EXPORTIMAGE,-2},
	L"File-Setup",{0,ID_FILE_SETUP,-2},
	L"File-Exit",{0,ID_FILE_EXIT,-2},
	L"Edit",{1,-1,},
	L"Edit-Undo",{1,ID_EDIT_UNDO,-2},
	L"Edit-Redo",{1,ID_EDIT_REDO,-2},
	L"Edit-SelectAll",{1,ID_EDIT_SELECTALL,-2},
	L"Edit-SelectVisible",{1,ID_EDIT_SELECTVISIBLE,-2},
	L"Edit-Primitive",{1,6,-1},
	L"Edit-Primitive-Group",{1,6,ID_PRIMITIVE_GROUP,-2},
	L"Edit-Primitive-UnGroup",{1,6,ID_PRIMITIVE_UNGROUP,-2},
	L"Edit-Primitive-CutPrimitive",{1,6,ID_PRIMITIVE_CUTPRIMITIVE,-2},
	L"Edit-Primitive-CopyPrimitive",{1,6,ID_PRIMITIVE_COPYPRIMITIVE,-2},
	L"Edit-Primitive-PastePrimitive",{1,6,ID_PRIMITIVE_PASTEPRIMITIVE,-2},
	L"Edit-Primitive-DelPrimitive",{1,6,ID_PRIMITIVE_DELETEPRIMITIVE,-2},
	L"Edit-Primitive-PasteColor",{1,6,ID_PRIMITIVE_PASTECOLOR,-2},
	L"Edit-Primitive-PasteEffects",{1,6,ID_PRIMITIVE_PASTEEFFECTS,-2},
	L"Edit-Primitive-CopyCoodinate",{1,6,ID_PRIMITIVE_COPYCOODINATE,-2},
	L"Edit-Primitive-MoveToParent",{1,6,ID_PRIMITIVE_MOVETOPARENT,-2},
	L"Edit-Primitive-MoveToForeground",{1,6,ID_PRIMITIVE_MOVETOFOREGROUND,-2},
	L"Edit-Primitive-MoveToBackground",{1,6,ID_PRIMITIVE_MOVETOBACKGROUND,-2},
	L"Edit-Primitive-MoveToTopMost",{1,6,ID_PRIMITIVE_MOVETOTOPMOST,-2},
	L"Edit-Primitive-Rename",{1,6,ID_PRIMITIVE_RENAME,-2},
	L"Edit-Primitive-MirrorHorz",{1,6,ID_PRIMITIVE_MIRRORHORIZONTAL,-2},
	L"Edit-Primitive-MirrorVert",{1,6,ID_PRIMITIVE_MIRRORVERTICAL,-2},
	L"Edit-Primitive-Transform",{1,6,ID_PRIMITIVE_TRANSFORM,-2},
	L"Edit-Primitive-MakeCircle",{1,6,ID_PRIMITIVE_MAKECIRCLE,-2},
	L"Edit-Primitive-EditShape",{1,6,ID_PRIMITIVE_EDITSHAPE,-2},
	L"Edit-Primitive-EditImage",{1,6,ID_PRIMITIVE_EDITKNOB,-2},
	L"Edit-Primitive-RefreshImage",{1,6,ID_PRIMITIVE_REFRESH,-2},
	L"Edit-Primitive-ExtractImageFile",{1,6,ID_PRIMITIVE_EXTRACTFILE,-2},
	L"Edit-Primitive-Align",{1,6,28,-1},
	L"Edit-Primitive-Align-HorzLeft",{1,6,28,ID_PRIMITIVE_ALIGNHORZL,-2},
	L"Edit-Primitive-Align-HorzCenter",{1,6,28,ID_PRIMITIVE_ALIGNHORZC,-2},
	L"Edit-Primitive-Align-HorzRight",{1,6,28,ID_PRIMITIVE_ALIGNHORZR,-2},
	L"Edit-Primitive-Align-VertTop",{1,6,28,ID_PRIMITIVE_ALIGNVERTT,-2},
	L"Edit-Primitive-Align-VertCenter",{1,6,28,ID_PRIMITIVE_ALIGNVERTC,-2},
	L"Edit-Primitive-Align-VertBottom",{1,6,28,ID_PRIMITIVE_ALIGNVERTB,-2},
	L"Edit-Primitive-Distribution",{1,6,29,-1},
	L"Edit-Primitive-Distribution-HorzLeft",{1,6,29,ID_PRIMITIVE_DISTRIBUTEHORZL,-2},
	L"Edit-Primitive-Distribution-HorzCenter",{1,6,29,ID_PRIMITIVE_DISTRIBUTEHORZC,-2},
	L"Edit-Primitive-Distribution-HorzRight",{1,6,29,ID_PRIMITIVE_DISTRIBUTEHORZR,-2},
	L"Edit-Primitive-Distribution-VertTop",{1,6,29,ID_PRIMITIVE_DISTRIBUTEVERTT,-2},
	L"Edit-Primitive-Distribution-VertCenter",{1,6,29,ID_PRIMITIVE_DISTRIBUTEVERTC,-2},
	L"Edit-Primitive-Distribution-VertBottom",{1,6,29,ID_PRIMITIVE_DISTRIBUTEVERTB,-2},
	L"Display",{2,-1,},
	L"Display-ZoomIn",{2,ID_DISPLAY_ZOOMIN,-2},
	L"Display-Zoom1",{2,ID_DISPLAY_ZOOM1,-2},
	L"Display-ZoomOut",{2,ID_DISPLAY_ZOOMOUT,-2},
	L"Display-Tree",{2,ID_DISPLAY_TREE,-2},
	L"Display-Properties",{2,ID_DISPLAY_PROPERTIES,-2},
	L"Display-Color",{2,ID_DISPLAY_COLOR,-2},
	L"Canvas",{3,-1,},
	L"Canvas-ImageProp",{3,ID_CANVAS_SIZE,-2},
	L"Canvas-Trimming",{3,ID_CANVAS_TRIMMING,-2},
	L"Canvas-TrimmingVisible",{3,ID_CANVAS_TRIMMINGVISIBLE,-2},
	L"Canvas-Grid",{3,ID_CANVAS_GRID,-2},
	L"Canvas-Background",{3,ID_CANVAS_BACKGROUND,-2},
	L"Canvas-Workspace",{3,ID_CANVAS_WORKSPACE,-2},
	L"Tool",{4,-1,},
	L"Tool-Arrow",{4,ID_TOOL_ARROW,-2},
	L"Tool-Hand",{4,ID_TOOL_HAND,-2},
	L"Tool-Rect",{4,ID_TOOL_RECT,-2},
	L"Tool-Ellipse",{4,ID_TOOL_ELLIPSE,-2},
	L"Tool-Polygon",{4,ID_TOOL_POLYGON,-2},
	L"Tool-Text",{4,ID_TOOL_TEXT,-2},
	L"Tool-Curve",{4,ID_TOOL_CURVE,-2},
	L"Tool-Shape",{4,ID_TOOL_SHAPE,-2},
	L"Tool-Lines",{4,ID_TOOL_LINES,-2},
	L"Tool-Gradation",{4,ID_TOOL_GRADATION,-2},
	L"Tool-Trimming",{4,ID_TOOL_TRIMMING,-2},
	L"Tool-InsertImage",{4,ID_TOOL_INSERTIMAGE,-2},
	L"Tool-InsertNewKnob",{4,ID_TOOL_INSERTNEWKNOB,-2},
	L"Help",{5,-1,},
	L"Help-About",{5,ID_HELP_ABOUT,-2},
	0,0,0
};
struct tag_controltab {
	wchar_t *strKey;
	wchar_t *strDisp;
	int id;
} controltab[]={
	L"Pos",0,IDC_PROP_POS,
	L"Size",0,IDC_PROP_SIZE,
	L"Operation",0,IDC_PROP_OPERATION,
	L"Slant",0,IDC_PROP_SLANT,
	L"Angle",0,IDC_PROP_ANGLE,
	L"Corner",0,IDC_PROP_CORNER,
	L"RoundSeparate",0,IDC_ROUNDSEPARATE,
	L"RoundXYSeparate",0,IDC_ROUNDXYSEPARATE,
	L"Color",0,IDC_PROP_COLOR,
	L"Alpha",0,IDC_PROP_ALPHA,
	L"Alpha1",0,IDC_PROP_ALPHA1,
	L"Alpha2",0,IDC_PROP_ALPHA2,
	L"Color-Fill1",0,IDC_PROP_FILL1,
	L"Color-Fill2",0,IDC_PROP_FILL2,
	L"Color-Border",0,IDC_PROP_BORDER,
	L"Gradation",0,IDC_PROP_GRADATION,
	L"Smoother",0,IDC_PROP_SMOOTHER,
	L"Fill",0,IDC_FILL,
	L"Close",0,IDC_CLOSE,
	L"Border",0,IDC_BORDER,
	L"BorderAsFill",0,IDC_BORDERASFILL,
	L"Pie",0,IDC_PIE,
	L"MasterAlpha",0,IDC_PROP_MASTERALPHA,
	L"BorderWidth",0,IDC_PROP_BORDERWIDTH,
	L"BorderJoin",0,IDC_PROP_BORDERJOIN,
	L"Diffuse",0,IDC_PROP_DIFFUSE,
	L"Antialias",0,IDC_ANTIALIAS,
	L"Spec",0,IDC_PROP_SPEC,
	L"Spec-Light1",0,IDC_PROP_SPEC_LIGHT1,
	L"Spec-Light2",0,IDC_PROP_SPEC_LIGHT2,
	L"Spec-Type",0,IDC_PROP_SPEC_TYPE,
	L"Spec-Dir",0,IDC_PROP_SPEC_DIR,
	L"Spec-Spec",0,IDC_PROP_SPEC_SPEC,
	L"Spec-Hilight",0,IDC_PROP_SPEC_HILIGHT,
	L"Spec-HilightW",0,IDC_PROP_SPEC_HILIGHTW,
	L"Spec-Ambient",0,IDC_PROP_SPEC_AMBIENT,
	L"Emboss",0,IDC_PROP_EMBOSS,
	L"Emboss-Dir",0,IDC_PROP_EMBOSS_DIR,
	L"Emboss-Width",0,IDC_PROP_EMBOSS_WIDTH,
	L"Emboss-Depth",0,IDC_PROP_EMBOSS_DEPTH,
	L"Emboss-DepthB",0,IDC_PROP_EMBOSS_DEPTHB,
	L"Emboss-DepthF",0,IDC_PROP_EMBOSS_DEPTHF,
	L"Tex",0,IDC_PROP_TEX,
	L"Tex-Type",0,IDC_PROP_TEX_TYPE,
	L"Tex-Intensity",0,IDC_PROP_TEX_INTENSITY,
	L"Tex-Zoom",0,IDC_PROP_TEX_ZOOM,
	L"Shadow",0,IDC_PROP_SHADOW,
	L"Shadow-Drop",0,IDC_PROP_SHADOW_DROP,
	L"Shadow-Inside",0,IDC_PROP_SHADOW_INSIDE,
	L"Shadow-Color",0,IDC_PROP_SHADOW_COLOR,
	L"Shadow-Dir",0,IDC_PROP_SHADOW_DIR,
	L"Shadow-Offset",0,IDC_PROP_SHADOW_OFFSET,
	L"Shadow-Density",0,IDC_PROP_SHADOW_DENSITY,
	L"Shadow-Diffuse",0,IDC_PROP_SHADOW_DIFFUSE,
	L"Start",0,IDC_PROP_START,
	L"Stop",0,IDC_PROP_STOP,
	L"Text",0,IDC_PROP_TEXT,
	L"Font",0,IDC_PROP_FONT,
	L"FontSize",0,IDC_PROP_FONTSIZE,
	L"FontAspect",0,IDC_PROP_FONTASPECT,
	L"FontSpacing",0,IDC_PROP_FONTSPACING,
	L"Bold",0,IDC_BOLD,
	L"Italic",0,IDC_ITALIC,
	L"Fix",0,IDC_FIX,
	L"AutoSize",0,IDC_AUTOSIZE,
	L"SmallFontOpt",0,IDC_SMALLFONTOPT,
	L"Vertex",0,IDC_PROP_VERTEX,
	L"StarDepth",0,IDC_PROP_STARDEPTH,
	L"Curve",0,IDC_PROP_CURVE,
	L"Twist",0,IDC_PROP_TWIST,
	L"Close",0,IDC_PROP_CLOSE,
	L"LineCap",0,IDC_PROP_LINECAP,
	L"LineType",0,IDC_PROP_LINETYPE,
	0,0,
};
struct tag_msgtab {
	wchar_t *strKey;
	wchar_t *strDisp;
	int id;
} msgtab[]={
	L"NotSaved",0,MSG_NOTSAVED,
	L"RegisterColor",0,MSG_REGISTERCOLOR,
	L"EndShapeEdit",0,MSG_ENDSHAPEEDIT,
	L"DelPoint",0,MSG_DELPOINT,
	L"CloseShape",0,MSG_CLOSESHAPE,
	L"ToolTipMoveToParent",0,MSG_MOVETOPARENT,
	L"ToolTipMoveToBG",0,MSG_MOVETOBG,
	L"ToolTipMoveToFG",0,MSG_MOVETOFG,
	L"ToolTipUndo",0,MSG_UNDO,
	L"ToolTipRedo",0,MSG_REDO,
	L"ToolTipZoomOut",0,MSG_ZOOMOUT,
	L"ToolTipZoom1",0,MSG_ZOOM1,
	L"ToolTipZoomIn",0,MSG_ZOOMIN,
	L"ToolTipGridVisible",0,MSG_GRIDVISIBLE,
	L"ToolTipGridEnable",0,MSG_GRIDENABLE,
	L"ToolTipDisplayTree",0,MSG_DISPLAYTREE,
	L"ToolTipAlignCenter",0,MSG_ALIGNCENTER,
	L"ToolTipDisplayProp",0,MSG_DISPLAYPROP,
	L"ToolTipDisplayColor",0,MSG_DISPLAYCOLOR,
	L"ToolTipArrowTool",0,MSG_ARROWTOOL,
	L"ToolTipHandTool",0,MSG_HANDTOOL,
	L"ToolTipRectTool",0,MSG_RECTTOOL,
	L"ToolTipEllipseTool",0,MSG_ELLIPSETOOL,
	L"ToolTipPolygonTool",0,MSG_POLYGONTOOL,
	L"ToolTipTextTool",0,MSG_TEXTTOOL,
	L"ToolTipCurveTool",0,MSG_CURVETOOL,
	L"ToolTipShapeTool",0,MSG_SHAPETOOL,
	L"ToolTipLinesTool",0,MSG_LINESTOOL,
	L"ToolTipGradationTool",0,MSG_GRADATIONTOOL,
	L"ToolTipTrimmingTool",0,MSG_TRIMMINGTOOL,
	L"ToolTipInsertImage",0,MSG_INSERTIMAGE,
	L"ToolTipInsertNewKnob",0,MSG_INSERTNEWKNOB,
	L"DialogOpenProject",0,MSG_DIALOGOPENPROJECT,
	L"DialogSaveProjectAs",0,MSG_DIALOGSAVEPROJECTAS,
	L"DialogExportImage",0,MSG_DIALOGEXPORTIMAGE,
	L"DialogImportFromLib",0,MSG_DIALOGIMPORTFROMLIB,
	L"DialogExportToLib",0,MSG_DIALOGEXPORTTOLIB,
	0,0,0
};
class Lang {
public:
	Lang(App *app,wchar_t*strLangFile) {
		int i;
		PrivateProfileReader ppr(strLangFile);
		wchar_t strName[256];
		if(ppr.fp==0)
			return;
		ppr.SetSection(L"Properties");
		for(i=0;controltab[i].strKey;++i) {
			ppr.ReadString(controltab[i].strKey,L"",strName,256);
			if(strName[0]) {
				controltab[i].strDisp=new wchar_t[wcslen(strName)+1];
				wcscpy(controltab[i].strDisp,strName);
			}
		}
		ppr.SetSection(L"Messages");
		for(i=0;msgtab[i].strKey;++i) {
			ppr.ReadString(msgtab[i].strKey,L"",strName,256);
			if(strName[0]) {
				msgtab[i].strDisp=new wchar_t[wcslen(strName)+1];
				wcscpy(msgtab[i].strDisp,strName);
			}
		}
	}
	void SetupMenu(App *app,wchar_t *strLangFile) {
		HMENU hmenu=GetMenu(app->hwnd);
		HMENU hmenuSub;
		MENUITEMINFO mii;
		int i,j;
		wchar_t *p,*q;
		PrivateProfileReader ppr(strLangFile);
		wchar_t strName[256];
		wchar_t strTemp[256];
		if(ppr.fp==0)
			return;
		ppr.SetSection(L"Menu");
		for(i=0;menutab[i].str;++i) {
			ppr.ReadString(menutab[i].str,L"",strName,256);
			for(p=strName,q=strTemp;*p;) {
				if(*p=='\\') {
					++p;
					if(*p=='t') {
						++p;
						*q++='\t';
					}
				}
				else
					*q++=*p++;
			}
			*q=0;
			ZeroMemory(&mii,sizeof(MENUITEMINFO));
			mii.cbSize = sizeof(MENUITEMINFO);
			mii.fMask = MIIM_TYPE;
			mii.fType = MFT_STRING;
			mii.fState = MFS_ENABLED;
			mii.dwTypeData = strTemp;
			hmenuSub=hmenu;
			for(j=0;menutab[i].n[j+1]>=0;++j) {
				hmenuSub=GetSubMenu(hmenuSub,menutab[i].n[j]);
			}
			if(strTemp[0]) {
				if(menutab[i].n[j+1]==-1)
					SetMenuItemInfo(hmenuSub,menutab[i].n[j],1,&mii);
				else
					SetMenuItemInfo(hmenuSub,menutab[i].n[j],0,&mii);
			}
		}
		DrawMenuBar(app->hwnd);
	}
	wchar_t *GetID(int id) {
		int i;
		for(i=0;msgtab[i].strKey;++i) {
			if(msgtab[i].id==id) {
				if(msgtab[i].strDisp)
					return (wchar_t*)msgtab[i].strDisp;
				else
					return (wchar_t*)id;
			}
		}
		return (wchar_t*)id;
	}
};

void CBEXSetImageList(HWND hwnd,int id,HIMAGELIST iml) {
	SendDlgItemMessage(hwnd, id, CBEM_SETIMAGELIST, 0, (LPARAM)iml);
}
void CBEXInsert(HWND hwnd,int id,int iNum,wchar_t *str) {
	COMBOBOXEXITEM cbei;
	cbei.mask=CBEIF_TEXT;
	cbei.iItem=iNum;
	cbei.pszText=str;
	SendDlgItemMessage(hwnd,id,CBEM_INSERTITEM,0,(LPARAM)&cbei);
}
void CBEXInsert(HWND hwnd,int id,int iNum,int iImg,wchar_t *str) {
	COMBOBOXEXITEM cbei;
	cbei.mask=CBEIF_TEXT|CBEIF_IMAGE|CBEIF_SELECTEDIMAGE;
	cbei.iItem=iNum;
	cbei.iImage=iImg;
	cbei.iSelectedImage=iImg;
	cbei.pszText=str;
	SendDlgItemMessage(hwnd,id,CBEM_INSERTITEM,0,(LPARAM)&cbei);
}
void CBEXGetText(HWND hwnd,int id,int n,wchar_t *str,int iMax) {
	COMBOBOXEXITEM cbei;
	memset(&cbei,0,sizeof COMBOBOXEXITEM);
	cbei.mask=CBEIF_TEXT;
	cbei.pszText=str;
	cbei.cchTextMax=iMax;
	cbei.iItem=n;
	SendDlgItemMessage(hwnd,id,CBEM_GETITEM,0,(LPARAM)&cbei);
	if(cbei.pszText!=str)
		wcscpy(str,cbei.pszText);
}
void CBEXAdd(HWND hwnd,int id,wchar_t *str) {
	CBEXInsert(hwnd,id,-1,str);
}
void CBEXAdd(HWND hwnd,int id,int iImg,wchar_t *str) {
	CBEXInsert(hwnd,id,-1,iImg,str);
}
void CBEXDelete(HWND hwnd,int id,int iNum) {
	SendDlgItemMessage(hwnd,id,CBEM_DELETEITEM,(WPARAM)iNum,0);
}
class FontSelector {
	HBITMAP hbmp,hbmpOld;
	HDC hdcMem;
	FontFamily *fam;
	int iMax;
public:
	HIMAGELIST himl;
	FontSelector(void) {
		HDC hdcScr;
		RECT rc;
		int i;
		InstalledFontCollection *ifc=new InstalledFontCollection();
		iMax=ifc->GetFamilyCount();
		fam=new FontFamily[iMax];
		ifc->GetFamilies(iMax,fam,&iMax);
		hdcScr=GetDC(0);
		hdcMem=CreateCompatibleDC(hdcScr);
		hbmp=CreateCompatibleBitmap(hdcScr,40*iMax,24);
		hbmpOld=(HBITMAP)SelectObject(hdcMem,hbmp);
		himl=ImageList_Create(40,24,ILC_COLOR24,64,64);
		rc.left=rc.top=0;
		rc.right=40*iMax;
		rc.bottom=24;
		FillRect(hdcMem,&rc,(HBRUSH)GetStockObject(WHITE_BRUSH));
		Graphics *g;
		RectF rcf;
		g=new Graphics(hdcMem);
		g->SetSmoothingMode(SmoothingModeHighQuality);
		g->SetPixelOffsetMode(PixelOffsetModeHighQuality);
		for(i=0;i<iMax;++i) {
			Font font(&fam[i],22,FontStyleRegular,UnitPixel);
			SolidBrush brush(Color(255, 0, 0));
			StringFormat format;
			rcf.X=(float)(i*40);
			rcf.Y=0;
			rcf.Width=40;
			rcf.Height=24;
			g->DrawString(L"Abc",-1,&font,rcf,&format,&brush);
		}
		delete g;
		SelectObject(hdcMem,hbmpOld);
		ImageList_Add(himl,hbmp,0);
		delete ifc;
		ReleaseDC(0,hdcScr);
	}
	~FontSelector(void) {
		DeleteObject((HGDIOBJ)hbmp);
		DeleteDC(hdcMem);
		delete[] fam;
	}
	void Set(HWND hdlg,int id) {
		int i;
		if(GetDlgItem(hdlg,id)==NULL)
			return;
		CBEXSetImageList(hdlg,id,himl);
		for(i=0;i<iMax;++i) {
			wchar_t wstr[256];
			fam[i].GetFamilyName(wstr);
			CBEXAdd(hdlg,id,i,wstr);
		}
	}
};
///////////////////////////////////////////////////////////////
// Utils
///////////////////////////////////////////////////////////////
float FMax(float f1,float f2,float f3,float f4) {
	f1=max(f1,f2);
	f1=max(f1,f3);
	f1=max(f1,f4);
	return f1;
}
float FMin(float f1,float f2,float f3,float f4) {
	f1=min(f1,f2);
	f1=min(f1,f3);
	f1=min(f1,f4);
	return f1;
}
void CheckButton(HWND hwnd,int id,bool state) {
	CheckDlgButton(hwnd,id,state?BST_CHECKED:BST_UNCHECKED);
}
void CheckButton(HWND hwnd,int id,int state) {
	CheckDlgButton(hwnd,id,state?BST_CHECKED:BST_UNCHECKED);
}
void PathAddRect(GraphicsPath *path,RectF rcf) {
	if(rcf.Width<0)
		rcf.X+=rcf.Width,rcf.Width=-rcf.Width;
	if(rcf.Height<0)
		rcf.Y+=rcf.Height,rcf.Height=-rcf.Height;
	if(rcf.Width<=.00001f)
		rcf.Width=.00001f;
	if(rcf.Height<=.00001f)
		rcf.Height=.00001f;
	path->AddRectangle(rcf);
}
class CheckUniqStr {
	wchar_t *str;
	CheckUniqStr *next;
public:
	CheckUniqStr(void) {
		str=0;
		next=0;
	}
	~CheckUniqStr(void) {
		if(next)
			delete next;
		if(str)
			delete[] str;
	}
	bool Check(wchar_t *s) {
		CheckUniqStr *p;
		for(p=this;p->next;p=p->next) {
			if(p->str && wcscmp(p->str,s)==0)
				return false;
		}
		p->next=new CheckUniqStr();
		p->str=new wchar_t[wcslen(s)+1];
		wcscpy(p->str,s);
		return true;
	}
};
void GetXY(LPARAM lparam,int *px,int *py) {
	int x,y;
	x=LOWORD(lparam);
	y=HIWORD(lparam);
	if(x&0x8000)
		x|=0xffff0000;
	if(y&0x8000)
		y|=0xffff0000;
	*px=x,*py=y;
}

void Sort(int *x1,int *x2) {
	int t;
	if(*x1>=*x2)
		t=*x1,*x1=*x2,*x2=t;
}
void PTSwap(PointF *ptf1,PointF *ptf2) {
	PointF ptf;
	ptf=*ptf1;
	*ptf1=*ptf2;
	*ptf2=ptf;
}
void FSwap(REAL *x,REAL *y) {
	REAL t;
	t=*x,*x=*y,*y=t;
}
void ISwap(int *x,int *y) {
	int t;
	t=*x,*x=*y,*y=t;
}
void FSort(REAL *x,REAL *y) {
	REAL t;
	if(*x>*y)
		t=*x,*x=*y,*y=t;
}
DWORD DwToRgb(DWORD dw) {
	return ((dw&0xff)<<16)|(dw&0xff00)|((dw&0xff0000)>>16);
}
void Chop(wchar_t *str) {
	if(str&&*str)
		*(str+wcslen(str)-1)=0;
}
bool IsEmbed(wchar_t *str) {
	if(*str=='(' && *(str+wcslen(str)-1)==')')
		return true;
	return false;
}
wchar_t *EmbedNameToDispName(wchar_t *str) {
	static wchar_t strDisp[MAX_PATH*2];
	if(IsEmbed(str)) {
		strDisp[0]='(';
		wcscpy(strDisp+1,PathFindFileName(str+1));
	}
	else
		wcscpy(strDisp,PathFindFileName(str));
	return strDisp;
}
wchar_t *EmbedNameToTempName(wchar_t *str) {
	static wchar_t strTemp[MAX_PATH*2];
	wchar_t strTemp2[MAX_PATH];
	wchar_t *p;
	if(!IsEmbed(str))
		return str;
	wcscpy(strTemp,theApp->strTempDir);
	wcscat(strTemp,L"\\");
	wcscat(strTemp,strInstanceID);
	wcscpy(strTemp2,str);
	for(p=strTemp2;*p;++p) {
		if(*p=='\\'||*p==':')
			*p='_';
	}
	wcscat(strTemp,strTemp2+1);
	Chop(strTemp);
	return strTemp;
}
wchar_t *EmbedNameToTempTextureName(wchar_t *str) {
	static wchar_t strTemp[MAX_PATH*2];
	wchar_t strTemp2[MAX_PATH];
	wchar_t *p;
	if(!IsEmbed(str))
		return str;
	wcscpy(strTemp,theApp->strTempTextureDir);
	wcscat(strTemp,L"\\");
	wcscat(strTemp,strInstanceID);
	wcscpy(strTemp2,str);
	for(p=strTemp2;*p;++p) {
		if(*p=='\\'||*p==':')
			*p='_';
	}
	wcscat(strTemp,strTemp2+1);
	Chop(strTemp);
	return strTemp;
}
wchar_t *FileNameToEmbedName(wchar_t *str) {
	static wchar_t strEmbed[MAX_PATH*2];
	wsprintf(strEmbed,L"(%s)",str);
	return strEmbed;
}
wchar_t *TextureNameToFileName(wchar_t *str) {
	static wchar_t strFile[MAX_PATH*2];
	wchar_t *ext;
	if(IsEmbed(str)) {
		wcscpy(strFile,EmbedNameToTempTextureName(str));
		ext=PathFindExtension(strFile);
		if(ext==0||*ext==0)
			wcscat(strFile,L".bmp");
	}
	else {
		wcscpy(strFile,theApp->strTextureDir);
		wcscat(strFile,L"\\");
		wcscat(strFile,str);
		ext=PathFindExtension(strFile);
		if(ext==0||*ext==0)
			wcscat(strFile,L".bmp");
	}
	return strFile;
}
void SetDlgItemFloat2(HWND hwnd,int iID,float fVal,bool bSign) {
	wchar_t str[64];
	swprintf(str,L"%.2f",fVal);
	SetDlgItemText(hwnd,iID,str);
}
void SetDlgItemFloat3(HWND hwnd,int iID,float fVal,bool bSign) {
	wchar_t str[64];
	swprintf(str,L"%.3f",fVal);
	SetDlgItemText(hwnd,iID,str);
}
float GetDlgItemFloat(HWND hwnd,int iID,BOOL *lpTrans,BOOL bSign) {
	wchar_t str[64];
	float fVal;
	fVal=0.f;
	GetDlgItemText(hwnd,iID,str,64);
	swscanf(str,L"%f",&fVal);
	return fVal;
}
PointF GetCenter(PointF pt1,PointF pt2) {
	return PointF((pt1.X+pt2.X)*.5f,(pt1.Y+pt2.Y)*.5f);
}
PointF GetCenter(RectF rcf) {
	return PointF(rcf.X+rcf.Width*.5f,rcf.Y+rcf.Height*.5f);
}

#define  HLSMAX   240 /* H,L, and S vary over 0-HLSMAX */ 
#define  RGBMAX   255   /* R,G, and B vary over 0-RGBMAX */ 
#define UNDEFINED (HLSMAX*2/3)
void  RGBtoHLS(DWORD lRGBColor,int *phue,int *plum,int *psat) {
	WORD R,G,B;          /* input RGB values */ 
	short int H,L,S;
	BYTE cMax,cMin;      /* max and min RGB values */ 
	WORD  Rdelta,Gdelta,Bdelta; /* intermediate value: % of spread from max */ 
	R = (lRGBColor>>16)&0xff;
	G = (lRGBColor>>8)&0xff;
	B = lRGBColor&0xff;
	cMax = (BYTE)max( max(R,G), B);
	cMin = (BYTE)min( min(R,G), B);
	L = ( ((cMax+cMin)*HLSMAX) + RGBMAX )/(2*RGBMAX);
	if (cMax == cMin) {           /* r=g=b --> achromatic case */ 
		S = 0;                     /* saturation */ 
		H = UNDEFINED;             /* hue */ 
	}
	else {                        /* chromatic case */ 
		if (L <= (HLSMAX/2))
			S = ( ((cMax-cMin)*HLSMAX) + ((cMax+cMin)/2) ) / (cMax+cMin);
		else
			S = ( ((cMax-cMin)*HLSMAX) + ((2*RGBMAX-cMax-cMin)/2) ) / (2*RGBMAX-cMax-cMin);
		Rdelta = ( ((cMax-R)*(HLSMAX/6)) + ((cMax-cMin)/2) ) / (cMax-cMin);
		Gdelta = ( ((cMax-G)*(HLSMAX/6)) + ((cMax-cMin)/2) ) / (cMax-cMin);
		Bdelta = ( ((cMax-B)*(HLSMAX/6)) + ((cMax-cMin)/2) ) / (cMax-cMin);
		if (R == cMax)
			H = Bdelta - Gdelta;
		else if (G == cMax)
			H = (HLSMAX/3) + Rdelta - Bdelta;
		else /* B == cMax */ 
			H = ((2*HLSMAX)/3) + Gdelta - Rdelta;
		if (H < 0)
			H += HLSMAX;
		if (H > HLSMAX)
			H -= HLSMAX;
	}
	*phue=H;
	*plum=L;
	*psat=S;
}
WORD HueToRGB(WORD n1,WORD n2,WORD hue) {
	/* range check: note values passed add/subtract thirds of range */ 
	if (hue < 0)
		hue += HLSMAX;
	if (hue > HLSMAX)
		hue -= HLSMAX;
      /* return r,g, or b value from this tridrant */ 
	if (hue < (HLSMAX/6))
		return ( n1 + (((n2-n1)*hue+(HLSMAX/12))/(HLSMAX/6)) );
	if (hue < (HLSMAX/2))
		return ( n2 );
	if (hue < ((HLSMAX*2)/3))
		return ( n1 +  (((n2-n1)*(((HLSMAX*2)/3)-hue)+(HLSMAX/12))/(HLSMAX/6)));
	else
		return ( n1 );
}
DWORD HLStoRGB(WORD hue,WORD lum,WORD sat) {
	WORD R,G,B;                /* RGB component values */ 
	WORD  Magic1,Magic2;       /* calculated magic numbers (really!) */ 
	if (sat == 0) {            /* achromatic case */ 
		R=G=B=(lum*RGBMAX)/HLSMAX;
		if (hue != UNDEFINED) {
            /* ERROR */ 
		}
	}
	else  {                    /* chromatic case */ 
         /* set up magic numbers */ 
		if (lum <= (HLSMAX/2))
			Magic2 = (lum*(HLSMAX + sat) + (HLSMAX/2))/HLSMAX;
		else
			Magic2 = lum + sat - ((lum*sat) + (HLSMAX/2))/HLSMAX;
		Magic1 = 2*lum-Magic2;
         /* get RGB, change units from HLSMAX to RGBMAX */ 
		R = (HueToRGB(Magic1,Magic2,hue+(HLSMAX/3))*RGBMAX + (HLSMAX/2))/HLSMAX;
		G = (HueToRGB(Magic1,Magic2,hue)*RGBMAX + (HLSMAX/2)) / HLSMAX;
		B = (HueToRGB(Magic1,Magic2,hue-(HLSMAX/3))*RGBMAX + (HLSMAX/2))/HLSMAX;
	}
	return (R<<16)|(G<<8)|B;
}

RectF Union(RectF *rc1,RectF *rc2) {
	RectF rc;
	if(rc1->Width<=0 || rc1->Height<=0)
		return *rc2;
	if(rc2->Width<=0 || rc2->Height<=0)
		return *rc1;
	rc.Union(rc,*rc1,*rc2);
	return rc;
}
class RecentFiles {
	static int id[8];
public:
	wchar_t strRecentProj[10][MAX_PATH];
	void UpdateMenu(HWND hwnd) {
		int i;
		if(hwnd) {
			HMENU hmenu=GetMenu(hwnd);
			HMENU hmenuSub=GetSubMenu(hmenu,0);
			MENUITEMINFO mii;
			wchar_t strName[MAX_PATH];
			for(i=0;i<8;++i) {
				if(strRecentProj[i][0]) {
					swprintf(strName,L"&%d: %s",i+1,PathFindFileName(strRecentProj[i]));
					ZeroMemory(&mii,sizeof(MENUITEMINFO));
					mii.cbSize = sizeof(MENUITEMINFO);
					mii.fMask = MIIM_TYPE;
					mii.fType = MFT_STRING;
					mii.fState = MFS_ENABLED;
					mii.dwTypeData = strName;
					SetMenuItemInfo(hmenuSub,id[i],0,&mii);
				}
			}
			DrawMenuBar(hwnd);
		}
	}
	wchar_t *Get(int i) {
		return strRecentProj[i];
	}
	void Add(HWND hwnd,wchar_t *str) {
		int i,j;
		if(str&&*str) {
			for(i=0;i<10;++i) {
				if(wcscmp(strRecentProj[i],str)==0) {
					for(j=i;j<9;++j)
						wcscpy(strRecentProj[j],strRecentProj[j+1]);
					strRecentProj[9][0]=0;
					break;
				}
			}
			for(i=9;i>=1;--i) {
				wcscpy(strRecentProj[i],strRecentProj[i-1]);
			}
			wcscpy(strRecentProj[0],str);
		}
		UpdateMenu(hwnd);
	}
};
int RecentFiles::id[]={ID_FILE_1,ID_FILE_2,ID_FILE_3,ID_FILE_4,ID_FILE_5,ID_FILE_6,ID_FILE_7,ID_FILE_8};

class ColorTool {
	HWND hwndMap,hwndBar,hwndColor;
	Bitmap *bmpMap, *bmpBar;
	int hueTab[5],lumTab[5],satTab[5];
	DWORD rgbTab[5];
	int iSel;
	Primitive *prTarget;
	WNDPROC wndprocOrg;
public:
	HWND hwnd;
	int xWin,yWin;
	void UpdateBar(void);
	void MouseMoveBar(int x,int y);
	void MouseMoveMap(int x,int y);
	static LRESULT CALLBACK wndprocSubclass(HWND hwnd,UINT msg,WPARAM wparam,LPARAM lparam);
	static LRESULT CALLBACK wndprocBar(HWND hwnd,UINT msg,WPARAM wparam,LPARAM lparam);
	static LRESULT CALLBACK wndprocMap(HWND hwnd,UINT msg,WPARAM wparam,LPARAM lparam);
	static BOOL CALLBACK dlgprocFrame(HWND hwnd,UINT uMsg,WPARAM wparam,LPARAM lparam);
	ColorTool(HWND hwndParent,int x,int y);
	~ColorTool(void);
	void Show(Primitive *pr,int iSelect);
	void Show(int);
	int IsShow(void);
	void Set(int i);
	void Set(Primitive *pr);
	void SetColor(DWORD rgb);
	void Show(Primitive *pr);
	void UpdateByKey(int id);
	void Update(bool bAdjust=true);
	DWORD GetCurrentCol(int i) {return rgbTab[i];}
	void LoadPal(void);
	void SavePal(void);
};

class Bezier {
public:
	Matrix *mxScale;
	RectF rcOrg;
	PointF ptAnchor[320];
	int iClose;
	int iIndex;
	int iFocus;
	bool bDrag;
	double dScaleX,dScaleY;
	Bezier(void);
	~Bezier(void);
	Bezier& operator=(Bezier& t);
	void Reset(void);
	void AddPoint(PointF pt);
	void Down(PointF pt);
	void Drag(PointF pt);
	void Up(PointF pt);
	Primitive *Close(bool bFill=true);
	void Draw(HDC hdc,PointF pt1=PointF(0,0),PointF pt2=PointF(0,0),Matrix *mxRot=0);
	void Draw(Graphics *g,PointF pt1=PointF(0,0),PointF pt2=PointF(0,0),Matrix *mxRot=0);
	void DelPoint(Primitive *pr=0);
	void MirrorH(REAL x);
	void MirrorV(REAL y);
	void Normalize(RectF *prcf);
	void NormalizeRot(Primitive *pr);
	void SetScale(Primitive *pr);
	int HitTest(int x,int y,int iControl=0,PointF pt1=PointF(0,0),PointF pt2=PointF(0,0),Matrix *mxRot=0);
	void GetPath(int iAdjust,int iClose,GraphicsPath *path,RectF *rcfBound);
};
class Tex {
	void Load(void);
	void LoadBmp(void);
public:
	wchar_t strFile[MAX_PATH];
	wchar_t strName[MAX_PATH*2];
	Bitmap *bmp;
	int iWidth;
	int iHeight;
	int *piLumi;
	int *piAlpha;
	float fRot,fZoomX,fZoomY,fOffX,fOffY;
	Matrix *mx;
	bool bInitialize;
	bool bInitializeThumb;
	Tex *Clone(void);
	Tex(void);
	Tex(wchar_t *strFile);
	Tex(wchar_t *strDir,wchar_t *strFile);
	~Tex(void);
	void SetRotZoom(float r,float offx,float offy,float zx,float zy);
	void SetupMx(void);
	int Get(float x,float y);
	int GetAlpha(float x,float y);
	Bitmap *GetBmp(void);
};
class Texture {
	Bitmap *bmpInit;
	int CountTex(wchar_t *wstrType,wchar_t *wstrDir);
public:
	int iInitializing;
	Tex **tex;
	Tex *texCurrent;
	int iTexMax;
	Texture(void);
	~Texture(void);
	void Init(wchar_t *);
	void Reset(Primitive *p);
	int Get(int x,int y);
	int GetAlpha(int x,int y);
	int Find(wchar_t *);
	wchar_t *GetName(int n);
	HIMAGELIST CreateImageList(Primitive *p);
};
class LightingParam {
public:
	int iSpecularType;
	int iSpecularDir;
	int iSpecularOffset;
	int iSpecular;
	int iHighlight;
	int iHighlightWidth;
	int iAmbient;
	float fDirX,fDirY;
	float fDir3X,fDir3Y,fDir3Z;
	float fSpecular;
	float fHighlight;
	float fHighlightWidth;
	float fAmbient;
	void Setup(void) {
		fSpecular=(float)iSpecular*.01f;
		fHighlight=(float)iHighlight*.01f;
		fHighlightWidth=(float)iHighlightWidth*.01f;
		fAmbient=(float)iAmbient*.01f;
		fDirX=-sinf((float)(iSpecularDir*PI/180));
		fDirY=cosf((float)(iSpecularDir*PI/180));
		fDir3Z=cosf(iSpecularOffset*.01f*PI*.5f);
		float f=sqrtf(1.f-fDir3Z*fDir3Z);
		if(iSpecularOffset>0)
			f=-f;
		fDir3X=fDirX*f;
		fDir3Y=fDirY*f;
	}
};
typedef enum {Normal,Invert,Border} BasePaintMode;
class ExpoShaper {
	bool bInit;
	int iLinear,iSmooth;
	int iVal[257];
public:
	ExpoShaper(void) {
		iLinear=iSmooth=-1;
		bInit=false;
	}
	~ExpoShaper(void) {
	}
	void Init(void) {
		iLinear=iSmooth=-1;
		Setup(0,0);
		bInit=true;
	}
	void Setup(int iLin,int iSm) {
		int t,i,iOld,iWidth,iCount;
		float fT,fX,fY,fYOld,fSm,fSmTable[257];
		if(bInit==true && iLin==iLinear && iSm==iSmooth)
			return;
		bInit=true;
		iLinear=iLin;
		iSmooth=iSm;
		if(iLin==0 && iSm==0) {
			for(i=0;i<256;++i)
				iVal[i]=i;
		}
		fSm=(float)iSm*.005f;
		fSmTable[256]=1.f;
		iOld=0;
		fYOld=0.f;
		for(t=0;t<256;++t) {
			fT=(float)t/256.f;
			fX=3*(1-fT)*(1-fT)*fT*fSm+3*(1-fT)*fT*fT*(1-fSm)+fT*fT*fT;
			fY=3*(1-fT)*fT*fT+fT*fT*fT;
			i=(int)(fX*256);
			if(i>=256)
				i=256;
			iWidth=i-iOld;
			iCount=0;
			while(iOld<i)
				fSmTable[iOld++]=fYOld+(fY-fYOld)*iCount++/iWidth;
			fYOld=fY;
		}
		iWidth=256-iOld;
		iCount=0;
		while(iOld<256)
			fSmTable[iOld++]=fYOld+(fY-fYOld)*iCount++/iWidth;
		float coeff=(float)iLin*.01f;
		if(fabs(coeff)<=0.0005f) {
			for(i=0;i<256;++i)
				iVal[i]=(int)(fSmTable[i]*255);
		}
		else {
			float c,c2;
			float fI,fR;
			c2=coeff * 2.f * PI;
			fR=1.f/(1.f-exp(c2));
			for(i=0;i<256;++i) {
				fI=(float)i/255;
				c=(1.f-exp(fI*c2))*fR;
				if(c<0)
					c=0.f;
				if(c>1.f)
					c=1.f;
				iVal[i]=fSmTable[(int)(c*255)]*255;
			}
		}
		iVal[256]=255;
	}
	int Get(float x) {
		if(x>=1.f)
			return 255;
		if(x<=0.f)
			return 0;
		return iVal[(int)(x*256)];
	}
};
class Primitive {
	void Specular(int *a,int *b,float x,float y,LightingParam *lp);
	float fEmbossDirX,fEmbossDirY;
public:
	Primitive *prParent;
	Primitive *prNext;
	bool bInitializing;
	int iNum;
	HTREEITEM hitem;
	Bitmap *bmpBase,*bmp,*bmpPreview;
	int *piBumpEmboss;
	int iVisible;
	int iLock;
	int iAutoSize;
	int iSmallFontOpt;
	int iOperation;
	int iBmpOffX;
	int iBmpOffY;
	int iBmpOrg;
	int iBmpBaseDirty,iBmpDirty,iBmpPrevDirty;
	prim prType;
	int iMargin;
	PointF ptfOrg[2];
	PointF ptRect[4];
	RectF rcOutline;
	RectF rcBmp;
	Bezier *bz;
	Image *img;
	GraphicsPath *pathOutline;
	GraphicsPath *path;
	ExpoShaper exs;
	int iBmpSize;
	DWORD dwColor1;
	DWORD dwColor2;
	DWORD dwColor3;
	PointF ptGrad[2];
	PointF ptSpec[2];
	int iGradationType;
	int iLinear;
	int iSmoother;
	float rAngle;
	float fSlant;
	int iStart;
	int iStop;
	int iAntialias;
	int iVertex;
	REAL rTension;
	REAL rTwist;
	int iLineType;
	int iStarDepth;
	int iFill;
	int iBorder;
	int iBorderType;
	int iBorderJoin;
	int iLineCap;
	int iClose;
	int iBorderAsFill;
	int iPie;
	float fWidth;
	int iAlpha;
	float fRoundX1;
	float fRoundX2;
	float fRoundX3;
	float fRoundX4;
	int iRoundSeparate;
	float fRoundY1;
	float fRoundY2;
	float fRoundY3;
	float fRoundY4;
	int iRoundXYSeparate;
	int iCornerC1;
	int iCornerC2;
	int iCornerC3;
	int iCornerC4;

	LightingParam light[2];
	int iEmbossDirEnable;
	int iEmbossDir;
	REAL rEmbossWidth;
	float fEmbossDepth;
	float fEmbossDepth2;
	int iDiffuse;
	DWORD dwShadowColor;
	int iShadowDirEnable;
	int iShadowDir;
	int iShadowOffset;
	int iShadowDensity;
	REAL rShadowDiffuse;
	DWORD dwIShadowColor;
	int iIShadowDirEnable;
	int iIShadowDir;
	int iIShadowOffset;
	int iIShadowDensity;
	REAL rIShadowDiffuse;
	int iTexture;
	int iUseTextureAlpha;
	int iTextureType;
	int iTextureZoomXYSepa;
	float fTextureZoomX;
	float fTextureZoomY;
	float fTextureOffX;
	float fTextureOffY;
	float fTextureRot;
	float fFontSize;
	float fFontAspect;
	float fFontSpacing;
	int iFontSizeUnit;
	int iUseTextPath;
	int iKeepDir;
	float fPathOffset;
	int iBold;
	int iItalic;
	int iFix;

	Matrix *mxRot,*mxShift;
	Tex *texEmbed;
	wchar_t strName[64];
	wchar_t strTextureName[128];
	wchar_t strText[320];
	wchar_t strFont[128];
	wchar_t strFile[MAX_PATH];
	void CalcPath(int iDirty,int iAdjustRect);
	void CalcParent(bool bSkipThis=true);
	Primitive(bool bInit);
	Primitive(prim prType,float x1=-1,float y1=0,float x2=0,float y2=0);
	Primitive(Image,int x,int y);
	~Primitive(void);
	void GetPathBounds(GraphicsPath *path,RectF *rcf);
	void AdjustBezier(void);
	void SetDirty(void) {iBmpBaseDirty=iBmpDirty=iBmpPrevDirty=2;}
	void SetPos(float x,float y,float w,float h,int iAdjustRect);
	int SetPos(int iDrag,float x,float y);
	void SetAngle(REAL rA);
	bool SetColor(int);
	bool IsVisible(float x,float y);
	bool MakeBaseBmp(Bitmap **pbmp);
	void Assemble(void);
	void SetBmpSize(Bitmap *pbmp,RectF rcf);
	void Effect(BitmapData bmpd,int iW,int iH,bool bBmp);
	void Paint(bool bBmp,DWORD *pdw,int iWidth,int iHeight,int iStride);
	void MakeAlphaMap(bool bBmp,int *piAlpha,DWORD *Scan0,int Width, int Height, int Stride);
	DWORD GetGrad(int x,int y);
	PointF GradPoint(int n);
	int IsLock(void);
	void Lock(int);
	void Normalize(void);
	void MakeCircle(void);
	void DrawBasePaint(Graphics *g,Primitive *prBase,Bitmap *bmp,BasePaintMode mode,Color col,int iOffX,int iOffY);
	void BasePaint(Graphics *g,Primitive *prBase,Bitmap *bmp,int iOffX,int iOffY);
	void PaintChild(void);
};
class PrimitiveList {
public:
	RectF rc;
	int iParent;
	int iCount;
	Primitive *pr;
	PrimitiveList *next;
	PrimitiveList(void) {
		next=0;
		pr=0;
		iParent=0;
		iCount=0;
		rc.X=rc.Y=rc.Width=rc.Height=-1;
	}
	~PrimitiveList(void) {
		if(next)
			delete next;
		next=0;
		if(pr)
			delete pr;
		pr=0;
	}
	PrimitiveList *Clone(void) {
		PrimitiveList *pl,*pl1;
		pl=new PrimitiveList();
		for(pl1=this;pl1->next!=NULL;pl1=pl1->next)
			pl->Add(pl1->pr,pl1->iParent);
		return pl;
	}
	void Clear(void) {
		if(next)
			delete next;
		next=0;
		if(pr)
			delete pr;
		pr=0;
		rc.X=rc.Y=rc.Width=rc.Height=-1;
		iCount=0;
	}
	void Add(Primitive* p,int iPar) {
		PrimitiveList *pl;
		RectF rcf;
		for(pl=this;pl->next!=NULL;pl=pl->next)
			;
		pl->iParent=iPar;
		pl->CopyFrom(p);
		pl->next=new PrimitiveList();
		rc.Width=rc.Height=0;
		rcf.X=pr->ptfOrg[0].X;
		rcf.Y=pr->ptfOrg[0].Y;
		rcf.Width=pr->ptfOrg[1].X-rcf.X;
		rcf.Height=pr->ptfOrg[1].Y-rcf.Y;
		if(rcf.Width<0)
			rcf.X+=rcf.Width,rcf.Width=-rcf.Width;
		if(rcf.Height<0)
			rcf.Y+=rcf.Height,rcf.Height=-rcf.Height;
		rc=Union(&rc,&rcf);
		++iCount;
	}
	void CopyFrom(Primitive *p) {
		if(pr)
			delete pr;
		pr=new Primitive(false);
		Copy(pr,p);
	}
	void CopyTo(Primitive *p) {
		Copy(p,pr);
	}
	void Copy(Primitive *dst,Primitive *src);
};
class PrimitivePtrList {
public:
	int iCount;
	Primitive *pr;
	PrimitivePtrList *next;
	RectF rc;
	bool bSel;
	PrimitivePtrList(void) {
		pr=NULL;
		next=NULL;
		iCount=0;
		rc.Width=-1;
		bSel=false;
	}
	~PrimitivePtrList(void) {
		if(next)
			delete next;
	}
	PrimitivePtrList *Clone(void) {
		PrimitivePtrList *clone,*pl;
		clone=new PrimitivePtrList();
		clone->Clear();
		for(pl=this;pl->pr!=NULL;pl=pl->next) {
			clone->Add(pl->pr,pl->bSel);
		}
		return clone;
	}
	void Clear(void) {
		if(next)
			delete next;
		next=NULL;
		pr=NULL;
		iCount=0;
		rc.Width=-1;
		bSel=false;
	}
	PrimitivePtrList *Check(Primitive *p,bool bSel=false) {
		PrimitivePtrList *pl;
		if(next==0 || p==0)
			return 0;
		for(pl=this;pl->pr!=NULL;pl=pl->next) {
			if(pl->pr==p) {
				if(bSel && pl->bSel==false)
					return 0;
				return pl;
			}
		}
		return 0;
	}
	void SetCount(void) {
		int i=0;
		PrimitivePtrList *pl;
		for(pl=this;pl->next!=NULL;pl=pl->next)
			++i;
		iCount=i;
	}
	void Insert(Primitive *p,bool bSel);
	void Add(Primitive *p,bool bSel=true);
	void Del(Primitive *p,bool bSyncTree=true);
	void RecalcRect(void) {
		PrimitivePtrList *pl;
		if(this->pr)
			rc=this->pr->rcOutline;
		for(pl=this;pl->next!=NULL;pl=pl->next) {
			rc=Union(&rc,&pl->pr->rcOutline);
		}
	}
};
class Tree {
	HTREEITEM hitemDragging;
	HTREEITEM hitemTarget;
	HTREEITEM hitemSelOld;
	bool bAfter;
	bool bStepin;
	bool fEdit;
	bool bDragging;
	bool bAreaSelecting;
	int iImageItem;
	int iImageText;
	Primitive *prRoot;
	WNDPROC wndprocOrg;
	HTREEITEM MoveItemExec(Primitive *p,Primitive *pTarget,Primitive *pAfter,bool bFirst=false);
	HICON hiconVisible0,hiconVisible1,hiconVisibleSolo0,hiconVisibleSolo1,hiconLock0,hiconLock1;
	static LRESULT CALLBACK wndprocVisible(HWND hwnd,UINT uMsg,WPARAM wparam,LPARAM lparam);
	static LRESULT CALLBACK wndprocSubclassTree(HWND hwnd,UINT msg,WPARAM wparam,LPARAM lparam);
public:
	HWND hwnd,hwndVisible,hwndFrame;
	int iDisableResponse;
	bool bFloat;
	Primitive *prCurrent;
	PrimitivePtrList prlCurrent;
	Tree(HWND hwnd);
	~Tree(void);
	void SetName(Primitive *);
	Primitive *AddItem(Primitive *prParent,Primitive *prAfter,wchar_t *str,prim prType,float x1=-1,float y1=-1,float x2=-1,float y2=-1);
	Primitive *AddItem(Primitive *prParent,Primitive *prAfter,wchar_t *wstrFile,int x=-1,int y=-1);
	void ChangeIcon(Primitive *pr);
	Primitive *Group(void);
	void UnGroup(void);
	void DelAll(void);
	void RefreshAll(void);
	void DelItem(Primitive*,bool bRenum=true);
	Primitive *MoveItem(Primitive *p,Primitive *pTarget,Primitive *pAfter,bool bFirst=false);
	Primitive *MoveDragItem(Primitive *p,Primitive *pTarget,bool bAfter,bool bStepin);
	void CopyItem(Primitive*);
	void PasteItem(Primitive*);
	void MovePos(Primitive *p,float dx,float dy,int iDirty=2);
	void SetPos(Primitive *p,float x,float y);
	void Pos(int x,int y);
	void Size(int x,int y);
	Primitive *GetFirstFromTV(void);
	Primitive *GetLastFromTV(void);
	Primitive *GetNextFromTV(Primitive*);
	Primitive *GetPrevFromTV(Primitive*);
	Primitive *GetParentFromTV(Primitive*);
	Primitive *GetChildFromTV(Primitive*);
	bool IsCheckFromTV(Primitive*);

	Primitive *GetPrimitive(HTREEITEM hitem);
	Primitive *GetParent(Primitive*);
	Primitive *GetChild(Primitive*);
	Primitive *GetNextSibling(Primitive*);
	Primitive *GetPrevSibling(Primitive*);
	int GetParentNum(Primitive*);
	int GetCount(void);
	int GetChildNum(Primitive*);
	Primitive *GetFirst(void);
	Primitive *GetNext(Primitive*);
	Primitive *FindFromPos(int x,int y,int iNext);
	Primitive *FindFromNum(int n);
	Primitive *GetCurrent(void);
	Primitive *GetCurrentNext(Primitive*);
	void MoveParent(void);
	void MoveUp(void);
	void MoveDown(void);
	void TopMost(void);
	Primitive *MoveParent(Primitive*);
	Primitive *MoveUp(Primitive*);
	Primitive *MoveDown(Primitive*);
	void Dup(Primitive *p,Primitive *pTarget);
	void Foreground(Primitive *p);
	void Background(Primitive *p);
	Primitive *TopMost(Primitive *p);
	void BeginDrag(NMTREEVIEW *);
	void MouseMove(int,int);
	void LButtonUp(void);
	void Renumber(void);
	void Recalc(void);
	void MarkItem(Primitive *pr,int m);
	void SelectClear(void);
	void Select1(Primitive*pr,int iRefresh=1);
	bool IsSelected(Primitive*pr,bool bSel=false);
	bool IsGrouped(Primitive*pr);
	void SelectAdd(Primitive*pr,int iRefresh=1);
	void SelectDel(Primitive*pr);
	void SelectAll(void);
	void SelectVisible(void);
	void SelectInvert(void);
	void TrimmingExec(int x0,int y0,int x1,int y1);
	void Align(int iPos);
	void Distribution(int iPos);
	bool IsHit(Primitive*pr,int x,int y);
	void MoveSelected(int dx,int dy);
	HTREEITEM GetItemHandle(Primitive*);
	static LRESULT CALLBACK dlgprocTreeFrame(HWND hdlg, UINT message, WPARAM wparam, LPARAM lparam);
	void Show(int i);
	int IsShow(void);
};
class JournalLog {
public:
	JournalLog(void) {memset(this,0,sizeof JournalLog);}
	~JournalLog(void) {if(prlUndo) delete prlUndo;}
	PrimitiveList *prlUndo;
	int iWidth;
	int iHeight;
	Color colBackground;
	Color colWorkspace;
	bool bEdit;
	int iProp;
};
class Journal {
	Tree *tree;
	JournalLog *log;
	int iBuffNum;
	int iIndex;
	int iIndexStart;
	int iIndexEnd;
	bool bOnce;
	void CopyToLog(int);
	void CopyFromLog(int);
public:
	Journal(Tree*,int iLevel);
	~Journal(void);
	void Record(void);
	void RecordOnce(void);
	void RecordReset(void);
	void Undo(void);
	void Redo(void);
};
class ToolTip {
	HWND hwndTarget;
	HINSTANCE hinst;
	TOOLINFO ti;
	WNDPROC wndprocOrg;
public:
	HWND hwndTT;
	static LRESULT CALLBACK wndprocSubclass(HWND hwnd,UINT msg,WPARAM wparam,LPARAM lparam) {
		ToolTip *p=(ToolTip*)(LONG_PTR)GetWindowLongPtr(hwnd,GWLP_USERDATA);
		switch(msg) {
		case WM_DESTROY:
			WNDPROC wndproc;
			wndproc=p->wndprocOrg;
			delete p;
			return (CallWindowProc(wndproc, hwnd, msg, wparam, lparam));
		default:
			break;
		}
		return (CallWindowProc(p->wndprocOrg, hwnd, msg, wparam, lparam));
	}
	ToolTip(HWND hwnd,wchar_t *str) {
		RECT rc;
		wchar_t strClass[20];
		GetClassName(hwnd, strClass, sizeof(strClass));
		if(wcscmp(strClass,L"ComboBoxEx32")==0) {
			hwnd=GetWindow(hwnd,GW_CHILD);
		}
		hwndTT=CreateWindowEx(0,TOOLTIPS_CLASS,NULL,WS_POPUP|TTS_NOPREFIX|TTS_ALWAYSTIP
				,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,hwnd,NULL,hinstMain,NULL);
		hinst=hinstMain;
		hwndTarget=hwnd;
//		This make the bug 'FileOpenDialog can't come front
//		SetWindowPos(hwndTT,HWND_TOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);
		GetClientRect(hwnd,&rc);
		ZeroMemory(&ti,TTTOOLINFO_V1_SIZE);
		ti.cbSize=TTTOOLINFO_V1_SIZE;
		ti.uFlags=TTF_SUBCLASS;
		ti.hwnd=hwnd;
		ti.lpszText=str;
		ti.rect=rc;
		ti.uId=(UINT)hwndTT;
		SendMessage(hwndTT, TTM_ADDTOOL, 0, (LPARAM) (LPTOOLINFO) &ti);
		SendMessage(hwndTT, TTM_ACTIVATE,(WPARAM)TRUE,0);
		SetWindowLongPtr(hwnd,GWLP_USERDATA,(LONG)(LONG_PTR)this);
		wndprocOrg=(WNDPROC)(LONG_PTR)SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG)(LONG_PTR)wndprocSubclass);
	}
	~ToolTip(void) {
		SetWindowLongPtr(hwndTarget,GWLP_WNDPROC,(LONG)(LONG_PTR)wndprocOrg);
	}
	void UpdateText(wchar_t *str) {
		ti.lpszText=str;
		SendMessage(hwndTT,TTM_UPDATETIPTEXT,0,(WPARAM)&ti);
	}
};
class PrivateProfileWriter {
	FILE *fp;
public:
	PrivateProfileWriter(wchar_t *strFile,wchar_t *strMode) {
		fp=_wfopen(strFile,strMode);
		fputc(0xff,fp);
		fputc(0xfe,fp);
		fwprintf(fp,L"\r\n");
	}
	~PrivateProfileWriter(void) {
		fclose(fp);
	}
	void Section(wchar_t *strSection) {
		fwprintf(fp,L"[%s]\r\n",strSection);
	}
	void WriteStr(wchar_t *strKey,wchar_t *strValue) {
		if(strValue==0)
			strValue=L"";
		fwprintf(fp,L"%s=%s\r\n",strKey,strValue);
	}
	void WriteInt(wchar_t *strKey,int iValue) {
		fwprintf(fp,L"%s=%d\r\n",strKey,iValue);
	}
	void WriteFloat(wchar_t *strKey,float fValue) {
		fwprintf(fp,L"%s=%f\r\n",strKey,fValue);
	}
	static char Hex(int x) {
		return (char)(*(L"0123456789abcdef"+(x&0xf)));
	}
	void AppendFile(wchar_t *strFile) {
		FILE *fpFile;
		BYTE bDat[1024];
		int iLen;
		fpFile=_wfopen(strFile,L"rb");
		for(;;) {
			iLen=(int)fread(bDat,1,1024,fpFile);
			if(iLen==0)
				break;
			fwrite(bDat,1,iLen,fp);
		}
		fclose(fpFile);
	}
	void EmbedFile(wchar_t *strKeyBase,wchar_t *strFile) {
		int j;
		BYTE bDat[256];
		wchar_t strD[520],*p;
		wchar_t strKey[64];
		int iLen,iCount;
		FILE *fp;
		fp=_wfopen(strFile,L"rb");
		if(fp==NULL)
			return;
		iCount=0;
		for(;;) {
			iLen=(int)fread(bDat,1,252,fp);
			if(iLen==0)
				break;
			j=0;
			p=strD;
			while(iLen>=7) {
				*p++=0x8000|((WORD)bDat[j]<<6)|(bDat[j+1]>>2);
				*p++=0x8000|((WORD)(bDat[j+1]&0x03)<<12)|((WORD)bDat[j+2]<<4)|(bDat[j+3]>>4);
				*p++=0x8000|((WORD)(bDat[j+3]&0x0f)<<10)|((WORD)bDat[j+4]<<2)|(bDat[j+5]>>6);
				*p++=0x8000|((WORD)(bDat[j+5]&0x3f)<<8)|bDat[j+6];
				j+=7;
				iLen-=7;
			}
			while(iLen--) {
				*p++=0xc000|(bDat[j]&0xff);
				++j;
			}
			*p=0;
			swprintf(strKey,L"%s%d",strKeyBase,iCount++);
			WriteStr(strKey,strD);
		}
		fclose(fp);
	}
};

float wstrtofloat(wchar_t *wstr,wchar_t **pwstr) {
	bool bSign=0;
	float fVal=0.f;
	float fFrac;
	while(*wstr && *wstr<=' ')
		++wstr;
	if(*wstr=='-') {
		bSign=1;
		++wstr;
	}
	while(*wstr&&*wstr<=' ')
		++wstr;
	while(*wstr>='0'&&*wstr<='9') {
		fVal=fVal*10+*wstr-'0';
		++wstr;
	}
	if(*wstr=='.'||*wstr==',') {
		++wstr;
		fFrac=1.f;
		while(*wstr>='0'&&*wstr<='9') {
			fFrac*=0.1f;
			fVal+=fFrac*(*wstr-'0');
			++wstr;
		}
	}
	if(pwstr)
		*pwstr=wstr;
	if(bSign)
		return -fVal;
	return fVal;
}
//EditControl with modify check
typedef enum {Int,Float2,Float3,Text} ParamType;
class DoubleEdit {
public:
	WNDPROC wndprocOrg;
	HWND hwndTarget;
	bool bEdit;
	ParamType tType;
	HBRUSH hbr;
	int iID;
	static void EditValue(DoubleEdit *pde,float v) {
		wchar_t wstr[32];
		float fVal;
		if(pde->tType==Text)
			return;
		GetWindowText(pde->hwndTarget,wstr,32);
		fVal=wstrtofloat(wstr,NULL)+v;
		if(pde->tType==Float3)
			swprintf(wstr,L"%.3f",fVal);
		else if(pde->tType==Float2)
			swprintf(wstr,L"%.2f",fVal);
		else
			swprintf(wstr,L"%d",(int)fVal);
		SetWindowText(pde->hwndTarget,wstr);
		if(GetWindowLong(pde->hwndTarget,GWL_STYLE)&ES_MULTILINE) {
			int id=GetWindowLong(pde->hwndTarget,GWL_ID);
			SendMessage(GetParent(pde->hwndTarget),WM_COMMAND,(WPARAM)((EN_CHANGE<<16)|id),(LPARAM)pde->hwndTarget);
		}
		pde->bEdit=true;
		InvalidateRect(pde->hwndTarget,NULL,TRUE);
	}
	static LRESULT CALLBACK wndprocSubclassEdit(HWND hwnd,UINT msg,WPARAM wparam,LPARAM lparam) {
		DoubleEdit *pde=(DoubleEdit*)(LONG_PTR)GetWindowLongPtr(hwnd,GWLP_USERDATA);
		int i;
		switch(msg) {
		case WM_USER_CTLCOLOR:
			return pde->CtlColorEdit(wparam,lparam);
		case WM_KILLFOCUS:
			if(pde->bEdit) {
				SendMessage(GetParent(hwnd),WM_USER_ENDEDIT,(WPARAM)pde->iID,0);
				pde->bEdit=false;
				InvalidateRect(hwnd,NULL,TRUE);
			}
			break;
		case WM_MOUSEWHEEL:
			if(pde->tType==Text)
				break;
			i=HIWORD(wparam);
			if(i&0x8000)
				i|=0xffff0000;
			if(pde->tType==Float3) {
				if(LOWORD(wparam)&MK_SHIFT)
					EditValue(pde,(i/120)*0.001f);
				else
					EditValue(pde,(float)(i/120));
			}
			else if(pde->tType==Float2) {
				if(LOWORD(wparam)&MK_SHIFT)
					EditValue(pde,(i/120)*0.01f);
				else
					EditValue(pde,(float)(i/120));
			}
			else {
				EditValue(pde,(float)i/120);
			}
			return 0;
			break;
		case WM_KEYDOWN:
			switch(LOWORD(wparam)) {
			case VK_UP:
				if(GetAsyncKeyState(VK_SHIFT)) {
					if(pde->tType==Float3)
						EditValue(pde,0.001f);
					if(pde->tType==Float2)
						EditValue(pde,0.01f);
				}
				else
					EditValue(pde,1.f);
				return 0;
			case VK_DOWN:
				if(GetAsyncKeyState(VK_SHIFT)) {
					if(pde->tType==Float3)
						EditValue(pde,-0.001f);
					if(pde->tType==Float2)
						EditValue(pde,-0.01f);
				}
				else
					EditValue(pde,-1.f);
				return 0;
			case VK_RETURN:
				if(pde->bEdit) {
					SendMessage(GetParent(hwnd),WM_USER_ENDEDIT,(WPARAM)pde->iID,0);
					pde->bEdit=false;
					InvalidateRect(hwnd,NULL,TRUE);
				}
				break;
			case VK_ESCAPE:
				if(pde->bEdit==false) {
					SendMessage(GetParent(hwnd),WM_USER_STARTEDIT,0,0);
					pde->bEdit=true;
					InvalidateRect(hwnd,NULL,TRUE);
				}
				break;
			case VK_CONTROL:
			case VK_SHIFT:
			case VK_MENU:
				break;
			default:
				pde->bEdit=true;
				SendMessage(GetParent(hwnd),WM_USER_STARTEDIT,0,0);
				break;
			}
			break;
		case WM_DESTROY:
			WNDPROC wndproc;
			wndproc=pde->wndprocOrg;
			delete pde;
			return (CallWindowProc(wndproc, hwnd, msg, wparam, lparam));
		default:
			break;
		}
		return (CallWindowProc(pde->wndprocOrg, hwnd, msg, wparam, lparam));
	}
	LRESULT CtlColorEdit(WPARAM wparam,LPARAM lparam) {
		if(bEdit) {
			SetBkMode((HDC)wparam,TRANSPARENT);
			SetBkColor((HDC)wparam,RGB(255,255,128));
			return (LRESULT)hbr;
		}
		return FALSE;
	}
	DoubleEdit(HWND hwnd,ParamType tInit) {
		bEdit=false;
		tType=tInit;
		hwndTarget=hwnd;
		hbr=CreateSolidBrush(RGB(255,255,128));
		iID=GetWindowLong(hwnd,GWL_ID);
		SetWindowLongPtr(hwnd,GWLP_USERDATA,(LONG)(LONG_PTR)this);
		wndprocOrg=(WNDPROC)(LONG_PTR)SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG)(LONG_PTR)wndprocSubclassEdit);
	}
	~DoubleEdit(void) {
		SetWindowLongPtr(hwndTarget,GWLP_WNDPROC,(LONG)(LONG_PTR)wndprocOrg);
		DeleteObject((HGDIOBJ)hbr);
	}
};
// Spin contorl subclass
//   make slider style control from existing spin control.
//   destroy itself when the base control is destroyed.
class Slider {
public:
	WNDPROC wndprocOrg;
	float fValue,fValueOrg;
	POINT ptfOrg;
	int iMin,iMax;
	HWND hwndBuddy,hwndScroll,hwndTarget;
	HICON hiconSpin;
	bool fPress;
	ParamType tType;
	static void Add(HWND hwnd,int iMinInit,int iMaxInit,ParamType tTypeInit=Int) {
		if(hwnd)
			new Slider(hwnd,iMinInit,iMaxInit,tTypeInit);
	}
	static LRESULT CALLBACK wndprocSubclassUpDown(HWND hwnd,UINT msg,WPARAM wparam,LPARAM lparam) {
		RECT rcClient;
		PAINTSTRUCT ps;
		wchar_t wstr[32];
		SCROLLINFO si;
		HDC hdc;
		Slider *psli=(Slider*)(LONG_PTR)GetWindowLongPtr(hwnd,GWLP_USERDATA);
		switch(msg) {
		case WM_PAINT:
			hdc = BeginPaint(hwnd, &ps);
			GetClientRect(hwnd,&rcClient);
			DrawFrameControl(hdc, &rcClient, DFC_BUTTON, DFCS_BUTTONPUSH|DFCS_PUSHED);
			InflateRect(&rcClient,-2,-2);
			DrawFrameControl(hdc, &rcClient, DFC_BUTTON, DFCS_BUTTONPUSH | (psli->fPress?DFCS_PUSHED:0));
			DrawIconEx(hdc,rcClient.left,rcClient.top,psli->hiconSpin,rcClient.right-rcClient.left,rcClient.bottom-rcClient.top,0,NULL,DI_NORMAL);
			EndPaint(hwnd, &ps);
			break;
		case WM_LBUTTONDOWN:
			GetCursorPos(&psli->ptfOrg);
			GetWindowText(psli->hwndBuddy,wstr,32);
			swscanf(wstr,L"%f",&psli->fValueOrg);
			psli->hwndScroll=CreateWindow(L"SCROLLBAR",L"",WS_POPUP|WS_CHILD|WS_VISIBLE|SBS_VERT,psli->ptfOrg.x,psli->ptfOrg.y-64,16,128,hwnd,0,hinstMain,0);
			ZeroMemory(&si,sizeof(si));
			si.cbSize=sizeof(si);
			si.nPos=(int)(psli->iMax+psli->iMin-psli->fValueOrg);
			si.nMax=psli->iMax;
			si.nMin=psli->iMin;
			si.fMask=SIF_RANGE|SIF_POS;
			SetScrollInfo(psli->hwndScroll,SB_CTL,&si,true);
			SetCapture(hwnd);
			psli->fPress=true;
			InvalidateRect(hwnd,NULL,true);
			return TRUE;
		case WM_MOUSEMOVE:
			psli->MouseMove();
			return TRUE;
		case WM_RBUTTONDOWN:			// avoid remaining slider if rbutton down on scroll-bar
		case WM_LBUTTONUP:
			ReleaseCapture();
			DestroyWindow(psli->hwndScroll);
			SetFocus(psli->hwndBuddy);
			psli->fPress=false;
			InvalidateRect(hwnd,NULL,true);
			SendMessage(psli->hwndBuddy,WM_KEYDOWN,VK_RETURN,0);
			SendMessage(psli->hwndBuddy,WM_KEYUP,VK_RETURN,0);
			break;
		case WM_DESTROY:
			WNDPROC wndproc;
			wndproc=psli->wndprocOrg;
			delete psli;
			return (CallWindowProc(wndproc, hwnd, msg, wparam, lparam));
		default:
			break;
		}
		return (CallWindowProc(psli->wndprocOrg, hwnd, msg, wparam, lparam));
	}
	Slider(HWND hwnd,int iMinInit,int iMaxInit,ParamType tTypeInit=Int) {
		hwndTarget=hwnd;
		tType=tTypeInit;
		hiconSpin=LoadIcon(hinstMain,MAKEINTRESOURCE(IDI_UPDOWN));
		SendMessage(hwnd,UDM_SETRANGE32, (WPARAM)iMinInit, (LPARAM)iMaxInit);
		SetWindowLongPtr(hwnd,GWLP_USERDATA,(LONG)(LONG_PTR)this);
		hwndBuddy=(HWND)SendMessage(hwnd,UDM_GETBUDDY,0,0);
		new DoubleEdit(hwndBuddy,tType);
		iMin=iMinInit,iMax=iMaxInit;
		fPress=false;
		wndprocOrg=(WNDPROC)(LONG_PTR)SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG)(LONG_PTR)wndprocSubclassUpDown);
	}
	~Slider(void) {
		SetWindowLongPtr(hwndTarget,GWLP_WNDPROC,(LONG)(LONG_PTR)wndprocOrg);
		DestroyIcon(hiconSpin);
	}
	void MouseMove(void) {
		wchar_t wstr[32];
		POINT ptCur;
		float fV;
		SCROLLINFO si;
		if(GetCapture()!=hwndTarget)
			return;
		GetCursorPos(&ptCur);
		if(GetAsyncKeyState(VK_SHIFT)) {
			if(tType==Float2||tType==Float3)
				fV=fValueOrg+(ptfOrg.y-ptCur.y)*0.01f;
			else
				fV=(float)((int)fValueOrg+(ptfOrg.y-ptCur.y));
		}
		else {
			fV=(float)((int)fValueOrg+(ptfOrg.y-ptCur.y));
		}
		if(fV<iMin)
			fV=(float)iMin;
		if(fV>iMax)
			fV=(float)iMax;
		if(fV!=fValue) {
			SendMessage(hwndBuddy,WM_KEYDOWN,VK_ESCAPE,0);
			fValue=fV;
			fValueOrg=fValue;
			ptfOrg=ptCur;
			if(tType==Float3)
				swprintf(wstr,L"%.3f",fValue);
			else if(tType==Float2)
				swprintf(wstr,L"%.2f",fValue);
			else
				swprintf(wstr,L"%d",(int)fValue);
			SetWindowText(hwndBuddy,wstr);
			if(GetWindowLong(hwndBuddy,GWL_STYLE)&ES_MULTILINE) {
				int id=GetWindowLong(hwndBuddy,GWL_ID);
				SendMessage(GetParent(hwndBuddy),WM_COMMAND,(WPARAM)((EN_CHANGE<<16)|id),(LPARAM)hwndBuddy);
			}
			UpdateWindow(hwndBuddy);
			ZeroMemory(&si,sizeof(si));
			si.cbSize=sizeof(si);
			si.fMask=SIF_POS;
			si.nPos=(int)(iMax+iMin-fValue);
			SetScrollInfo(hwndScroll,SB_CTL,&si,true);
		}
	}
};
class Gausian {
	int MakeTable(float sigma);
	float fTable[64];
	int iTable[64];
	int iEm1;
	int iEm2;
	bool bBmp;
	int xMax;
public:
	Gausian(void);
	void SetMode(float sigma,float fEm1Init,float fEm2Init,bool bBmpInit);
	int GetAlphaValue(DWORD dw);
	void AlphaToInt(float s,int *piSrc,int *piDest,int w,int h,int iOff,float fDirX,float fDirY,int iInside);
	void AlphaToIntDiff(DWORD *pdwSrc,int *piDest,int w,int h,int iSrcStride,float fDirX,float fDirY);
};
class Screen {
public:
	int iWidth;
	int iHeight;
	int iFramesX;
	int iFramesY;
	int iAnimFrames;
	int iGridX;
	int iGridY;
	int iGridEnable;
	int iGridVisible;
	bool bStart;
	int iBgTransparent;
	int iOutScale;
	Color colBackground;
	Color colWorkspace;
	float fZoom;
	HANDLE hDrawRequest,hDrawRequest2;
	HANDLE hRenderWaiting,hRenderWaiting2;
	int iOffsetX,iOffsetY;
	int iDispOffX,iDispOffY;
	bool fRender;
	volatile bool fRenderBreak;
	HWND hwnd,hwndParent;
	HDC hdcMem,hdcMem2,hdcMemPreview,hdcScreen;
	DWORD *pdwMem,*pdwMem2,*pdwMemPreview;
	HBITMAP hbmpMem,hbmpMemOld,hbmpMem2,hbmpMem2Old,hbmpMemPreview,hbmpMemPreviewOld;
	HBITMAP hbmpScreen,hbmpScreenOld;
	Screen(HWND hwnd);
	~Screen(void);
	volatile int iExitDrawThread;
	static void DrawThread(void *);
	void Start(void);
	void WaitRender(void);
	void CanvasSize(int,int);
	void Trimming(void);
	void TrimmingVisible(void);
	void SetRender(bool);
	void ShowGrid(bool f);
	void Pos(int x,int y);
	void Size(int x,int y);
	void Paint(HDC hdc,RECT *);
	void Register(void);
	void Draw(bool bBgFill=true);
	void Send(void) {if(bStart) SetEvent(hDrawRequest);}
	void DrawHandle(HDC hdc,Primitive*);
	void DrawHandle(HDC hdc,PrimitivePtrList*);
	void DrawHandleMark(Graphics *g,PointF pt,int f=0,Color col=Color::White);
	void DrawGuide(Graphics *g,PointF ptfZero,PointF ptfMin,PointF ptfMax,PointF ptfScr);
	void DrawInvertRect(int x1,int y1,int x2,int y2);
	void ZoomIn(void);
	void ZoomOut(void);
	void Zoom1(void);
	void Zoom2(void);
	void GridVisible(int);
	void GridEnable(int);
	void ToggleGridVisible(void);
	PointF LogicalToScreen(float x,float y);
	PointF LogicalToScreen(PointF pt);
	PointF ScreenToLogical(PointF fp);
	void SetGradPoint(int iDrag,Primitive *pr,float xOrg,float yOrg,float xCur,float yCur,WPARAM wparam);
	int HitTestHandle(tool t,int sx,int sy,Primitive *p=0);
	int HitTest(Primitive **p,tool t,int sx,int sy,int iMode=0,int iKeyState=0); //iMode=0:CurrentSelectionOnly 1:All 2:Next
	bool Check(float x,float y,PointF pt);
	int GridX(int);
	int GridY(int);
	void SetGrid(int,int);
	static LRESULT CALLBACK wndproc(HWND hwnd,UINT message,WPARAM wparam,LPARAM lparam);
	static LRESULT CALLBACK dlgprocCanvasSize(HWND hwnd,UINT message,WPARAM wparam,LPARAM lparam);
	static LRESULT CALLBACK dlgprocGridSetup(HWND hwnd,UINT message,WPARAM wparam,LPARAM lparam);
};
class Prop {
	void SetScroll(void);
public:
	int iPage;
	prim prType;
	ToolTip *tipTex,*tipText,*tipFont;
	int iOffsetY;
	int iSubHeight;
	bool bFloat;
	HIMAGELIST himlBorderjoin;
	HIMAGELIST himlLineCap;
	HIMAGELIST himlTexture;
	Primitive *prCurrent;
	HWND hwndParent,hwndFrame,hwndSub,hwndAnim;
	FontSelector fsel;
	Prop(HWND);
	~Prop(void);
	void SetEnabledParam(void);
	void Pos(int x,int y);
	void Size(int x,int y);
	void SetupPos(Primitive *pr=0);
	void SetupFont(void);
	void Setup(Primitive *p,bool bAdjust=true);
	void SetupColor(void);
	void SetType(prim prType);
	void SetPage(int i);
	void Scroll(int,int);
	void ChangeIntValue(int iVal,Primitive *pr,int *piVal,int iAdjustRect=1);
	void ChangeOperation(int iVal,Primitive *pr,int *piVal,int iAdjustRect=1);
	void ChangeClose(int iVal,Primitive *pr,int *piVal);
	void ChangeString(wchar_t *strVal,Primitive *pr,wchar_t *strPr,int iAdjustRect=1);
	void CheckChange(HWND hwnd,int id,Primitive *pr,int *piVal,int iAdjust=1);
	void EnChange(HWND hwnd,int id,Primitive *pr,float *pfVal,int iAdjust=1);
	void EnChange(HWND hwnd,int id,Primitive *pr,int *piVal);
	void EnChangeAlpha(HWND hwnd,int id,Primitive *pr,DWORD *pdwCol);
	static LRESULT CALLBACK dlgprocPropFrame(HWND hdlg, UINT message, WPARAM wparam, LPARAM lparam);
	static LRESULT CALLBACK dlgprocPropSub(HWND hdlg, UINT message, WPARAM wparam, LPARAM lparam);
	static LRESULT CALLBACK dlgprocPropDummy(HWND hdlg,UINT message, WPARAM wparam, LPARAM lparam);
	static LRESULT CALLBACK wndprocSubclassPropSub(HWND hdlg,UINT message,WPARAM wparam,LPARAM lparam);
	WNDPROC wndprocOrg;
	void Show(int);
	int IsShow(void);
	void Destroy(void);
	void Resume(void);
};
class Tool {
	void SetButtonState(void);
public:
	tool toolCurrent;
	tool toolMaster;
	HWND hwndParent;
	HWND hwnd;
	bool bOnetime;
	HICON hiconArrow,hiconEdit,hiconHand,hiconRect,hiconEllipse,hiconPolygon,hiconText,hiconCurve,hiconShape,hiconLines,hiconGradation,hiconTrimming,hiconImage,hiconKnob;
	Tool(HWND hwnd);
	~Tool(void);
	static LRESULT CALLBACK dlgproc(HWND hwnd,UINT message,WPARAM wparam,LPARAM lparam);
	void SetCursorType(void);
	void Select(tool t,bool bOne=false);
	void SelectTemp(tool t);
	void Resume(void);
	void EndOneTime(void);
	void Pos(int x,int y);
	void Size(int x,int y);
};
class CmdBar {
public:
	HWND hwndParent;
	HWND hwnd;
	HICON hiconUp,hiconDown,hiconParent;
	HICON hiconAlignCenter,hiconZoomIn,hiconZoom1,hiconZoomOut,hiconGridVisible,hiconGridEnable,hiconDispTree,hiconDispProp,hiconDispColor;
	CmdBar(HWND hwnd);
	~CmdBar(void);
	static LRESULT CALLBACK dlgproc(HWND hwnd,UINT message,WPARAM wparam,LPARAM lparam);
	void Pos(int x,int y);
	void Size(int x,int y);
	void SetGridVisible(int);
	void SetGridEnable(int);
};
/////////////////////////////////////////////////////////////////////////////
Bezier *bzTemp;
Tree *theTree;
Journal *theJournal;
Screen *theScreen;
Prop *theProp;
Tool *theTool;
CmdBar *theCmdBar;
Texture *theTexture;
PrimitiveList *theClip,*theClip2;
Gausian *theGausian;
RecentFiles *theRecentFiles;
CRITICAL_SECTION csData;

class HideTools {
public:
	int iShowProp,iShowTree,iShowColor;
	HideTools(void) {}
	~HideTools(void) {}
	void Hide(void) {
		if(theApp->iAlwaysFront) {
			EnableWindow(theProp->hwndFrame,FALSE);
			EnableWindow(theTree->hwndFrame,FALSE);
			EnableWindow(theApp->pal->hwnd,FALSE);
		}
	}
	void Restore(void) {
		if(theApp->iAlwaysFront) {
			EnableWindow(theProp->hwndFrame,TRUE);
			EnableWindow(theTree->hwndFrame,TRUE);
			EnableWindow(theApp->pal->hwnd,TRUE);
		}
	}
};

void LoadPal(PrivateProfileReader *ppr) {
	int i;
	wchar_t str[24];
	ppr->SetSection(L"Pal");
	for(i=0;i<24;++i) {
		wsprintf(str,L"Col%d",i);
		colCust[i]=ppr->ReadInt(str,colCust[i]);
	}
	InvalidateRect(theApp->pal->hwnd,NULL,TRUE);
}
void SavePal(PrivateProfileWriter *ppw) {
	int i;
	wchar_t str[24];
	ppw->Section(L"Pal");
	for(i=0;i<24;++i) {
		wsprintf(str,L"Col%d",i);
		ppw->WriteInt(str,colCust[i]);
	}
}
POINT GetCenterPos(void) {
	POINT pt;
	RECT rc;
	GetClientRect(theScreen->hwnd,&rc);
	pt.x=theScreen->GridX((int)(((rc.left+rc.right)/2-theScreen->iDispOffX)/theScreen->fZoom));
	pt.y=theScreen->GridY((int)(((rc.top+rc.bottom)/2-theScreen->iDispOffY)/theScreen->fZoom));
	return pt;
}
Image *ImageFromFile(wchar_t *wstr) {
	HANDLE h;
	WORD wID;
	DWORD dwSize,dwLen;
	Image *img;
	IStream* pIStream;
	char *pBuf;
	LARGE_INTEGER li;
	if(wcsicmp(PathFindExtension(wstr),L".skin")==0) {
		PrivateProfileReader ppr(wstr);
		ppr.SetSection(L"Thumbnail");
		ppr.ExtractFile(L"T",theApp->strThumbnail);
		h=CreateFile(theApp->strThumbnail,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);
		dwSize=GetFileSize(h,NULL);
		pBuf=new char[dwSize];
		ReadFile(h,pBuf,dwSize,&dwLen,0);
		CreateStreamOnHGlobal(NULL,TRUE,&pIStream);
		pIStream->Write(pBuf,dwSize,0);
		img=new Image(pIStream);
		delete[] pBuf;
		CloseHandle(h);
		pIStream->Release();
		return img;
	}
	if(wcsicmp(PathFindExtension(wstr),L".knob")==0) {
		h=CreateFile(wstr,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);
		ReadFile(h,&wID,2,&dwLen,0);
		img=0;
		if(wID==0x5089) {
			CloseHandle(h);
		}
		else if(wID==0x4d4b) {
			li.QuadPart=0;
			ReadFile(h,&dwSize,4,&dwLen,0);
			pBuf=new char[dwSize];
			ReadFile(h,pBuf,dwSize-6,&dwLen,0);
			CreateStreamOnHGlobal(NULL,TRUE,&pIStream);
			pIStream->Write("\x89\x50\x4e\x47\x0d\x0a",6,0);
			pIStream->Write(pBuf,dwSize-6,0);
			pIStream->Seek(li,STREAM_SEEK_SET,0);
			img=new Image(pIStream);
			delete[] pBuf;
			CloseHandle(h);
			pIStream->Release();
			return img;
		}
		else
			CloseHandle(h);
	}
	h=CreateFile(wstr,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);
	if(h==INVALID_HANDLE_VALUE) {
		HICON hiconNofile;
		hiconNofile=LoadIcon(hinstMain,MAKEINTRESOURCE(IDI_NOFILE));
		img=new Bitmap(hiconNofile);
		return img;
	}
	dwSize=GetFileSize(h,NULL);
	pBuf=new char[dwSize];
	ReadFile(h,pBuf,dwSize,&dwLen,0);
	CreateStreamOnHGlobal(NULL,TRUE,&pIStream);
	pIStream->Write(pBuf,dwSize,0);
	img=new Image(pIStream);
	delete[] pBuf;
	CloseHandle(h);
	pIStream->Release();
	return img;
}

void DelPrimitive(bool bRecord=true) {
	Primitive **pr,*prParent;
	PrimitivePtrList *prl;
	int i,j;
	bool bRecalc;
	prl=&theTree->prlCurrent;
	if(prl->next==0)
		return;
	if(bRecord)
		theJournal->Record();
	theApp->Edit();
	pr=new Primitive*[prl->iCount];
	for(i=0;prl && prl->next;prl=prl->next,++i) {
		pr[i]=prl->pr;
	}
	EnterCriticalSection(&csData);
	for(j=0;j<i;++j) {
		if(theTree->GetItemHandle(pr[j])) {
			bRecalc=false;
			prParent=theTree->GetParent(pr[j]);
			if(pr[j]->iOperation)
				bRecalc=true;
			theTree->DelItem(pr[j]);
			if(bRecalc&&prParent)
				prParent->CalcPath(2,1);
		}
	}
	theApp->UpdateTitle();
	theTree->Renumber();
	theTree->Recalc();
	theTree->Select1(NULL);
	if(theTool->toolCurrent==tEdit || theTool->toolCurrent==tGradation)
		theTool->Select(tArrow);
	delete[] pr;
	theProp->SetType(prtNone);
	theProp->prCurrent=0;
	theScreen->Send();
	LeaveCriticalSection(&csData);
}
void CopyCoodinate(void) {
	char str[64];
	char *pMem;
	if(theProp->prCurrent) {
		sprintf(str,"%d,%d",(int)(theProp->prCurrent->ptfOrg[0].X),(int)(theProp->prCurrent->ptfOrg[0].Y));
		HGLOBAL hglb=GlobalAlloc(GHND, (strlen(str)+1));
		pMem = (char*)GlobalLock(hglb);
		memcpy(pMem,str,(strlen(str)+1));
		GlobalUnlock(hglb);
		OpenClipboard(hwndMain);
		EmptyClipboard();
		SetClipboardData(CF_TEXT, hglb);
		CloseClipboard();
	}
}
void CopyPrimitive(bool bIgnoreCurrent=false) {
	void Save(wchar_t *str,bool bExport,bool bIgnoreCurrent);
	Save(theApp->strClipboard,true,bIgnoreCurrent);
	theClip->Clear();
	theClip->Add(theProp->prCurrent,0);
}
void CopyPrimitive2(bool bIgnoreCurrent=false) {
	void Save(wchar_t *str,bool bExport,bool bIgnoreCurrent);
	Save(theApp->strClipboard2,true,bIgnoreCurrent);
	theClip2->Clear();
	if(theProp->prCurrent)
		theClip2->Add(theProp->prCurrent,0);
}
void CutPrimitive(bool bIgnoreCurrent=false,bool bRecord=true) {
	CopyPrimitive(bIgnoreCurrent);
	DelPrimitive(bRecord);
}
void CutPrimitive2(bool bIgnoreCurrent=false,bool bRecord=true) {
	CopyPrimitive2(bIgnoreCurrent);
	DelPrimitive(bRecord);
}
void PastePrimitive(bool bAsChild,bool bRec) {
	void Open(wchar_t *,int,bool,bool,int);
	Open(theApp->strClipboard,2,bAsChild,bRec,1);
	theTree->Recalc();
}
void PastePrimitive2(bool bAsChild,bool bRec,int iRefresh=1) {
	void Open(wchar_t *,int,bool,bool,int);
	Open(theApp->strClipboard2,2,bAsChild,bRec,iRefresh);
	theTree->Recalc();
}
void RefreshAll(void) {
	Primitive *pr;
	pr=theTree->GetFirst();
	while(pr) {
		if(pr->prType==prtImage) {
			delete pr->img;
			pr->img=ImageFromFile(EmbedNameToTempName(pr->strFile));
			pr->CalcPath(2,1);
		}
		pr=theTree->GetNext(pr);
	}
	theScreen->Send();
}
bool GroupIfMulti(void) {
	if(theTree->prlCurrent.iCount>1) {
		Group(false);
		return true;
	}
	return false;
}
void Group(int iRefresh) {
	Primitive *pr,*prParent,*prPrev,*prGroup;
	pr=theTree->prlCurrent.pr;
	if(pr==NULL)
		return;
	prParent=theTree->GetParent(pr);
	prPrev=theTree->GetPrevSibling(pr);
	if(iRefresh)
		theJournal->Record();
	theApp->Edit();
	EnterCriticalSection(&csData);
	CutPrimitive2(false,false);
	if(prPrev)
		prGroup=theTree->AddItem(prParent,prPrev,L"Group",prtGroup);
	else
		prGroup=theTree->AddItem(prParent,(Primitive*)-1,L"Group",prtGroup);
	theTree->Select1(prGroup,0);
	PastePrimitive2(true,false,0);
	theTree->Select1(prGroup);
	if(iRefresh)
		theScreen->Send();
	LeaveCriticalSection(&csData);
}
void UnGroup(int iRefresh) {
	Primitive *pr,*prParent,*prPrev;
	pr=theTree->prlCurrent.pr;
	if(pr==NULL || pr->prType!=prtGroup) {
		return;
	}
	if(iRefresh)
		theJournal->Record();
	theApp->Edit();
	prParent=theTree->GetParent(pr);
	prPrev=theTree->GetPrevSibling(pr);
	if(prParent==pr)
		prParent=NULL;
	EnterCriticalSection(&csData);
	CutPrimitive2(true,false);
	if(prPrev) {
		theTree->Select1(prPrev,0);
		PastePrimitive2(false,false);
	}
	else {
		theTree->Select1(prParent,0);
		PastePrimitive2(true,false);
	}
	theScreen->Send();
	LeaveCriticalSection(&csData);
}
void PasteColor(void) {
	PrimitivePtrList *prl;
	if(theClip->next==0)
		return;
	theJournal->Record();
	theApp->Edit();
	EnterCriticalSection(&csData);
	for(prl=&theTree->prlCurrent;prl->next;prl=prl->next) {
		prl->pr->dwColor1=theClip->pr->dwColor1;
		prl->pr->dwColor2=theClip->pr->dwColor2;
		prl->pr->dwColor3=theClip->pr->dwColor3;
		prl->pr->dwShadowColor=theClip->pr->dwShadowColor;
		prl->pr->dwIShadowColor=theClip->pr->dwIShadowColor;
		prl->pr->SetDirty();
	}
	theScreen->Send();
	LeaveCriticalSection(&csData);
}
void PasteEffects(void) {
	PrimitivePtrList *prl;
	if(theClip->next==0)
		return;
	theJournal->Record();
	theApp->Edit();
	EnterCriticalSection(&csData);
	for(prl=&theTree->prlCurrent;prl->next;prl=prl->next) {
		prl->pr->light[0]=theClip->pr->light[0];
		prl->pr->light[1]=theClip->pr->light[1];

		prl->pr->rEmbossWidth=theClip->pr->rEmbossWidth;
		prl->pr->fEmbossDepth=theClip->pr->fEmbossDepth;
		prl->pr->fEmbossDepth2=theClip->pr->fEmbossDepth2;
		prl->pr->iEmbossDir=theClip->pr->iEmbossDir;
		prl->pr->iEmbossDirEnable=theClip->pr->iEmbossDirEnable;
		prl->pr->iTextureType=theClip->pr->iTextureType;
		prl->pr->iUseTextureAlpha=theClip->pr->iUseTextureAlpha;
		prl->pr->iTexture=theClip->pr->iTexture;
		prl->pr->iTextureZoomXYSepa=theClip->pr->iTextureZoomXYSepa;
		prl->pr->fTextureZoomX=theClip->pr->fTextureZoomX;
		prl->pr->fTextureZoomY=theClip->pr->fTextureZoomY;
		prl->pr->fTextureOffX=theClip->pr->fTextureOffX;
		prl->pr->fTextureOffY=theClip->pr->fTextureOffY;
		prl->pr->fTextureRot=theClip->pr->fTextureRot;
		wcscpy(prl->pr->strTextureName,theClip->pr->strTextureName);
		prl->pr->iShadowDirEnable=theClip->pr->iShadowDirEnable;
		prl->pr->iShadowDir=theClip->pr->iShadowDir;
		prl->pr->iShadowOffset=theClip->pr->iShadowOffset;
		prl->pr->iShadowDensity=theClip->pr->iShadowDensity;
		prl->pr->rShadowDiffuse=theClip->pr->rShadowDiffuse;
		prl->pr->iIShadowDirEnable=theClip->pr->iIShadowDirEnable;
		prl->pr->iIShadowDir=theClip->pr->iIShadowDir;
		prl->pr->iIShadowOffset=theClip->pr->iIShadowOffset;
		prl->pr->iIShadowDensity=theClip->pr->iIShadowDensity;
		prl->pr->rIShadowDiffuse=theClip->pr->rIShadowDiffuse;
		prl->pr->SetDirty();
	}
	theProp->Setup(theTree->prlCurrent.pr);
	theScreen->Send();
	LeaveCriticalSection(&csData);
}
void Shortcut(int k,int m) {
	if(m) {
		switch(k) {
		case '#':
			theScreen->ToggleGridVisible();
			break;
		case '+':
		case ';':
			theScreen->ZoomIn();
			break;
		case '-':
			theScreen->ZoomOut();
			break;
		case '1':
			theScreen->Zoom1();
			break;
		case '2':
			theScreen->Zoom2();
			break;
		case 't':
			theTool->Select(tText,true);
			break;
		case 'T':
			theTool->Select(tText);
			break;
		case 'r':
			theTool->Select(tRect,true);
			break;
		case 'R':
			theTool->Select(tRect);
			break;
		case 'l':
			theTool->Select(tLines,true);
			break;
		case 'L':
			theTool->Select(tLines);
			break;
		case 'm':
		case 'M':
			theTool->Select(tTrimming);
			break;
		case 'e':
			theTool->Select(tEllipse,true);
			break;
		case 'E':
			theTool->Select(tEllipse);
			break;
		case 'p':
			theTool->Select(tPolygon,true);
			break;
		case 'P':
			theTool->Select(tPolygon);
			break;
		case 'a':
		case 'A':
			theTool->Select(tArrow);
			break;
		case 'c':
		case 'C':
			theTool->Select(tCurve);
			break;
		case 's':
		case 'S':
			theTool->Select(tShape);
			break;
		case 'h':
		case 'H':
			theTool->Select(tHand);
			break;
		case 'g':
		case 'G':
			theTool->Select(tGradation);
			break;
		case 'X'-0x40:
			CutPrimitive();
			break;
		case 'C'-0x40:
			CopyPrimitive();
			break;
		case 'V'-0x40:
			PastePrimitive(false,true);
			break;
		}
	}
	else {
		switch(k) {
		case VK_DELETE:
			DelPrimitive();
			break;
		}
	}
}
LRESULT CALLBACK wndprocDummy(HWND hwnd,UINT msg,WPARAM wparam,LPARAM lparam) {
	return 0;
	return DefWindowProc(hwnd,msg,wparam,lparam);
}
static WNDPROC wndprocPropOrg;
wchar_t *GetExportFileName(HWND hwnd,wchar_t **strType) {
	static wchar_t strExportFile[MAX_PATH];
	wchar_t *strExt;
	OPENFILENAME ofn;
	HideTools hide;
	memset(&ofn,0,sizeof(ofn));
	ofn.lStructSize=sizeof(ofn);
	ofn.hwndOwner=theApp->hwnd;
	ofn.hInstance=hinstMain;
	ofn.lpstrFilter=FILEFILTEREXPORT;
	ofn.lpstrFile=strExportFile;
	ofn.nFilterIndex=1;
	ofn.lpstrTitle=theLang->GetID(MSG_DIALOGEXPORTIMAGE);
	ofn.lpstrInitialDir=theApp->strDocDir;
	ofn.lpstrDefExt=L"png";
	ofn.nMaxFile=MAX_PATH;
	ofn.Flags=OFN_HIDEREADONLY|OFN_EXPLORER;
	ofn.Flags|=OFN_OVERWRITEPROMPT;
	hide.Hide();
	if(GetSaveFileName(&ofn)) {
		if(strType) {
			switch(ofn.nFilterIndex) {
			case 2:
				*strType=L".bmp24";
				break;
			case 3:
				*strType=L".bmp32";
				break;
			case 4:
				*strType=L".png";
				break;
			case 5:
				*strType=L".pngo";
				break;
			case 6:
				*strType=L".jpg";
				break;
			case 7:
				*strType=L".gif";
				break;
			case 1:
			default:
				strExt=PathFindExtension(strExportFile);
				if(wcsicmp(strExt,L".bmp")!=0
						&& wcsicmp(strExt,L".png")!=0
						&& wcsicmp(strExt,L".jpg")!=0
						&& wcsicmp(strExt,L".gif")!=0) {
					MessageBox(hwndMain,L"Unknown File Type",L"KnobMan",MB_OK);
					return NULL;
				}
				*strType=strExportFile;
			}
		}
		hide.Restore();
		return strExportFile;
	}
	else {
		hide.Restore();
		return NULL;
	}
}
/*
wchar_t *Control::GetProjFileName(HWND hwnd,int c) {
	static wchar_t wstr[MAX_PATH];
	wcscpy_s(wstr,MAX_PATH,wstrCurrentProject);
	memset(&ofn,0,sizeof(ofn));
	ofn.lStructSize=sizeof(ofn);
	ofn.hwndOwner=hwnd;
	ofn.hInstance=hinstMain;
	ofn.lpstrFilter=L"*.knob\0*.knob\0*.*\0*.*\0";
	ofn.lpstrFile=wstr;
	ofn.nFilterIndex=1;
	ofn.lpstrInitialDir=wstrCurrentDir;
	ofn.lpstrDefExt=L"knob";
	ofn.nMaxFile=MAX_PATH;
	ofn.Flags=OFN_HIDEREADONLY|OFN_EXPLORER;
	CoInitialize(NULL);
	if(c=='S') {
		ofn.Flags|=OFN_OVERWRITEPROMPT;
		if(GetSaveFileName(&ofn)) {
			theRecentFiles->Add(wstr);
			return wstr;
		}
		else
			return NULL;
	}
	if(c=='L') {
		ofn.Flags|=OFN_FILEMUSTEXIST;
		if(GetOpenFileName(&ofn)) {
			theRecentFiles->Add(wstr);
			return wstr;
		}
		else
			return NULL;
	}
	return NULL;
}
*/
wchar_t *GetProjFileName(HWND hwnd,int c,bool bIsLib) {
	OPENFILENAME ofn;
	static wchar_t strTemp[MAX_PATH];
	HideTools hide;
	memset(&ofn,0,sizeof(ofn));
	ofn.lStructSize=sizeof(ofn);
	ofn.hwndOwner=hwnd;
	ofn.hInstance=hinstMain;
	ofn.lpstrFile=strTemp;
	ofn.nFilterIndex=1;
	if(bIsLib) {
		if(c=='L')
			ofn.lpstrTitle=theLang->GetID(MSG_DIALOGIMPORTFROMLIB);
		else
			ofn.lpstrTitle=theLang->GetID(MSG_DIALOGEXPORTTOLIB);
		ofn.lpstrFilter=L"*.sklib\0*.sklib\0*.*\0*.*\0";
		strTemp[0]=0;
		ofn.lpstrInitialDir=theApp->strLibDir;
		ofn.lpstrDefExt=L"sklib";
	}
	else {
		if(c=='L')
			ofn.lpstrTitle=theLang->GetID(MSG_DIALOGOPENPROJECT);
		else
			ofn.lpstrTitle=theLang->GetID(MSG_DIALOGSAVEPROJECTAS);
		ofn.lpstrFilter=L"*.skin\0*.skin\0*.*\0*.*\0";
		wcscpy(strTemp,theApp->strCurrentProject);
		ofn.lpstrInitialDir=theApp->strDocDir;
		ofn.lpstrDefExt=L"skin";
	}
	ofn.nMaxFile=MAX_PATH;
	ofn.Flags=OFN_HIDEREADONLY|OFN_EXPLORER;
	hide.Hide();
	CoInitialize(NULL);
	if(c=='S') {
		ofn.Flags|=OFN_OVERWRITEPROMPT;
		if(GetSaveFileName(&ofn)) {
			hide.Restore();
			return strTemp;
		}
		else {
			hide.Restore();
			return NULL;
		}
	}
	if(c=='L') {
		ofn.Flags|=OFN_FILEMUSTEXIST;
		if(GetOpenFileName(&ofn)) {
			hide.Restore();
			return strTemp;
		}
		else {
			hide.Restore();
			return NULL;
		}
	}
	return NULL;
}

void InvertRect(HWND hwnd,int x1,int y1,int x2,int y2) {
	RECT rc;
	HDC hdc=GetDC(hwnd);
	int minx=min(x1,x2);
	int maxx=max(x1,x2);
	int miny=min(y1,y2);
	int maxy=max(y1,y2);
	rc.left=minx,rc.top=miny,rc.right=maxx,rc.bottom=miny+1;
	InvertRect(hdc,&rc);
	rc.top=maxy,rc.bottom=maxy+1;
	InvertRect(hdc,&rc);
	rc.left=minx,rc.top=miny,rc.right=minx+1,rc.bottom=maxy;
	InvertRect(hdc,&rc);
	rc.left=maxx,rc.right=maxx+1;
	InvertRect(hdc,&rc);
	ReleaseDC(hwnd,hdc);
}
/*
	wchar_t wstr[MAX_PATH];
	OPENFILENAME ofn;
	wcscpy(wstr,wstrCommand);
	ZeroMemory(&ofn,sizeof(ofn));
	ofn.lStructSize=sizeof(ofn);
	ofn.hwndOwner=hwndMain;
	ofn.hInstance=hinstMain;
	ofn.lpstrFilter=L"*.*\0*.*\0";
	ofn.lpstrFile=wstr;
	ofn.nFilterIndex=1;
	ofn.lpstrInitialDir=L"";
	ofn.lpstrDefExt=L"";
	ofn.nMaxFile=MAX_PATH;
	ofn.lpstrTitle=wstrTitle;
	ofn.Flags=OFN_HIDEREADONLY|OFN_EXPLORER;
	if(GetOpenFileName(&ofn)) {
		wcscpy(wstrCommand,wstr);
	}

*/
void ColorTool::LoadPal(void) {
	static wchar_t wstrFile[MAX_PATH];
	OPENFILENAMEW ofnw;
	ZeroMemory(&ofnw,sizeof(ofnw));
	ofnw.lStructSize=sizeof(ofnw);
	ofnw.hwndOwner=theApp->pal->hwnd;
	ofnw.hInstance=hinstMain;
	ofnw.lpstrFilter=L"SkinMan Palette (*.skpal)\0*.skpal\0All Files(*.*)\0*.*\0\0";
	ofnw.lpstrFile=wstrFile;
	ofnw.nFilterIndex=1;
	ofnw.lpstrInitialDir=theApp->strPalDir;
	ofnw.lpstrDefExt=L"skpal";
	ofnw.nMaxFile=MAX_PATH;
	ofnw.lpstrTitle=L"Load Palette";
	ofnw.Flags=OFN_HIDEREADONLY|OFN_EXPLORER;
	if(GetOpenFileNameW(&ofnw)) {
		PrivateProfileReader ppr(wstrFile);
		::LoadPal(&ppr);
	}
}
void ColorTool::SavePal(void) {
	static wchar_t wstrFile[MAX_PATH];
	OPENFILENAMEW ofnw;
	ZeroMemory(&ofnw,sizeof(ofnw));
	ofnw.lStructSize=sizeof(ofnw);
	ofnw.lpstrInitialDir=theApp->strPalDir;
	ofnw.lpstrFile=wstrFile;
	ofnw.nMaxFile=MAX_PATH;
	ofnw.hwndOwner=theApp->pal->hwnd;
	ofnw.lpstrFilter=L"SkinMan Palette (*.skpal)\0*.skpal\0All Files(*.*)\0*.*\0\0";
	ofnw.lpstrDefExt=L"skpal";
	ofnw.nFilterIndex=1;
	if(GetSaveFileNameW(&ofnw)) {
		PrivateProfileWriter ppw(wstrFile,L"wb");
		::SavePal(&ppw);
	}
}
void ColorTool::UpdateByKey(int id) {
	int r,g,b;
	switch(id) {
	case IDC_H:
	case IDC_L:
	case IDC_S:
		hueTab[iSel]=GetDlgItemInt(hwnd,IDC_H,NULL,false);
		lumTab[iSel]=GetDlgItemInt(hwnd,IDC_L,NULL,false);
		satTab[iSel]=GetDlgItemInt(hwnd,IDC_S,NULL,false);
		rgbTab[iSel]=HLStoRGB(hueTab[iSel],lumTab[iSel],satTab[iSel]);
		break;
	case IDC_R:
	case IDC_G:
	case IDC_B:
		r=GetDlgItemInt(hwnd,IDC_R,NULL,false);
		g=GetDlgItemInt(hwnd,IDC_G,NULL,false);
		b=GetDlgItemInt(hwnd,IDC_B,NULL,false);
		rgbTab[iSel]=(r<<16)|(g<<8)|b;
		RGBtoHLS(rgbTab[iSel],&hueTab[iSel],&lumTab[iSel],&satTab[iSel]);
		break;
	}
	UpdateBar();
	InvalidateRect(hwndMap,NULL,false);
	InvalidateRect(hwndBar,NULL,false);
	Update();
}
void ColorTool::UpdateBar(void) {
	int xx,yy;
	Rect rcf;
	DWORD *pdw;
	BitmapData bmpd;
	rcf.X=rcf.Y=0;
	rcf.Width=10;
	rcf.Height=240;
	bmpBar->LockBits(&rcf,ImageLockModeRead|ImageLockModeWrite,PixelFormat32bppARGB,&bmpd);
	for(yy=0;yy<240;++yy) {
		pdw=(DWORD*)((char*)bmpd.Scan0+bmpd.Stride*yy);
		for(xx=0;xx<10;++xx) {
			*pdw=0xff000000|HLStoRGB(hueTab[iSel],240-yy,satTab[iSel]);
			++pdw;
		}
	}
	bmpBar->UnlockBits(&bmpd);
}
void ColorTool::MouseMoveBar(int x,int y) {
	RECT rc;
	GetClientRect(hwndBar,&rc);
	lumTab[iSel]=240-y*240/rc.bottom;
	lumTab[iSel]=max(0,min(240,lumTab[iSel]));
	rgbTab[iSel]=HLStoRGB(hueTab[iSel],lumTab[iSel],satTab[iSel]);

	SetDlgItemInt(hwnd,IDC_H,hueTab[iSel],false);
	SetDlgItemInt(hwnd,IDC_L,lumTab[iSel],false);
	SetDlgItemInt(hwnd,IDC_S,satTab[iSel],false);
	SetDlgItemInt(hwnd,IDC_R,(rgbTab[iSel]>>16)&0xff,false);
	SetDlgItemInt(hwnd,IDC_G,(rgbTab[iSel]>>8)&0xff,false);
	SetDlgItemInt(hwnd,IDC_B,rgbTab[iSel]&0xff,false);

	InvalidateRect(hwndBar,NULL,false);
	Update(false);
}
void ColorTool::MouseMoveMap(int x,int y) {
	RECT rc;
	GetClientRect(hwndMap,&rc);
	hueTab[iSel]=x*240/rc.right;
	satTab[iSel]=240-y*240/rc.bottom;
	hueTab[iSel]=max(0,min(240,hueTab[iSel]));
	satTab[iSel]=max(0,min(240,satTab[iSel]));
	rgbTab[iSel]=HLStoRGB(hueTab[iSel],lumTab[iSel],satTab[iSel]);
	UpdateBar();

	SetDlgItemInt(hwnd,IDC_H,hueTab[iSel],false);
	SetDlgItemInt(hwnd,IDC_L,lumTab[iSel],false);
	SetDlgItemInt(hwnd,IDC_S,satTab[iSel],false);
	SetDlgItemInt(hwnd,IDC_R,(rgbTab[iSel]>>16)&0xff,false);
	SetDlgItemInt(hwnd,IDC_G,(rgbTab[iSel]>>8)&0xff,false);
	SetDlgItemInt(hwnd,IDC_B,rgbTab[iSel]&0xff,false);

	InvalidateRect(hwndMap,NULL,false);
	InvalidateRect(hwndBar,NULL,false);
	Update(false);
}
LRESULT CALLBACK ColorTool::wndprocSubclass(HWND hwnd,UINT msg,WPARAM wparam,LPARAM lparam) {
	static ColorTool *p=(ColorTool*)(LONG_PTR)GetWindowLongPtr(hwnd,GWLP_USERDATA);
	HMENU hmenu;
	POINT pt;
	int id;
	switch(msg) {
	case WM_RBUTTONDOWN:
		hmenu=CreatePopupMenu();
		GetCursorPos(&pt);
		AppendMenu(hmenu,MF_STRING,0x10,(wchar_t*)theLang->GetID(MSG_REGISTERCOLOR));
		switch(TrackPopupMenu(hmenu,TPM_LEFTALIGN|TPM_TOPALIGN|TPM_RETURNCMD,pt.x,pt.y,0,hwnd,NULL)) {
		case 0x10:
			id=GetWindowLong(hwnd,GWL_ID);
			colCust[id-IDC_PAL0]=DwToRgb(p->rgbTab[p->iSel]);
			InvalidateRect(hwnd,NULL,false);
			break;
		}
		DestroyMenu(hmenu);
		break;
	}
	return (CallWindowProc(p->wndprocOrg, hwnd, msg, wparam, lparam));
}
LRESULT CALLBACK ColorTool::wndprocBar(HWND hwnd,UINT msg,WPARAM wparam,LPARAM lparam) {
	PAINTSTRUCT ps;
	HDC hdc;
	RECT rc;
	Rect rcf;
	int x,y;
	Graphics *g;
	static ColorTool *p;
	static bool bDragging=false;
	switch(msg) {
	case WM_CREATE:
		p=(ColorTool*)((CREATESTRUCT*)lparam)->lpCreateParams;
		return TRUE;
	case WM_PAINT:
		hdc=BeginPaint(hwnd,&ps);
		GetClientRect(hwnd,&rc);
		g=new Graphics(hdc);
		rc.right=(rc.left+rc.right)/2;
		rcf.X=rc.right;
		rcf.Y=0;
		rcf.Height=rc.bottom;
		rcf.Width=rc.right;
		g->DrawImage(p->bmpBar,0,0,rc.right,rc.bottom);
		g->FillRectangle(&SolidBrush(Color::White),rcf);
		rcf.Y=(240-p->lumTab[p->iSel])*rc.bottom/240;
		rcf.Height=1;
		g->FillRectangle(&SolidBrush(Color::Black),rcf);
		for(x=0;x<4;++x) {
			rcf.X+=2;
			rcf.Y-=1;
			rcf.Width-=2;
			rcf.Height+=2;
			g->FillRectangle(&SolidBrush(Color::Black),rcf);
		}
		delete g;
		EndPaint(hwnd,&ps);
		break;
	case WM_LBUTTONDOWN:
		theJournal->Record();
		bDragging=true;
		SetCapture(hwnd);
		GetXY(lparam,&x,&y);
		p->MouseMoveBar(x,y);
		break;
	case WM_LBUTTONUP:
		if(bDragging) {
			bDragging=false;
			GetXY(lparam,&x,&y);
			p->MouseMoveBar(x,y);
			ReleaseCapture();
		}
		break;
	case WM_MOUSEMOVE:
		if(bDragging) {
			GetXY(lparam,&x,&y);
			p->MouseMoveBar(x,y);
		}
		SetCursor(hcurArrow);
		break;
	default:
		return DefWindowProc(hwnd, msg, wparam, lparam);
	}
	return 0;
}
LRESULT CALLBACK ColorTool::wndprocMap(HWND hwnd,UINT msg,WPARAM wparam,LPARAM lparam) {
	PAINTSTRUCT ps;
	HDC hdc;
	RECT rc;
	Rect rcf;
	Graphics *g;
	int x,y;
	static ColorTool *p;
	static bool bDragging=false;
	switch(msg) {
	case WM_CREATE:
		p=(ColorTool*)((CREATESTRUCT*)lparam)->lpCreateParams;
		return TRUE;
	case WM_LBUTTONDOWN:
		bDragging=true;
		theJournal->Record();
		SetCapture(hwnd);
		GetXY(lparam,&x,&y);
		p->MouseMoveMap(x,y);
		break;
	case WM_MOUSEMOVE:
		if(bDragging) {
			GetXY(lparam,&x,&y);
			p->MouseMoveMap(x,y);
		}
		SetCursor(hcurArrow);
		break;
	case WM_LBUTTONUP:
		if(bDragging) {
			bDragging=false;
			GetXY(lparam,&x,&y);
			p->MouseMoveMap(x,y);
			ReleaseCapture();
		}
		break;
	case WM_PAINT:
		hdc=BeginPaint(hwnd,&ps);
		GetClientRect(hwnd,&rc);
		g=new Graphics(hdc);
		x=p->hueTab[p->iSel]*rc.right/240;
		y=(240-p->satTab[p->iSel])*rc.bottom/240;
		g->DrawImage(p->bmpMap,0,0,rc.right,rc.bottom);
		rcf.X=x;
		rcf.Y=0;
		rcf.Width=1;
		rcf.Height=rc.bottom;
		g->FillRectangle(&SolidBrush(Color::Black),rcf);
		rcf.X=0;
		rcf.Y=y;
		rcf.Width=rc.right;
		rcf.Height=1;
		g->FillRectangle(&SolidBrush(Color::Black),rcf);
		rcf.X=x-8;
		rcf.Y=y-8;
		rcf.Width=16;
		rcf.Height=16;
		g->DrawEllipse(&Pen(Color::White,4),rcf);
		rcf.X=x-8;
		rcf.Y=y-8;
		rcf.Width=16;
		rcf.Height=16;
		g->DrawEllipse(&Pen(Color::Black,2),rcf);
		delete g;
		EndPaint(hwnd,&ps);
		break;
	default:
		return DefWindowProc(hwnd, msg, wparam, lparam);
	}
	return 0;
}
BOOL CALLBACK ColorTool::dlgprocFrame(HWND hwnd,UINT uMsg,WPARAM wparam,LPARAM lparam) {
	static ColorTool *p=0;
	static WNDPROC wndprocOrg;
	DWORD dw;
	int x,y,r,g,b;
	RECT rc;
	POINT pt;
	COLORREF col;
	static HDC hdcMem;
	static HBITMAP hbmpMem,hbmpMemOld;
	static bool bPipetteMode=false;
	HDC hdc;
	switch(uMsg) {
	case WM_INITDIALOG:
		p=(ColorTool*)lparam;
		p->hwndMap=CreateWindowEx(0,L"Color-Map",L"",WS_CHILD|WS_VISIBLE,0,0,20,20,GetDlgItem(hwnd,IDC_COLORMAP),NULL,hinstMain,(void*)p);
		p->hwndBar=CreateWindowEx(0,L"Color-Bar",L"",WS_CHILD|WS_VISIBLE,0,0,20,20,GetDlgItem(hwnd,IDC_COLORBAR),NULL,hinstMain,(void*)p);
		p->hwndColor=GetDlgItem(hwnd,IDC_COLOR);
		SetTimer(hwnd,1,200,NULL);
		SetFocus(0);
		return TRUE;
	case WM_CLOSE:
		p->Show(0);
		theApp->CheckMenu();
		break;
	case WM_DESTROY:
		KillTimer(hwnd,1);
		p->hwnd=NULL;
		p=0;
		break;
	case WM_MOUSEWHEEL:
		SendMessage(theScreen->hwnd,WM_MOUSEWHEEL,wparam,lparam);
		break;
	case WM_LBUTTONDOWN:
		GetCursorPos(&pt);
		GetWindowRect(GetDlgItem(hwnd,IDC_PIPETTE),&rc);
		if(PtInRect(&rc,pt)) {
			SetCursor(hcurPipette);
			SetCapture(hwnd);
			bPipetteMode=true;
		}
		break;
	case WM_LBUTTONUP:
		if(bPipetteMode) {
			bPipetteMode=false;
			ReleaseCapture();
			p->Update();
		}
		SetFocus(p->hwndMap);
		break;
	case WM_TIMER:
		if(bPipetteMode) {
			SetCursor(hcurPipette);
			GetXY(lparam,&x,&y);
			GetCursorPos(&pt);
			hdc=GetDC(0);
			col=GetPixel(hdc,pt.x,pt.y);
			ReleaseDC(0,hdc);
			r=GetRValue(col);
			g=GetGValue(col);
			b=GetBValue(col);
			p->rgbTab[p->iSel]=(r<<16)|(g<<8)|b;
			RGBtoHLS(p->rgbTab[p->iSel],&p->hueTab[p->iSel],&p->lumTab[p->iSel],&p->satTab[p->iSel]);
			p->UpdateBar();
			InvalidateRect(p->hwndMap,NULL,false);
			InvalidateRect(p->hwndBar,NULL,false);
			SetDlgItemInt(p->hwnd,IDC_H,p->hueTab[p->iSel],false);
			SetDlgItemInt(p->hwnd,IDC_L,p->lumTab[p->iSel],false);
			SetDlgItemInt(p->hwnd,IDC_S,p->satTab[p->iSel],false);
			SetDlgItemInt(p->hwnd,IDC_R,(p->rgbTab[p->iSel]>>16)&0xff,false);
			SetDlgItemInt(p->hwnd,IDC_G,(p->rgbTab[p->iSel]>>8)&0xff,false);
			SetDlgItemInt(p->hwnd,IDC_B,p->rgbTab[p->iSel]&0xff,false);
			p->Update(false);
		}
		break;
	case WM_DRAWITEM:
		if(wparam==IDC_COLOR) {
			RECT rc;
			HBRUSH hbr;
			DWORD dw;
			DRAWITEMSTRUCT *di=(DRAWITEMSTRUCT*)lparam;
			HDC hdc=di->hDC;
			dw=p->rgbTab[p->iSel];
			hbr=CreateSolidBrush(DwToRgb(dw));
			rc=di->rcItem;
			FillRect(hdc,&rc,hbr);
			DeleteObject((HGDIOBJ)hbr);
			return TRUE;
		}
		if(wparam>=IDC_PAL0 && wparam<=IDC_PAL23) {
			UINT uiState;
			RECT rc;
			DRAWITEMSTRUCT *di=(DRAWITEMSTRUCT*)lparam;
			HDC hdc=di->hDC;
			HBRUSH hbr;
			DWORD dw;
			dw=colCust[wparam-IDC_PAL0];
			hbr=CreateSolidBrush(dw);
			if(di->itemState&ODS_SELECTED)
				uiState=DFCS_BUTTONPUSH | DFCS_PUSHED;
			else
				uiState=DFCS_BUTTONPUSH;
			DrawFrameControl(hdc, &di->rcItem, DFC_BUTTON, uiState);
			rc=di->rcItem;
			InflateRect(&rc,-3,-3);
			FillRect(hdc,&rc,hbr);
			InflateRect(&rc,-2,-2);
			if(di->itemState&ODS_FOCUS)
				DrawFocusRect(hdc,&rc);
			DeleteObject((HGDIOBJ)hbr);
			return TRUE;
		}
		break;
	case WM_USER_ENDEDIT:
		theJournal->Record();
		switch(LOWORD(wparam)) {
		case IDC_R:
		case IDC_G:
		case IDC_B:
			r=GetDlgItemInt(hwnd,IDC_R,0,false);
			g=GetDlgItemInt(hwnd,IDC_G,0,false);
			b=GetDlgItemInt(hwnd,IDC_B,0,false);
			p->rgbTab[p->iSel]=(r<<16)|(g<<8)|b;
			RGBtoHLS(p->rgbTab[p->iSel],&p->hueTab[p->iSel],&p->lumTab[p->iSel],&p->satTab[p->iSel]);
			break;
		case IDC_H:
		case IDC_L:
		case IDC_S:
			p->hueTab[p->iSel]=GetDlgItemInt(hwnd,IDC_H,0,false);
			p->lumTab[p->iSel]=GetDlgItemInt(hwnd,IDC_L,0,false);
			p->satTab[p->iSel]=GetDlgItemInt(hwnd,IDC_S,0,false);
			p->rgbTab[p->iSel]=HLStoRGB(p->hueTab[p->iSel],p->lumTab[p->iSel],p->satTab[p->iSel]);
			break;
		}
		p->UpdateBar();
		InvalidateRect(p->hwndMap,NULL,false);
		InvalidateRect(p->hwndBar,NULL,false);
		p->Update();
		break;
	case WM_COMMAND:
		switch(wparam) {
		case IDC_PAL0:
		case IDC_PAL1:
		case IDC_PAL2:
		case IDC_PAL3:
		case IDC_PAL4:
		case IDC_PAL5:
		case IDC_PAL6:
		case IDC_PAL7:
		case IDC_PAL8:
		case IDC_PAL9:
		case IDC_PAL10:
		case IDC_PAL11:
		case IDC_PAL12:
		case IDC_PAL13:
		case IDC_PAL14:
		case IDC_PAL15:
		case IDC_PAL16:
		case IDC_PAL17:
		case IDC_PAL18:
		case IDC_PAL19:
		case IDC_PAL20:
		case IDC_PAL21:
		case IDC_PAL22:
		case IDC_PAL23:
			theJournal->Record();
			dw=colCust[wparam-IDC_PAL0];
			p->SetColor(((dw&0xff)<<16)|(dw&0xff00)|((dw&0xff0000)>>16));
			break;
		case IDC_COLFILL1:
			p->Set(0);
			break;
		case IDC_COLFILL2:
			p->Set(1);
			break;
		case IDC_COLBORDER:
			p->Set(2);
			break;
		case IDC_COLDSHADOW:
			p->Set(3);
			break;
		case IDC_COLISHADOW:
			p->Set(4);
			break;
		case IDC_LOADPAL:
			p->LoadPal();
			break;
		case IDC_SAVEPAL:
			p->SavePal();
			break;
		}
		break;
	}
	return FALSE;
}
ColorTool::ColorTool(HWND hwndParent,int xInit,int yInit) {
	WNDCLASSEX wcexMap,wcexBar;
	DWORD *pdw;
	BitmapData bmpd;
	Rect rc;
	RECT rcClient;
	int i,x,y;
	iSel=0;
	hwnd=0;
	xWin=xInit;
	yWin=yInit;
	rgbTab[0]=0xffff8080;
	rgbTab[1]=0xff804040;
	rgbTab[2]=0xff000000;
	rgbTab[3]=0xff000000;
	rgbTab[4]=0xff000000;
	for(i=0;i<5;++i)
		RGBtoHLS(rgbTab[i],&hueTab[i],&lumTab[i],&satTab[i]);
	bmpMap=new Bitmap(240,240);
	rc.X=rc.Y=0;
	rc.Width=rc.Height=240;
	bmpMap->LockBits(&rc,ImageLockModeRead|ImageLockModeWrite,PixelFormat32bppARGB,&bmpd);
	for(y=0;y<240;++y) {
		pdw=(DWORD*)((char*)bmpd.Scan0+bmpd.Stride*y);
		for(x=0;x<240;++x) {
			*pdw=0xff000000|HLStoRGB(x,128,240-y);
			++pdw;
		}
	}
	bmpMap->UnlockBits(&bmpd);
	bmpBar=new Bitmap(10,240);
	ZeroMemory(&wcexMap,sizeof(wcexMap));
	ZeroMemory(&wcexBar,sizeof(wcexBar));
	wcexMap.cbSize = wcexBar.cbSize = sizeof(WNDCLASSEX); 
	wcexMap.style = wcexBar.style = CS_HREDRAW | CS_VREDRAW;
	wcexMap.lpfnWndProc	= (WNDPROC)wndprocMap;
	wcexBar.lpfnWndProc = (WNDPROC)wndprocBar;
	wcexMap.hInstance = wcexBar.hInstance = hinstMain;
	wcexMap.lpszClassName = L"Color-Map";
	wcexBar.lpszClassName = L"Color-Bar";
	RegisterClassEx(&wcexMap);
	RegisterClassEx(&wcexBar);
	hwnd=CreateDialogParam(hinstMain,MAKEINTRESOURCE(IDD_COLOR),hwndParent,dlgprocFrame,(LPARAM)this);

	for(i=IDC_PAL0;i<=IDC_PAL23;++i) {
		SetWindowLongPtr(GetDlgItem(hwnd,i),GWLP_USERDATA,(LONG)(LONG_PTR)this);
		wndprocOrg=(WNDPROC)(LONG_PTR)SetWindowLongPtr(GetDlgItem(hwnd,i), GWLP_WNDPROC, (LONG)(LONG_PTR)wndprocSubclass);
	}
	ShowWindow(hwnd,SW_NORMAL);
	GetClientRect(GetDlgItem(hwnd,IDC_COLORMAP),&rcClient);
	MoveWindow(hwndMap,0,0,rcClient.right,rcClient.bottom,true);
	GetClientRect(GetDlgItem(hwnd,IDC_COLORBAR),&rcClient);
	MoveWindow(hwndBar,0,0,rcClient.right,rcClient.bottom,true);
	new DoubleEdit(GetDlgItem(hwnd,IDC_R),Int);
	new DoubleEdit(GetDlgItem(hwnd,IDC_G),Int);
	new DoubleEdit(GetDlgItem(hwnd,IDC_B),Int);
	new DoubleEdit(GetDlgItem(hwnd,IDC_H),Int);
	new DoubleEdit(GetDlgItem(hwnd,IDC_L),Int);
	new DoubleEdit(GetDlgItem(hwnd,IDC_S),Int);
	SetWindowPos(hwnd,0,xWin,yWin,0,0,SWP_NOZORDER|SWP_NOSIZE);
	Show(NULL,0);
}
ColorTool::~ColorTool(void) {
	DestroyWindow(hwnd);
	delete bmpMap;
	delete bmpBar;
}
void ColorTool::Show(int i) {
	if(i)
		ShowWindow(hwnd,SW_NORMAL);
	else
		ShowWindow(hwnd,SW_HIDE);
}
int ColorTool::IsShow(void) {
	return IsWindowVisible(hwnd);
}
void ColorTool::Show(Primitive *pr,int iSelect) {
	SetForegroundWindow(hwnd);
	iSel=iSelect;
	Set(pr);
	Show(1);
}
void ColorTool::Set(Primitive *pr) {
	int i;
	if(pr) {
		prTarget=pr;
		rgbTab[0]=pr->dwColor1;
		rgbTab[1]=pr->dwColor2;
		rgbTab[2]=pr->dwColor3;
		rgbTab[3]=pr->dwShadowColor;
		rgbTab[4]=pr->dwIShadowColor;
		for(i=0;i<5;++i)
			RGBtoHLS(rgbTab[i],&hueTab[i],&lumTab[i],&satTab[i]);
	}
	else {
		prTarget=NULL;
		for(i=0;i<5;++i) {
			rgbTab[i]=hueTab[i]=lumTab[i]=satTab[i]=0;
		}
	}
	Set(iSel);
}
void ColorTool::Set(int i) {
	wchar_t wstr[16];
	iSel=i;
	if(hwnd) {
		UpdateBar();
		InvalidateRect(hwndMap,NULL,false);
		InvalidateRect(hwndBar,NULL,false);
		SetDlgItemInt(hwnd,IDC_H,hueTab[iSel],false);
		SetDlgItemInt(hwnd,IDC_L,lumTab[iSel],false);
		SetDlgItemInt(hwnd,IDC_S,satTab[iSel],false);
		SetDlgItemInt(hwnd,IDC_R,(rgbTab[iSel]>>16)&0xff,false);
		SetDlgItemInt(hwnd,IDC_G,(rgbTab[iSel]>>8)&0xff,false);
		SetDlgItemInt(hwnd,IDC_B,rgbTab[iSel]&0xff,false);
		CheckButton(hwnd,IDC_COLFILL1,iSel==0);
		CheckButton(hwnd,IDC_COLFILL2,iSel==1);
		CheckButton(hwnd,IDC_COLBORDER,iSel==2);
		CheckButton(hwnd,IDC_COLDSHADOW,iSel==3);
		CheckButton(hwnd,IDC_COLISHADOW,iSel==4);
		swprintf(wstr,L"%02x%02x%02x",(rgbTab[iSel])&0xff,(rgbTab[iSel]>>8)&0xff,(rgbTab[iSel]>>16)&0xff);
		SetWindowText(hwndColor,wstr);
		InvalidateRect(hwndColor,NULL,false);
	}
}
void ColorTool::SetColor(DWORD dw) {
	rgbTab[iSel]=dw;
	RGBtoHLS(rgbTab[iSel],&hueTab[iSel],&lumTab[iSel],&satTab[iSel]);
	Set(iSel);
	Update();
}
void ColorTool::Update(bool bAdjust) {
	wchar_t wstr[16];
	PrimitivePtrList *prl;
	if(theTree->prlCurrent.next) {
		for(prl=&theTree->prlCurrent;prl->next;prl=prl->next) {
			if(prl->bSel)
				theApp->UpdateColor(prl->pr,iSel,rgbTab[iSel],bAdjust,false);
		}
	}
	else
		theApp->UpdateColor(theProp->prCurrent,iSel,rgbTab[iSel],bAdjust,false);
	swprintf(wstr,L"%02x%02x%02x",(rgbTab[iSel])&0xff,(rgbTab[iSel]>>8)&0xff,(rgbTab[iSel]>>16)&0xff);
	SetWindowText(hwndColor,wstr);
	InvalidateRect(hwndColor,NULL,false);
	theProp->SetupColor();
}
void GetVirtualScreenRect(RECT *prc) {
	prc->left=GetSystemMetrics(SM_XVIRTUALSCREEN);
	prc->top=GetSystemMetrics(SM_YVIRTUALSCREEN);
	prc->right=prc->left+GetSystemMetrics(SM_CXVIRTUALSCREEN);
	prc->bottom=prc->top+GetSystemMetrics(SM_CYVIRTUALSCREEN);
}
void LimitMove(RECT *prc,RECT *prcLim) {
	if(prc->left>=prcLim->right-64)
		OffsetRect(prc,prcLim->right-64-prc->left,0);
	if(prc->top>=prcLim->bottom-64)
		OffsetRect(prc,0,prcLim->bottom-64-prc->top);
	if(prc->right<=prcLim->left+64)
		OffsetRect(prc,prcLim->left+64-prc->right,0);
	if(prc->bottom<=prcLim->top+64)
		OffsetRect(prc,0,prcLim->top+64-prc->bottom);
}
App::App(HINSTANCE hinstInit,wchar_t *strCmdLine) {
	OSVERSIONINFO ovi;
	HRESULT hr;
	RECT rcVirtualScr;
	wchar_t *pstr,*pstr2,str[MAX_PATH];
	int i;
	hinst=hinstInit;
	hwndColor=NULL;
	GetVirtualScreenRect(&rcVirtualScr);
	theRecentFiles=new RecentFiles();
	ovi.dwOSVersionInfoSize=sizeof(ovi);
	GetVersionEx(&ovi);
	GetModuleFileName(NULL,strModule,MAX_PATH);
	wcscpy(strModuleDir,strModule);
	PathRemoveFileSpec(strModuleDir);
	wcscpy(strDocDir,strModuleDir);
	wcscpy(strImageDir,strModuleDir);
	wcscpy(strLibDir,strModuleDir);
	wcscpy(strPalDir,strModuleDir);
	wcscpy(strDefaultKnob,strModuleDir);
	wcscat(strDefaultKnob,L"\\default.knob");
	if(ovi.dwMajorVersion<6) {
		wcscpy(strAppDataDir,strModuleDir);
		wcscpy(strImageDir,strModuleDir);
	}
	else {
		hr = SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, strAppDataDir);
		if(!SUCCEEDED(hr))
			wcscpy(strAppDataDir,strModuleDir);
		else {
			wcscat(strAppDataDir,L"\\g200kg");
			if(!PathIsDirectory(strAppDataDir))
				_wmkdir(strAppDataDir);
			wcscat(strAppDataDir,L"\\SkinMan");
			if(!PathIsDirectory(strAppDataDir))
				_wmkdir(strAppDataDir);
		}
	}
	wcscpy(strLatestBackup,strAppDataDir);
	wcscat(strLatestBackup,L"\\$Backup$.skin");
	wcscpy(strClipboard,strAppDataDir);
	wcscat(strClipboard,L"\\$Clipboard$");
	wcscpy(strClipboard2,strAppDataDir);
	wcscat(strClipboard2,L"\\$Clipboard2$");
	wcscpy(strThumbnail,strAppDataDir);
	wcscat(strThumbnail,L"\\$Thumbnail$.png");
	wcscpy(strIniFile,strAppDataDir);
	wcscat(strIniFile,L"\\SkinMan.ini");
	wcscpy(strTempDir,strAppDataDir);
	wcscat(strTempDir,L"\\Temp");
	wcscpy(strTextureDir,strModuleDir);
	wcscat(strTextureDir,L"\\Texture");
	wcscpy(strLangFile,strModuleDir);
	wsprintf(str,L"\\Language\\%d.txt",lang);
	wcscat(strLangFile,str);
	wcscpy(strTempTextureDir,strTempDir);
	wcscat(strTempTextureDir,L"\\Texture");
	wcscpy(str,strCmdLine);
	if(strCmdLine[0]=='\"') {
		for(pstr=strCmdLine+1,pstr2=str;*pstr;++pstr,++pstr2) {
			*pstr2=*pstr;
			if(*pstr=='\"')
				break;
		}
		*pstr2=0;
	}
	wcscpy(strCurrentProject,str);
	if(!PathIsDirectory(strTextureDir))
		_wmkdir(strTextureDir);
	if(!PathIsDirectory(strTempDir))
		_wmkdir(strTempDir);
	if(!PathIsDirectory(strTempTextureDir))
		_wmkdir(strTempTextureDir);
	PrivateProfileReader ppr(strIniFile);
	ppr.SetSection(L"Window");
	rcWin.left=ppr.ReadInt(L"X",200);
	rcWin.top=ppr.ReadInt(L"Y",0);
	rcWin.right=rcWin.left+ppr.ReadInt(L"Width",750);
	rcWin.bottom=rcWin.top+ppr.ReadInt(L"Height",640);
	LimitMove(&rcWin,&rcVirtualScr);
	rcTree.left=ppr.ReadInt(L"TreeX",0);
	rcTree.top=ppr.ReadInt(L"TreeY",0);
	rcTree.right=rcTree.left+ppr.ReadInt(L"TreeWidth",220);
	rcTree.bottom=rcTree.top+ppr.ReadInt(L"TreeHeight",300);
	LimitMove(&rcTree,&rcVirtualScr);
	rcProp.left=ppr.ReadInt(L"PropX",950);
	rcProp.top=ppr.ReadInt(L"PropY",0);
	rcProp.right=rcProp.left+ppr.ReadInt(L"PropWidth",220);
	rcProp.bottom=rcProp.top+ppr.ReadInt(L"PropHeight",600);
	LimitMove(&rcProp,&rcVirtualScr);
	rcColor.left=rcColor.right=ppr.ReadInt(L"ColorX",300);
	rcColor.top=rcColor.bottom=ppr.ReadInt(L"ColorY",300);
	rcColor.right=rcColor.left+ppr.ReadInt(L"ColorWidth",200);
	rcColor.bottom=rcColor.top+ppr.ReadInt(L"ColorHeight",200);
	LimitMove(&rcColor,&rcVirtualScr);
	iAlwaysFront=ppr.ReadInt(L"AlwaysFront",1);
	ppr.SetSection(L"Screen");
	iDefaultGridX=ppr.ReadInt(L"GridX",8);
	iDefaultGridY=ppr.ReadInt(L"GridY",8);
	iDefaultGridEnable=ppr.ReadInt(L"GridEnable",1);
	iDefaultGridDisp=ppr.ReadInt(L"GridDisp",0);
	iDispGuide=ppr.ReadInt(L"DispGuide",1);
	iCursorMode=ppr.ReadInt(L"CursorMode",0);
	ppr.SetSection(L"Undo");
	iUndoLevel=ppr.ReadInt(L"Level",8);
	ppr.SetSection(L"Folders");
	ppr.ReadString(L"DocDir",strDocDir,str,MAX_PATH);
	wcscpy(strDocDir,str);
	ppr.ReadString(L"ImageDir",strImageDir,str,MAX_PATH);
	wcscpy(strImageDir,str);
	ppr.ReadString(L"LibDir",strLibDir,str,MAX_PATH);
	wcscpy(strLibDir,str);
	ppr.ReadString(L"PalDir",strPalDir,str,MAX_PATH);
	wcscpy(strPalDir,str);
	ppr.SetSection(L"RecentFiles");
	for(i=7;i>=0;--i) {
		wchar_t str[8];
		wchar_t strFile[MAX_PATH];
		swprintf(str,L"%d",i+1);
		ppr.ReadString(str,L"",strFile,MAX_PATH);
		theRecentFiles->Add(0,strFile);
	}
	hiconCopy=LoadIcon(hinstMain,MAKEINTRESOURCE(IDI_COPY));
	hbmpCopy=(HBITMAP)LoadImage(hinst,MAKEINTRESOURCE(IDB_COPY),IMAGE_BITMAP,16,16,LR_DEFAULTCOLOR);
	hbmpFileNew=(HBITMAP)LoadImage(hinst,MAKEINTRESOURCE(IDB_FILENEW),IMAGE_BITMAP,16,16,LR_DEFAULTCOLOR);
	hbmpFileOpen=(HBITMAP)LoadImage(hinst,MAKEINTRESOURCE(IDB_FILEOPEN),IMAGE_BITMAP,16,16,LR_DEFAULTCOLOR);
	hbmpFileSave=(HBITMAP)LoadImage(hinst,MAKEINTRESOURCE(IDB_FILESAVE),IMAGE_BITMAP,16,16,LR_DEFAULTCOLOR);
	hbmpExportImage=(HBITMAP)LoadImage(hinst,MAKEINTRESOURCE(IDB_EXPORTIMAGE),IMAGE_BITMAP,16,16,LR_DEFAULTCOLOR);
	hbmpZoomIn=(HBITMAP)LoadImage(hinst,MAKEINTRESOURCE(IDB_ZOOMIN),IMAGE_BITMAP,16,16,LR_DEFAULTCOLOR);
	hbmpZoom1=(HBITMAP)LoadImage(hinst,MAKEINTRESOURCE(IDB_ZOOM1),IMAGE_BITMAP,16,16,LR_DEFAULTCOLOR);
	hbmpZoomOut=(HBITMAP)LoadImage(hinst,MAKEINTRESOURCE(IDB_ZOOMOUT),IMAGE_BITMAP,16,16,LR_DEFAULTCOLOR);
	hbmpEazel=(HBITMAP)LoadImage(hinst,MAKEINTRESOURCE(IDB_EAZEL),IMAGE_BITMAP,16,16,LR_DEFAULTCOLOR);
	hbmpShapeEdit=(HBITMAP)LoadImage(hinst,MAKEINTRESOURCE(IDB_SHAPEEDIT),IMAGE_BITMAP,16,16,LR_DEFAULTCOLOR);
	hbmpMirrorH=(HBITMAP)LoadImage(hinst,MAKEINTRESOURCE(IDB_MIRRORH),IMAGE_BITMAP,16,16,LR_DEFAULTCOLOR);
	hbmpMirrorV=(HBITMAP)LoadImage(hinst,MAKEINTRESOURCE(IDB_MIRRORV),IMAGE_BITMAP,16,16,LR_DEFAULTCOLOR);
	hbmpArrow=(HBITMAP)LoadImage(hinst,MAKEINTRESOURCE(IDB_TOOLARROW),IMAGE_BITMAP,16,16,LR_DEFAULTCOLOR);
	hbmpHand=(HBITMAP)LoadImage(hinst,MAKEINTRESOURCE(IDB_TOOLHAND),IMAGE_BITMAP,16,16,LR_DEFAULTCOLOR);
	hbmpRect=(HBITMAP)LoadImage(hinst,MAKEINTRESOURCE(IDB_TOOLRECT),IMAGE_BITMAP,16,16,LR_DEFAULTCOLOR);
	hbmpEllipse=(HBITMAP)LoadImage(hinst,MAKEINTRESOURCE(IDB_TOOLELLIPSE),IMAGE_BITMAP,16,16,LR_DEFAULTCOLOR);
	hbmpPolygon=(HBITMAP)LoadImage(hinst,MAKEINTRESOURCE(IDB_TOOLPOLYGON),IMAGE_BITMAP,16,16,LR_DEFAULTCOLOR);
	hbmpText=(HBITMAP)LoadImage(hinst,MAKEINTRESOURCE(IDB_TOOLTEXT),IMAGE_BITMAP,16,16,LR_DEFAULTCOLOR);
	hbmpCurve=(HBITMAP)LoadImage(hinst,MAKEINTRESOURCE(IDB_TOOLCURVE),IMAGE_BITMAP,16,16,LR_DEFAULTCOLOR);
	hbmpShape=(HBITMAP)LoadImage(hinst,MAKEINTRESOURCE(IDB_TOOLSHAPE),IMAGE_BITMAP,16,16,LR_DEFAULTCOLOR);
	hbmpLines=(HBITMAP)LoadImage(hinst,MAKEINTRESOURCE(IDB_TOOLLINES),IMAGE_BITMAP,16,16,LR_DEFAULTCOLOR);
	hbmpGradation=(HBITMAP)LoadImage(hinst,MAKEINTRESOURCE(IDB_TOOLGRADATION),IMAGE_BITMAP,16,16,LR_DEFAULTCOLOR);
	hbmpTrimming=(HBITMAP)LoadImage(hinst,MAKEINTRESOURCE(IDB_TOOLTRIMMING),IMAGE_BITMAP,16,16,LR_DEFAULTCOLOR);
	hbmpImage=(HBITMAP)LoadImage(hinst,MAKEINTRESOURCE(IDB_TOOLINSERTIMAGE),IMAGE_BITMAP,16,16,LR_DEFAULTCOLOR);
	hbmpNewKnob=(HBITMAP)LoadImage(hinst,MAKEINTRESOURCE(IDB_TOOLINSERTNEWKNOB),IMAGE_BITMAP,16,16,LR_DEFAULTCOLOR);
	hbmpUndo=(HBITMAP)LoadImage(hinst,MAKEINTRESOURCE(IDB_UNDO),IMAGE_BITMAP,16,16,LR_DEFAULTCOLOR);
	hbmpRedo=(HBITMAP)LoadImage(hinst,MAKEINTRESOURCE(IDB_REDO),IMAGE_BITMAP,16,16,LR_DEFAULTCOLOR);
	hbmpMiter=(HBITMAP)LoadImage(hinst,MAKEINTRESOURCE(IDB_MITER),IMAGE_BITMAP,32,24,LR_DEFAULTCOLOR);
	hbmpBevel=(HBITMAP)LoadImage(hinst,MAKEINTRESOURCE(IDB_BEVEL),IMAGE_BITMAP,32,24,LR_DEFAULTCOLOR);
	hbmpRound=(HBITMAP)LoadImage(hinst,MAKEINTRESOURCE(IDB_ROUND),IMAGE_BITMAP,32,24,LR_DEFAULTCOLOR);
	hbmpFlatCap=(HBITMAP)LoadImage(hinst,MAKEINTRESOURCE(IDB_FLATCAP),IMAGE_BITMAP,32,24,LR_DEFAULTCOLOR);
	hbmpSquareCap=(HBITMAP)LoadImage(hinst,MAKEINTRESOURCE(IDB_SQUARECAP),IMAGE_BITMAP,32,24,LR_DEFAULTCOLOR);
	hbmpRoundCap=(HBITMAP)LoadImage(hinst,MAKEINTRESOURCE(IDB_ROUNDCAP),IMAGE_BITMAP,32,24,LR_DEFAULTCOLOR);
	hbmpTriangleCap=(HBITMAP)LoadImage(hinst,MAKEINTRESOURCE(IDB_TRIANGLECAP),IMAGE_BITMAP,32,24,LR_DEFAULTCOLOR);
	hbmpAlignHorzLeft=(HBITMAP)LoadImage(hinst,MAKEINTRESOURCE(IDB_ALIGN_HORZ_LEFT),IMAGE_BITMAP,16,16,LR_DEFAULTCOLOR);
	hbmpAlignHorzCenter=(HBITMAP)LoadImage(hinst,MAKEINTRESOURCE(IDB_ALIGN_HORZ_CENTER),IMAGE_BITMAP,16,16,LR_DEFAULTCOLOR);
	hbmpAlignHorzRight=(HBITMAP)LoadImage(hinst,MAKEINTRESOURCE(IDB_ALIGN_HORZ_RIGHT),IMAGE_BITMAP,16,16,LR_DEFAULTCOLOR);
	hbmpAlignVertTop=(HBITMAP)LoadImage(hinst,MAKEINTRESOURCE(IDB_ALIGN_VERT_TOP),IMAGE_BITMAP,16,16,LR_DEFAULTCOLOR);
	hbmpAlignVertCenter=(HBITMAP)LoadImage(hinst,MAKEINTRESOURCE(IDB_ALIGN_VERT_CENTER),IMAGE_BITMAP,16,16,LR_DEFAULTCOLOR);
	hbmpAlignVertBottom=(HBITMAP)LoadImage(hinst,MAKEINTRESOURCE(IDB_ALIGN_VERT_BOTTOM),IMAGE_BITMAP,16,16,LR_DEFAULTCOLOR);
	hbmpDistHorzLeft=(HBITMAP)LoadImage(hinst,MAKEINTRESOURCE(IDB_DIST_HORZ_LEFT),IMAGE_BITMAP,16,16,LR_DEFAULTCOLOR);
	hbmpDistHorzCenter=(HBITMAP)LoadImage(hinst,MAKEINTRESOURCE(IDB_DIST_HORZ_CENTER),IMAGE_BITMAP,16,16,LR_DEFAULTCOLOR);
	hbmpDistHorzRight=(HBITMAP)LoadImage(hinst,MAKEINTRESOURCE(IDB_DIST_HORZ_RIGHT),IMAGE_BITMAP,16,16,LR_DEFAULTCOLOR);
	hbmpDistVertTop=(HBITMAP)LoadImage(hinst,MAKEINTRESOURCE(IDB_DIST_VERT_TOP),IMAGE_BITMAP,16,16,LR_DEFAULTCOLOR);
	hbmpDistVertCenter=(HBITMAP)LoadImage(hinst,MAKEINTRESOURCE(IDB_DIST_VERT_CENTER),IMAGE_BITMAP,16,16,LR_DEFAULTCOLOR);
	hbmpDistVertBottom=(HBITMAP)LoadImage(hinst,MAKEINTRESOURCE(IDB_DIST_VERT_BOTTOM),IMAGE_BITMAP,16,16,LR_DEFAULTCOLOR);
}
void App::CheckMenu(void) {
	MENUITEMINFO mii;
	HMENU hmenu;
	hmenu=GetSubMenu(GetMenu(hwnd),2);
	ZeroMemory(&mii,sizeof mii);
	mii.cbSize=sizeof mii;
	mii.fMask=MIIM_STATE;
	mii.fState=theProp->IsShow()?MFS_CHECKED:MFS_UNCHECKED;
	SetMenuItemInfo(hmenu,ID_DISPLAY_PROPERTIES,0,&mii);
	mii.fState=theTree->IsShow()?MFS_CHECKED:MFS_UNCHECKED;
	SetMenuItemInfo(hmenu,ID_DISPLAY_TREE,0,&mii);
	mii.fState=theApp->pal->IsShow()?MFS_CHECKED:MFS_UNCHECKED;
	SetMenuItemInfo(hmenu,ID_DISPLAY_COLOR,0,&mii);
	mii.fState=theScreen->fRender?MFS_UNCHECKED:MFS_CHECKED;
	SetMenuItemInfo(hmenu,ID_DISPLAY_DRAFT,0,&mii);
	DrawMenuBar(theApp->hwnd);
	CheckButton(theCmdBar->hwnd,IDC_DISPTREE,theTree->IsShow());
	CheckButton(theCmdBar->hwnd,IDC_DISPPROP,theProp->IsShow());
	CheckButton(theCmdBar->hwnd,IDC_DISPCOLOR,theApp->pal->IsShow());
}
void App::SaveSetup(int i) {
	PrivateProfileWriter ppw(strIniFile,L"wb");
	ppw.Section(L"Window");
	ppw.WriteInt(L"X",rcWin.left);
	ppw.WriteInt(L"Y",rcWin.top);
	ppw.WriteInt(L"Width",rcWin.right-rcWin.left);
	ppw.WriteInt(L"Height",rcWin.bottom-rcWin.top);
	GetWindowRect(theTree->hwndFrame,&rcTree);
	ppw.WriteInt(L"TreeX",rcTree.left);
	ppw.WriteInt(L"TreeY",rcTree.top);
	ppw.WriteInt(L"TreeWidth",rcTree.right-rcTree.left);
	ppw.WriteInt(L"TreeHeight",rcTree.bottom-rcTree.top);
	GetWindowRect(theProp->hwndFrame,&rcProp);
	ppw.WriteInt(L"PropX",rcProp.left);
	ppw.WriteInt(L"PropY",rcProp.top);
	ppw.WriteInt(L"PropWidth",rcProp.right-rcProp.left);
	ppw.WriteInt(L"PropHeight",rcProp.bottom-rcProp.top);
	GetWindowRect(theApp->pal->hwnd,&rcColor);
	ppw.WriteInt(L"ColorX",rcColor.left);
	ppw.WriteInt(L"ColorY",rcColor.top);
	ppw.WriteInt(L"ColorWidth",rcColor.right-rcColor.left);
	ppw.WriteInt(L"ColorHeight",rcColor.bottom-rcColor.top);
	ppw.WriteInt(L"AlwaysFront",iAlwaysFront);
	ppw.Section(L"Screen");
	ppw.WriteInt(L"GridX",iDefaultGridX);
	ppw.WriteInt(L"GridY",iDefaultGridY);
	ppw.WriteInt(L"GridEnable",iDefaultGridEnable);
	ppw.WriteInt(L"GridDisp",iDefaultGridDisp);
	ppw.WriteInt(L"DispGuide",iDispGuide);
	ppw.WriteInt(L"CursorMode",iCursorMode);
	ppw.Section(L"Folders");
	ppw.WriteStr(L"DocDir",strDocDir);
	ppw.WriteStr(L"ImageDir",strImageDir);
	ppw.WriteStr(L"LibDir",strLibDir);
	ppw.WriteStr(L"PalDir",strPalDir);
	ppw.Section(L"Undo");
	ppw.WriteInt(L"Level",iUndoLevel);
	ppw.Section(L"RecentFiles");
	for(i=0;i<8;++i) {
		wchar_t wstr[8];
		swprintf(wstr,L"%d",i+1);
		ppw.WriteStr(wstr,theRecentFiles->Get(i));
	}
}
App::~App(void) {
	delete statusbar;
	delete pal;
}
void App::Edit(bool b) {
	if(bEdit!=b) {
		bEdit=b;
		UpdateTitle();
	}
}
void App::UpdateColor(Primitive *pr,int iSel,DWORD rgb,bool bAdjust,bool bRefresh) {
	if(pr) {
		switch(iSel) {
		case 0:
			pr->dwColor1=(pr->dwColor1&0xff000000)|rgb;
			break;
		case 1:
			pr->dwColor2=(pr->dwColor2&0xff000000)|rgb;
			break;
		case 2:
			pr->dwColor3=(pr->dwColor3&0xff000000)|rgb;
			break;
		case 3:
			pr->dwShadowColor=(pr->dwShadowColor&0xff000000)|rgb;
			break;
		case 4:
			pr->dwIShadowColor=(pr->dwIShadowColor&0xff000000)|rgb;
			break;
		}
		if(bRefresh) {
			theProp->Setup(pr,bAdjust);
			Edit();
		}
		PostMessage(hwnd,WM_USER_UPDATEPRIMITIVE,(WPARAM)pr,0);
	}
}
void App::UpdateTitle(void) {
	wchar_t str[MAX_PATH+16];
	swprintf(str,L"SkinMan [%s]%s",strCurrentProject,bEdit?L"*":L" ");
	SetWindowText(hwnd,str);
}
Gausian::Gausian(void) {
}
int Gausian::MakeTable(float sigma) {
	int i,iSum,x,xMax;
	float f,fSum;
	ZeroMemory(iTable,64*sizeof(int));
	for(i=0;i<64;++i)
		fTable[i]=0.f;
	if(sigma<=0.f) {
		fTable[0]=1.f;
		iTable[0]=0x800;
		for(i=1;i<64;++i)
			fTable[i]=0.f;
		return 0;
	}
	xMax=min(32,(int)(sigma*3)+1);
	fSum=0.f;
	iSum=0;
	for(x=0;x<xMax;++x) {
		f=expf(-x*x/(2*sigma*sigma));
		fTable[x]=f;
		iTable[x]=(int)(f*0x800);
		if(x==0)
			fSum+=f;
		else
			fSum+=(f*2.f);
	}
	for(x=0;x<xMax;++x) {
		fTable[x]=(float)(fTable[x]/sqrtf(sigma)*0.2);
		iTable[x]=(int)(iTable[x]/fSum);
	}
	return xMax;
}
void Gausian::SetMode(float sigma,float fEm1Init,float fEm2Init,bool bBmpInit) {
	xMax=MakeTable(sigma);
	iEm1=(int)(fEm1Init*4096/100);
	iEm2=(int)(fEm2Init*4096/100);
	bBmp=bBmpInit;
}
void Gausian::AlphaToInt(float sigma,int *piSrc,int *piDest,int w,int h,int iOff,float fDirX,float fDirY,int iInside) {
	int x,y,i,*pi,*pi2,*pi2End,xx,yy;
	int *ps,*pd;
	int *piTemp;
	int iOffX,iOffY;
	int a0,a1,a2,a3;
	xMax=MakeTable(sigma);
	fDirX*=-iOff;
	fDirY*=-iOff;
	iOffX=(int)floorf(fDirX);
	iOffY=(int)floorf(fDirY);
	fDirX-=iOffX;
	fDirY-=iOffY;
	piTemp=new int[w*h];
	ZeroMemory(piTemp,w*h*sizeof(int));
	ZeroMemory(piDest,w*h*sizeof(int));
	for(y=0;y<h;++y) {
		for(x=0;x<w;++x) {
			a0=a1=a2=a3=0;
			if((unsigned)(y+iOffY)<(unsigned)h && (unsigned)(x+iOffX)<(unsigned)w)
				a0=*(piSrc+(y+iOffY)*w+x+iOffX);
			if((unsigned)(y+iOffY)<(unsigned)h && (unsigned)(x+iOffX+1)<(unsigned)w)
				a1=*(piSrc+(y+iOffY)*w+x+iOffX+1);
			if((unsigned)(y+iOffY+1)<(unsigned)h && (unsigned)(x+iOffX)<(unsigned)w)
				a2=*(piSrc+(y+iOffY+1)*w+x+iOffX);
			if((unsigned)(y+iOffY+1)<(unsigned)h && (unsigned)(x+iOffX+1)<(unsigned)w)
				a3=*(piSrc+(y+iOffY+1)*w+x+iOffX+1);
			a0+=(a1-a0)*fDirX;
			a2+=(a3-a2)*fDirX;
			a0+=(a2-a0)*fDirY;
			*(piDest+y*w+x)=a0;
		}
	}
	for(y=0;y<h;++y) {
		pi=piTemp+y*w;
		yy=y;
		ps=piDest+yy*w;
		if((unsigned)yy<(unsigned)h) {
			if(iInside) {
				for(x=0;x<w;++x) {
					for(i=-xMax;i<=xMax;++i) {
						xx=x+i;
						if((unsigned)xx<(unsigned)w) {
							*pi += iTable[abs(i)]*(255-*(ps+i));
						}
					}
					++pi;
					++ps;
				}
			}
			else {
				for(x=0;x<w;++x) {
					for(i=-xMax;i<=xMax;++i) {
						xx=x+i;
						if((unsigned)xx<(unsigned)w) {
							*pi += iTable[abs(i)]*(*(ps+i));
						}
					}
					++pi;
					++ps;
				}
			}
		}
	}
	for(x=0;x<w;++x) {
		pi=piTemp+x;
		pi2=piDest+x;
		for(y=0;y<h;++y) {
			for(i=-xMax;i<=xMax;++i) {
				yy=y+i;
				if((unsigned)yy<(unsigned)h) {
					*pi2+=iTable[abs(i)]* *(pi+i*w);
				}
			}
			pi+=w;
			pi2+=w;
		}
	}
	for(pi2=piDest,pi2End=piDest+h*w;pi2<pi2End;++pi2) {
		*pi2=(*pi2)>>14;
	}
	delete[] piTemp;
}
int Gausian::GetAlphaValue(DWORD dw) {
	if(bBmp) {
		if(iEm2) {
			int l;
			l=(dw&0xff)*28+((dw>>8)&0xff)*151+((dw>>16)&0xff)*77;
			return ((int)(dw>>24)*iEm1+(l*iEm2>>8))>>12;
		}
		return (int)(dw>>24)*iEm1>>12;
	}
	return ((int)(dw&0xff)*iEm1+(int)((dw>>16)&0xff)*iEm2)>>12;
}
int GetLumiValue(DWORD dw) {
	int l;
	l=(dw&0xff)*28+((dw>>8)&0xff)*151+((dw>>16)&0xff)*77;
	return l;
}
void Gausian::AlphaToIntDiff(DWORD *pdwSrc,int *piDest,int w,int h,int iSrcStride,float fDirX,float fDirY) {
	int x,y,i,*pi,*pi2,xx,yy;
	float fAX,fBX;
	DWORD dw;
	int *piTemp,*piTemp2,iTemp;
	int iAX0,iAX1,iAY0,iAY1,iBX0,iBX1,iBY0,iBY1;
	piTemp=new int[w*h];
	piTemp2=new int[w*h];
	ZeroMemory(piTemp,w*h*sizeof(int));
	ZeroMemory(piDest,w*h*sizeof(int));
	ZeroMemory(piTemp2,w*h*sizeof(int));
	for(y=0;y<h;++y) {
		pi=piTemp+y*w;
		for(x=0;x<w;++x) {
			dw=*(DWORD*)((BYTE*)pdwSrc+y*iSrcStride+x*4);
			iAX0=iBX0=iAX1=iBX1=iAY0=iAY1=iBY0=iBY1=0;
			if(bBmp) {
				fAX=(float)((dw>>24)<<8);
				fBX=(float)GetLumiValue(dw);
				if(x>=1) {
					dw=*(DWORD*)((BYTE*)pdwSrc+y*iSrcStride+(x-1)*4);
					iAX0=((int)(dw>>24)*iEm1>>12);
					iBX0=(GetLumiValue(dw)*iEm2>>20);
				}
				if(x<w-1) {
					dw=*(DWORD*)((BYTE*)pdwSrc+y*iSrcStride+(x+1)*4);
					iAX1=((int)(dw>>24)*iEm1>>12);
					iBX1=(GetLumiValue(dw)*iEm2>>20);
				}
				if(y>=1) {
					dw=*(DWORD*)((BYTE*)pdwSrc+(y-1)*iSrcStride+x*4);
					iAY0=((int)(dw>>24)*iEm1>>12);
					iBY0=(GetLumiValue(dw)*iEm2>>20);
				}
				if(y<h-1) {
					dw=*(DWORD*)((BYTE*)pdwSrc+(y+1)*iSrcStride+x*4);
					iAY1=((int)(dw>>24)*iEm1>>12);
					iBY1=(GetLumiValue(dw)*iEm2>>20);
				}
			}
			else {
				fAX=(float)((dw&0xff)<<8);
				fBX=(float)((dw&0xff0000)>>8);
				if(x>=1) {
					dw=*(DWORD*)((BYTE*)pdwSrc+y*iSrcStride+(x-1)*4);
					iAX0=((int)(dw&0xff)*iEm1>>12);
					iBX0=(((int)(dw&0xff0000)>>16)*iEm2>>12);
				}
				if(x<w-1) {
					dw=*(DWORD*)((BYTE*)pdwSrc+y*iSrcStride+(x+1)*4);
					iAX1=((int)(dw&0xff)*iEm1>>12);
					iBX1=(((int)(dw&0xff0000)>>16)*iEm2>>12);
				}
				if(y>=1) {
					dw=*(DWORD*)((BYTE*)pdwSrc+(y-1)*iSrcStride+x*4);
					iAY0=((int)(dw&0xff)*iEm1>>12);
					iBY0=(((int)(dw&0xff0000)>>16)*iEm2>>12);
				}
				if(y<h-1) {
					dw=*(DWORD*)((BYTE*)pdwSrc+(y+1)*iSrcStride+x*4);
					iAY1=((int)(dw&0xff)*iEm1>>12);
					iBY1=(((int)(dw&0xff0000)>>16)*iEm2>>12);
				}
			}
			iTemp=(int)(((iAY1-iAY0)*fDirY+(iAX1-iAX0)*fDirX)*abs(fAX))
				+(int)(((iBY1-iBY0)*fDirY+(iBX1-iBX0)*fDirX)*abs(fBX));
			*pi=iTemp;
			++pi;
		}
	}
	for(y=0;y<h;++y) {
		pi=piTemp+y*w;
		pi2=piTemp2+y*w;
		for(x=0;x<w;++x) {
			for(i=-xMax;i<=xMax;++i) {
				xx=x+i;
				if((unsigned)xx<(unsigned)w) {
					*pi2+=(int)(fTable[abs(i)]* *(pi+i));
				}
			}
			++pi;
			++pi2;
		}
	}
	for(x=0;x<w;++x) {
		pi=piTemp2+x;
		pi2=piDest+x;
		for(y=0;y<h;++y) {
			*pi2=0;
			for(i=-xMax;i<=xMax;++i) {
				yy=y+i;
				if((unsigned)yy<(unsigned)h) {
					*pi2+=(int)(fTable[abs(i)]* *(pi+i*w));
				}
			}
			pi+=w;
			pi2+=w;
		}
	}
	delete[] piTemp;
	delete[] piTemp2;
}
Bezier::Bezier(void) {
	dScaleX=dScaleY=0.;
	rcOrg.X=0;
	rcOrg.Y=0;
	rcOrg.Width=1.f;
	rcOrg.Height=1.f;
	iClose=0;
	mxScale=0;
	iIndex=iFocus=0;
}
Bezier::~Bezier(void) {
}
Bezier& Bezier::operator=(Bezier& t) {
	memcpy(this,&t,sizeof(Bezier));
	if(t.mxScale)
		mxScale=t.mxScale->Clone();
	return *this;
}
void Bezier::MirrorH(REAL x) {
	int i;
	for(i=0;i<iIndex;++i)
		ptAnchor[i].X=x+x-ptAnchor[i].X;
}
void Bezier::MirrorV(REAL y) {
	int i;
	for(i=0;i<iIndex;++i)
		ptAnchor[i].Y=y+y-ptAnchor[i].Y;
}
void Bezier::NormalizeRot(Primitive *pr) {
	RectF rcf;
	PointF ptf,ptf2,ptfDiff,ptfCenterOld,ptfCenterNew;
	float fAng;
	if(pr==0)
		return;
	ptfCenterOld=GetCenter(pr->ptfOrg[0],pr->ptfOrg[1]);
	fAng=-pr->rAngle*PI/180.f;
	double dsin=sin(fAng);
	double dcos=cos(fAng);
	GetPath(0,pr->iClose,pr->path,&rcf);
	pr->ptfOrg[0].X=rcf.X;
	pr->ptfOrg[0].Y=rcf.Y;
	pr->ptfOrg[1].X=rcf.X+rcf.Width;
	pr->ptfOrg[1].Y=rcf.Y+rcf.Height;
	ptfCenterNew=GetCenter(pr->ptfOrg[0],pr->ptfOrg[1]);
	ptf=ptfCenterNew-ptfCenterOld;
	ptf2.X=(float)(ptf.X*dcos+ptf.Y*dsin);
	ptf2.Y=(float)(-ptf.X*dsin+ptf.Y*dcos);
	ptfDiff=ptf2-ptf;
	pr->ptfOrg[0]=pr->ptfOrg[0]+ptfDiff;
	pr->ptfOrg[1]=pr->ptfOrg[1]+ptfDiff;
	SetScale(pr);
	pr->CalcPath(2,0);
}
void Bezier::Normalize(RectF *prcf) {
	return;
	int i;
	PointF ptMin,ptMax;
	float fDX,fDY;
	ptMin=PointF(1.0e10,1.0e10);
	ptMax=PointF(-1.0e10,-1.0e10);
	for(i=0;i<iIndex;++i) {
		if(ptMin.X>ptAnchor[i].X)
			ptMin.X=ptAnchor[i].X;
		if(ptMin.Y>ptAnchor[i].Y)
			ptMin.Y=ptAnchor[i].Y;
		if(ptMax.X<ptAnchor[i].X)
			ptMax.X=ptAnchor[i].X;
		if(ptMax.Y<ptAnchor[i].Y)
			ptMax.Y=ptAnchor[i].Y;
	}
	fDX=ptMax.X-ptMin.X;
	fDY=ptMax.Y-ptMin.Y;
	for(i=0;i<iIndex;++i) {
		if(fDX)
			ptAnchor[i].X=(ptAnchor[i].X-ptMin.X)*1000.f/fDX;
		else
			ptAnchor[i].X=0;
		if(fDY)
			ptAnchor[i].Y=(ptAnchor[i].Y-ptMin.Y)*1000.f/fDY;
		else
			ptAnchor[i].Y=0;
	}
	if(prcf) {
		prcf->X=ptMin.X;
		prcf->Y=ptMin.Y;
		prcf->Width=ptMax.X-ptMin.X;
		prcf->Height=ptMax.Y-ptMin.Y;
	}
}
void Bezier::Reset(void) {
	iIndex=0;
	iFocus=0;
	rcOrg.X=rcOrg.Y=0;
	rcOrg.Width=rcOrg.Height=0.f;
	dScaleX=dScaleY=1.f;
	iClose=0;
	if(mxScale)
		delete mxScale;
	mxScale=new Matrix();
	bDrag=false;
}
void Bezier::DelPoint(Primitive *pr) {
	int i;
	if(iFocus>=1 && iIndex>=9) {
		for(i=iFocus-1;i<iIndex-3;++i)
			ptAnchor[i]=ptAnchor[i+3];
		iIndex-=3;
		if(iFocus>iIndex)
			iFocus=iIndex-2;
		if(pr) {
			GetPath(0,pr->iClose,pr->path,&pr->rcOutline);
			pr->ptfOrg[0].X=pr->rcOutline.X;
			pr->ptfOrg[0].Y=pr->rcOutline.Y;
			pr->ptfOrg[1].X=pr->rcOutline.X+pr->rcOutline.Width;
			pr->ptfOrg[1].Y=pr->rcOutline.Y+pr->rcOutline.Height;
			pr->CalcPath(2,1);
		}
	}
}
void Bezier::AddPoint(PointF pt) {
	Down(pt);
	Up(pt);
}
void Bezier::Down(PointF pt) {
	int i;
	for(i=iIndex;i>iFocus;--i) {
		ptAnchor[i+3]=ptAnchor[i];
	}
	bDrag=true;
	if(iFocus<1)
		iFocus=1;
	else
		iFocus+=3;
	iIndex+=3;
	ptAnchor[iFocus-1]=ptAnchor[iFocus]=ptAnchor[iFocus+1]=pt;
}
void Bezier::Drag(PointF pt) {
	if(iFocus>=1) {
		ptAnchor[iFocus+1]=pt;
		if(iIndex>=3) {
			ptAnchor[iFocus-1]=ptAnchor[iFocus]+ptAnchor[iFocus]-pt;
		}
	}
}
void Bezier::Up(PointF pt) {
	Drag(pt);
	bDrag=false;
}
Primitive *Bezier::Close(bool bFill) {
	RectF rc;
	GraphicsPath path;
	Primitive *prParent,*prAfter,*prim;
	bDrag=false;
	if(iIndex>=6) {
		bzTemp->GetPath(0,bFill,&path,&rc);
		prAfter=theTree->prlCurrent.pr;
		prParent=theTree->GetParent(prAfter);
		EnterCriticalSection(&csData);
		prim=theTree->AddItem(prParent,prAfter,bFill?L"Shape":L"Curve",prtShape,rc.X,rc.Y,rc.X+rc.Width,rc.Y+rc.Height);
		if(bFill==false) {
			prim->iFill=0;
			prim->iBorder=1;
			prim->iClose=0;
		}
		prim->CalcPath(2,1);
		theApp->Edit();
		theScreen->Send();
		LeaveCriticalSection(&csData);
		return prim;
	}
	else
		return NULL;
}

void Bezier::Draw(HDC hdc,PointF pt1,PointF pt2,Matrix *mxRot) {
	Graphics *g=new Graphics(hdc);
	Draw(g,pt1,pt2,mxRot);
	delete g;
}
void Bezier::Draw(Graphics *g,PointF pt1,PointF pt2,Matrix *mxRot) {
	int i,j;
	PointF pt[5];
	if(iIndex==0)
		return;
	Pen pen(Color(255,80,25,25));
	PointF ptfCenter=PointF((pt1.X+pt2.X)*.5f,(pt1.Y+pt2.Y)*.5f);
	for(i=0;i<iIndex-3;i+=3) {
		for(j=1;j<5;++j) {
			pt[j].X=ptAnchor[i+j].X;
			pt[j].Y=ptAnchor[i+j].Y;
			if(mxScale)
				mxScale->TransformPoints(&pt[j]);
			TCheck(mxScale);
			if(mxRot)
				mxRot->TransformPoints(&pt[j]);
			pt[j].X=pt[j].X*theScreen->fZoom+theScreen->iDispOffX;
			pt[j].Y=pt[j].Y*theScreen->fZoom+theScreen->iDispOffY;
		}
		g->DrawBezier(&pen,pt[1],pt[2],pt[3],pt[4]);
		if(i==0)
			theScreen->DrawHandleMark(g,pt[1],3);
		else
			theScreen->DrawHandleMark(g,pt[1]);
		theScreen->DrawHandleMark(g,pt[4]);
	}
	pt[0]=ptAnchor[iFocus-1];
	pt[1]=ptAnchor[iFocus];
	pt[2]=ptAnchor[iFocus+1];
	if(mxScale)
		mxScale->TransformPoints(&pt[0],3);
	TCheck(mxScale);
	if(mxRot)
		mxRot->TransformPoints(&pt[0],3);
	pt[0].X=pt[0].X*theScreen->fZoom+theScreen->iDispOffX;
	pt[0].Y=pt[0].Y*theScreen->fZoom+theScreen->iDispOffY;
	pt[1].X=pt[1].X*theScreen->fZoom+theScreen->iDispOffX;
	pt[1].Y=pt[1].Y*theScreen->fZoom+theScreen->iDispOffY;
	pt[2].X=pt[2].X*theScreen->fZoom+theScreen->iDispOffX;
	pt[2].Y=pt[2].Y*theScreen->fZoom+theScreen->iDispOffY;
	if(iIndex>3) {
		g->DrawLine(&pen,pt[0],pt[1]);
		theScreen->DrawHandleMark(g,pt[0],2);
	}
	g->DrawLine(&pen,pt[1],pt[2]);
	theScreen->DrawHandleMark(g,pt[2],2);
	if(iFocus==1)
		theScreen->DrawHandleMark(g,pt[1],3,Color::Red);
	else
		theScreen->DrawHandleMark(g,pt[1],0,Color::Red);
}
int Bezier::HitTest(int x,int y,int iControl,PointF pt1,PointF pt2,Matrix *mxRot) {
	int i;
	PointF pt;
	float fX,fY;
	Matrix *mx=new Matrix();
	PointF ptfCenter=PointF((pt1.X+pt2.X)*.5f,(pt1.Y+pt2.Y)*.5f);
	mx->Translate(-ptfCenter.X,-ptfCenter.Y,MatrixOrderAppend);
	if(pt1.X>pt2.X)
		mx->Scale(-1.f,1.f,MatrixOrderAppend);
	if(pt1.Y>pt2.Y)
		mx->Scale(1.f,-1.f,MatrixOrderAppend);
	mx->Translate(ptfCenter.X,ptfCenter.Y,MatrixOrderAppend);
	fX=(x-theScreen->iDispOffX)/theScreen->fZoom;
	fY=(y-theScreen->iDispOffY)/theScreen->fZoom;
	if(iControl==0 && iFocus>0) {
		pt=ptAnchor[i=iFocus-1];
		if(mxScale)
			mxScale->TransformPoints(&pt);
		TCheck(mxScale);
		if(mxRot)
			mxRot->TransformPoints(&pt);
		mx->TransformPoints(&pt);
		if(theScreen->Check(fX,fY,pt)) {
			delete mx;
			return i;
		}
		pt=ptAnchor[i=iFocus+1];
		if(mxScale)
			mxScale->TransformPoints(&pt);
		TCheck(mxScale);
		if(mxRot)
			mxRot->TransformPoints(&pt);
		mx->TransformPoints(&pt);
		if(theScreen->Check(fX,fY,pt)) {
			delete mx;
			return i;
		}
		pt=ptAnchor[i=iFocus];
		if(mxScale)
			mxScale->TransformPoints(&pt);
		TCheck(mxScale);
		if(mxRot)
			mxRot->TransformPoints(&pt);
		mx->TransformPoints(&pt);
		if(theScreen->Check(fX,fY,pt)) {
			delete mx;
			return i;
		}
	}
	for(i=1;i<iIndex-1;i+=3) {
		pt=ptAnchor[i];
		if(mxScale)
			mxScale->TransformPoints(&pt);
		TCheck(mxScale);
		if(mxRot)
			mxRot->TransformPoints(&pt);
		mx->TransformPoints(&pt);
		if(theScreen->Check(fX,fY,pt)) {
			delete mx;
			return i;
		}
	}
	delete mx;
	return -1;
}
void Bezier::SetScale(Primitive *pr) {
	if(pr) {
		RectF rcf;
		rcf.X=pr->ptfOrg[0].X;
		rcf.Y=pr->ptfOrg[0].Y;
		rcf.Width=pr->ptfOrg[1].X-rcf.X;
		rcf.Height=pr->ptfOrg[1].Y-rcf.Y;
		GetPath(1,pr->iClose,pr->path,&rcf);
	}
}
void Bezier::GetPath(int iAdjust,int iClose,GraphicsPath *path,RectF *rcfBounds) {
	RectF rcf;
	path->Reset();
	if(iIndex>3) {
		if(iClose) {
			ptAnchor[iIndex]=ptAnchor[0];
			ptAnchor[iIndex+1]=ptAnchor[1];
			path->AddBeziers(&ptAnchor[1],iIndex+1);
			path->CloseFigure();
		}
		else {
			path->AddBeziers(&ptAnchor[1],iIndex-2);
		}
	}
	path->Flatten();
	if(iAdjust) {
		float fScaleX,fScaleY;
		path->GetBounds(&rcf);
		if(mxScale)
			delete mxScale;
		mxScale=new Matrix();
		TCheck(mxScale);
		mxScale->Translate(-rcf.X,-rcf.Y);
		TCheck(mxScale);
		fScaleX=fScaleY=1.0f;
		if(rcf.Width)
			fScaleX=rcfBounds->Width/rcf.Width;
		if(rcf.Height)
			fScaleY=rcfBounds->Height/rcf.Height;
		mxScale->Scale(fScaleX,fScaleY,MatrixOrderAppend);
		mxScale->Translate(rcfBounds->X,rcfBounds->Y,MatrixOrderAppend);
		TCheck(mxScale);
		path->Transform(mxScale);
	}
	else {
		if(mxScale)
			path->Transform(mxScale);
		path->GetBounds(rcfBounds);
	}
}
Tex::Tex(void) {
	memset(this,0,sizeof Tex);
	mx=new Matrix();
}
Tex::Tex(wchar_t *str) {
	memset(this,0,sizeof Tex);
	wcscpy(strName,PathFindFileName(str));
	*wcsrchr(strName,'.')=0;
	wcscpy(strFile,str);
	mx=new Matrix();
}
Tex::Tex(wchar_t *strDir,wchar_t *strFN) {
	wchar_t *p;
	memset(this,0,sizeof Tex);
	wcscpy(strName,strFN);
	p=wcsrchr(strName,'.');
	if(wcsicmp(p,L".bmp")==0)
		*p=0;
	wcscpy(strFile,strDir);
	wcscat(strFile,L"\\");
	wcscat(strFile,strFN);
	mx=new Matrix();
}
Tex::~Tex(void) {
	if(bmp)
		delete bmp;
	if(piLumi)
		delete[] piLumi;
	if(piAlpha)
		delete[] piAlpha;
	if(mx)
		delete mx;
}
Tex *Tex::Clone(void) {
	Tex *p=new Tex();
	wcscpy(p->strName,strName);
	wcscpy(p->strFile,strFile);
	p->iHeight=iHeight;
	p->iWidth=iWidth;
	p->piLumi=p->piAlpha=0;
	p->bmp=0;
	p->bInitialize=bInitializeThumb=false;
	p->Load();
	p->mx=new Matrix();
	return p;
}
void Tex::LoadBmp(void) {
	int *pi,*piA;
	Rect rc;
	BitmapData bmpd;
	int xx,yy,a,r,g,b;
	DWORD *pdw,dw;
	Bitmap *bmp2;
	bool bHasAlpha;
	if(bInitializeThumb==false) {
		if(bmp)
			delete bmp;
		bmp=new Bitmap(strFile);
		iWidth=bmp->GetWidth();
		iHeight=bmp->GetHeight();
		if(piLumi)
			delete[] piLumi;
		if(piAlpha)
			delete[] piAlpha;
		bInitializeThumb=true;
		pi=piLumi=new int[iWidth*iHeight];
		piA=piAlpha=new int[iWidth*iHeight];
		rc.X=rc.Y=0;
		rc.Width=iWidth;
		rc.Height=iHeight;
		bHasAlpha=false;
		bmp->LockBits(&rc,ImageLockModeRead,PixelFormat32bppARGB,&bmpd);
		for(yy=0;yy<iHeight;++yy) {
			pdw=(DWORD*)((BYTE*)bmpd.Scan0+yy*bmpd.Stride);
			for(xx=0;xx<iWidth;++xx) {
				dw=*pdw;
				a=dw>>24;
				r=(dw>>16)&0xff;
				g=(dw>>8)&0xff;
				b=dw&0xff;
//				*pi++=a*(r*3+g*6+b)/2550;
//				*pi++=(((r*3+g*6+b)*a/10)+(255-a)*128)/255-128;
//				*pi++=(a&0xff)+(((r*3+g*6+b)/10-128)<<8);
				*pi++=((r*3+g*6+b)/10)-128;
				*piA++=a;
				++pdw;
				if(a!=255)
					bHasAlpha=true;
			}
		}
		bmp2=new Bitmap(40,24,PixelFormat32bppARGB);
		bmp->UnlockBits(&bmpd);
		int w=bmp->GetWidth();
		int h=bmp->GetHeight();
		Graphics g(bmp2);
		if(bHasAlpha)
			g.Clear(Color(255,128,128));
		else
			g.Clear(Color(128,128,128));
		g.DrawImage(bmp,0,0,w/2-20,h/2-12,40,24,UnitPixel);
		delete bmp;
		bmp=bmp2;
	}
}
void Tex::Load(void) {
	if(bInitializeThumb==false)
		LoadBmp();
	SetupMx();
	bInitialize=bInitializeThumb=true;
}
void Tex::SetRotZoom(float r,float offx,float offy,float zx,float zy) {
	fRot=r;
	fZoomX=zx;
	fZoomY=zy;
	fOffX=offx;
	fOffY=offy;
	SetupMx();
}
void Tex::SetupMx(void) {
	mx->Reset();
	mx->Rotate(fRot,MatrixOrderAppend);
	if(fZoomX!=0.f && fZoomY!=0.f)
		mx->Scale(1.f/fZoomX,1.f/fZoomY,MatrixOrderAppend);
	mx->Translate(iWidth*(.5f+fOffX/100.f),iHeight*(.5f+fOffY/100.f),MatrixOrderAppend);
}
int Tex::Get(float x,float y) {
	int t0,t1,t2,t3,xx,yy,xx1,yy1;
	if(bInitialize==false)
		Load();
	if(fZoomX==0.f || fZoomY==0.f || iWidth==0 || iHeight==0)
		return 0;
	PointF ptf=PointF(x,y);
	mx->TransformPoints(&ptf);
	while(ptf.X<0)
		ptf.X+=(float)iWidth;
	while(ptf.Y<0)
		ptf.Y+=(float)iHeight;
	xx=(int)ptf.X;
	yy=(int)ptf.Y;
	ptf.X-=(float)xx;
	ptf.Y-=(float)yy;
	xx%=iWidth;
	yy%=iHeight;
	xx1=(xx+1)%iWidth;
	yy1=(yy+1)%iHeight;
	t0=*(piLumi+xx+yy*iWidth);
	t1=*(piLumi+xx1+yy*iWidth);
	t2=*(piLumi+xx+yy1*iWidth);
	t3=*(piLumi+xx1+yy1*iWidth);
	t0+=(int)((t1-t0)*ptf.X);
	t2+=(int)((t3-t2)*ptf.X);
	t0+=(int)((t2-t0)*ptf.Y);
	return t0;
}
int Tex::GetAlpha(float x,float y) {
	int t0,t1,t2,t3,xx,yy,xx1,yy1;
	if(bInitialize==false)
		Load();
	if(fZoomX==0.f || fZoomY==0.f || iWidth==0 || iHeight==0)
		return 0;
	PointF ptf=PointF(x,y);
	mx->TransformPoints(&ptf);
	while(ptf.X<0)
		ptf.X+=(float)iWidth;
	while(ptf.Y<0)
		ptf.Y+=(float)iHeight;
	xx=(int)ptf.X;
	yy=(int)ptf.Y;
	ptf.X-=(float)xx;
	ptf.Y-=(float)yy;
	xx%=iWidth;
	yy%=iHeight;
	xx1=(xx+1)%iWidth;
	yy1=(yy+1)%iHeight;
	t0=*(piAlpha+xx+yy*iWidth);
	t1=*(piAlpha+xx1+yy*iWidth);
	t2=*(piAlpha+xx+yy1*iWidth);
	t3=*(piAlpha+xx1+yy1*iWidth);
	t0+=(int)((t1-t0)*ptf.X);
	t2+=(int)((t3-t2)*ptf.X);
	t0+=(int)((t2-t0)*ptf.Y);
	return t0;
}
Bitmap *Tex::GetBmp(void) {
	LoadBmp();
	return bmp;
}
Texture::Texture(void) {
	iTexMax=0;
	bmpInit=0;
	texCurrent=0;
}
Texture::~Texture(void) {
	int i;
	iInitializing=0;
	for(i=0;i<iTexMax;++i)
		delete tex[i];
	delete tex;
}
int Texture::CountTex(wchar_t *wstrType,wchar_t *wstrTextureDir) {
	HANDLE h;
	WIN32_FIND_DATA FindFileData;
	wchar_t wstrFile[MAX_PATH];
	int i;
	i=0;
	wcscpy(wstrFile,wstrTextureDir);
	wcscat(wstrFile,L"\\");
	wcscat(wstrFile,wstrType);
	h=FindFirstFile(wstrFile,&FindFileData);
	if(h!=INVALID_HANDLE_VALUE) {
		do {
			++i;
		} while(FindNextFile(h,&FindFileData));
		FindClose(h);
	}
	return i;
}
int texcomparefunc(const void* t1,const void* t2) {
	Tex **tex1=(Tex**)t1;
	Tex **tex2=(Tex**)t2;
	return wcsicmp((*tex1)->strName,(*tex2)->strName);
}
void Texture::Init(wchar_t *wstrTextureDir) {
	WIN32_FIND_DATA FindFileData;
	wchar_t strFile[MAX_PATH];
	wchar_t *str;
	wchar_t strMess[MAX_PATH];
	HANDLE h;
	int i,j;
	HDC hdcScr;
	wchar_t *type[5]={L"*.bmp",L"*.png",L"*.jpg",L"*.jpeg",L"*.gif"};
	theSplash->Message(L"Initializing Texture:");

	iTexMax=0;
	for(j=0;j<5;++j)
		iTexMax+=CountTex(type[j],wstrTextureDir);
	tex=new Tex*[iTexMax];
	i=0;
	for(j=0;j<5;++j) {
		wcscpy(strFile,wstrTextureDir);
		wcscat(strFile,L"\\");
		wcscat(strFile,type[j]);
		h=FindFirstFile(strFile,&FindFileData);
		if(h!=INVALID_HANDLE_VALUE) {
			do {
				str=FindFileData.cFileName;
				tex[i++]=new Tex(wstrTextureDir,str);
				if(i>=iTexMax)
					break;
			} while(FindNextFile(h,&FindFileData));
			FindClose(h);
		}
	}
	qsort(tex,iTexMax,sizeof(Tex*),texcomparefunc);
	hdcScr=GetDC(0);
	bmpInit=new Bitmap(40*(iTexMax+1),24,PixelFormat24bppRGB);
	Graphics g(bmpInit);
	for(i=0;i<iTexMax;++i) {
		wsprintf(strMess,L"Texture:%s",tex[i]->strName);
		theSplash->Message(strMess);
		g.DrawImage(tex[i]->GetBmp(),(i+1)*40,0,0,0,40,24,UnitPixel);
	}
}
HIMAGELIST Texture::CreateImageList(Primitive *p) {
	HIMAGELIST himl;
	HBITMAP hbmp;
	himl=ImageList_Create(40,24,ILC_COLOR24,32,32);
	Graphics g(bmpInit);
	g.FillRectangle(&SolidBrush(Color(255,255,255)),Rect(0,0,40,24));
	if(p && p->texEmbed) {
		Image *img=p->texEmbed->GetBmp();
		g.DrawImage(img,0,0,0,0,40,24,UnitPixel);
	}
	bmpInit->GetHBITMAP(Color(255,255,255),&hbmp);
	ImageList_Add(himl,hbmp,0);
	DeleteObject((HGDIOBJ)hbmp);
	return himl;
}
void Texture::Reset(Primitive *p) {
	if(p->iTextureType<0)
		texCurrent=p->texEmbed;
	else if(p->iTextureType<iTexMax)
		texCurrent=tex[p->iTextureType];
	else
		texCurrent=NULL;
	if(texCurrent) {
		if(p->iTextureZoomXYSepa)
			texCurrent->SetRotZoom(-p->fTextureRot,-p->fTextureOffX,-p->fTextureOffY,p->fTextureZoomX/100.f,p->fTextureZoomY/100.f);
		else
			texCurrent->SetRotZoom(-p->fTextureRot,-p->fTextureOffX,-p->fTextureOffY,p->fTextureZoomX/100.f,p->fTextureZoomX/100.f);
	}
}
wchar_t *Texture::GetName(int n) {
	if(n<0)
		return L"()";
	if(n<iTexMax)
		return tex[n]->strName;
	return NULL;
}
int Texture::Get(int x,int y) {
	if(texCurrent)
		return texCurrent->Get((float)x,(float)y);
	return 0;
}
int Texture::GetAlpha(int x,int y) {
	if(texCurrent)
		return texCurrent->GetAlpha((float)x,(float)y);
	return 255;
}
int Texture::Find(wchar_t *str) {
	int i;
	for(i=0;i<iTexMax;++i) {
		if(wcscmp(tex[i]->strName,str)==0)
			return i;
	}
	return 0;
}
Screen::Screen(HWND hwndParentInit) {
	HDC hdc;
	InitializeCriticalSection(&csData);
	hDrawRequest=CreateEvent(NULL,FALSE,FALSE,NULL);
	hDrawRequest2=CreateEvent(NULL,FALSE,FALSE,NULL);
	hRenderWaiting2=CreateEvent(NULL,TRUE,TRUE,NULL);
	bStart=false;
	fRenderBreak=false;
	iExitDrawThread=0;
	iWidth=640;
	iHeight=480;
	iFramesX=iFramesY=1;
	iAnimFrames=1;
	iBgTransparent=1;
	colBackground=Color(255,255,255);
	colWorkspace=Color(100,100,100);
	iOffsetX=iOffsetY=0;
	iDispOffX=iDispOffY=0;
	iGridX=theApp->iDefaultGridX;
	iGridY=theApp->iDefaultGridY;
	iGridEnable=theApp->iDefaultGridEnable;
	iGridVisible=theApp->iDefaultGridDisp;
	fRender=true;
	fZoom=1.f;
	iOutScale=1;
	hwndParent=hwndParentInit;
	hdcMem=CreateCompatibleDC(NULL);
	hdcMem2=CreateCompatibleDC(NULL);
	hdcMemPreview=CreateCompatibleDC(NULL);
	hdcScreen=CreateCompatibleDC(NULL);
	hdc=GetDC(theApp->hwnd);
	hbmpMem=CreateCompatibleBitmap(hdc,iWidth,iHeight);
	hbmpMem2=CreateCompatibleBitmap(hdc,iWidth,iHeight);
	hbmpMemPreview=CreateCompatibleBitmap(hdc,iWidth/iOutScale,iHeight/iOutScale*iAnimFrames);
	hbmpScreen=CreateCompatibleBitmap(hdc,10,10);
	ReleaseDC(theApp->hwnd,hdc);
	hbmpMemOld=(HBITMAP)SelectObject(hdcMem,hbmpMem);
	hbmpMem2Old=(HBITMAP)SelectObject(hdcMem2,hbmpMem2);
	hbmpMemPreviewOld=(HBITMAP)SelectObject(hdcMemPreview,hbmpMemPreview);
	hbmpScreenOld=(HBITMAP)SelectObject(hdcScreen,hbmpScreen);
	Register();
	hwnd=CreateWindowEx(WS_EX_CLIENTEDGE,L"Screen",L"",WS_CHILD|WS_VISIBLE|WS_HSCROLL|WS_VSCROLL,0,0,0,0,hwndParent,NULL,hinstMain,(LPVOID)this);
	theApp->Edit(false);
	Send();
}
void Screen::Start(void) {
	bStart=true;
	_beginthread(DrawThread,0,this);
}
Screen::~Screen(void) {
	iExitDrawThread=1;
	while(iExitDrawThread==1) {
		Sleep(10);
	}
	SelectObject(hdcMem,hbmpMemOld);
	DeleteObject((HGDIOBJ)hbmpMem);
	DeleteDC(hdcMem);
	SelectObject(hdcMem2,hbmpMem2Old);
	DeleteObject((HGDIOBJ)hbmpMem2);
	DeleteDC(hdcMem2);
	SelectObject(hdcMemPreview,hbmpMemPreviewOld);
	DeleteObject((HGDIOBJ)hbmpMemPreview);
	DeleteDC(hdcMemPreview);
	SelectObject(hdcScreen,hbmpScreenOld);
	DeleteObject((HGDIOBJ)hbmpScreen);
	DeleteDC(hdcScreen);
}
void Screen::Register(void) {
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX); 
	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)wndproc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hinstMain;
	wcex.hIcon			= NULL;
	wcex.hCursor		= NULL;
	wcex.hbrBackground	= NULL;
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= L"Screen";
	wcex.hIconSm		= NULL;
	RegisterClassEx(&wcex);
}
void Screen::SetRender(bool f) {
	fRender=f;
}
void Screen::ShowGrid(bool f) {
	if(f)
		iGridVisible=1;
	else
		iGridVisible=0;
}
void Screen::Size(int x,int y) {
	HDC hdc=GetDC(theApp->hwnd);
	SelectObject(hdcScreen,hbmpScreenOld);
	DeleteObject((HGDIOBJ)hbmpScreen);
	hbmpScreen=CreateCompatibleBitmap(hdc,x,y);
	ReleaseDC(theApp->hwnd,hdc);
	hbmpScreenOld=(HBITMAP)SelectObject(hdcScreen,hbmpScreen);
	SetWindowPos(hwnd,0,0,0,x,y,SWP_NOZORDER|SWP_NOMOVE);
}
void Screen::Pos(int x,int y) {
	SetWindowPos(hwnd,0,x,y,0,0,SWP_NOZORDER|SWP_NOSIZE);
}
void Screen::SetGrid(int x,int y) {
	iGridX=x;
	iGridY=y;
}
int Screen::GridX(int x) {
	if(iGridEnable==0 || iGridX==0 || ((GetAsyncKeyState(VK_MENU)&0x8000)!=0))
		return x;
	return (x+iGridX/2)/iGridX*iGridX;
}
int Screen::GridY(int y) {
	if(iGridEnable==0 || iGridY==0 || ((GetAsyncKeyState(VK_MENU)&0x8000)!=0))
		return y;
	return (y+iGridY/2)/iGridY*iGridY;
}
void Screen::DrawGuide(Graphics *g,PointF ptfZero,PointF ptfMin,PointF ptfMax,PointF ptfScr) {
	float fDash1[2];
	if(theApp->iDispGuide==0)
		return;
	fDash1[0]=1.f,fDash1[1]=5.f;
	Pen pen1(Color::White);
	pen1.SetDashPattern(fDash1,2);
	Pen pen2(Color::Gray);
	pen2.SetDashPattern(fDash1,2);
	pen2.SetDashOffset(3.0);
	g->DrawLine(&pen1,ptfZero.X,ptfMin.Y,ptfScr.X,ptfMin.Y);
	g->DrawLine(&pen2,ptfZero.X,ptfMin.Y,ptfScr.X,ptfMin.Y);
	g->DrawLine(&pen1,ptfZero.X,ptfMax.Y,ptfScr.X,ptfMax.Y);
	g->DrawLine(&pen2,ptfZero.X,ptfMax.Y,ptfScr.X,ptfMax.Y);
	g->DrawLine(&pen1,ptfMin.X,ptfZero.Y,ptfMin.X,ptfScr.Y);
	g->DrawLine(&pen2,ptfMin.X,ptfZero.Y,ptfMin.X,ptfScr.Y);
	g->DrawLine(&pen1,ptfMax.X,ptfZero.Y,ptfMax.X,ptfScr.Y);
	g->DrawLine(&pen2,ptfMax.X,ptfZero.Y,ptfMax.X,ptfScr.Y);
}
void Screen::DrawHandleMark(Graphics *g,PointF pt,int f,Color col) {
	int x,y;
	x=(int)(pt.X);
	y=(int)(pt.Y);
	switch(f) {
	case 0:
		g->FillRectangle(&SolidBrush(Color::Black),x-4,y-4,8,8);
		g->FillRectangle(&SolidBrush(col),x-3,y-3,6,6);
		break;
	case 1:
		g->FillRectangle(&SolidBrush(col),x-5,y-5,10,10);
		g->FillRectangle(&SolidBrush(Color::Black),x-4,y-4,8,8);
		break;
	case 2:
		g->FillEllipse(&SolidBrush(Color::Black),x-4,y-4,8,8);
		g->FillEllipse(&SolidBrush(col),x-3,y-3,6,6);
		break;
	case 3:
		g->FillRectangle(&SolidBrush(Color::Black),x-4,y-4,8,8);
		g->FillRectangle(&SolidBrush(col),x-3,y-3,6,6);
		g->DrawLine(&Pen(Color::Black),x-4,y-4,x+4,y+4);
		g->DrawLine(&Pen(Color::Black),x-4,y+4,x+4,y-4);
		break;
	}
}
PointF Mid(PointF x,PointF y) {
	x=x+y;
	x.X/=2;
	x.Y/=2;
	return x;
}
void Screen::TrimmingVisible(void) {
	RectF rc;
	Primitive *pr;
	theJournal->Record();
	rc.X=rc.Y=rc.Width=rc.Height=0.f;
	pr=theTree->GetFirst();
	if(pr) {
		for(;pr;pr=theTree->GetNext(pr)) {
			if(pr->iVisible&4) {
				if(rc.IsEmptyArea())
					rc=pr->rcOutline;
				else
					rc.Union(rc,rc,pr->rcOutline);
			}
		}
		CanvasSize((int)rc.Width,(int)rc.Height);
	}
	for(pr=theTree->GetFirst();pr;pr=theTree->GetNext(pr)) {
		theTree->MovePos(pr,-rc.X,-rc.Y);
	}
	theScreen->Send();
}
void Screen::Trimming(void) {
	RectF rc;
	Primitive *pr;
	theJournal->Record();
	rc.X=rc.Y=rc.Width=rc.Height=0.f;
	pr=theTree->GetFirst();
	if(pr) {
		rc=pr->rcOutline;
		for(;pr;pr=theTree->GetNext(pr)) {
			rc.Union(rc,rc,pr->rcOutline);
		}
		CanvasSize((int)rc.Width,(int)rc.Height);
	}
	for(pr=theTree->GetFirst();pr;pr=theTree->GetNext(pr)) {
		theTree->MovePos(pr,-rc.X,-rc.Y);
	}
	theScreen->Send();
}
void Screen::CanvasSize(int x,int y) {
	HDC hdc;
	BITMAPINFO bmi;
	fRenderBreak=true;
	EnterCriticalSection(&csData);
	iWidth=x;
	iHeight=y;
	SelectObject(hdcMem,hbmpMemOld);
	DeleteObject((HGDIOBJ)hbmpMem);
	SelectObject(hdcMem2,hbmpMem2Old);
	DeleteObject((HGDIOBJ)hbmpMem2);
	SelectObject(hdcMemPreview,hbmpMemPreviewOld);
	DeleteObject((HGDIOBJ)hbmpMemPreview);
	hdc=GetDC(theApp->hwnd);
	ZeroMemory(&bmi,sizeof(bmi));
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = iWidth;
	bmi.bmiHeader.biHeight = -iHeight;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;
	bmi.bmiHeader.biCompression=BI_RGB;
	hbmpMem=CreateDIBSection(NULL,&bmi,DIB_RGB_COLORS,(void**)&pdwMem,NULL,0);
	hbmpMemOld=(HBITMAP)SelectObject(hdcMem,hbmpMem);
	hbmpMem2=CreateDIBSection(NULL,&bmi,DIB_RGB_COLORS,(void**)&pdwMem2,NULL,0);
	hbmpMem2Old=(HBITMAP)SelectObject(hdcMem2,hbmpMem2);
	bmi.bmiHeader.biHeight=-iHeight/iOutScale*iAnimFrames;
	bmi.bmiHeader.biWidth=iWidth/iOutScale;
	hbmpMemPreview=CreateDIBSection(NULL,&bmi,DIB_RGB_COLORS,(void**)&pdwMemPreview,NULL,0);
	hbmpMemPreviewOld=(HBITMAP)SelectObject(hdcMemPreview,hbmpMemPreview);
	ReleaseDC(theApp->hwnd,hdc);
	theApp->Edit();
	Send();
	LeaveCriticalSection(&csData);
}
void Screen::DrawHandle(HDC hdc,Primitive *p) {
	int i;
	PointF ptfMin,ptfMax;
	if(p==NULL)
		return;
	Graphics *g=new Graphics(hdc);
	ptfMin.X=FMin(p->ptRect[0].X,p->ptRect[1].X,p->ptRect[2].X,p->ptRect[3].X);
	ptfMin.Y=FMin(p->ptRect[0].Y,p->ptRect[1].Y,p->ptRect[2].Y,p->ptRect[3].Y);
	ptfMax.X=FMax(p->ptRect[0].X,p->ptRect[1].X,p->ptRect[2].X,p->ptRect[3].X);
	ptfMax.Y=FMax(p->ptRect[0].Y,p->ptRect[1].Y,p->ptRect[2].Y,p->ptRect[3].Y);
	DrawGuide(g,LogicalToScreen(0.f,0.f),LogicalToScreen(ptfMin),LogicalToScreen(ptfMax),LogicalToScreen((float)iWidth,(float)iHeight));
	for(i=0;i<4;++i)
		DrawHandleMark(g,LogicalToScreen(p->ptRect[i]));
	delete g;
}
void Screen::DrawHandle(HDC hdc,PrimitivePtrList *prl) {
	PointF pt;
	if(prl==0 || prl->iCount==0)
		return;
	if(prl->iCount>1) {
		prl->RecalcRect();
		Graphics *g=new Graphics(hdc);
		pt=LogicalToScreen(prl->rc.X,prl->rc.Y);
		DrawHandleMark(g,pt,1);
		pt.X+=prl->rc.Width*fZoom;
		DrawHandleMark(g,pt,1);
		pt.Y+=prl->rc.Height*fZoom;
		DrawHandleMark(g,pt,1);
		pt.X-=prl->rc.Width*fZoom;
		DrawHandleMark(g,pt,1);
		delete g;
	}
	if(prl->iCount==1) {
		DrawHandle(hdc,theProp->prCurrent);
		return;
	}
	if(theProp->prCurrent && theProp->prCurrent->prType!=prtGroup) {
		if((GetAsyncKeyState(VK_CONTROL)&0x8000)||(GetAsyncKeyState(VK_SHIFT)&0x8000))
			DrawHandle(hdc,theProp->prCurrent);
	}
}
bool Screen::Check(float x,float y,PointF pt) {
	float dx=4.f/fZoom;
	if(x>=pt.X-dx&&x<pt.X+dx&&y>=pt.Y-dx&&y<pt.Y+dx)
		return true;
	return false;
}
//
// HitTest 
// iMode=0:CurrentSelected iMode=1:All iMode=2:Next
// b4:Group b3:Primitive b2:Hwndle b1:CornerY b0:CornerX
Primitive *NextPrim(Primitive *p,float x,float y) {
	Primitive *pOld,*p2;
	if(p==0)
		p=theTree->GetFirst();
	if(p==0)
		return 0;
	pOld=0;
	p2=p;
	for(;;) {
		if(p->IsLock()==0 && p->IsVisible(x,y))
			pOld=p;
		p=theTree->GetNext(p);
		if(p==0)
			p=theTree->GetFirst();
		if(p2==p)
			break;
	}
	return pOld;
}
int Screen::HitTest(Primitive **pp,tool t,int x,int y,int iMode,int iKeyState) {
	float fX,fY;
	int iRet;
	Primitive *p;
	PrimitivePtrList *prl;
	p=theTree->prlCurrent.pr;
	fX=(x-iDispOffX)/fZoom;
	fY=(y-iDispOffY)/fZoom;
	prl=&theTree->prlCurrent;
	p=theProp->prCurrent;
	iRet=0;
	switch(t) {
	case tArrow:
		if(iMode==0 && prl->iCount==0) {
				*pp=0;
				return 0;
		}
		if(iMode==0||iMode==1) {
			if(prl->iCount==1 || (prl->iCount>=1 && ((iKeyState&MK_CONTROL)||iKeyState&MK_SHIFT))) {
				if(p=theProp->prCurrent) {
					if(Check(fX,fY,p->ptRect[0]))
						return 4;
					if(Check(fX,fY,p->ptRect[1]))
						return 5;
					if(Check(fX,fY,p->ptRect[2]))
						return 7;
					if(Check(fX,fY,p->ptRect[3]))
						return 6;
					if(p->IsVisible(fX,fY)) {
						*pp=p;
						return 8;
					}
				}
			}
			if(prl->iCount>1) {
				if(Check(fX,fY,PointF(prl->rc.X,prl->rc.Y)))
					return 20;
				if(Check(fX,fY,PointF(prl->rc.X+prl->rc.Width,prl->rc.Y)))
					return 21;
				if(Check(fX,fY,PointF(prl->rc.X,prl->rc.Y+prl->rc.Height)))
					return 22;
				if(Check(fX,fY,PointF(prl->rc.X+prl->rc.Width,prl->rc.Y+prl->rc.Height)))
					return 23;
				if(prl->rc.Contains(fX,fY)) {
					*pp=0;
					p=NextPrim(theProp->prCurrent,fX,fY);
					if(p) {
						*pp=p;
						return 24;
					}
					return 16;
				}
			}
		}
		if(prl->iCount==0)
			p=0;
		if(!prl->rc.Contains(fX,fY))
			p=0;
		*pp=NextPrim(p,fX,fY);
		if(*pp==0)
			return 0;
		if(prl->Check(*pp))
			return 24;
		return 8;
	case tGradation:
		if(p==0)
			return 0;
		if((iKeyState&(MK_CONTROL|MK_SHIFT))==0) {
			if(Check(fX,fY,p->GradPoint(0)))
				return 128;
			if(Check(fX,fY,p->GradPoint(1)))
				return 128+32;
		}
		break;
	}
	return 0;
}
int Screen::HitTestHandle(tool t,int x,int y,Primitive *p) {
	float fX,fY;
	if(p==NULL)
		p=theTree->prlCurrent.pr;
	fX=(x-iDispOffX)/fZoom;
	fY=(y-iDispOffY)/fZoom;
	if(t==tArrow) {
		if(p==NULL)
			return 0;
		if(Check(fX,fY,p->ptRect[0]))
			return 4;
		if(Check(fX,fY,p->ptRect[1]))
			return 5;
		if(Check(fX,fY,p->ptRect[2]))
			return 7;
		if(Check(fX,fY,p->ptRect[3]))
			return 6;
		return 0;
	}
	if(t==tGradation) {
		PointF pt1=PointF(p->rcOutline.X+p->ptGrad[0].X*p->rcOutline.Width+theScreen->iDispOffX
					,p->rcOutline.Y+p->ptGrad[0].Y*p->rcOutline.Height+theScreen->iDispOffY);
		PointF pt2=PointF(p->rcOutline.X+p->ptGrad[1].X*p->rcOutline.Width+theScreen->iDispOffX
					,p->rcOutline.Y+p->ptGrad[1].Y*p->rcOutline.Height+theScreen->iDispOffY);
		return 0;
	}
	return 0;
}
void Screen::WaitRender(void) {
}
void Screen::DrawThread(void *p) {
	Screen *sc=(Screen*)p;
	for(;;) {
		SetEvent(sc->hRenderWaiting);
		WaitForSingleObject(sc->hDrawRequest,INFINITE);
		WaitForSingleObject(sc->hRenderWaiting2,INFINITE);
		sc->fRenderBreak=false;
		ResetEvent(sc->hRenderWaiting);
		sc->Draw(true);
		if(sc->iExitDrawThread) {
			sc->iExitDrawThread=2;
			break;
		}
	}
}
void Screen::Draw(bool bBgFill) {
	Primitive *p,*pp;
	Graphics *g;
	int iVisibleMode;
	if(hdcMem==NULL)
		return;
	EnterCriticalSection(&csData);
	g=new Graphics(hdcMem);
	if(bBgFill) {
		g->FillRectangle(&SolidBrush(colBackground),0,0,iWidth,iHeight);
	}
	else {
		ZeroMemory(pdwMem,sizeof(DWORD)*iWidth*iHeight);
	}
	delete g;
	iVisibleMode=1;
	for(p=theTree->GetFirst();p;p=theTree->GetNext(p)) {
		p->iVisible|=4;
		for(pp=p;pp;pp=theTree->GetParent(pp)) {
			if((pp->iVisible&1)==0)
				p->iVisible&=~4;
		}
		if(p->iVisible&2)
			iVisibleMode=2;
	}
	if(iVisibleMode==2) {
		for(p=theTree->GetFirst();p;p=theTree->GetNext(p)) {
			p->iVisible&=~4;
			for(pp=p;pp;pp=theTree->GetParent(pp)) {
				if(pp->iVisible&2) {
					p->iVisible|=4;
					break;
				}
				else if((pp->iVisible&1)==0) {
					break;
				}
			}
		}
	}
	p=theTree->GetFirst();
	if(p) {
		do {
			p->Assemble();
			if(fRenderBreak) {
				LeaveCriticalSection(&csData);
				return;
			}
		} while((p=theTree->GetNext(p))!=NULL);
		SetEvent(hDrawRequest2);
	}
	BitBlt(hdcMem2,0,0,iWidth,iHeight,hdcMem,0,0,SRCCOPY);
	LeaveCriticalSection(&csData);
	InvalidateRect(hwnd,NULL,false);
}
void Screen::DrawInvertRect(int x1,int y1,int x2,int y2) {
	x1=(int)(x1*fZoom+iDispOffX);
	x2=(int)(x2*fZoom+iDispOffX);
	y1=(int)(y1*fZoom+iDispOffY);
	y2=(int)(y2*fZoom+iDispOffY);
	::InvertRect(hwnd,x1,y1,x2,y2);
}
void Screen::Zoom1(void) {
	fZoom=1.f;
	InvalidateRect(theApp->hwnd,NULL,false);
}
void Screen::Zoom2(void) {
	fZoom=2.f;
	InvalidateRect(theApp->hwnd,NULL,false);
}
void Screen::ZoomIn(void) {
	if(fZoom<16.f)
		fZoom*=2.f;
	InvalidateRect(theApp->hwnd,NULL,false);
}
void Screen::ZoomOut(void) {
	if(fZoom>0.1f)
		fZoom*=0.5f;
	InvalidateRect(theApp->hwnd,NULL,false);
}
void Screen::GridVisible(int i) {
	iGridVisible=i;
	InvalidateRect(theApp->hwnd,NULL,false);
}
void Screen::ToggleGridVisible(void) {
	GridVisible(iGridVisible^1);
	theCmdBar->SetGridVisible(iGridVisible);
}
void Screen::GridEnable(int i) {
	iGridEnable=i;
}
void GridPoint(HDC hdc,int x,int y,int w) {
	DWORD dwCol=GetPixel(hdc,x,y);
	DWORD dwColPt;
	if(((dwCol>>8)&0xff)>0x80)
		dwColPt=0;
	else
		dwColPt=0xffffff;
	if(w<=8)
		SetPixel(hdc,x,y,dwColPt);
	else {
		SetPixel(hdc,x-1,y,dwColPt);
		SetPixel(hdc,x,y,dwColPt);
		SetPixel(hdc,x+1,y,dwColPt);
		SetPixel(hdc,x,y-1,dwColPt);
		SetPixel(hdc,x,y+1,dwColPt);
	}
}
PointF Screen::ScreenToLogical(PointF ptf) {
	return PointF((ptf.X-iDispOffX)/fZoom,(ptf.Y-iDispOffY)/fZoom);
}
PointF Screen::LogicalToScreen(float x,float y) {
	return PointF(x*fZoom+iDispOffX,y*fZoom+iDispOffY);
}
PointF Screen::LogicalToScreen(PointF ptf) {
	return PointF(ptf.X*fZoom+iDispOffX,ptf.Y*fZoom+iDispOffY);
}
void Screen::Paint(HDC hdc,RECT *prc) {
	int i,x,y,xx,yy,iW;
	Primitive *p;
	PointF pt1,pt2;
	int iShift,iControl;
	RECT rc;
	HBRUSH brWorkspace=CreateSolidBrush(theScreen->colWorkspace.ToCOLORREF());
	iW=(int)(min(iGridX,iGridY)*fZoom);
	FillRect(hdcScreen,prc,brWorkspace);
	DeleteObject((HGDIOBJ)brWorkspace);
	iDispOffX=(int)(prc->right/2-(iWidth/2+iOffsetX)*fZoom);
	iDispOffY=(int)(prc->bottom/2-(iHeight/2+iOffsetY)*fZoom);
	if(iFramesX>1 || iFramesY>1) {
		for(i=1;i<iFramesX;++i) {
			rc.left=iDispOffX+(iWidth*i/iFramesX)*fZoom;
			rc.right=rc.left+1;
			rc.top=iDispOffY-16;
			rc.bottom=iDispOffY+(iHeight*fZoom)+16;
			FillRect(hdcScreen,&rc,(HBRUSH)GetStockObject(BLACK_BRUSH));
		}
		for(i=1;i<iFramesY;++i) {
			rc.left=iDispOffX-16;
			rc.right=iDispOffX+(iWidth*fZoom)+16;
			rc.top=iDispOffY+(iHeight*i/iFramesY)*fZoom;
			rc.bottom=rc.top+1;
			FillRect(hdcScreen,&rc,(HBRUSH)GetStockObject(BLACK_BRUSH));
		}
	}
	EnterCriticalSection(&csData);
	SetStretchBltMode(hdcScreen,COLORONCOLOR);
	StretchBlt(hdcScreen,iDispOffX,iDispOffY,(int)(iWidth*fZoom),(int)(iHeight*fZoom),hdcMem2,0,0,iWidth,iHeight,SRCCOPY);
	LeaveCriticalSection(&csData);
	switch(theTool->toolCurrent) {
	case tEdit:
		if(theTree->prlCurrent.pr && (theTree->prlCurrent.pr->prType==prtShape||theTree->prlCurrent.pr->prType==prtText))
			theTree->prlCurrent.pr->bz->Draw(hdcScreen,theTree->prlCurrent.pr->ptfOrg[0],theTree->prlCurrent.pr->ptfOrg[1],theTree->prlCurrent.pr->mxRot);
		break;
	case tCurve:
	case tShape:
		bzTemp->Draw(hdcScreen);
		break;
	case tGradation:
		p=theProp->prCurrent;
		if(p) {
			Graphics g(hdcScreen);
			iShift=GetAsyncKeyState(VK_SHIFT)&0x8000;
			iControl=GetAsyncKeyState(VK_CONTROL)&0x8000;
			if(iShift) {
				pt1=LogicalToScreen(p->GradPoint(2));
				DrawHandleMark(&g,pt1,3,Color::Green);
			}
			if(iControl) {
				pt1=LogicalToScreen(p->GradPoint(3));
				DrawHandleMark(&g,pt1,3,Color::Blue);
			}
			if(iShift==0 && iControl==0) {
				pt1=LogicalToScreen(p->GradPoint(0));
				pt2=LogicalToScreen(p->GradPoint(1));
				g.DrawLine(&Pen(Color(0,0,0)),pt1,pt2);
				DrawHandleMark(&g,pt1,3,Color::LightPink);
				DrawHandleMark(&g,pt2,0,Color::LightPink);
			}
		}
		break;
	default:
		DrawHandle(hdcScreen,&theTree->prlCurrent);
		break;
	}
	if(iGridVisible) {
		if(iGridX>1 && iGridY>1) {
			for(y=0;y<iHeight;y+=iGridY) {
				yy=iDispOffY+(int)(y*fZoom);
				for(x=0;x<iWidth;x+=iGridX) {
					xx=iDispOffX+(int)(x*fZoom);
					GridPoint(hdcScreen,xx,yy,iW);
				}
			}
		}
	}
	BitBlt(hdc,prc->left,prc->top,prc->right,prc->bottom,hdcScreen,0,0,SRCCOPY);
}
class Zoomer {
	PointF *ptfTab;
	float *fWidthTab;
	float *fRoundTab;
	float *fFontSizeTab;
	float *fFontAspectTab;
	float *fAngleTab;
	float *fSlantTab;
	PointF ptfOrg,ptCur,ptDiff;
	PrimitivePtrList *prlOrg;
	PrimitivePtrList prlTemp;
	float rAngle,fSlant;
	float fFontSize;
	PointF ptfFix;
	int iDrag;
	int iAllText;
public:
	Zoomer(Primitive *pr,PointF ptfOrgInit,PointF ptCurInit,float rA,float fS,int iDragInit,int *sizex=0,int *sizey=0) {
		memset(this,0,sizeof(Zoomer));
		prlTemp.Clear();
		prlTemp.Add(pr);
		iDrag=iDragInit;
		prlOrg=&prlTemp;
		ptfOrg=ptfOrgInit;
		ptCur=ptCurInit;
		ptDiff=PointF(0,0);
		ptfTab=new PointF[2];
		fWidthTab=fRoundTab=0;
		ptfTab[0]=pr->ptfOrg[0];
		ptfTab[1]=pr->ptfOrg[1];
		fFontSize=pr->fFontSize;
		rAngle=rA;
		fSlant=fS;
		switch(iDrag) {
		case 4:
			ptfFix=pr->ptRect[2];
			break;
		case 5:
			ptfFix=pr->ptRect[3];
			break;
		case 6:
			ptfFix=pr->ptRect[1];
			break;
		case 7:
			ptfFix=pr->ptRect[0];
			break;
		}
		if(sizex) {
			*sizex=FMax(pr->ptRect[0].X,pr->ptRect[1].X,pr->ptRect[2].X,pr->ptRect[3].X)
				-FMin(pr->ptRect[0].X,pr->ptRect[1].X,pr->ptRect[2].X,pr->ptRect[3].X);
			*sizey=FMax(pr->ptRect[0].Y,pr->ptRect[1].Y,pr->ptRect[2].Y,pr->ptRect[3].Y)
				-FMin(pr->ptRect[0].Y,pr->ptRect[1].Y,pr->ptRect[2].Y,pr->ptRect[3].Y);
		}
	}
	Zoomer(PrimitivePtrList *prlInit,PointF ptfOrgInit,PointF ptCurInit,int iDragInit,int iAllTextInit=0) {
		int i;
		float t;
		PointF ptfCenter;
		Matrix mx;
		memset(this,0,sizeof(Zoomer));
		iAllText=iAllTextInit;
		iDrag=iDragInit;
		prlOrg=prlInit;
		PrimitivePtrList *prl;
		ptfOrg=ptfOrgInit;
		ptCur=ptCurInit;
		if((iDrag&1)==0)
			t=ptfOrg.X,ptfOrg.X=ptCur.X,ptCur.X=t;
		if((iDrag&2)==0)
			t=ptfOrg.Y,ptfOrg.Y=ptCur.Y,ptCur.Y=t;
		ptDiff=PointF(0,0);//ptOpp-ptfOrg;
		ptfTab=new PointF[prlOrg->iCount*3];
		fWidthTab=new float[prlOrg->iCount];
		fRoundTab=new float[prlOrg->iCount*8];
		fFontSizeTab=new float[prlOrg->iCount];
		fFontAspectTab=new float[prlOrg->iCount];
		fAngleTab=new float[prlOrg->iCount];
		fSlantTab=new float[prlOrg->iCount];
		for(i=0,prl=prlOrg;prl->next;++i) {
			fFontAspectTab[i]=prl->pr->fFontAspect;
			fFontSizeTab[i]=prl->pr->fFontSize;
			fAngleTab[i]=prl->pr->rAngle;
			fSlantTab[i]=prl->pr->fSlant;
			fWidthTab[i]=prl->pr->fWidth;
			fRoundTab[i*8]=prl->pr->fRoundX1;
			fRoundTab[i*8+1]=prl->pr->fRoundX2;
			fRoundTab[i*8+2]=prl->pr->fRoundX3;
			fRoundTab[i*8+3]=prl->pr->fRoundX4;
			fRoundTab[i*8+4]=prl->pr->fRoundY1;
			fRoundTab[i*8+5]=prl->pr->fRoundY2;
			fRoundTab[i*8+6]=prl->pr->fRoundY3;
			fRoundTab[i*8+7]=prl->pr->fRoundY4;
			ptfCenter=GetCenter(prl->pr->ptfOrg[0],prl->pr->ptfOrg[1]);
			mx.Reset();
			mx.Translate(-ptfCenter.X,-ptfCenter.Y);
			mx.Shear(-prl->pr->fSlant*.01f,0,MatrixOrderAppend);
			mx.Rotate(prl->pr->rAngle,MatrixOrderAppend);
			mx.Translate(ptfCenter.X,ptfCenter.Y,MatrixOrderAppend);
			ptfTab[i*3]=prl->pr->ptfOrg[0];
			ptfTab[i*3+1]=prl->pr->ptfOrg[1];
			ptfTab[i*3+2].X=ptfTab[i*3+1].X;
			ptfTab[i*3+2].Y=ptfTab[i*3].Y;
			mx.TransformPoints(&ptfTab[i*3],3);
			ptfTab[i*3]=ptfTab[i*3]-ptfOrg;
			ptfTab[i*3+1]=ptfTab[i*3+1]-ptfOrg;
			ptfTab[i*3+2]=ptfTab[i*3+2]-ptfOrg;
			prl=prl->next;
		}
	}
	~Zoomer(void) {
		delete[] ptfTab;
		if(fWidthTab)
			delete[] fWidthTab;
		if(fRoundTab)
			delete[] fRoundTab;
		if(fFontSizeTab)
			delete[] fFontSizeTab;
		if(fFontAspectTab)
			delete[] fFontAspectTab;
		if(fAngleTab)
			delete[] fAngleTab;
		if(fSlantTab)
			delete[] fSlantTab;
	}
	void MoveOne(PointF ptf,int *sx=0,int *sy=0) {
		Matrix mx;
		PointF ptfCenter;
		PointF ptf2;
		float t,fDis,fDisCur;
		ptf=ptf-ptDiff;
		ptfCenter=GetCenter(ptfFix,ptf);
		ptf2=ptfFix;
		if(rAngle!=0.f || fSlant!=0.f) {
			mx.Translate(-ptfCenter.X,-ptfCenter.Y);
			mx.Rotate(-rAngle,MatrixOrderAppend);
			mx.Shear(fSlant*0.01f,0,MatrixOrderAppend);
			mx.Translate(ptfCenter.X,ptfCenter.Y,MatrixOrderAppend);
			mx.TransformPoints(&ptf);
			mx.TransformPoints(&ptf2);
		}
		if((iDrag&1)==0)
			t=ptf2.X,ptf2.X=ptf.X,ptf.X=t;
		if((iDrag&2)==0)
			t=ptf2.Y,ptf2.Y=ptf.Y,ptf.Y=t;
		if(prlOrg->pr->prType==prtText && prlOrg->pr->iUseTextPath && prlOrg->pr->iAutoSize) {
			fDis=sqrtf((ptf.X-ptfOrg.X)*(ptf.X-ptfOrg.X)+(ptf.Y-ptfOrg.Y)*(ptf.Y-ptfOrg.Y));
			fDisCur=sqrtf((ptCur.X-ptfOrg.X)*(ptCur.X-ptfOrg.X)+(ptCur.Y-ptfOrg.Y)*(ptCur.Y-ptfOrg.Y));
			if(fDisCur)
				prlOrg->pr->fFontSize=fFontSize*fDis/fDisCur;
			else
				prlOrg->pr->fFontSize=1.f;
		}
		prlOrg->pr->SetPos(ptf2.X,ptf2.Y,ptf.X-ptf2.X,ptf.Y-ptf2.Y,1);
		if(sx)
			*sx=ptf.X-ptf2.X,*sy=ptf.Y-ptf2.Y;
	}
	void Move(PointF ptf) {
		int i;
		double dZX,dZY,dZXY;
		Matrix mx;
		PointF ptfCenter,ptfWork,ptfOrg2[3],ptfDiffAng;
		PrimitivePtrList *prl;
		float fNewAngle,fNewSlant;
		ptf=ptf-ptDiff;
//		if(abs(ptf.X-ptfOrg.X)<1.f)
//			ptf.X=ptfOrg.X+1.f;
//		if(abs(ptf.Y-ptfOrg.Y)<1.f)
//			ptf.Y=ptfOrg.Y+1.f;
		ptfCenter=GetCenter(ptfFix,ptf);
		for(i=0,prl=prlOrg;prl->next;++i) {
			ptfWork=ptf;
			dZX=(ptfWork.X-ptfOrg.X)/(ptCur.X-ptfOrg.X);
			dZY=(ptfWork.Y-ptfOrg.Y)/(ptCur.Y-ptfOrg.Y);
			dZXY=sqrt(abs(dZX*dZY));
			prl->pr->fWidth=fWidthTab[i]*dZXY;
			if(prl->pr->iRoundXYSeparate) {
				double dAZX=abs(dZX),dAZY=abs(dZY);
				prl->pr->fRoundX1=fRoundTab[i*8]*dAZX;
				prl->pr->fRoundX2=fRoundTab[i*8+1]*dAZX;
				prl->pr->fRoundX3=fRoundTab[i*8+2]*dAZX;
				prl->pr->fRoundX4=fRoundTab[i*8+3]*dAZX;
				prl->pr->fRoundY1=fRoundTab[i*8+4]*dAZY;
				prl->pr->fRoundY2=fRoundTab[i*8+5]*dAZY;
				prl->pr->fRoundY3=fRoundTab[i*8+6]*dAZY;
				prl->pr->fRoundY4=fRoundTab[i*8+7]*dAZY;
			}
			else {
				prl->pr->fRoundX1=fRoundTab[i*8]*dZXY;
				prl->pr->fRoundX2=fRoundTab[i*8+1]*dZXY;
				prl->pr->fRoundX3=fRoundTab[i*8+2]*dZXY;
				prl->pr->fRoundX4=fRoundTab[i*8+3]*dZXY;
				prl->pr->fRoundY1=fRoundTab[i*8+4]*dZXY;
				prl->pr->fRoundY2=fRoundTab[i*8+5]*dZXY;
				prl->pr->fRoundY3=fRoundTab[i*8+6]*dZXY;
				prl->pr->fRoundY4=fRoundTab[i*8+7]*dZXY;
			}
			ptfOrg2[0].X=(ptfTab[i*3].X)*dZX+ptfOrg.X;
			ptfOrg2[0].Y=(ptfTab[i*3].Y)*dZY+ptfOrg.Y;
			ptfOrg2[1].X=(ptfTab[i*3+1].X)*dZX+ptfOrg.X;
			ptfOrg2[1].Y=(ptfTab[i*3+1].Y)*dZY+ptfOrg.Y;
			ptfOrg2[2].X=(ptfTab[i*3+2].X)*dZX+ptfOrg.X;
			ptfOrg2[2].Y=(ptfTab[i*3+2].Y)*dZY+ptfOrg.Y;
			ptfCenter=GetCenter(ptfOrg2[0],ptfOrg2[1]);
			ptfDiffAng=ptfOrg2[2]-ptfOrg2[0];
			fNewAngle=atan2(ptfDiffAng.Y,ptfDiffAng.X)*180.f/PI;
			ptfDiffAng=ptfOrg2[1]-ptfOrg2[2];
			fNewSlant=atan2(ptfDiffAng.Y,ptfDiffAng.X);
			fNewSlant=fNewSlant*180.f/PI-fNewAngle-90.f;
			fNewSlant=tan(fNewSlant*PI/180.f)*100.f;
			mx.Reset();
			mx.Translate(-ptfCenter.X,-ptfCenter.Y);
			mx.Rotate(-fNewAngle,MatrixOrderAppend);
			mx.Shear(fNewSlant*.01f,0,MatrixOrderAppend);
			mx.Translate(ptfCenter.X,ptfCenter.Y,MatrixOrderAppend);
			mx.TransformPoints(ptfOrg2,3);
			if(prl->pr->prType==prtText) {
				if(prl->pr->iUseTextPath) {
					prl->pr->fFontSize=fFontSizeTab[i]*dZXY;
				}
				if(iAllText && dZY!=0.0 && prl->pr->iUseTextPath==0) {
					prl->pr->fFontSize=fFontSizeTab[i]*dZY;
					prl->pr->fFontAspect=fFontAspectTab[i]/dZY*dZY;
				}
			}
			prl->pr->rAngle=fNewAngle;
			prl->pr->fSlant=fNewSlant;
			prl->pr->SetPos(ptfOrg2[0].X,ptfOrg2[0].Y,ptfOrg2[1].X-ptfOrg2[0].X,ptfOrg2[1].Y-ptfOrg2[0].Y,1);
			prl=prl->next;
		}
	}
};
Zoomer *theZoomer;

int iSizeX=100;
int iSizeY=100;
int iAllText=1;
BOOL CALLBACK dlgprocTransform(HWND hwnd,UINT uMsg,WPARAM wparam,LPARAM lparam) {
	switch(uMsg) {
	case WM_INITDIALOG:
		SetDlgItemInt(hwnd,IDC_ZOOMX,iSizeX,false);
		SetDlgItemInt(hwnd,IDC_ZOOMY,iSizeY,false);
		CheckButton(hwnd,IDC_ALLTEXT,iAllText);
		break;
	case WM_COMMAND:
		switch(wparam) {
		case IDCANCEL:
			EndDialog(hwnd,0);
			break;
		case IDOK:
			iSizeX=GetDlgItemInt(hwnd,IDC_ZOOMX,0,false);
			iSizeY=GetDlgItemInt(hwnd,IDC_ZOOMY,0,false);
			iAllText=IsDlgButtonChecked(hwnd,IDC_ALLTEXT);
			EndDialog(hwnd,1);
			break;
		}
		break;
	}
	return FALSE;
}
void Transform(void) {
	HideTools hide;
	hide.Hide();
	if(DialogBox(theApp->hinst,MAKEINTRESOURCE(IDD_TRANSFORM),theApp->hwnd,(DLGPROC)dlgprocTransform)) {
		PointF ptf1,ptf2;
		theJournal->Record();
		theApp->Edit();
		ptf1=PointF(theTree->prlCurrent.rc.X,theTree->prlCurrent.rc.Y);
		ptf2=PointF(theTree->prlCurrent.rc.Width,theTree->prlCurrent.rc.Height)+ptf1;
		Zoomer zm(&theTree->prlCurrent,ptf1,ptf2,23,iAllText);
		zm.Move(PointF(ptf1.X+(ptf2.X-ptf1.X)*iSizeX*.01f,ptf1.Y+(ptf2.Y-ptf1.Y)*iSizeY*.01f));
		theScreen->Send();
		InvalidateRect(theScreen->hwnd,NULL,false);
	}
	hide.Restore();
}
BOOL CALLBACK dlgprocRename(HWND hdlg,UINT uMsg,WPARAM wparam,LPARAM lparam) {
	static wchar_t *str;
	switch(uMsg) {
	case WM_INITDIALOG:
		str=(wchar_t*)lparam;
		SetDlgItemText(hdlg,IDC_NAME,str);
		break;
	case WM_COMMAND:
		switch(wparam) {
		case IDOK:
			GetDlgItemText(hdlg,IDC_NAME,str,64);
			EndDialog(hdlg,1);
			break;
		case IDCANCEL:
			EndDialog(hdlg,0);
			break;
		}
		break;
	}
	return FALSE;
}
void GroupRename(void) {
	Primitive *pr;
	wchar_t str[256];
	HideTools hide;
	pr=theTree->prlCurrent.pr;
	if(pr) {
		wcscpy(str,pr->strName);
		hide.Hide();
		if(DialogBoxParam(theApp->hinst,MAKEINTRESOURCE(IDD_RENAME),theApp->hwnd,(DLGPROC)dlgprocRename,(LPARAM)str)) {
			theJournal->Record();
			theApp->Edit();
			wcscpy(pr->strName,str);
			theTree->SetName(pr);
		}
		hide.Restore();
	}
}
void GroupMirrorH(void) {
	PrimitivePtrList *prl;
	float fCX;
	PointF ptf[2];
	prl=&theTree->prlCurrent;
	fCX=prl->rc.X+prl->rc.Width*0.5f;
	while(prl->next) {
		ptf[0].X=fCX+fCX-prl->pr->ptfOrg[1].X;
		ptf[0].Y=prl->pr->ptfOrg[0].Y;
		ptf[1].X=fCX+fCX-prl->pr->ptfOrg[0].X;
		ptf[1].Y=prl->pr->ptfOrg[1].Y;
		prl->pr->rAngle=-prl->pr->rAngle;
		if(prl->pr->prType==prtText && prl->pr->iAutoSize==0)
			prl->pr->fFontAspect=-prl->pr->fFontAspect;
		prl->pr->SetPos(ptf[1].X,ptf[0].Y,ptf[0].X-ptf[1].X,ptf[1].Y-ptf[0].Y,1);
		prl=prl->next;
	}
	theProp->Setup(theTree->prlCurrent.pr);
	theScreen->Send();
	InvalidateRect(theScreen->hwnd,NULL,false);
}
void GroupMirrorV(void) {
	PrimitivePtrList *prl;
	float fCY;
	PointF ptf[2];
	prl=&theTree->prlCurrent;
	fCY=prl->rc.Y+prl->rc.Height*0.5f;
	while(prl->next) {
		ptf[0].X=prl->pr->ptfOrg[0].X;
		ptf[0].Y=fCY+fCY-prl->pr->ptfOrg[1].Y;
		ptf[1].X=prl->pr->ptfOrg[1].X;
		ptf[1].Y=fCY+fCY-prl->pr->ptfOrg[0].Y;
		prl->pr->rAngle=-prl->pr->rAngle;
		if(prl->pr->prType==prtText && prl->pr->iAutoSize==0)
			prl->pr->fFontSize=-prl->pr->fFontSize;
		prl->pr->SetPos(ptf[0].X,ptf[1].Y,ptf[1].X-ptf[0].X,ptf[0].Y-ptf[1].Y,1);
		prl=prl->next;
	}
	theProp->Setup(theTree->prlCurrent.pr);
	theScreen->Send();
	InvalidateRect(theScreen->hwnd,NULL,false);
}
void ExtractImage(wchar_t *strName) {
	static wchar_t strExportFile[MAX_PATH];
	OPENFILENAME ofn;
	HideTools hide;
	memset(&ofn,0,sizeof(ofn));
	wcscpy(strExportFile,strName);
	ofn.lStructSize=sizeof(ofn);
	ofn.hwndOwner=theApp->hwnd;
	ofn.hInstance=hinstMain;
	ofn.lpstrFilter=L"All Images\0*.bmp;*.png;*.jpeg;*.jpg;*.gif\0BMP\0*.bmp\0PNG\0*.png\0JPG/JPEG\0*.jpg;*.jpeg\0GIF\0*.gif\0All Files\0*.*\0";
	ofn.lpstrFile=strExportFile;
	ofn.nFilterIndex=1;
	ofn.lpstrTitle=L"Extract Images/Knob to File";//theLang->GetID(MSG_DIALOGEXPORTIMAGE);
	ofn.lpstrInitialDir=theApp->strDocDir;
	ofn.lpstrDefExt=L"";
	ofn.nMaxFile=MAX_PATH;
	ofn.Flags=OFN_HIDEREADONLY|OFN_EXPLORER;
	ofn.Flags|=OFN_OVERWRITEPROMPT;
	hide.Hide();
	if(GetSaveFileName(&ofn)) {
		CopyFile(strName,ofn.lpstrFile,false);
	}
	hide.Restore();
}
int DoPrimitiveMenu(int id) {
	Primitive *pr;
	wchar_t strName[MAX_PATH];
	switch(id) {
	case ID_PRIMITIVE_GROUP:
		Group();
		break;
	case ID_PRIMITIVE_UNGROUP:
		UnGroup();
		break;
	case ID_PRIMITIVE_MIRRORHORIZONTAL:
		theJournal->Record();
		theApp->Edit();
		GroupMirrorH();
		break;
	case ID_PRIMITIVE_MIRRORVERTICAL:
		theJournal->Record();
		theApp->Edit();
		GroupMirrorV();
		break;
	case ID_PRIMITIVE_MOVETOPARENT:
		theTree->MoveParent();
		break;
	case ID_PRIMITIVE_MOVETOBACKGROUND:
		theTree->MoveUp();
		break;
	case ID_PRIMITIVE_MOVETOFOREGROUND:
		theTree->MoveDown();
		break;
	case ID_PRIMITIVE_MOVETOTOPMOST:
		theTree->TopMost();
		break;
	case ID_PRIMITIVE_RENAME:
		GroupRename();
		break;
	case ID_PRIMITIVE_PASTECOLOR:
		PasteColor();
		break;
	case ID_PRIMITIVE_PASTEEFFECTS:
		PasteEffects();
		break;
	case ID_PRIMITIVE_DELETEPRIMITIVE:
		DelPrimitive();
		break;
	case ID_PRIMITIVE_CUTPRIMITIVE:
		CutPrimitive();
		break;
	case ID_PRIMITIVE_COPYPRIMITIVE:
		CopyPrimitive();
		break;
	case ID_PRIMITIVE_COPYCOODINATE:
		CopyCoodinate();
		break;
	case ID_PRIMITIVE_PASTEPRIMITIVE:
		PastePrimitive(false,true);
		break;
	case ID_PRIMITIVE_TRANSFORM:
		Transform();
		break;
	case ID_PRIMITIVE_MAKECIRCLE:
		pr=theProp->prCurrent;
		if(pr && pr->prType==prtShape || pr->prType==prtText) {
			pr->MakeCircle();
		}
		break;
	case ID_PRIMITIVE_EDITSHAPE:
		pr=theProp->prCurrent;
		if(pr && pr->prType==prtShape || pr->prType==prtText) {
			pr->Normalize();
			theTool->Select(tEdit);
		}
		break;
	case ID_PRIMITIVE_EDITKNOB:
		pr=theTree->prlCurrent.pr;
		if(pr&&pr->prType==prtImage) {
			wcscpy(strName,pr->strFile);
			if(IsEmbed(strName))
				wcscpy(strName,EmbedNameToTempName(pr->strFile));
			if(PathFileExists(strName))
				ShellExecute(NULL,L"",strName,0,0,SW_SHOWNORMAL);
		}
		break;
	case ID_PRIMITIVE_EXTRACTFILE:
		pr=theTree->prlCurrent.pr;
		if(pr&&pr->prType==prtImage) {
			wcscpy(strName,pr->strFile);
			if(IsEmbed(strName))
				wcscpy(strName,EmbedNameToTempName(pr->strFile));
			if(PathFileExists(strName)) {
				ExtractImage(strName);
			}
		}
		break;
	case ID_PRIMITIVE_REFRESH:
		RefreshAll();
		break;
	case ID_PRIMITIVE_ALIGNCENTER:
		theTree->Align(1);
		theTree->Align(5);
		break;
	case ID_PRIMITIVE_ALIGNHORZL:
		theTree->Align(0);
		break;
	case ID_PRIMITIVE_ALIGNHORZC:
		theTree->Align(1);
		break;
	case ID_PRIMITIVE_ALIGNHORZR:
		theTree->Align(2);
		break;
	case ID_PRIMITIVE_ALIGNVERTT:
		theTree->Align(4);
		break;
	case ID_PRIMITIVE_ALIGNVERTC:
		theTree->Align(5);
		break;
	case ID_PRIMITIVE_ALIGNVERTB:
		theTree->Align(6);
		break;
	case ID_PRIMITIVE_DISTRIBUTEHORZL:
		theTree->Distribution(0);
		break;
	case ID_PRIMITIVE_DISTRIBUTEHORZC:
		theTree->Distribution(1);
		break;
	case ID_PRIMITIVE_DISTRIBUTEHORZR:
		theTree->Distribution(2);
		break;
	case ID_PRIMITIVE_DISTRIBUTEVERTT:
		theTree->Distribution(4);
		break;
	case ID_PRIMITIVE_DISTRIBUTEVERTC:
		theTree->Distribution(5);
		break;
	case ID_PRIMITIVE_DISTRIBUTEVERTB:
		theTree->Distribution(6);
		break;
	default:
		return 0;
	}
	return 1;
}
void RClickMenu(HWND hwnd,POINT pt,Primitive *pr) {
	HMENU hmenu;
	int id;
	hmenu=GetMenu(theApp->hwnd);
	hmenu=GetSubMenu(hmenu,1);
	hmenu=GetSubMenu(hmenu,6);
	id=TrackPopupMenu(hmenu,TPM_LEFTALIGN|TPM_TOPALIGN|TPM_RETURNCMD,pt.x,pt.y,0,hwnd,NULL);
	DoPrimitiveMenu(id);
}
void Screen::SetGradPoint(int iDrag,Primitive *pr,float xOrg,float yOrg,float xCur,float yCur,WPARAM wparam) {
	RectF *prcf=&pr->rcOutline;
	if((wparam&(MK_SHIFT|MK_CONTROL))==0) {
		switch(iDrag) {
		case 128:
			pr->ptGrad[0].X=(xCur-prcf->X)/prcf->Width;
			pr->ptGrad[0].Y=(yCur-prcf->Y)/prcf->Height;
			break;
		case 128+32:
			pr->ptGrad[1].X=(xCur-prcf->X)/prcf->Width;
			pr->ptGrad[1].Y=(yCur-prcf->Y)/prcf->Height;
			break;
		default:
			pr->ptGrad[0].X=(xOrg-prcf->X)/prcf->Width;
			pr->ptGrad[0].Y=(yOrg-prcf->Y)/prcf->Height;
			pr->ptGrad[1].X=(xCur-prcf->X)/prcf->Width;
			pr->ptGrad[1].Y=(yCur-prcf->Y)/prcf->Height;
			break;
		}
	}
	else {
		if(wparam&MK_SHIFT) {
			pr->ptSpec[0].X=(xCur-prcf->X)/prcf->Width;
			pr->ptSpec[0].Y=(yCur-prcf->Y)/prcf->Height;
		}
		if(wparam&MK_CONTROL) {
			pr->ptSpec[1].X=(xCur-prcf->X)/prcf->Width;
			pr->ptSpec[1].Y=(yCur-prcf->Y)/prcf->Height;
		}
	}
}
LRESULT CALLBACK Screen::wndproc(HWND hwnd,UINT message,WPARAM wparam,LPARAM lparam) {
	HDC hdc;
	PAINTSTRUCT ps;
	RECT rcClient;
	CREATESTRUCT *lp;
	wchar_t str[128];
	PrimitivePtrList *prl;
	Primitive *pr;
	HMENU hmenu;
	int sx,sy,iHit;
	POINT pt;
	int t,px,py,xx,sizex,sizey,gdx,gdy;
	float fxx,fyy,fxx2,fyy2;
	int xCur1,yCur1,xCur2,yCur2;
	int ddx,ddy;
	static int x,y,xOrg,yOrg,xScrOrg,yScrOrg,gx,gy,gxOrg,gyOrg,dx,dy,dxOld,dyOld;
	static int xCur,yCur,xScrCur,yScrCur;
	static int xOld1,yOld1,xOld2,yOld2;
	static int iDrag,iStartMove,iNewSel;
	static PointF ptStart[2];
	static int iDirX,iDirY;
	static int iShift,iControl;
	static int bDragging=false;
	static PointF ptfCenter;
	Screen *sc=(Screen*)GetWindowLong(hwnd,GWL_USERDATA);
	switch(message) {
	case WM_CREATE:
		lp=(CREATESTRUCT*)lparam;
		SetWindowLong(hwnd,GWL_USERDATA,(LONG)lp->lpCreateParams);
		SetScrollRange(hwnd,SB_HORZ,0,1024,false);
		SetScrollRange(hwnd,SB_VERT,0,1024,false);
		SetScrollPos(hwnd,SB_HORZ,512,true);
		SetScrollPos(hwnd,SB_VERT,512,true);
		theScreen=sc=(Screen*)(lp->lpCreateParams);
		sc->CanvasSize(sc->iWidth,sc->iHeight);
		break;
	case WM_PAINT:
		hdc = BeginPaint(hwnd, &ps);
		GetClientRect(hwnd,&rcClient);
		sc->Paint(hdc,&rcClient);
		EndPaint(hwnd, &ps);
		break;
	case WM_VSCROLL:
		if(LOWORD(wparam)==SB_THUMBTRACK) {
			SetScrollPos(hwnd,SB_VERT,HIWORD(wparam),true);
			sc->iOffsetY=((sc->iHeight*(HIWORD(wparam)-512))>>9);
			InvalidateRect(hwnd,NULL,false);
		}
		break;
	case WM_HSCROLL:
		if(LOWORD(wparam)==SB_THUMBTRACK) {
			SetScrollPos(hwnd,SB_HORZ,HIWORD(wparam),true);
			sc->iOffsetX=((sc->iWidth*(HIWORD(wparam)-512))>>9);
			InvalidateRect(hwnd,NULL,false);
		}
		break;
	case WM_RBUTTONDOWN:
		pt.x=LOWORD(lparam);
		pt.y=HIWORD(lparam);
		ClientToScreen(hwnd,&pt);
		pr=theTree->prlCurrent.pr;
		if(pr && (pr->prType==prtShape ||pr->prType==prtText) && theTool->toolCurrent==tEdit) {
			hmenu=CreatePopupMenu();
			AppendMenu(hmenu,MF_STRING,0x11,(wchar_t*)theLang->GetID(MSG_ENDSHAPEEDIT));
			AppendMenu(hmenu,MF_SEPARATOR,0,0);
			AppendMenu(hmenu,MF_STRING,0x10,(wchar_t*)theLang->GetID(MSG_DELPOINT));
			switch(TrackPopupMenu(hmenu,TPM_LEFTALIGN|TPM_TOPALIGN|TPM_RETURNCMD,pt.x,pt.y,0,hwnd,NULL)) {
			case 0x10:
				theJournal->Record();
				pr->bz->DelPoint(pr);
				pr->CalcPath(2,1);
				theApp->Edit();
				sc->Send();
				break;
			case 0x11:
				theTool->Select(tArrow);
//				pr->CalcPath(2,0);
				break;
			}
			DestroyMenu(hmenu);
		}
		else if(theTool->toolCurrent==tShape||theTool->toolCurrent==tCurve) {
			hmenu=CreatePopupMenu();
			AppendMenu(hmenu,MF_STRING,0x10,(wchar_t*)theLang->GetID(MSG_CLOSESHAPE));
			AppendMenu(hmenu,MF_STRING,0x11,(wchar_t*)theLang->GetID(MSG_DELPOINT));
			switch(TrackPopupMenu(hmenu,TPM_LEFTALIGN|TPM_TOPALIGN|TPM_RETURNCMD,pt.x,pt.y,0,hwnd,NULL)) {
			case 0x10:
				theTree->Select1(bzTemp->Close(theTool->toolCurrent==tCurve?false:true));
				theTool->Select(tArrow);
				InvalidateRect(hwnd,NULL,false);
				break;
			case 0x11:
				bzTemp->DelPoint();
				InvalidateRect(hwnd,NULL,false);
				break;
			}
			DestroyMenu(hmenu);
		}
		else {
			RClickMenu(hwnd,pt,pr);
		}
		InvalidateRect(theScreen->hwnd,NULL,false);
		break;
	case WM_LBUTTONDOWN:
		SetFocus(hwnd);
		SetCapture(hwnd);
		bDragging=true;
		xScrCur=xScrOrg=x=LOWORD(lparam);
		yScrCur=yScrOrg=y=HIWORD(lparam);
		gxOrg=(int)((x-sc->iDispOffX)/sc->fZoom);
		gyOrg=(int)((y-sc->iDispOffY)/sc->fZoom);
		xOld1=xOld2=xOrg=sc->GridX(gxOrg);
		yOld1=yOld2=yOrg=sc->GridY(gyOrg);
		dx=dy=dxOld=dyOld=0;
		iStartMove=0;
		iShift=wparam&MK_SHIFT;
		iControl=wparam&MK_CONTROL;
		swprintf(str,L"%d,%d=>%d,%d (%+d,%+d)",xOrg,yOrg,x,y,dx,dy);
		theApp->statusbar->Display(str);
		switch(theTool->toolCurrent) {
		case tEdit:
			if(theTree->prlCurrent.pr==0) {
				theTool->Select(tArrow);
				break;
			}
			ptfCenter=PointF((theTree->prlCurrent.pr->ptfOrg[0].X+theTree->prlCurrent.pr->ptfOrg[1].X)*.5f,(theTree->prlCurrent.pr->ptfOrg[0].Y+theTree->prlCurrent.pr->ptfOrg[1].Y)*.5f);
			iDirX=theTree->prlCurrent.pr->ptfOrg[0].X>theTree->prlCurrent.pr->ptfOrg[1].X?-1:1;
			iDirY=theTree->prlCurrent.pr->ptfOrg[0].Y>theTree->prlCurrent.pr->ptfOrg[1].Y?-1:1;
			iDrag=theTree->prlCurrent.pr->bz->HitTest(x,y,iControl,theTree->prlCurrent.pr->ptfOrg[0],theTree->prlCurrent.pr->ptfOrg[1],theTree->prlCurrent.pr->mxRot);
			if(iDrag<0 && iShift) {
				theTree->prlCurrent.pr->bz->Down(PointF((REAL)sc->GridX(xOrg),(REAL)sc->GridY(yOrg)));
				iDrag=theTree->prlCurrent.pr->bz->iFocus;
			}
			else {
				if(iDrag>=0) {
					if(iDrag%3==1)
						theTree->prlCurrent.pr->bz->iFocus=iDrag;
				}
				else {
					pr=theTree->FindFromPos(xOrg,yOrg,0);
					if(pr==0) {
						theTree->prlCurrent.Clear();
						theTool->Select(tArrow);
						iDrag=0;
					}
					else if(pr!=theTree->prlCurrent.pr) {
						theTree->prlCurrent.Clear();
						theTree->prlCurrent.Add(pr);
						if(pr->prType!=prtShape || pr->prType==prtText) {
							theTool->Select(tArrow);
							iDrag=0;
						}
					}
				}
			}
			InvalidateRect(theScreen->hwnd,NULL,false);
			theApp->statusbar->Display2(L"ALT=IgnoreGrid");
			break;
		case tArrow:
			iNewSel=0;
			iDrag=sc->HitTest(&pr,tArrow,xScrOrg,yScrOrg,1,wparam);
			if(theProp->prCurrent) {
				theTree->prlCurrent.RecalcRect();
				if(iDrag&16) {
					ptStart[0]=PointF(theTree->prlCurrent.rc.GetLeft(),theTree->prlCurrent.rc.GetTop());
					ptStart[1]=PointF(theTree->prlCurrent.rc.GetRight(),theTree->prlCurrent.rc.GetBottom());
				}
				else {
					ptStart[0]=theProp->prCurrent->ptfOrg[0];//PointF(theProp->prCurrent->ptfOrg[0].X,theProp->prCurrent->ptfOrg[0].Y);
					ptStart[1]=theProp->prCurrent->ptfOrg[1];//PointF(theProp->prCurrent->ptfOrg[1].X,theProp->prCurrent->ptfOrg[1].Y);
				}
			}
			switch(iDrag) {
			case 4:
			case 5:
			case 6:
			case 7:
				InvalidateRect(hwnd,NULL,false);
				sc->Send();
				if(theZoomer)
					delete theZoomer;
				theZoomer=new Zoomer(theProp->prCurrent,ptStart[0],ptStart[1],theProp->prCurrent->rAngle,theProp->prCurrent->fSlant,iDrag,&sizex,&sizey);
				swprintf(str,L"%d,%d=>%d,%d [%d,%d]",xOrg,yOrg,x,y,sizex,sizey);
				theApp->statusbar->Display(str);
				theApp->statusbar->Display2(L"ALT=IgnoreGrid +Shift=KeepAspect");
				break;
			case 20:
			case 21:
			case 22:
			case 23:
				InvalidateRect(hwnd,NULL,false);
				sc->Send();
				if(theZoomer)
					delete theZoomer;
				theZoomer=new Zoomer(&theTree->prlCurrent,ptStart[0],ptStart[1],iDrag);
				theApp->statusbar->Display2(L"ALT=IgnoreGrid +Shift=KeepAspect");
				break;
			case 8:
			case 16:
			case 24:
				if(pr) {
					if(iControl!=0) {
						if(theTree->IsSelected(pr,true))
							theTree->SelectDel(pr);
						else
							theTree->SelectAdd(pr);
					}
					else if(iShift!=0) {
						theTree->SelectAdd(pr);
					}
					else {
						if(iDrag==8) {
							if(theTree->prlCurrent.pr==0)
								iNewSel=1;
							else {
								if(theProp->prCurrent!=pr)
									iNewSel=1;
							}
							theTree->Select1(pr);
						}
					}
				}
				InvalidateRect(hwnd,NULL,false);
				sc->Send();
				theApp->statusbar->Display2(L"ALT=IgnoreGrid");
				break;
			case 0:
			default:
				if(iShift==0 && iControl==0) {
					theTree->Select1(NULL);
					sc->DrawInvertRect(xOrg,yOrg,xOrg,yOrg);
				}
				break;
			}
			break;
		case tHand:
			break;
		case tEllipse:
		case tRect:
		case tPolygon:
		case tTrimming:
		case tLines:
			sc->DrawInvertRect(xOrg,yOrg,xOrg,yOrg);
			iDrag=0;
			break;
		case tCurve:
		case tShape:
			if(bzTemp->iIndex>3 && bzTemp->HitTest(x,y,1)==1) {
				theTree->Select1(bzTemp->Close(theTool->toolCurrent==tCurve?false:true));
				theTool->Select(tArrow);
				iNewSel=1;
				iDrag=8;
			}
			else
				bzTemp->Down(PointF((REAL)sc->GridX(xOrg),(REAL)sc->GridY(yOrg)));
			InvalidateRect(hwnd,NULL,false);
			break;
		case tText:
			break;
		case tGradation:
			iDrag=sc->HitTest(&pr,tGradation,xScrOrg,yScrOrg,1,wparam);
			break;
		}
		break;
	case WM_MOUSEMOVE:
		GetXY(lparam,&sx,&sy);
		iShift=wparam&MK_SHIFT;
		iControl=wparam&MK_CONTROL;
		dxOld=dx,dyOld=dy;
		gx=(int)((sx-sc->iDispOffX)/sc->fZoom);
		gy=(int)((sy-sc->iDispOffY)/sc->fZoom);
		gdx=gx-gxOrg;
		gdy=gy-gyOrg;
		x=sc->GridX(gx);
		y=sc->GridY(gy);
		dx=x-xOrg;
		dy=y-yOrg;
		if(bDragging)
			swprintf(str,L"%d,%d=>%d,%d (%+d,%+d)",xOrg,yOrg,x,y,dx,dy);
		else
			wsprintf(str,L"%d,%d",x,y);
		theApp->statusbar->Display(str);
		pr=0;
		if((prl=&theTree->prlCurrent)!=0)
			pr=prl->pr;
		if(bDragging) {
			if(iShift) {
				xx=max(x-xOrg,y-yOrg);
				px=xOrg+xx,py=yOrg+xx;
			}
			else
				px=x,py=y;
			if(iControl)
				xCur1=xOrg-(px-xOrg),yCur1=yOrg-(py-yOrg),xCur2=px,yCur2=py;
			else
				xCur1=xOrg,yCur1=yOrg,xCur2=px,yCur2=py;
			switch(theTool->toolCurrent) {
			case tHand:
				sc->iOffsetX-=gdx;
				sc->iOffsetY-=gdy;
				InvalidateRect(hwnd,NULL,false);
				if(sc->iWidth)
					SetScrollPos(hwnd,SB_HORZ,(sc->iOffsetX<<9)/sc->iWidth+512,true);
				if(sc->iHeight)
					SetScrollPos(hwnd,SB_VERT,(sc->iOffsetY<<9)/sc->iHeight+512,true);
				break;
			case tEdit:
				if(iDrag>=0) {
					theJournal->RecordOnce();
					if(pr && pr->bz) {
						PointF *pt=pr->bz->ptAnchor;
						PointF ptD;
						PointF ptfCur(x,y);
						Matrix *mx=pr->mxRot->Clone();
						mx->Invert();
						mx->TransformPoints(&ptfCur);
						delete mx;
						if(pr->bz->mxScale) {
							Matrix *mx2=pr->bz->mxScale->Clone();
							mx2->Invert();
							mx2->TransformPoints(&ptfCur);
							delete mx2;
						}
						switch(iDrag%3) {
						case 1:
							ptD=ptfCur-pt[iDrag];
							pt[iDrag]=ptfCur;
							if(iDrag>=1)
								pt[iDrag-1]=pt[iDrag-1]+ptD;
							if(iDrag<pr->bz->iIndex)
								pt[iDrag+1]=pt[iDrag+1]+ptD;
							break;
						case 0:
							pt[iDrag]=ptfCur;
							if(iShift==0) {
								pt[iDrag+2]=pt[iDrag+1]+pt[iDrag+1]-pt[iDrag];
							}
							break;
						case 2:
							pt[iDrag]=ptfCur;
							if(iShift==0) {
								pt[iDrag-2]=pt[iDrag-1]+pt[iDrag-1]-pt[iDrag];
							}
							break;
						}
						if(pr->prType!=prtText) {
							pr->bz->GetPath(0,pr->iClose,pr->path,&pr->rcOutline);
						}
						pr->CalcPath(2,0);
						theApp->Edit();
						sc->Send();
						InvalidateRect(hwnd,NULL,false);
					}
				}
				break;
			case tArrow:
				if(abs(dx)+abs(dy)>MOVESENSITIVITY)
					iStartMove=1;
				switch(iDrag) {
				case 8:
				case 16:
				case 24:
					theScreen->fRenderBreak=true;
					EnterCriticalSection(&csData);
					if(iStartMove) {
						theApp->Edit();
						theJournal->RecordOnce();
						for(prl=&theTree->prlCurrent;prl->next;prl=prl->next) {
							theTree->MovePos(prl->pr,dx-dxOld,dy-dyOld,0);
						}
						prl->RecalcRect();
					}
					theProp->SetupPos(prl->pr);
					sc->Send();
					InvalidateRect(hwnd,NULL,false);
					UpdateWindow(hwnd);
					LeaveCriticalSection(&csData);
					break;
				case 4:
				case 5:
				case 6:
				case 7:
				case 20:
				case 21:
				case 22:
				case 23:
					theScreen->fRenderBreak=true;
					theJournal->RecordOnce();
					theApp->Edit();
					theScreen->fRenderBreak=true;
					EnterCriticalSection(&csData);
					fxx=sc->GridX(x),fyy=sc->GridY(y);
					if(iShift) {
						switch(iDrag) {
						case 5:
						case 6:
						case 21:
						case 22:
							fxx=fxx-ptStart[0].X;
							fyy=fyy-ptStart[1].Y;
							fyy2=fxx*(ptStart[0].Y-ptStart[1].Y)/(ptStart[1].X-ptStart[0].X);
							if(fyy2>fyy)
								fyy=fyy2;
							else {
								fxx2=fyy*(ptStart[1].X-ptStart[0].X)/(ptStart[0].Y-ptStart[1].Y);
								fxx=fxx2;
							}
							fxx+=ptStart[0].X;
							fyy+=ptStart[1].Y;
							break;
						case 4:
						case 7:
						case 20:
						case 23:
							fxx=fxx-ptStart[0].X;
							fyy=fyy-ptStart[0].Y;
							fyy2=fxx*(ptStart[1].Y-ptStart[0].Y)/(ptStart[1].X-ptStart[0].X);
							if(fyy2>fyy)
								fyy=fyy2;
							else {
								fxx2=fyy*(ptStart[1].X-ptStart[0].X)/(ptStart[1].Y-ptStart[0].Y);
								fxx=fxx2;
							}
							fxx+=ptStart[0].X;
							fyy+=ptStart[0].Y;
							break;
						}
					}
					if(iDrag<20) {
						theZoomer->MoveOne(PointF(fxx,fyy),&sizex,&sizey);
						swprintf(str,L"%d,%d=>%d,%d [%d,%d]",xOrg,yOrg,x,y,sizex,sizey);
						swprintf(str,L"%d,%d=>%d,%d",xOrg,yOrg,x,y);
						theApp->statusbar->Display(str);
					}
					else
						theZoomer->Move(PointF(fxx,fyy));
					theProp->SetupPos();
					theProp->SetupFont();
					sc->Send();
					InvalidateRect(hwnd,0,0);
					UpdateWindow(hwnd);
					LeaveCriticalSection(&csData);
					break;
				case 0:
					theScreen->fRenderBreak=true;
					EnterCriticalSection(&csData);
					sc->DrawInvertRect(xOld1,yOld1,xOld2,yOld2);
					sc->DrawInvertRect(xCur1,yCur1,xCur2,yCur2);
					LeaveCriticalSection(&csData);
					break;
				}
				break;
			case tRect:
			case tEllipse:
			case tPolygon:
			case tTrimming:
			case tLines:
				sc->DrawInvertRect(xOld1,yOld1,xOld2,yOld2);
				sc->DrawInvertRect(xCur1,yCur1,xCur2,yCur2);
				break;
			case tCurve:
			case tShape:
				bzTemp->Drag(PointF(sc->GridX(x),sc->GridY(y)));
				InvalidateRect(hwnd,NULL,false);
				break;
			case tGradation:
				if(theProp->prCurrent) {
					theJournal->RecordOnce();
					theApp->Edit();
					theScreen->fRenderBreak=true;
					EnterCriticalSection(&csData);
					sc->SetGradPoint(iDrag,theProp->prCurrent,xOrg,yOrg,xCur,yCur,wparam);
					LeaveCriticalSection(&csData);
					theProp->prCurrent->CalcPath(2,1);
					sc->Send();
				}
			}
		}
		else {
			xCur1=yCur1=xCur2=yCur2=0;
			switch(theTool->toolCurrent) {
			case tArrow:
				switch(sc->HitTest(&pr,tArrow,sx,sy,0,wparam)) {
				case 4:
				case 7:
				case 20:
				case 23:
					SetCursor(hcurSizeNWSE);
					break;
				case 5:
				case 6:
				case 21:
				case 22:
					SetCursor(hcurSizeNESW);
					break;
				case 8:
				case 16:
				case 24:
					SetCursor(hcurMove);
					break;
				default:
						SetCursor(hcurArrow);
					break;
				}
				break;
			case tEdit:
				if(theTree->prlCurrent.pr && (theTree->prlCurrent.pr->prType==prtShape||theTree->prlCurrent.pr->prType==prtText)) {
					if((iHit=theTree->prlCurrent.pr->bz->HitTest(sx,sy,0,theTree->prlCurrent.pr->ptfOrg[0],theTree->prlCurrent.pr->ptfOrg[1],theTree->prlCurrent.pr->mxRot))>=0) {
						SetCursor(hcurShapeEdit);
						if(iHit%3!=1)
							theApp->statusbar->Display2(L"SHIFT=SeparateHandle");
					}
					else if(iShift) {
						SetCursor(hcurShapePlus);
						theApp->statusbar->Display2(L"SHIFT=AddPoint");
					}
					else {
						SetCursor(hcurArrow);
						theApp->statusbar->Display2(L"SHIFT=AddPoint");
					}
				}
				else
					theApp->statusbar->Display2(L"SPACE=HandTool");
				break;
			case tHand:
				SetCursor(hcurHand);
				theApp->statusbar->Display2(L"SPACE=HandTool");
				break;
			case tRect:
				SetCursor(hcurRect);
				theApp->statusbar->Display2(L"SPACE=HandTool ALT=IgnoreGrid SHIFT=Square CTRL=FromCenter");
				break;
			case tEllipse:
				SetCursor(hcurEllipse);
				theApp->statusbar->Display2(L"SPACE=HandTool ALT=IgnoreGrid SHIFT=Square CTRL=FromCenter");
				break;
			case tPolygon:
				SetCursor(hcurPolygon);
				theApp->statusbar->Display2(L"SPACE=HandTool ALT=IgnoreGrid SHIFT=Square CTRL=FromCenter");
				break;
			case tLines:
				SetCursor(hcurLines);
				theApp->statusbar->Display2(L"SPACE=HandTool ALT=IgnoreGrid SHIFT=Square CTRL=FromCenter");
				break;
			case tText:
				SetCursor(hcurText);
				theApp->statusbar->Display2(L"SPACE=HandTool");
				break;
			case tCurve:
				if(bzTemp->iIndex>3 && bzTemp->HitTest(sx,sy)==1)
					SetCursor(hcurCurveClose);
				else
					SetCursor(hcurCurvePlus);
				theApp->statusbar->Display2(L"SPACE=HandTool");
				break;
			case tShape:
				if(bzTemp->iIndex>3 && bzTemp->HitTest(sx,sy)==1)
					SetCursor(hcurShapeClose);
				else
					SetCursor(hcurShapePlus);
				theApp->statusbar->Display2(L"SPACE=HandTool");
				break;
			case tGradation:
				switch(sc->HitTest(&pr,tGradation,sx,sy,0,wparam)) {
				case 128:
				case 128+32:
					SetCursor(hcurMove);
					break;
				default:
					SetCursor(hcurGradation);
					break;
				}
				theApp->statusbar->Display2(L"SPACE=HandTool");
				break;
			case tTrimming:
				SetCursor(hcurTrimming);
				theApp->statusbar->Display2(L"SPACE=HandTool");
				break;
			}
		}
		xScrCur=sx;
		yScrCur=sy;
		xCur=x;
		yCur=y;
		xOld1=xCur1,xOld2=xCur2,yOld1=yCur1,yOld2=yCur2;
		break;
	case WM_LBUTTONUP:
		if(!bDragging)
			break;
		theApp->statusbar->Display2(L"SPACE=HandTool");
		GetXY(lparam,&sx,&sy);
		xCur=x=(int)(sc->GridX((sx-sc->iDispOffX)/sc->fZoom));
		yCur=y=(int)(sc->GridY((sy-sc->iDispOffY)/sc->fZoom));
		iControl=wparam&MK_CONTROL;
		iShift=wparam&MK_SHIFT;
		if(iShift) {
			xx=max(x-xOrg,y-yOrg);
			px=xOrg+xx,py=yOrg+xx;
		}
		else
			px=x,py=y;
		if(iControl)
			xCur1=xOrg-(px-xOrg),yCur1=yOrg-(py-yOrg),xCur2=px,yCur2=py;
		else
			xCur1=xOrg,yCur1=yOrg,xCur2=px,yCur2=py;
		if(xCur1>xCur2)
			t=xCur1,xCur1=xCur2,xCur2=t;
		if(yCur1>yCur2)
			t=yCur1,yCur1=yCur2,yCur2=t;
		theJournal->RecordReset();
		switch(theTool->toolCurrent) {
		case tEdit:
			pr=0;
			if((prl=&theTree->prlCurrent)!=0)
				pr=prl->pr;
			if(pr->prType==prtShape) {
				pr->bz->NormalizeRot(pr);
			}
			break;
		case tArrow:
			if(iDrag==0) {
//				Primitive *prOld=0;
//				if(iNewSel==0&&theProp->prCurrent)
//					prOld=theProp->prCurrent;
				if(iShift==0&&iControl==0)
					theTree->Select1(NULL);
				Primitive *pr,*pr1;
				RectF rcf(min(xCur1,xCur2),min(yCur1,yCur2),abs(xCur2-xCur1),abs(yCur2-yCur1));
				rcf.Inflate(.1f,.1f);	// This is needed for select objects that has some position error.
				pr1=NULL;
				for(pr=theTree->GetFirst();pr;pr=theTree->GetNext(pr)) {
					if(pr->IsLock()==0 && rcf.Contains(pr->rcOutline)) {
						theTree->SelectAdd(pr,0);
						if(pr1==NULL)
							pr1=pr;
					}
				}
				theTree->SelectAdd(pr1);
				theTree->prlCurrent.RecalcRect();
				InvalidateRect(theScreen->hwnd,NULL,false);
			}
			else {

				if(iStartMove==0 && iNewSel==0 && iShift==0 && iControl==0) {
					pr=theTree->FindFromPos(sx,sy,1);
					if(pr) {
						if(iShift||iControl) {
							if(theTree->IsSelected(pr,true))
								theTree->SelectDel(pr);
							else
								theTree->SelectAdd(pr);
						}
						else
							theTree->Select1(pr);
					}
				}
				theTree->prlCurrent.RecalcRect();
				InvalidateRect(theScreen->hwnd,NULL,false);

			}
			break;
		case tTrimming:
			if(iShift) {
				xx=max(x-xOrg,y-yOrg);
				xCur=xOrg+xx,yCur=yOrg+xx;
			}
			if(iControl) {
				xOrg=xOrg-(xCur-xOrg);
				yOrg=yOrg-(yCur-yOrg);
			}
			pr=theTree->prlCurrent.pr;
			theJournal->Record();
			theTree->TrimmingExec(xOrg,yOrg,xCur,yCur);
			sc->DrawInvertRect(xOrg,yOrg,xCur,yCur);
			break;
		case tRect:
			if(iShift) {
				xx=max(x-xOrg,y-yOrg);
				xCur=xOrg+xx,yCur=yOrg+xx;
			}
			if(iControl) {
				xOrg=xOrg-(xCur-xOrg);
				yOrg=yOrg-(yCur-yOrg);
			}
			pr=theTree->prlCurrent.pr;
			theJournal->Record();
			theTree->Select1(theTree->AddItem(theTree->GetParent(pr),pr,L"Rect",prtRect,xOrg,yOrg,xCur,yCur));
			theProp->SetType(theTree->prlCurrent.pr->prType);
			theProp->Setup(theTree->prlCurrent.pr);
			sc->DrawInvertRect(xOrg,yOrg,xCur,yCur);
			theTool->EndOneTime();
			theTree->prlCurrent.pr->CalcPath(2,1);
			theScreen->Send();
			break;
		case tEllipse:
			if(iShift) {
				xx=max(x-xOrg,y-yOrg);
				xCur=xOrg+xx,yCur=yOrg+xx;
			}
			if(iControl) {
				xOrg=xOrg-(xCur-xOrg);
				yOrg=yOrg-(yCur-yOrg);
			}
			theJournal->Record();
			pr=theTree->prlCurrent.pr;
			theTree->Select1(theTree->AddItem(theTree->GetParent(pr),pr,L"Ellipse",prtEllipse,xOrg,yOrg,xCur,yCur));
			sc->DrawInvertRect(xOrg,yOrg,xCur,yCur);
			theProp->SetType(theTree->prlCurrent.pr->prType);
			theProp->Setup(theTree->prlCurrent.pr);
			theTool->EndOneTime();
			theTree->prlCurrent.pr->CalcPath(2,1);
			theScreen->Send();
			break;
		case tLines:
			if(iShift) {
				xx=max(x-xOrg,y-yOrg);
				xCur=xOrg+xx,yCur=yOrg+xx;
			}
			if(iControl) {
				xOrg=xOrg-(xCur-xOrg);
				yOrg=yOrg-(yCur-yOrg);
			}
			theJournal->Record();
			pr=theTree->prlCurrent.pr;
			theTree->Select1(theTree->AddItem(theTree->GetParent(pr),pr,L"Lines",prtLines,xOrg,yOrg,xCur,yCur));
			sc->DrawInvertRect(xOrg,yOrg,xCur,yCur);
			theProp->SetType(theTree->prlCurrent.pr->prType);
			theProp->Setup(theTree->prlCurrent.pr);
			theTool->EndOneTime();
			theTree->prlCurrent.pr->CalcPath(2,1);
			theScreen->Send();
			break;
		case tPolygon:
			if(iShift) {
				xx=max(x-xOrg,y-yOrg);
				xCur=xOrg+xx,yCur=yOrg+xx;
			}
			if(iControl) {
				xOrg=xOrg-(xCur-xOrg);
				yOrg=yOrg-(yCur-yOrg);
			}
			theJournal->Record();
			pr=theTree->prlCurrent.pr;
			theTree->Select1(theTree->AddItem(theTree->GetParent(pr),pr,L"Polygon",prtPolygon,xOrg,yOrg,xCur,yCur));
			sc->DrawInvertRect(xOrg,yOrg,xCur,yCur);
			theProp->SetType(theTree->prlCurrent.pr->prType);
			theProp->Setup(theTree->prlCurrent.pr);
			theTool->EndOneTime();
			theTree->prlCurrent.pr->CalcPath(2,1);
			theScreen->Send();
			break;
		case tCurve:
		case tShape:
			theJournal->Record();
			bzTemp->Up(PointF((REAL)sc->GridX(xCur),(REAL)sc->GridY(yCur)));
			InvalidateRect(theScreen->hwnd,NULL,false);
			break;
		case tText:
			theJournal->Record();
			pr=theTree->prlCurrent.pr;
			theTree->Select1(theTree->AddItem(theTree->GetParent(pr),pr,L"Text",prtText,xOrg,yOrg,xOrg,yOrg));
			theProp->SetType(theTree->prlCurrent.pr->prType);
			theProp->Setup(theTree->prlCurrent.pr);
			theTree->SetName(theTree->prlCurrent.pr);
			theTool->EndOneTime();
			theTree->prlCurrent.pr->CalcPath(2,0);
			theTree->Select1(theTree->prlCurrent.pr);
			theScreen->Send();
			break;
		case tGradation:
			if(theProp->prCurrent) {
				sc->SetGradPoint(iDrag,theProp->prCurrent,xOrg,yOrg,xCur,yCur,wparam);
				theProp->prCurrent->CalcPath(2,1);
				sc->Send();
				InvalidateRect(hwnd,NULL,false);
			}
			break;
		}
		iDrag=0;
		bDragging=false;
		ReleaseCapture();
		break;
	case WM_MOUSEWHEEL:
		POINT pt;
		pt.x=LOWORD(lparam);
		pt.y=HIWORD(lparam);
		xx=HIWORD(wparam);
		if(xx&0x8000)
			xx|=0xffff0000;
		HWND hwndPoint;
		hwndPoint=WindowFromPoint(pt);
		if(hwndPoint==theScreen->hwnd) {
			if(wparam&MK_CONTROL) {
				if(xx>0)
					theScreen->fZoom*=1.2f;
				else
					theScreen->fZoom/=1.2f;
				if(theScreen->fZoom<0.125f)
					theScreen->fZoom=0.125f;
				if(theScreen->fZoom>16.f)
					theScreen->fZoom=16.f;
			}
			else {
				xx=xx/theScreen->fZoom/5;
				if(wparam&MK_SHIFT)
					theScreen->iOffsetX-=xx;
				else
					theScreen->iOffsetY-=xx;
			}
		}
		if(hwndPoint==theProp->hwndFrame||hwndPoint==theProp->hwndSub) {
			theProp->Scroll(-xx/24,0);
		}
		InvalidateRect(theScreen->hwnd,NULL,false);
		break;
	case WM_CHAR:
		if(bDragging==false)
			Shortcut((int)wparam,1);
		break;
	case WM_KEYDOWN:
		if(bDragging==false)
			Shortcut((int)wparam,0);
		ddx=ddy=0;
		switch(wparam) {
		case VK_SHIFT:
			switch(theTool->toolCurrent) {
			case tEdit:
				if(GetCursor()==hcurArrow)
						SetCursor(hcurShapePlus);
				break;
			}
			iShift=1;
			break;
		case VK_UP:
			if(iShift)
				ddx=0,ddy=-theScreen->iGridY;
			else
				ddx=0,ddy=-1;
			break;
		case VK_DOWN:
			if(iShift)
				ddx=0,ddy=theScreen->iGridY;
			else
				ddx=0,ddy=1;
			break;
		case VK_LEFT:
			if(iShift)
				ddx=-theScreen->iGridX,ddy=0;
			else
				ddx=-1,ddy=0;
			break;
		case VK_RIGHT:
			if(iShift)
				ddx=theScreen->iGridX,ddy=0;
			else
				ddx=1,ddy=0;
			break;
		}
		if(ddx||ddy) {
			if(GetAsyncKeyState(VK_CONTROL)&0x8000) {
				theScreen->iOffsetX+=ddx/theScreen->fZoom*20;
				theScreen->iOffsetY+=ddy/theScreen->fZoom*20;
				InvalidateRect(theScreen->hwnd,NULL,false);
			}
			else {
				theJournal->RecordOnce();
				theTree->MoveSelected(ddx,ddy);
				theApp->Edit();
				sc->Send();
				InvalidateRect(hwnd,NULL,false);
			}
		}
		break;
	case WM_KEYUP:
		if(wparam==VK_CONTROL) {
			switch(theTool->toolCurrent) {
			case tArrow:
				SetCursor(hcurArrow);
				break;
			}
		}
		if(wparam==VK_SHIFT) {
			switch(theTool->toolCurrent) {
			case tArrow:
				SetCursor(hcurArrow);
				break;
			case tEdit:
				if(GetCursor()==hcurShapePlus)
					SetCursor(hcurArrow);
				break;
			}
			iShift=0;
		}
		break;

	default:
		return DefWindowProc(hwnd, message, wparam, lparam);
	}
	return 0;
}
LRESULT CALLBACK Screen::dlgprocCanvasSize(HWND hdlg, UINT message, WPARAM wparam, LPARAM lparam) {
	int x,y;
	switch (message) {
	case WM_INITDIALOG:
		SetDlgItemInt(hdlg,IDC_WIDTH,theScreen->iWidth,false);
		SetDlgItemInt(hdlg,IDC_HEIGHT,theScreen->iHeight,false);
		SetDlgItemInt(hdlg,IDC_FRAMESX,theScreen->iFramesX,false);
		SetDlgItemInt(hdlg,IDC_FRAMESY,theScreen->iFramesY,false);
		return TRUE;
	case WM_COMMAND:
		if (LOWORD(wparam) == IDOK || LOWORD(wparam) == IDCANCEL) {
			if(LOWORD(wparam)==IDOK) {
				theJournal->Record();
				x=GetDlgItemInt(hdlg,IDC_WIDTH,NULL,false);
				y=GetDlgItemInt(hdlg,IDC_HEIGHT,NULL,false);
				theScreen->iFramesX=max(1,GetDlgItemInt(hdlg,IDC_FRAMESX,NULL,false));
				theScreen->iFramesY=max(1,GetDlgItemInt(hdlg,IDC_FRAMESY,NULL,false));
				theScreen->CanvasSize(x,y);
				InvalidateRect(theApp->hwnd,NULL,false);
			}
			EndDialog(hdlg, LOWORD(wparam));
			return TRUE;
		}
		break;
	}
	return FALSE;
}
LRESULT CALLBACK Screen::dlgprocGridSetup(HWND hdlg, UINT message, WPARAM wparam, LPARAM lparam) {
	switch (message) {
	case WM_INITDIALOG:
		SetDlgItemInt(hdlg,IDC_WIDTH,theScreen->iGridX,false);
		SetDlgItemInt(hdlg,IDC_HEIGHT,theScreen->iGridY,false);
		CheckButton(hdlg,IDC_VISIBLE,theScreen->iGridVisible);
		CheckButton(hdlg,IDC_ENABLE,theScreen->iGridEnable);
		return TRUE;
	case WM_COMMAND:
		if (LOWORD(wparam) == IDOK || LOWORD(wparam) == IDCANCEL) {
			if(LOWORD(wparam)==IDOK) {
				theScreen->iGridX=GetDlgItemInt(hdlg,IDC_WIDTH,NULL,false);
				theScreen->iGridY=GetDlgItemInt(hdlg,IDC_HEIGHT,NULL,false);
				theScreen->iGridEnable=IsDlgButtonChecked(hdlg,IDC_ENABLE);
				theScreen->iGridVisible=IsDlgButtonChecked(hdlg,IDC_VISIBLE);
				theCmdBar->SetGridVisible(theScreen->iGridVisible);
				InvalidateRect(theApp->hwnd,NULL,false);
			}
			EndDialog(hdlg, LOWORD(wparam));
			return TRUE;
		}
		break;
	}
	return FALSE;
}
CmdBar::CmdBar(HWND hwndInit) {
	hwndParent=hwndInit;
	hiconParent=LoadIcon(hinstMain,MAKEINTRESOURCE(IDI_PARENT));
	hiconUp=LoadIcon(hinstMain,MAKEINTRESOURCE(IDI_UP));
	hiconDown=LoadIcon(hinstMain,MAKEINTRESOURCE(IDI_DOWN));
	hiconZoomIn=LoadIcon(hinstMain,MAKEINTRESOURCE(IDI_ZOOMIN));
	hiconZoom1=LoadIcon(hinstMain,MAKEINTRESOURCE(IDI_ZOOM1));
	hiconZoomOut=LoadIcon(hinstMain,MAKEINTRESOURCE(IDI_ZOOMOUT));
	hiconGridVisible=LoadIcon(hinstMain,MAKEINTRESOURCE(IDI_GRIDVISIBLE));
	hiconGridEnable=LoadIcon(hinstMain,MAKEINTRESOURCE(IDI_GRIDENABLE));
	hiconDispTree=LoadIcon(hinstMain,MAKEINTRESOURCE(IDI_DISPTREE));
	hiconDispProp=LoadIcon(hinstMain,MAKEINTRESOURCE(IDI_DISPPROP));
	hiconDispColor=LoadIcon(hinstMain,MAKEINTRESOURCE(IDI_DISPCOLOR));
	hiconAlignCenter =LoadIcon(hinstMain, MAKEINTRESOURCE(IDI_ALIGNCENTER));
	hwnd=CreateDialogParam(hinstMain,MAKEINTRESOURCE(IDD_CMDBAR),hwndInit,(DLGPROC)dlgproc,(LPARAM)this);
}
CmdBar::~CmdBar(void) {
}
void CmdBar::SetGridEnable(int i) {
	CheckButton(hwnd,IDC_GRIDENABLE,i);
}
void CmdBar::SetGridVisible(int i) {
	CheckButton(hwnd,IDC_GRIDVISIBLE,i);
}
void CmdBar::Pos(int x,int y) {
	SetWindowPos(hwnd,0,x,y,0,0,SWP_NOZORDER|SWP_NOSIZE);
}
void CmdBar::Size(int x,int y) {
	SetWindowPos(hwnd,0,0,0,x,y,SWP_NOZORDER|SWP_NOMOVE);
}
LRESULT CALLBACK CmdBar::dlgproc(HWND hwnd,UINT message,WPARAM wparam,LPARAM lparam) {
	static CmdBar *p;
	switch(message) {
	case WM_INITDIALOG:
		p=(CmdBar*)lparam;
		SendDlgItemMessage(hwnd,IDC_PARENT,BM_SETIMAGE,IMAGE_ICON,(LPARAM)p->hiconParent);
		new ToolTip(GetDlgItem(hwnd,IDC_PARENT),(wchar_t*)theLang->GetID(MSG_MOVETOPARENT));
		SendDlgItemMessage(hwnd,IDC_UP,BM_SETIMAGE,IMAGE_ICON,(LPARAM)p->hiconUp);
		new ToolTip(GetDlgItem(hwnd,IDC_UP),(wchar_t*)theLang->GetID(MSG_MOVETOBG));
		SendDlgItemMessage(hwnd,IDC_DOWN,BM_SETIMAGE,IMAGE_ICON,(LPARAM)p->hiconDown);
		new ToolTip(GetDlgItem(hwnd,IDC_DOWN),(wchar_t*)theLang->GetID(MSG_MOVETOFG));
		SendDlgItemMessage(hwnd,IDC_UNDO,BM_SETIMAGE,IMAGE_BITMAP,(LPARAM)theApp->hbmpUndo);
		new ToolTip(GetDlgItem(hwnd,IDC_UNDO),(wchar_t*)theLang->GetID(MSG_UNDO));
		SendDlgItemMessage(hwnd,IDC_REDO,BM_SETIMAGE,IMAGE_BITMAP,(LPARAM)theApp->hbmpRedo);
		new ToolTip(GetDlgItem(hwnd,IDC_REDO),(wchar_t*)theLang->GetID(MSG_REDO));
		SendDlgItemMessage(hwnd,IDC_ZOOMOUT,BM_SETIMAGE,IMAGE_ICON,(LPARAM)p->hiconZoomOut);
		new ToolTip(GetDlgItem(hwnd,IDC_ZOOMOUT),(wchar_t*)theLang->GetID(MSG_ZOOMOUT));
		SendDlgItemMessage(hwnd,IDC_ZOOM1,BM_SETIMAGE,IMAGE_ICON,(LPARAM)p->hiconZoom1);
		new ToolTip(GetDlgItem(hwnd,IDC_ZOOM1),(wchar_t*)theLang->GetID(MSG_ZOOM1));
		SendDlgItemMessage(hwnd,IDC_ZOOMIN,BM_SETIMAGE,IMAGE_ICON,(LPARAM)p->hiconZoomIn);
		new ToolTip(GetDlgItem(hwnd,IDC_ZOOMIN),(wchar_t*)theLang->GetID(MSG_ZOOMIN));
		SendDlgItemMessage(hwnd,IDC_GRIDVISIBLE,BM_SETIMAGE,IMAGE_ICON,(LPARAM)p->hiconGridVisible);
		new ToolTip(GetDlgItem(hwnd,IDC_GRIDVISIBLE),(wchar_t*)theLang->GetID(MSG_GRIDVISIBLE));
		SendDlgItemMessage(hwnd,IDC_GRIDENABLE,BM_SETIMAGE,IMAGE_ICON,(LPARAM)p->hiconGridEnable);
		new ToolTip(GetDlgItem(hwnd,IDC_GRIDENABLE),(wchar_t*)theLang->GetID(MSG_GRIDENABLE));
		SendDlgItemMessage(hwnd,IDC_DISPTREE,BM_SETIMAGE,IMAGE_ICON,(LPARAM)p->hiconDispTree);
		new ToolTip(GetDlgItem(hwnd,IDC_DISPTREE),(wchar_t*)theLang->GetID(MSG_DISPLAYTREE));
		SendDlgItemMessage(hwnd,IDC_DISPPROP,BM_SETIMAGE,IMAGE_ICON,(LPARAM)p->hiconDispProp);
		new ToolTip(GetDlgItem(hwnd,IDC_DISPPROP),(wchar_t*)theLang->GetID(MSG_DISPLAYPROP));
		SendDlgItemMessage(hwnd,IDC_DISPCOLOR,BM_SETIMAGE,IMAGE_ICON,(LPARAM)p->hiconDispColor);
		new ToolTip(GetDlgItem(hwnd,IDC_DISPCOLOR),(wchar_t*)theLang->GetID(MSG_DISPLAYCOLOR));
		SendDlgItemMessage(hwnd, IDC_ALIGNCENTER, BM_SETIMAGE, IMAGE_ICON, (LPARAM)p->hiconAlignCenter);
		new ToolTip(GetDlgItem(hwnd, IDC_ALIGNCENTER), (wchar_t*)theLang->GetID(MSG_ALIGNCENTER));
		CheckButton(hwnd,IDC_GRIDVISIBLE,theScreen->iGridVisible);
		CheckButton(hwnd,IDC_GRIDENABLE,theScreen->iGridEnable);
		CheckButton(hwnd,IDC_DISPTREE,true);
		CheckButton(hwnd,IDC_DISPPROP,true);
		CheckButton(hwnd,IDC_DISPCOLOR,true);
		return 0;
	case WM_COMMAND:
		switch(wparam) {
		case IDC_PARENT:
			theJournal->Record();
			theTree->MoveParent();
			break;
		case IDC_UP:
			theJournal->Record();
			theTree->MoveUp();
			break;
		case IDC_DOWN:
			theJournal->Record();
			theTree->MoveDown();
			break;
		case IDC_UNDO:
			theJournal->Undo();
			break;
		case IDC_REDO:
			theJournal->Redo();
			break;
		case IDC_ZOOMIN:
			theScreen->ZoomIn();
			break;
		case IDC_ZOOM1:
			theScreen->Zoom1();
			break;
		case IDC_ZOOMOUT:
			theScreen->ZoomOut();
			break;
		case IDC_GRIDVISIBLE:
			theScreen->GridVisible(IsDlgButtonChecked(hwnd,IDC_GRIDVISIBLE));
			break;
		case IDC_GRIDENABLE:
			theScreen->GridEnable(IsDlgButtonChecked(hwnd,IDC_GRIDENABLE));
			break;
		case IDC_DISPTREE:
			theTree->Show(IsDlgButtonChecked(hwnd,IDC_DISPTREE));
			theApp->CheckMenu();
			break;
		case IDC_ALIGNCENTER:
			theTree->Align(1);
			theTree->Align(5);
			break;
		case IDC_DISPPROP:
			theProp->Show(IsDlgButtonChecked(hwnd,IDC_DISPPROP));
			theApp->CheckMenu();
			break;
		case IDC_DISPCOLOR:
			theApp->pal->Show(IsDlgButtonChecked(hwnd,IDC_DISPCOLOR));
			theApp->CheckMenu();
			break;
		}
		SetFocus(theScreen->hwnd);
		break;
	}
	return FALSE;
}
Tool::Tool(HWND hwndInit) {
	hwndParent=hwndInit;
	toolMaster=toolCurrent=tArrow;
	bOnetime=false;
	hwnd=CreateDialogParam(hinstMain,MAKEINTRESOURCE(IDD_TOOL),hwndInit,(DLGPROC)dlgproc,(LPARAM)this);
}
Tool::~Tool(void) {
}
void Tool::Pos(int x,int y) {
	SetWindowPos(hwnd,0,x,y,0,0,SWP_NOZORDER|SWP_NOSIZE);
}
void Tool::Size(int x,int y) {
	SetWindowPos(hwnd,0,0,0,x,y,SWP_NOZORDER|SWP_NOMOVE);
}
LRESULT CALLBACK Tool::dlgproc(HWND hwnd,UINT message,WPARAM wparam,LPARAM lparam) {
	static Tool *ptool;
	switch(message) {
	case WM_INITDIALOG:
		ptool=(Tool*)lparam;
		SendDlgItemMessage(hwnd,IDC_SELECT,BM_SETIMAGE,IMAGE_BITMAP,(LPARAM)theApp->hbmpArrow);
		new ToolTip(GetDlgItem(hwnd,IDC_SELECT),(wchar_t*)theLang->GetID(MSG_ARROWTOOL));
		SendDlgItemMessage(hwnd,IDC_HANDMOVE,BM_SETIMAGE,IMAGE_BITMAP,(LPARAM)theApp->hbmpHand);
		new ToolTip(GetDlgItem(hwnd,IDC_HANDMOVE),(wchar_t*)theLang->GetID(MSG_HANDTOOL));
		SendDlgItemMessage(hwnd,IDC_RECT,BM_SETIMAGE,(WPARAM)IMAGE_BITMAP,(LPARAM)theApp->hbmpRect);
		new ToolTip(GetDlgItem(hwnd,IDC_RECT),(wchar_t*)theLang->GetID(MSG_RECTTOOL));
		SendDlgItemMessage(hwnd,IDC_ELLIPSE,BM_SETIMAGE,IMAGE_BITMAP,(LPARAM)theApp->hbmpEllipse);
		new ToolTip(GetDlgItem(hwnd,IDC_ELLIPSE),(wchar_t*)theLang->GetID(MSG_ELLIPSETOOL));
		SendDlgItemMessage(hwnd,IDC_POLYGON,BM_SETIMAGE,IMAGE_BITMAP,(LPARAM)theApp->hbmpPolygon);
		new ToolTip(GetDlgItem(hwnd,IDC_POLYGON),(wchar_t*)theLang->GetID(MSG_POLYGONTOOL));
		SendDlgItemMessage(hwnd,IDC_TEXT,BM_SETIMAGE,IMAGE_BITMAP,(LPARAM)theApp->hbmpText);
		new ToolTip(GetDlgItem(hwnd,IDC_TEXT),(wchar_t*)theLang->GetID(MSG_TEXTTOOL));
		SendDlgItemMessage(hwnd,IDC_CURVE,BM_SETIMAGE,IMAGE_BITMAP,(LPARAM)theApp->hbmpCurve);
		new ToolTip(GetDlgItem(hwnd,IDC_CURVE),(wchar_t*)theLang->GetID(MSG_CURVETOOL));
		SendDlgItemMessage(hwnd,IDC_SHAPE,BM_SETIMAGE,IMAGE_BITMAP,(LPARAM)theApp->hbmpShape);
		new ToolTip(GetDlgItem(hwnd,IDC_SHAPE),(wchar_t*)theLang->GetID(MSG_SHAPETOOL));
		SendDlgItemMessage(hwnd,IDC_LINES,BM_SETIMAGE,IMAGE_BITMAP,(LPARAM)theApp->hbmpLines);
		new ToolTip(GetDlgItem(hwnd,IDC_LINES),(wchar_t*)theLang->GetID(MSG_LINESTOOL));
		SendDlgItemMessage(hwnd,IDC_GRADATION,BM_SETIMAGE,IMAGE_BITMAP,(LPARAM)theApp->hbmpGradation);
		new ToolTip(GetDlgItem(hwnd,IDC_GRADATION),(wchar_t*)theLang->GetID(MSG_GRADATIONTOOL));
		SendDlgItemMessage(hwnd,IDC_TRIMMING,BM_SETIMAGE,IMAGE_BITMAP,(LPARAM)theApp->hbmpTrimming);
		new ToolTip(GetDlgItem(hwnd,IDC_TRIMMING),(wchar_t*)theLang->GetID(MSG_TRIMMINGTOOL));
		SendDlgItemMessage(hwnd,IDC_IMAGE,BM_SETIMAGE,IMAGE_BITMAP,(LPARAM)theApp->hbmpImage);
		new ToolTip(GetDlgItem(hwnd,IDC_IMAGE),(wchar_t*)theLang->GetID(MSG_INSERTIMAGE));
		SendDlgItemMessage(hwnd,IDC_KNOB,BM_SETIMAGE,IMAGE_BITMAP,(LPARAM)theApp->hbmpNewKnob);
		new ToolTip(GetDlgItem(hwnd,IDC_KNOB),(wchar_t*)theLang->GetID(MSG_INSERTNEWKNOB));
		CheckButton(hwnd,IDC_SELECT,true);
		return 0;
	case WM_COMMAND:
		switch(wparam) {
		case IDC_SELECT:
			SendMessage(theApp->hwnd,WM_COMMAND,ID_TOOL_ARROW,0);
			break;
		case IDC_HANDMOVE:
			SendMessage(theApp->hwnd,WM_COMMAND,ID_TOOL_HAND,0);
			break;
		case IDC_RECT:
			SendMessage(theApp->hwnd,WM_COMMAND,ID_TOOL_RECT,0);
			break;
		case IDC_ELLIPSE:
			SendMessage(theApp->hwnd,WM_COMMAND,ID_TOOL_ELLIPSE,0);
			break;
		case IDC_POLYGON:
			SendMessage(theApp->hwnd,WM_COMMAND,ID_TOOL_POLYGON,0);
			break;
		case IDC_TEXT:
			SendMessage(theApp->hwnd,WM_COMMAND,ID_TOOL_TEXT,0);
			break;
		case IDC_LINES:
			SendMessage(theApp->hwnd,WM_COMMAND,ID_TOOL_LINES,0);
			break;
		case IDC_CURVE:
			SendMessage(theApp->hwnd,WM_COMMAND,ID_TOOL_CURVE,0);
			break;
		case IDC_SHAPE:
			SendMessage(theApp->hwnd,WM_COMMAND,ID_TOOL_SHAPE,0);
			break;
		case IDC_GRADATION:
			SendMessage(theApp->hwnd,WM_COMMAND,ID_TOOL_GRADATION,0);
			break;
		case IDC_TRIMMING:
			SendMessage(theApp->hwnd,WM_COMMAND,ID_TOOL_TRIMMING,0);
			break;
		case IDC_IMAGE:
			SendMessage(theApp->hwnd,WM_COMMAND,ID_TOOL_INSERTIMAGE,0);
			break;
		case IDC_KNOB:
			SendMessage(theApp->hwnd,WM_COMMAND,ID_TOOL_INSERTNEWKNOB,0);
			break;
		}
		SetFocus(theScreen->hwnd);
		break;
	}
	return FALSE;
}
void Tool::SetButtonState(void) {
	int i,iTarget;
	static int tabButton[]={IDC_SELECT,IDC_SELECT,IDC_HANDMOVE,IDC_RECT,IDC_ELLIPSE,IDC_POLYGON,IDC_TEXT,IDC_CURVE,IDC_SHAPE,IDC_LINES,IDC_GRADATION,IDC_TRIMMING,0};
	static int tabTool[]={tArrow,tEdit,tHand,tRect,tEllipse,tPolygon,tText,tCurve,tShape,tLines,tGradation,tTrimming,0};
	for(i=0;tabButton[i];++i)
		if(tabTool[i]==toolCurrent)
			iTarget=tabButton[i];
	for(i=0;tabButton[i];++i)
		CheckButton(hwnd,tabButton[i],tabButton[i]==iTarget);
}
void Tool::SetCursorType(void) {
	switch(toolCurrent) {
	case tArrow:
		SetCursor(hcurArrow);
		break;
	case tHand:
		SetCursor(hcurHand);
		break;
	case tRect:
		SetCursor(hcurRect);
		break;
	case tEllipse:
		SetCursor(hcurEllipse);
		break;
	case tPolygon:
		SetCursor(hcurPolygon);
		break;
	case tText:
		SetCursor(hcurText);
		break;
	case tCurve:
		SetCursor(hcurCurvePlus);
		break;
	case tShape:
		SetCursor(hcurShapePlus);
		break;
	case tLines:
		SetCursor(hcurLines);
		break;
	case tGradation:
		SetCursor(hcurGradation);
		break;
	case tTrimming:
		SetCursor(hcurTrimming);
		break;
	}
}
void Tool::Select(tool t,bool bOne) {
	toolMaster=toolCurrent=t;
	bOnetime=bOne;
	SetButtonState();
	SetCursorType();
	if(t==tShape||t==tCurve)
		bzTemp->Reset();
	InvalidateRect(theScreen->hwnd,NULL,false);
}
void Tool::SelectTemp(tool t) {
	if(toolCurrent==t)
		return;
	toolCurrent=t;
	SetButtonState();
	SetCursorType();
}
void Tool::Resume(void) {
	if(toolCurrent==toolMaster)
		return;
	toolCurrent=toolMaster;
	SetButtonState();
	SetCursorType();
}
void Tool::EndOneTime(void) {
	if(bOnetime) {
		Select(tArrow);
		bOnetime=false;
	}
}
Primitive::Primitive(bool bInit) {
	memset(this,0,sizeof Primitive);
	if(bInit)
		exs.Init();
	path=new GraphicsPath();
	pathOutline=new GraphicsPath();
	mxRot=new Matrix();
	mxShift=new Matrix();
	prType=prtNone;
}
Primitive::Primitive(prim pr,float x1,float y1,float x2,float y2) {
	memset(this,0,sizeof Primitive);
	exs.Init();
	iVisible=1;
	iLock=0;
	iAutoSize=1;
	iSmallFontOpt=0;
	prNext=prParent=0;
	prType=pr;
	iOperation=0;
	iGradationType=1;
	iLinear=0;
	iSmoother=0;
	rAngle=0.f;
	fSlant=0.f;
	fWidth=1.f;
	iStart=-180;
	iStop=180;
	iVertex=3;
	rTension=0.f;
	rTwist=0.f;
	iStarDepth=100;
	iLineType=0;
	iFill=1;
	iAntialias=1;
	iBorder=0;
	iBorderType=0;
	iBorderJoin=2;
	iLineCap=0;
	iClose=1;
	iBorderAsFill=0;
	iPie=1;
	iAlpha=100;
	fRoundX1=fRoundX2=fRoundX3=fRoundX4=0.f;
	fRoundY1=fRoundY2=fRoundY3=fRoundY4=0.f;
	iRoundSeparate=0;
	iRoundXYSeparate=0;
	iCornerC1=iCornerC2=iCornerC3=iCornerC4=0;
	path=new GraphicsPath();
	pathOutline=new GraphicsPath();
	mxRot=new Matrix();
	mxShift=new Matrix();
	ptfOrg[0].X=x1;
	ptfOrg[0].Y=y1;
	ptfOrg[1].X=x2;
	ptfOrg[1].Y=y2;
	iOperation=0;
	dwColor1=0xffff8080;
	dwColor2=0xff800000;
	dwColor3=0xff000000;

	ptGrad[0]=ptGrad[1]=PointF(0.f,0.f);
	ptSpec[0]=ptSpec[1]=PointF(0.5f,0.5f);

	iDiffuse=0;

	light[0].iSpecularDir=light[1].iSpecularDir=-45;
	light[0].iSpecularOffset=light[1].iSpecularOffset=-50;
	light[0].iSpecularType=light[1].iSpecularType=0;
	light[0].iSpecular=light[1].iSpecular=50;
	light[0].iHighlight=light[1].iHighlight=25;
	light[0].iHighlightWidth=light[1].iHighlightWidth=0;
	light[0].iAmbient=light[1].iAmbient=50;

	iEmbossDirEnable=0;
	iEmbossDir=-45;
	rEmbossWidth=2.f;
	fEmbossDepth=fEmbossDepth2=0.f;
	iTextureType=-1;
	iTexture=0;
	iUseTextureAlpha=0;
	iTextureZoomXYSepa=0;
	fTextureZoomX=100;
	fTextureZoomY=100;
	fTextureOffX=0.f;
	fTextureOffY=0.f;
	fTextureRot=0.f;
	dwShadowColor=0x00000000;
	iShadowDirEnable=0;
	iShadowDir=-45;
	iShadowOffset=8;
	iShadowDensity=0;
	rShadowDiffuse=0.f;
	dwIShadowColor=0x00000000;
	iIShadowDirEnable=0;
	iIShadowDir=-45;
	iIShadowOffset=8;
	iIShadowDensity=0;
	rIShadowDiffuse=0.f;
	iMargin=0;
	iBold=iItalic=iFix=0;
	fFontSize=12.f;
	fPathOffset=0.f;
	fFontAspect=100.f;
	fFontSpacing=100.f;
	iFontSizeUnit=0;
	iUseTextPath=0;
	iKeepDir=0;
	wcscpy(strName,L"");
	wcscpy(strText,L"Text");
	wcscpy(strFont,L"Arial");
	wcscpy(strFile,L"");
	wcscpy(strTextureName,L"");
	if(pr==prtShape) {
		bz=new Bezier();
		*bz=*bzTemp;
		bzTemp->Reset();
	}
	else if(pr==prtText) {
		bz=new Bezier();
		bz->AddPoint(ptfOrg[0]);
		bz->AddPoint(PointF(ptfOrg[0].X+200,ptfOrg[0].Y));
	}
	else
		bz=new Bezier();
	texEmbed=NULL;
	img=NULL;
	bmpBase=bmp=bmpPreview=NULL;
	iBmpOffX=iBmpOffY=0;
	iBmpBaseDirty=iBmpDirty=iBmpPrevDirty=2;
	bInitializing=false;
	hitem=NULL;
	if(prType==prtLines) {
		iBorder=1;
		iFill=0;
		iVertex=8;
		iStarDepth=0;
	}
}
Primitive::~Primitive(void) {
	if(bz)
		delete bz;
	if(texEmbed)
		delete texEmbed;
	if(mxRot)
		delete mxRot;
	delete mxShift;
	delete path;
	delete pathOutline;
	if(bmp)
		delete bmp;
	if(bmpPreview)
		delete bmpPreview;
	if(bmpBase)
		delete bmpBase;
	if(img)
		delete img;
}
void Primitive::GetPathBounds(GraphicsPath *path,RectF *rcf) {
	path->GetBounds(rcf);
	if(rcf->Width==0 && rcf->Height==0) {
		rcf->X=ptfOrg[0].X;
		rcf->Y=ptfOrg[0].Y;
	}
}
bool Primitive::IsVisible(float x,float y) {
	float x1,y1,x2,y2;
	PointF ptf(x,y);
	if(prType==prtGroup) {
		if(x>=ptfOrg[0].X && x<=ptfOrg[1].X && y>=ptfOrg[0].Y && y<=ptfOrg[1].Y)
			return true;
		return false;
	}
	if(mxRot) {
		Matrix *mx=mxRot->Clone();
		mx->Invert();
		mx->TransformPoints(&ptf);
		delete mx;
	}
	x1=ptfOrg[0].X;
	x2=ptfOrg[1].X;
	FSort(&x1,&x2);
	y1=ptfOrg[0].Y;
	y2=ptfOrg[1].Y;
	FSort(&y1,&y2);
	if(ptf.X>=x1-1 && ptf.X<=x2+1 && ptf.Y>=y1-1 && ptf.Y<=y2+1)
		return true;
	return false;
}
int Primitive::IsLock(void) {
	Primitive *pr=this;
	for(;;) {
		pr=theTree->GetParent(pr);
		if(pr==0)
			return iLock;
		if(pr->prType==prtGroup)
			return iLock|2;
	}
}
void Primitive::Lock(int i) {
	if(i)
		iLock=1;
	else
		iLock=0;
}
void Primitive::SetBmpSize(Bitmap *pbmp,RectF rcf) {
	int x=(int)(rcf.X+rcf.Width+0.99f)-(int)rcf.X;
	int y=(int)(rcf.Y+rcf.Height+0.99f)-(int)rcf.Y;
	if(pbmp && pbmp->GetWidth()==x && pbmp->GetHeight()==y)
		return;
	if(x<=0)
		x=1;
	if(y<=0)
		y=1;
	if(bmp)
		delete bmp;
	if(bmpPreview)
		delete bmpPreview;
	if(bmpBase)
		delete bmpBase;
	bmp=new Bitmap(x,y);
	bmpPreview=0;
	bmpBase=0;
}
PointF Primitive::GradPoint(int n) {
	if(n>=2)
		return PointF(rcOutline.X+ptSpec[n-2].X*rcOutline.Width,rcOutline.Y+ptSpec[n-2].Y*rcOutline.Height);
	return PointF(rcOutline.X+ptGrad[n].X*rcOutline.Width,rcOutline.Y+ptGrad[n].Y*rcOutline.Height);
}
void PrimitiveList::Copy(Primitive *dst,Primitive *src) {
	if(dst->bz)
		delete dst->bz;
	dst->bz=0;
	if(dst->texEmbed)
		delete dst->texEmbed;
	dst->texEmbed=0;
	if(dst->path)
		delete dst->path;
	dst->path=0;
	if(dst->pathOutline)
		delete dst->pathOutline;
	dst->pathOutline=0;
	if(dst->bmp)
		delete dst->bmp;
	dst->bmp=0;
	if(dst->bmpPreview)
		delete dst->bmpPreview;
	dst->bmpPreview=0;
	if(dst->bmpBase)
		delete dst->bmpBase;
	dst->bmpBase=0;
	if(dst->img)
		delete dst->img;
	dst->img=0;

	wcscpy(dst->strFile,src->strFile);
	wcscpy(dst->strFont,src->strFont);
	wcscpy(dst->strName,src->strName);
	wcscpy(dst->strText,src->strText);
	wcscpy(dst->strTextureName,src->strTextureName);
	dst->iBmpBaseDirty=dst->iBmpDirty=dst->iBmpPrevDirty=2;
	dst->prType=src->prType;

	dst->iVisible=src->iVisible;
	dst->iLock=src->iLock;
	dst->iAutoSize=src->iAutoSize;
	dst->iSmallFontOpt=src->iSmallFontOpt;
	dst->dwColor1=src->dwColor1;
	dst->dwColor2=src->dwColor2;
	dst->dwColor3=src->dwColor3;
	dst->ptGrad[0]=src->ptGrad[0];
	dst->ptGrad[1]=src->ptGrad[1];
	dst->ptSpec[0]=src->ptSpec[0];
	dst->ptSpec[1]=src->ptSpec[1];
	dst->ptfOrg[0]=src->ptfOrg[0];
	dst->ptfOrg[1]=src->ptfOrg[1];
	dst->ptRect[0]=src->ptRect[0];
	dst->ptRect[1]=src->ptRect[1];
	dst->ptRect[2]=src->ptRect[2];
	dst->ptRect[3]=src->ptRect[3];
	dst->pathOutline=src->pathOutline->Clone();
	dst->path=src->path->Clone();
	dst->rcOutline=src->rcOutline;
	dst->iOperation=src->iOperation;
	dst->iGradationType=src->iGradationType;
	dst->iLinear=src->iLinear;
	dst->iSmoother=src->iSmoother;
	dst->iFill=src->iFill;
	dst->iAntialias=src->iAntialias;
	dst->iBorder=src->iBorder;
	dst->iBorderType=src->iBorderType;
	dst->iBorderJoin=src->iBorderJoin;
	dst->iLineCap=src->iLineCap;
	dst->iClose=src->iClose;
	dst->iBorderAsFill=src->iBorderAsFill;
	dst->iPie=src->iPie;
	dst->fWidth=src->fWidth;
	dst->iAlpha=src->iAlpha;
	dst->rAngle=src->rAngle;
	dst->fSlant=src->fSlant;
	dst->iStart=src->iStart;
	dst->iStop=src->iStop;
	dst->iVertex=src->iVertex;
	dst->rTension=src->rTension;
	dst->rTwist=src->rTwist;
	dst->iStarDepth=src->iStarDepth;
	dst->iLineType=src->iLineType;
	dst->fRoundX1=src->fRoundX1;
	dst->fRoundX2=src->fRoundX2;
	dst->fRoundX3=src->fRoundX3;
	dst->fRoundX4=src->fRoundX4;
	dst->iRoundSeparate=src->iRoundSeparate;
	dst->fRoundY1=src->fRoundY1;
	dst->fRoundY2=src->fRoundY2;
	dst->fRoundY3=src->fRoundY3;
	dst->fRoundY4=src->fRoundY4;
	dst->iRoundXYSeparate=src->iRoundXYSeparate;
	dst->iCornerC1=src->iCornerC1;
	dst->iCornerC2=src->iCornerC2;
	dst->iCornerC3=src->iCornerC3;
	dst->iCornerC4=src->iCornerC4;
	dst->iDiffuse=src->iDiffuse;
	dst->light[0]=src->light[0];
	dst->light[1]=src->light[1];

	dst->iEmbossDirEnable=src->iEmbossDirEnable;
	dst->iEmbossDir=src->iEmbossDir;
	dst->rEmbossWidth=src->rEmbossWidth;
	dst->fEmbossDepth=src->fEmbossDepth;
	dst->fEmbossDepth2=src->fEmbossDepth2;
	dst->iTextureType=src->iTextureType;
	dst->iTexture=src->iTexture;
	dst->iUseTextureAlpha=src->iUseTextureAlpha;
	dst->iTextureZoomXYSepa=src->iTextureZoomXYSepa;
	dst->fTextureZoomX=src->fTextureZoomX;
	dst->fTextureZoomY=src->fTextureZoomY;
	dst->fTextureOffX=src->fTextureOffX;
	dst->fTextureOffY=src->fTextureOffY;
	dst->fTextureRot=src->fTextureRot;
	dst->dwShadowColor=src->dwShadowColor;
	dst->iShadowDirEnable=src->iShadowDirEnable;
	dst->iShadowDir=src->iShadowDir;
	dst->iShadowOffset=src->iShadowOffset;
	dst->iShadowDensity=src->iShadowDensity;
	dst->rShadowDiffuse=src->rShadowDiffuse;
	dst->dwIShadowColor=src->dwIShadowColor;
	dst->iIShadowDirEnable=src->iIShadowDirEnable;
	dst->iIShadowDir=src->iIShadowDir;
	dst->iIShadowOffset=src->iIShadowOffset;
	dst->iIShadowDensity=src->iIShadowDensity;
	dst->rIShadowDiffuse=src->rIShadowDiffuse;
	dst->fFontSize=src->fFontSize;
	dst->fFontAspect=src->fFontAspect;
	dst->fPathOffset=src->fPathOffset;
	dst->fFontSpacing=src->fFontSpacing;
	dst->iFontSizeUnit=src->iFontSizeUnit;
	dst->iUseTextPath=src->iUseTextPath;
	dst->iKeepDir=src->iKeepDir;
	dst->iBold=src->iBold;
	dst->iItalic=src->iItalic;
	dst->iFix=src->iFix;
	if(src->texEmbed) {
		dst->texEmbed=src->texEmbed->Clone();
	}
	if(src->prType==prtShape || src->prType==prtText) {
		dst->bz=new Bezier();
		*dst->bz=*src->bz;
	}
	if(src->prType==prtImage) {
		dst->img=src->img->Clone();
	}
}
void PrimitivePtrList::Insert(Primitive *p,bool bSelInit) {
	PrimitivePtrList *pl=new PrimitivePtrList();
	*pl=*this;
	bSel=bSelInit;
	next=pl;
	pr=p;
	SetCount();
}
void PrimitivePtrList::Del(Primitive *p,bool bSyncTree) {
	PrimitivePtrList *pl,*pl2;
	Primitive *p2;
	pl=this;
	if(p==0 || pl==0)
		return;
	pl=this;
	for(;pl->next!=0;) {
		if(pl->pr==p) {
			if(bSyncTree)
				theTree->MarkItem(pl->pr,0);
			pl2=pl->next;
			*pl=*pl2;
		}
		else {
			for(p2=pl->pr->prParent;p2;p2=p2->prParent) {
				if(p2!=p) {
					if(Check(p2,true))
						break;
				}
			}
			if(pl->bSel==false && p2==0) {
				if(bSyncTree)
					theTree->MarkItem(pl->pr,0);
				pl2=pl->next;
				*pl=*pl2;
			}
			else
				pl=pl->next;
		}
	}
	SetCount();
}
void PrimitivePtrList::Add(Primitive *p,bool bSel) {
	PrimitivePtrList *pl,*plTemp;
	Primitive *pOrg=p;
	HTREEITEM hitem,hitemLast;
	bool bContinue;
	RectF rcf;
	theTree->Renumber();
	for(pl=this;pl->pr!=NULL && pl->pr->iNum<p->iNum;pl=pl->next)
		;
	hitem=p->hitem;
	for(;;) {
		hitemLast=TreeView_GetNextSibling(theTree->hwnd,hitem);
		if(hitemLast==NULL) {
			hitem=TreeView_GetParent(theTree->hwnd,hitem);
			if(hitem==NULL)
				break;
		}
		else
			break;
	}
	bContinue=false;
	for(;;) {
		if((plTemp=Check(p))==0) {
//			pl->Insert(p,bSel);
			if(p==pOrg) {
				pl->Insert(p,true);
				theTree->MarkItem(p,2);
			}
			else {
				pl->Insert(p,false);
				theTree->MarkItem(p,1);
			}
			bContinue=true;
			rcf.X=pl->pr->ptfOrg[0].X;
			rcf.Y=pl->pr->ptfOrg[0].Y;
			rcf.Width=pl->pr->ptfOrg[1].X-rcf.X;
			rcf.Height=pl->pr->ptfOrg[1].Y-rcf.Y;
			if(rcf.Width<0)
				rcf.X+=rcf.Width,rcf.Width=-rcf.Width;
			if(rcf.Height<0)
				rcf.Y+=rcf.Height,rcf.Height=-rcf.Height;
			rc=Union(&rc,&rcf);
			++iCount;
			pl=pl->next;
		}
		else {
			if(p==pOrg)
				plTemp->bSel=true;
//			if(bContinue)
//				pl->bSel|=bSel;
//			else
//				pl->bSel=bSel;
//			bContinue=true;
		}
		if((p=theTree->GetNext(p))==NULL)
			break;
		if(p->hitem==hitemLast)
			break;
	}
	SetCount();
}
int aGaus[8][5][5]={
	{{19,31,19},{31,52,31},{19,31,19}},
	{{5,8,9,8,5},{8,12,14,12,8},{9,14,16,14,9},{8,12,14,12,8},{5,8,9,8,5}},

};
void Primitive::Specular(int *a,int *b,float x,float y,LightingParam *lp) {
	float fVal;
	float fVal2;
	float fPeak;
	REAL rX;
	REAL rY;
	REAL rZ,rTh;
	int aTemp;
	fVal=fPeak=0.f;
	rX=x,rY=y;
	switch(lp->iSpecularType) {
	case 1:
		fVal=-(lp->fDirX*x+lp->fDirY*y);
		fPeak=fVal+0.5f;
		if(fPeak>1.0f)
			fPeak=2.0f-fPeak;
		if(fPeak<=0)
			fPeak=0;
		else
			fPeak=expf(logf(fPeak)*(210-lp->fHighlightWidth*200)*.1f);
		fVal=fVal*lp->fSpecular*255+lp->fAmbient*512;
		aTemp=max(0,fVal);
		*a=*a*aTemp>>8;
		*b+=(int)(fPeak*lp->fHighlight*256);
		break;
	case 2:
		rZ=1.f-(x*x+y*y);
		if(rZ>0)
			rZ=sqrtf(rZ);
		fVal=-x*lp->fDir3X-y*lp->fDir3Y+(rZ-1.f)*lp->fDir3Z;
		fVal2=fVal+lp->fDir3Z;
		if(fVal2<=0)
			fPeak=0;
		else
			fPeak=expf(logf(fVal2)*(210-lp->fHighlightWidth*200)*.1f);
		fVal=fVal*lp->fSpecular*255+lp->fAmbient*512;
		aTemp=max(0,fVal);
		*a=*a*aTemp>>8;
		*b+=(int)(fPeak*lp->fHighlight*256);
		break;
	case 3:
		rTh=atan2(y,x);
		fVal=fVal2=sinf(rTh*2.f-(lp->iSpecularDir+45)*PI/90.f);
		if(fVal2<=0)
			fPeak=0;
		else
			fPeak=expf(logf(fVal2)*(210-lp->fHighlightWidth*200)*.1f);
		fVal=fVal*lp->fSpecular*255+lp->fAmbient*512;
		aTemp=(int)(max(0,fVal));
		*a=*a*aTemp>>8;
		*b+=(int)(fPeak*lp->fHighlight*256);
		break;
	}
}
void Primitive::Paint(bool bBmp,DWORD *pdwBase,int iWidth,int iHeight,int iStride) {
	int x1,x2,y,y2,iborder;
	DWORD *pdw1,dwCol;
	exs.Setup(this->iLinear,this->iSmoother);
	if(bBmp==0) {
		for(y=0;y<iHeight;++y) {
			pdw1=(DWORD*)((BYTE*)pdwBase+y*iStride);
			y2=y+iBmpOffY;
			x1=iBmpOffX,x2=x1+iWidth;
			for(;x1<x2;++x1,++pdw1) {
				dwCol=GetGrad(x1,y2);
				iborder=(*pdw1>>8)&0xff;
				dwCol=Blend(dwCol,dwColor3,iborder);
				dwCol=(dwCol&0xffffff)|(((dwCol>>24)*(*pdw1&0xff)/255)<<24);
				*pdw1=dwCol;
			}
		}
	}
	else {
		for(y=0;y<iHeight;++y) {
			pdw1=(DWORD*)((BYTE*)pdwBase+y*iStride);
			for(x1=0;x1<iWidth;++x1,++pdw1) {
				dwCol=GetGrad(x1+iBmpOffX,y+iBmpOffY);
				dwCol=(*pdw1&0xffffff)|(((dwCol>>24)*(*pdw1>>24)/255)<<24);
				*pdw1=dwCol;
			}
		}
	}
}
/*
void Primitive::MakeAlphaMap(bool bBmp,int *piAlpha,DWORD *pdwBase,int iWidth,int iHeight,int iStride) {
	int x,x1,x2,x3,y,y1,y2,y3;
	int iSum1,iSum3,iDat;
	int *pi;
	int *piAlphaTemp,*piEnd;
	DWORD *pdw1,*pdw3;
	if(iDiffuse==0) {
		if(iUseTextureAlpha) {
			theTexture->Reset(this);
			for(y=0;y<iHeight;++y) {
				pdw1=(DWORD*)((BYTE*)pdwBase+y*iStride);
				pi=piAlpha+y*iWidth;
				for(x=0;x<iWidth;++x) {
					int a=theTexture->GetAlpha(x-iWidth/2,y-iHeight/2);
					*pi++=((*pdw1++)>>24)*a/255;
				}
			}
		}
		else {
			for(y=0;y<iHeight;++y) {
				pdw1=(DWORD*)((BYTE*)pdwBase+y*iStride);
				pi=piAlpha+y*iWidth;
				piEnd=pi+iWidth;
				while(pi<piEnd) {
					*pi++=(*pdw1++)>>24;
				}
			}
		}
		return;
	}
	piAlphaTemp=new int[iWidth*iHeight];
	for(y=0;y<iHeight;++y) {
		iSum1=iSum3=0;
		pdw1=(DWORD*)((BYTE*)pdwBase+y*iStride);
		pdw3=pdw1-iDiffuse;
		pi=piAlphaTemp+y*iWidth-iDiffuse/2;
		for(x1=0,x2=-iDiffuse/2,x3=-iDiffuse;x1<iWidth+iDiffuse;++x1,++x2,++x3,++pdw1,++pdw3,++pi) {
			if(x1>=iWidth)
				iDat=0;
			else
				iDat=(*pdw1)>>24;
			iSum1+=iDat;
			if(x3>=0 && x3<iWidth)
				iDat=(*pdw3)>>24;
			else
				iDat=0;
			iSum3+=iDat;
			if(x2>=0 && x2<iWidth)
				*pi=(iSum1-iSum3)/iDiffuse;
		}
	}
	for(x=0;x<iWidth;++x) {
		int *pi1,*pi2,*pi3;
		iSum1=iSum3=0;
		pi1=piAlphaTemp+x;
		pi3=(int*)((BYTE*)pi1-iDiffuse*iStride);
		pi2=piAlpha+x-(iDiffuse/2)*iWidth;
		for(y1=0,y2=-iDiffuse/2,y3=-iDiffuse;y1<iHeight+iDiffuse;++y1,++y2,++y3,pi1=(int*)((BYTE*)pi1+iStride),pi2+=iWidth,pi3=(int*)((BYTE*)pi3+iStride)) {
			if(y1<iHeight)
				iSum1+=*pi1;
            if(y3>=0 && y3<iHeight)
				iSum3+=*pi3;
			if(y2>=0 && y2<iHeight) {
				int a=(iSum1-iSum3)/iDiffuse-128;
				*pi2=max(0,a)*2;
			}
		}
	}
	delete[] piAlphaTemp;
}
*/
void Primitive::MakeAlphaMap(bool bBmp,int *piAlpha,DWORD *pdwBase,int iWidth,int iHeight,int iStride) {
	int x,x1,x2,x3,y,y1,y2,y3;
	int iSum1,iSum3,iDat;
	int *pi;
	int *piAlphaTemp,*piEnd;
	int *pi1,*pi3;
	DWORD *pdw1;
	if(iUseTextureAlpha) {
		theTexture->Reset(this);
		for(y=0;y<iHeight;++y) {
			pdw1=(DWORD*)((BYTE*)pdwBase+y*iStride);
			pi=piAlpha+y*iWidth;
			for(x=0;x<iWidth;++x) {
				int a=theTexture->GetAlpha(x-iWidth/2,y-iHeight/2);
				*pi++=((*pdw1++)>>24)*a/255;
			}
		}
	}
	else {
		for(y=0;y<iHeight;++y) {
			pdw1=(DWORD*)((BYTE*)pdwBase+y*iStride);
			pi=piAlpha+y*iWidth;
			piEnd=pi+iWidth;
			while(pi<piEnd) {
				*pi++=(*pdw1++)>>24;
			}
		}
	}
	if(iDiffuse!=0) {
		piAlphaTemp=new int[iWidth*iHeight];
		for(y=0;y<iHeight;++y) {
			iSum1=iSum3=0;
			pi1=piAlpha+y*iWidth;
			pi3=pi1-iDiffuse;
			pi=piAlphaTemp+y*iWidth-iDiffuse/2;
			for(x1=0,x2=-iDiffuse/2,x3=-iDiffuse;x1<iWidth+iDiffuse;++x1,++x2,++x3,++pi1,++pi3,++pi) {
				if(x1>=iWidth)
					iDat=0;
				else
					iDat=*pi1;
				iSum1+=iDat;
				if(x3>=0 && x3<iWidth)
					iDat=*pi3;
				else
					iDat=0;
				iSum3+=iDat;
				if(x2>=0 && x2<iWidth)
					*pi=(iSum1-iSum3)/iDiffuse;
			}
		}
		for(x=0;x<iWidth;++x) {
			int *pi1,*pi2,*pi3;
			iSum1=iSum3=0;
			pi1=piAlphaTemp+x;
			pi3=pi1-iDiffuse*iWidth;
			pi2=piAlpha+x-(iDiffuse/2)*iWidth;
			for(y1=0,y2=-iDiffuse/2,y3=-iDiffuse;y1<iHeight+iDiffuse;++y1,++y2,++y3,pi1=(int*)((BYTE*)pi1+iStride),pi2+=iWidth,pi3=(int*)((BYTE*)pi3+iStride)) {
				if(y1<iHeight)
					iSum1+=*pi1;
				if(y3>=0 && y3<iHeight)
					iSum3+=*pi3;
				if(y2>=0 && y2<iHeight) {
					int a=(iSum1-iSum3)/iDiffuse-128;
					*pi2=max(0,a)*2;
				}
			}
		}
		delete[] piAlphaTemp;
	}
}
inline DWORD Primitive::GetGrad(int x,int y) {
	float fX,fY,fAbs1,fVal,iCoeff1,iCoeff2;
	PointF ptf0,ptfD;
	if(iGradationType==0)
		return dwColor1;
	if(ptGrad[0].Equals(ptGrad[1]))
		return dwColor1;
	ptf0.X=ptGrad[0].X*rcOutline.Width;
	ptf0.Y=ptGrad[0].Y*rcOutline.Height;
	ptfD=ptGrad[1]-ptGrad[0];
	ptfD.X*=rcOutline.Width;
	ptfD.Y*=rcOutline.Height;
	fAbs1=1.f/(ptfD.X*ptfD.X+ptfD.Y*ptfD.Y);
	fX=x-rcOutline.X-ptf0.X;
	fY=y-rcOutline.Y-ptf0.Y;
	switch(iGradationType) {
	case 1:
		fVal=((fX*ptfD.X+fY*ptfD.Y)*fAbs1);
		break;
	case 2:
		fVal=(abs(fX*ptfD.X+fY*ptfD.Y)*fAbs1);
		break;
	case 3:
		fVal=((fX*fX+fY*fY)*fAbs1);
		break;
	default:
		return dwColor1;
	}
//	return Blend(dwColor1,dwColor2,exposhape(fVal,(float)iLinear*.01f,(float)iSmoother*.01f));
	return Blend(dwColor1,dwColor2,exs.Get(fVal));
}
void Primitive::Effect(BitmapData bmpd,int iW,int iH,bool bBmp) {
	REAL rAEmboss=abs(rEmbossWidth);
	int *piAlpha,*piBumpShadow,*piBumpIShadow;
	int *pi,*pi3,*piI3,*pia;
	int a,b;
	DWORD dwCol;
	DWORD *pdw;
	int x,y;
	int iSpMax;
	int iSh;
	int iAlpha256=(iAlpha<<8)/100;
	int iTexture256=(iTexture<<8)/100;
	int iShadowDensity256=(iShadowDensity<<8)/100;
	int iIShadowDensity256=(iIShadowDensity<<8)/100;
	float fShadowDirX,fShadowDirY;
	float fIShadowDirX,fIShadowDirY;
	float rY1,rY2;
//	if(_heapchk()!=_HEAPOK)
//		MessageBox(0,L"HEAP",L"HEAP",MB_OK);
	x=iShadowDirEnable?iShadowDir:light[0].iSpecularDir;
	fShadowDirX=-sinf((float)(x*PI/180));
	fShadowDirY=cosf((float)(x*PI/180));
	x=iIShadowDirEnable?iIShadowDir:light[0].iSpecularDir;
	fIShadowDirX=-sinf((float)(x*PI/180));
	fIShadowDirY=cosf((float)(x*PI/180));
	x=iEmbossDirEnable?iEmbossDir:light[0].iSpecularDir;
	fEmbossDirX=-sinf((float)(x*PI/180));
	fEmbossDirY=cosf((float)(x*PI/180));
	if(iW==0||iH==0)
		return;
	iSh=(int)(min(8,max(0,rShadowDiffuse)));
	rAEmboss=min(16,max(0,rAEmboss));
	piAlpha=new int[iW*iH];
	piBumpShadow=new int[iW*iH];
	piBumpIShadow=new int[iW*iH];
	ZeroMemory(piAlpha,iW*iH*sizeof(int));
	piBumpEmboss=new int[iW*iH];
	ZeroMemory(piBumpEmboss,iW*iH*sizeof(int));
	if(theScreen->fRender && (fEmbossDepth || fEmbossDepth2)) {
		theGausian->SetMode(rAEmboss,fEmbossDepth,fEmbossDepth2,prType==prtImage?true:false);
		theGausian->AlphaToIntDiff((DWORD*)bmpd.Scan0,piBumpEmboss,bmpd.Width,bmpd.Height,bmpd.Stride,fEmbossDirX,fEmbossDirY);
	}
	Paint(bBmp,(DWORD*)bmpd.Scan0,bmpd.Width,bmpd.Height,bmpd.Stride);
	if(theScreen->fRender) {
		MakeAlphaMap(bBmp,piAlpha,(DWORD*)bmpd.Scan0,bmpd.Width,bmpd.Height,bmpd.Stride);
		pi=piBumpShadow;
		if(iShadowDensity)
			theGausian->AlphaToInt(rShadowDiffuse,piAlpha,piBumpShadow,bmpd.Width,bmpd.Height,iShadowOffset,fShadowDirX,fShadowDirY,FALSE);
		if(iIShadowDensity)
			theGausian->AlphaToInt(rIShadowDiffuse,piAlpha,piBumpIShadow,bmpd.Width,bmpd.Height,iIShadowOffset,fIShadowDirX,fIShadowDirY,TRUE);
		iSpMax=iW+iH;
		theTexture->Reset(this);
		PointF ptfCenter=GradPoint(2);
		PointF ptfCenter2=GradPoint(3);
		ptfCenter.X-=iBmpOffX;
		ptfCenter.Y-=iBmpOffY;
		ptfCenter2.X-=iBmpOffX;
		ptfCenter2.Y-=iBmpOffY;
		PointF ptfSize(ptfOrg[1].X-ptfOrg[0].X,ptfOrg[1].Y-ptfOrg[0].Y);
		light[0].Setup();
		light[1].Setup();
		for(y=0;y<iH;++y) {
			pdw=(DWORD*)((BYTE*)bmpd.Scan0+y*bmpd.Stride);
			pi=piBumpEmboss+y*iW;
			pi3=piBumpShadow+y*iW;
			piI3=piBumpIShadow+y*iW;
			pia=piAlpha+y*iW;
			rY1=(y-ptfCenter.Y)*2/ptfSize.Y;
			rY2=(y-ptfCenter2.Y)*2/ptfSize.Y;
			for(x=0;x<iW;++x) {
				int ialpha,iIalpha;
				ialpha=iAlpha256*(*pia)>>8;
				dwCol=*pdw&0xffffff;
				a=255;
				b=0;
				if(light[0].iSpecularType)
					Specular(&a,&b,(x-ptfCenter.X)*2/ptfSize.X,rY1,&light[0]);
				if(light[1].iSpecularType)
					Specular(&a,&b,(x-ptfCenter2.X)*2/ptfSize.X,rY2,&light[1]);
				if(iTexture256) {
//	 				b+=((theTexture->Get(x-iW/2,y-iH/2)*iTexture255)>>8);
					b+=((theTexture->Get(x-iW/2,y-iH/2)*iTexture256)>>8);
				}
				if(fEmbossDepth||fEmbossDepth2) {
					b+=(*pi>>10);
				}
				*pdw=(ialpha<<24)|Bright(dwCol,a,b);
				if(iIShadowDensity && ialpha!=0) {
					iIalpha=(((ialpha* *piI3)>>16) *iIShadowDensity256)>>8;
//					iIalpha=(*piI3>>8)*iIShadowDensity255>>8;
					*pdw=Blend(*pdw,(iIalpha<<24)|dwIShadowColor);
				}
				if(iShadowDensity && ialpha!=255) {
//					ialpha=((((255-ialpha)* (*pi3>>8))>>8) *iShadowDensity255)>>8;
					ialpha=(*pi3>>8)*iShadowDensity256>>8;
					*pdw=Blend((ialpha<<24)|dwShadowColor,*pdw);
				}
				++pdw;
				++pi;
				++pi3;
				++piI3;
				++pia;
			}
		}
	}
	delete[] piAlpha;
	delete[] piBumpShadow;
	delete[] piBumpIShadow;
	delete[] piBumpEmboss;
	piBumpEmboss=0;
}
void Primitive::MakeCircle(void) {
	float fHandle=55.2285f*.5f;
	PointF ptf;
	bz->iIndex=12;
	ptf=ptfOrg[0];
	bz->ptAnchor[0]=ptf+PointF(50-fHandle,0);
	bz->ptAnchor[1]=ptf+PointF(50,0);
	bz->ptAnchor[2]=ptf+PointF(50+fHandle,0);
	bz->ptAnchor[3]=ptf+PointF(100,50-fHandle);
	bz->ptAnchor[4]=ptf+PointF(100,50);
	bz->ptAnchor[5]=ptf+PointF(100,50+fHandle);
	bz->ptAnchor[6]=ptf+PointF(50+fHandle,100);
	bz->ptAnchor[7]=ptf+PointF(50,100);
	bz->ptAnchor[8]=ptf+PointF(50-fHandle,100);
	bz->ptAnchor[9]=ptf+PointF(0,50+fHandle);
	bz->ptAnchor[10]=ptf+PointF(0,50);
	bz->ptAnchor[11]=ptf+PointF(0,50-fHandle);
	ptfOrg[1]=ptfOrg[0]+PointF(100,100);
	SetPos(ptfOrg[0].X,ptfOrg[0].Y,ptfOrg[1].X-ptfOrg[0].X,ptfOrg[1].Y-ptfOrg[0].Y,0);
	theProp->SetupPos(this);
	theScreen->Send();
}
void Primitive::Normalize(void) {
	PointF ptf0,ptf1;
	ptf0=ptfOrg[0];
	ptf1=ptfOrg[1];
	if(ptf0.X>ptf1.X) {
		FSwap(&ptf0.X,&ptf1.X);
		bz->MirrorH((ptf0.X+ptf1.X)*.5f);
	}
	if(ptf0.Y>ptf1.Y) {
		FSwap(&ptf0.Y,&ptf1.Y);
		bz->MirrorV((ptf0.Y+ptf1.Y)*.5f);
	}
	SetPos(ptf0.X,ptf0.Y,ptf1.X-ptf0.X,ptf1.Y-ptf0.Y,0);
	theProp->SetupPos(this);
}
void OperationAdd(Bitmap *bmp1,Bitmap *bmp2) {
	int x,y,p1,p2,b1,b2,i1,i2;
	Rect rc;
	BitmapData bmpd1,bmpd2;
	DWORD *pdw1,*pdw2;
	rc.X=0,rc.Y=0;
	rc.Width=bmp1->GetWidth();
	rc.Height=bmp1->GetHeight();
	bmp1->LockBits(&rc,ImageLockModeRead|ImageLockModeWrite,PixelFormat32bppARGB,&bmpd1);
	bmp2->LockBits(&rc,ImageLockModeRead,PixelFormat32bppARGB,&bmpd2);
	for(y=0;y<rc.Height;++y) {
		pdw1=(DWORD*)((char*)bmpd1.Scan0+bmpd1.Stride*y);
		pdw2=(DWORD*)((char*)bmpd2.Scan0+bmpd2.Stride*y);
		for(x=0;x<rc.Width;++x,++pdw1,++pdw2) {
			p1=b1=i1=*pdw1;
			p2=b2=i2=*pdw2;
			p1&=0xff;
			p2&=0xff;
			b1&=0xff00;
			b2&=0xff00;
			i1&=0xff0000;
			i2&=0xff0000;
			if(p2>p1)
				p1=p2;
			if(b2<b1)
				b1=b2;
			if(i2>i1)
				i1=i2;
			*pdw1=0xff000000|i1|p1|b1;
		}
	}
	bmp1->UnlockBits(&bmpd1);
	bmp2->UnlockBits(&bmpd2);
}
void OperationMul(Bitmap *bmp1,bool bBmp1,Bitmap *bmp2) {
	int x,y,p1,p2,b1,b2,i1,i2;
	Rect rc;
	BitmapData bmpd1,bmpd2;
	DWORD *pdw1,*pdw2;
	rc.X=0,rc.Y=0;
	rc.Width=bmp1->GetWidth();
	rc.Height=bmp1->GetHeight();
	bmp1->LockBits(&rc,ImageLockModeRead|ImageLockModeWrite,PixelFormat32bppARGB,&bmpd1);
	bmp2->LockBits(&rc,ImageLockModeRead,PixelFormat32bppARGB,&bmpd2);
	if(bBmp1) {
		for(y=0;y<rc.Height;++y) {
			pdw1=(DWORD*)((char*)bmpd1.Scan0+bmpd1.Stride*y);
			pdw2=(DWORD*)((char*)bmpd2.Scan0+bmpd2.Stride*y);
			for(x=0;x<rc.Width;++x,++pdw1,++pdw2) {
				p1=*pdw2&0xff;
				if(p1<(*pdw1>>24))
					*pdw1=(*pdw1&0xffffff)|(p1<<24);
			}
		}
	}
	else {
		for(y=0;y<rc.Height;++y) {
			pdw1=(DWORD*)((char*)bmpd1.Scan0+bmpd1.Stride*y);
			pdw2=(DWORD*)((char*)bmpd2.Scan0+bmpd2.Stride*y);
			for(x=0;x<rc.Width;++x,++pdw1,++pdw2) {
				p1=b1=i1=*pdw1;
				p2=b2=i2=*pdw2;
				p1&=0xff;
				p2&=0xff;
				b1&=0xff00;
				b2&=0xff00;
				i1&=0xff0000;
				i2&=0xff0000;
				if(p2<p1)
					p1=p2;
				if(b2>b1)
					b1=b2;
				if(i2<i1)
					i1=i2;
				*pdw1=0xff000000|i1|p1|b1;
			}
		}
	}
	bmp1->UnlockBits(&bmpd1);
	bmp2->UnlockBits(&bmpd2);
}
void OperationSub(Bitmap *bmp1,bool bBmp1,Bitmap *bmp2) {
	int x,y,p1,p2,b1,b2,i1,i2;
	Rect rc;
	BitmapData bmpd1,bmpd2;
	DWORD *pdw1,*pdw2;
	rc.X=0,rc.Y=0;
	rc.Width=bmp1->GetWidth();
	rc.Height=bmp1->GetHeight();
	bmp1->LockBits(&rc,ImageLockModeRead|ImageLockModeWrite,PixelFormat32bppARGB,&bmpd1);
	bmp2->LockBits(&rc,ImageLockModeRead,PixelFormat32bppARGB,&bmpd2);
	if(bBmp1) {
		for(y=0;y<rc.Height;++y) {
			pdw1=(DWORD*)((char*)bmpd1.Scan0+bmpd1.Stride*y);
			pdw2=(DWORD*)((char*)bmpd2.Scan0+bmpd2.Stride*y);
			for(x=0;x<rc.Width;++x,++pdw1,++pdw2) {
				p1=0xff-*pdw2&0xff;
				if(p1<(*pdw1>>24))
					*pdw1=(*pdw1&0xffffff)|(p1<<24);
			}
		}
	}
	else {
		for(y=0;y<rc.Height;++y) {
			pdw1=(DWORD*)((char*)bmpd1.Scan0+bmpd1.Stride*y);
			pdw2=(DWORD*)((char*)bmpd2.Scan0+bmpd2.Stride*y);
			for(x=0;x<rc.Width;++x,++pdw1,++pdw2) {
				p1=b1=i1=*pdw1;
				p2=b2=i2=*pdw2;
				p1&=0xff;
				b1&=0xff00;
				i1&=0xff0000;
				p2&=0xff;
				b2&=0xff00;
				i2&=0xff0000;
				if(0xff-(i2>>16)<p1)
					p1=0xff-(i2>>16);
				if((p2<<8)>b1)
					b1=(p2<<8);
				i1=p1;
				if(0xff-(b1>>8)<i1)
					i1=0xff-(b1>>8);
				i1<<=16;
				*pdw1=0xff000000|i1|b1|p1;
			}
		}
	}
	bmp1->UnlockBits(&bmpd1);
	bmp2->UnlockBits(&bmpd2);
}
void Primitive::DrawBasePaint(Graphics *g,Primitive *prBase,Bitmap *bmp,BasePaintMode mode,Color col,int iOffX,int iOffY) {
	Matrix mx;
	GraphicsPath *pathTemp=path->Clone();
	mx.Translate(iBmpOffX-iOffX,iBmpOffY-iOffY);
	pathTemp->Transform(&mx);
	switch(mode) {
	case Normal:
		g->FillPath(&SolidBrush(col),pathTemp);
		break;
	case Invert:
		pathTemp->SetFillMode(FillModeAlternate);
		PathAddRect(pathTemp,RectF(0,0,bmp->GetWidth(),bmp->GetHeight()));
		g->FillPath(&SolidBrush(col),pathTemp);
		break;
	case Border:
		Pen pen(col,prBase->fWidth);
		pen.SetLineJoin((Gdiplus::LineJoin)prBase->iBorderJoin);
		pen.SetLineCap((Gdiplus::LineCap)iLineCap,(Gdiplus::LineCap)prBase->iLineCap,DashCapFlat);
		g->DrawPath(&pen,pathTemp);
		break;
	}
	delete pathTemp;
}
void Primitive::BasePaint(Graphics *g,Primitive *prBase,Bitmap *bmp,int iOffX,int iOffY) {
	int iDrawBorder=prBase->iBorder;
	int iAsFill=0;
	if(iDrawBorder!=0)
		iDrawBorder=prBase->iBorderType+1;
	if(iOperation==2) {
		iDrawBorder=((iDrawBorder&1)<<1)|((iDrawBorder&2)>>1);
	}
	if(iDrawBorder!=0 && iBorderAsFill==0)
		iAsFill=255;
	g->Clear(Color(255,0,iAsFill,0));
	if(iDrawBorder==1)
		DrawBasePaint(g,prBase,bmp,Border,Color(255,0,iAsFill,255),iOffX,iOffY);
	if(iFill&&prType!=prtLines)
		DrawBasePaint(g,prBase,bmp,Normal,Color(255,255,0,255),iOffX,iOffY);
	if(iDrawBorder==2 || iDrawBorder==3)
		DrawBasePaint(g,prBase,bmp,Border,Color(255,0,iAsFill,255),iOffX,iOffY);
	if(iDrawBorder==2)
		DrawBasePaint(g,prBase,bmp,Invert,Color(255,0,iAsFill,0),iOffX,iOffY);
}
void MixImage(DWORD *pdwDest,int iWD,int iHD,int iPosX,int iPosY,DWORD *pdwSrc,int iWS,int iHS) {
	int y;
	DWORD *pdwD,*pdwS,*pdwDEnd;
	int iSrcStride;

	if(iWD==0 || iHD==0)
		return;
	iSrcStride=iWS;
	if(iPosX<0)
		iWS+=iPosX,pdwSrc-=iPosX,iPosX=0;
	if(iPosX+iWS>iWD)
		iWS=iWD-iPosX;
	if(iPosY<0)
		iHS+=iPosY,pdwSrc-=iPosY*iSrcStride,iPosY=0;
	if(iPosY+iHS>iHD)
		iHS=iHD-iPosY;
	for(y=0;y<iHS && theScreen->fRenderBreak==false;++y) {
		pdwD=pdwDest+(y+iPosY)*iWD+iPosX;
		pdwS=pdwSrc+y*iSrcStride;
		pdwDEnd=pdwD+iWS;
		while(pdwD<pdwDEnd) {
			*pdwD=Blend(*pdwD,*pdwS);
			++pdwD;
			++pdwS;
		}
	}
}
void InvY(HDC hdc,int x0,int y0,int x1,int y1) {
	HBITMAP hbmp,hbmpOld;
	HDC hdcMem;
	int sx,sy;
	Sort(&x0,&x1);
	Sort(&y0,&y1);
	sx=x1-x0;
	sy=y1-y0;
	hbmp=CreateCompatibleBitmap(hdc,sx,1);
	hdcMem=CreateCompatibleDC(hdc);
	hbmpOld=(HBITMAP)SelectObject(hdcMem,(HGDIOBJ)hbmp);
	--y1;
	while(y1>y0) {
		BitBlt(hdcMem,0,0,sx,1,hdc,x0,y0,SRCCOPY);
		BitBlt(hdc,x0,y0,sx,1,hdc,x0,y1,SRCCOPY);
		BitBlt(hdc,x0,y1,sx,1,hdcMem,0,0,SRCCOPY);
		--y1;
		++y0;
	}
	SelectObject(hdcMem,(HGDIOBJ)hbmpOld);
	DeleteDC(hdcMem);
	DeleteObject((HGDIOBJ)hbmp);
}
//BaseBmp
//  0xaarrggbb
//   bb:paint part
//   gg:border part
//   rr:body part
bool Primitive::MakeBaseBmp(Bitmap **pbmp) {
	PointF pt0,ptfCur;
	wchar_t strTemp[2];
	float fEmSize;
	FontFamily *fontf;
	Font *font;
	int style;
	float fXSpace,fCWidth,fOffs;
	wchar_t *str;
	Brush *br;
	Primitive *pChild;
	bool bBmp;
	PointF ptfVertex[3];
	PointF ptfOff;
	RectF rcf2,rcfM;
	Bitmap *bmp2;
	Graphics *g2,*gBmp;
	int iDrawBorder;
	int x,y;
//	if(*pbmp==NULL || (*pbmp)->GetWidth()!=rcBmp.Width || (*pbmp)->GetHeight()!=rcBmp.Height) {
//		SetBmpSize((int)(rcBmp.Width+0.5f),(int)(rcBmp.Height+0.5f));
//	}
	SetBmpSize(*pbmp,rcBmp);
	gBmp=new Graphics(*pbmp);
	if(theScreen->fRender) {
		gBmp->SetPixelOffsetMode(PixelOffsetModeHighQuality);
	}
	if(iAntialias && theScreen->fRender)
		gBmp->SetSmoothingMode(SmoothingModeHighQuality);
	else
		gBmp->SetSmoothingMode(SmoothingModeNone);
	iDrawBorder=iBorder;
	if(iDrawBorder)
		iDrawBorder=iBorderType+1;
	if(prType==prtLines)
		iDrawBorder=3;
	switch(prType) {
	case prtImage:
		gBmp->Clear(Color(0,255,255,255));
		gBmp->SetInterpolationMode(InterpolationModeHighQualityBilinear);
		if(rAngle==0.f && fSlant==0.f)
			gBmp->DrawImage(img,ptRect[0].X-iBmpOffX,ptRect[0].Y-iBmpOffY,ptRect[2].X-ptRect[0].X,ptRect[2].Y-ptRect[0].Y);
		else {
			ptfOff=PointF(iBmpOffX,iBmpOffY);
			ptfVertex[0]=ptRect[0]-ptfOff;
			ptfVertex[1]=ptRect[1]-ptfOff;
			ptfVertex[2]=ptRect[3]-ptfOff;
			gBmp->DrawImage(img,ptfVertex,3);
		}
		bBmp=true;
		pChild=theTree->GetChild(this);
		if(pChild) {
			x=(*pbmp)->GetWidth();
			y=(*pbmp)->GetHeight();
			bmp2=new Bitmap(x,y);
			g2=new Graphics(bmp2);
			if(theScreen->fRender) {
				g2->SetPixelOffsetMode(PixelOffsetModeHighQuality);
			}
			if(iAntialias && theScreen->fRender)
				g2->SetSmoothingMode(SmoothingModeHighQuality);
			else
				g2->SetSmoothingMode(SmoothingModeNone);
			for(;pChild;pChild=theTree->GetNextSibling(pChild)) {
				pChild->BasePaint(g2,this,*pbmp,iBmpOffX,iBmpOffY);
				switch(pChild->iOperation) {
				case 1:
					break;
				case 2:
					OperationSub(*pbmp,true,bmp2);
					break;
				case 3:
					OperationMul(*pbmp,true,bmp2);
					break;
				}
			}
			delete g2;
			delete bmp2;
		}
		break;
	case prtText:
		if(iSmallFontOpt && rAngle==0 && fFontAspect==100.f && iFill==1 && iBorder==0 && iUseTextPath==0) {
			bBmp=false;
			if(iAntialias)
				gBmp->SetTextRenderingHint(TextRenderingHintAntiAliasGridFit);
			else
				gBmp->SetTextRenderingHint(TextRenderingHintSingleBitPerPixelGridFit);
			gBmp->Clear(Color(255,0,0,0));
			fontf=new FontFamily(strFont);
			br=new SolidBrush(Color(255,0,0,255));
			ptfCur.X=min(ptfOrg[0].X,ptfOrg[1].X);
			ptfCur.Y=min(ptfOrg[0].Y,ptfOrg[1].Y);
			style=0;
			if(iBold)
				style|=FontStyleBold;
			if(iItalic)
				style|=FontStyleItalic;
			if(iFontSizeUnit==0)
				fEmSize=fFontSize;
			else
				fEmSize=fFontSize*96/72;
			fEmSize=max(0.01,abs(fEmSize));
			font=new Font(fontf,max(0.01,abs(fEmSize)),style,UnitPixel);
			gBmp->MeasureString(L"  ",-1,font,PointF(0.f,0.f),&rcf2);
			gBmp->MeasureString(L"M ",-1,font,PointF(0.f,0.f),&rcfM);
			fXSpace=rcf2.Width;
			fOffs=0.f;
			for(str=strText;*str;) {
				if(*str==0x20) {
					ptfCur.X+=fXSpace+fXSpace*(fFontSpacing-100)*.01f;
					++str;
					continue;
				}
				strTemp[0]=*str;
				strTemp[1]=0;
				gBmp->MeasureString(strTemp,1,font,PointF(0.f,0.f),&rcf2);
				if(iFix) {
					fCWidth=(rcfM.Width-fXSpace)+fXSpace*(fFontSpacing-100)*.01f;
					fOffs=(rcfM.Width-rcf2.Width)*.5f;
				}
				else {
					fCWidth=(rcf2.Width-fXSpace)+fXSpace*(fFontSpacing-100)*.01f;
				}
				pt0=ptfCur;
				pt0.X=pt0.X+fOffs-iBmpOffX;
				pt0.Y=pt0.Y-iBmpOffY;
				gBmp->DrawString(strTemp,1,font,pt0,br);
				++str;
				ptfCur.X+=fCWidth;
			}
			if(ptfOrg[0].Y>ptfOrg[1].Y) {
				InvY(gBmp->GetHDC(),ptfOrg[0].X,ptfOrg[1].X,ptfOrg[1].Y,ptfOrg[0].Y);
			}
			delete br;
			delete fontf;
			delete font;
			break;
		}
		// else dont break;
	default:
		bBmp=false;
		BasePaint(gBmp,this,*pbmp,iBmpOffX,iBmpOffY);
		pChild=theTree->GetChild(this);
		if(pChild) {
			x=(*pbmp)->GetWidth();
			y=(*pbmp)->GetHeight();
			bmp2=new Bitmap(x,y);
			g2=new Graphics(bmp2);
			if(theScreen->fRender) {
				g2->SetPixelOffsetMode(PixelOffsetModeHighQuality);
			}
			if(iAntialias && theScreen->fRender)
				g2->SetSmoothingMode(SmoothingModeHighQuality);
			else
				g2->SetSmoothingMode(SmoothingModeNone);
			for(;pChild;pChild=theTree->GetNextSibling(pChild)) {
				pChild->BasePaint(g2,this,*pbmp,iBmpOffX,iBmpOffY);
				switch(pChild->iOperation) {
				case 1:
					OperationAdd(*pbmp,bmp2);
					break;
				case 2:
					OperationSub(*pbmp,false,bmp2);
					break;
				case 3:
					OperationMul(*pbmp,false,bmp2);
					break;
				}
			}
			delete g2;
			delete bmp2;
		}
		break;
	}
	delete gBmp;
	return bBmp;
}
void Primitive::Assemble(void) {
	PointF pt0,ptfCur;
	bool bBmp;
	PointF ptfVertex[3];
	PointF ptfOff;
	RectF rcf2,rcfM;
	BitmapData bmpd;
	bBmp=false;
	if((iVisible&4)==0)
		return;
	if(iOperation)
		return;
	if(iBmpDirty==2 || bmp==0) {
		bBmp=MakeBaseBmp(&bmp);
		int iW=bmp->GetWidth();
		int iH=bmp->GetHeight();
		Rect rc(0,0,iW,iH);
		bmp->LockBits(&rc,ImageLockModeRead|ImageLockModeWrite,PixelFormat32bppARGB,&bmpd);
		Effect(bmpd,iW,iH,bBmp);
		iBmpDirty=0;
		MixImage(theScreen->pdwMem,theScreen->iWidth,theScreen->iHeight,iBmpOffX,iBmpOffY,(DWORD*)bmpd.Scan0,bmpd.Stride>>2,bmpd.Height);
		bmp->UnlockBits(&bmpd);
	}
	else {
		int iW=bmp->GetWidth();
		int iH=bmp->GetHeight();
		Rect rc(0,0,iW,iH);
		bmp->LockBits(&rc,ImageLockModeRead|ImageLockModeWrite,PixelFormat32bppARGB,&bmpd);
		MixImage(theScreen->pdwMem,theScreen->iWidth,theScreen->iHeight,iBmpOffX,iBmpOffY,(DWORD*)bmpd.Scan0,bmpd.Stride>>2,bmpd.Height);
		bmp->UnlockBits(&bmpd);
	}
}
void Primitive::CalcParent(bool bSkipThis) {
	Primitive *p;
	p=this;
	if(bSkipThis)
		p=theTree->GetParent(p);
	for(;p;p=theTree->GetParent(p)) {
		if(p->prType==prtGroup)
			p->CalcPath(2,1);
	}	
}
bool ClipLine(PointF *ptf1,PointF *ptf2,RectF *rcf) {
	REAL x,y;
	x=rcf->X;
	if(ptf1->X<x) {
		if(ptf2->X<x)
			return false;
		ptf1->Y=ptf1->Y+(ptf2->Y-ptf1->Y)*(x-ptf1->X)/(ptf2->X-ptf1->X);
		ptf1->X=x;
	}
	if(ptf2->X<x) {
		ptf2->Y=ptf2->Y+(ptf1->Y-ptf2->Y)*(x-ptf2->X)/(ptf1->X-ptf2->X);
		ptf2->X=x;
	}
	x=rcf->X+rcf->Width;
	if(ptf1->X>x) {
		if(ptf2->X>x)
			return false;
		ptf1->Y=ptf1->Y+(ptf2->Y-ptf1->Y)*(x-ptf1->X)/(ptf2->X-ptf1->X);
		ptf1->X=x;
	}
	if(ptf2->X>x) {
		ptf2->Y=ptf2->Y+(ptf1->Y-ptf2->Y)*(x-ptf2->X)/(ptf1->X-ptf2->X);
		ptf2->X=x;
	}
	y=rcf->Y;
	if(ptf1->Y<y) {
		if(ptf2->Y<y)
			return false;
		ptf1->X=ptf1->X+(ptf2->X-ptf1->X)*(y-ptf1->Y)/(ptf2->Y-ptf1->Y);
		ptf1->Y=y;
	}
	if(ptf2->Y<y) {
		ptf2->X=ptf2->X+(ptf1->X-ptf2->X)*(y-ptf2->Y)/(ptf1->Y-ptf2->Y);
		ptf2->Y=y;
	}
	y=rcf->Y+rcf->Height;
	if(ptf1->Y>y) {
		if(ptf2->Y>y)
			return false;
		ptf1->X=ptf1->X+(ptf2->X-ptf1->X)*(y-ptf1->Y)/(ptf2->Y-ptf1->Y);
		ptf1->Y=y;
	}
	if(ptf2->Y>y) {
		ptf2->X=ptf2->X+(ptf1->X-ptf2->X)*(y-ptf2->Y)/(ptf1->Y-ptf2->Y);
		ptf2->Y=y;
	}
	return true;
}
void Primitive::CalcPath(int iDirty,int iAdjustRect) {
	RectF rc,rcTL,rcTR,rcBL,rcBR,rcf,rcf2,rect2,rcfM;
	PointF ptfOrgOld[2],pt0,pt,pt1,pt2,ptfCenter,ptfCenterNew,ptfCenterNew2,ptfCur,*ptfTab;
	int iRSgn1,iRSgn2,iRSgn3,iRSgn4,style,i,iS1,iS2;
	GraphicsPath *pathTemp;
	double rA,rX,rY,dRatio,dSsin,dScos;
	float fXSpace,fHorzRatio,fVertRatio,dx,dy,rAng,fCWidth,fOffs;
	float fRX1,fRX2,fRX3,fRX4,fRY1,fRY2,fRY3,fRY4,fSsin,fScos,fS1,fS2;
	Primitive *pChild;
	wchar_t wText[320],*str,strTemp[2];
	FontFamily *fontf;
	Font *font;
	Graphics *g;
	Matrix mx;
	if(rAngle>=0.f)
		rAng=rAngle;
	else
		rAng=360.f+rAngle;
	ptfCenter=PointF((ptfOrg[0].X+ptfOrg[1].X)*.5f,(ptfOrg[0].Y+ptfOrg[1].Y)*.5f);
	RectF rect(ptfOrg[0].X,ptfOrg[0].Y,ptfOrg[1].X-ptfOrg[0].X,ptfOrg[1].Y-ptfOrg[0].Y);
	if(rect.Height==0)
		rect.Height=0.001f;
	rect2=rect;
	path->Reset();
	pathOutline->Reset();
	iNum=3;
	switch(prType) {
	case prtRect:
		dx=abs(ptfOrg[1].X-ptfOrg[0].X);
		dy=abs(ptfOrg[1].Y-ptfOrg[0].Y);
		if(iRoundSeparate) {
			iRSgn1=iCornerC1,iRSgn2=iCornerC2,iRSgn3=iCornerC3,iRSgn4=iCornerC4;
			fRX1=fRY1=fRoundX1,fRX2=fRY2=fRoundX2,fRX3=fRY3=fRoundX3,fRX4=fRY4=fRoundX4;
			if(iRoundXYSeparate) {
				fRY1=fRoundY1,fRY2=fRoundY2,fRY3=fRoundY3,fRY4=fRoundY4;
			}
		}
		else {
			iRSgn1=iRSgn2=iRSgn3=iRSgn4=iCornerC1;
			fRX1=fRX2=fRX3=fRX4=fRY1=fRY2=fRY3=fRY4=fRoundX1;
			if(iRoundXYSeparate)
				fRY1=fRY2=fRY3=fRY4=fRoundY1;
		}
		if(fRX1==0 && fRX2==0 && fRX3==0 && fRX4==0) {
			PathAddRect(path,rect);
		}
		else {
			pt1=ptfOrg[0],pt2=ptfOrg[1];
			if(rect.Width<0) {
				FSwap(&pt1.X,&pt2.X);
				ISwap(&iRSgn1,&iRSgn2),ISwap(&iRSgn3,&iRSgn4);
				FSwap(&fRX1,&fRX2),FSwap(&fRY1,&fRY2),FSwap(&fRX3,&fRX4),FSwap(&fRY3,&fRY4);
			}
			if(rect.Height<0) {
				FSwap(&pt1.Y,&pt2.Y);
				ISwap(&iRSgn1,&iRSgn4),ISwap(&iRSgn2,&iRSgn3);
				FSwap(&fRY1,&fRY4),FSwap(&fRX1,&fRX4),FSwap(&fRY2,&fRY3),FSwap(&fRX2,&fRX3);
			}
			switch(iRSgn1) {
			case 0:	path->AddArc(RectF(pt1.X,pt1.Y,fRX1,fRY1),180.f,90.f);		break;
			case 1:	path->AddLine(pt1.X,pt1.Y+fRY1*.5f,pt1.X+fRX1*.5f,pt1.Y);	break;
			case 2:	path->AddArc(RectF(pt1.X-fRX1*.5f,pt1.Y-fRY1*.5f,fRX1,fRY1),90.f,-90.f); break;
			case 3:
				path->AddLine(pt1.X,pt1.Y+fRY1*.5f,pt1.X+fRX1*.5f,pt1.Y+fRY1*.5f);
				path->AddLine(pt1.X+fRX1*.5f,pt1.Y+fRY1*.5f,pt1.X+fRX1*.5f,pt1.Y);
				break;
			}
			path->AddLine(pt1.X+fRX1*.5f,pt1.Y,pt2.X-fRX2*.5f,pt1.Y);
			switch(iRSgn2) {
			case 0:	path->AddArc(RectF(pt2.X-fRX2,pt1.Y,fRX2,fRY2),270.f,90.f);	break;
			case 1:	path->AddLine(pt2.X-fRX2*.5f,pt1.Y,pt2.X,pt1.Y+fRY2*.5f);	break;
			case 2:	path->AddArc(RectF(pt2.X-fRX2*.5f,pt1.Y-fRY2*.5f,fRX2,fRY2),180.f,-90.f); break;
			case 3:
				path->AddLine(pt2.X-fRX2*.5f,pt1.Y,pt2.X-fRX2*.5f,pt1.Y+fRY2*.5f);
				path->AddLine(pt2.X-fRX2*.5f,pt1.Y+fRY2*.5f,pt2.X,pt1.Y+fRY2*.5f);
				break;
			}
			path->AddLine(pt2.X,pt1.Y+fRY2*.5f,pt2.X,pt2.Y-fRY3*.5f);
			switch(iRSgn3) {
			case 0:	path->AddArc(RectF(pt2.X-fRX3,pt2.Y-fRY3,fRX3,fRY3),0.f,90.f);	break;
			case 1:	path->AddLine(pt2.X,pt2.Y-fRY3*.5f,pt2.X-fRX3*.5f,pt2.Y);	break;
			case 2:	path->AddArc(RectF(pt2.X-fRX3*.5f,pt2.Y-fRY3*.5f,fRX3,fRY3),270.f,-90.f); break;
			case 3:
				path->AddLine(pt2.X,pt2.Y-fRY3*.5f,pt2.X-fRX3*.5f,pt2.Y-fRY3*.5f);
				path->AddLine(pt2.X-fRX3*.5f,pt2.Y-fRY3*.5f,pt2.X-fRX3*.5f,pt2.Y);
				break;
			}
			path->AddLine(pt2.X-fRX3*.5,pt2.Y,pt1.X+fRX4*.5f,pt2.Y);
			switch(iRSgn4) {
			case 0:	path->AddArc(RectF(pt1.X,pt2.Y-fRY4,fRX4,fRY4),90.f,90.f);	break;
			case 1:	path->AddLine(pt1.X+fRX4*.5f,pt2.Y,pt1.X,pt2.Y-fRY4*.5f);	break;
			case 2:	path->AddArc(RectF(pt1.X-fRX4*.5f,pt2.Y-fRY4*.5f,fRX4,fRY4),0.f,-90.f);	break;
			case 3:
				path->AddLine(pt1.X+fRX4*.5f,pt2.Y,pt1.X+fRX4*.5f,pt2.Y-fRY4*.5f);
				path->AddLine(pt1.X+fRX4*.5f,pt2.Y-fRY4*.5,pt1.X,pt2.Y-fRY4*.5f);
				break;
			}
			path->AddLine(pt1.X,pt2.Y-fRY4*.5f,pt1.X,pt1.Y+fRY1*.5f);
			path->CloseFigure();
		}
		PathAddRect(pathOutline,rect);
		break;
	case prtEllipse:
		iS1=iStart;
		iS2=iStop;
		if(rect.Width<0) {
			rect.X+=rect.Width;
			rect.Width=-rect.Width;
			iS1=-iS1;
			iS2=-iS2;
		}
		if(rect.Height<0) {
			rect.Y+=rect.Height;
			rect.Height=-rect.Height;
			iS1=180-iS1;
			iS2=180-iS2;
		}
		iS1=iS1-90;
		iS2=iS2-90;
		if(iS1>iS2)
			Sort(&iS1,&iS2);
		if(iS2>=iS1+360)
			path->AddEllipse(rect);
		else {
			if(iPie) {
				PointF ptf;
				path->AddArc(rect,(REAL)iS1,(REAL)(iS2-iS1));
				path->GetLastPoint(&ptf);
				path->AddLine(ptf,PointF(rect.X+rect.Width*.5f,rect.Y+rect.Height*.5f));
				if(iClose)
					path->CloseFigure();
			}
			else {
				path->AddArc(rect,(REAL)iS1,(REAL)(iS2-iS1));
				if(iClose)
					path->CloseFigure();
			}
		}
		PathAddRect(pathOutline,rect);
		break;
	case prtLines:
		switch(iLineType) {
		case 0:
			fS1=(PI*2)*(float)iStart/360;
			fS2=(PI*2)*(float)iStop/360;
			ptfCenter=PointF((ptfOrg[0].X+ptfOrg[1].X)*0.5f,(ptfOrg[0].Y+ptfOrg[1].Y)*0.5f);
			pt0=PointF(0,ptfOrg[0].Y-ptfCenter.Y);
			pt.X=0,pt.Y=-1;
			dSsin=sin(rTwist*2*PI/360);
			dScos=cos(rTwist*2*PI/360);
			for(i=0;i<=iVertex;++i) {
				if(i<iVertex || fS2-fS1<PI*2) {
					rA=fS1+(fS2-fS1)*(float)i/iVertex;
					pt=PointF(sin(rA),-cos(rA));
					pt2=PointF(pt.X*dScos-pt.Y*dSsin,pt.X*dSsin+pt.Y*dScos);
					pt=PointF(pt.X*rect.Width*.5f,pt.Y*rect.Height*.5f);
					pt2=PointF(pt2.X*iStarDepth*.005f*rect.Width,pt2.Y*iStarDepth*.005f*rect.Height);
					path->StartFigure();
					path->AddLine(pt+ptfCenter,pt2+ptfCenter);
				}
			}
			break;
		case 1:
			ptfCenter=PointF((ptfOrg[0].X+ptfOrg[1].X)*0.5f,(ptfOrg[0].Y+ptfOrg[1].Y)*0.5f);
			rX=ptfOrg[1].X-ptfOrg[0].X;
			rY=ptfOrg[1].Y-ptfOrg[0].Y;
			rX=sqrtf(rX*rX+rY*rY)*.005f*(100-iStarDepth);
			mx.Reset();
			mx.RotateAt(rTwist,ptfCenter);
			for(i=iVertex*iStart/200;i<iVertex*iStop/200;++i) {
				rA=(ptfOrg[1].Y-ptfOrg[0].Y)*(double)(i+iVertex/2)/(iVertex-1);
				pt=PointF(ptfCenter.X-rX,ptfOrg[0].Y+rA);
				pt2=PointF(ptfCenter.X+rX,ptfOrg[0].Y+rA);
				mx.TransformPoints(&pt);
				mx.TransformPoints(&pt2);
				if(ClipLine(&pt,&pt2,&rect)) {
					path->StartFigure();
					path->AddLine(pt,pt2);
				}
			}
			break;
		case 2:
			ptfCenter=PointF((ptfOrg[0].X+ptfOrg[1].X)*0.5f,(ptfOrg[0].Y+ptfOrg[1].Y)*0.5f);
			rX=ptfOrg[1].X-ptfOrg[0].X;
			rY=ptfOrg[1].Y-ptfOrg[0].Y;
			rX=sqrtf(rX*rX+rY*rY)*.005f*(100-iStarDepth);
			mx.Reset();
			mx.RotateAt(rTwist,ptfCenter);
			for(i=iVertex*iStart/200;i<iVertex*iStop/200;++i) {
				rA=(ptfOrg[1].X-ptfOrg[0].X)*(double)(i+iVertex/2)/(iVertex-1);
				pt=PointF(ptfOrg[0].X+rA,ptfCenter.Y-rX);
				pt2=PointF(ptfOrg[0].X+rA,ptfCenter.Y+rX);
				mx.TransformPoints(&pt);
				mx.TransformPoints(&pt2);
				if(ClipLine(&pt,&pt2,&rect)) {
					path->StartFigure();
					path->AddLine(pt,pt2);
				}
			}
			break;
		}
		PathAddRect(pathOutline,rect);
		break;
	case prtPolygon:
		ptfCenter=PointF((ptfOrg[0].X+ptfOrg[1].X)*0.5f,(ptfOrg[0].Y+ptfOrg[1].Y)*0.5f);
		pt0=PointF(0,ptfOrg[0].Y-ptfCenter.Y);
		pt.X=0,pt.Y=-1;
		ptfTab=new PointF[iVertex*2+1];
		ptfTab[0]=pt0+ptfCenter;
		fSsin=sinf(rTwist*2*PI/360);
		fScos=cosf(rTwist*2*PI/360);
		for(i=1;i<=iVertex;++i) {
			rA=PI*2*i/iVertex;
			pt2=PointF(sinf((float)rA),-cosf((float)rA));
			pt1=PointF((pt.X+pt2.X)*iStarDepth*.005f,(pt.Y+pt2.Y)*iStarDepth*.005f);
			pt1=PointF(pt1.X*fScos-pt1.Y*fSsin,pt1.X*fSsin+pt1.Y*fScos);
			pt=pt2;
			ptfTab[i*2-1]=PointF(pt1.X*rect.Width*.5f,pt1.Y*rect.Height*.5f)+ptfCenter;
			ptfTab[i*2]=PointF(pt2.X*rect.Width*.5f,pt2.Y*rect.Height*.5f)+ptfCenter;
		}
		if(rTension)
			path->AddClosedCurve(ptfTab,iVertex*2,rTension*.1f);
		else {
			path->AddLines(ptfTab,iVertex*2);
			path->CloseFigure();
		}
		delete[] ptfTab;
		PathAddRect(pathOutline,rect);
		break;
	case prtImage:
		PathAddRect(path,rect);
		PathAddRect(pathOutline,rect);
		break;
	case prtText:
		fontf=new FontFamily(strFont);
		wcscpy(wText,strText);
		style=0;
		if(iBold)
			style|=FontStyleBold;
		if(iItalic)
			style|=FontStyleItalic;
		float fEmSize;
		if(iFontSizeUnit==0)
			fEmSize=fFontSize;
		else
			fEmSize=fFontSize*96/72;
		font=new Font(fontf,max(0.01,abs(fEmSize)),style,UnitPixel);
		g=new Graphics(theScreen->hwnd);
		g->MeasureString(L"  ",-1,font,PointF(0.f,0.f),&rcf2);
		fXSpace=rcf2.Width;
		g->MeasureString(L"M ",-1,font,PointF(0.f,0.f),&rcfM);
		if(iUseTextPath) {
			PathData pd;
			int iCount;
			PointF ptfV;
			float fCharWidth;
			float fDist,fNextDist,fdx,fdy,fRate,fRot,fDistSum,fTotal;
			Matrix mx;
			PointF ptfTop;
			bz->GetPath(0,iClose,path,&rect);
			path->GetPathData(&pd);
			pathTemp=new GraphicsPath();
			if(pd.Count>1) {
				fDist=fDistSum=0.f;
				iCount=1;
				path->Reset();
				fNextDist=fDist=0.f;
				ptfCur=pd.Points[0];
				if(fFontAspect!=100.f||fFontSize<0) {
					fHorzRatio=fFontAspect*.01f;
					fVertRatio=1.f;
					if(fFontSize<0)
						fVertRatio=-1.f;
				}
				else
					fHorzRatio=fVertRatio=1.f;
				if(fPathOffset) {
					fTotal=0.f;
					for(;;) {
						fdx=pd.Points[iCount].X-ptfCur.X;
						fdy=pd.Points[iCount].Y-ptfCur.Y;
						fNextDist=sqrtf(fdx*fdx+fdy*fdy);
						fTotal+=fNextDist;
						ptfCur=pd.Points[iCount];
						++iCount;
						if(iCount>=pd.Count)
							iCount=0;
						if(iCount==1) {
							fDist=fTotal*fPathOffset*.01f;
							if(fDist<0.f)
								fDist+=fTotal;
							break;
						}
					}
				}
				fOffs=0.f;
				for(str=wText;*str;) {
					pathTemp->Reset();
					if(*str==0x20) {
						fCharWidth=fXSpace*fFontSpacing*.01f*fHorzRatio;
					}
					else {
						strTemp[0]=*str;
						strTemp[1]=0;
						g->MeasureString(strTemp,1,font,PointF(0.f,0.f),&rcf2);
						if(iFix) {
							fCharWidth=(rcfM.Width-fXSpace)*fFontSpacing*.01f*fHorzRatio;
							fOffs=(rcfM.Width-rcf2.Width-fXSpace)*fFontSpacing*.01f*fHorzRatio*.5f;
						}
						else {
							fCharWidth=(rcf2.Width-fXSpace)*fFontSpacing*.01f*fHorzRatio;
						}
						pathTemp->AddString(strTemp,1,fontf,style,max(0.01,abs(fEmSize)),PointF(0,0),StringFormat::GenericDefault());
						mx.Reset();
						mx.Scale(fHorzRatio,fVertRatio);
					}
					fDist+=fCharWidth*.5f+fOffs;
					for(;;) {
						fdx=pd.Points[iCount].X-ptfCur.X;
						fdy=pd.Points[iCount].Y-ptfCur.Y;
						fNextDist=sqrtf(fdx*fdx+fdy*fdy);
						fDistSum+=fNextDist;
						if(fNextDist>=fDist) {
							fRot=atan2f(fdy,fdx)*180/PI;
							if(fNextDist) {
								fRate=fDist/fNextDist;
								ptfCur.X+=fdx*fRate;
								ptfCur.Y+=fdy*fRate;
							}
							fDist=0.f;
							break;
						}
						ptfCur=pd.Points[iCount];
						++iCount;
						if(iCount>=pd.Count) {
							fRot=0.f;
							iCount=0;
							if(fDistSum==0.f)
								break;
						}
						fDist-=fNextDist;
					}
					if(*str!=0x20) {
						RectF rcf;
						ptfV=PointF(-fCharWidth*.5f,-fEmSize*.5f)+ptfCur;
						GetPathBounds(pathTemp,&rcf);
						mx.Translate(ptfV.X,ptfV.Y,MatrixOrderAppend);
						if(iKeepDir==0)
							mx.RotateAt(fRot,ptfCur,MatrixOrderAppend);
						pathTemp->Transform(&mx);
						path->AddPath(pathTemp,FALSE);
					}
					fDist+=fCharWidth*.5f-fOffs;
					++str;
				}
			}
			delete pathTemp;
		}
		else {
			ptfCur=ptfOrg[0];
			fOffs=0.f;
			for(str=wText;*str;) {
				PointF ptf;
				if(*str==0x20) {
					ptfCur.X+=fXSpace+fXSpace*(fFontSpacing-100)*.01f;
					++str;
					continue;
				}
				strTemp[0]=*str;
				strTemp[1]=0;
				g->MeasureString(strTemp,1,font,PointF(0.f,0.f),&rcf2);
				if(iFix) {
					fCWidth=(rcfM.Width-fXSpace)+fXSpace*(fFontSpacing-100)*.01f;
					fOffs=(rcfM.Width-rcf2.Width)*.5f;
				}
				else {
					fCWidth=(rcf2.Width-fXSpace)+fXSpace*(fFontSpacing-100)*.01f;
				}
				ptf=ptfCur;
				ptf.X+=fOffs;
				path->AddString(strTemp,1,fontf,style,max(0.01,abs(fEmSize)),ptf,StringFormat::GenericDefault());
				++str;
				g->MeasureString(strTemp,1,font,PointF(0.f,0.f),&rcf2);
				ptfCur.X+=fCWidth;
			}
		}
		delete g;
		delete font;

		fHorzRatio=1.0f;
		GetPathBounds(path,&rc);
		if(rc.Width==0 && rc.Height==0) {
			rc.X=ptfOrg[0].X;
			rc.Y=ptfOrg[1].Y;
			rc.Width=ptfOrg[1].X-ptfOrg[0].X;
			rc.Height=ptfOrg[1].Y-ptfOrg[0].Y;
		}
		else {
			rc.Width=rc.X+rc.Width-ptfOrg[0].X;
			rc.Height=rc.Y+rc.Height-ptfOrg[0].Y;
			rc.X=ptfOrg[0].X;
			rc.Y=ptfOrg[0].Y;
		}

		if(iAutoSize==0&&iUseTextPath==0)
			iAdjustRect=0;
		switch(iAdjustRect) {
		case 0:		//adjust rect to fontsize,aspect
			if(iUseTextPath) {
				pathTemp=new GraphicsPath();
				if(bz==NULL)
					bz=new Bezier();
				rect2=rect;
				bz->GetPath(0,iClose,pathTemp,&rect);
				delete pathTemp;
				rect.GetLocation(&ptfOrg[0]);
				ptfOrg[1].X=rect.X+rect.Width;
				ptfOrg[1].Y=rect.Y+rect.Height;
			}
			else {
				if(fFontAspect!=100.f||fFontSize<0) {
					float rY;
					fHorzRatio=fFontAspect*.01f;
					rY=1.f;
					if(fFontSize<0)
						rY=-1.f;
					mx.Reset();
					mx.Translate(-rc.X,-rc.Y,MatrixOrderAppend);
					mx.Scale(fHorzRatio,rY,MatrixOrderAppend);
					mx.Translate(rc.X,rc.Y,MatrixOrderAppend);
					path->Transform(&mx);
				}
				GetPathBounds(path,&rc);
				if(fFontAspect<0)
					rc.X+=rc.Width,rc.Width=-rc.Width;
				if(fFontSize<0)
					rc.Y+=rc.Height,rc.Height=-rc.Height;
				rc.Width=rc.X+rc.Width-ptfOrg[0].X;
				rc.Height=rc.Y+rc.Height-ptfOrg[0].Y;
				rc.X=ptfOrg[0].X;
				rc.Y=ptfOrg[0].Y;
				rect=rc;
				ptfOrg[1].X=rc.X+rc.Width;
				ptfOrg[1].Y=rc.Y+rc.Height;
				theProp->SetupPos(this);
			}
			break;
		case 1:	//adjust fontsize,aspect to rect
			if(iUseTextPath) {
				pathTemp=new GraphicsPath();
				if(bz==NULL)
					bz=new Bezier();
				rect2=rect;
				bz->GetPath(1,iClose,pathTemp,&rect);
				delete pathTemp;
				if(rect2.Width<0) {
					rect.X+=rect.Width;
					rect.Width=-rect.Width;
				}
				if(rect2.Height<0) {
					rect.Y+=rect.Height;
					rect.Height=-rect.Height;
				}
				if(rect.Width!=0 && rect.Height!=0) {
					ptfOrgOld[0].X=rect.X;
					ptfOrgOld[0].Y=rect.Y;
					ptfOrgOld[1].X=rect.X+rect.Width;
					ptfOrgOld[1].Y=rect.Y+rect.Height;
					mx.Reset();
					mx.Translate(-ptfOrgOld[0].X,-ptfOrgOld[0].Y,MatrixOrderAppend);
					if(ptfOrgOld[1].X!=ptfOrgOld[0].X && ptfOrgOld[1].Y!=ptfOrgOld[0].Y && ptfOrg[1].X!=ptfOrg[0].X && ptfOrg[1].Y!=ptfOrg[0].Y) {
						dx=(ptfOrg[1].X-ptfOrg[0].X)/(ptfOrgOld[1].X-ptfOrgOld[0].X);
						dy=(ptfOrg[1].Y-ptfOrg[0].Y)/(ptfOrgOld[1].Y-ptfOrgOld[0].Y);
						mx.Scale(dx,dy,MatrixOrderAppend);
					}
					mx.Translate(ptfOrg[0].X,ptfOrg[0].Y,MatrixOrderAppend);
					mx.TransformPoints(bz->ptAnchor,bz->iIndex);
					path->Transform(&mx);
				}
			}
			else {
				dRatio=abs(ptfOrg[1].Y-ptfOrg[0].Y)/rc.Height;
				fFontAspect=(ptfOrg[1].X-ptfOrg[0].X)/(rc.Width*dRatio)*100.f;
				fFontSize=abs(fFontSize*dRatio);
				if(ptfOrg[0].Y>ptfOrg[1].Y)
					fFontSize=-fFontSize;
				mx.Reset();
				mx.Translate(-rc.X,-rc.Y,MatrixOrderAppend);
				mx.Scale(rect.Width/rc.Width,rect.Height/rc.Height,MatrixOrderAppend);
				mx.Translate(rc.X,rc.Y,MatrixOrderAppend);
				path->Transform(&mx);
			}
			break;
		}
		if(rect.Height==0)
			rect.Height=0.01;
		PathAddRect(pathOutline,rect);
		path->Flatten();
		path->SetFillMode(FillModeWinding);
		delete fontf;
		break;
	case prtShape:
		bz->GetPath(0,iClose,path,&rcOutline);
		PathAddRect(pathOutline,rect);
		break;
	}
	mxRot->Reset();
	if(fSlant!=0.f || rAng!=0.f) {
		mxRot->Translate(-ptfCenter.X,-ptfCenter.Y);
		mxRot->Shear(-fSlant*0.01f,0,MatrixOrderAppend);
		mxRot->Rotate(rAng,MatrixOrderAppend);
		mxRot->Translate(ptfCenter.X,ptfCenter.Y,MatrixOrderAppend);
		path->Transform(mxRot);
		pathOutline->Transform(mxRot);
	}
	GetPathBounds(path,&rcOutline);
	if(this->prType==prtGroup) {
		rcOutline.Width=rcOutline.Height=0;
		if(hitem) {
			for(pChild=theTree->GetChild(this);pChild;pChild=theTree->GetNextSibling(pChild)) {
				rcOutline=Union(&rcOutline,&pChild->rcOutline);
			}
		}
		ptfOrg[0].X=rcOutline.X;
		ptfOrg[0].Y=rcOutline.Y;
		ptfOrg[1].X=rcOutline.X+rcOutline.Width;
		ptfOrg[1].Y=rcOutline.Y+rcOutline.Height;
		ptRect[0]=ptfOrg[0];
		ptRect[1].X=ptfOrg[1].X;
		ptRect[1].Y=ptfOrg[0].Y;
		ptRect[2]=ptfOrg[1];
		ptRect[3].X=ptfOrg[0].X;
		ptRect[3].Y=ptfOrg[1].Y;
		PathAddRect(pathOutline,rcOutline);
	}
	else {
		pathOutline->GetPathPoints(ptRect,4);
		if(ptfOrg[0].X>ptfOrg[1].X) {
			PTSwap(&ptRect[0],&ptRect[1]);
			PTSwap(&ptRect[2],&ptRect[3]);
		}
		if(ptfOrg[0].Y>ptfOrg[1].Y) {
			PTSwap(&ptRect[0],&ptRect[3]);
			PTSwap(&ptRect[1],&ptRect[2]);
		}
		if(hitem) {
			for(pChild=theTree->GetChild(this);pChild;pChild=theTree->GetNextSibling(pChild)) {
				if(pChild->iOperation==1) {
					rcOutline.Union(rcOutline,rcOutline,pChild->rcOutline);
				}
			}
		}
	}
	iBmpOrg=max(abs(iShadowOffset),abs(iIShadowOffset))+min(32,rShadowDiffuse*3)+(fWidth+1);
	rcBmp=rcOutline;
//	rcBmp.Inflate((REAL)iBmpOrg+.5f,(REAL)iBmpOrg+.5f);
	rcBmp.Inflate((REAL)iBmpOrg,(REAL)iBmpOrg);
	iBmpOffX=rcBmp.X;
	iBmpOffY=rcBmp.Y;
	mxShift->Reset();
	mxShift->Translate(-iBmpOffX,-iBmpOffY);
	path->Transform(mxShift);
	pathOutline->Transform(mxShift);
	iBmpBaseDirty=iBmpDirty=iBmpPrevDirty=iDirty;
	if(iOperation!=0) {
		Primitive *p=theTree->GetParent(this);
		if(p) {
			p->iBmpBaseDirty=p->iBmpDirty=p->iBmpPrevDirty=2;
			p->CalcPath(2,1);
		}
	}
}
void Primitive::AdjustBezier(void) {
	int i;
	RectF rc;
	if(prType!=prtShape)
		return;
	for(i=0;i<bz->iIndex;++i) {
	}
}
void Primitive::SetPos(float x,float y,float w,float h,int iAdjustRect) {
	ptfOrg[0].X=x;
	ptfOrg[0].Y=y;
	ptfOrg[1].X=x+w;
	ptfOrg[1].Y=y+h;
	if(prType==prtShape||prType==prtText) {
		RectF rcf(x,y,w,h);
		bz->GetPath(1,iClose,path,&rcf);
	}
	CalcPath(2,1);
	CalcParent();
	theTree->prlCurrent.RecalcRect();
}
int Primitive::SetPos(int iDrag,float x,float y) {
	Matrix mx;
	REAL rX;
	PointF ptf(x,y);
	switch(iDrag) {
	case 4:
		mx.RotateAt(-rAngle,PointF((x+ptRect[2].X)/2,(y+ptRect[2].Y)/2));
		ptfOrg[0]=ptf;
		ptfOrg[1]=ptRect[2];
		mx.TransformPoints(ptfOrg,2);
		break;
	case 5:
		mx.RotateAt(-rAngle,PointF((x+ptRect[3].X)/2,(y+ptRect[3].Y)/2));
		ptfOrg[0]=ptf;
		ptfOrg[1]=ptRect[3];
		mx.TransformPoints(ptfOrg,2);
		rX=ptfOrg[0].X;
		ptfOrg[0].X=ptfOrg[1].X;
		ptfOrg[1].X=rX;
		break;
	case 7:
		mx.RotateAt(-rAngle,PointF((x+ptRect[0].X)/2,(y+ptRect[0].Y)/2));
		ptfOrg[1]=ptf;
		ptfOrg[0]=ptRect[0];
		mx.TransformPoints(&ptfOrg[0],2);
		break;
	case 6:
		mx.RotateAt(-rAngle,PointF((x+ptRect[1].X)/2,(y+ptRect[1].Y)/2));
		ptfOrg[0]=ptf;
		ptfOrg[1]=ptRect[1];
		mx.TransformPoints(ptfOrg,2);
		rX=ptfOrg[0].Y;
		ptfOrg[0].Y=ptfOrg[1].Y;
		ptfOrg[1].Y=rX;
		break;
	}
	if(ptfOrg[0].X==ptfOrg[1].X)
		ptfOrg[1].X+=0.01f;
	if(ptfOrg[0].Y==ptfOrg[1].Y)
		ptfOrg[1].Y+=0.01f;
	CalcPath(2,1);
	CalcParent();
	theTree->prlCurrent.RecalcRect();
	return iDrag;
}
LRESULT CALLBACK Tree::wndprocSubclassTree(HWND hwnd,UINT msg,WPARAM wparam,LPARAM lparam) {
	Tree *tree=(Tree*)(LONG_PTR)GetWindowLongPtr(hwnd,GWLP_USERDATA);
	RECT rc1;
	Primitive *pr;
	static NMTREEVIEW nm;
	static TVHITTESTINFO tvhi;
	static RECT rcDrag,rc2;
	static int iSameItemPress=0;
	switch(msg) {
	case WM_MOUSEWHEEL:
		SendMessage(theScreen->hwnd,WM_MOUSEWHEEL,wparam,lparam);
		InvalidateRect(tree->hwndVisible,NULL,false);
		break;
	case WM_VSCROLL:
		InvalidateRect(tree->hwndVisible,NULL,false);
		break;
	case WM_LBUTTONDOWN:
		DebugMessage1(L"LBUTTONDOWN\n");
		GetXY(lparam,(int*)&rcDrag.left,(int*)&rcDrag.top);
		rc2.left=rc2.right=tvhi.pt.x=rcDrag.left;
		rc2.top=rc2.bottom=tvhi.pt.y=rcDrag.top;
		TreeView_HitTest(hwnd,&tvhi);
		if(tvhi.hItem==0||tvhi.flags==TVHT_ONITEMRIGHT) {
			tree->bAreaSelecting=true;
			::InvertRect(hwnd,rc2.left,rc2.top,rc2.right,rc2.bottom);
			tree->hitemSelOld=TreeView_GetSelection(hwnd);
			SetCapture(hwnd);
		}
		else {
			memset(&nm,0,sizeof(nm));
			tree->hitemSelOld=TreeView_GetSelection(hwnd);
			tree->bAreaSelecting=false;
			nm.itemOld.hItem=tree->hitemSelOld;
			nm.itemOld.lParam=(LPARAM)tree->GetPrimitive(tree->hitemSelOld);
			nm.itemNew.hItem=tvhi.hItem;
			nm.itemNew.lParam=(LPARAM)tree->GetPrimitive(tvhi.hItem);
			nm.hdr.code=TVN_SELCHANGED;
			if(nm.itemOld.hItem==nm.itemNew.hItem)
				iSameItemPress=1;
			else
				iSameItemPress=0;
		}
		break;
	case WM_MOUSEMOVE:
		DebugMessage1(L"MouseMove\n");
		if(tree->bAreaSelecting) {
			::InvertRect(hwnd,rc2.left,rc2.top,rc2.right,rc2.bottom);
			GetXY(lparam,(int*)&rcDrag.right,(int*)&rcDrag.bottom);
			rc2.top=min(rcDrag.top,rcDrag.bottom);
			rc2.left=min(rcDrag.left,rcDrag.right);
			rc2.right=max(rcDrag.left,rcDrag.right);
			rc2.bottom=max(rcDrag.top,rcDrag.bottom);
			::InvertRect(hwnd,rc2.left,rc2.top,rc2.right,rc2.bottom);
		}
		else {
			if(iSameItemPress) {
				if((wparam&MK_LBUTTON)==0) {
					GetXY(lparam,(int*)&rcDrag.left,(int*)&rcDrag.top);
					tvhi.pt.x=rcDrag.left;
					tvhi.pt.y=rcDrag.top;
					TreeView_HitTest(hwnd,&tvhi);
					if(tvhi.hItem==nm.itemNew.hItem) {
						DebugMessage2(L"SameItem %08x\n",wparam);
						SendMessage(::GetParent(hwnd),WM_NOTIFY,(WPARAM)'TREE',(LPARAM)&nm);
					}
				}
			}
		}
		iSameItemPress=0;
		break;
	case WM_LBUTTONUP:
		DebugMessage1(L"LBUTTONUP\n");
		if(tree->bAreaSelecting) {
			bool bTreeSel=false;
			Primitive *prFirst=0;
			::InvertRect(hwnd,rc2.left,rc2.top,rc2.right,rc2.bottom);
			GetXY(lparam,(int*)&rcDrag.right,(int*)&rcDrag.bottom);
			rc2.top=min(rcDrag.top,rcDrag.bottom);
			rc2.left=min(rcDrag.left,rcDrag.right);
			rc2.right=max(rcDrag.left,rcDrag.right);
			rc2.bottom=max(rcDrag.top,rcDrag.bottom);
			if((GetAsyncKeyState(VK_SHIFT)&0x8000)==0)
				tree->SelectClear();
			for(pr=tree->GetFirst();pr;pr=tree->GetNext(pr)) {
				TreeView_GetItemRect(hwnd,pr->hitem,&rc1,TRUE);
				if(rc1.right<rc2.left || rc1.left>rc2.right || rc1.top>rc2.bottom || rc1.bottom<rc2.top)
					continue;
				tree->SelectAdd(pr,0);
				if(prFirst==0)
					prFirst=pr;
				if(pr==theProp->prCurrent)
					bTreeSel=true;
			}
			if(bTreeSel==false && prFirst)
				tree->SelectAdd(prFirst,1);
			tree->bAreaSelecting=false;
			ReleaseCapture();
		}
		iSameItemPress=0;
		break;
	case WM_KEYDOWN:
		if(theApp->iCursorMode) {
			switch(wparam) {
			case VK_SHIFT:
			case VK_UP:
			case VK_DOWN:
			case VK_LEFT:
			case VK_RIGHT:
				SendMessage(theScreen->hwnd,WM_KEYDOWN,wparam,lparam);
				return 0;
			}
		}
		break;
	case WM_KEYUP:
		if(theApp->iCursorMode) {
			switch(wparam) {
			case VK_SHIFT:
			case VK_UP:
			case VK_DOWN:
			case VK_LEFT:
			case VK_RIGHT:
				SendMessage(theScreen->hwnd,WM_KEYUP,wparam,lparam);
				return 0;
			}
		}
		if(wparam==VK_CONTROL) {
			switch(theTool->toolCurrent) {
			case tArrow:
				SetCursor(hcurArrow);
				break;
			}
		}
		break;
	}
	return (CallWindowProc(tree->wndprocOrg, hwnd, msg, wparam, lparam));
}
LRESULT CALLBACK Tree::dlgprocTreeFrame(HWND hdlg, UINT message, WPARAM wparam, LPARAM lparam) {
	NMTREEVIEW *pnm;
	NMTVKEYDOWN *pnmk;
	Primitive *prNew,*prOld,*pr,*pr1,*pr2,*prt;
	PrimitivePtrList *pl;
	int k,x,y;
	HMENU hmenu;
	Tree *tree=(Tree*)(LONG_PTR)GetWindowLongPtr(hdlg,GWLP_USERDATA);
	switch (message) {
	case WM_INITDIALOG:
		return TRUE;
	case WM_MOUSEWHEEL:
		SendMessage(theScreen->hwnd,WM_MOUSEWHEEL,wparam,lparam);
		break;
	case WM_SIZE:
		x=LOWORD(lparam);
		y=HIWORD(lparam);
		if(theTree)
			theTree->Size(x,y);
		break;
	case WM_NOTIFY:
		pnm=(NMTREEVIEW*)lparam;
		pnmk=(NMTVKEYDOWN*)lparam;
		if((int)wparam=='TREE') {
			switch(pnm->hdr.code) {
			case TVN_SELCHANGED:
				DebugMessage1(L"TVN_SELCHANGED\n");
				if(theTree->iDisableResponse==0) {
					DebugMessage1(L"Change\n");
					prNew=(Primitive*)pnm->itemNew.lParam;
					prOld=(Primitive*)pnm->itemOld.lParam;
					if((GetAsyncKeyState(VK_CONTROL)&0x8000)!=0) {
						if(theTree->IsSelected(prNew,true)) {
							theTree->SelectDel(prNew);
							for(pl=&theTree->prlCurrent;pl;pl=pl->next) {
								if(pl->next&&pl->bSel==true) {
									theTree->SelectAdd(pl->pr);
									break;
								}
							}
						}
						else
							theTree->SelectAdd(prNew);
					}
					else if(prOld && (GetAsyncKeyState(VK_SHIFT)&0x8000)!=0) {
						pr1=prNew;
						pr2=prOld;
						if(pr1&&pr2) {
							theTree->Renumber();
							if(pr1->iNum>pr2->iNum) {
								prt=pr1;
								pr1=pr2;
								pr2=prt;
							}
							for(;pr1!=pr2;pr1=theTree->GetNext(pr1)) {
								theTree->SelectAdd(pr1,0);
							}
							theTree->SelectAdd(prNew,1);
						}
					}
					else if(theTree->IsSelected(prNew,true)==0)
						theTree->Select1(prNew);
					else
						theTree->SelectAdd(prNew,1);
					InvalidateRect(hdlg,NULL,false);
				}
				break;
			case TVN_ITEMEXPANDED:
				InvalidateRect(hdlg,NULL,false);
				break;
			case TVN_BEGINDRAG:
				theTree->BeginDrag(pnm);
				SetCapture(hdlg); 
				break;
			case TVN_KEYDOWN:
				Shortcut(pnmk->wVKey,0);
				k=MapVirtualKey(pnmk->wVKey,2);
				Shortcut(k,1);
				return 1;	//avoid incremental search
			case NM_RCLICK:
				POINT pt;
			    TVHITTESTINFO tvht;
				HTREEITEM item;
				GetCursorPos(&pt);
				tvht.pt=pt;
				ScreenToClient(theTree->hwnd,&tvht.pt);
		        item=TreeView_HitTest(theTree->hwnd, &tvht);
				if(item) {
					Primitive *p=theTree->GetPrimitive(item);
					if(p && theTree->IsSelected(p,true)==false)
						theTree->Select1(p);
				}
				hmenu=CreatePopupMenu();
				pr=theTree->prlCurrent.pr;
				if(pr)
					RClickMenu(theTree->hwndFrame,pt,pr);
				break;
			}
		}
		break;
	case WM_MOUSEMOVE:
		theTree->MouseMove(LOWORD(lparam),HIWORD(lparam));
		break;
	case WM_LBUTTONUP:
		if(GetCapture()==hdlg) {
			theTree->LButtonUp();
			ReleaseCapture();
		}
		break;
	case WM_CLOSE:
		theTree->Show(0);
		theApp->CheckMenu();
		break;
	}
	return FALSE;
}
LRESULT CALLBACK Tree::wndprocVisible(HWND hwnd,UINT message,WPARAM wparam,LPARAM lparam) {
	HDC hdc;
	PAINTSTRUCT ps;
	CREATESTRUCT *lp;
	RECT rc;
	int x,y;
	Primitive *p;
	Tree *tree=(Tree*)(LONG_PTR)GetWindowLongPtr(hwnd,GWLP_USERDATA);
	switch(message) {
	case WM_CREATE:
		lp=(CREATESTRUCT*)lparam;
		SetWindowLong(hwnd,GWL_USERDATA,(LONG)lp->lpCreateParams);
		break;
	case WM_PAINT:
		hdc = BeginPaint(hwnd, &ps);
		for(p=theTree->GetFirst();p;p=theTree->GetNext(p)) {
			if(TreeView_GetItemRect(theTree->hwnd,p->hitem,&rc,true)) {
				DrawIconEx(hdc,0,rc.top,(p->iVisible&1)?tree->hiconVisible1:tree->hiconVisible0,16,rc.bottom-rc.top,0,0,DI_NORMAL);
				DrawIconEx(hdc,16,rc.top,(p->iVisible&2)?tree->hiconVisibleSolo1:tree->hiconVisibleSolo0,16,rc.bottom-rc.top,0,0,DI_NORMAL);
				DrawIconEx(hdc,32,rc.top,(p->IsLock()&1)?tree->hiconLock1:tree->hiconLock0,16,rc.bottom-rc.top,0,0,DI_NORMAL);
			}
		}
		EndPaint(hwnd, &ps);
		break;
	case WM_LBUTTONDOWN:
		y=HIWORD(lparam);
		x=LOWORD(lparam);
		for(p=theTree->GetFirst();p;p=theTree->GetNext(p)) {
			TreeView_GetItemRect(theTree->hwnd,p->hitem,&rc,true);
			if(y>=rc.top&&y<rc.bottom)
				break;
		}
		if(p) {
			if(x>=32)
				p->Lock((p->IsLock()&1)^1);
			else if(x>=16)
				p->iVisible^=2;
			else
				p->iVisible^=1;
			InvalidateRect(hwnd,NULL,true);
			theApp->Edit();
			theScreen->Send();
		}
		break;
	default:
		return DefWindowProc(hwnd, message, wparam, lparam);
	}
	return 0;
}
Tree::Tree(HWND hwndParent) {
	HIMAGELIST himl;
	HICON hicon;
	prCurrent=NULL;
	prRoot=NULL;
	iDisableResponse=0;
	hitemSelOld=0;
	bFloat=true;
	bDragging=bAreaSelecting=false;
	theApp->Edit(false);
	hwndFrame=CreateDialog(hinstMain,(LPCTSTR)IDD_TREE,hwndParent,(DLGPROC)dlgprocTreeFrame);
	hiconVisible0=LoadIcon(hinstMain,MAKEINTRESOURCE(IDI_VISIBLE0));
	hiconVisible1=LoadIcon(hinstMain,MAKEINTRESOURCE(IDI_VISIBLE1));
	hiconVisibleSolo0=LoadIcon(hinstMain,MAKEINTRESOURCE(IDI_VISIBLESOLO0));
	hiconVisibleSolo1=LoadIcon(hinstMain,MAKEINTRESOURCE(IDI_VISIBLESOLO1));
	hiconLock0=LoadIcon(hinstMain,MAKEINTRESOURCE(IDI_LOCK0));
	hiconLock1=LoadIcon(hinstMain,MAKEINTRESOURCE(IDI_LOCK1));
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX); 
	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)wndprocVisible;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hinstMain;
	wcex.hIcon			= NULL;
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= L"Visible";
	wcex.hIconSm		= LoadIcon(wcex.hInstance, (LPCTSTR)IDI_SKINMAN);
	RegisterClassEx(&wcex);
	hwndVisible=CreateWindowEx(WS_EX_CLIENTEDGE,L"Visible",L"",WS_CHILD|WS_VISIBLE,0,0,16,100,hwndFrame,NULL,hinstMain,this);
	hwnd=CreateWindowEx(WS_EX_CLIENTEDGE,WC_TREEVIEW,L"Tree",WS_CHILD|WS_VISIBLE|WS_VSCROLL|TVS_HASLINES|TVS_HASBUTTONS|TVS_LINESATROOT|TVS_SHOWSELALWAYS,0,0,0,0,hwndFrame,NULL,hinstMain,0);
	SetWindowLong(hwnd,GWL_ID,'TREE');
	SetWindowLongPtr(hwnd,GWLP_USERDATA,(LONG)(LONG_PTR)this);
	wndprocOrg=(WNDPROC)(LONG_PTR)SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG)(LONG_PTR)wndprocSubclassTree);
	if((himl = ImageList_Create(16, 16, FALSE, 3, 0)) == NULL)
		return; 
	hicon=LoadIcon(hinstMain, MAKEINTRESOURCE(IDI_ITEMRECT)); 
	iImageItem = ImageList_AddIcon(himl, hicon); 
	DeleteObject(hicon); 
	hicon=LoadIcon(hinstMain, MAKEINTRESOURCE(IDI_ITEMELLIPSE)); 
	iImageItem = ImageList_AddIcon(himl, hicon); 
	DeleteObject(hicon); 
	hicon=LoadIcon(hinstMain, MAKEINTRESOURCE(IDI_ITEMPOLYGON)); 
	iImageItem = ImageList_AddIcon(himl, hicon); 
	DeleteObject(hicon); 
	hicon=LoadIcon(hinstMain, MAKEINTRESOURCE(IDI_ITEMTEXT)); 
	iImageText = ImageList_AddIcon(himl, hicon); 
	DeleteObject(hicon); 
	hicon=LoadIcon(hinstMain, MAKEINTRESOURCE(IDI_ITEMSHAPE)); 
	iImageText = ImageList_AddIcon(himl, hicon);
	DeleteObject(hicon); 
	hicon=LoadIcon(hinstMain, MAKEINTRESOURCE(IDI_ITEMLINES));
	iImageText= ImageList_AddIcon(himl, hicon);
	DeleteObject(hicon);
	hicon=LoadIcon(hinstMain, MAKEINTRESOURCE(IDI_ITEMIMAGE)); 
	iImageText = ImageList_AddIcon(himl, hicon); 
	DeleteObject(hicon); 
	hicon=LoadIcon(hinstMain, MAKEINTRESOURCE(IDI_ITEMGROUP));
	iImageText = ImageList_AddIcon(himl, hicon);
	DeleteObject(hicon);
	hicon=LoadIcon(hinstMain, MAKEINTRESOURCE(IDI_ITEMRECT_H)); 
	ImageList_AddIcon(himl, hicon); 
	DeleteObject(hicon); 
	hicon=LoadIcon(hinstMain, MAKEINTRESOURCE(IDI_ITEMELLIPSE_H)); 
	iImageItem = ImageList_AddIcon(himl, hicon); 
	DeleteObject(hicon); 
	hicon=LoadIcon(hinstMain, MAKEINTRESOURCE(IDI_ITEMPOLYGON_H)); 
	iImageItem = ImageList_AddIcon(himl, hicon); 
	DeleteObject(hicon); 
	hicon=LoadIcon(hinstMain, MAKEINTRESOURCE(IDI_ITEMTEXT_H)); 
	ImageList_AddIcon(himl, hicon); 
	DeleteObject(hicon); 
	hicon=LoadIcon(hinstMain, MAKEINTRESOURCE(IDI_ITEMSHAPE_H)); 
	iImageText = ImageList_AddIcon(himl, hicon); 
	DeleteObject(hicon); 
	hicon=LoadIcon(hinstMain, MAKEINTRESOURCE(IDI_ITEMLINES_H));
	iImageText= ImageList_AddIcon(himl, hicon);
	DeleteObject(hicon);
	hicon=LoadIcon(hinstMain, MAKEINTRESOURCE(IDI_ITEMIMAGE_H)); 
	iImageText = ImageList_AddIcon(himl, hicon); 
	DeleteObject(hicon); 
	hicon=LoadIcon(hinstMain, MAKEINTRESOURCE(IDI_ITEMGROUP_H));
	iImageText = ImageList_AddIcon(himl, hicon);
	DeleteObject(hicon);
	hicon=LoadIcon(hinstMain, MAKEINTRESOURCE(IDI_ITEMRECT_S)); 
	ImageList_AddIcon(himl, hicon); 
	DeleteObject(hicon); 
	hicon=LoadIcon(hinstMain, MAKEINTRESOURCE(IDI_ITEMELLIPSE_S)); 
	iImageItem = ImageList_AddIcon(himl, hicon); 
	DeleteObject(hicon); 
	hicon=LoadIcon(hinstMain, MAKEINTRESOURCE(IDI_ITEMPOLYGON_S)); 
	iImageItem = ImageList_AddIcon(himl, hicon); 
	DeleteObject(hicon); 
	hicon=LoadIcon(hinstMain, MAKEINTRESOURCE(IDI_ITEMTEXT_S)); 
	ImageList_AddIcon(himl, hicon); 
	DeleteObject(hicon); 
	hicon=LoadIcon(hinstMain, MAKEINTRESOURCE(IDI_ITEMSHAPE_S)); 
	iImageText = ImageList_AddIcon(himl, hicon); 
	DeleteObject(hicon); 
	hicon=LoadIcon(hinstMain, MAKEINTRESOURCE(IDI_ITEMLINES_S));
	iImageText= ImageList_AddIcon(himl, hicon);
	DeleteObject(hicon);
	hicon=LoadIcon(hinstMain, MAKEINTRESOURCE(IDI_ITEMIMAGE_S)); 
	iImageText = ImageList_AddIcon(himl, hicon); 
	DeleteObject(hicon); 
	hicon=LoadIcon(hinstMain, MAKEINTRESOURCE(IDI_ITEMGROUP_S));
	iImageText = ImageList_AddIcon(himl, hicon);
	DeleteObject(hicon);
	TreeView_SetImageList(hwnd, himl, TVSIL_NORMAL); 
}
Tree::~Tree(void) {
	SetWindowLongPtr(hwnd,GWLP_WNDPROC,(LONG)(LONG_PTR)wndprocOrg);
	DestroyWindow(hwndFrame);
}
void Tree::Show(int i) {
	if(i)
		ShowWindow(hwndFrame,SW_NORMAL);
	else
		ShowWindow(hwndFrame,SW_HIDE);
}
int Tree::IsShow(void) {
	return IsWindowVisible(hwndFrame);
}
void Tree::Pos(int x,int y) {
	SetWindowPos(hwndVisible,0,x,y,0,0,SWP_NOZORDER|SWP_NOSIZE);
	SetWindowPos(hwnd,0,x+52,y,0,0,SWP_NOZORDER|SWP_NOSIZE);
}
void Tree::Size(int x,int y) {
	SetWindowPos(hwndVisible,0,0,0,52,y,SWP_NOZORDER|SWP_NOMOVE);
	SetWindowPos(hwnd,0,0,0,x-52,y,SWP_NOZORDER|SWP_NOMOVE);
}
void Tree::SetName(Primitive *p) {
	wchar_t str[1024];
	TVITEM tvitem;
	switch(p->prType) {
	case prtText:
		swprintf(str,L"%s [%s]",p->strName,p->strText);
		break;
	case prtImage:
		swprintf(str,L"%s [%s]",p->strName,EmbedNameToDispName(p->strFile));
		break;
	default:
		swprintf(str,L"%s",p->strName);
		break;
	}
	ZeroMemory(&tvitem,sizeof(TVITEM));
	tvitem.hItem=p->hitem;
	tvitem.mask=TVIF_TEXT;
	tvitem.pszText=str;
	TreeView_SetItem(hwnd,&tvitem);
}
Primitive *Tree::AddItem(Primitive *prParent,Primitive *prAfter,wchar_t *wstrFile,int x,int y) {
	Primitive *pr;
	wchar_t str[MAX_PATH];
	wchar_t strNew[MAX_PATH];
	wchar_t *strName,*strName2;
	Image *img;
	pr=NULL;
	if(!IsEmbed(wstrFile)) {
		wcscpy(strNew,FileNameToEmbedName(wstrFile));
		if(PathFileExists(wstrFile))
			wcscpy(str,wstrFile);
		else {
			wcscpy(str,theApp->strCurrentProject);
			strName=PathFindFileName(str);
			strName2=PathFindFileName(wstrFile);
			if(strName)
				wcscpy(strName,strName2);
		}
		CopyFile(str,EmbedNameToTempName(strNew),TRUE);
	}
	else
		wcscpy(strNew,wstrFile);
	img=ImageFromFile(EmbedNameToTempName(wstrFile));
	if(img) {
		POINT pt=GetCenterPos();
		if(x!=-1) {
			pt.x=x;
			pt.y=y;
		}
		pr=AddItem(prParent,prAfter,L"Image",prtImage,pt.x,pt.y,pt.x+img->GetWidth(),pt.y+img->GetHeight());
		wcscpy(pr->strFile,strNew);
		pr->prType=prtImage;
		pr->img=img;
		pr->CalcPath(2,1);
		SetName(pr);
		if(theScreen)
			theScreen->Send();
		theApp->Edit();
	}
	return pr;
}
void Tree::ChangeIcon(Primitive *pr) {
	TVITEM tvitem;
	ZeroMemory(&tvitem,sizeof(tvitem));
	switch(pr->prType) {
	case prtRect:
		tvitem.iImage=tvitem.iSelectedImage=0;
		break;
	case prtEllipse:
		tvitem.iImage=tvitem.iSelectedImage=1;
		break;
	case prtPolygon:
		tvitem.iImage=tvitem.iSelectedImage=2;
		break;
	case prtText:
		tvitem.iImage=tvitem.iSelectedImage=3;
		break;
	case prtShape:
		tvitem.iImage=tvitem.iSelectedImage=4;
		break;
	case prtLines:
		tvitem.iImage=tvitem.iSelectedImage=5;
		break;
	case prtImage:
		tvitem.iImage=tvitem.iSelectedImage=6;
		break;
	case prtGroup:
		tvitem.iImage=tvitem.iSelectedImage=7;
		break;
	default:
		tvitem.iImage=tvitem.iSelectedImage=0;
		break;
	}
	tvitem.mask=TVIF_IMAGE|TVIF_SELECTEDIMAGE;
	tvitem.hItem=pr->hitem;
	TreeView_SetItem(hwnd,&tvitem);
}
Primitive *Tree::AddItem(Primitive *prParent,Primitive *prAfter,wchar_t *str,prim pr,float x1,float y1,float x2,float y2) {
	TV_INSERTSTRUCT tvi;
	Primitive *ppr;
	ZeroMemory(&tvi,sizeof(tvi));
	if(prParent==NULL)
		tvi.hParent=TVI_ROOT;
	else
		tvi.hParent=prParent->hitem;
	if(prAfter==NULL)
		tvi.hInsertAfter=TVI_LAST;
	else if(prAfter==(void*)-1)
		tvi.hInsertAfter=TVI_FIRST;
	else
		tvi.hInsertAfter=prAfter->hitem;
	tvi.item.mask=TVIF_TEXT|TVIF_PARAM|TVIF_IMAGE|TVIF_SELECTEDIMAGE;
	tvi.item.pszText=str;
	tvi.item.iImage=1;
	tvi.item.lParam=(LPARAM)(ppr=new Primitive(pr,x1,y1,x2,y2));
	wcscpy(ppr->strName,str);
	ppr->hitem=TreeView_InsertItem(hwnd,&tvi);
	ChangeIcon(ppr);
	Renumber();
	SetName(ppr);
	InvalidateRect(hwndVisible,NULL,true);
//	if(theScreen)
//		theScreen->Send();
	theApp->Edit();
	return ppr;
}
int Tree::GetCount(void) {
	return TreeView_GetCount(hwnd);
}
void Tree::CopyItem(Primitive *p) {
	Primitive *p2=theTree->GetChild(p);
}
void Tree::PasteItem(Primitive *p) {
}
void Tree::Recalc(void) {
	Primitive *pr,*prParent;
	pr=GetLastFromTV();
	for(;pr;pr=GetPrevFromTV(pr)) {
		if(pr->prType==prtGroup)
			pr->CalcPath(2,1);
		if(pr->iOperation) {
			prParent=theTree->GetParent(pr);
			if(prParent)
				prParent->CalcPath(2,1);
		}
	}	
}
void Tree::Renumber(void) {
	Primitive *p,*pPrev;
	int iCount=0;
	p=GetFirstFromTV();
	if(p==NULL) {
		prRoot=NULL;
		return;
	}
	pPrev=0;
	for(;p;p=GetNextFromTV(p)) {
		p->prParent=GetParentFromTV(p);
		if(pPrev)
			pPrev->prNext=p;
		else
			prRoot=p;
		pPrev=p;
		p->iNum=++iCount;
	}
	pPrev->prNext=0;
}
void Tree::DelAll(void) {

	Primitive *pr,*prNext;
	EnterCriticalSection(&csData);
	iDisableResponse++;
	pr=prRoot;
	while(pr) {
		prNext=pr->prNext;
		delete pr;
		pr=prNext;
	}
	TreeView_DeleteAllItems(hwnd);
	Renumber();
	prlCurrent.Clear();
	iDisableResponse--;
	LeaveCriticalSection(&csData);
	InvalidateRect(this->hwndVisible,NULL,TRUE);
}
void Tree::DelItem(Primitive *p,bool bRenum) {
	Primitive *pChild;
	iDisableResponse++;
	while((pChild=GetChildFromTV(p))!=NULL) {
		DelItem(pChild,false);
	}
	if(p) {
		HTREEITEM hitem=p->hitem;
		TreeView_DeleteItem(hwnd,hitem);
		InvalidateRect(hwndVisible,NULL,true);
		theApp->Edit();
		delete p;
	}
	if(bRenum)
		Renumber();
	iDisableResponse--;
}
Primitive *Tree::GetPrimitive(HTREEITEM hitem) {
	TVITEM item;
	if(hitem==NULL)
		return NULL;
	item.mask=TVIF_PARAM;
	item.hItem=hitem;
	TreeView_GetItem(hwnd,&item);
	return (Primitive*)item.lParam;
}
int Tree::GetParentNum(Primitive *p) {
	p=GetParent(p);
	if(p==NULL)
		return 0;
	return p->iNum;
}
Primitive *Tree::GetParent(Primitive *p) {
	if(p==NULL)
		return NULL;
	return p->prParent;
}
Primitive *Tree::GetParentFromTV(Primitive *p) {
	HTREEITEM hitem;
	if(p==NULL)
		return NULL;
	hitem=TreeView_GetParent(hwnd,p->hitem);
	return GetPrimitive(hitem);
}
Primitive *Tree::GetChildFromTV(Primitive *p) {
	HTREEITEM hitem;
	if(p==NULL)
		return NULL;
	hitem=TreeView_GetChild(hwnd,p->hitem);
	return GetPrimitive(hitem);
}
Primitive *Tree::GetChild(Primitive *p) {
	Primitive *pNext=p->prNext;
	if(p && p->prNext && p->prNext->prParent==p)
			return pNext;
	return NULL;
}

Primitive *Tree::GetNextSibling(Primitive *p) {
	Primitive *prParent;
	if(p==0)
		return 0;
	prParent=p->prParent;
	for(;;) {
		p=p->prNext;
		if(p==0)
			return 0;
		if(p->prParent==prParent)
			return p;
	}
	return 0;
/*
	HTREEITEM hitem;
	if(p==NULL)
		return NULL;
	hitem=TreeView_GetNextSibling(hwnd,p->hitem);
	return GetPrimitive(hitem);
*/
}
Primitive *Tree::GetPrevSibling(Primitive *p) {
	HTREEITEM hitem;
	if(p==NULL)
		return NULL;
	hitem=TreeView_GetPrevSibling(hwnd,p->hitem);
	return GetPrimitive(hitem);
}
int Tree::GetChildNum(Primitive *p) {
	int i;
	Primitive *pn=GetNextSibling(p);
	Primitive *pc=GetChild(p);
	i=0;
	while(pc&&pc!=pn) {
		pc=GetNext(pc);
		++i;
	}
	return i;
}
Primitive *Tree::GetFirst(void) {
	return prRoot;
}
Primitive *Tree::GetNext(Primitive *pr) {
	if(pr)
		return pr->prNext;
	return 0;
}
Primitive *Tree::GetFirstFromTV(void) {
	Primitive *p;
	TVITEM item;
	item.mask=TVIF_HANDLE;
	item.hItem=TreeView_GetRoot(hwnd);
	if(item.hItem==NULL)
		return NULL;
	item.mask=TVIF_HANDLE|TVIF_PARAM;
	TreeView_GetItem(hwnd,&item);
	p=(Primitive*)item.lParam;
	p->hitem=item.hItem;
	return p;
}
Primitive *Tree::GetLastFromTV(void) {
	Primitive *p;
	TVITEM item;
	HTREEITEM hitem,hitemNext;
	item.mask=TVIF_HANDLE;
	hitem=TreeView_GetRoot(hwnd);
	for(;;) {
		hitemNext=TreeView_GetNextSibling(hwnd,hitem);
		if(hitemNext==NULL) {
			hitemNext=TreeView_GetChild(hwnd,hitem);
			if(hitemNext==0)
				break;
		}
		hitem=hitemNext;
	}
	item.hItem=hitem;
	if(item.hItem==NULL)
		return NULL;
	item.mask=TVIF_HANDLE|TVIF_PARAM;
	TreeView_GetItem(hwnd,&item);
	p=(Primitive*)item.lParam;
	p->hitem=item.hItem;
	return p;
}
Primitive *Tree::GetNextFromTV(Primitive *p) {
	HTREEITEM hitem,hitemNext;
	TVITEM item;
	if(p==0)
		return 0;
	hitem=p->hitem;
	if((hitemNext=TreeView_GetChild(hwnd,hitem))==NULL) {
		if((hitemNext=TreeView_GetNextSibling(hwnd,hitem))==NULL) {
			for(;;) {
				hitem=TreeView_GetParent(hwnd,hitem);
				if(hitem==NULL)
					break;
				hitemNext=TreeView_GetNextSibling(hwnd,hitem);
				if(hitemNext)
					break;
			}
		}
	}
	if(hitemNext==NULL)
		return NULL;
	item.mask=TVIF_PARAM;
	item.hItem=hitemNext;
	TreeView_GetItem(hwnd,&item);
	p=(Primitive*)item.lParam;
	p->hitem=item.hItem;
	return p;
}
Primitive *Tree::GetPrevFromTV(Primitive *p) {
	HTREEITEM hitem,hitemPrev;
	TVITEM item;
	if(p==0)
		return 0;
	hitem=p->hitem;
	if((hitemPrev=TreeView_GetPrevSibling(hwnd,hitem))==NULL) {
		hitemPrev=TreeView_GetParent(hwnd,hitem);
	}
	else {
		hitem=hitemPrev;
		for(;;) {
			hitemPrev=TreeView_GetChild(hwnd,hitem);
			if(hitemPrev==NULL) {
				hitemPrev=hitem;
				break;
			}
			while(hitemPrev) {
				hitem=hitemPrev;
				hitemPrev=TreeView_GetNextSibling(hwnd,hitem);
			}
		}
	}
	if(hitemPrev==NULL)
		return NULL;
	item.mask=TVIF_PARAM;
	item.hItem=hitemPrev;
	TreeView_GetItem(hwnd,&item);
	p=(Primitive*)item.lParam;
	p->hitem=item.hItem;
	return p;
}
bool Tree::IsCheckFromTV(Primitive *p) {
	TVITEM item;
	HTREEITEM hitem;
	if(p==0)
		return false;
	hitem=p->hitem;
	item.mask=TVIF_IMAGE;
	item.hItem=p->hitem;
	TreeView_GetItem(hwnd,&item);
	if(item.iImage>=8)
		return true;
	else
		return false;
}
bool Tree::IsSelected(Primitive *pr,bool bSel) {
	return prlCurrent.Check(pr,bSel)!=0;
}
bool Tree::IsGrouped(Primitive *pr) {
	for(;;) {
		if(pr->prType==prtGroup)
			return true;
		pr=GetParent(pr);
		if(pr==NULL)
			return false;
	}
}
void Tree::SelectAdd(Primitive *pr,int iRefresh) {
	if(pr==NULL)
		return;
	DebugMessage1(L"SelectAdd\n");
	iDisableResponse++;
	prlCurrent.Add(pr,true);
	MarkItem(pr,2);
	if(iRefresh) {
		TreeView_SelectItem(hwnd,pr->hitem);
		theProp->SetType(pr->prType);
		theProp->Setup(pr);
	}
	iDisableResponse--;
}
void Tree::SelectDel(Primitive *pr) {
	prlCurrent.Del(pr,true);
}
void Tree::MarkItem(Primitive *pr,int m) {
	TVITEM tvitem;
	tvitem.mask=TVIF_HANDLE|TVIF_IMAGE|TVIF_SELECTEDIMAGE;
	tvitem.hItem=pr->hitem;
	TreeView_GetItem(hwnd,&tvitem);
	tvitem.iImage=(tvitem.iImage&7)+m*8;
	tvitem.iSelectedImage=(tvitem.iSelectedImage&7)+m*8;
	TreeView_SetItem(hwnd,&tvitem);
}
void Tree::SelectClear(void) {
	Primitive *pr;
	prlCurrent.Clear();
	for(pr=GetFirst();pr;pr=GetNext(pr)) {
		MarkItem(pr,0);
	}
	theScreen->Send();
}
void Tree::Select1(Primitive *pr,int iRefresh) {
	if(pr) {
		SelectClear();
		SelectAdd(pr,iRefresh);
	}
	else {
		SelectClear();
		TreeView_SelectItem(hwnd,NULL);
	}
}
void Tree::RefreshAll(void) {
	Primitive *pr;
	for(pr=GetFirst();pr;pr=GetNext(pr)) {
		pr->SetDirty();
	}
	theScreen->Send();
}
void Tree::SelectInvert(void) {
	int iSel;
	Primitive *pr;
	PrimitivePtrList *pl,*p;
	pl=prlCurrent.Clone();
	Select1(NULL);
	for(pr=GetFirst();pr;pr=GetNext(pr)) {
		iSel=0;
		for(p=pl;p->next;p=p->next) {
			if(p->pr==pr) {
				iSel=1;
				break;
			}
		}
		if(iSel==0)
			SelectAdd(pr,0);
	}
	delete pl;
}
void Tree::SelectAll(void) {
	Primitive *p;
	p=GetFirst();
	Select1(p);
	while((p=GetNext(p))!=NULL) {
		SelectAdd(p,0);
	}
	theScreen->Send();
}
void Tree::SelectVisible(void) {
	Primitive *p;
	Select1(NULL);
	for(p=GetFirst();p;p=GetNext(p)) {
		if((p->iVisible&4)!=0)
			SelectAdd(p,0);
	}
	theScreen->Send();
}
void Tree::TrimmingExec(int x0,int y0,int x1,int y1) {
	Primitive *pr;
	if(x0==x1 || y0==y1)
		return;
	Sort(&x0,&x1);
	Sort(&y0,&y1);
	EnterCriticalSection(&csData);
	theScreen->CanvasSize(x1-x0,y1-y0);
	for(pr=GetFirst();pr;pr=GetNext(pr)) {
		MovePos(pr,-x0,-y0);
	}
	theScreen->Send();
	LeaveCriticalSection(&csData);
}
int fnDistComp(const void*p1,const void*p2) {
	return ((PrTabItem*)p1)->x-((PrTabItem*)p2)->x;
}
PointF RefPoint(Primitive *pr,int iPos) {
	int x0,y0,x1,y1;
	if(pr) {
		x0=pr->ptfOrg[0].X;
		y0=pr->ptfOrg[0].Y;
		x1=pr->ptfOrg[1].X;
		y1=pr->ptfOrg[1].Y;
	}
	else {
		x0=y0=0.f;
		x1=theScreen->iWidth;
		y1=theScreen->iHeight;
	}
	switch(iPos) {
	case 0:
		return PointF(x0,0.f);
		break;
	case 1:
		return PointF((x0+x1)*.5f,0.f);
		break;
	case 2:
		return PointF(x1,0.f);
		break;
	case 4:
		return PointF(0.f,y0);
	case 5:
		return PointF(0.f,(y0+y1)*.5f);
	case 6:
		return PointF(0.f,y1);
	}
	return PointF(0,0);
}
void Tree::Distribution(int iPos) {
	PrimitivePtrList *prl;
	Primitive *pr;
	float fMin,fMax,x;
	PrTabItem *prTab;
	int iCount,i,j;
	if(prlCurrent.iCount<3)
		return;
	theJournal->Record();
	fMin=FLOATMAX;
	fMax=FLOATMIN;
	iCount=0;
	for(prl=&prlCurrent;prl->next;prl=prl->next) {
		if(IsCheckFromTV(prl->pr))
			++iCount;
	}
	prTab=new PrTabItem[iCount];
	for(i=0,prl=&prlCurrent;prl->next;prl=prl->next) {
		if(IsCheckFromTV(prl->pr)) {
			pr=prl->pr;
			if(iPos<4)
				x=RefPoint(pr,iPos).X;
			else
				x=RefPoint(pr,iPos).Y;
			prTab[i].pr=pr;
			prTab[i].x=(int)x;
			if(x<fMin)
				fMin=x;
			if(x>fMax)
				fMax=x;
			++i;
		}
	}
	qsort(prTab,iCount,sizeof PrTabItem,fnDistComp);
	for(i=0;i<iCount;++i) {
		prTab[i].x=(int)(fMin+(fMax-fMin)*i/(iCount-1));
	}
	x=0;
	for(prl=&prlCurrent;prl->next;prl=prl->next) {
		if(IsCheckFromTV(prl->pr)) {
			for(j=0;j<iCount;++j) {
				if(prTab[j].pr==prl->pr) {
					if(iPos>=4)
						x=prTab[j].x-RefPoint(prl->pr,iPos).Y;
					else
						x=prTab[j].x-RefPoint(prl->pr,iPos).X;
				}
			}
		}
		EnterCriticalSection(&csData);
		if(iPos>=4)
			MovePos(prl->pr,0,x);
		else
			MovePos(prl->pr,x,0);
		LeaveCriticalSection(&csData);
	}
	theScreen->Send();
	delete[] prTab;
}
void Tree::Align(int iPos) {
	PointF ptf,ptfD;
	Primitive *pr,*prOrg,*prParent;
	PrimitivePtrList *prl;
	theJournal->Record();
	prOrg=theProp->prCurrent;
	if(prOrg==0)
		prOrg=prlCurrent.pr;
	if(prOrg==0)
		return;
	if(prlCurrent.iCount==1)
		ptf=RefPoint(0,iPos);
	else
		ptf=RefPoint(prOrg,iPos);
	prParent=theTree->GetParent(prOrg);
	ptfD=PointF(0.f,0.f);
	for(prl=&prlCurrent;prl->next;prl=prl->next) {
		pr=prl->pr;
		if(IsCheckFromTV(prl->pr)) {
			ptfD=ptf-RefPoint(pr,iPos);
		}
		EnterCriticalSection(&csData);
		MovePos(pr,ptfD.X,ptfD.Y);
		LeaveCriticalSection(&csData);
	}
	theScreen->Send();
}
void Tree::MoveParent(void) {
	bool bGrouped=GroupIfMulti();
	Primitive *p=prlCurrent.pr;
	if(p)
		Select1(MoveParent(p));
	if(bGrouped)
		::UnGroup(0);
}
void Tree::MoveUp(void) {
	bool bGrouped=GroupIfMulti();
	Primitive *p=prlCurrent.pr;
	if(p)
		Select1(MoveUp(p));
	if(bGrouped)
		::UnGroup(0);
}
void Tree::MoveDown(void) {
	bool bGrouped=GroupIfMulti();
	Primitive *p=prlCurrent.pr;
	if(p)
		Select1(MoveDown(p));
	if(bGrouped)
		::UnGroup(0);
}
void Tree::TopMost(void) {
	theJournal->Record();
	bool bGrouped=GroupIfMulti();
	Primitive *p=prlCurrent.pr;
	if(p)
		Select1(TopMost(p));
	if(bGrouped)
		::UnGroup(0);
}
Primitive *Tree::MoveParent(Primitive *p) {
	Primitive *pTarget=theTree->GetParent(p);
	Primitive *pParent=theTree->GetParent(pTarget);
	if(pTarget) {
		p=MoveItem(p,pParent,pTarget);
		if(p->iOperation)
			pTarget->CalcPath(2,1);
	}
	Recalc();
	return p;
}
Primitive *Tree::TopMost(Primitive *p) {
	if(p)
		return MoveItem(p,0,0);
	return 0;
}
Primitive *Tree::MoveUp(Primitive *p) {
	Primitive *pTarget=theTree->GetPrevSibling(p);
	if(pTarget) {
		pTarget=theTree->GetPrevSibling(pTarget);
		return MoveItem(p,GetParent(p),pTarget,true);
	}
	return p;
}
Primitive *Tree::MoveDown(Primitive *p) {
	Primitive *pTarget=theTree->GetNextSibling(p);
	if(pTarget)
		return MoveItem(p,GetParent(p),pTarget);
	return p;
}
void Tree::Foreground(Primitive *p) {
	MoveItem(p,GetParent(p),0);
}
void Tree::Dup(Primitive *p,Primitive *pTarget) {
}
HTREEITEM Tree::MoveItemExec(Primitive *p,Primitive *pTarget,Primitive *pAfter,bool bFirst) {
	TV_INSERTSTRUCT tvi;
	HTREEITEM hitem,hitemChild,hitemOld;
	wchar_t str[256];
	ZeroMemory(&tvi,sizeof(tvi));
	if(pTarget==NULL)
		tvi.hParent=TVI_ROOT;
	else
		tvi.hParent=pTarget->hitem;
	if(pAfter)
		tvi.hInsertAfter=pAfter->hitem;
	else {
		if(bFirst)
			tvi.hInsertAfter=TVI_FIRST;
		else
			tvi.hInsertAfter=TVI_LAST;
	}
	tvi.item.mask=TVIF_TEXT|TVIF_PARAM|TVIF_IMAGE|TVIF_SELECTEDIMAGE;
	tvi.item.pszText=str;
	tvi.item.cchTextMax=256;
	tvi.item.hItem=p->hitem;
	TreeView_GetItem(hwnd,&tvi.item);
	tvi.item.lParam=(LPARAM)p;
	hitem=TreeView_InsertItem(hwnd,&tvi);
	hitemOld=p->hitem;
	p->hitem=hitem;
	while(hitemChild=TreeView_GetChild(hwnd,hitemOld)) {
		MoveItemExec(GetPrimitive(hitemChild),p,pAfter);
	}
	TreeView_DeleteItem(hwnd,hitemOld);
	InvalidateRect(hwndVisible,NULL,true);
	return hitem;
}
Primitive *Tree::MoveItem(Primitive *p,Primitive *pTarget,Primitive *pAfter,bool bFirst) {
	Primitive *p2;
	HTREEITEM hitem;
	for(p2=pTarget;p2;p2=GetParent(p2))
		if(p2==p)
			return p;
	EnterCriticalSection(&csData);
	hitem=MoveItemExec(p,pTarget,pAfter,bFirst);
	Renumber();
	if(theScreen)
		theScreen->Send();
	LeaveCriticalSection(&csData);
	return GetPrimitive(hitem);
}
Primitive *Tree::MoveDragItem(Primitive *p,Primitive *pTarget,bool bAfter,bool bStepin) {
	Primitive *pParent,*pAfter,*pParentOld;
	bool bFirst;
	pParentOld=GetParent(p);
	if(bStepin) {
		p=MoveItem(p,pTarget,NULL,false);
		if(p->iOperation) {
			if(pParentOld)
				pParentOld->CalcPath(2,1);
			if(pTarget)
				pTarget->CalcPath(2,1);
		}
		return p;
	}
	pParent=GetParent(pTarget);
	bFirst=false;
	if(bAfter)
		pAfter=pTarget;
	else {
		pAfter=GetPrevSibling(pTarget);
		if(pAfter==0)
			bFirst=true;
	}
	p=MoveItem(p,pParent,pAfter,bFirst);
	if(p->iOperation && pParentOld) {
		pParentOld->CalcPath(2,1);
	}
	return p;
}
void Tree::MovePos(Primitive *p,float dx,float dy,int iDirty) {
	if(p==NULL)
		return;
	p->ptfOrg[0].X+=dx;
	p->ptfOrg[0].Y+=dy;
	p->ptfOrg[1].X+=dx;
	p->ptfOrg[1].Y+=dy;
	if(p->bz) {
		RectF rcf(p->ptfOrg[0].X,p->ptfOrg[0].Y,p->ptfOrg[1].X-p->ptfOrg[0].X,p->ptfOrg[1].Y-p->ptfOrg[0].Y);
		p->bz->GetPath(1,p->iClose,p->path,&rcf);
	}
	p->CalcPath(iDirty,0);
	p->CalcParent();
	if(p->iOperation) {
		p=theTree->GetParent(p);
		if(p)
			p->CalcPath(2,1);
	}
}
void Tree::MoveSelected(int dx,int dy) {
	PrimitivePtrList *prl;
	EnterCriticalSection(&csData);
	for(prl=&prlCurrent;prl->next;prl=prl->next)
		MovePos(prl->pr,(float)dx,(float)dy);
	theProp->SetupPos();
	LeaveCriticalSection(&csData);
}
void Tree::SetPos(Primitive *p,float x,float y) {
	float dx,dy;
	Matrix mx;
	HTREEITEM hitemLast;
	if(p==NULL)
		return;
	dx=x-p->ptfOrg[0].X;
	dy=y-p->ptfOrg[0].Y;
	p->ptfOrg[0].X=x;
	p->ptfOrg[0].Y=y;
	p->ptfOrg[1].X+=dx;
	p->ptfOrg[1].Y+=dy;
	if(p->bz) {
		mx.Reset();
		mx.Translate(dx,dy);
		mx.TransformPoints(p->bz->ptAnchor,p->bz->iIndex);
	}
	p->CalcPath(1,1);
	hitemLast=TreeView_GetNextSibling(hwnd,p->hitem);
	if(hitemLast==NULL)
		hitemLast=TreeView_GetNextSibling(hwnd,TreeView_GetParent(hwnd,p->hitem));
	while(p=theTree->GetNext(p)) {
		if(p->hitem==hitemLast)
			break;
		p->ptfOrg[0].X+=dx;
		p->ptfOrg[0].Y+=dy;
		p->ptfOrg[1].X+=dx;
		p->ptfOrg[1].Y+=dy;
		if(p->bz) {
			mx.Reset();
			mx.Translate(dx,dy);
			mx.TransformPoints(p->bz->ptAnchor,p->bz->iIndex);
		}
		p->CalcPath(1,1);
	}
}
Primitive *Tree::FindFromNum(int n) {
	Primitive *p;
	int iCount=0;
	if(n<=0)
		return NULL;
	for(p=GetFirst();p;p=GetNext(p)) {
		if(++iCount>=n)
			return p;
	}
	return NULL;
}
bool Tree::IsHit(Primitive *pr,int x,int y) {
	if(pr->IsLock())
		return false;
	return pr->pathOutline->IsVisible(x,y)==TRUE;
}
HTREEITEM Tree::GetItemHandle(Primitive *pr) {
	Primitive *p;
	for(p=GetFirst();p;p=GetNext(p))
		if(p==pr)
			return p->hitem;
	return NULL;
}
Primitive *Tree::FindFromPos(int x,int y,int iNext) {
	Primitive *p;
	p=0;
	theScreen->HitTest(&p,tArrow,x,y,2,0);
	return p;
}
void Tree::BeginDrag(NMTREEVIEW *pnmtv) {
    HIMAGELIST himl;
    RECT rcItem;
	Primitive *pr=(Primitive*)(pnmtv->itemNew.lParam);
    DWORD dwLevel;
    DWORD dwIndent;
	if(IsSelected(pr,true)==0)
		Select1(pr);
    himl = TreeView_CreateDragImage(hwnd, pnmtv->itemNew.hItem); 
    TreeView_GetItemRect(hwnd, pnmtv->itemNew.hItem, &rcItem, TRUE); 
    dwLevel = (DWORD)(pnmtv->itemNew.lParam); 
    dwIndent = (DWORD) SendMessage(hwnd, TVM_GETINDENT, 0, 0); 
    ImageList_BeginDrag(himl, 0,0,0); 
	ImageList_DragEnter(hwnd,0,0);
	hitemDragging=pnmtv->itemNew.hItem;
 	bDragging = TRUE; 
}
void Tree::MouseMove(int x,int y) {
    TVHITTESTINFO tvht;
	RECT rc;
	DebugMessage1(L"MOUSEMOVE\n");
    if(bDragging) { 
		ImageList_DragShowNolock(FALSE);
        tvht.pt.x = x; 
        tvht.pt.y = y; 
        if ((hitemTarget = TreeView_HitTest(hwnd, &tvht)) != NULL) {
			TreeView_GetItemRect(hwnd,hitemTarget,&rc,0);
			if(y<(rc.top+(rc.bottom-rc.top)/4)) {
				TreeView_SetInsertMark(hwnd,hitemTarget,FALSE);
	            TreeView_SelectDropTarget(hwnd, NULL); 
				bAfter=false;
				bStepin=false;
			}
			else if(y<rc.top+(rc.bottom-rc.top)*3/4) {
				TreeView_SetInsertMark(hwnd,NULL,0);
	            TreeView_SelectDropTarget(hwnd, hitemTarget); 
				bStepin=true;
			}
			else {
				TreeView_SetInsertMark(hwnd,hitemTarget,TRUE);
	            TreeView_SelectDropTarget(hwnd, NULL); 
				bAfter=true;
				bStepin=false;
			}
			UpdateWindow(hwnd);
        } 
		else {
//			TreeView_SelectItem(hwnd,NULL);
			TreeView_SetInsertMark(hwnd,NULL,FALSE);
            TreeView_SelectDropTarget(hwnd, NULL); 
			UpdateWindow(hwnd);
		}
		ImageList_DragShowNolock(TRUE);
		ImageList_DragMove(x, y); 
    } 
}
void Tree::LButtonUp(void) {
	DebugMessage1(L"LBUTTONUP\n");
	Primitive *prTarget;
    if(bDragging) { 
		prTarget=GetPrimitive(hitemTarget);
		TreeView_SetInsertMark(hwnd,NULL,0);
        ImageList_EndDrag(); 
		ImageList_DragLeave(hwnd);
		bDragging = FALSE;
		theJournal->Record();
//		MoveDragItem(GetPrimitive(hitemDragging),GetPrimitive(hitemTarget),bAfter,bStepin);
		TreeView_SelectDropTarget(hwnd,NULL);
		if(prTarget && !IsSelected(prTarget)) {
			this->iDisableResponse++;
			bool bGrouped=GroupIfMulti();
			Select1(MoveDragItem(prlCurrent.pr,prTarget,bAfter,bStepin));
			if(bGrouped)
				::UnGroup(0);
			this->iDisableResponse--;
		}
		InvalidateRect(hwnd,NULL,TRUE);
		theScreen->Send();
    } 
}

Journal::Journal(Tree *treeInit,int iLevel) {
	tree=treeInit;
	iIndexStart=iIndexEnd=iIndex=0;
	bOnce=0;
	iBuffNum=iLevel+1;
	log=new JournalLog[iBuffNum];
}
Journal::~Journal(void) {
	delete[] log;
}
void Journal::CopyFromLog(int i) {
	Primitive *pr,*prParent;
	PrimitiveList *prl=log[i].prlUndo;
	int iParentOrg,iParent,iCount;
	EnterCriticalSection(&csData);
	if(log[i].iWidth!=theScreen->iWidth || log[i].iHeight!=theScreen->iHeight) {
		theScreen->CanvasSize(log[i].iWidth,log[i].iHeight);
	}
	theScreen->colBackground=log[i].colBackground;
	theScreen->colWorkspace=log[i].colWorkspace;
	theTree->DelAll();
	prParent=NULL;
	iCount=log[i].iProp-1;
	if(iCount<0)
		theProp->SetType(prtNone);
	if(prl->next) {
		pr=theTree->prlCurrent.pr=theTree->AddItem(NULL,NULL,prl->pr->strName,prl->pr->prType);
		prl->CopyTo(pr);
		pr->CalcPath(2,1);
		theTree->SetName(pr);
		theTree->Renumber();
		iParentOrg=pr->iNum;
		prl=prl->next;
		if(iCount==0) {
			theProp->SetType(pr->prType);
			theProp->Setup(pr);
		}
	}
	for(;prl->next;prl=prl->next) {
		iParent=iParentOrg+prl->iParent;
		prParent=theTree->FindFromNum(iParent);
		pr=theTree->prlCurrent.pr=theTree->AddItem(prParent,NULL,prl->pr->strName,prl->pr->prType);
		prl->CopyTo(pr);
		pr->CalcPath(2,1);
		theTree->SetName(pr);
		--iCount;
		if(iCount==0) {
			theProp->SetType(pr->prType);
			theProp->Setup(pr);
		}
	}
	theApp->Edit(log[i].bEdit);
	theScreen->Send();
	LeaveCriticalSection(&csData);
	theApp->UpdateTitle();
}
void Journal::CopyToLog(int i) {
	Primitive *pr;
	int iCur;
	PrimitiveList *prl;
	if(log[i].prlUndo)
		delete log[i].prlUndo;
	prl=log[i].prlUndo=new PrimitiveList();
	log[i].iWidth=theScreen->iWidth;
	log[i].iHeight=theScreen->iHeight;
	log[i].bEdit=theApp->IsEdit();
	log[i].colBackground=theScreen->colBackground;
	log[i].colWorkspace=theScreen->colWorkspace;
	if(theProp->prCurrent)
		log[i].iProp=theProp->prCurrent->iNum;
	else
		log[i].iProp=-1;
	pr=theTree->GetFirst();
	if(pr) {
		prl->Add(pr,0);
		theTree->Renumber();
		for(pr=theTree->GetNext(pr);pr;pr=theTree->GetNext(pr)) {
			iCur=theTree->GetParentNum(pr);
			prl->Add(pr,iCur-1);
		}
	}
}
void Journal::RecordOnce(void) {
	if(bOnce==false)
		Record();
	bOnce=true;
}
void Journal::RecordReset(void) {
	bOnce=false;
}
void Journal::Record(void) {
	if(iBuffNum<2)
		return;
	theTree->Renumber();
	CopyToLog(iIndex);
	++iIndex;
	if(iIndex>=iBuffNum)
		iIndex=0;
	iIndexEnd=iIndex;
	if(iIndex==iIndexStart)
		++iIndexStart;
	if(iIndexStart>=iBuffNum)
		iIndexStart=0;
	if(log[iIndex].prlUndo) {
		delete log[iIndex].prlUndo;
		log[iIndex].prlUndo=0;
	}
}
void Journal::Undo(void) {
	theTree->Renumber();
	if(iIndex==iIndexStart)
		return;
	CopyToLog(iIndex);
	if(--iIndex<0)
		iIndex=iBuffNum-1;
	CopyFromLog(iIndex);
	theTree->Select1(theProp->prCurrent);
	theTree->Recalc();
}
void Journal::Redo(void) {
	if(iIndex==iIndexEnd)
		return;
	if(++iIndex>=iBuffNum)
		iIndex=0;
	CopyFromLog(iIndex);
	theTree->Select1(theProp->prCurrent);
	theTree->Recalc();
}

Prop::Prop(HWND hwndInit) {
	HWND hwnd;
	bFloat=true;
	tipTex=tipText=tipFont=0;
	hwndParent=hwndInit;
	himlBorderjoin=ImageList_Create(32,24,ILC_COLOR24,3,3);
	himlLineCap=ImageList_Create(32,24,ILC_COLOR24,3,3);
	himlTexture=0;
	ImageList_Add(himlBorderjoin,theApp->hbmpMiter,0);
	ImageList_Add(himlBorderjoin,theApp->hbmpBevel,0);
	ImageList_Add(himlBorderjoin,theApp->hbmpRound,0);
	ImageList_Add(himlLineCap,theApp->hbmpFlatCap,0);
	ImageList_Add(himlLineCap,theApp->hbmpSquareCap,0);
	ImageList_Add(himlLineCap,theApp->hbmpRoundCap,0);
	ImageList_Add(himlLineCap,theApp->hbmpTriangleCap,0);
	hwndFrame=CreateDialog(hinstMain,MAKEINTRESOURCE(IDD_PROPS1),hwndParent,(DLGPROC)dlgprocPropFrame);
	SetWindowLongPtr(hwndFrame,GWLP_USERDATA,(LONG)(LONG_PTR)this);
	hwndSub=hwndAnim=NULL;
	prCurrent=0;
	iOffsetY=0;
	iSubHeight=0;
	iPage=0;
	CBEXAdd(hwndFrame,IDC_TYPE,L"Rect");
	CBEXAdd(hwndFrame,IDC_TYPE,L"Ellipse");
	CBEXAdd(hwndFrame,IDC_TYPE,L"Polygon");
	CBEXAdd(hwndFrame,IDC_TYPE,L"Text");
	CBEXAdd(hwndFrame,IDC_TYPE,L"Shape");
	CBEXAdd(hwndFrame,IDC_TYPE,L"Lines");
	CBEXAdd(hwndFrame,IDC_TYPE,L"Image");
	CBEXAdd(hwndFrame,IDC_TYPE,L"Group");
	MoveWindow(hwndFrame,theApp->rcProp.left,theApp->rcProp.top,theApp->rcProp.right-theApp->rcProp.left,theApp->rcProp.bottom-theApp->rcProp.top,0);
	hwnd=GetDlgItem(hwndFrame,IDC_FRAME);
//	hwndAnim=CreateDialogParam(hinstMain,MAKEINTRESOURCE(IDD_PROPANIM),hwnd,(DLGPROC)dlgprocPropSub,(LPARAM)this);
//	MoveWindow(hwndAnim,0,0,100,100,TRUE);

	hwnd=GetDlgItem(hwndFrame,IDC_FRAME);
}
Prop::~Prop(void) {
	ImageList_Destroy(himlBorderjoin);
	ImageList_Destroy(himlLineCap);
	if(himlTexture)
		ImageList_Destroy(himlTexture);
	himlTexture=0;
	DestroyWindow(hwndFrame);
}
void Prop::Destroy(void) {
}
void Prop::Resume(void) {
}
void Prop::Show(int i) {
	if(i)
		ShowWindow(hwndFrame,SW_NORMAL);
	else
		ShowWindow(hwndFrame,SW_HIDE);
}
int Prop::IsShow(void) {
	return IsWindowVisible(hwndFrame);
}
BOOL CALLBACK dlgprocTTT(HWND hwnd,UINT msg,WPARAM wparam,LPARAM lparam) {
	switch(msg) {
	case IDOK:
		EndDialog(hwnd,0);
		break;
	}
	return FALSE;
}
void SetMenu(prim pr) {
	HMENU hmenu=GetMenu(theApp->hwnd);
	hmenu=GetSubMenu(hmenu,1);
	hmenu=GetSubMenu(hmenu,6);
	if(pr==prtShape||pr==prtText) {
		EnableMenuItem(hmenu,ID_PRIMITIVE_MAKECIRCLE,MF_BYCOMMAND|MF_ENABLED);
		EnableMenuItem(hmenu,ID_PRIMITIVE_EDITSHAPE,MF_BYCOMMAND|MF_ENABLED);
	}
	else {
		EnableMenuItem(hmenu,ID_PRIMITIVE_MAKECIRCLE,MF_BYCOMMAND|MF_GRAYED);
		EnableMenuItem(hmenu,ID_PRIMITIVE_EDITSHAPE,MF_BYCOMMAND|MF_GRAYED);
	}
	if(pr==prtImage) {
		EnableMenuItem(hmenu,ID_PRIMITIVE_EDITKNOB,MF_BYCOMMAND|MF_ENABLED);
		EnableMenuItem(hmenu,ID_PRIMITIVE_EXTRACTFILE,MF_BYCOMMAND|MF_ENABLED);
	}
	else {
		EnableMenuItem(hmenu,ID_PRIMITIVE_EDITKNOB,MF_BYCOMMAND|MF_GRAYED);
		EnableMenuItem(hmenu,ID_PRIMITIVE_EXTRACTFILE,MF_BYCOMMAND|MF_GRAYED);
	}
}
void Prop::SetPage(int i) {
	iPage=i;
	if(i==0) {
		ShowWindow(hwndSub,SW_NORMAL);
//		ShowWindow(hwndAnim,SW_HIDE);
	}
	else {
		ShowWindow(hwndSub,SW_HIDE);
//		ShowWindow(hwndAnim,SW_NORMAL);
	}
}
void Prop::SetType(prim pr) {
	HWND hwnd;
	RECT rc;
	DebugMessage1(L"SetType\n");
	SetMenu(pr);
	if(prType==pr)
		return;
	this->prCurrent=NULL;
	prType=pr;
	if(hwndSub) {
		tipTex=tipText=tipFont=0;
		SetWindowLongPtr(hwndSub, GWLP_WNDPROC, (LONG)(LONG_PTR)wndprocOrg);
		DestroyWindow(hwndSub);
		if(himlTexture)
			ImageList_Destroy(himlTexture);
		himlTexture=0;
	}
	hwnd=GetDlgItem(hwndFrame,IDC_FRAME);
	switch(pr) {
	case prtGroup:
	case prtNone:
		hwndSub=CreateDialogParam(hinstMain,MAKEINTRESOURCE(IDD_PROPNONE),hwnd,(DLGPROC)dlgprocPropSub,(LPARAM)this);
		break;
	case prtRect:
		hwndSub=CreateDialogParam(hinstMain,MAKEINTRESOURCE(IDD_PROPRECT),hwnd,(DLGPROC)dlgprocPropSub,(LPARAM)this);
		break;
	case prtEllipse:
		hwndSub=CreateDialogParam(hinstMain,MAKEINTRESOURCE(IDD_PROPELLIPSE),hwnd,(DLGPROC)dlgprocPropSub,(LPARAM)this);
		break;
	case prtPolygon:
		hwndSub=CreateDialogParam(hinstMain,MAKEINTRESOURCE(IDD_PROPPOLYGON),hwnd,(DLGPROC)dlgprocPropSub,(LPARAM)this);
		break;
	case prtLines:
		hwndSub=CreateDialogParam(hinstMain,MAKEINTRESOURCE(IDD_PROPLINES),hwnd,(DLGPROC)dlgprocPropSub,(LPARAM)this);
		break;
	case prtText:
		hwndSub=CreateDialogParam(hinstMain,MAKEINTRESOURCE(IDD_PROPTEXT),hwnd,(DLGPROC)dlgprocPropSub,(LPARAM)this);
		SetupFont();
		break;
	case prtShape:
		hwndSub=CreateDialogParam(hinstMain,MAKEINTRESOURCE(IDD_PROPSHAPE),hwnd,(DLGPROC)dlgprocPropSub,(LPARAM)this);
		break;
	case prtImage:
		hwndSub=CreateDialogParam(hinstMain,MAKEINTRESOURCE(IDD_PROPIMAGE),hwnd,(DLGPROC)dlgprocPropSub,(LPARAM)this);
		break;
	}
	wndprocOrg=(WNDPROC)(LONG_PTR)SetWindowLongPtr(hwndSub, GWLP_WNDPROC, (LONG)(LONG_PTR)wndprocSubclassPropSub);
	GetWindowRect(hwndSub,&rc);
	iSubHeight=rc.bottom-rc.top;
	Size(-1,-1);
	Scroll(0,0);
	SetScroll();
	if(iPage==0)
		ShowWindow(hwndSub,SW_NORMAL);
	else
		ShowWindow(hwndSub,SW_HIDE);
}
void Prop::SetScroll(void) {
	int yPane;
	SCROLLINFO si;
	RECT rc;
	GetWindowRect(GetDlgItem(hwndFrame,IDC_FRAME),&rc);
	yPane=rc.bottom-rc.top;
	ZeroMemory(&si,sizeof(si));
	si.cbSize=sizeof(si);
	si.fMask=SIF_POS|SIF_RANGE|SIF_PAGE;
	si.nPos=iOffsetY;
	si.nMin=0;
	si.nMax=100;
	if(iSubHeight) {
		si.nPage=yPane*100/iSubHeight;
		SetScrollInfo(hwndFrame,SB_VERT,&si,true);
	}
}
void Prop::Size(int x,int y) {
	RECT rc,rcClient,rcControl;
	POINT pt;
	int iWidth;
	GetWindowRect(hwndFrame,&rc);
	if(x<0&&y<0) {
		GetWindowRect(hwndFrame,&rc);
		x=rc.right-rc.left;
		y=rc.bottom-rc.top;
	}
	if(x<0)
		x=200;
	if(y<0)
		y=500;
	GetClientRect(hwndFrame,&rcClient);
	GetWindowRect(GetDlgItem(hwndFrame,IDC_FRAME),&rc);
	pt.x=0;
	pt.y=rc.top;
	ScreenToClient(hwndFrame,&pt);
	SetWindowPos(GetDlgItem(hwndFrame,IDC_FRAME),0,0,0,x,y-pt.y,SWP_NOZORDER|SWP_NOMOVE);
	SetWindowPos(hwndSub,0,0,0,rcClient.right,iSubHeight+rcClient.bottom*2,SWP_NOZORDER|SWP_NOMOVE);
//	SetWindowPos(hwndAnim,0,0,0,rcClient.right,iSubHeight+rcClient.bottom*2,SWP_NOZORDER|SWP_NOMOVE);
	GetWindowRect(GetDlgItem(hwndSub,IDC_FONT),&rcControl);
	iWidth=rcClient.right-rcControl.left+rc.left-32;
	SetWindowPos(GetDlgItem(hwndSub,IDC_FONT),0,0,0,iWidth,rcControl.bottom-rcControl.top,SWP_NOZORDER|SWP_NOMOVE);
	SendDlgItemMessage(hwndSub,IDC_FONT,CB_SETDROPPEDWIDTH,iWidth*2,0);
	GetWindowRect(GetDlgItem(hwndSub,IDC_TEXT),&rcControl);
	iWidth=rcClient.right-rcControl.left+rc.left-32;
	SetWindowPos(GetDlgItem(hwndSub,IDC_TEXT),0,0,0,iWidth,rcControl.bottom-rcControl.top,SWP_NOZORDER|SWP_NOMOVE);
	GetWindowRect(GetDlgItem(hwndSub,IDC_TEXTURETYPE),&rcControl);
	iWidth=rcClient.right-rcControl.left+rc.left-32;
	SetWindowPos(GetDlgItem(hwndSub,IDC_TEXTURETYPE),0,0,0,iWidth,rc.bottom-rc.top,SWP_NOZORDER|SWP_NOMOVE);
	SendDlgItemMessage(hwndSub,IDC_TEXTURETYPE,CB_SETDROPPEDWIDTH,iWidth*2,0);
	SetScroll();
	InvalidateRect(hwndFrame,NULL,false);
}
void Prop::Pos(int x,int y) {
	RECT rcAll;
	POINT pt;
	GetVirtualScreenRect(&rcAll);
	pt.x=x,pt.y=y;
	if(PtInRect(&rcAll,pt)==FALSE)
		x=0,y=0;
	SetWindowPos(hwndFrame,0,x,y,0,0,SWP_NOZORDER|SWP_NOSIZE);
}
void Prop::SetupPos(Primitive *pr) {
	Primitive *p=prCurrent;
	if(p==NULL)
		return;
	if(pr && pr!=p)
		return;
	SetDlgItemFloat2(hwndSub,IDC_WIDTH,p->fWidth,true);
	SetDlgItemFloat2(hwndSub,IDC_ROUNDX1,p->fRoundX1,true);
	SetDlgItemFloat2(hwndSub,IDC_ROUNDX2,p->fRoundX2,true);
	SetDlgItemFloat2(hwndSub,IDC_ROUNDX3,p->fRoundX3,true);
	SetDlgItemFloat2(hwndSub,IDC_ROUNDX4,p->fRoundX4,true);
	SetDlgItemFloat2(hwndSub,IDC_ROUNDY1,p->fRoundY1,true);
	SetDlgItemFloat2(hwndSub,IDC_ROUNDY2,p->fRoundY2,true);
	SetDlgItemFloat2(hwndSub,IDC_ROUNDY3,p->fRoundY3,true);
	SetDlgItemFloat2(hwndSub,IDC_ROUNDY4,p->fRoundY4,true);
	SetDlgItemFloat3(hwndSub,IDC_X,p->ptfOrg[0].X,true);
	SetDlgItemFloat3(hwndSub,IDC_Y,p->ptfOrg[0].Y,true);
	SetDlgItemFloat3(hwndSub,IDC_W,(p->ptfOrg[1].X-p->ptfOrg[0].X),true);
	SetDlgItemFloat3(hwndSub,IDC_H,(p->ptfOrg[1].Y-p->ptfOrg[0].Y),true);
}
void Prop::SetupFont(void) {
	Primitive *p=prCurrent;
	if(p==NULL)
		return;
	if(p->prType!=prtText)
		return;
	SetDlgItemFloat3(hwndSub,IDC_FONTSIZE,p->fFontSize,true);
	SetDlgItemFloat3(hwndSub,IDC_FONTASPECT,p->fFontAspect,true);
	SetDlgItemFloat3(hwndSub,IDC_FONTSPACING,p->fFontSpacing,true);
}
void Prop::SetupColor(void) {
	wchar_t str[80];
	Primitive *p=prCurrent;
	if(p==NULL)
		return;
	swprintf(str,L"%08x",p->dwColor1);
	SetDlgItemText(hwndSub,IDC_COLOR1,str);
	swprintf(str,L"%08x",p->dwColor2);
	SetDlgItemText(hwndSub,IDC_COLOR2,str);
	swprintf(str,L"%08x",p->dwColor3);
	SetDlgItemText(hwndSub,IDC_COLOR3,str);
	swprintf(str,L"%08x",p->dwShadowColor);
	SetDlgItemText(hwndSub,IDC_SHADOWCOLOR,str);
	swprintf(str,L"%08x",p->dwIShadowColor);
	SetDlgItemText(hwndSub,IDC_ISHADOWCOLOR,str);
}
wchar_t *CornerStr(int n) {
	switch(n) {
	case 0:
		return L"R";
	case 1:
		return L"/";
	case 2:
		return L"-R";
	case 3:
		return L"L";
	}
	return L"";
}
void Prop::Setup(Primitive *p,bool bAdjust) {
	int i,iBD,iD,iPD;
	wchar_t *strType[]={L"Rect",L"Ellipse",L"Polygon",L"Text",L"Shape",L"Lines",L"Image",L"Group"};
	wchar_t str[320];
	prCurrent=p;
	if(p==NULL) {
		return;
	}
	if(bAdjust)
		theApp->pal->Set(p);
	iD=p->iBmpDirty;
	iBD=p->iBmpBaseDirty;
	iPD=p->iBmpPrevDirty;
	p->bInitializing=true;
	if(himlTexture)
		ImageList_Destroy(himlTexture);
	himlTexture=theTexture->CreateImageList(p);
	CBEXSetImageList(hwndSub,IDC_TEXTURETYPE,himlTexture);
	CBEXDelete(hwndSub,IDC_TEXTURETYPE,0);
	int iLen=wcslen(p->strTextureName);
	if(iLen>=2 && *p->strTextureName=='(' && p->strTextureName[iLen-1]==')') {
		wchar_t strTmp[320],*strExt;
		wcscpy(strTmp,p->strTextureName);
		strExt=wcsrchr(strTmp,'.');
		if(strExt) {
			*strExt++=')';
			*strExt=0;
		}
		CBEXInsert(hwndSub,IDC_TEXTURETYPE,0,0,strTmp);
	}
	else
		CBEXInsert(hwndSub,IDC_TEXTURETYPE,0,0,p->strTextureName);
	SendDlgItemMessage(hwndFrame,IDC_TYPE,CB_SETCURSEL,(WPARAM)p->prType,0);
	SendDlgItemMessage(hwndSub,IDC_OPERATION,CB_SETCURSEL,(WPARAM)p->iOperation,0);
	SetDlgItemFloat3(hwndSub,IDC_X,p->ptfOrg[0].X,true);
	SetDlgItemFloat3(hwndSub,IDC_Y,p->ptfOrg[0].Y,true);
	SetDlgItemFloat3(hwndSub,IDC_W,(p->ptfOrg[1].X-p->ptfOrg[0].X),true);
	SetDlgItemFloat3(hwndSub,IDC_H,(p->ptfOrg[1].Y-p->ptfOrg[0].Y),true);
	SendDlgItemMessage(hwndSub,IDC_OPERATION,CB_SETCURSEL,(WPARAM)p->iOperation,0);

	SetupColor();
//	swprintf(str,L"%08x",p->dwColor1);
//	SetDlgItemText(hwndSub,IDC_COLOR1,str);
//	swprintf(str,L"%08x",p->dwColor2);
//	SetDlgItemText(hwndSub,IDC_COLOR2,str);
//	swprintf(str,L"%08x",p->dwColor3);
//	SetDlgItemText(hwndSub,IDC_COLOR3,str);
//	swprintf(str,L"%08x",p->dwShadowColor);
//	SetDlgItemText(hwndSub,IDC_SHADOWCOLOR,str);
//	swprintf(str,L"%08x",p->dwIShadowColor);
//	SetDlgItemText(hwndSub,IDC_ISHADOWCOLOR,str);
	SetDlgItemInt(hwndSub,IDC_ALPHA1,(p->dwColor1>>24)*100/255,false);
	SetDlgItemInt(hwndSub,IDC_ALPHA2,(p->dwColor2>>24)*100/255,false);
	SetDlgItemInt(hwndSub,IDC_ALPHA3,(p->dwColor3>>24)*100/255,false);
	SendDlgItemMessage(hwndSub,IDC_GRADATIONTYPE,CB_SETCURSEL,(WPARAM)p->iGradationType,0);
	SetDlgItemInt(hwndSub,IDC_LINEAR,p->iLinear,true);
	SetDlgItemInt(hwndSub,IDC_SMOOTHER,p->iSmoother,false);
	SetDlgItemFloat2(hwndSub,IDC_ROUNDX1,p->fRoundX1,true);
	SetDlgItemFloat2(hwndSub,IDC_ROUNDX2,p->fRoundX2,true);
	SetDlgItemFloat2(hwndSub,IDC_ROUNDX3,p->fRoundX3,true);
	SetDlgItemFloat2(hwndSub,IDC_ROUNDX4,p->fRoundX4,true);
	CheckButton(hwndSub,IDC_ROUNDSEPARATE,p->iRoundSeparate);
	SetDlgItemFloat2(hwndSub,IDC_ROUNDY1,p->fRoundY1,true);
	SetDlgItemFloat2(hwndSub,IDC_ROUNDY2,p->fRoundY2,true);
	SetDlgItemFloat2(hwndSub,IDC_ROUNDY3,p->fRoundY3,true);
	SetDlgItemFloat2(hwndSub,IDC_ROUNDY4,p->fRoundY4,true);
	CheckButton(hwndSub,IDC_ROUNDXYSEPARATE,p->iRoundXYSeparate);
	SetDlgItemText(hwndSub,IDC_CORNERC1,CornerStr(p->iCornerC1));
	SetDlgItemText(hwndSub,IDC_CORNERC2,CornerStr(p->iCornerC2));
	SetDlgItemText(hwndSub,IDC_CORNERC3,CornerStr(p->iCornerC3));
	SetDlgItemText(hwndSub,IDC_CORNERC4,CornerStr(p->iCornerC4));
	CheckButton(hwndSub,IDC_FILL,p->iFill);
	CheckButton(hwndSub,IDC_ANTIALIAS,p->iAntialias);
	CheckButton(hwndSub,IDC_BORDER,p->iBorder);
	SendDlgItemMessage(hwndSub,IDC_BORDERTYPE,CB_SETCURSEL,(WPARAM)p->iBorderType,0);
	SendDlgItemMessage(hwndSub,IDC_BORDERJOIN,CB_SETCURSEL,(WPARAM)p->iBorderJoin,0);
	SendDlgItemMessage(hwndSub,IDC_LINECAP,CB_SETCURSEL,(WPARAM)p->iLineCap,0);
	CheckButton(hwndSub,IDC_CLOSE,p->iClose);
	CheckButton(hwndSub,IDC_BORDERASFILL,p->iBorderAsFill);
	CheckButton(hwndSub,IDC_PIE,p->iPie);
	SetDlgItemFloat2(hwndSub,IDC_WIDTH,p->fWidth,false);
	SetDlgItemInt(hwndSub,IDC_ALPHA,p->iAlpha,false);
	SetDlgItemFloat3(hwndSub,IDC_ANGLE,p->rAngle,true);
	SetDlgItemFloat3(hwndSub,IDC_SLANT,p->fSlant,true);
	SetDlgItemInt(hwndSub,IDC_START,(int)p->iStart,true);
	SetDlgItemInt(hwndSub,IDC_STOP,(int)p->iStop,true);
	SetDlgItemInt(hwndSub,IDC_VERTEX,p->iVertex,false);
	SetDlgItemFloat2(hwndSub,IDC_TENSION,p->rTension,true);
	SetDlgItemFloat2(hwndSub,IDC_TWIST,p->rTwist,true);
	SetDlgItemInt(hwndSub,IDC_STARDEPTH,p->iStarDepth,true);
	SendDlgItemMessage(hwndSub,IDC_LINETYPE,CB_SETCURSEL,(WPARAM)p->iLineType,0);
	SetDlgItemInt(hwndSub,IDC_DIFFUSE,p->iDiffuse,false);
	SendDlgItemMessage(hwndSub,IDC_SPECULARTYPE,CB_SETCURSEL,(WPARAM)p->light[0].iSpecularType,0);
	SetDlgItemInt(hwndSub,IDC_SPECULARDIR,p->light[0].iSpecularDir,true);
	SetDlgItemInt(hwndSub,IDC_SPECULAROFF,p->light[0].iSpecularOffset,true);
	SetDlgItemInt(hwndSub,IDC_SPECULAR,p->light[0].iSpecular,true);
	SetDlgItemInt(hwndSub,IDC_HIGHLIGHT,p->light[0].iHighlight,true);
	SetDlgItemInt(hwndSub,IDC_HIGHLIGHTWIDTH,p->light[0].iHighlightWidth,false);
	SetDlgItemInt(hwndSub,IDC_AMBIENT,p->light[0].iAmbient,false);

	SendDlgItemMessage(hwndSub,IDC_SPECULARTYPE2,CB_SETCURSEL,(WPARAM)p->light[1].iSpecularType,0);
	SetDlgItemInt(hwndSub,IDC_SPECULARDIR2,p->light[1].iSpecularDir,true);
	SetDlgItemInt(hwndSub,IDC_SPECULAROFF2,p->light[1].iSpecularOffset,true);
	SetDlgItemInt(hwndSub,IDC_SPECULAR2,p->light[1].iSpecular,true);
	SetDlgItemInt(hwndSub,IDC_HIGHLIGHT2,p->light[1].iHighlight,true);
	SetDlgItemInt(hwndSub,IDC_HIGHLIGHTWIDTH2,p->light[1].iHighlightWidth,false);
	SetDlgItemInt(hwndSub,IDC_AMBIENT2,p->light[1].iAmbient,false);

	CheckButton(hwndSub,IDC_EMBOSSDIRENABLE,p->iEmbossDirEnable);
	SetDlgItemInt(hwndSub,IDC_EMBOSSDIR,p->iEmbossDir,true);
	SetDlgItemFloat2(hwndSub,IDC_EMBOSSWIDTH,p->rEmbossWidth,true);
	SetDlgItemFloat2(hwndSub,IDC_EMBOSSDEPTH,p->fEmbossDepth,true);
	SetDlgItemFloat2(hwndSub,IDC_EMBOSSDEPTH2,p->fEmbossDepth2,true);
	SendDlgItemMessage(hwndSub,IDC_TEXTURETYPE,CB_SETCURSEL,(WPARAM)p->iTextureType+1,0);
	GetDlgItemText(hwndSub,IDC_TEXTURETYPE,str,320);
	if(tipTex)
		tipTex->UpdateText(str);
	SetDlgItemInt(hwndSub,IDC_TEXTURE,p->iTexture,false);
	CheckButton(hwndSub,IDC_TEXTUREALPHA,p->iUseTextureAlpha);
	CheckButton(hwndSub,IDC_TEXTUREZOOMXYSEPA,p->iTextureZoomXYSepa);
	SetDlgItemFloat2(hwndSub,IDC_TEXTUREZOOMX,p->fTextureZoomX,false);
	SetDlgItemFloat2(hwndSub,IDC_TEXTUREZOOMY,p->fTextureZoomY,false);
	SetDlgItemFloat2(hwndSub,IDC_TEXTUREOFFX,p->fTextureOffX,true);
	SetDlgItemFloat2(hwndSub,IDC_TEXTUREOFFY,p->fTextureOffY,true);
	SetDlgItemFloat2(hwndSub,IDC_TEXTUREROT,p->fTextureRot,true);
	CheckButton(hwndSub,IDC_SHADOWDIRENABLE,p->iShadowDirEnable);
	SetDlgItemInt(hwndSub,IDC_SHADOWDIR,p->iShadowDir,true);
	SetDlgItemInt(hwndSub,IDC_SHADOWOFFSET,p->iShadowOffset,true);
	SetDlgItemInt(hwndSub,IDC_SHADOWDENSITY,p->iShadowDensity,true);
	SetDlgItemFloat2(hwndSub,IDC_SHADOWDIFFUSE,p->rShadowDiffuse,true);
	CheckButton(hwndSub,IDC_ISHADOWDIRENABLE,p->iIShadowDirEnable);
	SetDlgItemInt(hwndSub,IDC_ISHADOWDIR,p->iIShadowDir,true);
	SetDlgItemInt(hwndSub,IDC_ISHADOWOFFSET,p->iIShadowOffset,true);
	SetDlgItemInt(hwndSub,IDC_ISHADOWDENSITY,p->iIShadowDensity,true);
	SetDlgItemFloat2(hwndSub,IDC_ISHADOWDIFFUSE,p->rIShadowDiffuse,true);

	SetDlgItemText(hwndSub,IDC_TEXT,p->strText);
	SetDlgItemFloat3(hwndSub,IDC_FONTSIZE,p->fFontSize,true);
	SetDlgItemFloat3(hwndSub,IDC_FONTASPECT,p->fFontAspect,true);
	SetDlgItemFloat3(hwndSub,IDC_PATHOFFSET,p->fPathOffset,true);
	SetDlgItemFloat3(hwndSub,IDC_FONTSPACING,p->fFontSpacing,true);
	SendDlgItemMessage(hwndSub,IDC_FONTSIZEUNIT,CB_SETCURSEL,(WPARAM)p->iFontSizeUnit,0);
	CheckButton(hwndSub,IDC_AUTOSIZE,p->iAutoSize);
	CheckButton(hwndSub,IDC_SMALLFONTOPT,p->iSmallFontOpt);
	CheckButton(hwndSub,IDC_BOLD,p->iBold);
	CheckButton(hwndSub,IDC_ITALIC,p->iItalic);
	CheckButton(hwndSub,IDC_FIX,p->iFix);
	CheckButton(hwndSub,IDC_USETEXTPATH,p->iUseTextPath);
	CheckButton(hwndSub,IDC_KEEPDIR,p->iKeepDir);
	if(p->prType==prtText) {
		i=SendDlgItemMessage(hwndSub,IDC_FONT,CB_FINDSTRINGEXACT,(WPARAM)-1,(LPARAM)p->strFont);
		SendDlgItemMessage(hwndSub,IDC_FONT,CB_SETCURSEL,(WPARAM)i,0);
		if(tipFont)
			tipFont->UpdateText(p->strFont);
		if(tipText)
			tipText->UpdateText(p->strText);
	}
	SetEnabledParam();
	p->iBmpDirty=iD;
	p->iBmpBaseDirty=iBD;
	p->iBmpPrevDirty=iPD;
	if(theApp->pal->hwnd) {
		if(bAdjust)
			theApp->pal->Set(p);
	}
	p->bInitializing=false;
}
void Prop::Scroll(int n,int m) {
	SCROLLINFO si;
	RECT rc;
	POINT pt;
	int ysize;
	GetWindowRect(hwndSub,&rc);
	ysize=rc.bottom-rc.top;
	si.cbSize=sizeof(si);
	si.fMask=SIF_POS|SIF_PAGE;
	GetScrollInfo(hwndFrame,SB_VERT,&si);
	if(m)
		iOffsetY=n;
	else
		iOffsetY+=n;
	iOffsetY=min(100-(int)si.nPage,max(0,iOffsetY));
	si.cbSize=sizeof(si);
	si.fMask=SIF_POS;
	si.nPos=iOffsetY;
	SetScrollInfo(hwndFrame,SB_VERT,&si,true);
	SetWindowPos(hwndSub,0,0,-ysize*iOffsetY/100,0,0,SWP_NOZORDER|SWP_NOSIZE);
	GetWindowRect(hwndSub,&rc);
	pt.x=rc.right,pt.y=rc.bottom;
	ScreenToClient(hwndFrame,&pt);
}
LRESULT CALLBACK Prop::dlgprocPropFrame(HWND hdlg, UINT message, WPARAM wparam, LPARAM lparam) {
	int x,y;
	Prop *prop=(Prop*)(LONG_PTR)GetWindowLongPtr(hdlg,GWLP_USERDATA);
	Primitive *pr;
	prim pmOld,pmNew;
	switch (message) {
	case WM_INITDIALOG:
		ShowWindow(GetDlgItem(hdlg,IDC_PAGEPRIM),SW_HIDE);
		ShowWindow(GetDlgItem(hdlg,IDC_PAGEANIM),SW_HIDE);
		SetFocus(0);
		return FALSE;
	case WM_MOUSEWHEEL:
		SendMessage(theScreen->hwnd,WM_MOUSEWHEEL,wparam,lparam);
		break;
	case WM_SIZE:
		x=LOWORD(lparam);
		y=HIWORD(lparam);
		if(theProp) {
			theProp->Size(x,y);
		}
		break;
	case WM_VSCROLL:
		switch(LOWORD(wparam)) {
		case SB_THUMBTRACK:
			prop->Scroll(HIWORD(wparam),1);
			break;
		case SB_LINEDOWN:
			prop->Scroll(1,0);
			break;
		case SB_PAGEDOWN:
			prop->Scroll(30,0);
			break;
		case SB_LINEUP:
			prop->Scroll(-1,0);
			break;
		case SB_PAGEUP:
			prop->Scroll(-30,0);
			break;
		}
		break;
	case WM_CLOSE:
		prop->Show(0);
		theApp->CheckMenu();
		break;
	case WM_COMMAND:
		switch(LOWORD(wparam)) {
		case IDC_PAGEPRIM:
			prop->SetPage(0);
			break;
		case IDC_PAGEANIM:
			prop->SetPage(1);
			break;
		}
		switch(HIWORD(wparam)) {
		case CBN_SELCHANGE:
			if(LOWORD(wparam)==IDC_TYPE) {
				if(pr=prop->prCurrent) {
					pmOld=prop->prCurrent->prType;
					pmNew=(prim)SendDlgItemMessage(hdlg,IDC_TYPE,CB_GETCURSEL,0,0);
					if(pmOld==prtGroup||pmOld==prtShape||pmOld==prtImage||pmNew==prtGroup||pmNew==prtShape||pmNew==prtImage) {
						MessageBox(theProp->hwndFrame,L"Can't Convert from/to Group/Shape/Image",L"SkinMan",MB_OK);
						SendDlgItemMessage(hdlg,IDC_TYPE,CB_SETCURSEL,(int)pmOld,0);
					}
					else {
						pr->prType=pmNew;
						theTree->ChangeIcon(pr);
						prop->SetType(pr->prType);
						prop->Setup(pr);
						pr->CalcPath(2,1);
						theScreen->Send();
					}
				}
			}
			break;
		}
		break;
	}
	return FALSE;
}
void Prop::ChangeIntValue(int iValue,Primitive *pr,int *piVal,int iAdjustRect) {
	int *pi,iOffs;
	PrimitivePtrList *prl;
	iOffs=(int)piVal-(int)pr;
	for(prl=&theTree->prlCurrent;prl->next;prl=prl->next) {
		if(prl->bSel) {
			pi=(int*)(((BYTE*)prl->pr)+iOffs);
			if(iValue!=*pi) {
				*pi=iValue;
				prl->pr->CalcPath(2,iAdjustRect);
				theApp->Edit();
			}
		}
	}
	if(iValue!=*piVal) {
		*piVal=iValue;
		pr->CalcPath(2,iAdjustRect);
		theApp->Edit();
	}
}
void Prop::ChangeOperation(int iValue,Primitive *pr,int *piVal,int iAdjustRect) {
	int *pi,iOffs;
	PrimitivePtrList *prl;
	Primitive *p;
	iOffs=(int)piVal-(int)pr;
	for(prl=&theTree->prlCurrent;prl->next;prl=prl->next) {
		if(prl->bSel) {
			pi=(int*)(((BYTE*)prl->pr)+iOffs);
			if(iValue!=*pi) {
				*pi=iValue;
				prl->pr->CalcPath(2,iAdjustRect);
				p=theTree->GetParent(prl->pr);
				if(p)
					p->CalcPath(2,iAdjustRect);
				theApp->Edit();
			}
		}
	}
	if(iValue!=*piVal) {
		*piVal=iValue;
		pr->CalcPath(2,iAdjustRect);
		p=theTree->GetParent(pr);
		if(p)
			p->CalcPath(2,iAdjustRect);
		theApp->Edit();
	}
}
void Prop::ChangeString(wchar_t *strVal,Primitive *pr,wchar_t *strPr,int iAdjustRect) {
	int iOffs;
	wchar_t *str;
	PrimitivePtrList *prl;
	iOffs=(int)strPr-(int)pr;
	for(prl=&theTree->prlCurrent;prl->next;prl=prl->next) {
		if(prl->bSel) {
			str=(wchar_t*)(((BYTE*)prl->pr)+iOffs);
			if(wcscmp(strVal,str)!=0) {
				wcscpy(str,strVal);
				prl->pr->CalcPath(2,iAdjustRect);
				theApp->Edit();
			}
		}
	}
	if(wcscmp(strVal,strPr)!=0) {
		wcscpy(str,strVal);
		pr->CalcPath(2,iAdjustRect);
		theApp->Edit();
	}
}
void Prop::ChangeClose(int iValue,Primitive *pr,int *piVal) {
	int *pi,iOffs;
	PrimitivePtrList *prl;
	iOffs=(int)piVal-(int)pr;
	for(prl=&theTree->prlCurrent;prl->next;prl=prl->next) {
		if(prl->bSel) {
			pi=(int*)(((BYTE*)prl->pr)+iOffs);
			if(iValue!=*pi) {
				*pi=iValue;
				if(prl->pr->prType==prtShape) {
					RectF rcf;
					prl->pr->bz->GetPath(0,prl->pr->iClose,prl->pr->path,&rcf);
					prl->pr->ptfOrg[0].X=rcf.X;
					prl->pr->ptfOrg[0].Y=rcf.Y;
					prl->pr->ptfOrg[1].X=rcf.X+rcf.Width;
					prl->pr->ptfOrg[1].Y=rcf.Y+rcf.Height;
				}
				prl->pr->CalcPath(2,1);
				theApp->Edit();
			}
		}
	}
	if(iValue!=*piVal) {
		*piVal=iValue;
		if(pr->prType==prtShape) {
			RectF rcf;
			pr->bz->GetPath(0,pr->iClose,pr->path,&rcf);
			pr->ptfOrg[0].X=rcf.X;
			pr->ptfOrg[0].Y=rcf.Y;
			pr->ptfOrg[1].X=rcf.X+rcf.Width;
			pr->ptfOrg[1].Y=rcf.Y+rcf.Height;
		}
		pr->CalcPath(2,1);
		theApp->Edit();
	}
}
void Prop::CheckChange(HWND hwnd,int id,Primitive *pr,int *piVal,int iAdjustRect) {
	ChangeIntValue(IsDlgButtonChecked(hwnd,id),pr,piVal,iAdjustRect);
}
void Prop::EnChange(HWND hwnd,int id,Primitive *pr,int *piVal) {
	int iValue,*pi;
	int iOffs;
	PrimitivePtrList *prl;
	iValue=GetDlgItemInt(hwnd,id,NULL,true);
	iOffs=(int)piVal-(int)pr;
	for(prl=&theTree->prlCurrent;prl->next;prl=prl->next) {
		if(prl->bSel) {
			pi=(int*)(((BYTE*)prl->pr)+iOffs);
			if(iValue!=*pi) {
				*pi=iValue;
				prl->pr->CalcPath(2,1);
				theApp->Edit();
			}
		}
	}
	if(iValue!=*piVal) {
		*piVal=iValue;
		theApp->Edit();
		pr->CalcPath(2,1);
	}
}
void Prop::EnChangeAlpha(HWND hwnd,int id,Primitive *pr,DWORD *pdwCol) {
	int iAlpha=(GetDlgItemInt(hwnd,id,NULL,false)*255+50)/100;
	int iOffs=(int)pdwCol-(int)pr;
	DWORD *pdw;
	PrimitivePtrList *prl;
	for(prl=&theTree->prlCurrent;prl->next;prl=prl->next) {
		if(prl->bSel) {
			pdw=(DWORD*)(((BYTE*)prl->pr)+iOffs);
			if(iAlpha!=(*pdw>>24)) {
				*pdw=(iAlpha<<24)|(*pdw&0xffffff);
				prl->pr->CalcPath(2,1);
			}
		}
	}
	*pdwCol=(iAlpha<<24)|(*pdwCol&0xffffff);
	pr->CalcPath(pr->bInitializing?0:2,1);
	theApp->Edit();
}
void Prop::EnChange(HWND hwnd,int id,Primitive *pr,float *pfVal,int iAdjust) {
	float fValue,*pf;
	int iOffs;
	PrimitivePtrList *prl;
	fValue=GetDlgItemFloat(hwnd,id,NULL,true);
	iOffs=(int)pfVal-(int)pr;
	for(prl=&theTree->prlCurrent;prl->next;prl=prl->next) {
		if(prl->bSel) {
			pf=(float*)(((BYTE*)prl->pr)+iOffs);
			if(fValue!=*pf) {
				*pf=fValue;
				prl->pr->CalcPath(2,iAdjust);
				theApp->Edit();
			}
		}
	}
	if(fValue!=*pfVal) {
		*pfVal=fValue;
		theApp->Edit();
		pr->CalcPath(2,iAdjust);
	}
}
void Prop::SetEnabledParam(void) {
	BOOL b;
	if(IsDlgButtonChecked(hwndSub,IDC_ROUNDSEPARATE))
		b=TRUE;
	else
		b=FALSE;
	EnableWindow(GetDlgItem(hwndSub,IDC_ROUNDX2),b);
	EnableWindow(GetDlgItem(hwndSub,IDC_ROUNDX3),b);
	EnableWindow(GetDlgItem(hwndSub,IDC_ROUNDX4),b);
	EnableWindow(GetDlgItem(hwndSub,IDC_ROUNDY1),TRUE);
	EnableWindow(GetDlgItem(hwndSub,IDC_ROUNDY2),b);
	EnableWindow(GetDlgItem(hwndSub,IDC_ROUNDY3),b);
	EnableWindow(GetDlgItem(hwndSub,IDC_ROUNDY4),b);
	if(!IsDlgButtonChecked(hwndSub,IDC_ROUNDXYSEPARATE)) {
		EnableWindow(GetDlgItem(hwndSub,IDC_ROUNDY1),FALSE);
		EnableWindow(GetDlgItem(hwndSub,IDC_ROUNDY2),FALSE);
		EnableWindow(GetDlgItem(hwndSub,IDC_ROUNDY3),FALSE);
		EnableWindow(GetDlgItem(hwndSub,IDC_ROUNDY4),FALSE);
	}
	if(IsDlgButtonChecked(hwndSub,IDC_TEXTUREZOOMXYSEPA))
		EnableWindow(GetDlgItem(hwndSub,IDC_TEXTUREZOOMY),TRUE);
	else
		EnableWindow(GetDlgItem(hwndSub,IDC_TEXTUREZOOMY),FALSE);
}

LRESULT CALLBACK Prop::wndprocSubclassPropSub(HWND hwnd,UINT msg,WPARAM wparam,LPARAM lparam) {
	switch(msg) {
	case WM_KEYUP:
		if(wparam==VK_CONTROL) {
			switch(theTool->toolCurrent) {
			case tArrow:
				SetCursor(hcurArrow);
				break;
			}
		}
		break;
	}
	if(theProp->wndprocOrg)
		return (CallWindowProc(theProp->wndprocOrg, hwnd, msg, wparam, lparam));
	return 0;
}

LRESULT CALLBACK Prop::dlgprocPropSub(HWND hdlg, UINT message, WPARAM wparam, LPARAM lparam) {
	int i,iVal;
	wchar_t str[512];
	RECT rc;
	RectF rcf;
	Primitive *p;
	HWND hwndTemp;
	DRAWITEMSTRUCT *di;
	static Prop *prop=(Prop*)(LONG_PTR)GetWindowLongPtr(hdlg,GWLP_USERDATA);
	switch (message) {
	case WM_INITDIALOG:
		SetWindowLongPtr(hdlg,GWLP_USERDATA,(LONG)(LONG_PTR)(prop=(Prop*)lparam));
		SendDlgItemMessage(hdlg,IDC_OPERATION,CB_ADDSTRING,0,(LPARAM)L"Normal");
		SendDlgItemMessage(hdlg,IDC_OPERATION,CB_ADDSTRING,0,(LPARAM)L"Shape+");
		SendDlgItemMessage(hdlg,IDC_OPERATION,CB_ADDSTRING,0,(LPARAM)L"Shape-");
		SendDlgItemMessage(hdlg,IDC_OPERATION,CB_ADDSTRING,0,(LPARAM)L"Shape*");

		SendDlgItemMessage(hdlg,IDC_COPYCOODINATE,BM_SETIMAGE,IMAGE_BITMAP,(LPARAM)theApp->hbmpCopy);

		CBEXAdd(hdlg,IDC_ANIMMASKTYPE,0,L"Rotate");
		CBEXAdd(hdlg,IDC_ANIMMASKTYPE,1,L"Radius");
		CBEXAdd(hdlg,IDC_ANIMMASKTYPE,2,L"Horizontal");
		CBEXAdd(hdlg,IDC_ANIMMASKTYPE,3,L"Vertical");
		CBEXAdd(hdlg,IDC_ANIMMASKGRADATIONTYPE,0,L"SingleDir");
		CBEXAdd(hdlg,IDC_ANIMMASKGRADATIONTYPE,1,L"BiDir");

		Slider::Add(GetDlgItem(hdlg,IDC_XSPIN),-3000,3000,Float3);
		Slider::Add(GetDlgItem(hdlg,IDC_YSPIN),-3000,3000,Float3);
		Slider::Add(GetDlgItem(hdlg,IDC_WSPIN),-3000,3000,Float3);
		Slider::Add(GetDlgItem(hdlg,IDC_HSPIN),-3000,3000,Float3);
		Slider::Add(GetDlgItem(hdlg,IDC_ALPHA1SPIN),0,100);
		Slider::Add(GetDlgItem(hdlg,IDC_ALPHA2SPIN),0,100);
		Slider::Add(GetDlgItem(hdlg,IDC_ALPHA3SPIN),0,100);
		Slider::Add(GetDlgItem(hdlg,IDC_ALPHASPIN),0,100);
		Slider::Add(GetDlgItem(hdlg,IDC_WIDTHSPIN),1,100,Float2);
		Slider::Add(GetDlgItem(hdlg,IDC_ROUNDX1SPIN),0,1000,Float2);
		Slider::Add(GetDlgItem(hdlg,IDC_ROUNDX2SPIN),0,1000,Float2);
		Slider::Add(GetDlgItem(hdlg,IDC_ROUNDX3SPIN),0,1000,Float2);
		Slider::Add(GetDlgItem(hdlg,IDC_ROUNDX4SPIN),0,1000,Float2);
		Slider::Add(GetDlgItem(hdlg,IDC_ROUNDY1SPIN),0,1000,Float2);
		Slider::Add(GetDlgItem(hdlg,IDC_ROUNDY2SPIN),0,1000,Float2);
		Slider::Add(GetDlgItem(hdlg,IDC_ROUNDY3SPIN),0,1000,Float2);
		Slider::Add(GetDlgItem(hdlg,IDC_ROUNDY4SPIN),0,1000,Float2);
		Slider::Add(GetDlgItem(hdlg,IDC_LINEARSPIN),-100,100);
		Slider::Add(GetDlgItem(hdlg,IDC_SMOOTHERSPIN),0,100);
		Slider::Add(GetDlgItem(hdlg,IDC_ANGLESPIN),-360,360,Float3);
		Slider::Add(GetDlgItem(hdlg,IDC_SLANTSPIN),-1000,1000,Float3);
		Slider::Add(GetDlgItem(hdlg,IDC_STARTSPIN),-360,360);
		Slider::Add(GetDlgItem(hdlg,IDC_STOPSPIN),-360,360);
		Slider::Add(GetDlgItem(hdlg,IDC_VERTEXSPIN),3,64);
		Slider::Add(GetDlgItem(hdlg,IDC_TENSIONSPIN),0,150,Float2);
		Slider::Add(GetDlgItem(hdlg,IDC_TWISTSPIN),-360,360,Float2);
		Slider::Add(GetDlgItem(hdlg,IDC_STARDEPTHSPIN),-150,150);
		Slider::Add(GetDlgItem(hdlg,IDC_DIFFUSESPIN),0,100);

		Slider::Add(GetDlgItem(hdlg,IDC_SPECULARDIRSPIN),-360,360);
		Slider::Add(GetDlgItem(hdlg,IDC_SPECULAROFFSPIN),-100,100);
		Slider::Add(GetDlgItem(hdlg,IDC_SPECULARSPIN),-100,100);
		Slider::Add(GetDlgItem(hdlg,IDC_HIGHLIGHTSPIN),-100,100);
		Slider::Add(GetDlgItem(hdlg,IDC_HIGHLIGHTWIDTHSPIN),0,100);
		Slider::Add(GetDlgItem(hdlg,IDC_AMBIENTSPIN),0,100);
		Slider::Add(GetDlgItem(hdlg,IDC_SPECULARDIR2SPIN),-360,360);
		Slider::Add(GetDlgItem(hdlg,IDC_SPECULAROFF2SPIN),-100,100);
		Slider::Add(GetDlgItem(hdlg,IDC_SPECULAR2SPIN),-100,100);
		Slider::Add(GetDlgItem(hdlg,IDC_HIGHLIGHT2SPIN),-100,100);
		Slider::Add(GetDlgItem(hdlg,IDC_HIGHLIGHTWIDTH2SPIN),0,100);
		Slider::Add(GetDlgItem(hdlg,IDC_AMBIENT2SPIN),0,100);

		Slider::Add(GetDlgItem(hdlg,IDC_EMBOSSDIRSPIN),-360,360);
		Slider::Add(GetDlgItem(hdlg,IDC_EMBOSSWIDTHSPIN),0,100,Float2);
		Slider::Add(GetDlgItem(hdlg,IDC_EMBOSSDEPTHSPIN),-100,100,Float2);
		Slider::Add(GetDlgItem(hdlg,IDC_EMBOSSDEPTH2SPIN),-100,100,Float2);
		Slider::Add(GetDlgItem(hdlg,IDC_TEXTURESPIN),0,100);
		Slider::Add(GetDlgItem(hdlg,IDC_TEXTUREZOOMXSPIN),0,300,Float2);
		Slider::Add(GetDlgItem(hdlg,IDC_TEXTUREZOOMYSPIN),0,300,Float2);
		Slider::Add(GetDlgItem(hdlg,IDC_TEXTUREOFFXSPIN),-100,100,Float2);
		Slider::Add(GetDlgItem(hdlg,IDC_TEXTUREOFFYSPIN),-100,100,Float2);
		Slider::Add(GetDlgItem(hdlg,IDC_TEXTUREROTSPIN),-360,360,Float2);
		Slider::Add(GetDlgItem(hdlg,IDC_SHADOWDIRSPIN),-360,360);
		Slider::Add(GetDlgItem(hdlg,IDC_SHADOWOFFSETSPIN),-100,100);
		Slider::Add(GetDlgItem(hdlg,IDC_SHADOWDENSITYSPIN),0,100);
		Slider::Add(GetDlgItem(hdlg,IDC_SHADOWDIFFUSESPIN),0,100,Float2);
		Slider::Add(GetDlgItem(hdlg,IDC_ISHADOWDIRSPIN),-360,360);
		Slider::Add(GetDlgItem(hdlg,IDC_ISHADOWOFFSETSPIN),-100,100);
		Slider::Add(GetDlgItem(hdlg,IDC_ISHADOWDENSITYSPIN),0,100);
		Slider::Add(GetDlgItem(hdlg,IDC_ISHADOWDIFFUSESPIN),0,100,Float2);
		Slider::Add(GetDlgItem(hdlg,IDC_FONTSIZESPIN),-100,100,Float3);
		Slider::Add(GetDlgItem(hdlg,IDC_FONTASPECTSPIN),-1000,1000,Float3);
		Slider::Add(GetDlgItem(hdlg,IDC_FONTSPACINGSPIN),-100,500,Float3);
		Slider::Add(GetDlgItem(hdlg,IDC_PATHOFFSETSPIN),-100,100,Float3);
		if(prop->prCurrent) {
			CBEXAdd(hdlg,IDC_TEXTURETYPE,0,L"()");
		}
		else {
			CBEXAdd(hdlg,IDC_TEXTURETYPE,0,L"");
		}
		for(i=0;i<theTexture->iTexMax;++i) {
			wchar_t strName[320],*strExt;
			wcscpy_s(strName,320,theTexture->tex[i]->strName);
			strExt=wcsrchr(strName,'.');
			if(strExt)
				*strExt=0;
			CBEXAdd(hdlg,IDC_TEXTURETYPE,i+1,strName);
		}
		hwndTemp=GetDlgItem(hdlg,IDC_TEXTURETYPE);
		if(hwndTemp) {
			prop->tipTex=new ToolTip(hwndTemp,L"");
		}
		else
			prop->tipTex=0;
		GetClientRect(GetDlgItem(hdlg,IDC_TEXTURETYPE),&rc);
		SendDlgItemMessage(hdlg,IDC_TEXTURETYPE,CB_SETDROPPEDWIDTH,rc.right*2,0);

		SendDlgItemMessage(hdlg,IDC_SPECULARTYPE,CB_ADDSTRING,0,(LPARAM)L"None");
		SendDlgItemMessage(hdlg,IDC_SPECULARTYPE,CB_ADDSTRING,0,(LPARAM)L"Flat");
		SendDlgItemMessage(hdlg,IDC_SPECULARTYPE,CB_ADDSTRING,0,(LPARAM)L"Sphere");
		SendDlgItemMessage(hdlg,IDC_SPECULARTYPE,CB_ADDSTRING,0,(LPARAM)L"Metal");
		SendDlgItemMessage(hdlg,IDC_SPECULARTYPE2,CB_ADDSTRING,0,(LPARAM)L"None");
		SendDlgItemMessage(hdlg,IDC_SPECULARTYPE2,CB_ADDSTRING,0,(LPARAM)L"Flat");
		SendDlgItemMessage(hdlg,IDC_SPECULARTYPE2,CB_ADDSTRING,0,(LPARAM)L"Sphere");
		SendDlgItemMessage(hdlg,IDC_SPECULARTYPE2,CB_ADDSTRING,0,(LPARAM)L"Metal");

		SendDlgItemMessage(hdlg,IDC_GRADATIONTYPE,CB_ADDSTRING,0,(LPARAM)L"None");
		SendDlgItemMessage(hdlg,IDC_GRADATIONTYPE,CB_ADDSTRING,0,(LPARAM)L"Linear");
		SendDlgItemMessage(hdlg,IDC_GRADATIONTYPE,CB_ADDSTRING,0,(LPARAM)L"Symmetric");
		SendDlgItemMessage(hdlg,IDC_GRADATIONTYPE,CB_ADDSTRING,0,(LPARAM)L"Radial");

		SendDlgItemMessage(hdlg,IDC_FONTSIZEUNIT,CB_ADDSTRING,0,(LPARAM)L"Px");
		SendDlgItemMessage(hdlg,IDC_FONTSIZEUNIT,CB_ADDSTRING,0,(LPARAM)L"Pt");

		SendDlgItemMessage(hdlg,IDC_BORDERTYPE,CB_ADDSTRING,0,(LPARAM)L"External");
		SendDlgItemMessage(hdlg,IDC_BORDERTYPE,CB_ADDSTRING,0,(LPARAM)L"Internal");
		SendDlgItemMessage(hdlg,IDC_BORDERTYPE,CB_ADDSTRING,0,(LPARAM)L"Both");

		CBEXSetImageList(hdlg,IDC_BORDERJOIN,prop->himlBorderjoin);
		CBEXAdd(hdlg,IDC_BORDERJOIN,0,L"Miter");
		CBEXAdd(hdlg,IDC_BORDERJOIN,1,L"Bevel");
		CBEXAdd(hdlg,IDC_BORDERJOIN,2,L"Round");
		CBEXAdd(hdlg,IDC_LINETYPE,0,L"Radiate");
		CBEXAdd(hdlg,IDC_LINETYPE,0,L"Horizontal");
		CBEXAdd(hdlg,IDC_LINETYPE,0,L"Vertical");
		CBEXSetImageList(hdlg,IDC_LINECAP,prop->himlLineCap);
		CBEXAdd(hdlg,IDC_LINECAP,0,L"Flat");
		CBEXAdd(hdlg,IDC_LINECAP,1,L"Square");
		CBEXAdd(hdlg,IDC_LINECAP,2,L"Round");
		CBEXAdd(hdlg,IDC_LINECAP,2,L"Triangle");
		if(prop->prType==prtText) {
			prop->tipText=new ToolTip(GetDlgItem(hdlg,IDC_TEXT),L"");
			prop->tipFont=new ToolTip(GetDlgItem(hdlg,IDC_FONT),L"");
			prop->fsel.Set(hdlg,IDC_FONT);
			p=theTree->prlCurrent.pr;
			if(p) {
				i=(int)SendDlgItemMessage(hdlg,IDC_FONT,CB_FINDSTRINGEXACT,-1,(LPARAM)p->strFont);
				if(i!=CB_ERR)
					SendDlgItemMessage(hdlg,IDC_FONT,CB_SETCURSEL,(WPARAM)i,0);
			}
		}
		else {
			prop->tipText=prop->tipFont=0;
		}
		for(i=0;controltab[i].strKey;++i) {
			if(controltab[i].strDisp) {
				SetDlgItemText(hdlg,controltab[i].id,controltab[i].strDisp);
			}
		}
		return FALSE;
	case WM_LBUTTONDOWN:
		SetFocus(theProp->hwndFrame);
		break;
	case WM_DRAWITEM:
		di=(DRAWITEMSTRUCT*)lparam;
		switch(wparam) {
		case IDC_COLOR1:
		case IDC_COLOR2:
		case IDC_COLOR3:
		case IDC_SHADOWCOLOR:
		case IDC_ISHADOWCOLOR:
			UINT uiState;
			RECT rc;
			HDC hdc=di->hDC;
			HBRUSH hbr;
			DWORD dw;
			if(prop->prCurrent) {
				switch(wparam) {
				case IDC_COLOR1:
					dw=prop->prCurrent->dwColor1;
					break;
				case IDC_COLOR2:
					dw=prop->prCurrent->dwColor2;
					break;
				case IDC_COLOR3:
					dw=prop->prCurrent->dwColor3;
					break;
				case IDC_SHADOWCOLOR:
					dw=prop->prCurrent->dwShadowColor;
					break;
				case IDC_ISHADOWCOLOR:
					dw=prop->prCurrent->dwIShadowColor;
					break;
				}
				hbr=CreateSolidBrush(((dw&0xff)<<16)|(dw&0xff00)|((dw&0xff0000)>>16));
			}
			else
				hbr=CreateSolidBrush(RGB(0,0,0));
			if(di->itemState&ODS_SELECTED)
				uiState=DFCS_BUTTONPUSH | DFCS_PUSHED;
			else
				uiState=DFCS_BUTTONPUSH;
			DrawFrameControl(hdc, &di->rcItem, DFC_BUTTON, uiState);
			rc=di->rcItem;
			InflateRect(&rc,-3,-3);
			FillRect(hdc,&rc,hbr);
			InflateRect(&rc,-2,-2);
			if(di->itemState&ODS_FOCUS)
				DrawFocusRect(hdc,&rc);
			DeleteObject((HGDIOBJ)hbr);
			return TRUE;
		}
		break;
	case WM_USER_STARTEDIT:
		theJournal->Record();
		break;
	case WM_USER_ENDEDIT:
		break;
	case WM_COMMAND:
		switch(HIWORD(wparam)) {
		case BN_CLICKED:
			EnterCriticalSection(&csData);
			switch(LOWORD(wparam)) {
			case IDC_COPYCOODINATE:
				CopyCoodinate();
				break;
			case IDC_RESET:
				theJournal->Record();
				SetDlgItemInt(hdlg,IDC_W,prop->prCurrent->img->GetWidth(),false);
				SetDlgItemInt(hdlg,IDC_H,prop->prCurrent->img->GetHeight(),false);
				SendMessage(hdlg,WM_COMMAND,MAKELONG(IDC_W,EN_CHANGE),0);
				SendMessage(hdlg,WM_COMMAND,MAKELONG(IDC_H,EN_CHANGE),0);
				break;
			case IDC_COLOR1:
				prop->prCurrent->SetColor(0);
				break;
			case IDC_COLOR2:
				prop->prCurrent->SetColor(1);
				break;
			case IDC_COLOR3:
				prop->prCurrent->SetColor(2);
				break;
			case IDC_SHADOWCOLOR:
				prop->prCurrent->SetColor(3);
				break;
			case IDC_ISHADOWCOLOR:
				prop->prCurrent->SetColor(4);
				break;
			case IDC_BOLD:
				theJournal->Record();
				prop->CheckChange(hdlg,IDC_BOLD,prop->prCurrent,&prop->prCurrent->iBold,0);
				break;
			case IDC_ITALIC:
				theJournal->Record();
				prop->CheckChange(hdlg,IDC_ITALIC,prop->prCurrent,&prop->prCurrent->iItalic,0);
				break;
			case IDC_AUTOSIZE:
				theJournal->Record();
				prop->CheckChange(hdlg,IDC_AUTOSIZE,prop->prCurrent,&prop->prCurrent->iAutoSize);
				break;
			case IDC_SMALLFONTOPT:
				theJournal->Record();
				prop->CheckChange(hdlg,IDC_SMALLFONTOPT,prop->prCurrent,&prop->prCurrent->iSmallFontOpt,0);
				break;
			case IDC_FIX:
				theJournal->Record();
				prop->CheckChange(hdlg,IDC_FIX,prop->prCurrent,&prop->prCurrent->iFix);
				break;
			case IDC_USETEXTPATH:
				theJournal->Record();
				prop->CheckChange(hdlg,IDC_USETEXTPATH,prop->prCurrent,&prop->prCurrent->iUseTextPath);
				break;
			case IDC_KEEPDIR:
				theJournal->Record();
				prop->CheckChange(hdlg,IDC_KEEPDIR,prop->prCurrent,&prop->prCurrent->iKeepDir);
				break;
			case IDC_FILL:
				theJournal->Record();
				prop->CheckChange(hdlg,IDC_FILL,prop->prCurrent,&prop->prCurrent->iFill);
				break;
			case IDC_BORDER:
				theJournal->Record();
				prop->CheckChange(hdlg,IDC_BORDER,prop->prCurrent,&prop->prCurrent->iBorder);
				break;
			case IDC_BORDERASFILL:
				theJournal->Record();
				prop->CheckChange(hdlg,IDC_BORDERASFILL,prop->prCurrent,&prop->prCurrent->iBorderAsFill);
				break;
			case IDC_PIE:
				theJournal->Record();
				prop->CheckChange(hdlg,IDC_PIE,prop->prCurrent,&prop->prCurrent->iPie);
				break;
			case IDC_ANTIALIAS:
				theJournal->Record();
				prop->CheckChange(hdlg,IDC_ANTIALIAS,prop->prCurrent,&prop->prCurrent->iAntialias);
				break;
			case IDC_ROUNDSEPARATE:
				theJournal->Record();
				prop->CheckChange(hdlg,IDC_ROUNDSEPARATE,prop->prCurrent,&prop->prCurrent->iRoundSeparate);
				prop->SetEnabledParam();
				break;
			case IDC_ROUNDXYSEPARATE:
				theJournal->Record();
				prop->CheckChange(hdlg,IDC_ROUNDXYSEPARATE,prop->prCurrent,&prop->prCurrent->iRoundXYSeparate);
				prop->SetEnabledParam();
				break;
			case IDC_CORNERC1:
				theJournal->Record();
				iVal=(prop->prCurrent->iCornerC1+1)&3;
				prop->ChangeIntValue(iVal,prop->prCurrent,&prop->prCurrent->iCornerC1);
				SetDlgItemText(hdlg,IDC_CORNERC1,CornerStr(prop->prCurrent->iCornerC1));
				break;
			case IDC_CORNERC2:
				theJournal->Record();
				iVal=(prop->prCurrent->iCornerC2+1)&3;
				prop->ChangeIntValue(iVal,prop->prCurrent,&prop->prCurrent->iCornerC2);
				SetDlgItemText(hdlg,IDC_CORNERC2,CornerStr(prop->prCurrent->iCornerC2));
				break;
			case IDC_CORNERC3:
				theJournal->Record();
				iVal=(prop->prCurrent->iCornerC3+1)&3;
				prop->ChangeIntValue(iVal,prop->prCurrent,&prop->prCurrent->iCornerC3);
				SetDlgItemText(hdlg,IDC_CORNERC3,CornerStr(prop->prCurrent->iCornerC3));
				break;
			case IDC_CORNERC4:
				theJournal->Record();
				iVal=(prop->prCurrent->iCornerC4+1)&3;
				prop->ChangeIntValue(iVal,prop->prCurrent,&prop->prCurrent->iCornerC4);
				SetDlgItemText(hdlg,IDC_CORNERC4,CornerStr(prop->prCurrent->iCornerC4));
				break;
			case IDC_CLOSE:
				theJournal->Record();
				iVal=IsDlgButtonChecked(hdlg,IDC_CLOSE);
				prop->ChangeClose(iVal,prop->prCurrent,&prop->prCurrent->iClose);
				break;
			case IDC_TEXTUREALPHA:
				theJournal->Record();
				prop->CheckChange(hdlg,IDC_TEXTUREALPHA,prop->prCurrent,&prop->prCurrent->iUseTextureAlpha);
				break;
			case IDC_TEXTUREZOOMXYSEPA:
				theJournal->Record();
				prop->CheckChange(hdlg,IDC_TEXTUREZOOMXYSEPA,prop->prCurrent,&prop->prCurrent->iTextureZoomXYSepa);
				prop->SetEnabledParam();
				break;
			case IDC_SHADOWDIRENABLE:
				theJournal->Record();
				prop->CheckChange(hdlg,IDC_SHADOWDIRENABLE,prop->prCurrent,&prop->prCurrent->iShadowDirEnable);
				break;
			case IDC_ISHADOWDIRENABLE:
				theJournal->Record();
				prop->CheckChange(hdlg,IDC_ISHADOWDIRENABLE,prop->prCurrent,&prop->prCurrent->iIShadowDirEnable);
				break;
			case IDC_EMBOSSDIRENABLE:
				theJournal->Record();
				prop->CheckChange(hdlg,IDC_EMBOSSDIRENABLE,prop->prCurrent,&prop->prCurrent->iEmbossDirEnable);
				break;
			}
			theScreen->Send();
			LeaveCriticalSection(&csData);
			break;
		case CBN_SELCHANGE:
			if(prop&&prop->prCurrent) {
				switch(LOWORD(wparam))  {
				case IDC_FONTSIZEUNIT:
					theJournal->Record();
					EnterCriticalSection(&csData);
					prop->ChangeIntValue((int)SendDlgItemMessage(hdlg,IDC_FONTSIZEUNIT,CB_GETCURSEL,0,0),prop->prCurrent,&prop->prCurrent->iFontSizeUnit,0);
					theScreen->Send();
					LeaveCriticalSection(&csData);
					break;
				case IDC_SPECULARTYPE:
					theJournal->Record();
					EnterCriticalSection(&csData);
					prop->ChangeIntValue((int)SendDlgItemMessage(hdlg,IDC_SPECULARTYPE,CB_GETCURSEL,0,0),prop->prCurrent,&prop->prCurrent->light[0].iSpecularType);
					theScreen->Send();
					LeaveCriticalSection(&csData);
					break;
				case IDC_SPECULARTYPE2:
					theJournal->Record();
					EnterCriticalSection(&csData);
					prop->ChangeIntValue((int)SendDlgItemMessage(hdlg,IDC_SPECULARTYPE2,CB_GETCURSEL,0,0),prop->prCurrent,&prop->prCurrent->light[1].iSpecularType);
					theScreen->Send();
					LeaveCriticalSection(&csData);
					break;
				case IDC_TEXTURETYPE:
					theJournal->Record();
					EnterCriticalSection(&csData);
					prop->ChangeIntValue((int)SendDlgItemMessage(hdlg,IDC_TEXTURETYPE,CB_GETCURSEL,0,0)-1,prop->prCurrent,&prop->prCurrent->iTextureType);
					theScreen->Send();
					LeaveCriticalSection(&csData);
					memset(str,0,sizeof str);
					CBEXGetText(hdlg,IDC_TEXTURETYPE,prop->prCurrent->iTextureType+1,str,512);
					prop->tipTex->UpdateText(str);
					break;
				case IDC_GRADATIONTYPE:
					theJournal->Record();
					EnterCriticalSection(&csData);
					prop->ChangeIntValue((int)SendDlgItemMessage(hdlg,IDC_GRADATIONTYPE,CB_GETCURSEL,0,0),prop->prCurrent,&prop->prCurrent->iGradationType);
					theScreen->Send();
					LeaveCriticalSection(&csData);
					break;
				case IDC_BORDERTYPE:
					theJournal->Record();
					EnterCriticalSection(&csData);
					prop->ChangeIntValue((int)SendDlgItemMessage(hdlg,IDC_BORDERTYPE,CB_GETCURSEL,0,0),prop->prCurrent,&prop->prCurrent->iBorderType);
					theScreen->Send();
					LeaveCriticalSection(&csData);
					break;
				case IDC_BORDERJOIN:
					theJournal->Record();
					EnterCriticalSection(&csData);
					prop->ChangeIntValue((int)SendDlgItemMessage(hdlg,IDC_BORDERJOIN,CB_GETCURSEL,0,0),prop->prCurrent,&prop->prCurrent->iBorderJoin);
					theScreen->Send();
					LeaveCriticalSection(&csData);
					break;
				case IDC_LINECAP:
					theJournal->Record();
					EnterCriticalSection(&csData);
					prop->ChangeIntValue((int)SendDlgItemMessage(hdlg,IDC_LINECAP,CB_GETCURSEL,0,0),prop->prCurrent,&prop->prCurrent->iLineCap);
					theScreen->Send();
					LeaveCriticalSection(&csData);
					break;
				case IDC_LINETYPE:
					theJournal->Record();
					EnterCriticalSection(&csData);
					prop->ChangeIntValue((int)SendDlgItemMessage(hdlg,IDC_LINETYPE,CB_GETCURSEL,0,0),prop->prCurrent,&prop->prCurrent->iLineType);
					theScreen->Send();
					LeaveCriticalSection(&csData);
					break;
				case IDC_OPERATION:
					theJournal->Record();
					prop->ChangeOperation((int)SendDlgItemMessage(hdlg,IDC_OPERATION,CB_GETCURSEL,0,0),prop->prCurrent,&prop->prCurrent->iOperation);
//					theApp->Edit();
//					prop->prCurrent->iOperation=(int)SendDlgItemMessage(hdlg,IDC_OPERATION,CB_GETCURSEL,0,0);
//					prop->prCurrent->CalcPath(2,1);
//					p=theTree->GetParent(prop->prCurrent);
//					if(p)
//						p->CalcPath(2,1);
					theScreen->Send();
					break;
				case IDC_FONT:
					theJournal->Record();
//					theApp->Edit();
					i=(int)SendDlgItemMessage(hdlg,IDC_FONT,CB_GETCURSEL,0,0);
					SendDlgItemMessage(hdlg,IDC_FONT,CB_GETLBTEXT,(WPARAM)i,(LPARAM)str);
					prop->ChangeString(str,prop->prCurrent,prop->prCurrent->strFont,0);
					theScreen->Send();
					prop->tipFont->UpdateText(prop->prCurrent->strFont);
					break;
				}
			}
			break;
		case EN_CHANGE:
			if(prop&&prop->prCurrent) {
				float fX;
				if(prop->prCurrent->bInitializing)
					break;
				theScreen->fRenderBreak=true;
				EnterCriticalSection(&csData);
				switch(LOWORD(wparam)) {
/*
				case IDC_FRAMEMASKSTART:
					prop->EnChange(prop->hwndAnim,IDC_FRAMEMASKSTART,prop->prCurrent,&prop->prCurrent->fAnimFrameMaskStart);
					break;
				case IDC_FRAMEMASKSTOP:
					prop->EnChange(prop->hwndAnim,IDC_FRAMEMASKSTOP,prop->prCurrent,&prop->prCurrent->fAnimFrameMaskStop);
					break;
				case IDC_ANIMMASKGRADATION:
					prop->EnChange(prop->hwndAnim,IDC_ANIMMASKGRADATION,prop->prCurrent,&prop->prCurrent->fAnimMaskGradation);
					break;
				case IDC_ANIMMASKSTARTFROM:
					prop->EnChange(prop->hwndAnim,IDC_ANIMMASKSTARTFROM,prop->prCurrent,&prop->prCurrent->fAnimMaskStartFrom);
					break;
				case IDC_ANIMMASKSTARTTO:
					prop->EnChange(prop->hwndAnim,IDC_ANIMMASKSTARTTO,prop->prCurrent,&prop->prCurrent->fAnimMaskStartTo);
					break;
				case IDC_ANIMMASKSTOPFROM:
					prop->EnChange(prop->hwndAnim,IDC_ANIMMASKSTOPFROM,prop->prCurrent,&prop->prCurrent->fAnimMaskStopFrom);
					break;
				case IDC_ANIMMASKSTOPTO:
					prop->EnChange(prop->hwndAnim,IDC_ANIMMASKSTOPTO,prop->prCurrent,&prop->prCurrent->fAnimMaskStopTo);
					break;
				case IDC_ANIMOFFXFROM:
					prop->EnChange(prop->hwndAnim,IDC_ANIMOFFXFROM,prop->prCurrent,&prop->prCurrent->fAnimOffXFrom);
					break;
				case IDC_ANIMOFFXTO:
					prop->EnChange(prop->hwndAnim,IDC_ANIMOFFXTO,prop->prCurrent,&prop->prCurrent->fAnimOffXTo);
					break;
				case IDC_ANIMOFFYFROM:
					prop->EnChange(prop->hwndAnim,IDC_ANIMOFFYFROM,prop->prCurrent,&prop->prCurrent->fAnimOffYFrom);
					break;
				case IDC_ANIMOFFYTO:
					prop->EnChange(prop->hwndAnim,IDC_ANIMOFFYTO,prop->prCurrent,&prop->prCurrent->fAnimOffYTo);
					break;
				case IDC_ANIMROTCENTERX:
					prop->EnChange(prop->hwndAnim,IDC_ANIMROTCENTERX,prop->prCurrent,&prop->prCurrent->fAnimRotCenterX);
					break;
				case IDC_ANIMROTCENTERY:
					prop->EnChange(prop->hwndAnim,IDC_ANIMROTCENTERY,prop->prCurrent,&prop->prCurrent->fAnimRotCenterY);
					break;
				case IDC_ANIMANGLEFROM:
					prop->EnChange(prop->hwndAnim,IDC_ANIMANGLEFROM,prop->prCurrent,&prop->prCurrent->fAnimAngleFrom);
					break;
				case IDC_ANIMANGLETO:
					prop->EnChange(prop->hwndAnim,IDC_ANIMANGLETO,prop->prCurrent,&prop->prCurrent->fAnimAngleTo);
					break;
				case IDC_ANIMZOOMFROM:
					prop->EnChange(prop->hwndAnim,IDC_ANIMZOOMFROM,prop->prCurrent,&prop->prCurrent->fAnimZoomFrom);
					break;
				case IDC_ANIMZOOMTO:
					prop->EnChange(prop->hwndAnim,IDC_ANIMZOOMTO,prop->prCurrent,&prop->prCurrent->fAnimZoomTo);
					break;
				case IDC_ANIMALPHAFROM:
					prop->EnChange(prop->hwndAnim,IDC_ANIMALPHAFROM,prop->prCurrent,&prop->prCurrent->fAnimAlphaFrom);
					break;
				case IDC_ANIMALPHATO:
					prop->EnChange(prop->hwndAnim,IDC_ANIMALPHATO,prop->prCurrent,&prop->prCurrent->fAnimAlphaTo);
					break;
				case IDC_ANIMBRIGHTNESSFROM:
					prop->EnChange(prop->hwndAnim,IDC_ANIMBRIGHTNESSFROM,prop->prCurrent,&prop->prCurrent->fAnimBrightnessFrom);
					break;
				case IDC_ANIMBRIGHTNESSTO:
					prop->EnChange(prop->hwndAnim,IDC_ANIMBRIGHTNESSTO,prop->prCurrent,&prop->prCurrent->fAnimBrightnessTo);
					break;
				case IDC_ANIMCONTRASTFROM:
					prop->EnChange(prop->hwndAnim,IDC_ANIMCONTRASTFROM,prop->prCurrent,&prop->prCurrent->fAnimContrastFrom);
					break;
				case IDC_ANIMCONTRASTTO:
					prop->EnChange(prop->hwndAnim,IDC_ANIMCONTRASTTO,prop->prCurrent,&prop->prCurrent->fAnimContrastTo);
					break;
				case IDC_ANIMSATURATIONFROM:
					prop->EnChange(prop->hwndAnim,IDC_ANIMSATURATIONFROM,prop->prCurrent,&prop->prCurrent->fAnimSaturationFrom);
					break;
				case IDC_ANIMSATURATIONTO:
					prop->EnChange(prop->hwndAnim,IDC_ANIMSATURATIONTO,prop->prCurrent,&prop->prCurrent->fAnimSaturationTo);
					break;
				case IDC_ANIMHUEFROM:
					prop->EnChange(prop->hwndAnim,IDC_ANIMHUEFROM,prop->prCurrent,&prop->prCurrent->fAnimHueFrom);
					break;
				case IDC_ANIMHUETO:
					prop->EnChange(prop->hwndAnim,IDC_ANIMHUETO,prop->prCurrent,&prop->prCurrent->fAnimHueTo);
					break;
*/
				case IDC_X:
					fX=GetDlgItemFloat(hdlg,IDC_X,NULL,true);
					if(fX!=prop->prCurrent->ptfOrg[0].X) {
						theTree->SetPos(prop->prCurrent,fX,prop->prCurrent->ptfOrg[0].Y);
						theApp->Edit();
					}
					break;
				case IDC_Y:
					fX=GetDlgItemFloat(hdlg,IDC_Y,NULL,true);
					if(fX!=prop->prCurrent->ptfOrg[0].Y) {
						theTree->SetPos(prop->prCurrent,prop->prCurrent->ptfOrg[0].X,fX);
						theApp->Edit();
					}
					break;
				case IDC_W:
					fX=GetDlgItemFloat(hdlg,IDC_W,NULL,true);
					if(fX!=prop->prCurrent->ptfOrg[1].X-prop->prCurrent->ptfOrg[0].X) {
						prop->prCurrent->ptfOrg[1].X=prop->prCurrent->ptfOrg[0].X+fX;
						prop->prCurrent->SetPos(prop->prCurrent->ptfOrg[0].X,prop->prCurrent->ptfOrg[0].Y
							,prop->prCurrent->ptfOrg[1].X-prop->prCurrent->ptfOrg[0].X
							,prop->prCurrent->ptfOrg[1].Y-prop->prCurrent->ptfOrg[0].Y,1);
						theApp->Edit();
					}
					break;
				case IDC_H:
					fX=GetDlgItemFloat(hdlg,IDC_H,NULL,true);
					if(fX!=prop->prCurrent->ptfOrg[1].Y-prop->prCurrent->ptfOrg[0].Y) {
						prop->prCurrent->ptfOrg[1].Y=prop->prCurrent->ptfOrg[0].Y+fX;
						prop->prCurrent->SetPos(prop->prCurrent->ptfOrg[0].X,prop->prCurrent->ptfOrg[0].Y
							,prop->prCurrent->ptfOrg[1].X-prop->prCurrent->ptfOrg[0].X
							,prop->prCurrent->ptfOrg[1].Y-prop->prCurrent->ptfOrg[0].Y,1);
						theApp->Edit();
					}
					break;
				case IDC_ALPHA1:
					prop->EnChangeAlpha(hdlg,IDC_ALPHA1,prop->prCurrent,&prop->prCurrent->dwColor1);
					break;
				case IDC_ALPHA2:
					prop->EnChangeAlpha(hdlg,IDC_ALPHA2,prop->prCurrent,&prop->prCurrent->dwColor2);
					break;
				case IDC_ALPHA3:
					prop->EnChangeAlpha(hdlg,IDC_ALPHA3,prop->prCurrent,&prop->prCurrent->dwColor3);
					break;
				case IDC_WIDTH:
					prop->EnChange(prop->hwndSub,IDC_WIDTH,prop->prCurrent,&prop->prCurrent->fWidth);
					break;
				case IDC_ALPHA:
					prop->EnChange(prop->hwndSub,IDC_ALPHA,prop->prCurrent,&prop->prCurrent->iAlpha);
					break;
				case IDC_ANGLE:
					prop->EnChange(prop->hwndSub,IDC_ANGLE,prop->prCurrent,&prop->prCurrent->rAngle,1);
					break;
				case IDC_SLANT:
					prop->EnChange(prop->hwndSub,IDC_SLANT,prop->prCurrent,&prop->prCurrent->fSlant,1);
					break;
				case IDC_LINEAR:
					prop->EnChange(prop->hwndSub,IDC_LINEAR,prop->prCurrent,&prop->prCurrent->iLinear);
					break;
				case IDC_SMOOTHER:
					prop->EnChange(prop->hwndSub,IDC_SMOOTHER,prop->prCurrent,&prop->prCurrent->iSmoother);
					break;
				case IDC_START:
					prop->EnChange(prop->hwndSub,IDC_START,prop->prCurrent,&prop->prCurrent->iStart);
					break;
				case IDC_STOP:
					prop->EnChange(prop->hwndSub,IDC_STOP,prop->prCurrent,&prop->prCurrent->iStop);
					break;
				case IDC_VERTEX:
					prop->EnChange(prop->hwndSub,IDC_VERTEX,prop->prCurrent,&prop->prCurrent->iVertex);
					break;
				case IDC_TENSION:
					prop->EnChange(prop->hwndSub,IDC_TENSION,prop->prCurrent,&prop->prCurrent->rTension);
					break;
				case IDC_TWIST:
					prop->EnChange(prop->hwndSub,IDC_TWIST,prop->prCurrent,&prop->prCurrent->rTwist);
					break;
				case IDC_STARDEPTH:
					prop->EnChange(prop->hwndSub,IDC_STARDEPTH,prop->prCurrent,&prop->prCurrent->iStarDepth);
					break;
				case IDC_ROUNDX1:
					prop->EnChange(prop->hwndSub,IDC_ROUNDX1,prop->prCurrent,&prop->prCurrent->fRoundX1);
					if(prop->prCurrent->iRoundXYSeparate==0) {
						SetDlgItemFloat2(hdlg,IDC_ROUNDY1,prop->prCurrent->fRoundX1,false);
						prop->EnChange(prop->hwndSub,IDC_ROUNDY1,prop->prCurrent,&prop->prCurrent->fRoundY1);
					}
					if(prop->prCurrent->iRoundSeparate==0) {
						SetDlgItemFloat2(hdlg,IDC_ROUNDX2,prop->prCurrent->fRoundX1,false);
						SetDlgItemFloat2(hdlg,IDC_ROUNDX3,prop->prCurrent->fRoundX1,false);
						SetDlgItemFloat2(hdlg,IDC_ROUNDX4,prop->prCurrent->fRoundX1,false);
						prop->EnChange(prop->hwndSub,IDC_ROUNDX2,prop->prCurrent,&prop->prCurrent->fRoundX2);
						prop->EnChange(prop->hwndSub,IDC_ROUNDX3,prop->prCurrent,&prop->prCurrent->fRoundX3);
						prop->EnChange(prop->hwndSub,IDC_ROUNDX4,prop->prCurrent,&prop->prCurrent->fRoundX4);
						if(prop->prCurrent->iRoundXYSeparate==0) {
							SetDlgItemFloat2(hdlg,IDC_ROUNDY2,prop->prCurrent->fRoundX1,false);
							SetDlgItemFloat2(hdlg,IDC_ROUNDY3,prop->prCurrent->fRoundX1,false);
							SetDlgItemFloat2(hdlg,IDC_ROUNDY4,prop->prCurrent->fRoundX1,false);
							prop->EnChange(prop->hwndSub,IDC_ROUNDY2,prop->prCurrent,&prop->prCurrent->fRoundY2);
							prop->EnChange(prop->hwndSub,IDC_ROUNDY3,prop->prCurrent,&prop->prCurrent->fRoundY3);
							prop->EnChange(prop->hwndSub,IDC_ROUNDY4,prop->prCurrent,&prop->prCurrent->fRoundY4);
						}
					}
					break;
				case IDC_ROUNDX2:
					prop->EnChange(prop->hwndSub,IDC_ROUNDX2,prop->prCurrent,&prop->prCurrent->fRoundX2);
					if(prop->prCurrent->iRoundXYSeparate==0) {
						SetDlgItemFloat2(hdlg,IDC_ROUNDY2,prop->prCurrent->fRoundX2,false);
						prop->EnChange(prop->hwndSub,IDC_ROUNDY2,prop->prCurrent,&prop->prCurrent->fRoundY2);
					}
					break;
				case IDC_ROUNDX3:
					prop->EnChange(prop->hwndSub,IDC_ROUNDX3,prop->prCurrent,&prop->prCurrent->fRoundX3);
					if(prop->prCurrent->iRoundXYSeparate==0) {
						SetDlgItemFloat2(hdlg,IDC_ROUNDY3,prop->prCurrent->fRoundX3,false);
						prop->EnChange(prop->hwndSub,IDC_ROUNDY3,prop->prCurrent,&prop->prCurrent->fRoundY3);
					}
					break;
				case IDC_ROUNDX4:
					prop->EnChange(prop->hwndSub,IDC_ROUNDX4,prop->prCurrent,&prop->prCurrent->fRoundX4);
					if(prop->prCurrent->iRoundXYSeparate==0) {
						SetDlgItemFloat2(hdlg,IDC_ROUNDY4,prop->prCurrent->fRoundX4,false);
						prop->EnChange(prop->hwndSub,IDC_ROUNDY4,prop->prCurrent,&prop->prCurrent->fRoundY4);
					}
					break;
				case IDC_ROUNDY1:
					prop->EnChange(prop->hwndSub,IDC_ROUNDY1,prop->prCurrent,&prop->prCurrent->fRoundY1);
					if(prop->prCurrent->iRoundSeparate==0) {
						SetDlgItemFloat2(hdlg,IDC_ROUNDY2,prop->prCurrent->fRoundY1,false);
						prop->EnChange(prop->hwndSub,IDC_ROUNDY2,prop->prCurrent,&prop->prCurrent->fRoundY2);
						SetDlgItemFloat2(hdlg,IDC_ROUNDY3,prop->prCurrent->fRoundY1,false);
						prop->EnChange(prop->hwndSub,IDC_ROUNDY3,prop->prCurrent,&prop->prCurrent->fRoundY3);
						SetDlgItemFloat2(hdlg,IDC_ROUNDY4,prop->prCurrent->fRoundY1,false);
						prop->EnChange(prop->hwndSub,IDC_ROUNDY4,prop->prCurrent,&prop->prCurrent->fRoundY4);
					}
					break;
				case IDC_ROUNDY2:
					prop->EnChange(prop->hwndSub,IDC_ROUNDY2,prop->prCurrent,&prop->prCurrent->fRoundY2);
					break;
				case IDC_ROUNDY3:
					prop->EnChange(prop->hwndSub,IDC_ROUNDY3,prop->prCurrent,&prop->prCurrent->fRoundY3);
					break;
				case IDC_ROUNDY4:
					prop->EnChange(prop->hwndSub,IDC_ROUNDY4,prop->prCurrent,&prop->prCurrent->fRoundY4);
					break;
				case IDC_DIFFUSE:
					prop->EnChange(prop->hwndSub,IDC_DIFFUSE,prop->prCurrent,&prop->prCurrent->iDiffuse);
					break;
				case IDC_SPECULARDIR:
					prop->EnChange(prop->hwndSub,IDC_SPECULARDIR,prop->prCurrent,&prop->prCurrent->light[0].iSpecularDir);
					break;
				case IDC_SPECULARDIR2:
					prop->EnChange(prop->hwndSub,IDC_SPECULARDIR2,prop->prCurrent,&prop->prCurrent->light[1].iSpecularDir);
					break;
				case IDC_SPECULAROFF:
					prop->EnChange(prop->hwndSub,IDC_SPECULAROFF,prop->prCurrent,&prop->prCurrent->light[0].iSpecularOffset);
					break;
				case IDC_SPECULAROFF2:
					prop->EnChange(prop->hwndSub,IDC_SPECULAROFF2,prop->prCurrent,&prop->prCurrent->light[1].iSpecularOffset);
					break;
				case IDC_SPECULAR:
					prop->EnChange(prop->hwndSub,IDC_SPECULAR,prop->prCurrent,&prop->prCurrent->light[0].iSpecular);
					break;
				case IDC_SPECULAR2:
					prop->EnChange(prop->hwndSub,IDC_SPECULAR2,prop->prCurrent,&prop->prCurrent->light[1].iSpecular);
					break;
				case IDC_HIGHLIGHT:
					prop->EnChange(prop->hwndSub,IDC_HIGHLIGHT,prop->prCurrent,&prop->prCurrent->light[0].iHighlight);
					break;
				case IDC_HIGHLIGHT2:
					prop->EnChange(prop->hwndSub,IDC_HIGHLIGHT2,prop->prCurrent,&prop->prCurrent->light[1].iHighlight);
					break;
				case IDC_HIGHLIGHTWIDTH:
					prop->EnChange(prop->hwndSub,IDC_HIGHLIGHTWIDTH,prop->prCurrent,&prop->prCurrent->light[0].iHighlightWidth);
					break;
				case IDC_HIGHLIGHTWIDTH2:
					prop->EnChange(prop->hwndSub,IDC_HIGHLIGHTWIDTH2,prop->prCurrent,&prop->prCurrent->light[1].iHighlightWidth);
					break;
				case IDC_AMBIENT:
					prop->EnChange(prop->hwndSub,IDC_AMBIENT,prop->prCurrent,&prop->prCurrent->light[0].iAmbient);
					break;
				case IDC_AMBIENT2:
					prop->EnChange(prop->hwndSub,IDC_AMBIENT2,prop->prCurrent,&prop->prCurrent->light[1].iAmbient);
					break;
				case IDC_EMBOSSDIR:
					prop->EnChange(prop->hwndSub,IDC_EMBOSSDIR,prop->prCurrent,&prop->prCurrent->iEmbossDir);
					break;
				case IDC_EMBOSSWIDTH:
					prop->EnChange(prop->hwndSub,IDC_EMBOSSWIDTH,prop->prCurrent,&prop->prCurrent->rEmbossWidth);
					break;
				case IDC_EMBOSSDEPTH:
					prop->EnChange(prop->hwndSub,IDC_EMBOSSDEPTH,prop->prCurrent,&prop->prCurrent->fEmbossDepth);
					break;
				case IDC_EMBOSSDEPTH2:
					prop->EnChange(prop->hwndSub,IDC_EMBOSSDEPTH2,prop->prCurrent,&prop->prCurrent->fEmbossDepth2);
					break;
				case IDC_TEXTURE:
					prop->EnChange(prop->hwndSub,IDC_TEXTURE,prop->prCurrent,&prop->prCurrent->iTexture);
					break;
				case IDC_TEXTUREZOOMX:
					prop->EnChange(prop->hwndSub,IDC_TEXTUREZOOMX,prop->prCurrent,&prop->prCurrent->fTextureZoomX);
					break;
				case IDC_TEXTUREZOOMY:
					prop->EnChange(prop->hwndSub,IDC_TEXTUREZOOMY,prop->prCurrent,&prop->prCurrent->fTextureZoomY);
					break;
				case IDC_TEXTUREOFFX:
					prop->EnChange(prop->hwndSub,IDC_TEXTUREOFFX,prop->prCurrent,&prop->prCurrent->fTextureOffX);
					break;
				case IDC_TEXTUREOFFY:
					prop->EnChange(prop->hwndSub,IDC_TEXTUREOFFY,prop->prCurrent,&prop->prCurrent->fTextureOffY);
					break;
				case IDC_TEXTUREROT:
					prop->EnChange(prop->hwndSub,IDC_TEXTUREROT,prop->prCurrent,&prop->prCurrent->fTextureRot);
					break;
				case IDC_SHADOWDIR:
					prop->EnChange(prop->hwndSub,IDC_SHADOWDIR,prop->prCurrent,&prop->prCurrent->iShadowDir);
					break;
				case IDC_SHADOWOFFSET:
					prop->EnChange(prop->hwndSub,IDC_SHADOWOFFSET,prop->prCurrent,&prop->prCurrent->iShadowOffset);
					break;
				case IDC_SHADOWDENSITY:
					prop->EnChange(prop->hwndSub,IDC_SHADOWDENSITY,prop->prCurrent,&prop->prCurrent->iShadowDensity);
					break;
				case IDC_SHADOWDIFFUSE:
					prop->EnChange(prop->hwndSub,IDC_SHADOWDIFFUSE,prop->prCurrent,&prop->prCurrent->rShadowDiffuse);
					break;
				case IDC_ISHADOWDIR:
					prop->EnChange(prop->hwndSub,IDC_ISHADOWDIR,prop->prCurrent,&prop->prCurrent->iIShadowDir);
					break;
				case IDC_ISHADOWOFFSET:
					prop->EnChange(prop->hwndSub,IDC_ISHADOWOFFSET,prop->prCurrent,&prop->prCurrent->iIShadowOffset);
					break;
				case IDC_ISHADOWDENSITY:
					prop->EnChange(prop->hwndSub,IDC_ISHADOWDENSITY,prop->prCurrent,&prop->prCurrent->iIShadowDensity);
					break;
				case IDC_ISHADOWDIFFUSE:
					prop->EnChange(prop->hwndSub,IDC_ISHADOWDIFFUSE,prop->prCurrent,&prop->prCurrent->rIShadowDiffuse);
					break;
				case IDC_TEXT:
					GetDlgItemText(hdlg,IDC_TEXT,str,320);
					if(wcscmp(str,prop->prCurrent->strText)!=0) {
						theJournal->Record();
						wcscpy(prop->prCurrent->strText,str);
						theTree->SetName(prop->prCurrent);
						prop->prCurrent->CalcPath(2,0);
						theApp->Edit();
						prop->tipText->UpdateText(str);
					}
					break;
				case IDC_FONTSIZE:
					prop->EnChange(prop->hwndSub,IDC_FONTSIZE,prop->prCurrent,&prop->prCurrent->fFontSize,0);
					break;
				case IDC_FONTASPECT:
					prop->EnChange(prop->hwndSub,IDC_FONTASPECT,prop->prCurrent,&prop->prCurrent->fFontAspect,0);
					break;
				case IDC_PATHOFFSET:
					prop->EnChange(prop->hwndSub,IDC_PATHOFFSET,prop->prCurrent,&prop->prCurrent->fPathOffset,0);
					break;
				case IDC_FONTSPACING:
					prop->EnChange(prop->hwndSub,IDC_FONTSPACING,prop->prCurrent,&prop->prCurrent->fFontSpacing,0);
					break;
				}
				theScreen->Send();
				LeaveCriticalSection(&csData);
			}
			break;
		}
		break;
	default:
		return FALSE;
	}
	return TRUE;
}

void App::LayoutWindow(int x,int y) {
	int w,w2,yCmd;
	RECT rc,rcTree;
	SendMessage(statusbar->hwnd,WM_SIZE,0,MAKELONG(y,x));
	GetWindowRect(statusbar->hwnd,&rc);
	y-=(rc.bottom-rc.top);
	GetWindowRect(theProp->hwndFrame,&rc);
	w=0;
	if(theProp->bFloat==false)
		w=rc.right-rc.left;
	GetWindowRect(theTree->hwnd,&rcTree);
	if(theTree->bFloat==false)
		w=max(w,rcTree.right-rcTree.left);
	GetWindowRect(theTool->hwnd,&rc);
	w2=rc.right-rc.left;
	GetWindowRect(theCmdBar->hwnd,&rc);
	yCmd=rc.bottom-rc.top;
	theTree->Pos(0,0);
	GetClientRect(theTree->hwndFrame,&rc);
	theTree->Size(rc.right-rc.left,rc.bottom-rc.top);
	theTool->Pos(x-w2,0);
	theTool->Size(w2,y);
	theCmdBar->Pos(w,0);
	theCmdBar->Size(x-w-w2,yCmd);
	theScreen->Pos(w,yCmd);
	theScreen->Size(x-w-w2,y-yCmd);
	if(IsIconic(hwnd)==0&&IsZoomed(hwnd)==0)
		GetWindowRect(hwnd,&rcWin);
}
bool Primitive::SetColor(int i) {
	theApp->pal->Show(this,i);
	theApp->CheckMenu();
	return false;
}
void CopyOneItem(Primitive* dst,Primitive* pr) {
	if(dst->prType==prtShape||dst->prType==prtText)
		delete dst->bz;
	if(dst->prType==prtImage)
		delete dst->img;
	wcscpy(dst->strFile,pr->strFile);
	wcscpy(dst->strFont,pr->strFont);
	wcscpy(dst->strName,pr->strName);
	wcscpy(dst->strText,pr->strText);
	wcscpy(dst->strTextureName,pr->strTextureName);
	dst->bmp=pr->bmp;
	dst->prType=pr->prType;
	dst->dwColor1=pr->dwColor1;
	dst->dwColor2=pr->dwColor2;
	dst->dwColor3=pr->dwColor3;
	dst->ptGrad[0]=pr->ptGrad[0];
	dst->ptGrad[1]=pr->ptGrad[1];
	dst->ptSpec[0]=pr->ptSpec[0];
	dst->ptSpec[1]=pr->ptSpec[1];
	dst->ptfOrg[0]=pr->ptfOrg[0];
	dst->ptfOrg[1]=pr->ptfOrg[1];
	dst->ptRect[0]=pr->ptRect[0];
	dst->ptRect[1]=pr->ptRect[1];
	dst->ptRect[2]=pr->ptRect[2];
	dst->ptRect[3]=pr->ptRect[3];
	delete dst->pathOutline;
	dst->pathOutline=pr->pathOutline->Clone();
	delete dst->path;
	dst->path=pr->path->Clone();
	dst->rcOutline=pr->rcOutline;
	dst->iOperation=pr->iOperation;
	dst->iGradationType=pr->iGradationType;
	dst->iLinear=pr->iLinear;
	dst->iSmoother=pr->iSmoother;
	dst->iFill=pr->iFill;
	dst->iAntialias=pr->iAntialias;
	dst->iBorder=pr->iBorder;
	dst->iBorderType=pr->iBorderType;
	dst->iBorderJoin=pr->iBorderJoin;
	dst->iLineCap=pr->iLineCap;
	dst->iClose=pr->iClose;
	dst->iBorderAsFill=pr->iBorderAsFill;
	dst->iPie=pr->iPie;
	dst->fWidth=pr->fWidth;
	dst->iAlpha=pr->iAlpha;
	dst->rAngle=pr->rAngle;
	dst->fSlant=pr->fSlant;
	dst->iStart=pr->iStart;
	dst->iStop=pr->iStop;
	dst->iVertex=pr->iVertex;
	dst->rTension=pr->rTension;
	dst->rTwist=pr->rTwist;
	dst->iStarDepth=pr->iStarDepth;
	dst->iLineType=pr->iLineType;
	dst->fRoundX1=pr->fRoundX1;
	dst->fRoundX2=pr->fRoundX2;
	dst->fRoundX3=pr->fRoundX3;
	dst->fRoundX4=pr->fRoundX4;
	dst->iRoundSeparate=pr->iRoundSeparate;
	dst->fRoundY1=pr->fRoundY1;
	dst->fRoundY2=pr->fRoundY2;
	dst->fRoundY3=pr->fRoundY3;
	dst->fRoundY4=pr->fRoundY4;
	dst->iRoundXYSeparate=pr->iRoundXYSeparate;
	dst->iCornerC1=pr->iCornerC1;
	dst->iCornerC2=pr->iCornerC2;
	dst->iCornerC3=pr->iCornerC3;
	dst->iCornerC4=pr->iCornerC4;
	dst->iDiffuse=pr->iDiffuse;

	dst->light[0]=pr->light[0];
	dst->light[1]=pr->light[1];

	dst->iEmbossDirEnable=pr->iEmbossDirEnable;
	dst->iEmbossDir=pr->iEmbossDir;
	dst->rEmbossWidth=pr->rEmbossWidth;
	dst->fEmbossDepth=pr->fEmbossDepth;
	dst->fEmbossDepth2=pr->fEmbossDepth2;
	dst->iTextureType=pr->iTextureType;
	dst->iUseTextureAlpha=pr->iUseTextureAlpha;
	dst->iTexture=pr->iTexture;
	dst->iTextureZoomXYSepa=pr->iTextureZoomXYSepa;
	dst->fTextureZoomX=pr->fTextureZoomX;
	dst->fTextureZoomY=pr->fTextureZoomY;
	dst->fTextureOffX=pr->fTextureOffX;
	dst->fTextureOffY=pr->fTextureOffY;
	dst->fTextureRot=pr->fTextureRot;
	dst->dwShadowColor=pr->dwShadowColor;
	dst->iShadowDirEnable=pr->iShadowDirEnable;
	dst->iShadowDir=pr->iShadowDir;
	dst->iShadowOffset=pr->iShadowOffset;
	dst->iShadowDensity=pr->iShadowDensity;
	dst->rShadowDiffuse=pr->rShadowDiffuse;
	dst->dwIShadowColor=pr->dwIShadowColor;
	dst->iIShadowDirEnable=pr->iIShadowDirEnable;
	dst->iIShadowDir=pr->iIShadowDir;
	dst->iIShadowOffset=pr->iIShadowOffset;
	dst->iIShadowDensity=pr->iIShadowDensity;
	dst->rIShadowDiffuse=pr->rIShadowDiffuse;
	dst->fFontSize=pr->fFontSize;
	dst->fFontAspect=pr->fFontAspect;
	dst->fPathOffset=pr->fPathOffset;
	dst->fFontSpacing=pr->fFontSpacing;
	dst->iFontSizeUnit=pr->iFontSizeUnit;
	dst->iUseTextPath=pr->iUseTextPath;
	dst->iKeepDir=pr->iKeepDir;
	dst->iBold=pr->iBold;
	dst->iItalic=pr->iItalic;
	dst->iFix=pr->iFix;
	dst->bz=NULL;
	dst->img=NULL;
	if(pr->prType==prtShape||pr->prType==prtText) {
		dst->bz=new Bezier();
		*dst->bz=*pr->bz;
	}
	if(pr->prType==prtImage) {
		dst->img=pr->img->Clone();
	}
}
void UpdatePrimitive(int i) {
	if(theTree->prlCurrent.pr)
		theTree->prlCurrent.pr->iBmpDirty=i;
}
void AddPrimitive(void) {
	theTree->AddItem(NULL,NULL,L"Item",prtRect);
}
void MoveUp(void) {
	theTree->MoveUp(theTree->prlCurrent.pr);
}
void MoveDown(void) {
	theTree->MoveDown(theTree->prlCurrent.pr);
}
void Foreground(void) {
	theTree->Foreground(theTree->prlCurrent.pr);
}
void InsertNewKnob(void) {
	Primitive *pr;
	wchar_t strName[MAX_PATH];
	wchar_t strTemp[MAX_PATH];
	int i;
	for(i=0;i<10000;++i) {
		wsprintf(strName,L"(_%04d.knob)",i);
		wcscpy(strTemp,EmbedNameToTempName(strName));
		if(!PathFileExists(strTemp))
			break;
	}
	CopyFile(theApp->strDefaultKnob,strTemp,false);
	theJournal->Record();
	pr=theTree->prlCurrent.pr;
	theTree->Select1(theTree->AddItem(theTree->GetParent(pr),pr,strName));
	DoPrimitiveMenu(ID_PRIMITIVE_EDITKNOB);
}
void InsertImage(void) {
	static wchar_t wstrFile[MAX_PATH];
	OPENFILENAMEW ofnw;
	Primitive *pr;
	HideTools hide;
	ZeroMemory(&ofnw,sizeof(ofnw));
	ofnw.lStructSize=sizeof(ofnw);
	ofnw.lpstrInitialDir=theApp->strImageDir;
	ofnw.lpstrFile=wstrFile;
	ofnw.nMaxFile=MAX_PATH;
	ofnw.hwndOwner=theApp->hwnd;
	ofnw.lpstrFilter=L"All Images(*.bmp;*.png;*.jpg;*.jpeg;*.gif:*.knob;*.skin)\0*.bmp;*.png;*.jpg;*.jpeg;*.gif;*.knob;*.skin\0All Files(*.*)\0*.*\0\0";
	ofnw.nFilterIndex=1;
	hide.Hide();
	if(GetOpenFileNameW(&ofnw)) {
		theJournal->Record();
		pr=theTree->prlCurrent.pr;
		theTree->Select1(theTree->AddItem(theTree->GetParent(pr),pr,wstrFile));
	}
	hide.Restore();
}
void SelBackground(HWND hwnd) {
	CHOOSECOLOR cc;
	HideTools hide;
	ZeroMemory(&cc,sizeof(cc));
	cc.lStructSize=sizeof(cc);
	cc.hwndOwner=theApp->hwnd;
	cc.Flags=CC_RGBINIT|CC_FULLOPEN;
	cc.lpCustColors=colCust;
	cc.rgbResult=theScreen->colBackground.ToCOLORREF();
	hide.Hide();
	if(ChooseColor(&cc)) {
		theJournal->Record();
		EnterCriticalSection(&csData);
		theScreen->colBackground.SetFromCOLORREF(cc.rgbResult);
		theScreen->Send();
		LeaveCriticalSection(&csData);
		theApp->Edit();
	}
	hide.Restore();
}
void SelWorkspace(HWND hwnd) {
	CHOOSECOLOR cc;
	HideTools hide;
	ZeroMemory(&cc,sizeof(cc));
	cc.lStructSize=sizeof(cc);
	cc.hwndOwner=theApp->hwnd;
	cc.Flags=CC_RGBINIT|CC_FULLOPEN;
	cc.lpCustColors=colCust;
	cc.rgbResult=theScreen->colWorkspace.ToCOLORREF();
	hide.Hide();
	if(ChooseColor(&cc)) {
		theJournal->Record();
		EnterCriticalSection(&csData);
		theScreen->colWorkspace.SetFromCOLORREF(cc.rgbResult);
		theScreen->Send();
		LeaveCriticalSection(&csData);
		theApp->Edit();
	}
	hide.Restore();
}
void New(bool bDelName) {
	if(theApp->IsEdit() && MessageBox(theApp->hwnd,(wchar_t*)theLang->GetID(MSG_NOTSAVED),L"SkinMan",MB_OKCANCEL)==IDCANCEL)
		return;
	bzTemp->iIndex=0;
	theScreen->WaitRender();
	EnterCriticalSection(&csData);
	theTree->DelAll();
	theTree->Select1(NULL);
	theScreen->iFramesX=theScreen->iFramesY=1;
	theScreen->CanvasSize(640,480);
	theScreen->SetGrid(theApp->iDefaultGridX,theApp->iDefaultGridY);
	theScreen->iBgTransparent=1;
	theScreen->colBackground=Color(255,255,255);
	theScreen->colWorkspace=Color(100,100,100);
	theScreen->Send();
	theProp->SetType(prtNone);
	LeaveCriticalSection(&csData);
	if(bDelName)
		wcscpy(theApp->strCurrentProject,L"");
	theApp->Edit(false);
}
void SavePrimitive(PrivateProfileWriter *ppw,Primitive *p,int iCount,int piNums[],bool bExport) {
	Primitive *pParent;
	wchar_t strPrim[32],*strType;
	int iParent;
	swprintf(strPrim,L"Obj%05d",iCount);
	ppw->Section(strPrim);
	ppw->WriteStr(L"Name",p->strName);
	iParent=0;
	pParent=theTree->GetParent(p);
	if(pParent) {
		iParent=pParent->iNum;
		if(bExport) {
			if(piNums[iParent]>=0)
				iParent=piNums[iParent];
			else
				iParent=0;
		}
	}
	ppw->WriteInt(L"Parent",iParent);
	switch(p->prType) {
	case prtRect:
		strType=L"Rect";
		break;
	case prtEllipse:
		strType=L"Ellipse";
		break;
	case prtPolygon:
		strType=L"Polygon";
		break;
	case prtLines:
		strType=L"Lines";
		break;
	case prtText:
		strType=L"Text";
		break;
	case prtShape:
		strType=L"Shape";
		break;
	case prtImage:
		strType=L"Image";
		break;
	case prtGroup:
		strType=L"Group";
		break;
	default:
		strType=L"None";
	}
	ppw->WriteStr(L"Type",strType);
	ppw->WriteInt(L"Visible",p->iVisible);
	ppw->WriteInt(L"Lock",p->IsLock()&1);
	ppw->WriteInt(L"AutoSize",p->iAutoSize);
	ppw->WriteInt(L"SmallFontOpt",p->iSmallFontOpt);
	ppw->WriteFloat(L"X0",p->ptfOrg[0].X);
	ppw->WriteFloat(L"Y0",p->ptfOrg[0].Y);
	ppw->WriteFloat(L"X1",p->ptfOrg[1].X);
	ppw->WriteFloat(L"Y1",p->ptfOrg[1].Y);
	ppw->WriteInt(L"Color1",p->dwColor1);
	ppw->WriteInt(L"Color2",p->dwColor2);
	ppw->WriteInt(L"Color3",p->dwColor3);
	ppw->WriteFloat(L"Grad0X",p->ptGrad[0].X);
	ppw->WriteFloat(L"Grad0Y",p->ptGrad[0].Y);
	ppw->WriteFloat(L"Grad1X",p->ptGrad[1].X);
	ppw->WriteFloat(L"Grad1Y",p->ptGrad[1].Y);
	ppw->WriteFloat(L"SpecX",p->ptSpec[0].X);
	ppw->WriteFloat(L"SpecY",p->ptSpec[0].Y);
	ppw->WriteFloat(L"Spec2X",p->ptSpec[1].X);
	ppw->WriteFloat(L"Spec2Y",p->ptSpec[1].Y);
	ppw->WriteFloat(L"Angle",p->rAngle);
	ppw->WriteFloat(L"Slant",p->fSlant);
	ppw->WriteInt(L"Start",p->iStart);
	ppw->WriteInt(L"Stop",p->iStop);
	ppw->WriteInt(L"Vertex",p->iVertex);
	ppw->WriteFloat(L"Tension",p->rTension);
	ppw->WriteFloat(L"Twist",p->rTwist);
	ppw->WriteInt(L"StarDepth",p->iStarDepth);
	ppw->WriteInt(L"LineType",p->iLineType);
	ppw->WriteInt(L"RoundSeparate",p->iRoundSeparate);
	ppw->WriteInt(L"RoundXYSeparate",p->iRoundXYSeparate);
/*
	ppw->WriteFloat(L"RX1",p->fRoundX1);
	ppw->WriteFloat(L"RX2",p->fRoundX2);
	ppw->WriteFloat(L"RX3",p->fRoundX3);
	ppw->WriteFloat(L"RX4",p->fRoundX4);
	ppw->WriteFloat(L"RY1",p->fRoundY1);
	ppw->WriteFloat(L"RY2",p->fRoundY2);
	ppw->WriteFloat(L"RY3",p->fRoundY3);
	ppw->WriteFloat(L"RY4",p->fRoundY4);
*/
	ppw->WriteFloat(L"RoundX1",p->fRoundX1);
	ppw->WriteFloat(L"RoundX2",p->fRoundX2);
	ppw->WriteFloat(L"RoundX3",p->fRoundX3);
	ppw->WriteFloat(L"RoundX4",p->fRoundX4);
	ppw->WriteFloat(L"RoundY1",p->fRoundY1);
	ppw->WriteFloat(L"RoundY2",p->fRoundY2);
	ppw->WriteFloat(L"RoundY3",p->fRoundY3);
	ppw->WriteFloat(L"RoundY4",p->fRoundY4);
	ppw->WriteInt(L"CornerC1",p->iCornerC1);
	ppw->WriteInt(L"CornerC2",p->iCornerC2);
	ppw->WriteInt(L"CornerC3",p->iCornerC3);
	ppw->WriteInt(L"CornerC4",p->iCornerC4);
	ppw->WriteInt(L"Fill",p->iFill);
	ppw->WriteInt(L"Antialias",p->iAntialias);
	ppw->WriteInt(L"Border",p->iBorder);
	ppw->WriteInt(L"BorderType",p->iBorderType);
	ppw->WriteInt(L"BorderJoin",p->iBorderJoin);
	ppw->WriteInt(L"LineCap",p->iLineCap);
	ppw->WriteInt(L"AutoClose",p->iClose);
	ppw->WriteInt(L"BorderAsFill",p->iBorderAsFill);
	ppw->WriteInt(L"Pie",p->iPie);
	ppw->WriteInt(L"Operation",p->iOperation);
	ppw->WriteInt(L"GradationType",p->iGradationType);
	ppw->WriteInt(L"Linear",p->iLinear);
	ppw->WriteInt(L"Smoother",p->iSmoother);
	ppw->WriteFloat(L"Width",p->fWidth);
	ppw->WriteInt(L"Alpha",p->iAlpha);
	ppw->WriteInt(L"Diffuse",p->iDiffuse);

	ppw->WriteInt(L"SpecularDir",p->light[0].iSpecularDir);
	ppw->WriteInt(L"SpecularOffset",p->light[0].iSpecularOffset);
	ppw->WriteInt(L"SpecularType",p->light[0].iSpecularType);
	ppw->WriteInt(L"Specular",p->light[0].iSpecular);
	ppw->WriteInt(L"Highlight",p->light[0].iHighlight);
	ppw->WriteInt(L"HighlightWidth",p->light[0].iHighlightWidth);
	ppw->WriteInt(L"Ambient",p->light[0].iAmbient);
	ppw->WriteInt(L"SpecularDir2",p->light[1].iSpecularDir);
	ppw->WriteInt(L"SpecularOffset2",p->light[1].iSpecularOffset);
	ppw->WriteInt(L"SpecularType2",p->light[1].iSpecularType);
	ppw->WriteInt(L"Specular2",p->light[1].iSpecular);
	ppw->WriteInt(L"Highlight2",p->light[1].iHighlight);
	ppw->WriteInt(L"HighlightWidth2",p->light[1].iHighlightWidth);
	ppw->WriteInt(L"Ambient2",p->light[1].iAmbient);

	ppw->WriteInt(L"EmbossDirEnable",p->iEmbossDirEnable);
	ppw->WriteInt(L"EmbossDir",p->iEmbossDir);
	ppw->WriteFloat(L"EmbossWidth",p->rEmbossWidth);
	ppw->WriteFloat(L"EmbossDepth",p->fEmbossDepth);
	ppw->WriteFloat(L"EmbossDepth2",p->fEmbossDepth2);
	ppw->WriteInt(L"TextureDensity",p->iTexture);
	ppw->WriteInt(L"TextureAlpha",p->iUseTextureAlpha);
	if(p->iTexture||p->iUseTextureAlpha) {
		wchar_t *str;
		if(p->iTextureType<0) {
			ppw->WriteStr(L"TextureType",p->strTextureName);
		}
		else {
			str=theTexture->GetName(p->iTextureType);
			wchar_t strB[MAX_PATH];
			swprintf(strB,L"(%s)",str);
			ppw->WriteStr(L"TextureType",strB);
		}
	}
	ppw->WriteInt(L"TextureZoomXYSepa",p->iTextureZoomXYSepa);
	ppw->WriteFloat(L"TextureZoom",p->fTextureZoomX);
	ppw->WriteFloat(L"TextureZoomX",p->fTextureZoomX);
	ppw->WriteFloat(L"TextureZoomY",p->fTextureZoomY);
	ppw->WriteFloat(L"TextureOffX",p->fTextureOffX);
	ppw->WriteFloat(L"TextureOffY",p->fTextureOffY);
	ppw->WriteFloat(L"TextureRot",p->fTextureRot);
	if(p->iTexture||p->iUseTextureAlpha) {
		if(p->iTextureType<0)
			ppw->EmbedFile(L"TextureFile",TextureNameToFileName(p->strTextureName));
		else
			ppw->EmbedFile(L"TextureFile",TextureNameToFileName(theTexture->GetName(p->iTextureType)));
	}
	ppw->WriteInt(L"ShadowColor",p->dwShadowColor);
	ppw->WriteInt(L"ShadowDirEnable",p->iShadowDirEnable);
	ppw->WriteInt(L"ShadowDir",p->iShadowDir);
	ppw->WriteInt(L"ShadowOffset",p->iShadowOffset);
	ppw->WriteInt(L"ShadowDensity",p->iShadowDensity);
	ppw->WriteFloat(L"ShadowDiffuse",p->rShadowDiffuse);
	ppw->WriteInt(L"IShadowColor",p->dwIShadowColor);
	ppw->WriteInt(L"IShadowDirEnable",p->iIShadowDirEnable);
	ppw->WriteInt(L"IShadowDir",p->iIShadowDir);
	ppw->WriteInt(L"IShadowOffset",p->iIShadowOffset);
	ppw->WriteInt(L"IShadowDensity",p->iIShadowDensity);
	ppw->WriteFloat(L"IShadowDiffuse",p->rIShadowDiffuse);
	ppw->WriteStr(L"Text",p->strText);
	ppw->WriteStr(L"Font",p->strFont);
	ppw->WriteFloat(L"FontSize",p->fFontSize);
	ppw->WriteFloat(L"FontAsp",p->fFontAspect);
	ppw->WriteFloat(L"PathOffset",p->fPathOffset);
	ppw->WriteFloat(L"FontSpacing",p->fFontSpacing);
	ppw->WriteInt(L"FontSizeUnit",p->iFontSizeUnit);
	ppw->WriteInt(L"UseTextPath",p->iUseTextPath);
	ppw->WriteInt(L"KeepDir",p->iKeepDir);
	ppw->WriteInt(L"FontBold",p->iBold);
	ppw->WriteInt(L"FontItalic",p->iItalic);
	ppw->WriteInt(L"FontFix",p->iFix);

	if(p->prType==prtShape||p->prType==prtText) {
		int i;
		wchar_t str[32];
		ppw->WriteInt(L"BzCount",p->bz->iIndex);
		for(i=0;i<p->bz->iIndex;++i) {
			swprintf(str,L"BzPtX%d",i);
			ppw->WriteFloat(str,p->bz->ptAnchor[i].X);
			swprintf(str,L"BzPtY%d",i);
			ppw->WriteFloat(str,p->bz->ptAnchor[i].Y);
		}
	}
	if(p->prType==prtImage) {
		if(IsEmbed(p->strFile)) {
			ppw->WriteStr(L"Image",p->strFile);
			ppw->EmbedFile(L"ImageFile",EmbedNameToTempName(p->strFile));
		}
		else {
			ppw->WriteStr(L"Image",FileNameToEmbedName(p->strFile));
			ppw->EmbedFile(L"ImageFile",p->strFile);
		}
	}
}
void Save(wchar_t *str,bool bExport,bool bIgnoreCurrent) {
	void ExportExec(HWND hwnd,wchar_t *strFile,wchar_t *strType,bool bDialog,bool bOneFrame);
	Primitive *p;
	PrimitivePtrList *pl;
	int i,iCount,*piNums;
	ExportExec(theApp->hwnd,theApp->strThumbnail,L".png",false,true);
	PrivateProfileWriter ppw(str,L"wb");
	ppw.Section(L"Skin");
	ppw.WriteInt(L"Version",VERSION);
	ppw.WriteInt(L"Width",theScreen->iWidth);
	ppw.WriteInt(L"Height",theScreen->iHeight);
	ppw.WriteInt(L"FramesX",theScreen->iFramesX);
	ppw.WriteInt(L"FramesY",theScreen->iFramesY);
	ppw.WriteInt(L"BgTransparent",theScreen->iBgTransparent);
	ppw.WriteInt(L"BackgroundR",theScreen->colBackground.GetR());
	ppw.WriteInt(L"BackgroundG",theScreen->colBackground.GetG());
	ppw.WriteInt(L"BackgroundB",theScreen->colBackground.GetB());
	ppw.WriteInt(L"WorkspaceR",theScreen->colWorkspace.GetR());
	ppw.WriteInt(L"WorkspaceG",theScreen->colWorkspace.GetG());
	ppw.WriteInt(L"WorkspaceB",theScreen->colWorkspace.GetB());
	ppw.WriteInt(L"GridX",theScreen->iGridX);
	ppw.WriteInt(L"GridY",theScreen->iGridY);
	iCount=theTree->GetCount();
	piNums=new int[iCount+1];
	for(i=0;i<iCount;++i)
		piNums[i]=-1;
	theTree->Renumber();
	if(bExport) {
		iCount=0;
		pl=&theTree->prlCurrent;
		if(bIgnoreCurrent)
			pl=pl->next;
		for(;pl&&pl->next;pl=pl->next) {
			piNums[pl->pr->iNum]=iCount+1;
			++iCount;
		}
		ppw.WriteInt(L"ObjNum",iCount);
		SavePal(&ppw);
		iCount=0;
		pl=&theTree->prlCurrent;
		if(bIgnoreCurrent)
			pl=pl->next;
		for(;pl->next;pl=pl->next) {
			SavePrimitive(&ppw,pl->pr,iCount+1,piNums,bExport);
			++iCount;
		}
	}
	else {
		ppw.WriteInt(L"ObjNum",iCount);
		SavePal(&ppw);
		iCount=0;
		for(p=theTree->GetFirst();p;p=theTree->GetNext(p)) {
			piNums[iCount]=iCount;
			SavePrimitive(&ppw,p,iCount+1,piNums,bExport);
			++iCount;
		}
	}
	ppw.Section(L"Thumbnail");
	ppw.EmbedFile(L"T",theApp->strThumbnail);
	ppw.Section(L"End");
	delete[] piNums;
}
void Open(wchar_t *str,int iMode=0,bool bAsChild=false,bool bRec=true,int iRefresh=1) {
	int iVer;
	int iCount,iMax,iParent;
	int iInside;
	int r,g,b,w,h;
	float x0,y0,x1,y1;
	bool find;
	prim pr;
	PointF ptfCenter;
	wchar_t strSection[32];
	wchar_t strType[32];
	wchar_t strName[MAX_PATH];
	wchar_t strPrimName[64];
	wchar_t strFilename[MAX_PATH];
	Primitive *p,*pRootParent,*pRootAfter,*pParent;
	int iInit,iPosOrg;
	CheckUniqStr cus;

	wchar_t *strExt;
	strExt=PathFindExtension(str);

	RECT rc;
	GetClientRect(theScreen->hwnd,&rc);
	ptfCenter=PointF((float)theScreen->GridX((int)((rc.right/2-theScreen->iDispOffX)/theScreen->fZoom))
		,(float)theScreen->GridY((int)((rc.bottom/2-theScreen->iDispOffY)/theScreen->fZoom)));

	SetCursor(hcurWait);
	theScreen->WaitRender();
	EnterCriticalSection(&csData);
	PrivateProfileReader ppr(str);
	ppr.SetSection(L"Skin");
	iVer=ppr.ReadInt(L"Version",0);
	iMax=ppr.ReadInt(L"ObjNum",0);
	iPosOrg=0;
	iInit=0;
	if(iMode>=1) { // Import:1 or Paste:2
		if(bRec)
			theJournal->Record();
		theTree->Renumber();
		if(bAsChild) {
			pRootParent=theTree->prlCurrent.pr;
			pRootAfter=(Primitive*)-1;
		}
		else {
			PrimitivePtrList *pl;
			Primitive *prParent;
			pRootAfter=theTree->prlCurrent.pr;
			prParent=theTree->GetParent(pRootAfter);
			for(pl=&theTree->prlCurrent;pl->next;pl=pl->next) {
				if(theTree->GetParent(pl->pr)==prParent)
					pRootAfter=pl->pr;
			}
			pRootParent=theTree->GetParent(pRootAfter);
		}
		if(pRootAfter && pRootAfter!=(void*)-1)
			iPosOrg=pRootAfter->iNum;
		else
			iPosOrg=0;
	}
	else { // OpenProject
		New(false);
		w=ppr.ReadInt(L"Width",640);
		h=ppr.ReadInt(L"Height",480);
		theScreen->iFramesX=ppr.ReadInt(L"FramesX",1);
		theScreen->iFramesY=ppr.ReadInt(L"FramesY",1);
		theScreen->CanvasSize(w,h);
		theScreen->iBgTransparent=ppr.ReadInt(L"BgTransparent",1);
		r=ppr.ReadInt(L"BackgroundR",255);
		g=ppr.ReadInt(L"BackgroundG",255);
		b=ppr.ReadInt(L"BackgroundB",255);
		theScreen->colBackground=Color(r,g,b);
		r=ppr.ReadInt(L"WorkspaceR",100);
		g=ppr.ReadInt(L"WorkspaceG",100);
		b=ppr.ReadInt(L"WorkspaceB",100);
		theScreen->colWorkspace=Color(r,g,b);
		int gridx=ppr.ReadInt(L"GridX",8);
		int gridy=ppr.ReadInt(L"GridY",8);
		theScreen->SetGrid(gridx,gridy);
		LoadPal(&ppr);
	}
	theTree->Select1(NULL);
	for(iCount=0;iCount<iMax;++iCount) {
		swprintf(strSection,L"Obj%05d",iCount+1);
		ppr.SetSection(strSection);
		ppr.ReadString(L"Name",L"",strPrimName,64);
		ppr.ReadString(L"Type",L"None",strType,32);
		iParent=ppr.ReadInt(L"Parent",0);
		pParent=theTree->FindFromNum(iParent+iPosOrg);
		if(wcscmp(strType,L"Rect")==0)
			pr=prtRect;
		else if(wcscmp(strType,L"Ellipse")==0)
			pr=prtEllipse;
		else if(wcscmp(strType,L"Polygon")==0)
			pr=prtPolygon;
		else if(wcscmp(strType,L"Lines")==0)
			pr=prtLines;
		else if(wcscmp(strType,L"Text")==0)
			pr=prtText;
		else if(wcscmp(strType,L"Shape")==0)
			pr=prtShape;
		else if(wcscmp(strType,L"Image")==0)
			pr=prtImage;
		else if(wcscmp(strType,L"Group")==0)
			pr=prtGroup;
		else
			pr=prtNone;
		x0=ppr.ReadFloat(L"X0",0);
		y0=ppr.ReadFloat(L"Y0",0);
		x1=ppr.ReadFloat(L"X1",0);
		y1=ppr.ReadFloat(L"Y1",0);
		switch(pr) {
		case prtImage:
			ppr.ReadString(L"Image",L"",strName,MAX_PATH);
			if(IsEmbed(strName)) {
				wcscpy(strFilename,EmbedNameToTempName(strName));
				if(cus.Check(strFilename))
					ppr.ExtractFile(L"ImageFile",strFilename);
			}
			if(iMode==0 || (iParent&&pParent)) { //OpenProject
				p=theTree->AddItem(pParent,NULL,strName);
				if(p==0)
					p=theTree->AddItem(pParent,NULL,L"",prtRect);
			}
			else {
				p=theTree->AddItem(pRootParent,pRootAfter,strName);
				if(p==0)
					p=theTree->AddItem(pRootParent,pRootAfter,L"",prtRect);
				pRootAfter=p;
				if(iInit==0) {
					theTree->Renumber();
					iInit=1,iPosOrg=p->iNum-1;
				}
			}
			if(p) {
				ppr.ReadString(L"Name",L"",p->strName,MAX_PATH);
				p->ptfOrg[0].X=x0;
				p->ptfOrg[0].Y=y0;
				p->ptfOrg[1].X=x1;
				p->ptfOrg[1].Y=y1;
			}
			break;
		default:
			if(iMode==0 ||(iParent&&pParent)) //OpenProject
				p=theTree->AddItem(pParent,NULL,strPrimName,pr,x0,y0,x1,y1);
			else {
				p=theTree->AddItem(pRootParent,pRootAfter,strPrimName,pr,x0,y0,x1,y1);
				pRootAfter=p;
				if(iInit==0) {
					theTree->Renumber();
					iInit=1,iPosOrg=p->iNum-1;
				}
			}
			break;
		}
		iInit=1;
		if(p) {
			p->iVisible=ppr.ReadInt(L"Visible",1);
			p->Lock(ppr.ReadInt(L"Lock",0));
			p->iAutoSize=ppr.ReadInt(L"AutoSize",0);
			p->iSmallFontOpt=ppr.ReadInt(L"SmallFontOpt",0);
			r=ppr.ReadInt(L"R",0);
			g=ppr.ReadInt(L"G",0);
			b=ppr.ReadInt(L"B",0);
			p->dwColor1=ppr.ReadInt(L"Color1",0xff000000|(r<<16)|(g<<8)|b);
			p->dwColor2=ppr.ReadInt(L"Color2",0xff000000);
			p->dwColor3=ppr.ReadInt(L"Color3",0xff000000|(r<<16)|(g<<8)|b);
			p->ptGrad[0].X=ppr.ReadFloat(L"Grad0X",0.f);
			p->ptGrad[0].Y=ppr.ReadFloat(L"Grad0Y",0.f);
			p->ptGrad[1].X=ppr.ReadFloat(L"Grad1X",0.f);
			p->ptGrad[1].Y=ppr.ReadFloat(L"Grad1Y",0.f);
			p->ptSpec[0].X=ppr.ReadFloat(L"SpecX",0.5f);
			p->ptSpec[0].Y=ppr.ReadFloat(L"SpecY",0.5f);
			p->ptSpec[1].X=ppr.ReadFloat(L"Spec2X",p->ptSpec[0].X);
			p->ptSpec[1].Y=ppr.ReadFloat(L"Spec2Y",p->ptSpec[0].Y);
			p->rAngle=ppr.ReadFloat(L"Angle",0.f);
			p->fSlant=ppr.ReadFloat(L"Slant",0.f);
			p->iStart=ppr.ReadInt(L"Start",-180);
			p->iStop=ppr.ReadInt(L"Stop",180);
			p->iVertex=ppr.ReadInt(L"Vertex",3);
			p->rTension=ppr.ReadFloat(L"Tension",0.f);
			p->rTwist=ppr.ReadFloat(L"Twist",0.f);
			p->iStarDepth=ppr.ReadInt(L"StarDepth",100);
			p->iLineType=ppr.ReadInt(L"LineType",0);

			p->iRoundSeparate=ppr.ReadInt(L"RoundSeparate",0);
			p->iRoundXYSeparate=ppr.ReadInt(L"RoundXYSeparate",0);
			p->fRoundX1=ppr.ReadFloat(L"RX1",0.f,&find);
			if(find) {	//for 0.97f=<x<.995
				p->fRoundX2=ppr.ReadFloat(L"RX2",0.f);
				p->fRoundX3=ppr.ReadFloat(L"RX3",0.f);
				p->fRoundX4=ppr.ReadFloat(L"RX4",0.f);
				p->fRoundY1=ppr.ReadFloat(L"RY1",0.f);
				p->fRoundY2=ppr.ReadFloat(L"RY2",0.f);
				p->fRoundY3=ppr.ReadFloat(L"RY3",0.f);
				p->fRoundY4=ppr.ReadFloat(L"RY4",0.f);
				{
					float dx,dy;
					dx=abs(p->ptfOrg[1].X-p->ptfOrg[0].X);
					dy=abs(p->ptfOrg[1].Y-p->ptfOrg[0].Y);
					p->fRoundX1=p->fRoundX1*dx/100.f;
					p->fRoundX2=p->fRoundX2*dx/100.f;
					p->fRoundX3=p->fRoundX3*dx/100.f;
					p->fRoundX4=p->fRoundX4*dx/100.f;
					if(p->iRoundXYSeparate) {
						p->fRoundY1=p->fRoundY1*dy/100.f;
						p->fRoundY2=p->fRoundY2*dy/100.f;
						p->fRoundY3=p->fRoundY3*dy/100.f;
						p->fRoundY4=p->fRoundY4*dy/100.f;
					}
					else {
						p->fRoundY1=p->fRoundY1*dx/100.f;
						p->fRoundY2=p->fRoundY2*dx/100.f;
						p->fRoundY3=p->fRoundY3*dx/100.f;
						p->fRoundY4=p->fRoundY4*dx/100.f;
					}
				}
			}
			else {	//for x<0.97f || x>=.995
				p->fRoundX1=ppr.ReadFloat(L"Round",0.f);
				p->fRoundX2=ppr.ReadFloat(L"Round2",p->fRoundX1);
				p->fRoundX3=ppr.ReadFloat(L"Round3",p->fRoundX1);
				p->fRoundX4=ppr.ReadFloat(L"Round4",p->fRoundX1);
				p->fRoundX1=ppr.ReadFloat(L"RoundX1",p->fRoundX1);
				p->fRoundX2=ppr.ReadFloat(L"RoundX2",p->fRoundX2);
				p->fRoundX3=ppr.ReadFloat(L"RoundX3",p->fRoundX3);
				p->fRoundX4=ppr.ReadFloat(L"RoundX4",p->fRoundX4);
				p->fRoundY1=ppr.ReadFloat(L"RoundY1",p->fRoundX1);
				p->fRoundY2=ppr.ReadFloat(L"RoundY2",p->fRoundX2);
				p->fRoundY3=ppr.ReadFloat(L"RoundY3",p->fRoundX3);
				p->fRoundY4=ppr.ReadFloat(L"RoundY4",p->fRoundX4);
/*
				{
					float dx,dy;
					dx=p->ptfOrg[1].X-p->ptfOrg[0].X;
					dy=p->ptfOrg[1].Y-p->ptfOrg[0].Y;
					p->fRoundX1=p->fRoundX1*100.f/dx;
					p->fRoundX2=p->fRoundX2*100.f/dx;
					p->fRoundX3=p->fRoundX3*100.f/dx;
					p->fRoundX4=p->fRoundX4*100.f/dx;
					if(p->iRoundXYSeparate) {
						p->fRoundY1=p->fRoundY1*100.f/dy;
						p->fRoundY2=p->fRoundY2*100.f/dy;
						p->fRoundY3=p->fRoundY3*100.f/dy;
						p->fRoundY4=p->fRoundY4*100.f/dy;
					}
					else {
						p->fRoundY1=p->fRoundY1*100.f/dx;
						p->fRoundY2=p->fRoundY2*100.f/dx;
						p->fRoundY3=p->fRoundY3*100.f/dx;
						p->fRoundY4=p->fRoundY4*100.f/dx;
					}
				}
*/
			}
			p->iCornerC1=ppr.ReadInt(L"CornerC1",0);
			p->iCornerC2=ppr.ReadInt(L"CornerC2",0);
			p->iCornerC3=ppr.ReadInt(L"CornerC3",0);
			p->iCornerC4=ppr.ReadInt(L"CornerC4",0);
			p->iFill=ppr.ReadInt(L"Fill",1);
			p->iAntialias=ppr.ReadInt(L"Antialias",1);
			p->iBorder=ppr.ReadInt(L"Border",p->iFill^1);
			p->iBorderType=ppr.ReadInt(L"BorderType",0);
			p->iBorderJoin=ppr.ReadInt(L"BorderJoin",0);
			p->iLineCap=ppr.ReadInt(L"LineCap",0);
			p->iClose=ppr.ReadInt(L"AutoClose",1);
			p->iBorderAsFill=ppr.ReadInt(L"BorderAsFill",0);
			p->iPie=ppr.ReadInt(L"Pie",p->iClose);
			p->iOperation=ppr.ReadInt(L"Operation",0);
			p->iGradationType=ppr.ReadInt(L"GradationType",1);
			p->iLinear=ppr.ReadInt(L"Linear",0);
			p->iSmoother=ppr.ReadInt(L"Smoother",0);
			p->fWidth=ppr.ReadFloat(L"Width",1.f);
			p->iAlpha=ppr.ReadInt(L"Alpha",100);
			p->iDiffuse=ppr.ReadInt(L"Diffuse",0);
			p->light[0].iSpecularDir=ppr.ReadInt(L"LightDir",-45);

			p->light[0].iSpecularDir=ppr.ReadInt(L"SpecularDir",p->light[0].iSpecularDir);
			p->light[0].iSpecularOffset=ppr.ReadInt(L"SpecularOffset",-50);
			p->light[0].iSpecularType=ppr.ReadInt(L"SpecularType",0);
			p->light[0].iSpecular=ppr.ReadInt(L"Specular",50);
			p->light[0].iHighlight=ppr.ReadInt(L"Highlight",50);
			p->light[0].iHighlightWidth=ppr.ReadInt(L"HighlightWidth",0);
			p->light[0].iAmbient=ppr.ReadInt(L"Ambient",50);

			p->light[1].iSpecularDir=ppr.ReadInt(L"SpecularDir2",p->light[0].iSpecularDir);
			p->light[1].iSpecularOffset=ppr.ReadInt(L"SpecularOffset2",-50);
			p->light[1].iSpecularType=ppr.ReadInt(L"SpecularType2",0);
			p->light[1].iSpecular=ppr.ReadInt(L"Specular2",50);
			p->light[1].iHighlight=ppr.ReadInt(L"Highlight2",50);
			p->light[1].iHighlightWidth=ppr.ReadInt(L"HighlightWidth2",0);
			p->light[1].iAmbient=ppr.ReadInt(L"Ambient2",50);

			p->iEmbossDirEnable=ppr.ReadInt(L"EmbossDirEnable",0);
			p->iEmbossDir=ppr.ReadInt(L"EmbossDir",-45);
			p->rEmbossWidth=ppr.ReadFloat(L"EmbossWidth",0.f);
			p->fEmbossDepth=ppr.ReadFloat(L"EmbossDepth",0.f);
			p->fEmbossDepth2=ppr.ReadFloat(L"EmbossDepth2",0.f);
			ppr.ReadString(L"TextureType",L"",strName,256);
			p->iTextureType=-1;
			p->iTexture=ppr.ReadInt(L"TextureDensity",0);
			p->iUseTextureAlpha=ppr.ReadInt(L"TextureAlpha",0);
			if(p->iTexture||p->iUseTextureAlpha) {
				wcscpy(p->strTextureName,strName);
				if(IsEmbed(strName)) {
					wcscpy(strFilename,TextureNameToFileName(strName));
					if(cus.Check(strFilename))
						ppr.ExtractFile(L"TextureFile",strFilename);
					p->texEmbed=new Tex(strFilename);
				}
				else {
					p->iTextureType=theTexture->Find(strName);
					p->texEmbed=0;
				}
			}
			else {
				p->strTextureName[0]=0;
				p->texEmbed=0;
			}
			p->iTextureZoomXYSepa=ppr.ReadInt(L"TextureZoomXYSepa",0);
			p->fTextureZoomX=p->fTextureZoomY=ppr.ReadFloat(L"TextureZoom",100);
			p->fTextureZoomX=ppr.ReadFloat(L"TextureZoomX",p->fTextureZoomX);
			p->fTextureZoomY=ppr.ReadFloat(L"TextureZoomY",p->fTextureZoomY);
			p->fTextureOffX=ppr.ReadFloat(L"TextureOffX",0);
			p->fTextureOffY=ppr.ReadFloat(L"TextureOffY",0);
			p->fTextureRot=ppr.ReadFloat(L"TextureRot",0.f);

			iInside=ppr.ReadInt(L"ShadowInside",0);
			if(iInside) {
				p->dwIShadowColor=ppr.ReadInt(L"ShadowColor",0);
				p->iIShadowDirEnable=ppr.ReadInt(L"ShadowDirEnable",0);
				p->iIShadowDir=ppr.ReadInt(L"ShadowDir",-45);
				p->iIShadowOffset=ppr.ReadInt(L"ShadowOffset",0);
				p->iIShadowDensity=ppr.ReadInt(L"ShadowDensity",0);
				p->rIShadowDiffuse=ppr.ReadFloat(L"ShadowDiffuse",0.f);
			}
			else {
				p->dwShadowColor=ppr.ReadInt(L"ShadowColor",0);
				p->iShadowDirEnable=ppr.ReadInt(L"ShadowDirEnable",0);
				p->iShadowDir=ppr.ReadInt(L"ShadowDir",-45);
				p->iShadowOffset=ppr.ReadInt(L"ShadowOffset",0);
				p->iShadowDensity=ppr.ReadInt(L"ShadowDensity",0);
				p->rShadowDiffuse=ppr.ReadFloat(L"ShadowDiffuse",0.f);
				p->dwIShadowColor=ppr.ReadInt(L"IShadowColor",0);
				p->iIShadowDirEnable=ppr.ReadInt(L"IShadowDirEnable",0);
				p->iIShadowDir=ppr.ReadInt(L"IShadowDir",-45);
				p->iIShadowOffset=ppr.ReadInt(L"IShadowOffset",0);
				p->iIShadowDensity=ppr.ReadInt(L"IShadowDensity",0);
				p->rIShadowDiffuse=ppr.ReadFloat(L"IShadowDiffuse",0.f);
			}

			ppr.ReadString(L"Text",L"",p->strText,320);
			ppr.ReadString(L"Font",L"",p->strFont,128);
			p->fFontSize=ppr.ReadFloat(L"FontSize",12.f);
			p->fFontAspect=ppr.ReadFloat(L"FontAspect",0.f,&find);
			if(find) {
				p->fFontAspect=(p->fFontAspect<0)?10000.f/(100.f+p->fFontAspect):(100.f-p->fFontAspect);
			}
			else
				p->fFontAspect=ppr.ReadFloat(L"FontAsp",100.f);
			p->fPathOffset=ppr.ReadFloat(L"PathOffset",0.f);
			p->fFontSpacing=ppr.ReadFloat(L"FontSpacing",100.f);
			p->iFontSizeUnit=ppr.ReadInt(L"FontSizeUnit",0);
			p->iUseTextPath=ppr.ReadInt(L"UseTextPath",0);
			p->iKeepDir=ppr.ReadInt(L"KeepDir",0);
			p->iBold=ppr.ReadInt(L"FontBold",0);
			p->iItalic=ppr.ReadInt(L"FontItalic",0);
			p->iFix=ppr.ReadInt(L"FontFix",0);

			if(p->prType==prtShape||p->prType==prtText) {
				RectF rcf;
				int i,iMax;
				wchar_t str[32];
				p->bz->iIndex=iMax=ppr.ReadInt(L"BzCount",0);
				for(i=0;i<iMax;++i) {
					swprintf(str,L"BzPtX%d",i);
					p->bz->ptAnchor[i].X=ppr.ReadFloat(str,0);
					swprintf(str,L"BzPtY%d",i);
					p->bz->ptAnchor[i].Y=ppr.ReadFloat(str,0);
				}
				p->bz->iFocus=p->bz->iIndex-2;
				p->bz->SetScale(p);
			}
			theTree->SetName(p);
			if(p->prType==prtText)
				p->CalcPath(2,0);
			else
				p->CalcPath(2,1);
		}
		if(iMode>=1) { // Import or Paste
			if(iCount==0)
				theTree->Select1(p,1);
			else
				theTree->SelectAdd(p,0);
		}
	}
	if(theScreen)
		theScreen->Send();
	LeaveCriticalSection(&csData);
	InvalidateRect(theTree->hwnd,NULL,false);
	theTree->Recalc();
	if(iMode>=1) { //Import or Paste
		if(iMode==1) {
			int dx,dy;
			dx=(int)(ptfCenter.X-theTree->prlCurrent.rc.X);
			dy=(int)(ptfCenter.Y-theTree->prlCurrent.rc.Y);
			theTree->MoveSelected(dx,dy);
			theScreen->Draw();
		}
		theApp->Edit();
	}
	else
		theApp->Edit(false);
	theApp->UpdateTitle();
}
void Export(HWND hwnd) {
	wchar_t *p;
	if(theTree->prlCurrent.pr==NULL) {
		MessageBox(theApp->hwnd,L"No Primitives Selected.",L"SkinMan",MB_OK);
		return;
	}
	p=GetProjFileName(hwnd,'S',true);
	if(p) {
		Save(p,true,false);
	}
}
void Import(HWND hwnd) {
	wchar_t *p;
	p=GetProjFileName(hwnd,'L',true);
	if(p) {
		theJournal->Record();
		Open(p,1,false,true);
	}
}
void Open(HWND hwnd) {
	wchar_t *p;
	if(theApp->IsEdit() && MessageBox(theApp->hwnd,(wchar_t*)theLang->GetID(MSG_NOTSAVED),L"SkinMan",MB_OKCANCEL)==IDCANCEL)
		return;
	p=GetProjFileName(hwnd,'L',false);
	if(p) {
		theApp->Edit(false);
		wcscpy(theApp->strCurrentProject,p);
		Open(p,0,false,true);
		theRecentFiles->Add(hwnd,p);
	}
}
void SaveAs(HWND hwnd) {
	wchar_t *p;
	p=GetProjFileName(hwnd,'S',false);
	if(p) {
		wcscpy(theApp->strCurrentProject,p);
		Save(p,false,false);
		theRecentFiles->Add(hwnd,p);
		theApp->Edit(false);
		theApp->UpdateTitle();
	}
}
int GetEncoderClsid(const WCHAR* format, CLSID* pClsid) {
	UINT  num = 0;          // number of image encoders
	UINT  size = 0;         // size of the image encoder array in bytes
	char *buf;
	ImageCodecInfo* pImageCodecInfo = NULL;
	GetImageEncodersSize(&num, &size);
	if(size == 0)
		return -1;  // Failure
	pImageCodecInfo=(ImageCodecInfo*)(buf=new char[size]);
	if(pImageCodecInfo == NULL)
		return -1;  // Failure
	GetImageEncoders(num, size, pImageCodecInfo);
	for(UINT j = 0; j < num; ++j) {
		if( wcscmp(pImageCodecInfo[j].MimeType, format) == 0 ) {
			*pClsid = pImageCodecInfo[j].Clsid;
			delete[] buf;
			return j;  // Success
		}
	}
	delete[] buf;
	return -1;  // Failure
}
BOOL CALLBACK dlgprocPngOption(HWND hwnd,UINT uMsg,WPARAM wparam,LPARAM lparam) {
	switch(uMsg) {
	case WM_INITDIALOG:
		CheckButton(hwnd,IDC_TRANSPARENT,iTransparentBackground);
		break;
	case WM_COMMAND:
		switch(wparam) {
		case IDOK:
			iTransparentBackground=IsDlgButtonChecked(hwnd,IDC_TRANSPARENT);
			EndDialog(hwnd,0);
			break;
		}
		break;
	}
	return FALSE;
}
Bitmap *Make256color(Bitmap *bmpOrg) {
	Bitmap *bmp;
	Rect rc;
	BitmapData bmpdOrg,bmpd;
	DWORD dw,*pdwOrg;
	BYTE *pb;
	int x,y;
	struct hist {
		DWORD dwPal;
		int iCount;
	} hist[512];
	int iIndex,i,j;
	rc=Rect(0,0,bmpOrg->GetWidth(),bmpOrg->GetHeight());
	bmpOrg->LockBits(&rc,ImageLockModeRead|ImageLockModeWrite,PixelFormat32bppRGB,&bmpdOrg);
	for(j=0;j<256;++j)
		hist[j].iCount=0;
	hist[0].iCount=1;
	iIndex=1;
	for(y=0;y<bmpdOrg.Height;++y) {
		pdwOrg=(DWORD*)((char*)bmpdOrg.Scan0+bmpdOrg.Stride*y);
		for(x=0;x<rc.Width;++x,++pdwOrg) {
			for(j=0;j<iIndex;++j) {
				if(hist[j].dwPal==(*pdwOrg&0xffffff)) {
					hist[j].iCount++;
					break;
				}
			}
			if(j==iIndex) {
				hist[j].dwPal=(*pdwOrg&0xffffff);
				hist[j].iCount=1;
				++iIndex;
			}
			if(iIndex>256)
				break;
		}
		if(iIndex>256)
			break;
	}
	if(iIndex<=256) {
		ColorPalette *pal;
		DWORD paldat[260];
		bmp=new Bitmap(rc.Width,rc.Height,PixelFormat8bppIndexed);
		pal=(ColorPalette*)paldat;
		bmp->GetPalette(pal,256);
		pal->Flags=0;
		pal->Count=256;
		for(i=0;i<256;++i) {
			dw=hist[i].dwPal&0xffffff;
			pal->Entries[i]=dw;
		}
		bmp->SetPalette(pal);
		bmp->LockBits(&rc,ImageLockModeRead|ImageLockModeWrite,PixelFormat8bppIndexed,&bmpd);
		for(y=0;y<bmpd.Height;++y) {
			pb=(BYTE*)((char*)bmpd.Scan0+bmpd.Stride*y);
			pdwOrg=(DWORD*)((char*)bmpdOrg.Scan0+bmpdOrg.Stride*y);
			for(x=0;x<bmpd.Width;++x,++pb,++pdwOrg) {
				for(j=0;j<iIndex;++j) {
					if(hist[j].dwPal==(*pdwOrg&0xffffff)) {
						*pb=j;
						break;
					}
				}
			}
		}
		bmp->UnlockBits(&bmpd);
		bmpOrg->UnlockBits(&bmpdOrg);
		return bmp;
	}
	bmpOrg->UnlockBits(&bmpdOrg);
	return bmpOrg->Clone(rc,PixelFormat32bppARGB);
}
Bitmap *Make24bpp(Bitmap *bmpOrg) {
	Rect rc;
	Bitmap *bmp;
	rc=Rect(0,0,bmpOrg->GetWidth(),bmpOrg->GetHeight());
	bmp=new Bitmap(rc.Width,rc.Height,PixelFormat24bppRGB);
	Graphics g(bmp);
	g.DrawImage(bmpOrg,0,0,rc.Width,rc.Height);
	return bmp;
}
void ExportExec(HWND hwnd,wchar_t *strFile,wchar_t *strType,bool bDialog,bool bOneFrame=false) {
	wchar_t *strExt;
	CLSID clsid;
	bool bHasAlpha;
	Bitmap *bmp;
	bHasAlpha=false;
	if(strType==0)
		strType=strFile;
	strExt=wcsrchr(strType,'.');
	if(strExt==0) {
		MessageBox(hwnd,L"Unknown File type",L"SkinMan",MB_OK);
		return;
	}
	if(wcsicmp(strExt,L".png")==0 || wcsicmp(strExt,L".bmp32")==0)
		iTransparentBackground=1;
	else
		iTransparentBackground=0;
	if(wcsicmp(strExt,L".png")==0||wcsicmp(strExt,L".pngo")==0) {
//		if(bDialog) {
//			HideTools hide;
//			hide.Hide();
//			DialogBox(theApp->hinst,MAKEINTRESOURCE(IDD_PNGOPTION),theApp->hwnd,(DLGPROC)dlgprocPngOption);
//			hide.Restore();
//		}
//		else
//			iTransparentBackground=1;
		GetEncoderClsid(L"image/png", &clsid),bHasAlpha=true;
		EnterCriticalSection(&csData);
		if(iTransparentBackground) {
			if(bOneFrame && (theScreen->iFramesX!=1 || theScreen->iFramesY!=1)) {
				theScreen->Draw(false);
				bmp=new Bitmap(theScreen->iWidth/theScreen->iFramesX,theScreen->iHeight/theScreen->iFramesY,theScreen->iWidth*4,PixelFormat32bppARGB,(BYTE*)theScreen->pdwMem2);
				bmp->Save(strFile, &clsid, NULL);
				theScreen->Draw(true);
			}
			else {
				theScreen->Draw(false);
				bmp=new Bitmap(theScreen->iWidth,theScreen->iHeight,theScreen->iWidth*4,PixelFormat32bppARGB,(BYTE*)theScreen->pdwMem2);
				bmp->Save(strFile, &clsid, NULL);
				theScreen->Draw(true);
			}
		}
		else {
			bmp=new Bitmap(theScreen->iWidth,theScreen->iHeight,theScreen->iWidth*4,PixelFormat32bppRGB,(BYTE*)theScreen->pdwMem2);
			Bitmap bmp2(theScreen->iWidth,theScreen->iHeight,PixelFormat24bppRGB);
			Graphics g(&bmp2);
			g.DrawImage(bmp,0,0,theScreen->iWidth,theScreen->iHeight);
			bmp2.Save(strFile, &clsid, NULL);
		}
		delete bmp;
		LeaveCriticalSection(&csData);
		return;
	}
	else if(wcsicmp(strExt,L".jpg")==0)
		GetEncoderClsid(L"image/jpeg",&clsid);
	else if(wcsicmp(strExt,L".bmp")==0)
		GetEncoderClsid(L"image/bmp",&clsid);
	else if(wcsicmp(strExt,L".bmp24")==0)
		GetEncoderClsid(L"image/bmp",&clsid);
	else if(wcsicmp(strExt,L".bmp32")==0) {
		GetEncoderClsid(L"image/bmp",&clsid);
		bHasAlpha=true;
	}
	else if(wcsicmp(strExt,L".gif")==0)
		GetEncoderClsid(L"image/gif",&clsid);
	EnterCriticalSection(&csData);
	if(bHasAlpha && iTransparentBackground) {
		theScreen->Draw(false);
		bmp=new Bitmap(theScreen->iWidth,theScreen->iHeight,theScreen->iWidth*4,PixelFormat32bppARGB,(BYTE*)theScreen->pdwMem2);
		bmp->Save(strFile, &clsid, NULL);
		theScreen->Draw(true);
	}
	else {
/*		if(bOneFrame && theScreen->iFrames) {
			if(theScreen->iAlign)
				bmp=new Bitmap(theScreen->iWidth/theScreen->iFrames,theScreen->iHeight,theScreen->iWidth*4,PixelFormat32bppRGB,(BYTE*)theScreen->pdwMem2);
			else
				bmp=new Bitmap(theScreen->iWidth,theScreen->iHeight/theScreen->iFrames,theScreen->iWidth*4,PixelFormat32bppRGB,(BYTE*)theScreen->pdwMem2);
		}
		else
*/			bmp=new Bitmap(theScreen->iWidth,theScreen->iHeight,theScreen->iWidth*4,PixelFormat32bppRGB,(BYTE*)theScreen->pdwMem2);
		if(wcsicmp(strExt,L".gif")==0) {
			Bitmap *bmp2=Make256color(bmp);
			bmp2->Save(strFile, &clsid, NULL);
			delete bmp2;
		}
		else if(wcsicmp(strExt,L".bmp")==0 || wcsicmp(strExt,L".bmp24")==0) {
			Bitmap *bmp2=Make24bpp(bmp);
			bmp2->Save(strFile,&clsid,NULL);
			delete bmp2;
		}
		else
			bmp->Save(strFile, &clsid, NULL);
	}
	delete bmp;
	LeaveCriticalSection(&csData);
}
void ExportImage(HWND hwnd) {
	wchar_t *p;
	wchar_t *t;
	p=GetExportFileName(hwnd,&t);
	if(p)
		ExportExec(hwnd,p,t,true,false);
}
void AutoSave(void) {

}
void Save(HWND hwnd) {
	if(theApp->strCurrentProject[0]==0)
		return SaveAs(hwnd);
	Save(theApp->strCurrentProject,false,false);
	theApp->Edit(false);
}
int CALLBACK BrowseCallbackProc(HWND hwnd,UINT uMsg,LPARAM lParam,LPARAM lpData) {
    if(uMsg==BFFM_INITIALIZED) {
        SendMessage(hwnd,BFFM_SETSELECTION,(WPARAM)TRUE,lpData);
    }
    return 0;
}
void GetFolder(HWND hdlg,wchar_t *strFolder,wchar_t *strTitle) {
	static wchar_t dst_file[MAX_PATH];
    BROWSEINFO  binfo;
    LPITEMIDLIST idlist;

	wcscpy(dst_file,strFolder);
    binfo.hwndOwner=hdlg;
    binfo.pidlRoot=NULL;
    binfo.pszDisplayName=dst_file;
    binfo.lpszTitle=strTitle;
    binfo.ulFlags=BIF_RETURNONLYFSDIRS; 
    binfo.lpfn=&BrowseCallbackProc;               //コールバック関数を指定する
    binfo.lParam=(LPARAM)strFolder;                //コールバックに渡す引数
    binfo.iImage=(int)NULL;
    idlist=SHBrowseForFolder(&binfo);
	if(idlist)
	    SHGetPathFromIDList(idlist,strFolder);         //ITEMIDLISTからパスを得る
    CoTaskMemFree(idlist);                        //ITEMIDLISTの解放
}
LRESULT CALLBACK dlgprocSetup(HWND hwnd,UINT uMsg,WPARAM wparam,LPARAM lparam) {
	static int iChange;
	int i;
	switch(uMsg) {
	case WM_INITDIALOG:
		SetDlgItemText(hwnd,IDC_DOCDIR,theApp->strDocDir);
		SetDlgItemText(hwnd,IDC_IMAGEDIR,theApp->strImageDir);
		SetDlgItemText(hwnd,IDC_LIBDIR,theApp->strLibDir);
		SetDlgItemText(hwnd,IDC_PALDIR,theApp->strPalDir);
		CheckButton(hwnd,IDC_ALWAYSFRONT,theApp->iAlwaysFront);
		CheckButton(hwnd,IDC_GRIDENABLE,theApp->iDefaultGridEnable);
		CheckButton(hwnd,IDC_GRIDDISP,theApp->iDefaultGridDisp);
		CheckButton(hwnd,IDC_DISPGUIDE,theApp->iDispGuide);
		CheckButton(hwnd,IDC_CURSORMODE,theApp->iCursorMode);
		SetDlgItemInt(hwnd,IDC_DEFAULTGRIDX,theApp->iDefaultGridX,false);
		SetDlgItemInt(hwnd,IDC_DEFAULTGRIDY,theApp->iDefaultGridY,false);
		SetDlgItemInt(hwnd,IDC_UNDOLEVEL,theApp->iUndoLevel,false);
		iChange=0;
		break;
	case WM_COMMAND:
		switch(wparam) {
		case IDC_TOPMOST:
		case IDC_ALWAYSFRONT:
			iChange=1;
			break;
		case IDC_DOCDIRSET:
			GetFolder(hwnd,theApp->strDocDir,L".skin-file Folder");
			SetDlgItemText(hwnd,IDC_DOCDIR,theApp->strDocDir);
			iChange=1;
			break;
		case IDC_IMAGEDIRSET:
			GetFolder(hwnd,theApp->strImageDir,L"Image-file Folder");
			SetDlgItemText(hwnd,IDC_IMAGEDIR,theApp->strImageDir);
			iChange=1;
			break;
		case IDC_LIBDIRSET:
			GetFolder(hwnd,theApp->strLibDir,L"Lib-file Folder");
			SetDlgItemText(hwnd,IDC_LIBDIR,theApp->strLibDir);
			iChange=1;
			break;
		case IDC_PALDIRSET:
			GetFolder(hwnd,theApp->strPalDir,L"Pal-file Folder");
			SetDlgItemText(hwnd,IDC_PALDIR,theApp->strPalDir);
			iChange=1;
			break;
		case IDOK:
			GetDlgItemText(hwnd,IDC_DOCDIR,theApp->strDocDir,MAX_PATH);
			GetDlgItemText(hwnd,IDC_IMAGEDIR,theApp->strImageDir,MAX_PATH);
			GetDlgItemText(hwnd,IDC_LIBDIR,theApp->strLibDir,MAX_PATH);
			GetDlgItemText(hwnd,IDC_PALDIR,theApp->strPalDir,MAX_PATH);
			theApp->iAlwaysFront=IsDlgButtonChecked(hwnd,IDC_ALWAYSFRONT);
			theApp->iDefaultGridDisp=IsDlgButtonChecked(hwnd,IDC_GRIDDISP);
			theApp->iDefaultGridEnable=IsDlgButtonChecked(hwnd,IDC_GRIDENABLE);
			theApp->iDispGuide=IsDlgButtonChecked(hwnd,IDC_DISPGUIDE);
			theApp->iCursorMode=IsDlgButtonChecked(hwnd,IDC_CURSORMODE);
			theApp->iDefaultGridX=GetDlgItemInt(hwnd,IDC_DEFAULTGRIDX,NULL,false);
			theApp->iDefaultGridY=GetDlgItemInt(hwnd,IDC_DEFAULTGRIDY,NULL,false);
			i=GetDlgItemInt(hwnd,IDC_UNDOLEVEL,NULL,false);
			if(i!=theApp->iUndoLevel) {
				delete theJournal;
				theJournal=new Journal(theTree,theApp->iUndoLevel=i);
			}
			theApp->SaveSetup(1);
			if(iChange)
				MessageBox(hwnd,L"Need to Restart for reflect new settings",L"SkinMan",MB_OK);
			EndDialog(hwnd,1);
			break;
		}
	}
	return FALSE;
}
void Setup(HWND hwnd) {
	HideTools hide;
	hide.Hide();
	DialogBox(hinstMain,MAKEINTRESOURCE(IDD_SETUP),hwnd,(DLGPROC)dlgprocSetup);
	hide.Restore();
}
int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow) {
	MSG msg;
	HACCEL hAccelTable;
	hinstMain=hInstance;
//	APIDebugStart();

//T.Gaito for Fixing cannot displayed drives in FileOpenDialog-MyComputer dialog
//	CoInitializeEx(0,COINIT_MULTITHREADED|COINIT_DISABLE_OLE1DDE);
	CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
	_set_error_mode(_OUT_TO_MSGBOX );
	GdiplusStartup(&gdiToken, &gdiSI, NULL);

	theSplash=new Splash(hInstance);
	lang=GetUserDefaultLangID();
	while(*lpCmdLine=='-') {
		++lpCmdLine;
		switch(*lpCmdLine) {
		case 'k':
			++lpCmdLine;
			break;
		case 'l':
			++lpCmdLine;
			if(*lpCmdLine>='0'&&*lpCmdLine<='9') {
				swscanf(lpCmdLine,L"%d",&lang);
				while(*lpCmdLine>='0'&&*lpCmdLine<='9')
					++lpCmdLine;
			}
			else {
				switch(*lpCmdLine) {
				case 'e':
				case 'E':
					++lpCmdLine;
					lang=1033;
					break;
				case 'j':
				case 'J':
					++lpCmdLine;
					lang=1041;
					break;
				}
			}
			break;
		}
		while(*lpCmdLine && *lpCmdLine<=' ')
			++lpCmdLine;
	}

	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_SKINMAN, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);


	if (!InitInstance (hInstance, lpCmdLine, nCmdShow)) {
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_SKINMAN);

//	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);


	while(GetMessage(&msg, NULL, 0, 0)) {
		if((msg.message==WM_KEYDOWN||msg.message==WM_KEYUP)	&& (msg.wParam==VK_SHIFT||msg.wParam==VK_CONTROL) && theScreen && theScreen->hwnd && theTree) {
			if(theTree->prlCurrent.iCount>1 || theTool->toolCurrent==tGradation)
			InvalidateRect(theScreen->hwnd,NULL,FALSE);
		}
		if(TranslateAccelerator(theApp->hwnd, hAccelTable, &msg))
			continue;
		if(IsDialogMessage(theProp->hwndSub,&msg))
			continue;
		if(IsDialogMessage(theApp->pal->hwnd,&msg))
			continue;
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	if(FindWindow(szWindowClass,NULL)==NULL) {
		WIN32_FIND_DATA ffd;
		HANDLE h;
		SetCurrentDirectory(theApp->strTempDir);
		h=FindFirstFile(L"*.*",&ffd);
		if(h!=INVALID_HANDLE_VALUE) {
			do {
				if(FindWindow(szWindowClass,NULL)!=NULL)
					break;
				DeleteFile(ffd.cFileName);
			} while(FindNextFile(h,&ffd));
		}
		SetCurrentDirectory(L".\\Texture");
		h=FindFirstFile(L"*.*",&ffd);
		if(h!=INVALID_HANDLE_VALUE) {
			do {
				if(FindWindow(szWindowClass,NULL)!=NULL)
					break;
				DeleteFile(ffd.cFileName);
			} while(FindNextFile(h,&ffd));
		}
	}
	if(theZoomer)
		delete theZoomer;
	delete theApp;
	delete theLang;
	delete theProp;
	delete theClip;
	delete theClip2;
	delete theCmdBar;
	delete theTool;
	delete theJournal;
	delete theRecentFiles;
	delete theTexture;
	delete theGausian;
	delete theTree;
	delete bzTemp;
	GdiplusShutdown(gdiToken);

//	APIDebugEnd();

	return (int) msg.wParam;
}



ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX); 

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, (LPCTSTR)IDI_SKINMAN);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName=(LPCTSTR)theLang->GetID(IDC_SKINMAN);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, (LPCTSTR)IDI_SMALL);

	return RegisterClassEx(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, wchar_t *strCmdLine, int nCmdShow) {
	HWND hWnd;
	INITCOMMONCONTROLSEX icce;
	memset(&icce,0,sizeof(icce));
	icce.dwSize=sizeof(INITCOMMONCONTROLSEX);
	icce.dwICC=ICC_WIN95_CLASSES;
	InitCommonControlsEx(&icce);
	icce.dwICC=ICC_USEREX_CLASSES;
	InitCommonControlsEx(&icce);
	hcurRect=LoadCursor(hInstance,MAKEINTRESOURCE(IDC_CURSORRECT));
	hcurEllipse=LoadCursor(hInstance,MAKEINTRESOURCE(IDC_CURSORELLIPSE));
	hcurPolygon=LoadCursor(hInstance,MAKEINTRESOURCE(IDC_CURSORPOLYGON));
	hcurText=LoadCursor(hInstance,MAKEINTRESOURCE(IDC_CURSORTEXT));
	hcurShapeEdit=LoadCursor(hInstance,MAKEINTRESOURCE(IDC_CURSORSHAPEEDIT));
	hcurShapePlus=LoadCursor(hInstance,MAKEINTRESOURCE(IDC_CURSORSHAPEPLUS));
	hcurShapeClose=LoadCursor(hInstance,MAKEINTRESOURCE(IDC_CURSORSHAPECLOSE));
	hcurCurveEdit=LoadCursor(hInstance,MAKEINTRESOURCE(IDC_CURSORCURVEEDIT));
	hcurCurvePlus=LoadCursor(hInstance,MAKEINTRESOURCE(IDC_CURSORCURVEPLUS));
	hcurCurveClose=LoadCursor(hInstance,MAKEINTRESOURCE(IDC_CURSORCURVECLOSE));
	hcurArrow=LoadCursor(hInstance,MAKEINTRESOURCE(IDC_CURSORARROW));
	hcurHand=LoadCursor(hInstance,MAKEINTRESOURCE(IDC_CURSORHAND));
	hcurGradation=LoadCursor(hInstance,MAKEINTRESOURCE(IDC_CURSORGRADATION));
	hcurTrimming=LoadCursor(hInstance,MAKEINTRESOURCE(IDC_CURSORTRIMMING));
	hcurLines=LoadCursor(hInstance,MAKEINTRESOURCE(IDC_CURSORLINES));
	hcurMove=LoadCursor(hInstance,MAKEINTRESOURCE(IDC_CURSORMOVE));
	hcurSizeNESW=LoadCursor(hInstance,MAKEINTRESOURCE(IDC_CURSORNESW));
	hcurSizeNWSE=LoadCursor(hInstance,MAKEINTRESOURCE(IDC_CURSORNWSE));
	hcurSizeNS=LoadCursor(NULL,IDC_SIZENS);
	hcurSizeWE=LoadCursor(NULL,IDC_SIZEWE);
	hcurPipette=LoadCursor(hInstance,MAKEINTRESOURCE(IDC_CURSORPIPETTE));
	hcurWait=LoadCursor(NULL,IDC_WAIT);
	wsprintf(::strInstanceID,L"%08x",GetTickCount());
	bzTemp=new Bezier();
	theApp=new App(hInstance,strCmdLine);
	theTexture=new Texture();
	theTexture->Init(theApp->strTextureDir);
	theGausian=new Gausian();
	theSplash->Message(L"Setup Language");
	theLang=new Lang(theApp,theApp->strLangFile);


	hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		theApp->rcWin.left, theApp->rcWin.top, theApp->rcWin.right-theApp->rcWin.left, theApp->rcWin.bottom-theApp->rcWin.top, NULL, NULL, hInstance, NULL);
	theApp->hwnd=hWnd;
	if (!hWnd) {
      return FALSE;
	}
	theLang->SetupMenu(theApp,theApp->strLangFile);
	delete theSplash;
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
	theApp->iIconic=IsIconic(hWnd);
	return TRUE;
}
void SetToolsState(void) {
	if(theApp->iAlwaysFront) {
		if(GetForegroundWindow()==hwndMain) {
//			ShowWindow(theProp->hwndFrame,SW_RESTORE);
//			ShowWindow(theTree->hwndFrame,SW_RESTORE);
//			ShowWindow(theApp->pal->hwnd,SW_RESTORE);
//			SetWindowPos(theProp->hwndFrame,HWND_TOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);
//			SetWindowPos(theTree->hwndFrame,HWND_TOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);
//			SetWindowPos(theApp->pal->hwnd,HWND_TOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);
		}
		else {
//			MessageBeep(MB_ICONASTERISK);
//			ShowWindow(theProp->hwndFrame,SW_HIDE);
//			ShowWindow(theTree->hwndFrame,SW_HIDE);
//			ShowWindow(theApp->pal->hwnd,SW_HIDE);
//			SetWindowPos(theProp->hwndFrame,hwndMain,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);
//			SetWindowPos(theTree->hwndFrame,hwndMain,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);
//			SetWindowPos(theApp->pal->hwnd,hwndMain,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);
		}
	}
}
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;
	RECT rc;
	Primitive *pr;
	MENUITEMINFO mii;
	HDROP hdrop;
	HMENU hmenu;
	POINT ptDrop;
	PointF ptfDrop;
	HideTools hide;
	wchar_t *str;
	wchar_t strFile[MAX_PATH];
	switch (message) {
	case WM_CREATE:
		theApp->hwnd=hwnd;
		hwndMain=hwnd;
//		theTree=new Tree(GetDesktopWindow());
		if(theApp->iAlwaysFront)
			theTree=new Tree(hwnd);
		else
			theTree=new Tree(GetDesktopWindow());
		theJournal=new Journal(theTree,theApp->iUndoLevel);
		theTool=new Tool(hwnd);
		theScreen=new Screen(hwnd);
		if(theApp->iAlwaysFront) {
			theProp=new Prop(hwnd);
			theApp->pal=new ColorTool(hwnd,theApp->rcColor.left,theApp->rcColor.top);
		}
		else {
			theProp=new Prop(GetDesktopWindow());
			theApp->pal=new ColorTool(GetDesktopWindow(),theApp->rcColor.left,theApp->rcColor.top);
		}
//		if(theApp->iAlwaysFront) {
//			SetWindowPos(theProp->hwndFrame,HWND_TOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
//			SetWindowPos(theTree->hwndFrame,HWND_TOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
//			SetWindowPos(theApp->pal->hwnd,HWND_TOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
//		}
		theApp->statusbar=new StatusBar(hwnd);
		theClip=new PrimitiveList();
		theClip2=new PrimitiveList();
		theCmdBar=new CmdBar(hwnd);
		SetTimer(hwnd,0,100,NULL);
		ZeroMemory(&mii,sizeof(mii));
		mii.cbSize=sizeof(mii);
		mii.fMask=MIIM_CHECKMARKS;
		mii.hbmpUnchecked=mii.hbmpChecked=theApp->hbmpFileNew;
		SetMenuItemInfo(GetSubMenu(GetMenu(hwnd),0),ID_FILE_NEW,0,&mii);
		mii.hbmpUnchecked=mii.hbmpChecked=theApp->hbmpFileOpen;
		SetMenuItemInfo(GetSubMenu(GetMenu(hwnd),0),ID_FILE_OPEN,0,&mii);
		mii.hbmpUnchecked=mii.hbmpChecked=theApp->hbmpFileSave;
		SetMenuItemInfo(GetSubMenu(GetMenu(hwnd),0),ID_FILE_SAVE,0,&mii);
		mii.hbmpUnchecked=mii.hbmpChecked=theApp->hbmpExportImage;
		SetMenuItemInfo(GetSubMenu(GetMenu(hwnd),0),ID_FILE_EXPORTIMAGE,0,&mii);
		mii.hbmpUnchecked=mii.hbmpChecked=theApp->hbmpUndo;
		SetMenuItemInfo(GetSubMenu(GetMenu(hwnd),1),ID_EDIT_UNDO,0,&mii);
		mii.hbmpUnchecked=mii.hbmpChecked=theApp->hbmpRedo;
		SetMenuItemInfo(GetSubMenu(GetMenu(hwnd),1),ID_EDIT_REDO,0,&mii);
		mii.hbmpUnchecked=mii.hbmpChecked=theApp->hbmpShapeEdit;
		SetMenuItemInfo(GetSubMenu(GetSubMenu(GetMenu(hwnd),1),6),ID_PRIMITIVE_EDITSHAPE,0,&mii);
		mii.hbmpUnchecked=mii.hbmpChecked=theApp->hbmpMirrorH;
		SetMenuItemInfo(GetSubMenu(GetSubMenu(GetMenu(hwnd),1),6),ID_PRIMITIVE_MIRRORHORIZONTAL,0,&mii);
		mii.hbmpUnchecked=mii.hbmpChecked=theApp->hbmpMirrorV;
		SetMenuItemInfo(GetSubMenu(GetSubMenu(GetMenu(hwnd),1),6),ID_PRIMITIVE_MIRRORVERTICAL,0,&mii);
		mii.hbmpUnchecked=mii.hbmpChecked=theApp->hbmpZoomIn;
		SetMenuItemInfo(GetSubMenu(GetMenu(hwnd),2),ID_DISPLAY_ZOOMIN,0,&mii);
		mii.hbmpUnchecked=mii.hbmpChecked=theApp->hbmpZoom1;
		SetMenuItemInfo(GetSubMenu(GetMenu(hwnd),2),ID_DISPLAY_ZOOM1,0,&mii);
		mii.hbmpUnchecked=mii.hbmpChecked=theApp->hbmpZoomOut;
		SetMenuItemInfo(GetSubMenu(GetMenu(hwnd),2),ID_DISPLAY_ZOOMOUT,0,&mii);
		mii.hbmpUnchecked=mii.hbmpChecked=theApp->hbmpEazel;
		SetMenuItemInfo(GetSubMenu(GetMenu(hwnd),3),ID_CANVAS_SIZE,0,&mii);
		mii.hbmpUnchecked=mii.hbmpChecked=theApp->hbmpArrow;
		SetMenuItemInfo(GetSubMenu(GetMenu(hwnd),4),ID_TOOL_ARROW,0,&mii);
		mii.hbmpUnchecked=mii.hbmpChecked=theApp->hbmpHand;
		SetMenuItemInfo(GetSubMenu(GetMenu(hwnd),4),ID_TOOL_HAND,0,&mii);
		mii.hbmpUnchecked=mii.hbmpChecked=theApp->hbmpRect;
		SetMenuItemInfo(GetSubMenu(GetMenu(hwnd),4),ID_TOOL_RECT,0,&mii);
		mii.hbmpUnchecked=mii.hbmpChecked=theApp->hbmpEllipse;
		SetMenuItemInfo(GetSubMenu(GetMenu(hwnd),4),ID_TOOL_ELLIPSE,0,&mii);
		mii.hbmpUnchecked=mii.hbmpChecked=theApp->hbmpPolygon;
		SetMenuItemInfo(GetSubMenu(GetMenu(hwnd),4),ID_TOOL_POLYGON,0,&mii);
		mii.hbmpUnchecked=mii.hbmpChecked=theApp->hbmpCurve;
		SetMenuItemInfo(GetSubMenu(GetMenu(hwnd),4),ID_TOOL_CURVE,0,&mii);
		mii.hbmpUnchecked=mii.hbmpChecked=theApp->hbmpShape;
		SetMenuItemInfo(GetSubMenu(GetMenu(hwnd),4),ID_TOOL_SHAPE,0,&mii);
		mii.hbmpUnchecked=mii.hbmpChecked=theApp->hbmpText;
		SetMenuItemInfo(GetSubMenu(GetMenu(hwnd),4),ID_TOOL_TEXT,0,&mii);
		mii.hbmpUnchecked=mii.hbmpChecked=theApp->hbmpLines;
		SetMenuItemInfo(GetSubMenu(GetMenu(hwnd),4),ID_TOOL_LINES,0,&mii);
		mii.hbmpUnchecked=mii.hbmpChecked=theApp->hbmpGradation;
		SetMenuItemInfo(GetSubMenu(GetMenu(hwnd),4),ID_TOOL_GRADATION,0,&mii);
		mii.hbmpUnchecked=mii.hbmpChecked=theApp->hbmpTrimming;
		SetMenuItemInfo(GetSubMenu(GetMenu(hwnd),4),ID_TOOL_TRIMMING,0,&mii);
		mii.hbmpUnchecked=mii.hbmpChecked=theApp->hbmpImage;
		SetMenuItemInfo(GetSubMenu(GetMenu(hwnd),4),ID_TOOL_INSERTIMAGE,0,&mii);
		mii.hbmpUnchecked=mii.hbmpChecked=theApp->hbmpNewKnob;
		SetMenuItemInfo(GetSubMenu(GetMenu(hwnd),4),ID_TOOL_INSERTNEWKNOB,0,&mii);
		hmenu=GetSubMenu(GetSubMenu(GetMenu(hwnd),1),6);
		mii.hbmpUnchecked=mii.hbmpChecked=theApp->hbmpAlignHorzLeft;
		SetMenuItemInfo(GetSubMenu(hmenu,27),ID_PRIMITIVE_ALIGNHORZL,0,&mii);
		mii.hbmpUnchecked=mii.hbmpChecked=theApp->hbmpAlignHorzCenter;
		SetMenuItemInfo(GetSubMenu(hmenu,27),ID_PRIMITIVE_ALIGNHORZC,0,&mii);
		mii.hbmpUnchecked=mii.hbmpChecked=theApp->hbmpAlignHorzRight;
		SetMenuItemInfo(GetSubMenu(hmenu,27),ID_PRIMITIVE_ALIGNHORZR,0,&mii);
		mii.hbmpUnchecked=mii.hbmpChecked=theApp->hbmpAlignVertTop;
		SetMenuItemInfo(GetSubMenu(hmenu,27),ID_PRIMITIVE_ALIGNVERTT,0,&mii);
		mii.hbmpUnchecked=mii.hbmpChecked=theApp->hbmpAlignVertCenter;
		SetMenuItemInfo(GetSubMenu(hmenu,27),ID_PRIMITIVE_ALIGNVERTC,0,&mii);
		mii.hbmpUnchecked=mii.hbmpChecked=theApp->hbmpAlignVertBottom;
		SetMenuItemInfo(GetSubMenu(hmenu,27),ID_PRIMITIVE_ALIGNVERTB,0,&mii);
		mii.hbmpUnchecked=mii.hbmpChecked=theApp->hbmpDistHorzLeft;
		SetMenuItemInfo(GetSubMenu(hmenu,28),ID_PRIMITIVE_DISTRIBUTEHORZL,0,&mii);
		mii.hbmpUnchecked=mii.hbmpChecked=theApp->hbmpDistHorzCenter;
		SetMenuItemInfo(GetSubMenu(hmenu,28),ID_PRIMITIVE_DISTRIBUTEHORZC,0,&mii);
		mii.hbmpUnchecked=mii.hbmpChecked=theApp->hbmpDistHorzRight;
		SetMenuItemInfo(GetSubMenu(hmenu,28),ID_PRIMITIVE_DISTRIBUTEHORZR,0,&mii);
		mii.hbmpUnchecked=mii.hbmpChecked=theApp->hbmpDistVertTop;
		SetMenuItemInfo(GetSubMenu(hmenu,28),ID_PRIMITIVE_DISTRIBUTEVERTT,0,&mii);
		mii.hbmpUnchecked=mii.hbmpChecked=theApp->hbmpDistVertCenter;
		SetMenuItemInfo(GetSubMenu(hmenu,28),ID_PRIMITIVE_DISTRIBUTEVERTC,0,&mii);
		mii.hbmpUnchecked=mii.hbmpChecked=theApp->hbmpDistVertBottom;
		SetMenuItemInfo(GetSubMenu(hmenu,28),ID_PRIMITIVE_DISTRIBUTEVERTB,0,&mii);
		MoveWindow(theTree->hwndFrame,theApp->rcTree.left,theApp->rcTree.top,theApp->rcTree.right-theApp->rcTree.left,theApp->rcTree.bottom-theApp->rcTree.top,0);
		theTree->Show(1);
		theProp->Show(1);
		theApp->CheckMenu();
		theScreen->Start();
		New(false);
		if(theApp->strCurrentProject[0]) {
			Open(theApp->strCurrentProject);
			theRecentFiles->Add(hwnd,theApp->strCurrentProject);
		}
		theRecentFiles->UpdateMenu(hwnd);
		DragAcceptFiles(hwnd,TRUE);
		SetForegroundWindow(hwnd);
		SetToolsState();
		break;
	case WM_DROPFILES:
		hdrop=(HDROP)wparam;
		DragQueryFile(hdrop,0,strFile,MAX_PATH);
		DragQueryPoint(hdrop,&ptDrop);
		ClientToScreen(hwnd,&ptDrop);
		ScreenToClient(theScreen->hwnd,&ptDrop);
		ptfDrop=theScreen->ScreenToLogical(PointF(ptDrop.x,ptDrop.y));
		str=PathFindExtension(strFile);
		if(wcsicmp(str,L".SKIN")==0) {
			HMENU hmenu=CreatePopupMenu();
			POINT pt;
			Primitive *pr;
			GetCursorPos(&pt);
			AppendMenu(hmenu,MF_STRING,0x10,L"Open New File");
			AppendMenu(hmenu,MF_STRING,0x11,L"Embed to this file");
			switch(TrackPopupMenu(hmenu,TPM_LEFTALIGN|TPM_TOPALIGN|TPM_RETURNCMD,pt.x,pt.y,0,hwnd,NULL)) {
			case 0x10:
				DestroyMenu(hmenu);
				wcscpy(theApp->strCurrentProject,strFile);
				Open(strFile,0,false,true);
				theRecentFiles->Add(hwnd,strFile);
				break;
			case 0x11:
				DestroyMenu(hmenu);
				theJournal->Record();
				pr=theTree->prlCurrent.pr;
				theTree->Select1(theTree->AddItem(theTree->GetParent(pr),pr,strFile,ptfDrop.X,ptfDrop.Y));
				break;
			}
		}
		if(wcsicmp(str,L".KNOB")==0
				|| wcsicmp(str,L".BMP")==0
				|| wcsicmp(str,L".PNG")==0
				|| wcsicmp(str,L".JPG")==0
				|| wcsicmp(str,L".JPEG")==0
				|| wcsicmp(str,L".GIF")==0) {
			Primitive *pr;
			theJournal->Record();
			pr=theTree->prlCurrent.pr;
			theTree->Select1(theTree->AddItem(theTree->GetParent(pr),pr,strFile,ptfDrop.X,ptfDrop.Y));
		}
		if(wcsicmp(str,L".SKLIB")==0) {
			theJournal->Record();
			Open(strFile,1,false,true);
		}
		DragFinish(hdrop);
		break;
	case WM_SIZE:
		if(theApp->iIconic==1 && wparam==SIZE_RESTORED) {
			if(IsDlgButtonChecked(theCmdBar->hwnd,IDC_DISPTREE))
				theTree->Show(1);
			if(IsDlgButtonChecked(theCmdBar->hwnd,IDC_DISPPROP))
				theProp->Show(1);
			if(IsDlgButtonChecked(theCmdBar->hwnd,IDC_DISPCOLOR))
				theApp->pal->Show(1);
		}
		if(wparam==SIZE_MINIMIZED) {
			theApp->pal->Show(0);
			theProp->Show(0);
			theTree->Show(0);
			theApp->iIconic=1;
		}
		else
			theApp->iIconic=0;
		theApp->LayoutWindow(LOWORD(lparam),HIWORD(lparam));
		break;
	case WM_ACTIVATEAPP:
		SetToolsState();
		break;
	case WM_MOVE:
		GetClientRect(hwnd,&rc);
		theApp->LayoutWindow(rc.right,rc.bottom);
		break;
	case WM_LBUTTONUP:
		if(GetCapture()==hwnd) {
			theTree->LButtonUp();
			ReleaseCapture();
		}
		break;
	case WM_TIMER:
		if(GetAsyncKeyState(VK_SPACE)&0x8000)
			theTool->SelectTemp(tHand);
		else
			theTool->Resume();
		break;
	case WM_USER_UPDATEPRIMITIVE:
		pr=(Primitive*)wparam;
		pr->CalcPath(2,1);
		theScreen->Send();
		break;
	case WM_SETFOCUS:
		SetFocus(theScreen->hwnd);
		break;
	case WM_COMMAND:
		wmId    = LOWORD(wparam); 
		wmEvent = HIWORD(wparam); 
		if(DoPrimitiveMenu(wmId))
			break;
		switch (wmId) {
		case ID_FILE_1:
		case ID_FILE_2:
		case ID_FILE_3:
		case ID_FILE_4:
		case ID_FILE_5:
		case ID_FILE_6:
		case ID_FILE_7:
		case ID_FILE_8:
			if(*theRecentFiles->Get(wmId-ID_FILE_1)) {
				wcscpy(theApp->strCurrentProject,theRecentFiles->Get(wmId-ID_FILE_1));
				Open(theApp->strCurrentProject);
				theRecentFiles->Add(theApp->hwnd,theApp->strCurrentProject);
			}
			break;
		case ID_FILE_EXPORTIMAGE:
			ExportImage(theApp->hwnd);
			break;
		case ID_FILE_NEW:
			theJournal->Record();
			New(true);
			break;
		case ID_FILE_SAVEAS:
			SaveAs(theApp->hwnd);
			break;
		case ID_FILE_SAVE:
			Save(theApp->hwnd);
			break;
		case ID_FILE_OPEN:
			Open(theApp->hwnd);
			break;
		case ID_FILE_EXPORT:
			Export(theApp->hwnd);
			break;
		case ID_FILE_IMPORT:
			Import(theApp->hwnd);
			break;
		case ID_FILE_SETUP:
			Setup(theApp->hwnd);
			break;
		case ID_TOOL_ARROW:
			theTool->Select(tArrow);
			break;
		case ID_TOOL_HAND:
			theTool->Select(tHand);
			break;
		case ID_TOOL_RECT:
			theTool->Select(tRect,true);
			break;
		case ID_TOOL_ELLIPSE:
			theTool->Select(tEllipse,true);
			break;
		case ID_TOOL_POLYGON:
			theTool->Select(tPolygon,true);
			break;
		case ID_TOOL_TEXT:
			theTool->Select(tText,true);
			break;
		case ID_TOOL_CURVE:
			theTool->Select(tCurve);
			break;
		case ID_TOOL_SHAPE:
			theTool->Select(tShape);
			break;
		case ID_TOOL_LINES:
			theTool->Select(tLines,true);
			break;
		case ID_TOOL_GRADATION:
			theTool->Select(tGradation);
			break;
		case ID_TOOL_TRIMMING:
			theTool->Select(tTrimming);
			break;
		case ID_TOOL_INSERTIMAGE:
			InsertImage();
			break;
		case ID_TOOL_INSERTNEWKNOB:
			InsertNewKnob();
			break;
		case ID_EDIT_UNDO:
			theJournal->Undo();
			break;
		case ID_EDIT_REDO:
			theJournal->Redo();
			break;
		case ID_EDIT_SELECTALL:
			theTree->SelectAll();
			break;
		case ID_EDIT_SELECTVISIBLE:
			theTree->SelectVisible();
			break;
		case ID_DISPLAY_ZOOMIN:
			theScreen->ZoomIn();
			break;
		case ID_DISPLAY_ZOOM1:
			theScreen->Zoom1();
			break;
		case ID_DISPLAY_ZOOMOUT:
			theScreen->ZoomOut();
			break;
		case ID_DISPLAY_DRAFT:
			theScreen->SetRender(theScreen->fRender^1);
			theApp->CheckMenu();
			theTree->RefreshAll();
			break;
		case ID_DISPLAY_GRID:
			theScreen->iGridVisible=theScreen->iGridVisible^1;
			InvalidateRect(hwnd,NULL,false);
			break;
		case ID_DISPLAY_TREE:
			if(theTree->IsShow())
				theTree->Show(0);
			else
				theTree->Show(1);
			theApp->CheckMenu();
			break;
		case ID_DISPLAY_PROPERTIES:
			if(theProp->IsShow())
				theProp->Show(0);
			else
				theProp->Show(1);
			theApp->CheckMenu();
			break;
		case ID_DISPLAY_COLOR:
			if(theApp->pal->IsShow())
				theApp->pal->Show(0);
			else
				theApp->pal->Show(1);
			theApp->CheckMenu();
			break;
		case ID_HELP_ABOUT:
			hide.Hide();
			DialogBox(hinstMain, (LPCTSTR)IDD_ABOUTBOX, hwnd, (DLGPROC)About);
			hide.Restore();
			break;
		case ID_FILE_EXIT:
			SendMessage(hwnd,WM_CLOSE,0,0);
			break;
		case ID_CANVAS_SIZE:
			hide.Hide();
			DialogBox(hinstMain,(LPCTSTR)IDD_CANVASSIZE,hwnd,(DLGPROC)Screen::dlgprocCanvasSize);
			hide.Restore();
			break;
		case ID_CANVAS_TRIMMING:
			theScreen->Trimming();
			break;
		case ID_CANVAS_TRIMMINGVISIBLE:
			theScreen->TrimmingVisible();
			break;
		case ID_CANVAS_GRID:
			hide.Hide();
			DialogBox(hinstMain,(LPCTSTR)IDD_GRIDSETUP,hwnd,(DLGPROC)Screen::dlgprocGridSetup);
			hide.Restore();
			break;
		case ID_CANVAS_BACKGROUND:
			SelBackground(theApp->hwnd);
			break;
		case ID_CANVAS_WORKSPACE:
			SelWorkspace(theApp->hwnd);
			break;
		default:
			return DefWindowProc(hwnd, message, wparam, lparam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hwnd, &ps);
		EndPaint(hwnd, &ps);
		break;
	case WM_CLOSE:
		if(theApp->IsEdit() && MessageBox(hwnd,(wchar_t*)(theLang->GetID(MSG_NOTSAVED)),L"SkinMan",MB_OKCANCEL)==IDCANCEL)
			;
		else {
			theApp->SaveSetup(0);
			DestroyWindow(hwnd);
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hwnd, message, wparam, lparam);
	}
	return 0;
}

LRESULT CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	wchar_t str[32];
	switch (message) {
	case WM_INITDIALOG:
		swprintf(str,L"%d.%03d%s",VERSION/1000,VERSION%1000,VERSIONSUFFIX);
		SetDlgItemText(hDlg,IDC_VERSION,str);
		SetDlgItemInt(hDlg,IDC_LANGID,lang,0);
		return TRUE;
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		}
		break;
	}
	return FALSE;
}
