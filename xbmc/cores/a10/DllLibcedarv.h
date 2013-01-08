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


class DllLibA10decoderInterface
{
public:
  virtual ~DllLibA10decoderInterface() {};

  virtual cedarv_decoder_t* libcedarv_init(int *ret)=0;
  virtual int libcedarv_exit(cedarv_decoder_t *p)=0;
  virtual int mem_get_phy_addr(int addr)=0;
  virtual void* mem_palloc(unsigned int size, unsigned int align)=0;
  virtual void mem_pfree(void* p)=0;
};

class DllLibA10decoder : public DllDynamic, DllLibA10decoderInterface
{
  DECLARE_DLL_WRAPPER(DllLibA10decoder, "libbdv.so")

  DEFINE_METHOD1(cedarv_decoder_t *,            libcedarv_init, (int *p1))
  DEFINE_METHOD1(int,            libcedarv_exit, (cedarv_decoder_t *p1))
  DEFINE_METHOD1(int,            mem_get_phy_addr, (int p1))
  DEFINE_METHOD2(void*,          mem_palloc, (unsigned int p1, unsigned int p2))
  DEFINE_METHOD1(void,          mem_pfree, (void *p1))

  BEGIN_METHOD_RESOLVE()
    RESOLVE_METHOD(libcedarv_init)
    RESOLVE_METHOD(libcedarv_exit)
    RESOLVE_METHOD(mem_get_phy_addr)
    RESOLVE_METHOD(mem_palloc)
    RESOLVE_METHOD(mem_pfree)
  END_METHOD_RESOLVE()
};

#endif //__DLLLIBCEDARV__
