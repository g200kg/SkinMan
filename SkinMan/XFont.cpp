#include "stdafx.h"

#include "XFont.h"

#undef GraphicsPath
#undef Graphics
#undef InstalledFontCollection
#undef FontFamily
#undef Font

Status XGraphicsPath::AddPath(const XGraphicsPath *path, BOOL connect) {
	return GraphicsPath::AddPath((GraphicsPath*)path,connect);
}
PointF XGraphicsPath::PointFx2PointF(POINTFX fx,PointF ptfOffset) {
	PointF ptf;
	ptf.X=(float)(fx.x.value)+fx.x.fract*(1.f/65536.f);
	ptf.Y=-((float)(fx.y.value)+fx.y.fract*(1.f/65536.f));
	ptf=ptf+ptfOffset;
	return ptf;
}
GraphicsPath *XGraphicsPath::GetCharGlyph(HDC hdc,UINT chr,PointF ptfOffset) {
	GLYPHMETRICS gm;
	BYTE *buff;
	TTPOLYGONHEADER *phdr;
	TTPOLYCURVE *pcur;
	PointF ptf[4],ptfLast;
	int i,j,n,iTotalBytes,iTotalBytesCount,iPolygonBytes,iPolygonBytesCount;
	const MAT2 m={{0,1},{0,0},{0,0},{0,1}};
	GraphicsPath *path=new GraphicsPath();
	iTotalBytes=GetGlyphOutline(hdc, chr,GGO_BEZIER|GGO_UNHINTED,&gm,0,NULL,&m);
	if(iTotalBytes==0||iTotalBytes==GDI_ERROR)
		return path;
	buff=new BYTE[iTotalBytes];
	iTotalBytesCount=0;
	GetGlyphOutline(hdc,chr,GGO_BEZIER|GGO_UNHINTED,&gm,iTotalBytes,buff,&m);
	phdr=(TTPOLYGONHEADER*)buff;
	while(iTotalBytesCount<iTotalBytes) {
		ptfLast=PointFx2PointF(phdr->pfxStart,ptfOffset);
		iPolygonBytes=phdr->cb;
		iPolygonBytesCount=sizeof(TTPOLYGONHEADER);
		pcur=(TTPOLYCURVE*)(phdr+1);
		while(iPolygonBytesCount<iPolygonBytes) {
			n=pcur->cpfx;
			switch(pcur->wType) {
			case TT_PRIM_QSPLINE:
				break;
			case TT_PRIM_CSPLINE:
				for(j=0;j<n;j+=3) {
					ptf[0]=ptfLast;
					for(i=0;i<3;++i)
						ptf[i+1]=PointFx2PointF(pcur->apfx[j+i],ptfOffset);
					path->AddBezier(ptf[0],ptf[1],ptf[2],ptf[3]);
					ptfLast=ptf[3];
				}
				break;
			case TT_PRIM_LINE:
				for(j=0;j<n;++j) {
					ptf[0]=PointFx2PointF(pcur->apfx[j],ptfOffset);
					path->AddLine(ptfLast,ptf[0]);
					ptfLast=ptf[0];
				}
				break;
			}
			iPolygonBytesCount+=sizeof(POINTFX)*n+4;
			pcur=(TTPOLYCURVE*)((char*)pcur+sizeof(POINTFX)*n+4);
		}
		path->CloseFigure();
		iTotalBytesCount+=iPolygonBytes;
		phdr=(TTPOLYGONHEADER*)((char*)phdr+iPolygonBytes);
	}
	delete[] buff;
	return path;
}
Status XGraphics::DrawString(const WCHAR *str,INT len,const XFont *font,const RectF &rcf,const StringFormat *form,const Brush *br) {
	if(font->dwFontType&TRUETYPE_FONTTYPE)
		return Graphics::DrawString(str,len,font->font,rcf,form,br);
	else {
		HDC hdc=GetHDC();
		HFONT hfontOld=(HFONT)SelectObject(hdc,font->hfont);
		RECT rc;
		Color col;
		COLORREF cref;
		SolidBrush *sbr=(SolidBrush*)br;
		sbr->GetColor(&col);
		cref=RGB(col.GetR(),col.GetG(),col.GetB());
		rc.left=(LONG)rcf.X;
		rc.top=(LONG)rcf.Y;
		rc.right=rc.left+(LONG)rcf.Width;
		rc.bottom=rc.top+(LONG)rcf.Height;
		SetTextColor(hdc,cref);
		DrawText(hdc,str,len,&rc,DT_LEFT|DT_TOP);
		SelectObject(hdc,hfontOld);
		ReleaseHDC(hdc);
	}
	return Ok;
}
Status XGraphics::DrawString(const WCHAR *str,INT len,const XFont *font,const PointF &ptf,const Brush *br) {
	RectF rcf;
	Status s;
	rcf.X=ptf.X,rcf.Y=ptf.Y;
	rcf.Width=(float)GetSystemMetrics(SM_CXSCREEN);
	rcf.Height=(float)GetSystemMetrics(SM_CYSCREEN);
	s=DrawString(str,len,font,rcf,&StringFormat(),br);
	return s;
}
void XGraphics::MeasureString(wchar_t *str,int n,XFont *font,PointF ptf,RectF *rcf) {
	rcf->X=ptf.X;
	rcf->Y=ptf.Y;
	rcf->Width=0;
	rcf->Height=0;
	if(font->dwFontType&TRUETYPE_FONTTYPE)
		Graphics::MeasureString(str,n,font->font,ptf,rcf);
	else {
		HDC hdc=GetHDC();
		SIZE sz;
		wchar_t strTemp[1024];
		HFONT hfontOld=(HFONT)SelectObject(hdc,font->hfont);
		if(n<0)
			n=wcslen(str);
		wcsncpy_s(strTemp,1024,str,n);
		strTemp[n]=0;
		while(n>0 && strTemp[n-1]==' ')
			--n,strTemp[n]=0;
		++n;
		wcscat_s(strTemp,1024,L" ");
		GetTextExtentPoint32(hdc,strTemp,n,&sz);
		SelectObject(hdc,hfontOld);
		ReleaseHDC(hdc);
		rcf->X=ptf.X;
		rcf->Y=ptf.Y;
		rcf->Width=(float)sz.cx;
		rcf->Height=(float)sz.cy;
	}
}
XFont::XFont(XFontFamily *xfam,float fHeight,int style,Unit unit) {
	font=0;
	hfont=0;
	fSize=fHeight;
	xfam->lf.lfHeight=(int)-fHeight;
	dwFontType=xfam->dwFontType;
	if(dwFontType&TRUETYPE_FONTTYPE)
		font=new Font(xfam->fam,fHeight,style,unit);
	else
		hfont=CreateFontIndirect(&xfam->lf);
}
XFont::~XFont(void) {
	if(font)
		delete font;
	if(hfont)
		DeleteObject((HGDIOBJ)hfont);
}
Status XGraphicsPath::AddString(WCHAR *str,INT len,XFontFamily *xfam,INT s,REAL em,PointF& ptf,const StringFormat *form) {
	if(xfam->dwFontType&TRUETYPE_FONTTYPE)
		return GraphicsPath::AddString(str,len,xfam->fam,s,em,ptf,form);
	else {
		XFont font(xfam,em,0,UnitPixel);
		HDC hdc=GetDC(0);
		HFONT hfontOld=(HFONT)SelectObject(hdc,font.hfont);
		ptf.Y+=em;
		GraphicsPath *path=GetCharGlyph(hdc,*str,ptf);
		GraphicsPath::AddPath(path,false);
		delete path;
		SelectObject(hdc,hfontOld);
		ReleaseDC(0,hdc);
		return Ok;
	}
}
XFontFamily::XFontFamily(void) {
	memset(&lf,0,sizeof(lf));
	fam=0;
	dwFontType=0;
}
XFontFamily::XFontFamily(wchar_t *str) {
	HDC hdc=GetDC(0);
	memset(&lf,0,sizeof(lf));
	dwFontType=0;
	fam=new FontFamily(str);
	memset(&lf,0,sizeof(lf));
	lf.lfCharSet=DEFAULT_CHARSET;
	wcsncpy_s(lf.lfFaceName,LF_FACESIZE,str,32);
	lf.lfFaceName[31]=0;
	EnumFontFamiliesEx(hdc,&lf,(FONTENUMPROCW)funcEnumGet,(LPARAM)this,0);
	ReleaseDC(0,hdc);
}
XFontFamily::~XFontFamily(void) {
	if(fam)
		delete fam;
}
int CALLBACK XFontFamily::funcEnumGet(ENUMLOGFONTEX *elfe,NEWTEXTMETRICEX *ntme,DWORD dwFontType,LPARAM lp) {
	XFontFamily *p=(XFontFamily*)lp;
	p->dwFontType=dwFontType;
	return 0;
}
Status XFontFamily::GetFamilyName(wchar_t *str) {
	wcscpy_s(str,LF_FACESIZE,lf.lfFaceName);
	return Ok;
}

