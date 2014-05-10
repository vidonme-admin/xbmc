
#include "utils/log.h"
#include "utils/URIUtils.h"
#include "URL.h"
#include "DllLibbluray.h"
#include "filesystem/File.h"
#include "Util.h"
#include "filesystem/SpecialProtocol.h"

#ifdef TARGET_WINDOWS
#include "win32/WIN32Util.h"
#endif 

#include "gtest/gtest.h"

using namespace XFILE;

class TestVidonme : public testing::Test 
{
protected:
	
	static void SetUpTestCase() 
	{
		//get xbmc_home, and install folder 
		InitDirectoriesWin32();
	}

	static void TearDownTestCase()
	{
	}


	static bool InitDirectoriesWin32()
	{
#ifdef TARGET_WINDOWS
		CStdString xbmcPath;

		CUtil::GetHomePath(xbmcPath);
		SetEnvironmentVariable("XBMC_HOME", xbmcPath.c_str());
		CSpecialProtocol::SetXBMCBinPath(xbmcPath);
		CSpecialProtocol::SetXBMCPath(xbmcPath);

		CStdString strWin32UserFolder = CWIN32Util::GetProfilePath();

		//g_settings.m_logFolder = strWin32UserFolder;
		CSpecialProtocol::SetHomePath(strWin32UserFolder);
		CSpecialProtocol::SetMasterProfilePath(URIUtils::AddFileToFolder(strWin32UserFolder, "userdata"));
		CSpecialProtocol::SetTempPath(URIUtils::AddFileToFolder(strWin32UserFolder,"cache"));

		SetEnvironmentVariable("XBMC_PROFILE_USERDATA",CSpecialProtocol::TranslatePath("special://masterprofile/").c_str());

		//CreateUserDirs();

		// Expand the DLL search path with our directories
		CWIN32Util::ExtendDllPath();

		return true;
#else 
		return false;
#endif 
	}

};

//////////////////////////////////////////////////////////////////////////

TEST_F(TestVidonme, bluray_protocol_udf_open)
{
	CStdString strBlurayIsoPath = "s:/BD/H264/2012.iso";
	CStdString strSamplePath;
	
	//simulate the xbmc path logic 
	//bool CGUIWindowVideoBase::ShowPlaySelection(CFileItemPtr& item)

	CStdString ext = URIUtils::GetExtension(strBlurayIsoPath);
	ext.ToLower();

	if (ext == ".iso" ||  ext == ".img")
	{
		CURL url2("udf://");
		url2.SetHostName(strBlurayIsoPath);
		url2.SetFileName("BDMV/index.bdmv");
		if (CFile::Exists(url2.Get()))
		{
			url2.SetFileName("");

			CURL url("bluray://");
			url.SetHostName(url2.Get());
			strSamplePath = url.Get();
		}
	}

	//bool CBlurayDirectory::GetDirectory(const CStdString& path, CFileItemList &items)
	
	BLURAY* bd = NULL;
	DllLibbluray* dll = NULL;

	CURL url;
	url.Parse(strSamplePath);
	CStdString root = url.GetHostName();
	CStdString file = url.GetFileName();
	URIUtils::RemoveSlashAtEnd(file);

	dll = new DllLibbluray();
	bool bLoad = dll->Load();
	ASSERT_TRUE(bLoad);

	dll->bd_register_dir(DllLibbluray::dir_open);
	dll->bd_register_file(DllLibbluray::file_open);
	dll->bd_set_debug_handler(DllLibbluray::bluray_logger);
	dll->bd_set_debug_mask(DBG_CRIT | DBG_BLURAY | DBG_NAV);

	bd = dll->OpenBluray(root.c_str(), NULL);
	EXPECT_TRUE(bd != NULL);

	const BLURAY_DISC_INFO *disc_info = dll->bd_get_disc_info(bd);
	int titles = dll->bd_get_titles(bd, TITLES_RELEVANT, 0);
	ASSERT_TRUE(titles > 0);

	dll->CloseBluray(bd);
	delete dll;
}

TEST_F(TestVidonme, iso_path_open)
{
	CStdString strBlurayIsoPath = "s:/BD/H264/2012.iso";

	BLURAY* bd = NULL;
	DllLibbluray* dll = NULL;

	dll = new DllLibbluray();
	bool bLoad = dll->Load();
	ASSERT_TRUE(bLoad);

	dll->bd_register_dir(DllLibbluray::dir_open);
	dll->bd_register_file(DllLibbluray::file_open);
	dll->bd_set_debug_handler(DllLibbluray::bluray_logger);
	dll->bd_set_debug_mask(DBG_CRIT | DBG_BLURAY | DBG_NAV);

	bd = dll->OpenBluray(strBlurayIsoPath.c_str(), NULL);
	EXPECT_TRUE(bd != NULL);

	dll->CloseBluray(bd);
	delete dll;
}