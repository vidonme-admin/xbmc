#if defined(__ANDROID_ALLWINNER__)

#include "DVDVideoCodecAllWinner.h"
#include "DVDClock.h"
#include "utils/log.h"
#include "threads/Atomics.h"

#include <sys/ioctl.h>
#include <math.h>
#include "transform_color_format.h"

#include "cores/VideoRenderers/LinuxRendererGLES.h"
#include "cores/VideoRenderers/RenderFlags.h"

#include "Application.h"

#define AllWinnerDEBUG
#define MEDIAINFO

extern void AllWinnerVLExit (); 
extern bool AllWinnerVLInit (float aspect, int displayw, int displayh, int &width, int &height);

static ve_mutex_t  m_cedarv_req_ctx;
static int a31_use_direct_render_output = false; //temporary flag
static bool m_decoderUseYV12 = false;

int m_allwinner_productType = CApplication::ALLWINNER_UNKNOWN; 

#define _4CC(c1,c2,c3,c4) (((u32)(c4)<<24)|((u32)(c3)<<16)|((u32)(c2)<<8)|(u32)(c1))

static void freecallback (void *callbackpriv, void *pictpriv, cedarv_picture_t &pict)
{
  ((CDVDVideoCodecAllWinner*)callbackpriv)->FreePicture (pictpriv, pict);
}

CDVDVideoCodecAllWinner::CDVDVideoCodecAllWinner ()
{
  m_last_alloc_size = 0;
  m_hcedarv  = NULL;
  m_yuvdata  = NULL;
  memset(&m_picture, 0, sizeof(m_picture));

  m_productType = GetAllWinnerProductType (); 
  if (m_productType == CApplication::ALLWINNER_A31_PAD)
  {
    if (!g_libbdv_allwinner.IsLoaded ())
    {
      g_libbdv_allwinner.EnableDelayedUnload (false);
      if (!g_libbdv_allwinner.Load ())
      {
        CLog::Log (LOGDEBUG, "### AllWinner Load Codec Failed ###");
      }
    }

    if (!g_libbdv_allwinner_osal.IsLoaded ())
    {
      g_libbdv_allwinner_osal.EnableDelayedUnload (false);
      if (!g_libbdv_allwinner_osal.Load ())
      {
        CLog::Log (LOGDEBUG, "### AllWinner Load osal Failed ###");
      }
    }

    if (!g_libbdv_allwinner_base.IsLoaded ())
    {
      g_libbdv_allwinner_base.EnableDelayedUnload (false);
      if (!g_libbdv_allwinner_base.Load ())
      {
        CLog::Log (LOGDEBUG, "### AllWinner Load base Failed ###");
      }
    }

    if (!g_libbdv_allwinner_sunxi.IsLoaded ())
    {
      g_libbdv_allwinner_sunxi.EnableDelayedUnload (false);
      if (!g_libbdv_allwinner_sunxi.Load ())
      {
        CLog::Log (LOGDEBUG, "### AllWinner Load sunxi Failed ###");
      }
    }
  }
  else 
  {
    if (!g_libbdv_a10_xbmc_minipc.IsLoaded ())
    {
      g_libbdv_a10_xbmc_minipc.EnableDelayedUnload (false);
      if (!g_libbdv_a10_xbmc_minipc.Load ())
      {
        CLog::Log (LOGDEBUG, "### AllWinner Load sunxi Failed ###");
      }
    }
  }
}

CDVDVideoCodecAllWinner::~CDVDVideoCodecAllWinner ()
{
  Dispose ();
  
  if (m_productType != CApplication::ALLWINNER_A31_PAD)
    AllWinnerVLExit ();
}

int CDVDVideoCodecAllWinner::GetAllWinnerProductType ()
{
  static const char* filter1 = "sun6i";
  static const char* filter2 = "sun4i";

  if (m_allwinner_productType != CApplication::ALLWINNER_UNKNOWN)
    return m_allwinner_productType;

  FILE* pipe = popen("busybox cat /proc/cpuinfo", "r");

  if (pipe)
  {
    char line[1024];

    while (fgets (line, sizeof (line) - 1, pipe))
    {
      if (strstr (line, filter1))
      {
        m_allwinner_productType = CApplication::ALLWINNER_A31_PAD;
        break;
      }
      else if (strstr (line, filter2))
      {
        m_allwinner_productType = CApplication::ALLWINNER_A10_MINIPC;
        break;
      }
    }

    pclose (pipe);
  }

  return m_allwinner_productType;
}

