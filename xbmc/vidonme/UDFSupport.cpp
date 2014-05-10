
#if defined(__VIDONME_UDFSUPPORT__)
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include "system.h"
#include "UDFSupport.h"
#include "URL.h"
#include "Util.h"
#include "utils/URIUtils.h"
#include "utils/log.h"

#include "filesystem/DirectoryCache.h"
#include "filesystem/Directory.h"
#include "filesystem/FileCache.h"
#include "settings/GUISettings.h"
#include "commons/Exception.h"

#include <sys/stat.h>
#include <errno.h>

#include "DllLibbluray.h"


using namespace std;
using namespace XFILE;

namespace libudf
{

//////////////////////////////////////////////////////////////////////////
// tools function 

#define DVD_VIDEO_LB_LEN   2048
#define min(a,b) (((a)<(b))?(a):(b))

//add by wanggj
#ifdef WIN32
#define __attribute__(dummy_val)
#endif
//end

typedef struct _UDF_FILE {
    UDF_NODE* Node;
    UDF_NODE* NextNode; // Dir
    uint64_t SeekPos;   // File: in bytes
    uint64_t FileSize;  // File: in bytes
    UCHAR* FileCache;
} __attribute__((__packed__)) UDF_FILE;

// initialize and open a DVD device or file.

static CFile* file_open(const char *target)
{
    CFile* fp = new CFile();
    CStdString temp = target;
    CStdString strTarget = target;
    int flags = 0;

    if (temp.Left(9).Equals("fabudf://")) {
        strTarget = temp.substr(9,temp.size()-9);
        if (URIUtils::IsInternetStream( strTarget ))
            flags |= READ_CACHED;
    }
		else {
			if (URIUtils::IsInternetStream( strTarget ))
				flags |= READ_CACHED;
		}

    if (!fp->Open(strTarget, flags)) {
        CLog::Log(LOGERROR,"file_open - Could not open input");
        delete fp;
        return NULL;
    }

    return fp;
}

// seek into the device.
static int file_seek(CFile* fp, int blocks)
{
    off64_t pos = fp->Seek((off64_t)blocks * (off64_t)DVD_VIDEO_LB_LEN, SEEK_SET);
    if(pos < 0) {
        return pos;
    }
    // assert pos % DVD_VIDEO_LB_LEN == 0
    return (int) (pos / DVD_VIDEO_LB_LEN);
}

// read data from the device.
static int file_read(CFile* fp, void *buffer, int blocks, int flags)
{
    size_t len;
    ssize_t ret;

    len = (size_t)blocks * DVD_VIDEO_LB_LEN;
    while(len > 0) {
        ret = fp->Read(buffer, len);
        if(ret < 0) {
            /* One of the reads failed, too bad.  We won't even bother
            * returning the reads that went OK, and as in the POSIX spec
            * the file position is left unspecified after a failure. */
            return ret;
        }

        if(ret == 0) {
            /* Nothing more to read.  Return all of the whole blocks, if any.
            * Adjust the file position back to the previous block boundary. */
            size_t bytes = (size_t)blocks * DVD_VIDEO_LB_LEN - len;
            off_t over_read = -(bytes % DVD_VIDEO_LB_LEN);
            /*off_t pos =*/ fp->Seek(over_read, SEEK_CUR);
            /* should have pos % 2048 == 0 */
            return (int) (bytes / DVD_VIDEO_LB_LEN);
        }

        len -= ret;
    }

    return blocks;
}

// close the DVD device and clean up.
static int file_close(CFile* fp)
{
    fp->Close();
    return 0;
}

int UDFSplitIsoFile(const char* fullFilename, char* iso, char* file)
{
    const char* filename = strcasestr(fullFilename, ".iso");
    if (!filename) {
		//zydding, 20130314, maybe just pass path="*.*iso", not any other filename
		//sometime, the pass not fullname, just filename
		size_t size = strlen(fullFilename);
		memcpy(file, fullFilename, size);
		file[size] = 0;
        return 0;
    }

    filename += strlen(".iso");
    if (*filename != '/') {
		//zydding, 20130314, maybe just pass path="*.*iso", not any other filename
        //return -1;
    }

    size_t size = strlen(filename);
    memcpy(file, filename, size);
    file[size] = 0;

    size = filename - fullFilename;
    memcpy(iso, fullFilename,  filename - fullFilename);
    iso[size] = 0;
    return 0;
}

//////////////////////////////////////////////////////////////////////////

CUDF25::CUDF25()
{
    m_FileCacheName[0] = 0;
    m_FileCacheNode = NULL;

    m_fp = NULL;
    m_bInit = false;
}

CUDF25::CUDF25(const char* FullFileName)
{
    m_FileCacheName[0] = 0;
    m_FileCacheNode = NULL;

    char IsoName[256];
    char FileName[256];
    if (ParseUrl(FullFileName, IsoName, FileName) == false) {
        return;
    }

    m_fp = file_open(IsoName);

	if( m_fp )
		m_bInit = Parse();
	else
		m_bInit = false;
}

CUDF25::~CUDF25()
{
    m_bInit = false;

    if (m_fp) {
        m_fp->Close();
        m_fp = NULL;
    }
}

int64_t CUDF25::GetFileSize(HANDLE hFile)
{
    UDF_FILE* f = (UDF_FILE*)hFile;
    if (f == NULL) {
        return -1;
    }
    return f->FileSize;
}

int64_t CUDF25::GetFilePosition(HANDLE hFile)
{
    UDF_FILE* f = (UDF_FILE*)hFile;
    if (f == NULL) {
        return -1;
    }
    return f->SeekPos;
}

int64_t CUDF25::Seek(HANDLE hFile, int64_t Offset, int Whence)
{
    UDF_FILE* f = (UDF_FILE*)hFile;
    if (f == NULL) {
        return -1;
    }

    int64_t SeekPos = f->SeekPos;
    switch (Whence) {
    case SEEK_SET:
        // cur = pos
        f->SeekPos = Offset;
        break;

    case SEEK_CUR:
        // cur += pos
        f->SeekPos += Offset;
        break;
    case SEEK_END:
        // end += pos
        f->SeekPos = f->FileSize + Offset;
        break;
    }

    if (f->SeekPos > f->FileSize) {
        f->SeekPos = SeekPos;
        return f->SeekPos;
    }

    return f->SeekPos;
}

HANDLE CUDF25::OpenFile(const char* FullFileName)
{
    char IsoName[256];
    char FileName[256];
    if (ParseUrl(FullFileName, IsoName, FileName) == false) {
        return INVALID_HANDLE_VALUE;
    }

    if (m_bInit == false) {
        m_fp = file_open(IsoName);

		//check it, avoid open failed, cause parser crash

		if( m_fp )
			m_bInit = Parse();
		else
			return INVALID_HANDLE_VALUE;
    }

    // Check whether has open this, if doesn't return
    BOOL bUseCache = FALSE;
    UDF_NODE* Node;
    if (strcmp(m_FileCacheName, FileName) == 0) {
        Node = m_FileCacheNode;
        bUseCache = TRUE;

    } else {
        Node = GetNode(FileName);
        if (Node == NULL) {
            //printf("@@@cannot find: %s\n", FileName);
            return INVALID_HANDLE_VALUE;
        }
        m_FileCacheNode = Node;
        strcpy(m_FileCacheName, FileName);
    }

    UDF_FILE* f = new UDF_FILE;
    f->Node = Node;
    f->SeekPos = 0;
    f->FileSize = 0;
    f->FileCache = NULL;
    if (Node->IsDirectory == FALSE) {
        f->FileSize = Node->File.FileSize;

#if 1
        // If file size <= 4MB, we read total to memory
        if (f->FileSize > 0 && f->FileSize <= 7*1024*1024) {
            f->FileCache = (UCHAR*)malloc(f->FileSize);
            ULONG BytesRead;
            bool rs = CUDFReader::ReadFile(f->Node, 0, f->FileCache, f->FileSize, &BytesRead);
            if (BytesRead != f->FileSize) {
                int bp = 1;
            }
            assert(BytesRead == f->FileSize);

            /*if (strstr(FileName, "00053.mpls")) {
                if (f->FileCache[0] != 'M') {
                    int bp = 1;
                }
            }
            char n[128];
            sprintf(n, "d:\\%s", Node->Name);
            FILE* x = fopen(n, "wb");
            fwrite(f->FileCache, 1, f->FileSize, x);
            fclose(x);*/
        }
#endif
    }

    return (HANDLE)f;
}

long CUDF25::ReadFile(HANDLE hFile, unsigned char* Buffer, long Size)
{
    UDF_FILE* f = (UDF_FILE*)hFile;
    if (f == NULL || Buffer == NULL) {
        return -1;
    }

    if (f->SeekPos >= f->FileSize) {
        return 0;
    }
    
//if (strstr(f->Node->Name, ".png")) {
//    int bp = 1;
//}
    ULONG BytesRead = 0;
    // Check whether go in cache
    if (f->FileCache != NULL) {
        BytesRead = min(Size, f->FileSize - f->SeekPos);
        UCHAR* x = f->FileCache + f->SeekPos;
        //printf("read: %C, %d, %d %s\n", x[0], (ULONG)f->SeekPos, BytesRead, f->Node->Name);
        memcpy(Buffer, f->FileCache + f->SeekPos, BytesRead);
        f->SeekPos += BytesRead;

    } else {
        //printf("read: raw\n");
        bool rs = CUDFReader::ReadFile(f->Node, f->SeekPos, Buffer, Size, &BytesRead);
        f->SeekPos += BytesRead;
    }

#if 0
    FILE* x = fopen("d:\\x.bdmv", "wb");
    fwrite(Buffer, 1, BytesRead, x);
    fclose(x);
#endif
    return BytesRead;
}

void CUDF25::CloseFile(HANDLE hFile)
{
    UDF_FILE* f = (UDF_FILE*)hFile;
    if (f->FileCache) {
        free(f->FileCache);
    }

    if (f != NULL) {
        delete f;
    }
}

HANDLE CUDF25::OpenDir(const char* Dir)
{
    char IsoName[256];
    char FileName[256];
    if (ParseUrl(Dir, IsoName, FileName) == false) {
        return INVALID_HANDLE_VALUE;
    }

    if (m_bInit == false) {
        m_fp = file_open(IsoName);

		if( !m_fp )
			return NULL;

        m_bInit = Parse();
    }

    UDF_NODE* Node = GetNode((char*)FileName);
    if (Node != NULL) {
        if (Node->IsDirectory == FALSE) {
            return NULL;
        }

        UDF_FILE* f = new UDF_FILE;
        f->Node = Node;
        f->NextNode = Node->Child;
        f->SeekPos = 0;
        f->FileSize = 0;
        return (HANDLE)f;
    }
    return NULL;
}

UDF_NODE* CUDF25::ReadDir(HANDLE hDir)
{
    UDF_FILE* f = (UDF_FILE*)hDir;
    if (f == NULL) {
        return NULL;
    }

    UDF_NODE* rs = f->NextNode;
    if (rs != NULL) {
        f->NextNode = rs->Next;
    }
    return rs;
}

int CUDF25::CloseDir(HANDLE hDir)
{
    UDF_FILE* f = (UDF_FILE*)hDir;
    if (f != NULL) {
        delete f;
    }
    return 0;
}

ULONG CUDF25::ReadBlocks(ULONG Lba, ULONG Blocks, UCHAR* Buffer, int encrypted)
{
    int ret = 0;

    m_CritSection.lock();
    ret = file_seek(m_fp, (int)Lba);
    if (ret == (int)Lba) {
        ret = file_read(m_fp, (char*)Buffer, (int)Blocks, encrypted);
    }
    m_CritSection.unlock();
    return ret;
}

bool CUDF25::ParseUrl(const char* FullFileName, char* IsoName, char* FileName)
{
    IsoName[0] = 0;
    FileName[0] = 0;

    // First determine whether is fabudf://
    if (strncmp(FullFileName, "fabudf://", 9) == 0) {
        char* LastSlash = strrchr((char*)FullFileName, '/');
        if (LastSlash == NULL) {
            assert(0);
            m_bInit = false;
            return false;
        }

        // Found last '-'
        char* LastStrike = NULL;
        char* p = (char*)FullFileName;
        char* q = LastSlash - 1;
        while (q != p) {
            if (*q == '-') {
                LastStrike = q;
                break;
            }
            q--;
        }

        if (LastStrike == NULL) {
            assert(0);
            m_bInit = false;
            return false;
        }

        char* FileSeparator = strchr(LastStrike, '/');
        if (FileSeparator == NULL) {
            assert(0);
            m_bInit = false;
            return false;
        }

        int CopySize = FileSeparator - FullFileName;
        memcpy(IsoName, FullFileName, CopySize);
        IsoName[CopySize] = 0;
        strcpy(FileName, FileSeparator);

    } else {
        if (UDFSplitIsoFile(FullFileName, IsoName, FileName) != 0) {
			CLog::Log(LOGERROR, "%s: UDFSplitIsoFile failed, fullname=%s, iso=%s, file=%s", __FUNCTION__, FullFileName, IsoName, FileName);
            return false;
        }
    }

    return true;
}

//////////////////////////////////////////////////////////////////////////

CUDFFileEx::CUDFFileEx() :
m_pUDFXbmc(NULL),
m_bOpened(false),
m_hFile(NULL)
{
}

//*********************************************************************************************
CUDFFileEx::~CUDFFileEx()
{
  if (m_bOpened)
  {
    Close();
  }
}
//*********************************************************************************************
bool CUDFFileEx::Open(const CURL& url)
{
  CStdString strRoot = url.GetHostName();

  //check UDFRoot is exist 
  m_pUDFRoot = QueryUDFRoot(strRoot);

  if( m_pUDFRoot )
  {
	  CStdString strFileName = url.GetFileName();
	 
	  m_hFile = m_pUDFRoot->OpenFile((char*)strFileName.c_str());
	  if (m_hFile == INVALID_HANDLE_VALUE)
	  {
		  m_bOpened = false;
		  return false;
	  }

	  m_bOpened = true;
	  return true;
  }
  else
  {
	//using xbmc provide udf class
	m_pUDFXbmc = new XFILE::CUDFFile();

	return m_pUDFXbmc->Open(url);
  }

}

//*********************************************************************************************
unsigned int CUDFFileEx::Read(void *lpBuf, int64_t uiBufSize)
{
  if( m_pUDFXbmc )
  {
	return m_pUDFXbmc->Read(lpBuf, uiBufSize);
  }

  if (!m_bOpened) return 0;
  char *pData = (char *)lpBuf;

  int iResult = m_pUDFRoot->ReadFile( m_hFile, (unsigned char*)pData, (long)uiBufSize);
  if (iResult == -1)
    return 0;
  return iResult;
}

//*********************************************************************************************
void CUDFFileEx::Close()
{
  if( m_pUDFXbmc )
  {
	m_pUDFXbmc->Close();
	return;
  }

  if (!m_bOpened) return ;
  m_pUDFRoot->CloseFile( m_hFile);
}

//*********************************************************************************************
int64_t CUDFFileEx::Seek(int64_t iFilePosition, int iWhence)
{
  if( m_pUDFXbmc )
  {
	return m_pUDFXbmc->Seek(iFilePosition, iWhence);
  }

  if (!m_bOpened) return -1;
  int64_t lNewPos = m_pUDFRoot->Seek(m_hFile, iFilePosition, iWhence);
  return lNewPos;
}

//*********************************************************************************************
int64_t CUDFFileEx::GetLength()
{
  if( m_pUDFXbmc )
  {
	return m_pUDFXbmc->GetLength();
  }

  if (!m_bOpened) return -1;
  return m_pUDFRoot->GetFileSize(m_hFile);
}

//*********************************************************************************************
int64_t CUDFFileEx::GetPosition()
{
  if( m_pUDFXbmc )
  {
	return m_pUDFXbmc->GetPosition();
  }

  if (!m_bOpened) return -1;
  return m_pUDFRoot->GetFilePosition(m_hFile);
}

bool CUDFFileEx::Exists(const CURL& url)
{  
	CStdString strRoot = url.GetHostName();

	//check UDFRoot is exist 
	m_pUDFRoot = QueryUDFRoot(strRoot);

	if( m_pUDFRoot )
	{
		string strFName = "\\";
		strFName += url.GetFileName();
		for (int i = 0; i < (int)strFName.size(); ++i )
		{
			if (strFName[i] == '/') strFName[i] = '\\';
		}
		m_hFile = m_pUDFRoot->OpenFile((char*)strFName.c_str());
		if (m_hFile == INVALID_HANDLE_VALUE)
			return false;

		m_pUDFRoot->CloseFile(m_hFile);
		return true;
	}
	else
	{
		m_pUDFXbmc = new XFILE::CUDFFile();
		return m_pUDFXbmc->Exists(url);
	}
}

int CUDFFileEx::Stat(const CURL& url, struct __stat64* buffer)
{
	CStdString strRoot = url.GetHostName();

	//check UDFRoot is exist 
	m_pUDFRoot = QueryUDFRoot(strRoot);

	if( m_pUDFRoot )
	{
		string strFName = "\\";
		strFName += url.GetFileName();
		for (int i = 0; i < (int)strFName.size(); ++i )
		{
			if (strFName[i] == '/') strFName[i] = '\\';
		}
		m_hFile = m_pUDFRoot->OpenFile((char*)strFName.c_str());
		if (m_hFile != INVALID_HANDLE_VALUE)
		{
			buffer->st_size = m_pUDFRoot->GetFileSize(m_hFile);
			buffer->st_mode = _S_IFREG;
			m_pUDFRoot->CloseFile(m_hFile);
			return 0;
		}
		errno = ENOENT;
		return -1;
	}
	else
	{
		m_pUDFXbmc = new XFILE::CUDFFile();
		return m_pUDFXbmc->Stat(url, buffer);
	}
}






};
#endif
