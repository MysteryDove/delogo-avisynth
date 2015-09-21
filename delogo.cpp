//
//	Copylight (C) 2003 MakKi
//
//	Avisynth��GPL�Ȃ��߁A���̃\�t�g��GPL�ɂȂ�炵���ł��
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA, or visit
// http://www.gnu.org/copyleft/gpl.html .
//
//
#include <windows.h>
#include "avisynth.h"
#include "logo.h"
#include "delogo.h"


typedef struct {
	short dp_y1, y1;	/* �P�x�P     0�`4096 */
	short dp_u,  u; 	/* �F���i�j 0�`4096 */
	short dp_y2, y2;	/* �P�x�Q     */
	short dp_v,  v; 	/* �F���i�ԁj */
} LOGO_PIXEL_YUY2;

typedef struct {
	short dp_y1, y1;
	short dp_y2, y2;
	short dp_y3, y3;
	short dp_y4, y4;
	short dp_u,  u;
	short dp_v,  v;
} LOGO_PIXEL_YV12;

/********************************************************************
* 	Crate�֐�
*		Create_EraseLOGO
*		Create_AddLOGO
*********************************************************************/

//--------------------------------------------------------------------
//	Create_EraseLOGO
//
AVSValue __cdecl Create_EraseLOGO(AVSValue args, void *user_data, IScriptEnvironment *env)
{
	PClip clip(args[0].AsClip());
	const char *logofile = args[LOGOFILE].AsString(NULL);
	const char *logoname = args[LOGONAME].AsString(NULL);

	int  pos_x = args[POS_X].AsInt(0);
	int  pos_y = args[POS_Y].AsInt(0);
	int  depth = args[DEPTH].AsInt(LOGO_DEFAULT_DEPTH);
	int  start   = args[START].AsInt(0);
	int  fadein  = args[F_IN].AsInt(0);
	int  fadeout = args[F_OUT].AsInt(0);
	int  end     = args[END].AsInt(-1);

	const VideoInfo& vi = clip->GetVideoInfo();

	if(vi.IsYUY2())
		return new EraseLOGO_YUY2(clip,logofile,logoname,pos_x,pos_y,depth,start,fadein,fadeout,end,env);
	else
		env->ThrowError("ErazeLOGO : YUY2��p");

	return NULL;
}
//--------------------------------------------------------------------
//	Create_AddLOGO
//
AVSValue __cdecl Create_AddLOGO(AVSValue args, void *user_data, IScriptEnvironment *env)
{
	PClip clip(args[0].AsClip());
	const char *logofile = args[LOGOFILE].AsString(NULL);
	const char *logoname = args[LOGONAME].AsString(NULL);

	int  pos_x = args[POS_X].AsInt(0);
	int  pos_y = args[POS_Y].AsInt(0);
	int  depth = args[DEPTH].AsInt(LOGO_DEFAULT_DEPTH);
	int  start   = args[START].AsInt(0);
	int  fadein  = args[F_IN].AsInt(0);
	int  fadeout = args[F_OUT].AsInt(0);
	int  end     = args[END].AsInt(-1);

	const VideoInfo& vi = clip->GetVideoInfo();

	if(vi.IsYUY2())
		return new AddLOGO_YUY2(clip,logofile,logoname,pos_x,pos_y,depth,start,fadein,fadeout,end,env);
	else
		env->ThrowError("AddLOGO : YUY2��p");

	return NULL;
}



/********************************************************************
* 	GetFrame�֐�
*		EraseLOGO_YUY2::GetFrame
*		AddLOGO_YUY2::GetFrame
*********************************************************************/