bool CDVDVideoCodecAllWinner::Open (CDVDStreamInfo &hints, CDVDCodecOptions &options)
{

  if (m_productType == CApplication::ALLWINNER_A31_PAD)
  {
    if (!g_libbdv_allwinner.IsLoaded () || !g_libbdv_allwinner_osal.IsLoaded () || !g_libbdv_allwinner_base.IsLoaded () || !g_libbdv_allwinner_sunxi.IsLoaded ())
    {
      CLog::Log (LOGERROR, "Load failed, cannot start the codec!");
      return false;
    }
  }
  else 
  {
    if (!g_libbdv_a10_xbmc_minipc.IsLoaded ())
    {
      CLog::Log (LOGERROR, "Load failed, cannot start the codec!");
      return false;
    }
  }

  CLog::Log(LOGDEBUG, "--------------------- CDVDVideoCodecAllWinner: Open, %d------------------------ ", hints.codec);

  s32 ret;

  m_aspect = hints.aspect;

  memset (&m_info, 0, sizeof (m_info));

  m_info.frame_duration = 0;
  m_info.video_width = hints.width;
  m_info.video_height = hints.height;
  m_info.aspect_ratio = 1000;
  m_info.sub_format = CEDARV_SUB_FORMAT_UNKNOW;
  m_info.container_format = CEDARV_CONTAINER_FORMAT_UNKNOW;
  m_info.init_data_len = 0;
  m_info.init_data = NULL;

  if (hints.extradata && hints.extrasize > 0)
  {
    m_info.init_data_len = hints.extrasize;
    m_info.init_data = (u8 *)hints.extradata;
  }

  switch (hints.codec) {
    case CODEC_ID_MPEG1VIDEO:
      m_info.format     = CEDARV_STREAM_FORMAT_MPEG2;
      break;

    case CODEC_ID_MPEG2VIDEO:
      m_info.format     = CEDARV_STREAM_FORMAT_MPEG2;
      break;
    
    case CODEC_ID_H264:
      m_info.format = CEDARV_STREAM_FORMAT_H264;
      break;
    
    case CODEC_ID_MPEG4:
      m_info.format = CEDARV_STREAM_FORMAT_MPEG4;
      m_info.sub_format = CEDARV_MPEG4_SUB_FORMAT_XVID;
      break;
    
    case CODEC_ID_VP6F:
      m_info.format     = CEDARV_STREAM_FORMAT_MPEG4;
      m_info.sub_format = CEDARV_MPEG4_SUB_FORMAT_VP6;
      break;
    
    case CODEC_ID_WMV1:
      m_info.format     = CEDARV_STREAM_FORMAT_MPEG4;
      m_info.sub_format = CEDARV_MPEG4_SUB_FORMAT_WMV1;
      break;
    
    case CODEC_ID_WMV2:
      m_info.format     = CEDARV_STREAM_FORMAT_MPEG4;
      m_info.sub_format = CEDARV_MPEG4_SUB_FORMAT_WMV2;
      break;
    
    case CODEC_ID_MSMPEG4V1:
      m_info.format     = CEDARV_STREAM_FORMAT_MPEG4;
      m_info.sub_format = CEDARV_MPEG4_SUB_FORMAT_DIVX1;
      break;
    
    case CODEC_ID_MSMPEG4V2:
      m_info.format     = CEDARV_STREAM_FORMAT_MPEG4;
      m_info.sub_format = CEDARV_MPEG4_SUB_FORMAT_DIVX2;
      break;
    
    case CODEC_ID_MSMPEG4V3:
      m_info.format     = CEDARV_STREAM_FORMAT_MPEG4;
      m_info.sub_format = CEDARV_MPEG4_SUB_FORMAT_DIVX3;
      break;
    
    case CODEC_ID_RV10:
    case CODEC_ID_RV20:
    case CODEC_ID_RV30:
    case CODEC_ID_RV40:
      m_info.format = CEDARV_STREAM_FORMAT_REALVIDEO;
      break;
    
    case CODEC_ID_VC1:
      m_info.format	  = CEDARV_STREAM_FORMAT_VC1;
      break;
    
    case CODEC_ID_MJPEG:
      m_info.format = CEDARV_STREAM_FORMAT_MJPEG;
      break;
    
    case CODEC_ID_VP8:
      m_info.format = CEDARV_STREAM_FORMAT_VP8;
      break;
    
    default:
      CLog::Log(LOGERROR, "AllWinner: codecid %d is unknown.\n", hints.codec);
      return false;
  }

  if (m_productType == CApplication::ALLWINNER_A31_PAD)
  {
    if (g_libbdv_allwinner_base.ve_mutex_init (&m_cedarv_req_ctx, CEDARV_DECODE) < 0) {
      CLog::Log (LOGERROR, "AllWinner: ve_mutex_init failed. (%d)\n", ret);
      return false;
    }
    
    m_hcedarv = g_libbdv_allwinner.libcedarv_init (&ret);
    if (ret < 0) {
      CLog::Log (LOGERROR, "AllWinner: libcedarv_init failed. (%d)\n", ret);
      return false;
    }

    //m_decoderUseYV12 = true;
    //m_hcedarv->ioctrl (m_hcedarv, CEDARV_COMMAND_SET_PIXEL_FORMAT, 1);
  }
  else 
  {
    m_hcedarv = g_libbdv_a10_xbmc_minipc.libcedarv_init (&ret);
    if (ret < 0) {
      CLog::Log (LOGERROR, "AllWinner: minipc libcedarv_init failed. (%d)\n", ret);
      return false;
    }
  }
  
  ret = m_hcedarv->set_vstream_info (m_hcedarv, &m_info);
  if (ret < 0) {
    CLog::Log(LOGERROR, "AllWinner: set_vstream_m_info failed. (%d), id(%d), format(%d)\n", ret, hints.codec, m_info.format);
    return false;
  }

  ret = m_hcedarv->open (m_hcedarv);
  if(ret < 0) {
    CLog::Log (LOGERROR, "AllWinner: open failed. (%d)\n", ret);
    return false;
  }
  
  ret = m_hcedarv->ioctrl (m_hcedarv, CEDARV_COMMAND_PLAY, 0);
  if (ret < 0) {
    CLog::Log (LOGERROR, "AllWinner: CEDARV_COMMAND_PLAY failed. (%d)\n", ret);
    return false;
  }
  
  CLog::Log(LOGDEBUG, "AllWinner: cedar open.");
  
  int width = 0, height = 0;

  if (m_productType != CApplication::ALLWINNER_A31_PAD)
    AllWinnerVLInit (hints.aspect, hints.width, hints.height, width, height);
  
  return true;
}

