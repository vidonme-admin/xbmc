#include "DVDVideoCodecA10.h"
#include "DVDClock.h"
#include "utils/log.h"
#include "threads/Atomics.h"

#include <sys/ioctl.h>
#include <math.h>
#include "transform_color_format.h"

#include "cores/VideoRenderers/LinuxRendererA10.h"
#include "cores/VideoRenderers/RenderFlags.h"

#define A10DEBUG
#define MEDIAINFO

extern void A10VLExit(); 
extern bool A10VLInit(int &width, int &height, double &refreshRate);

static bool a10IsEnabled = false;

void EnableA10Renderer () 
{
	a10IsEnabled = true;
}

bool IsEnableA10Renderer ()
{
	return false;
}

#define _4CC(c1,c2,c3,c4) (((u32)(c4)<<24)|((u32)(c3)<<16)|((u32)(c2)<<8)|(u32)(c1))

static void freecallback(void *callbackpriv, void *pictpriv, cedarv_picture_t &pict)
{
	((CDVDVideoCodecA10*)callbackpriv)->FreePicture(pictpriv, pict);
}

CDVDVideoCodecA10::CDVDVideoCodecA10()
{
	m_hcedarv  = NULL;
	m_yuvdata  = NULL;
	m_hwrender = false;
	memset(&m_picture, 0, sizeof(m_picture));

	//check libbdv dll is loaded 

	if( !g_libbdv.IsLoaded() )
	{
		g_libbdv.EnableDelayedUnload(false);
		if( !g_libbdv.Load() )
			CLog::Log(LOGERROR, "Load codec failed !");
	}

	m_convert_bitstream = false;
}

CDVDVideoCodecA10::~CDVDVideoCodecA10()
{
	Dispose();
	A10VLExit();
}