//--------------------------------------------------------------------
//	EraseLOGO_YUY2::GetFrame
//
PVideoFrame __stdcall EraseLOGO_YUY2::GetFrame(int n, IScriptEnvironment *env)
{
	PVideoFrame frame(child->GetFrame(n, env));

	int fade = CalcFade(n);	// �t�F�[�h�s�����x�v�Z

	if(fade==0) return frame;	// �͈͊O�̂Ƃ��͂��̂܂ܕԂ�

	env->MakeWritable(&frame);

	BYTE* dst = frame->GetWritePtr();
	const int dst_w     = frame->GetRowSize();
	const int dst_pitch = frame->GetPitch();
	const int dst_h     = frame->GetHeight();

	LOGO_PIXEL_YUY2* lgp = (LOGO_PIXEL_YUY2*)(lgh +1);

	dst += lgh->x * 2 + lgh->y * dst_pitch;

	for(int i=0;i<lgh->h && i<dst_h-lgh->y;i++){
		for(int j=0;j<lgh->w && j<dst_w/2-lgh->x;j+=2){
			int dp = lgp->dp_y1 * fade / LOGO_FADE_MAX;	// Y1
			if(dp){
				if(dp==LOGO_FADE_MAX) dp--;
				dst[0] = Clamp((((dst[0]*16)*LOGO_MAX_DP - lgp->y1*dp +(LOGO_MAX_DP-dp)/2)/(LOGO_MAX_DP-dp) +8)/16 ,0,255);
			}
			dp = lgp->dp_u * fade / LOGO_FADE_MAX;		// U : Cb
			if(dp){
				if(dp==LOGO_FADE_MAX) dp--;
				dst[1] = Clamp((((dst[1]*16)*LOGO_MAX_DP - lgp->u*dp +(LOGO_MAX_DP-dp)/2)/(LOGO_MAX_DP-dp) +8)/16,0,255);
			}
			dp = lgp->dp_y2 * fade / LOGO_FADE_MAX;		// Y2
			if(dp){
				if(dp==LOGO_FADE_MAX) dp--;
				dst[2] = Clamp((((dst[2]*16)*LOGO_MAX_DP - lgp->y2*dp +(LOGO_MAX_DP-dp)/2)/(LOGO_MAX_DP-dp) +8)/16 ,0,255);
			}
			dp = lgp->dp_v * fade / LOGO_FADE_MAX;		// V : Cr
			if(dp){
				if(dp==LOGO_FADE_MAX) dp--;
				dst[3] = Clamp((((dst[3]*16)*LOGO_MAX_DP - lgp->v*dp +(LOGO_MAX_DP-dp)/2)/(LOGO_MAX_DP-dp) +8)/16,0,255);
			}
			dst += 4;
			lgp += 1;
		}
		dst += dst_pitch - j * 2;
		lgp += (lgh->w - j)/2;
	}

	return frame;
}

//--------------------------------------------------------------------
//	AddLOGO_YUY2::GetFrame
//
PVideoFrame __stdcall AddLOGO_YUY2::GetFrame(int n, IScriptEnvironment *env)
{
	PVideoFrame frame(child->GetFrame(n, env));

	int fade = CalcFade(n);	// �t�F�[�h�s�����x�v�Z

	if(fade==0) return frame;	// �͈͊O�̂Ƃ��͂��̂܂ܕԂ�

	env->MakeWritable(&frame);

	BYTE* dst = frame->GetWritePtr();
	const int dst_w = frame->GetRowSize();
	const int dst_pitch = frame->GetPitch();
	const int dst_h = frame->GetHeight();

	LOGO_PIXEL_YUY2* lgp = (LOGO_PIXEL_YUY2*)(lgh +1);

	dst += lgh->x * 2 + lgh->y * dst_pitch;

	for(int i=0;i<lgh->h && i<dst_h-lgh->y;i++){
		for(int j=0;j<lgh->w && j<dst_w/2-lgh->x;j+=2){
			int dp = lgp->dp_y1 * fade / LOGO_FADE_MAX;	// Y1
			if(dp) dst[0] = Clamp((((dst[0]*16)*(LOGO_MAX_DP-dp) + lgp->y1*dp)/LOGO_MAX_DP +8)/16, 0,255);
			dp = lgp->dp_u * fade / LOGO_FADE_MAX;		// U : Cb
			if(dp) dst[1] = Clamp((((dst[1]*16)*(LOGO_MAX_DP-dp) + lgp->u*dp)/LOGO_MAX_DP +8)/16, 0,255);
			dp = lgp->dp_y2 * fade / LOGO_FADE_MAX;		// Y2
			if(dp) dst[2] = Clamp((((dst[2]*16)*(LOGO_MAX_DP-dp) + lgp->y2*dp)/LOGO_MAX_DP +8)/16, 0,255);
			dp = lgp->dp_v * fade / LOGO_FADE_MAX;		// V : Cr
			if(dp) dst[3] = Clamp((((dst[3]*16)*(LOGO_MAX_DP-dp) + lgp->v*dp)/LOGO_MAX_DP +8)/16, 0,255);
			dst += 4;
			lgp += 1;
		}
		dst += dst_pitch - j * 2;
		lgp += (lgh->w - j)/2;
	}
	return frame;
}