/*
* Dispose, Free all resources
*/
void CDVDVideoCodecAllWinner::Dispose()
{
  CLog::Log (LOGDEBUG, "AllWinner: cedar dispose.");
  AllWinnerVLFreeQueue ();

  if (m_yuvdata)
  {
    free (m_yuvdata);
    m_yuvdata = NULL;
  }
  	
  if (m_hcedarv)
  {
    m_hcedarv->ioctrl (m_hcedarv, CEDARV_COMMAND_STOP, 0);
    m_hcedarv->close (m_hcedarv);

    if (m_productType == CApplication::ALLWINNER_A31_PAD)
    {
      g_libbdv_allwinner.libcedarv_exit (m_hcedarv);
      g_libbdv_allwinner_base.ve_mutex_destroy (&m_cedarv_req_ctx);
    }
    else
    {
      g_libbdv_a10_xbmc_minipc.libcedarv_exit (m_hcedarv);
    }

    m_hcedarv = NULL;
    CLog::Log (LOGDEBUG, "AllWinner: cedar dispose.");
  }
}

/*
* returns one or a combination of VC_ messages
* pData and iSize can be NULL, this means we should flush the rest of the data.
*/
int CDVDVideoCodecAllWinner::Decode (BYTE* pData, int iSize, double dts, double pts)
{
  s32                        ret;
  u8                        *buf0, *buf1;
  u32                        bufsize0, bufsize1;
  cedarv_stream_data_info_t  dinf;
  cedarv_picture_t           picture;
  
  int demuxer_bytes = iSize;
  uint8_t *demuxer_content = pData;

  if (!pData)
    return VC_BUFFER;
  
  if (!m_hcedarv)
    return VC_ERROR;
 
  ret = m_hcedarv->request_write (m_hcedarv, demuxer_bytes, &buf0, &bufsize0, &buf1, &bufsize1);

  if (ret < 0)
  {
    CLog::Log (LOGERROR, "AllWinner: request_write failed.\n");
    return VC_ERROR;
  }

  if (bufsize1)
  {
    memcpy (buf0, demuxer_content, bufsize0);
    memcpy (buf1, demuxer_content + bufsize0, bufsize1);
  }
  else
  {
    memcpy (buf0, demuxer_content, demuxer_bytes);
  }

  memset (&dinf, 0, sizeof (dinf));
  dinf.lengh = demuxer_bytes;
  dinf.flags = CEDARV_FLAG_FIRST_PART | CEDARV_FLAG_LAST_PART;

  if (DVD_NOPTS_VALUE != pts || DVD_NOPTS_VALUE != dts)
  {
    dinf.pts = llrint((pts != DVD_NOPTS_VALUE) ? pts : dts);
    dinf.flags |= CEDARV_FLAG_PTS_VALID;
  }

  m_hcedarv->update_data (m_hcedarv, &dinf);

  ret = m_hcedarv->decode (m_hcedarv);

  if (ret > 3 || ret < 0)
  {
    CLog::Log (LOGERROR, "AllWinner: decode(%d): %d, %f/%f\n", iSize, ret, pts, dts);
  }
  
  if (ret == 4)
  {
    CLog::Log (LOGNOTICE, "AllWinner: Out of decoder frame buffers. Freeing the queue.\n");

    if (m_productType != CApplication::ALLWINNER_A31_PAD)
    {
      //AllWinnerVLFreeAQueue ();
      //m_hcedarv->decode (m_hcedarv);
    }
  }

  if (m_hcedarv->picture_ready (m_hcedarv))
  {
    ret = m_hcedarv->display_request (m_hcedarv, &picture);

    if (ret > 3 || ret < -1)
    {
      CLog::Log (LOGERROR, "AllWinner: display_request(%d): %d\n", iSize, ret);
    }

    if (ret == 0)
    {
      float aspect_ratio = m_aspect;
      m_picture.pts     = pts;
      m_picture.dts     = dts;
      m_picture.iWidth  = picture.display_width;
      m_picture.iHeight = picture.display_height;

      if (picture.is_progressive) 
        m_picture.iFlags &= ~DVP_FLAG_INTERLACED;
      else                        
        m_picture.iFlags |= DVP_FLAG_INTERLACED;

      if (aspect_ratio <= 0.0)
        aspect_ratio = (float)m_picture.iWidth / (float)m_picture.iHeight;

      m_picture.iDisplayHeight = m_picture.iHeight;
      m_picture.iDisplayWidth  = ((int)lrint (m_picture.iHeight * aspect_ratio)) & -3;
      if (m_picture.iDisplayWidth > m_picture.iWidth)
      {
        m_picture.iDisplayWidth  = m_picture.iWidth;
        m_picture.iDisplayHeight = ((int)lrint (m_picture.iWidth / aspect_ratio)) & -3;
      }

      if (m_productType == CApplication::ALLWINNER_A31_PAD)
      {
        if (!a31_use_direct_render_output)
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
            m_yuvdata = (u8*)calloc (alloc_size, 1);
            if (!m_yuvdata) {
              CLog::Log(LOGERROR, "AllWinner: can not alloc m_yuvdata!");
              m_hcedarv->display_release (m_hcedarv, picture.id);
              return VC_ERROR;
            }
          }

          if (m_decoderUseYV12)
          {
            picture.y = (u8 *)g_libbdv_allwinner_osal.cedarv_address_phy2vir (picture.y);
            TransformToGPUBuffer (&picture, m_yuvdata);
          }
          else
          {
            picture.y = (u8 *)g_libbdv_allwinner_osal.cedarv_address_phy2vir (picture.y);
            picture.u = (u8 *)g_libbdv_allwinner_osal.cedarv_address_phy2vir (picture.u);

            TransformToYUVPlaner (&picture, m_yuvdata, display_height_align, display_width_align, dst_c_stride, dst_y_size, dst_c_size);
          }

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
          
          m_hcedarv->display_release (m_hcedarv, picture.id);
        }
        else
        {
/*
          int y_p = 0;
          int u_p = 0;
          if (picture.size_y)
          {
            y_p = (int)g_libbdv_allwinner_sunxi.sunxi_alloc_alloc (picture.size_y);
            memcpy ((void *)y_p, (void *)g_libbdv_allwinner_osal.cedarv_address_phy2vir(picture.y), picture.size_y);
            picture.y = (u8 *)g_libbdv_allwinner_sunxi.sunxi_alloc_vir2phy ((void *)y_p);
          }
          else
            picture.y = 0;
          
          if (picture.size_u)
          {
            u_p = (int)g_libbdv_allwinner_sunxi.sunxi_alloc_alloc (picture.size_u);
            memcpy ((void *)u_p, (void *)g_libbdv_allwinner_osal.cedarv_address_phy2vir(picture.u), picture.size_u);
            picture.u = (u8 *)g_libbdv_allwinner_sunxi.sunxi_alloc_vir2phy ((void *)u_p); 
          }
          else
            picture.u = 0;
          
          g_libbdv_allwinner_sunxi.sunxi_flush_cache_all ();
          
          m_hcedarv->display_release (m_hcedarv, picture.id);
*/
          m_picture.format     = RENDER_FMT_ALLWINNER;
          m_picture.allWinnerBuffer  = AllWinnerVLPutQueue (freecallback, (void*)this, NULL, picture);
          m_picture.iFlags    |= DVP_FLAG_ALLOCATED;
        }
      }
      else //if (m_productType == CApplication::ALLWINNER_A31_PAD)
      { 
        m_picture.format     = RENDER_FMT_ALLWINNER;
        m_picture.allWinnerBuffer  = AllWinnerVLPutQueue (freecallback, (void*)this, NULL, picture);
        m_picture.iFlags    |= DVP_FLAG_ALLOCATED;
      }
      return VC_PICTURE | VC_BUFFER;
    }
  }

  return VC_BUFFER;
}

