#include "stdafx.h"

#include <windows.h>


DWORD _fastcall Blend(DWORD dw1,DWORD dw2,int a) {
/*
	_asm {
		mov eax,a
		test eax,eax
		jle _blend_19
		cmp eax,255
		jl _blend_11
		mov eax,dw2
		mov dw1,eax
		jmp _blend_19
_blend_11:
		mov ebx,255
		sub ebx,eax
		movd mm3,eax
		movd mm4,ebx
		punpcklwd mm3,mm3
		punpcklwd mm3,mm3
		punpcklwd mm4,mm4
		punpcklwd mm4,mm4
		movd mm0,dw1
		movd mm1,dw2
		pxor mm5,mm5
		punpcklbw mm0,mm5
		punpcklbw mm1,mm5
		pmullw mm0,mm4
		pmullw mm1,mm3
		paddw mm0,mm1
		packuswb mm0,mm5
		movd dw1,mm0
		emms
_blend_19:
	};
	return dw1;
*/
	int a1,r1,g1,b1,a2,r2,g2,b2;
	if(a==0)
		return dw1;
	if(a==255)
		return dw2;
	a1=dw1>>24;
	r1=(dw1>>16)&0xff;
	g1=(dw1>>8)&0xff;
	b1=dw1&0xff;
	a2=dw2>>24;
	r2=(dw2>>16)&0xff;
	g2=(dw2>>8)&0xff;
	b2=dw2&0xff;
	a1=a1+(((a2-a1)*a)/255);
	r1=r1+(((r2-r1)*a)/255);
	g1=g1+(((g2-g1)*a)/255);
	b1=b1+(((b2-b1)*a)/255);
	return (a1<<24)+(r1<<16)+(g1<<8)+b1;

}
DWORD _fastcall Blend(DWORD dw1,DWORD dw2) {
	int a1,r1,g1,b1,a2,r2,g2,b2,asum;
	a2=dw2>>24;
	if(a2==255)
		return dw2;
	if(a2==0)
		return dw1;
	a1=dw1>>24;
	if(a1==0)
		return dw2;
	a1=((255-a2)*a1)/255;
	asum=a1+a2;
	r1=(dw1>>16)&0xff;
	g1=(dw1>>8)&0xff;
	b1=dw1&0xff;
	r2=(dw2>>16)&0xff;
	g2=(dw2>>8)&0xff;
	b2=dw2&0xff;
	r1=(r1*a1+r2*a2)/asum;
	g1=(g1*a1+g2*a2)/asum;
	b1=(b1*a1+b2*a2)/asum;
	return ((a1+a2)<<24)+(r1<<16)+(g1<<8)+b1;
/*
	int a,a11,a12,a1,a2;
	if((a2=dw2>>24)==255)
		return dw2;
	if(a2==0)
		return dw1;
	if((a1=dw1>>24)==0)
		return dw2;
	a=a1+(((255-a1)*a2)>>8);
	a11=((255-a2)*a1)>>8;
	a12=a11+a2;
	a11=a11*256/a12;
	a2=a2*256/a12;
	_asm {
		movd mm2,a11
		movd mm3,a2
		punpcklwd mm2,mm2
		punpcklwd mm3,mm3
		punpckldq mm2,mm2
		punpckldq mm3,mm3
		pxor mm5,mm5
		movd mm0,dw1
		movd mm1,dw2
		punpcklbw mm0,mm5
		punpcklbw mm1,mm5
		pmullw mm0,mm2
		pmullw mm1,mm3
		paddw mm0,mm1
		psrlw mm0,8
		packuswb mm0,mm5
		movd eax,mm0
		and eax,0xffffff
		mov ebx,a
		shl ebx,24
		or eax,ebx
		mov dw1,eax
		emms
	};
	return dw1;
*/
}
DWORD _fastcall Bright(DWORD dw,int iA,int iB) {
/*	_asm {
		mov eax,iA
		mov ebx,iB
		mov edx,dw
		cmp eax,255
		jne _bright_1
		test ebx,ebx
		je _bright_9
_bright_1:
		pxor mm5,mm5
		movd mm1,eax
		movd mm2,ebx
		movd mm0,edx
		punpcklwd mm1,mm1
		punpcklwd mm2,mm2
		punpckldq mm1,mm1
		punpckldq mm2,mm2
		punpcklbw mm0,mm5
		movq mm3,mm0
		pmullw mm0,mm1
		pmulhw mm3,mm1
		psrlw mm0,8
		psllw mm3,8
		por mm0,mm3
		paddw mm0,mm2
		packuswb mm0,mm5
		movd edx,mm0
		emms
_bright_9:
		and edx,0xffffff
		mov dw,edx
	};
	return dw;
*/

	int r,g,b;
	if(iA==255&&iB==0)
		return dw&0xffffff;
	r=(((dw>>16)&0xff)*iA>>8)+iB;
	g=(((dw>>8)&0xff)*iA>>8)+iB;
	b=((dw&0xff)*iA>>8)+iB;
	if(r<0)
		r=0;
	if(r>255)
		r=255;
	if(g<0)
		g=0;
	if(g>255)
		g=255;
	if(b<0)
		b=0;
	if(b>255)
		b=255;
	return (r<<16)|(g<<8)|b;

}