/********************************************************************
* 	���S�f�[�^�E�F���
*		Logo_YUY2
*		AdjustLogo
*		ReadLogoData
*********************************************************************/


//--------------------------------------------------------------------
//	YUY2�F���
//
void deLOGO_Base::Logo_YUY2(void)
{
	int i,j;
	LOGO_PIXEL* lgp = (LOGO_PIXEL*)(lgh +1);

	for(i=0;i<lgh->h;++i){
		for(j=0;j<lgh->w;++j){
			lgp->y  = TB601_Y(lgp->y);
			lgp->cb = TB601_C(lgp->cb);
			lgp->cr = TB601_C(lgp->cr);
			++lgp;
		}
	}

	BYTE srcdata[LOGO_MAXSIZE];
	memcpy(srcdata,logodata,LOGO_DATASIZE(lgh));

	// lgh->x,w��������
	int tx = lgh->x % 2;
	lgh->x -= tx;
	lgh->w += tx;
	int tw = lgh->w % 2;
	lgh->w += tw;

	LOGO_HEADER* srch = (LOGO_HEADER*)srcdata;
	LOGO_PIXEL*      src = (LOGO_PIXEL*)(srch +1);
	LOGO_PIXEL_YUY2* dst = (LOGO_PIXEL_YUY2*)(lgh +1);

	for(i=0;i<srch->h;i++){
		if(tx){	// srch->x���
			dst->dp_y1 = dst->y1 = 0;
			dst->dp_y2 = src->dp_y /2;
			dst->y2    = src->y;
			dst->dp_u = src->dp_cb /2;
			dst->u    = src->cb;
			dst->dp_v = src->dp_cr /2;
			dst->v    = src->cr;
			++dst;
			++src;
		}
		for(j=0;j<srch->w-tx-tw;j+=2){
			dst->dp_y1 = src[0].dp_y;
			dst->y1    = src[0].y;
			dst->dp_y2 = src[1].dp_y;
			dst->y2    = src[1].y;
			dst->dp_u = (src[0].dp_cb + src[1].dp_cb +1)/2;
			if(dst->dp_u)
				dst->u = (Abs(src[0].dp_cb)*src[0].cb + Abs(src[1].dp_cb)*src[1].cb +(Abs(src[0].dp_cb)+Abs(src[1].dp_cb))/2)
																			/ (Abs(src[0].dp_cb)+Abs(src[1].dp_cb)) +2048;
			dst->dp_v = (src[0].dp_cr + src[1].dp_cr +1)/2;
			if(dst->dp_v)
				dst->v = (Abs(src[0].dp_cr)*src[0].cr + Abs(src[1].dp_cr)*src[1].cr +(Abs(src[0].dp_cr)+Abs(src[1].dp_cr))/2)
																			/ (Abs(src[0].dp_cr)+Abs(src[1].dp_cr)) +2048;
			++dst;
			src += 2;
		}
		if(tw){	// srch->w+tx���
			dst->dp_y1 = src->dp_y;
			dst->y1    = src->y;
			dst->dp_y2 = dst->y2 = 0;
			dst->dp_u  = src->dp_cb /2;
			dst->u     = src->cb;
			dst->dp_v  = src->dp_cr /2;
			dst->v     = src->cr;
			++dst;
			++src;
		}
	}
}

