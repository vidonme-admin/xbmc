/*
*      Copyright (C) 2012-2013 Team XBMC
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

#include "AndroidStorageProvider.h"
#include "android/activity/XBMCApp.h"
#include "guilib/LocalizeStrings.h"
#include "filesystem/File.h"

//#include "utils/RegExp.h"
//#include "utils/StdString.h"
//#include "Util.h"

#include <sstream>
#include <utils/URIUtils.h>
#include <utils/log.h>

void CAndroidStorageProvider::GetLocalDrives(VECSOURCES &localDrives)
{
  CMediaSource share;

#if !defined(__ANDROID_ALLWINNER__)
  // external directory
  std::string path;
  if (CXBMCApp::GetExternalStorage(path) && !path.empty()  && XFILE::CFile::Exists(path))
  {
    share.strPath = path;
    share.strName = g_localizeStrings.Get(21456);
    share.m_ignore = true;
    localDrives.push_back(share);
  }
#else
  unsigned int new_tick = XbmcThreads::SystemClockMillis();
  if (new_tick - m_last_tick > 3000)
  {
    m_removableDrives.clear();
    innerGetRemovableDrives(m_removableDrives);
    m_last_tick = new_tick;
  }
  std::copy(m_removableDrives.begin(), m_removableDrives.end(), std::back_inserter(localDrives));

#endif //!defined(__ANDROID_ALLWINNER__)
  // root directory
  share.strPath = "/";
  share.strName = g_localizeStrings.Get(21453);
  localDrives.push_back(share);
}

void CAndroidStorageProvider::GetRemovableDrives(VECSOURCES &removableDrives)
{
#if !defined(__ANDROID_ALLWINNER__)
  // TODO
#elif 0 // !defined(__ANDROID_ALLWINNER__)
  unsigned int new_tick = XbmcThreads::SystemClockMillis();
  if (new_tick - m_last_tick > 3000)
  {
    m_removableDrives.clear();
    innerGetRemovableDrives(m_removableDrives);
    m_last_tick = new_tick;
  }
  removableDrives = m_removableDrives;
#endif // !defined(__ANDROID_ALLWINNER__)
}

std::vector<CStdString> CAndroidStorageProvider::GetDiskUsage()
{
#if !defined(__ANDROID_ALLWINNER__)
  std::vector<CStdString> result;

  std::string usage;
  // add header
  CXBMCApp::GetStorageUsage("", usage);
  result.push_back(usage);

  usage.clear();
  // add rootfs
  if (CXBMCApp::GetStorageUsage("/", usage) && !usage.empty())
    result.push_back(usage);

  usage.clear();
  // add external storage if available
  std::string path;
  if (CXBMCApp::GetExternalStorage(path) && !path.empty() &&
    CXBMCApp::GetStorageUsage(path, usage) && !usage.empty())
    result.push_back(usage);

  return result;
#else
  return innerGetDiskUsage();
#endif
}

bool CAndroidStorageProvider::Eject(CStdString mountpath)
{
  return false;
}

bool CAndroidStorageProvider::PumpDriveChangeEvents(IStorageEventsCallback *callback)
{
#if !defined(__ANDROID_ALLWINNER__)
  return false;
#else //!defined(__ANDROID_ALLWINNER__)
  VECSOURCES drives;
  GetRemovableDrives(drives);
  bool changed = drives.size() != m_removableLength;
  m_removableLength = drives.size();
  return changed;
#endif //!defined(__ANDROID_ALLWINNER__)
}


#if defined(__ANDROID_ALLWINNER__)

CAndroidStorageProvider::CAndroidStorageProvider()
{
  innerGetRemovableDrives(m_removableDrives);
  m_removableLength = m_removableDrives.size();
  m_last_tick = XbmcThreads::SystemClockMillis();
}

void CAndroidStorageProvider::innerGetRemovableDrives(VECSOURCES &drives)
{
  FILE* pipe = fopen("/proc/mounts", "r");
  if (!pipe) 
  {
    CLog::Log(LOGDEBUG, "%s: mount failed!", __FUNCTION__);
    return;
  }

  char line[1024];
  std::vector<CStdString> result;
  while (fgets(line, sizeof(line) - 1, pipe))
  {
    CLog::Log(LOGDEBUG, "%s: read line : %s", __FUNCTION__, line);
    static const char kRemovablePrefix[] = "/dev/block/vold/";
    if ( strncmp(line, kRemovablePrefix, sizeof(kRemovablePrefix)-1) != 0 ) 
    {
      CLog::Log(LOGDEBUG, "%s: not removable : %s", __FUNCTION__, line );
      continue;
    }

    std::istringstream iss(line);
    std::string deviceName;
    std::string mountPath;
    std::string fsType;
    iss >> deviceName >> mountPath >> fsType;

    // Ignore root
    // if (strcmp(mount, "/") == 0) continue;

    // Here we choose which filesystems are approved
    const char* const fs    = fsType.c_str();
    if (strcmp(fs, "fuseblk") == 0 || strcmp(fs, "vfat") == 0
      || strcmp(fs, "ext2") == 0 || strcmp(fs, "ext3") == 0
      || strcmp(fs, "reiserfs") == 0 || strcmp(fs, "xfs") == 0
      || strcmp(fs, "ntfs-3g") == 0 || strcmp(fs, "iso9660") == 0
      || strcmp(fs, "fusefs") == 0 || strcmp(fs, "hfs") == 0)
    {
      result.push_back(mountPath);
    }
  }
  fclose(pipe);

  for (unsigned int i = 0; i < result.size(); i++)
  {
    CMediaSource share;
    share.strPath = result[i];
    share.strName = URIUtils::GetFileName(result[i]);
    share.m_ignore = true;
    drives.push_back(share);
  }
}

std::vector<CStdString> CAndroidStorageProvider::innerGetDiskUsage()
{
  static const char* const excludes[] = {"rootfs","devtmpfs","tmpfs","none","/dev/loop", "udev", NULL};

  std::vector<CStdString> result;

  FILE* pipe = popen("busybox df -h", "r");
  if (!pipe) return result;

  char line[1024];
  while (fgets(line, sizeof(line) - 1, pipe))
  {
    bool ok=true;
    for (int i=0;excludes[i];++i)
    {
      if (strstr(line,excludes[i]))
      {
        ok=false;
        break;
      }
    }
    if (ok)
    {
      result.push_back(line);
    }
  }
  pclose(pipe);

  return result;
}

#endif //defined(__ANDROID_ALLWINNER__)