bool CDVDVideoCodecA10::Open(CDVDStreamInfo &hints, CDVDCodecOptions &options)
{
	//check dll is load success

	printf(" A10 Open\n");
	if( !g_libbdv.IsLoaded() )
	{
		CLog::Log(LOGERROR, "Load failed, cannot start the codec!");
		return false;
	}

	CLog::Log(LOGDEBUG, "--------------------- CDVDVideoCodecA10: Open, %d------------------------ ", hints.codec);

	s32 ret;

	if (getenv("NOA10")) {
		CLog::Log(LOGERROR, "--------------------- CDVDVideoCodecA10: disable ------------------------ ");
		CLog::Log(LOGNOTICE, "A10: disabled.\n");
		return false;
	}

	m_aspect = hints.aspect;

	memset(&m_info, 0, sizeof(m_info));

	m_info.frame_duration = 0;
	m_info.video_width = hints.width;
	m_info.video_height = hints.height;
	m_info.aspect_ratio = 1000;
	m_info.is_pts_correct = !m_hints.ptsinvalid;
	m_info.sub_format = CEDARV_SUB_FORMAT_UNKNOW;
	m_info.container_format = CEDARV_CONTAINER_FORMAT_UNKNOW;
	m_info.init_data_len = 0;
	m_info.init_data = NULL;

	switch(hints.codec) {
		//TODO: all the mapping ...

		//*CEDARV_STREAM_FORMAT_MPEG2
	case CODEC_ID_MPEG1VIDEO:
		m_info.format     = CEDARV_STREAM_FORMAT_MPEG2;
		m_info.sub_format = CEDARV_MPEG2_SUB_FORMAT_MPEG1;
		break;
	case CODEC_ID_MPEG2VIDEO:
		m_info.format     = CEDARV_STREAM_FORMAT_MPEG2;
		m_info.sub_format = CEDARV_MPEG2_SUB_FORMAT_MPEG2;
		break;

		//*CEDARV_STREAM_FORMAT_H264
	case CODEC_ID_H264:
		m_info.format = CEDARV_STREAM_FORMAT_H264;
		m_info.init_data_len = hints.extrasize;
		m_info.init_data = (u8*)hints.extradata;
		if(hints.codec_tag==27) //M2TS and TS
			m_info.container_format = CEDARV_CONTAINER_FORMAT_TS;
		
#if 0 
		//disable it temporary 
		// valid avcC data (bitstream) always starts with the value 1 (version)
		if ( *(char*)hints.extradata == 1 )
		{
			CLog::Log(LOGDEBUG, "A10: try to enable bitstream convert.");

			m_convert_bitstream = bitstream_convert_init(hints.extradata, hints.extrasize);

			if( m_convert_bitstream )
			{
				CLog::Log(LOGDEBUG, "A10: enable bitstream convert.");
			}
		}
#endif 

		break;

		//*CEDARV_STREAM_FORMAT_MPEG4
	case CODEC_ID_MPEG4:
		m_info.format = CEDARV_STREAM_FORMAT_MPEG4;
                switch(m_hints.codec_tag)
                {
                  case _4CC('D','I','V','X'):
                    m_info.sub_format = CEDARV_MPEG4_SUB_FORMAT_DIVX4;
                    break;
                  case _4CC('D','X','5','0'):
                  case _4CC('D','I','V','5'):
                    m_info.sub_format = CEDARV_MPEG4_SUB_FORMAT_DIVX5;
                    break;
                  case _4CC('X','V','I','D'):
                  case _4CC('M','P','4','V'):
                  case _4CC('P','M','P','4'):
                  case _4CC('F','M','P','4'):
                    m_info.sub_format = CEDARV_MPEG4_SUB_FORMAT_XVID;
                    break;
                  default:
                    CLog::Log(LOGERROR, "A10: (MPEG4)Codec Tag %d is unknown.\n", m_hints.codec_tag);
                    return false;
                }
		break;

		//DIVX4
		//DIVX5
		//SORENSSON_H263
		//H263
		//RMG2

		//VP6
	case CODEC_ID_VP6F:
		m_info.format     = CEDARV_STREAM_FORMAT_MPEG4;
		m_info.sub_format = CEDARV_MPEG4_SUB_FORMAT_VP6;
		m_info.init_data_len = hints.extrasize;
		m_info.init_data     = (u8*)hints.extradata;
		break;
		//WMV1
	case CODEC_ID_WMV1:
		m_info.format     = CEDARV_STREAM_FORMAT_MPEG4;
		m_info.sub_format = CEDARV_MPEG4_SUB_FORMAT_WMV1;
		break;
		//WMV2
	case CODEC_ID_WMV2:
		m_info.format     = CEDARV_STREAM_FORMAT_MPEG4;
		m_info.sub_format = CEDARV_MPEG4_SUB_FORMAT_WMV2;
		break;
		//DIVX1
	case CODEC_ID_MSMPEG4V1:
		m_info.format     = CEDARV_STREAM_FORMAT_MPEG4;
		m_info.sub_format = CEDARV_MPEG4_SUB_FORMAT_DIVX1;
		break;
		//DIVX2
	case CODEC_ID_MSMPEG4V2:
		m_info.format     = CEDARV_STREAM_FORMAT_MPEG4;
		m_info.sub_format = CEDARV_MPEG4_SUB_FORMAT_DIVX2;
		break;
		//DIVX3
	case CODEC_ID_MSMPEG4V3:
		m_info.format     = CEDARV_STREAM_FORMAT_MPEG4;
		m_info.sub_format = CEDARV_MPEG4_SUB_FORMAT_DIVX3;
		break;

		//*CEDARV_STREAM_FORMAT_REALVIDEO
	case CODEC_ID_RV10:
	case CODEC_ID_RV20:
	case CODEC_ID_RV30:
	case CODEC_ID_RV40:
		m_info.format = CEDARV_STREAM_FORMAT_REALVIDEO;
		break;

		//*CEDARV_STREAM_FORMAT_VC1
	case CODEC_ID_VC1:
		m_info.format	  = CEDARV_STREAM_FORMAT_VC1;
		break;

		//*CEDARV_STREAM_FORMAT_AVS

		//*CEDARV_STREAM_FORMAT_MJPEG
	case CODEC_ID_MJPEG:
		m_info.format = CEDARV_STREAM_FORMAT_MJPEG;
		break;

		//*CEDARV_STREAM_FORMAT_VP8
	case CODEC_ID_VP8:
		m_info.format = CEDARV_STREAM_FORMAT_VP8;
		break;

		//*
	default:
		CLog::Log(LOGERROR, "A10: codecid %d is unknown.\n", hints.codec);
		return false;
	}

	m_hcedarv = g_libbdv.libcedarv_init(&ret);
	if (ret < 0) {
		CLog::Log(LOGERROR, "A10: libcedarv_init failed. (%d)\n", ret);
		return false;
	}

	ret = m_hcedarv->set_vstream_info(m_hcedarv, &m_info);
	if (ret < 0) {
		CLog::Log(LOGERROR, "A10: set_vstream_m_info failed. (%d), id(%d), format(%d)\n", ret, hints.codec, m_info.format);
		return false;
	}

	ret = m_hcedarv->open(m_hcedarv);
	if(ret < 0) {
		CLog::Log(LOGERROR, "A10: open failed. (%d)\n", ret);
		return false;
	}

	ret = m_hcedarv->ioctrl(m_hcedarv, CEDARV_COMMAND_PLAY, 0);
	if (ret < 0) {
		CLog::Log(LOGERROR, "A10: CEDARV_COMMAND_PLAY failed. (%d)\n", ret);
		return false;
	}

	CLog::Log(LOGDEBUG, "A10: cedar open.");

	int width = 0, height = 0;
	double refresh =0.0;
	A10VLInit (width, height, refresh);

	/*
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
	pthread_create(&m_thread_t, &attr, thread_decode, this);
	pthread_attr_destroy(&attr);
	*/
	return true;
}

