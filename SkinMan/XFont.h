#pragma once

#include <windows.h>
#include <stdlib.h>
#include <shlobj.h>
#include <gdiplus.h>
#include <new.h>

using namespace Gdiplus;

#define USEXFONTREPLACE 1

class XFontFamily;
class XGraphics;
class XGraphicsPath;

class XFont {
public:
	DWORD dwFontType;
	Font *font;
	HFONT hfont;
	float fSize;
	XFont(XFontFamily *xfam,float fHeight,int style,Unit unit);
	~XFont(void);
};

class XFontFamily {
	static int CALLBACK funcEnumGet(ENUMLOGFONTEX *elfe,NEWTEXTMETRICEX *ntme,DWORD dwFontType,LPARAM lp);
public:
	DWORD dwFontType;
	FontFamily *fam;
	LOGFONT lf;
	XFontFamily(void);
	XFontFamily(wchar_t *str);
	~XFontFamily(void);
	Status GetFamilyName(wchar_t *str);
//	XFont *CreateFont(int iHeight);
//	void AddString(GraphicsPath *path,wchar_t *str,int n,int style,float size,PointF ptf,StringFormat form);
};

class XInstalledFontCollection {
	int iMax,iCount,iNumCount;
	XFontFamily *afPtr;
	LOGFONT lf;
	wchar_t strName[2048];
	static int CALLBACK funcEnumCount(ENUMLOGFONTEX *elfe,NEWTEXTMETRICEX *ntme,DWORD dwFontType,LPARAM lp);
	static int CALLBACK funcEnumGet(ENUMLOGFONTEX *elfe,NEWTEXTMETRICEX *ntme,DWORD dwFontType,LPARAM lp);
	static int funcSort(XFontFamily *p1,XFontFamily *p2);
public:
	XInstalledFontCollection(void);
	~XInstalledFontCollection(void);
	int GetFamilyCount(void);
	void GetFamilies(int iNum,XFontFamily *aff,int *piNum);
};

class XGraphics : public Graphics {
public:
	XGraphics(Image *img):Graphics(img) {}
	XGraphics(HDC hdc):Graphics(hdc) {}
	XGraphics(HDC hdc,HANDLE hdevice):Graphics(hdc,hdevice) {}
	XGraphics(HWND hwnd,BOOL icm=FALSE):Graphics(hwnd,icm) {}
	Status DrawString(const WCHAR *str,INT len,const XFont *font,const RectF &rcf,const StringFormat *form,const Brush *br);
	Status DrawString(const WCHAR *str,INT len,const XFont *font,const PointF &ptf,const Brush *br);
//	Status DrawString(const WCHAR *str,INT len,const XFont *font,const PointF &ptf,const StringFormat *form,const Brush *br);
	void MeasureString(wchar_t *str,int len,XFont *font,PointF ptf,RectF *rcf);
};
class XGraphicsPath : public GraphicsPath {
	static PointF PointFx2PointF(POINTFX fx,PointF ptfOffset);
	static GraphicsPath *GetCharGlyph(HDC hdc,UINT chr,PointF ptfOffset);
public:
	XGraphicsPath(void){}
	XGraphicsPath(Point *pt,BYTE *types,INT count,FillMode mode):GraphicsPath(pt,types,count,mode){}
	XGraphicsPath(FillMode mode):GraphicsPath(mode){}
	XGraphicsPath(PointF *ptf,BYTE *types,INT count,FillMode mode):GraphicsPath(ptf,types,count,mode){}
	Status AddString(WCHAR* str,INT len,XFontFamily* xfam,INT s,REAL em,Rect& rc,StringFormat* form);
	Status AddString(WCHAR *str,INT len,XFontFamily *xfam,INT s,REAL em,PointF& ptf,const StringFormat *form);
	Status AddString(WCHAR* str,INT len,XFontFamily* xfam,INT s,REAL em,Point& pt,StringFormat*form);
	Status AddString(WCHAR* str,INT len,XFontFamily* xfam,INT s,REAL em,RectF& rcf,StringFormat* form);
	XGraphicsPath *Clone(void) {return (XGraphicsPath*)GraphicsPath::Clone();}
	Status AddPath(const XGraphicsPath *path, BOOL connect);
};

#if USEXFONTREPLACE==1
#define GraphicsPath XGraphicsPath
#define Graphics XGraphics
#define InstalledFontCollection XInstalledFontCollection
#define FontFamily XFontFamily
#define Font XFont
#endif