//--------------------------------------------------------------------
//	���S�f�[�^������
//
void deLOGO_Base::AdjustLogo(int x,int y,int depth)
{
	BYTE srcdata[LOGO_MAXSIZE];
	LOGO_HEADER* data    = (LOGO_HEADER*)srcdata;
	LOGO_HEADER* adjdata = (LOGO_HEADER*)logodata;

	int  i,j;
	int  w,h;
	int  adjx,adjy;
	LOGO_PIXEL *df;
	LOGO_PIXEL *ex;

	// �\�[�X���S�f�[�^���R�s�[
	memcpy(srcdata,logodata,LOGO_DATASIZE(logodata));

	// ���S���R�s�[
	memcpy(adjdata->name,data->name,LOGO_MAX_NAME);

	// ������W�E�ʒu�[���i�ʒu������j
	if(x>=0){
		adjdata->x = data->x + int(x/4);
		adjx = x % 4;
	} else {
		adjdata->x = data->x + int((x-3)/4);
		adjx = (4 + (x%4)) %4;
	}
	if(y>=0){
		adjdata->y = data->y + int(y/4);
		adjy = y % 4;
	} else {
		adjdata->y = data->y + int((y-3)/4);
		adjy = (4 + (y%4)) %4;
	}

	adjdata->w = w = data->w + 1;	// 1/4�P�ʒ������邽��
	adjdata->h = h = data->h + 1;	// ���A�������P���₷

	// LOGO_PIXEL�̐擪
	df = (LOGO_PIXEL*)(data +1);
	ex = (LOGO_PIXEL*)(adjdata +1);

	//----------------------------------------------------- ��ԏ�̗�
	ex[0].dp_y  = df[0].dp_y *(4-adjx)*(4-adjy)*depth/128/16;	// ���[
	ex[0].dp_cb = df[0].dp_cb*(4-adjx)*(4-adjy)*depth/128/16;
	ex[0].dp_cr = df[0].dp_cr*(4-adjx)*(4-adjy)*depth/128/16;
	ex[0].y  = df[0].y;
	ex[0].cb = df[0].cb;
	ex[0].cr = df[0].cr;
	for(i=1;i<w-1;i++){									//��
		// Y
		ex[i].dp_y = (df[i-1].dp_y*adjx*(4-adjy)
							+ df[i].dp_y*(4-adjx)*(4-adjy)) *depth/128/16;
		if(ex[i].dp_y)
			ex[i].y  = (df[i-1].y*Abs(df[i-1].dp_y)*adjx*(4-adjy)
					+ df[i].y * Abs(df[i].dp_y)*(4-adjx)*(4-adjy))
				/(Abs(df[i-1].dp_y)*adjx*(4-adjy) + Abs(df[i].dp_y)*(4-adjx)*(4-adjy));
		// Cb
		ex[i].dp_cb = (df[i-1].dp_cb*adjx*(4-adjy)
							+ df[i].dp_cb*(4-adjx)*(4-adjy)) *depth/128/16;
		if(ex[i].dp_cb)
			ex[i].cb = (df[i-1].cb*Abs(df[i-1].dp_cb)*adjx*(4-adjy)
					+ df[i].cb * Abs(df[i].dp_cb)*(4-adjx)*(4-adjy))
				/ (Abs(df[i-1].dp_cb)*adjx*(4-adjy)+Abs(df[i].dp_cb)*(4-adjx)*(4-adjy));
		// Cr
		ex[i].dp_cr = (df[i-1].dp_cr*adjx*(4-adjy)
							+ df[i].dp_cr*(4-adjx)*(4-adjy)) *depth/128/16;
		if(ex[i].dp_cr)
			ex[i].cr = (df[i-1].cr*Abs(df[i-1].dp_cr)*adjx*(4-adjy)
					+ df[i].cr * Abs(df[i].dp_cr)*(4-adjx)*(4-adjy))
				/ (Abs(df[i-1].dp_cr)*adjx*(4-adjy)+Abs(df[i].dp_cr)*(4-adjx)*(4-adjy));
	}
	ex[i].dp_y  = df[i-1].dp_y * adjx *(4-adjy)*depth/128/16;	// �E�[
	ex[i].dp_cb = df[i-1].dp_cb* adjx *(4-adjy)*depth/128/16;
	ex[i].dp_cr = df[i-1].dp_cr* adjx *(4-adjy)*depth/128/16;
	ex[i].y  = df[i-1].y;
	ex[i].cb = df[i-1].cb;
	ex[i].cr = df[i-1].cr;

	//----------------------------------------------------------- ��
	for(j=1;j<h-1;j++){
		// �P�xY		//---------------------- ���[
		ex[j*w].dp_y = (df[(j-1)*data->w].dp_y*(4-adjx)*adjy
						+ df[j*data->w].dp_y*(4-adjx)*(4-adjy)) *depth/128/16;
		if(ex[j*w].dp_y)
			ex[j*w].y = (df[(j-1)*data->w].y*Abs(df[(j-1)*data->w].dp_y)*(4-adjx)*adjy
						+ df[j*data->w].y*Abs(df[j*data->w].dp_y)*(4-adjx)*(4-adjy))
				/ (Abs(df[(j-1)*data->w].dp_y)*(4-adjx)*adjy+Abs(df[j*data->w].dp_y)*(4-adjx)*(4-adjy));
		// �F��(��)Cb
		ex[j*w].dp_cb = (df[(j-1)*data->w].dp_cb*(4-adjx)*adjy
						+ df[j*data->w].dp_cb*(4-adjx)*(4-adjy)) *depth/128/ 16;
		if(ex[j*w].dp_cb)
			ex[j*w].cb = (df[(j-1)*data->w].cb*Abs(df[(j-1)*data->w].dp_cb)*(4-adjx)*adjy
						+ df[j*data->w].cb*Abs(df[j*data->w].dp_cb)*(4-adjx)*(4-adjy))
				/ (Abs(df[(j-1)*data->w].dp_cb)*(4-adjx)*adjy+Abs(df[j*data->w].dp_cb)*(4-adjx)*(4-adjy));
		// �F��(��)Cr
		ex[j*w].dp_cr = (df[(j-1)*data->w].dp_cr*(4-adjx)*adjy
						+ df[j*data->w].dp_cr*(4-adjx)*(4-adjy)) *depth/128/ 16;
		if(ex[j*w].dp_cr)
			ex[j*w].cr = (df[(j-1)*data->w].cr*Abs(df[(j-1)*data->w].dp_cr)*(4-adjx)*adjy
						+ df[j*data->w].cr*Abs(df[j*data->w].dp_cr)*(4-adjx)*(4-adjy))
				/ (Abs(df[(j-1)*data->w].dp_cr)*(4-adjx)*adjy+Abs(df[j*data->w].dp_cr)*(4-adjx)*(4-adjy));
		for(i=1;i<w-1;i++){	//------------------ ��
			// Y
			ex[j*w+i].dp_y = (df[(j-1)*data->w+i-1].dp_y*adjx*adjy
							+ df[(j-1)*data->w+i].dp_y*(4-adjx)*adjy
							+ df[j*data->w+i-1].dp_y*adjx*(4-adjy)
							+ df[j*data->w+i].dp_y*(4-adjx)*(4-adjy) ) *depth/128/16;
			if(ex[j*w+i].dp_y)
				ex[j*w+i].y = (df[(j-1)*data->w+i-1].y*Abs(df[(j-1)*data->w+i-1].dp_y)*adjx*adjy
							+ df[(j-1)*data->w+i].y*Abs(df[(j-1)*data->w+i].dp_y)*(4-adjx)*adjy
							+ df[j*data->w+i-1].y*Abs(df[j*data->w+i-1].dp_y)*adjx*(4-adjy)
							+ df[j*data->w+i].y*Abs(df[j*data->w+i].dp_y)*(4-adjx)*(4-adjy) )
					/ (Abs(df[(j-1)*data->w+i-1].dp_y)*adjx*adjy + Abs(df[(j-1)*data->w+i].dp_y)*(4-adjx)*adjy
						+ Abs(df[j*data->w+i-1].dp_y)*adjx*(4-adjy)+Abs(df[j*data->w+i].dp_y)*(4-adjx)*(4-adjy));
			// Cb
			ex[j*w+i].dp_cb = (df[(j-1)*data->w+i-1].dp_cb*adjx*adjy
							+ df[(j-1)*data->w+i].dp_cb*(4-adjx)*adjy
							+ df[j*data->w+i-1].dp_cb*adjx*(4-adjy)
							+ df[j*data->w+i].dp_cb*(4-adjx)*(4-adjy) ) *depth/128/16;
			if(ex[j*w+i].dp_cb)
				ex[j*w+i].cb = (df[(j-1)*data->w+i-1].cb*Abs(df[(j-1)*data->w+i-1].dp_cb)*adjx*adjy
							+ df[(j-1)*data->w+i].cb*Abs(df[(j-1)*data->w+i].dp_cb)*(4-adjx)*adjy
							+ df[j*data->w+i-1].cb*Abs(df[j*data->w+i-1].dp_cb)*adjx*(4-adjy)
							+ df[j*data->w+i].cb*Abs(df[j*data->w+i].dp_cb)*(4-adjx)*(4-adjy) )
					/ (Abs(df[(j-1)*data->w+i-1].dp_cb)*adjx*adjy + Abs(df[(j-1)*data->w+i].dp_cb)*(4-adjx)*adjy
						+ Abs(df[j*data->w+i-1].dp_cb)*adjx*(4-adjy) + Abs(df[j*data->w+i].dp_cb)*(4-adjx)*(4-adjy));
			// Cr
			ex[j*w+i].dp_cr = (df[(j-1)*data->w+i-1].dp_cr*adjx*adjy
							+ df[(j-1)*data->w+i].dp_cr*(4-adjx)*adjy
							+ df[j*data->w+i-1].dp_cr*adjx*(4-adjy)
							+ df[j*data->w+i].dp_cr*(4-adjx)*(4-adjy) ) *depth/128/16;
			if(ex[j*w+i].dp_cr)
				ex[j*w+i].cr = (df[(j-1)*data->w+i-1].cr*Abs(df[(j-1)*data->w+i-1].dp_cr)*adjx*adjy
							+ df[(j-1)*data->w+i].cr*Abs(df[(j-1)*data->w+i].dp_cr)*(4-adjx)*adjy
							+ df[j*data->w+i-1].cr*Abs(df[j*data->w+i-1].dp_cr)*adjx*(4-adjy)
							+ df[j*data->w+i].cr*Abs(df[j*data->w+i].dp_cr)*(4-adjx)*(4-adjy) )
					/ (Abs(df[(j-1)*data->w+i-1].dp_cr)*adjx*adjy +Abs(df[(j-1)*data->w+i].dp_cr)*(4-adjx)*adjy
						+ Abs(df[j*data->w+i-1].dp_cr)*adjx*(4-adjy)+Abs(df[j*data->w+i].dp_cr)*(4-adjx)*(4-adjy));
		}
		// Y		//----------------------- �E�[
		ex[j*w+i].dp_y = (df[(j-1)*data->w+i-1].dp_y*adjx*adjy
						+ df[j*data->w+i-1].dp_y*adjx*(4-adjy)) *depth/128/ 16;
		if(ex[j*w+i].dp_y)
			ex[j*w+i].y = (df[(j-1)*data->w+i-1].y*Abs(df[(j-1)*data->w+i-1].dp_y)*adjx*adjy
						+ df[j*data->w+i-1].y*Abs(df[j*data->w+i-1].dp_y)*adjx*(4-adjy))
				/ (Abs(df[(j-1)*data->w+i-1].dp_y)*adjx*adjy+Abs(df[j*data->w+i-1].dp_y)*adjx*(4-adjy));
		// Cb
		ex[j*w+i].dp_cb = (df[(j-1)*data->w+i-1].dp_cb*adjx*adjy
						+ df[j*data->w+i-1].dp_cb*adjx*(4-adjy)) *depth/128/ 16;
		if(ex[j*w+i].dp_cb)
			ex[j*w+i].cb = (df[(j-1)*data->w+i-1].cb*Abs(df[(j-1)*data->w+i-1].dp_cb)*adjx*adjy
						+ df[j*data->w+i-1].cb*Abs(df[j*data->w+i-1].dp_cb)*adjx*(4-adjy))
				/ (Abs(df[(j-1)*data->w+i-1].dp_cb)*adjx*adjy+Abs(df[j*data->w+i-1].dp_cb)*adjx*(4-adjy));
		// Cr
		ex[j*w+i].dp_cr = (df[(j-1)*data->w+i-1].dp_cr*adjx*adjy
						+ df[j*data->w+i-1].dp_cr*adjx*(4-adjy)) *depth/128/ 16;
		if(ex[j*w+i].dp_cr)
			ex[j*w+i].cr = (df[(j-1)*data->w+i-1].cr*Abs(df[(j-1)*data->w+i-1].dp_cr)*adjx*adjy
						+ df[j*data->w+i-1].cr*Abs(df[j*data->w+i-1].dp_cr)*adjx*(4-adjy))
				/ (Abs(df[(j-1)*data->w+i-1].dp_cr)*adjx*adjy+Abs(df[j*data->w+i-1].dp_cr)*adjx*(4-adjy));
	}
	//--------------------------------------------------------- ��ԉ�
	ex[j*w].dp_y  = df[(j-1)*data->w].dp_y *(4-adjx)*adjy *depth/128/16;	// ���[
	ex[j*w].dp_cb = df[(j-1)*data->w].dp_cb*(4-adjx)*adjy *depth/128/16;
	ex[j*w].dp_cr = df[(j-1)*data->w].dp_cr*(4-adjx)*adjy *depth/128/16;
	ex[j*w].y  = df[(j-1)*data->w].y;
	ex[j*w].cb = df[(j-1)*data->w].cb;
	ex[j*w].cr = df[(j-1)*data->w].cr;
	for(i=1;i<w-1;i++){		// ��
		// Y
		ex[j*w+i].dp_y = (df[(j-1)*data->w+i-1].dp_y * adjx * adjy
								+ df[(j-1)*data->w+i].dp_y * (4-adjx) *adjy) *depth/128/16;
		if(ex[j*w+i].dp_y)
			ex[j*w+i].y = (df[(j-1)*data->w+i-1].y*Abs(df[(j-1)*data->w+i-1].dp_y)*adjx*adjy
						+ df[(j-1)*data->w+i].y*Abs(df[(j-1)*data->w+i].dp_y)*(4-adjx)*adjy)
				/ (Abs(df[(j-1)*data->w+i-1].dp_y)*adjx*adjy +Abs(df[(j-1)*data->w+i].dp_y)*(4-adjx)*adjy);
		// Cb
		ex[j*w+i].dp_cb = (df[(j-1)*data->w+i-1].dp_cb * adjx * adjy
								+ df[(j-1)*data->w+i].dp_cb * (4-adjx) *adjy) *depth/128/16;
		if(ex[j*w+i].dp_cb)
			ex[j*w+i].cb = (df[(j-1)*data->w+i-1].cb*Abs(df[(j-1)*data->w+i-1].dp_cb)*adjx*adjy
						+ df[(j-1)*data->w+i].cb*Abs(df[(j-1)*data->w+i].dp_cb)*(4-adjx)*adjy )
				/ (Abs(df[(j-1)*data->w+i-1].dp_cb)*adjx*adjy +Abs(df[(j-1)*data->w+i].dp_cb)*(4-adjx)*adjy);
		// Cr
		ex[j*w+i].dp_cr = (df[(j-1)*data->w+i-1].dp_cr * adjx * adjy
								+ df[(j-1)*data->w+i].dp_cr * (4-adjx) *adjy) *depth/128/16;
		if(ex[j*w+i].dp_cr)
			ex[j*w+i].cr = (df[(j-1)*data->w+i-1].cr*Abs(df[(j-1)*data->w+i-1].dp_cr)*adjx*adjy
						+ df[(j-1)*data->w+i].cr*Abs(df[(j-1)*data->w+i].dp_cr)*(4-adjx)*adjy)
				/ (Abs(df[(j-1)*data->w+i-1].dp_cr)*adjx*adjy +Abs(df[(j-1)*data->w+i].dp_cr)*(4-adjx)*adjy);
	}
	ex[j*w+i].dp_y  = df[(j-1)*data->w+i-1].dp_y *adjx*adjy *depth/128/16;	// �E�[
	ex[j*w+i].dp_cb = df[(j-1)*data->w+i-1].dp_cb*adjx*adjy *depth/128/16;
	ex[j*w+i].dp_cr = df[(j-1)*data->w+i-1].dp_cr*adjx*adjy *depth/128/16;
	ex[j*w+i].y  = df[(j-1)*data->w+i-1].y;
	ex[j*w+i].cb = df[(j-1)*data->w+i-1].cb;
	ex[j*w+i].cr = df[(j-1)*data->w+i-1].cr;
}

