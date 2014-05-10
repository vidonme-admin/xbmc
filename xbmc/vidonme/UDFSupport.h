#ifndef __UDF_SUPPORT_H__
#define __UDF_SUPPORT_H__

#include "filesystem/IFile.h"
#include "filesystem/File.h"
#include "filesystem/Directory.h"
#include "filesystem/UDFFile.h"
#include "filesystem/UDFDirectory.h"
#include "threads/SingleLock.h"

#include "libudf/udfstruct.h"
#include "libudf/udfreader.h"

#include "boost/shared_ptr.hpp"
//library support in the libudf/lib 


#if defined(__VIDONME_UDFSUPPORT__)

namespace libudf
{

//////////////////////////////////////////////////////////////////////////

#define MAX_UDF_FILE_NAME_LEN 2048

class CUDF25 : public CUDFReader
{
public:

    CUDF25();
    CUDF25(const char* FullFileName);
    ~CUDF25();
    virtual ULONG ReadBlocks(ULONG Lba, ULONG Blocks, UCHAR* Buffer, int encrypted);

    int64_t GetFileSize(HANDLE hFile);
    int64_t GetFilePosition(HANDLE hFile);
    int64_t Seek(HANDLE hFile, int64_t Offset, int Whence);

    HANDLE OpenFile(const char* FullFileName);
    long ReadFile(HANDLE hFile, unsigned char* pBuffer, long lSize);
    void CloseFile(HANDLE hFile);

    HANDLE OpenDir(const char* Dir);
    UDF_NODE* ReadDir(HANDLE hDir);
    int CloseDir(HANDLE hDir);

	bool IsInit(){ return m_bInit; }

private:

    bool ParseUrl(const char* FullFileName, char* IsoName, char* FileName);
    
private:

    bool m_bInit;
    XFILE::CFile* m_fp;

    char m_FileCacheName[256];
    UDF_NODE* m_FileCacheNode;

    CCriticalSection m_CritSection;
};

typedef boost::shared_ptr<CUDF25> CUDF25Ptr;

//////////////////////////////////////////////////////////////////////////


class CUDFFileEx : public XFILE::IFile
{
public:

	CUDFFileEx();
	virtual ~CUDFFileEx();
	virtual int64_t GetPosition();
	virtual int64_t GetLength();
	virtual bool Open(const CURL& url);
	virtual bool Exists(const CURL& url);
	virtual int Stat(const CURL& url, struct __stat64* buffer);
	virtual unsigned int Read(void* lpBuf, int64_t uiBufSize);
	virtual int64_t Seek(int64_t iFilePosition, int iWhence = SEEK_SET);
	virtual void Close();

protected:

	bool m_bOpened;
	HANDLE m_hFile;
	CUDF25Ptr m_pUDFRoot;

	//for not support our udf-system file, still using xbmc implement
	XFILE::CUDFFile* m_pUDFXbmc;

};

//////////////////////////////////////////////////////////////////////////
// tool function 

bool IsUDFPath( const char* path, CStdString& strUDFRootKey );

CUDF25Ptr QueryUDFRoot( CStdString& strUDFRootKey );

void ReleaseUDFRoot( CStdString& strUDFRootKey );

};

#endif

#endif