bool CDVDVideoCodecA10::DoOpen()
{
	s32 ret;

	printf(" A10 DoOpen\n");
	m_hcedarv = g_libbdv.libcedarv_init(&ret);
	if (ret < 0)
	{
		CLog::Log(LOGERROR, "A10: libcedarv_init failed. (%d)\n", ret);
		goto Error;
	}

	ret = m_hcedarv->set_vstream_info(m_hcedarv, &m_info);
	if (ret < 0)
	{
		CLog::Log(LOGERROR, "A10: set_vstream_info failed. (%d)\n", ret);
		goto Error;
	}

	ret = m_hcedarv->open(m_hcedarv);
	if (ret < 0)
	{
		CLog::Log(LOGERROR, "A10: open failed. (%d)\n", ret);
		goto Error;
	}

	ret = m_hcedarv->ioctrl(m_hcedarv, CEDARV_COMMAND_PLAY, 0);
	if (ret < 0)
	{
		CLog::Log(LOGERROR, "A10: CEDARV_COMMAND_PLAY failed. (%d)\n", ret);
		goto Error;
	}

	CLog::Log(LOGDEBUG, "A10: cedar open.");
	return true;

Error:

	Dispose();
	return false;
}

/*
* Dispose, Free all resources
*/
void CDVDVideoCodecA10::Dispose()
{
	CLog::Log(LOGDEBUG, "A10: cedar dispose.");
        A10VLFreeQueue();
	/*if (m_yuvdata)
	{
	free(m_yuvdata);
	m_yuvdata = NULL;
	}
	*/
	if (m_hcedarv)
	{
		m_hcedarv->ioctrl(m_hcedarv, CEDARV_COMMAND_STOP, 0);
		m_hcedarv->close(m_hcedarv);
		g_libbdv.libcedarv_exit(m_hcedarv);
		m_hcedarv = NULL;
		CLog::Log(LOGDEBUG, "A10: cedar dispose.");
	}

	if (m_convert_bitstream)
	{
		if (m_sps_pps_context.sps_pps_data)
		{
			free(m_sps_pps_context.sps_pps_data);
			m_sps_pps_context.sps_pps_data = NULL;
		}
	}
}

