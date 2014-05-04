#pragma once
/*
 *      Copyright (C) 2012 Team XBMC
 *      http://www.xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */
#ifndef __DLLLIBCEDARVH__
#define __DLLLIBCEDARVH__

#include "system.h"

#include "DynamicDll.h"


////////////////////////////////////////////////////////////////////////////////////////////

typedef enum CEDARV_USAGE
{
  CEDARV_UNKNOWN           = 0,
  CEDARV_ENCODE_BACKGROUND = 1,
  CEDARV_DECODE_BACKGROUND = 2,
  CEDARV_ENCODE            = 3,
  CEDARV_DECODE            = 4,
}cedarv_usage_t;

typedef void* ve_mutex_t;

class DllLibAllWinnerA10XBMCDecoderInterface
{
public:
  virtual ~DllLibAllWinnerA10XBMCDecoderInterface() {};

  virtual cedarv_decoder_t* libcedarv_init(int *ret)=0;
  virtual int libcedarv_exit(cedarv_decoder_t *p)=0;
  virtual int mem_get_phy_addr(int addr)=0;
};

class DllLibAllWinnerA10XBMCDecoder : public DllDynamic, DllLibAllWinnerA10XBMCDecoderInterface
{
  DECLARE_DLL_WRAPPER(DllLibAllWinnerA10XBMCDecoder, "libbdv.so")

  DEFINE_METHOD1(cedarv_decoder_t *, libcedarv_init, (int *p1))
  DEFINE_METHOD1(int, libcedarv_exit, (cedarv_decoder_t *p1))
  DEFINE_METHOD1(int,            mem_get_phy_addr, (int p1))

  BEGIN_METHOD_RESOLVE()
    RESOLVE_METHOD(libcedarv_init)
    RESOLVE_METHOD(libcedarv_exit)
    RESOLVE_METHOD(mem_get_phy_addr)
  END_METHOD_RESOLVE()
};

////////////////////////////////////////////////////////////////////////////////////////////

class DllLibAllWinnerDecoderInterface
{
public:
  virtual ~DllLibAllWinnerDecoderInterface() {};

  virtual cedarv_decoder_t* libcedarv_init(int *ret)=0;
  virtual int libcedarv_exit(cedarv_decoder_t *p)=0;
};

class DllLibAllWinnerAllWinnerDecoder : public DllDynamic, DllLibAllWinnerDecoderInterface
{
  DECLARE_DLL_WRAPPER(DllLibAllWinnerAllWinnerDecoder, "/system/lib/libcedarv.so")

  DEFINE_METHOD1(cedarv_decoder_t *, libcedarv_init, (int *p1))
  DEFINE_METHOD1(int, libcedarv_exit, (cedarv_decoder_t *p1))

  BEGIN_METHOD_RESOLVE()
    RESOLVE_METHOD(libcedarv_init)
    RESOLVE_METHOD(libcedarv_exit)
  END_METHOD_RESOLVE()
};

////////////////////////////////////////////////////////////////////////////////////////////

class DllLibAllWinnerOsalInterface
{
public:
  virtual ~DllLibAllWinnerOsalInterface() {};

  virtual unsigned int cedarv_address_phy2vir (void *addr) = 0;
};

class DllLibAllWinnerAllWinnerOsal : public DllDynamic, DllLibAllWinnerOsalInterface
{
  DECLARE_DLL_WRAPPER(DllLibAllWinnerAllWinnerOsal, "/system/lib/libcedarxosal.so")

  DEFINE_METHOD1(unsigned int,  cedarv_address_phy2vir, (void *p1))

  BEGIN_METHOD_RESOLVE()
    RESOLVE_METHOD(cedarv_address_phy2vir)
  END_METHOD_RESOLVE()
};

////////////////////////////////////////////////////////////////////////////////////////////

class DllLibAllWinnerBaseInterface
{
public:
  virtual ~DllLibAllWinnerBaseInterface() {};

  virtual int ve_mutex_init (ve_mutex_t* mutex, cedarv_usage_t usage) = 0;
  virtual void ve_mutex_destroy (ve_mutex_t* mutex) = 0;
};

class DllLibAllWinnerAllWinnerBase : public DllDynamic, DllLibAllWinnerBaseInterface
{
  DECLARE_DLL_WRAPPER(DllLibAllWinnerAllWinnerBase, "/system/lib/libcedarxbase.so")

  DEFINE_METHOD2(int, ve_mutex_init, (ve_mutex_t *p1, cedarv_usage_t p2))
  DEFINE_METHOD1(void, ve_mutex_destroy, (ve_mutex_t *p1))

  BEGIN_METHOD_RESOLVE()
    RESOLVE_METHOD(ve_mutex_init)
    RESOLVE_METHOD(ve_mutex_destroy)
  END_METHOD_RESOLVE()
};

////////////////////////////////////////////////////////////////////////////////////////////

class DllLibAllWinnerSunxiInterface
{
public:
  virtual ~DllLibAllWinnerSunxiInterface() {};
 
  virtual int sunxi_alloc_alloc (int size) = 0;
  virtual void sunxi_alloc_free (void * pbuf) = 0;
  virtual int sunxi_alloc_vir2phy (void * pbuf) = 0;
  virtual int sunxi_alloc_phy2vir (void * pbuf) = 0;
  virtual void sunxi_flush_cache_all () = 0;
};

class DllLibAllWinnerAllWinnerSunxi : public DllDynamic, DllLibAllWinnerSunxiInterface
{
  DECLARE_DLL_WRAPPER(DllLibAllWinnerAllWinnerSunxi, "/system/lib/libsunxi_alloc.so")

  DEFINE_METHOD1(int, sunxi_alloc_alloc, (int p1))
  DEFINE_METHOD1(void, sunxi_alloc_free, (void *p1))
  DEFINE_METHOD1(int, sunxi_alloc_vir2phy, (void *p1))
  DEFINE_METHOD1(int, sunxi_alloc_phy2vir, (void *p1))
  DEFINE_METHOD0(void, sunxi_flush_cache_all)

  BEGIN_METHOD_RESOLVE()
    RESOLVE_METHOD(sunxi_alloc_alloc)
    RESOLVE_METHOD(sunxi_alloc_free)
    RESOLVE_METHOD(sunxi_alloc_vir2phy)
    RESOLVE_METHOD(sunxi_alloc_phy2vir)
    RESOLVE_METHOD(sunxi_flush_cache_all)
  END_METHOD_RESOLVE()
};

#endif //__DLLLIBCEDARV__