/*
* Reset the decoder.
* Should be the same as calling Dispose and Open after each other
*/
void CDVDVideoCodecAllWinner::Reset ()
{
  CLog::Log (LOGDEBUG, "AllWinner: reset requested");
  m_hcedarv->ioctrl (m_hcedarv, CEDARV_COMMAND_RESET, 0);
}

/*
* returns true if successfull
* the data is valid until the next Decode call
*/
bool CDVDVideoCodecAllWinner::GetPicture (DVDVideoPicture* pDvdVideoPicture)
{
  if (m_picture.iFlags & DVP_FLAG_ALLOCATED)
  {
    *pDvdVideoPicture = m_picture;
    return true;
  }

  return false;
}

void CDVDVideoCodecAllWinner::SetDropState (bool bDrop)
{
}

const char* CDVDVideoCodecAllWinner::GetName ()
{
  return "AllWinner";
}

void CDVDVideoCodecAllWinner::FreePicture (void *pictpriv, cedarv_picture_t &pict)
{
 /* if (m_productType == CApplication::ALLWINNER_A31_PAD)
  {
    if (pict.y)
      g_libbdv_allwinner_sunxi.sunxi_alloc_free ((void *)g_libbdv_allwinner_osal.cedarv_address_phy2vir (pict.y));
    if (pict.u)
      g_libbdv_allwinner_sunxi.sunxi_alloc_free ((void *)g_libbdv_allwinner_osal.cedarv_address_phy2vir (pict.u));        
  }
  else  */
  {
    m_hcedarv->display_release (m_hcedarv, pict.id);
  }
}

#endif