/*
* returns one or a combination of VC_ messages
* pData and iSize can be NULL, this means we should flush the rest of the data.
*/
int CDVDVideoCodecA10::Decode(BYTE* pData, int iSize, double dts, double pts)
{
	s32                        ret;
	u8                        *buf0, *buf1;
	u32                        bufsize0, bufsize1;
	cedarv_stream_data_info_t  dinf;
	cedarv_picture_t           picture;

	int demuxer_bytes = iSize;
	uint8_t *demuxer_content = pData;
	bool bitstream_convered  = false;

	if (!pData)
		return VC_BUFFER;

	if (!m_hcedarv)
		return VC_ERROR;

	//bitstream support
#if 0
	if (m_convert_bitstream)
	{
		// convert demuxer packet from bitstream to bytestream (AnnexB)
		int bytestream_size = 0;
		uint8_t *bytestream_buff = NULL;

		bitstream_convert(demuxer_content, demuxer_bytes, &bytestream_buff, &bytestream_size);
		if (bytestream_buff && (bytestream_size > 0))
		{
			bitstream_convered = true;
			demuxer_bytes = bytestream_size;
			demuxer_content = bytestream_buff;
		}
	}
#endif

	ret = m_hcedarv->request_write(m_hcedarv, demuxer_bytes, &buf0, &bufsize0, &buf1, &bufsize1);

	if(ret < 0)
	{
		CLog::Log(LOGERROR, "A10: request_write failed.\n");
		return VC_ERROR;
	}
	if (bufsize1)
	{
		memcpy(buf0, demuxer_content, bufsize0);
		memcpy(buf1, demuxer_content+bufsize0, bufsize1);
	}
	else
	{
		memcpy(buf0, demuxer_content, demuxer_bytes);
	}

	memset(&dinf, 0, sizeof(dinf));
	dinf.lengh = iSize;
#ifdef CEDARV_FLAG_DECODE_NO_DELAY
	dinf.flags = CEDARV_FLAG_FIRST_PART | CEDARV_FLAG_LAST_PART | CEDARV_FLAG_DECODE_NO_DELAY;
#else
	dinf.flags = CEDARV_FLAG_FIRST_PART | CEDARV_FLAG_LAST_PART;
#endif

        dinf.pts = llrint(pts);
        dinf.flags |= CEDARV_FLAG_PTS_VALID;
	m_hcedarv->update_data(m_hcedarv, &dinf);

	ret = m_hcedarv->decode(m_hcedarv);


	//bitstream support
	if (bitstream_convered)
		free(demuxer_content);

	if (ret > 3 || ret < 0)
	{
		CLog::Log(LOGERROR, "A10: decode(%d): %d\n", iSize, ret);
	}

	if (ret == 4)
	{
		CLog::Log(LOGNOTICE, "A10: Out of decoder frame buffers. Freeing the queue.\n");
                A10VLFreeQueue();

		m_hcedarv->decode(m_hcedarv);
	}

	ret = m_hcedarv->display_request(m_hcedarv, &picture);

	if (ret > 3 || ret < -1)
	{
		CLog::Log(LOGERROR, "A10: display_request(%d): %d\n", iSize, ret);
	}

	if (ret == 0)
	{
		float aspect_ratio = m_aspect;
		m_picture.pts     = pts;
		m_picture.dts     = dts;
		m_picture.iWidth  = picture.display_width;
		m_picture.iHeight = picture.display_height;

		if (picture.is_progressive) m_picture.iFlags &= ~DVP_FLAG_INTERLACED;
		else                        m_picture.iFlags |= DVP_FLAG_INTERLACED;

		/* XXX: we suppose the screen has a 1.0 pixel ratio */ // CDVDVideo will compensate it.
		if (aspect_ratio <= 0.0)
			aspect_ratio = (float)m_picture.iWidth / (float)m_picture.iHeight;

		m_picture.iDisplayHeight = m_picture.iHeight;
		m_picture.iDisplayWidth  = ((int)lrint(m_picture.iHeight * aspect_ratio)) & -3;
		if (m_picture.iDisplayWidth > m_picture.iWidth)
		{
			m_picture.iDisplayWidth  = m_picture.iWidth;
			m_picture.iDisplayHeight = ((int)lrint(m_picture.iWidth / aspect_ratio)) & -3;
		}
#if 0
		{
			u32 width32;
			u32 height32;
			u32 height64;
			u32 ysize;
			u32 csize;

			m_picture.format = RENDER_FMT_YUV420P;

			width32  = (picture.display_width  + 31) & ~31;
			height32 = (picture.display_height + 31) & ~31;
			height64 = (picture.display_height + 63) & ~63;

			ysize = width32*height32;   //* for y.
			csize = width32*height64/2; //* for u and v together.

			int             display_height_align;
			int             display_width_align;
			int             dst_c_stride;
			int             dst_y_size;
			int             dst_c_size;
			int             alloc_size;

			picture.display_height = (picture.display_height + 7) & (~7);
			display_height_align = (picture.display_height + 1) & (~1);
			display_width_align  = (picture.display_width + 15) & (~15);
			dst_y_size           = display_width_align * display_height_align;
			dst_c_stride         = (picture.display_width/2 + 15) & (~15);
			dst_c_size           = dst_c_stride * (display_height_align/2);
			alloc_size           = dst_y_size + dst_c_size * 2;


			if (!m_yuvdata) {
				m_yuvdata = (u8*)calloc(alloc_size, 1);
				if (!m_yuvdata) {
					CLog::Log(LOGERROR, "A10: can not alloc m_yuvdata!");
					m_hcedarv->display_release(m_hcedarv, picture.id);
					return VC_ERROR;
				}
			}
			TransformToYUVPlaner (&picture, m_yuvdata, display_height_align, display_width_align, dst_c_stride, dst_y_size, dst_c_size);
			if (!(m_picture.iFlags & DVP_FLAG_ALLOCATED)) {
				u32 width16  = (picture.display_width  + 15) & ~15;

				m_picture.iFlags |= DVP_FLAG_ALLOCATED;

				m_picture.iLineSize[0] = display_width_align;   //Y
				m_picture.iLineSize[1] = dst_c_stride; //U
				m_picture.iLineSize[2] = dst_c_stride; //V
				m_picture.iLineSize[3] = 0;

				m_picture.data[0] = m_yuvdata;
				m_picture.data[1] = m_yuvdata+dst_y_size + dst_c_size;
				m_picture.data[2] = m_yuvdata+dst_y_size;

			}


			m_hcedarv->display_release(m_hcedarv, picture.id);
		}
#else
                //u8 *y_p = (u8 *)g_libbdv.mem_palloc (picture.size_y, 1024);
                //u8 *u_p = (u8 *)g_libbdv.mem_palloc (picture.size_u, 1024);
                //memcpy (y_p, picture.y, picture.size_y);
                //memcpy (u_p, picture.u, picture.size_u);
		//m_hcedarv->display_release(m_hcedarv, picture.id);
                //picture.y = y_p;
                //picture.u = u_p; 
		m_picture.format     = RENDER_FMT_A10BUF;
		m_picture.a10buffer  = A10VLPutQueue(freecallback, (void*)this, NULL, picture);
		m_picture.iFlags    |= DVP_FLAG_ALLOCATED;
#endif

		return VC_PICTURE | VC_BUFFER;
	}

	CLog::Log(LOGINFO, "A10: Decode need more data");

	return VC_BUFFER;
}