XInstalledFontCollection::XInstalledFontCollection(void) {
	HDC hdc=GetDC(0);
	iMax=0;
	memset(&lf,0,sizeof(lf));
	lf.lfCharSet=DEFAULT_CHARSET;
	strName[0]=0;
	EnumFontFamiliesEx(hdc,&lf,(FONTENUMPROCW)funcEnumCount,(LPARAM)this,0);
	ReleaseDC(0,hdc);
}
XInstalledFontCollection::~XInstalledFontCollection(void) {
}
int CALLBACK XInstalledFontCollection::funcEnumCount(ENUMLOGFONTEX *elfe,NEWTEXTMETRICEX *ntme,DWORD dwFontType,LPARAM lp) {
	XInstalledFontCollection *p=(XInstalledFontCollection*)lp;
	if((dwFontType&(TRUETYPE_FONTTYPE|DEVICE_FONTTYPE)) && wcscmp(p->strName,elfe->elfLogFont.lfFaceName)!=0 && elfe->elfLogFont.lfFaceName[0]!='@') {
		++p->iMax;
		wcscpy_s(p->strName,LF_FACESIZE,elfe->elfLogFont.lfFaceName);
	}
	return 1;
}
int CALLBACK XInstalledFontCollection::funcEnumGet(ENUMLOGFONTEX *elfe,NEWTEXTMETRICEX *ntme,DWORD dwFontType,LPARAM lp) {
	XInstalledFontCollection *p=(XInstalledFontCollection*)lp;
	if((dwFontType&(TRUETYPE_FONTTYPE|DEVICE_FONTTYPE)) && wcscmp(p->strName,elfe->elfLogFont.lfFaceName)!=0 && elfe->elfLogFont.lfFaceName[0]!='@') {
		new(p->afPtr) XFontFamily(elfe->elfLogFont.lfFaceName);
		wcscpy_s(p->strName,LF_FACESIZE,elfe->elfLogFont.lfFaceName);
		p->afPtr++;
		++p->iCount;
	}
	if(p->iCount>=p->iNumCount)
		return 0;
	return 1;
}
int XInstalledFontCollection::GetFamilyCount(void) {
	return iMax;
}
int XInstalledFontCollection::funcSort(XFontFamily *p1,XFontFamily *p2) {
	return wcscmp(p1->lf.lfFaceName,p2->lf.lfFaceName);
}
void XInstalledFontCollection::GetFamilies(int iNum,XFontFamily *aff,int *piNum) {
	HDC hdc=GetDC(0);
	iCount=0;
	afPtr=aff;
	iNumCount=iNum;
	strName[0]=0;
	memset(&lf,0,sizeof(lf));
	lf.lfCharSet=DEFAULT_CHARSET;
	EnumFontFamiliesEx(hdc,&lf,(FONTENUMPROCW)funcEnumGet,(LPARAM)this,0);
	ReleaseDC(0,hdc);
	qsort(aff,iNumCount,sizeof(XFontFamily),(int (*)(const void*, const void*))funcSort);
	if(piNum)
		*piNum=iNumCount;
}