//--------------------------------------------------------------------
//	���S�f�[�^�ǂݍ���
//
void deLOGO_Base::ReadLogoData(const char* logofile,const char* logoname)
{
	LOGO_HEADER* data = (LOGO_HEADER*)logodata;
	HANDLE hFile;
	LOGO_HEADER lgh;
	DWORD readed = 0;
	ULONG ptr;
	unsigned char num;	// �t�@�C���Ɋ܂܂��f�[�^�̐�
	int i;

	if(logofile==NULL)
		throw "���S�f�[�^�t�@�C�����w�肳��Ă��܂���";

	// �t�@�C���I�[�v��
	hFile = CreateFile(logofile,GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if(hFile==INVALID_HANDLE_VALUE){
		throw "���S�f�[�^�t�@�C����������܂���";
	}
	if(GetFileSize(hFile, NULL)<sizeof(LOGO_HEADER)+LOGO_FILE_HEADER_STR_SIZE){	// �T�C�Y�m�F
		CloseHandle(hFile);
		throw "���S�f�[�^�t�@�C�����s���ł�";
	}

	SetFilePointer(hFile,31, 0, FILE_BEGIN);	// �擪����31byte��
	ReadFile(hFile,&num,1,&readed,NULL);	// �f�[�^���擾

	int logodata_n = 0;	// �������݃f�[�^�J�E���^

	for(i=0;i<num;i++){

		// LOGO_HEADER �ǂݍ���
		readed = 0;
		ReadFile(hFile,&lgh,sizeof(LOGO_HEADER),&readed, NULL);
		if(readed!=sizeof(LOGO_HEADER)){
			throw "���S�f�[�^�̓ǂݍ��݂Ɏ��s���܂���";
		}

		// LOGO_HEADER�R�s�[
		*data = lgh;

		ptr = (ULONG)(data +1);

		// LOGO_PIXEL�ǂݍ���
		readed = 0;
		ReadFile(hFile,(void*)ptr,LOGO_PIXELSIZE(&lgh),&readed,NULL);

		if(logoname==NULL || lstrcmp(logoname,lgh.name)==0){	// �w�胍�S��������
			if(LOGO_PIXELSIZE(&lgh)>readed){	// �K�؂�΍�
				readed -= readed % 2;
				ptr    += readed;
				memset((void *)ptr,0,LOGO_PIXELSIZE(&lgh)-readed);
			}
			CloseHandle(hFile);
			return;	// ����I��
		}
	}

	CloseHandle(hFile);
	throw "���S�f�[�^��������܂���";
}