/*
* Reset the decoder.
* Should be the same as calling Dispose and Open after each other
*/
void CDVDVideoCodecA10::Reset()
{
	CLog::Log(LOGDEBUG, "A10: reset requested");
	m_hcedarv->ioctrl(m_hcedarv, CEDARV_COMMAND_RESET, 0);
}

/*
* returns true if successfull
* the data is valid until the next Decode call
*/
bool CDVDVideoCodecA10::GetPicture(DVDVideoPicture* pDvdVideoPicture)
{
	if (m_picture.iFlags & DVP_FLAG_ALLOCATED)
	{
		*pDvdVideoPicture = m_picture;
		return true;
	}
	return false;
}

void CDVDVideoCodecA10::SetDropState(bool bDrop)
{
}

const char* CDVDVideoCodecA10::GetName()
{
	return "A10";
}

void CDVDVideoCodecA10::FreePicture(void *pictpriv, cedarv_picture_t &pict)
{
	m_hcedarv->display_release(m_hcedarv, pict.id);
        //g_libbdv.mem_pfree (pict.y);
        //g_libbdv.mem_pfree (pict.u);        
}

////////////////////////////////////////////////////////////////////////////////////////////
bool CDVDVideoCodecA10::bitstream_convert_init(void *in_extradata, int in_extrasize)
{
	// based on h264_mp4toannexb_bsf.c (ffmpeg)
	// which is Copyright (c) 2007 Benoit Fouet <benoit.fouet@free.fr>
	// and Licensed GPL 2.1 or greater

	m_sps_pps_size = 0;
	m_sps_pps_context.sps_pps_data = NULL;

	// nothing to filter
	if (!in_extradata || in_extrasize < 6)
		return false;

	uint16_t unit_size;
	uint32_t total_size = 0;
	uint8_t *out = NULL, unit_nb, sps_done = 0;
	const uint8_t *extradata = (uint8_t*)in_extradata + 4;
	static const uint8_t nalu_header[4] = {0, 0, 0, 1};

	// retrieve length coded size
	m_sps_pps_context.length_size = (*extradata++ & 0x3) + 1;
	if (m_sps_pps_context.length_size == 3)
	{
		CLog::Log(LOGDEBUG, "A10: bitstream_convert_init length_size.");
		return false;
	}

	// retrieve sps and pps unit(s)
	unit_nb = *extradata++ & 0x1f;  // number of sps unit(s)
	if (!unit_nb)
	{
		unit_nb = *extradata++;       // number of pps unit(s)
		sps_done++;
	}
	while (unit_nb--)
	{
		unit_size = extradata[0] << 8 | extradata[1];
		total_size += unit_size + 4;
		if ( (extradata + 2 + unit_size) > ((uint8_t*)in_extradata + in_extrasize) )
		{
			free(out);
			return false;
		}
		out = (uint8_t*)realloc(out, total_size);
		if (!out)
			return false;

		memcpy(out + total_size - unit_size - 4, nalu_header, 4);
		memcpy(out + total_size - unit_size, extradata + 2, unit_size);
		extradata += 2 + unit_size;

		if (!unit_nb && !sps_done++)
			unit_nb = *extradata++;     // number of pps unit(s)
	}

	m_sps_pps_context.sps_pps_data = out;
	m_sps_pps_context.size = total_size;
	m_sps_pps_context.first_idr = 1;

	return true;
}

bool CDVDVideoCodecA10::bitstream_convert(BYTE* pData, int iSize, uint8_t **poutbuf, int *poutbuf_size)
{
	// based on h264_mp4toannexb_bsf.c (ffmpeg)
	// which is Copyright (c) 2007 Benoit Fouet <benoit.fouet@free.fr>
	// and Licensed GPL 2.1 or greater

	uint8_t *buf = pData;
	uint32_t buf_size = iSize;
	uint8_t  unit_type;
	int32_t  nal_size;
	uint32_t cumul_size = 0;
	const uint8_t *buf_end = buf + buf_size;

	do
	{
		if (buf + m_sps_pps_context.length_size > buf_end)
			goto fail;

		if (m_sps_pps_context.length_size == 1)
			nal_size = buf[0];
		else if (m_sps_pps_context.length_size == 2)
			nal_size = buf[0] << 8 | buf[1];
		else
			nal_size = buf[0] << 24 | buf[1] << 16 | buf[2] << 8 | buf[3];

		buf += m_sps_pps_context.length_size;
		unit_type = *buf & 0x1f;

		if (buf + nal_size > buf_end || nal_size < 0)
			goto fail;

		// prepend only to the first type 5 NAL unit of an IDR picture
		if (m_sps_pps_context.first_idr && unit_type == 5)
		{
			bitstream_alloc_and_copy(poutbuf, poutbuf_size,
				m_sps_pps_context.sps_pps_data, m_sps_pps_context.size, buf, nal_size);
			m_sps_pps_context.first_idr = 0;
		}
		else
		{
			bitstream_alloc_and_copy(poutbuf, poutbuf_size, NULL, 0, buf, nal_size);
			if (!m_sps_pps_context.first_idr && unit_type == 1)
				m_sps_pps_context.first_idr = 1;
		}

		buf += nal_size;
		cumul_size += nal_size + m_sps_pps_context.length_size;
	} while (cumul_size < buf_size);

	return true;

fail:
	free(*poutbuf);
	*poutbuf = NULL;
	*poutbuf_size = 0;
	return false;
}

void CDVDVideoCodecA10::bitstream_alloc_and_copy(
	uint8_t **poutbuf,      int *poutbuf_size,
	const uint8_t *sps_pps, uint32_t sps_pps_size,
	const uint8_t *in,      uint32_t in_size)
{
	// based on h264_mp4toannexb_bsf.c (ffmpeg)
	// which is Copyright (c) 2007 Benoit Fouet <benoit.fouet@free.fr>
	// and Licensed GPL 2.1 or greater

#define CHD_WB32(p, d) { \
	((uint8_t*)(p))[3] = (d); \
	((uint8_t*)(p))[2] = (d) >> 8; \
	((uint8_t*)(p))[1] = (d) >> 16; \
	((uint8_t*)(p))[0] = (d) >> 24; }

	uint32_t offset = *poutbuf_size;
	uint8_t nal_header_size = offset ? 3 : 4;

	*poutbuf_size += sps_pps_size + in_size + nal_header_size;
	*poutbuf = (uint8_t*)realloc(*poutbuf, *poutbuf_size);
	if (sps_pps)
		memcpy(*poutbuf + offset, sps_pps, sps_pps_size);

	memcpy(*poutbuf + sps_pps_size + nal_header_size + offset, in, in_size);
	if (!offset)
	{
		CHD_WB32(*poutbuf + sps_pps_size, 1);
	}
	else
	{
		(*poutbuf + offset + sps_pps_size)[0] = 0;
		(*poutbuf + offset + sps_pps_size)[1] = 0;
		(*poutbuf + offset + sps_pps_size)[2] = 1;
	}
}
