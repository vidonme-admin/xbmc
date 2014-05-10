
#if defined(__VIDONME_MEDIACENTER__)

#pragma once
#include <vector>
#include <map>
#include "interfaces/json-rpc/ProxyJSONRPC.h"
#include "boost/shared_ptr.hpp"

using namespace JSONRPC;

namespace VidOnMe
{
//////////////////////////////////////////////////////////////////////////
//Movie
struct MovieBaseInfo
{
  CProxyJSONRPC::ServerMovieInfo baseInfo;
};
typedef boost::shared_ptr<MovieBaseInfo> MovieBaseInfoPtr;

struct MovieDetails
{
  CProxyJSONRPC::ServerMovieDetail	details;
  CProxyJSONRPC::ServerMetaInfo  meta;
};
typedef boost::shared_ptr<MovieDetails> MovieDetailsPtr;

struct MoviePlaylists
{
  CProxyJSONRPC::ServerPlaylists playlists;
  int nGetThumbnail;
};
typedef boost::shared_ptr<MoviePlaylists> MoviePlaylistsPtr;

struct MovieInfo
{
  MovieBaseInfoPtr baseInfo;
  MovieDetailsPtr details;
  MoviePlaylistsPtr playlists;
};
typedef boost::shared_ptr<MovieInfo> MovieInfoPtr;

struct MoviesFilter
{
  CProxyJSONRPC::MovieSort sort;
  CProxyJSONRPC::Limits limits;
  CProxyJSONRPC::MovieFilter filter;
};
typedef boost::shared_ptr<MoviesFilter> MoviesFilterPtr;

//////////////////////////////////////////////////////////////////////////
//TV Show
struct TVShowBaseInfo
{
  CProxyJSONRPC::ServerTVShowInfo baseInfo;
};
typedef boost::shared_ptr<TVShowBaseInfo> TVShowBaseInfoPtr;

struct TVShowSeasonsInfo
{
  std::vector<CProxyJSONRPC::TVShowSeason> seasonsInfo;
};
typedef boost::shared_ptr<TVShowSeasonsInfo> TVShowSeasonsInfoPtr;

struct TVShowEpisodeInfo
{
  CProxyJSONRPC::TVShowEpisode episodeInfo;
};
typedef boost::shared_ptr<CProxyJSONRPC::TVShowEpisode> TVShowEpisodeInfoPtr;

struct TVShowInfo
{
  TVShowBaseInfoPtr baseInfo;
  TVShowSeasonsInfoPtr seasonsInfo;
};
typedef boost::shared_ptr<TVShowInfo> TVShowInfoPtr;

struct TVShowsFilter
{
  CProxyJSONRPC::TVShowSort	sort;
  CProxyJSONRPC::Limits	limits;
  CProxyJSONRPC::TVShowFilter filter;
};
typedef boost::shared_ptr<TVShowsFilter> TVShowsFilterPtr;

//////////////////////////////////////////////////////////////////////////
//Video
struct VideoInfo
{
  CProxyJSONRPC::ServerVideoInfo videoInfo;
};
typedef boost::shared_ptr<VideoInfo> VideoInfoPtr;

struct VideosInfo
{
  std::vector<VideoInfoPtr> videosInfo;
};
typedef boost::shared_ptr<VideosInfo> VideosInfoPtr;

struct VideosFilter
{
  CProxyJSONRPC::Limits	limits;
};
typedef boost::shared_ptr<VideosFilter> VideosFilterPtr;

//////////////////////////////////////////////////////////////////////////
//Picture
struct PictureInfo
{
  CProxyJSONRPC::ServerPictureSource pictureInfo;
};
typedef boost::shared_ptr<PictureInfo> PictureInfoPtr;

struct PicturesInfo
{
  std::vector<PictureInfoPtr> picturesInfo;
};
typedef boost::shared_ptr<PicturesInfo> PicturesInfoPtr;

struct PicturesFilter
{
  CProxyJSONRPC::Limits	limits;
};
typedef boost::shared_ptr<PicturesFilter> PicturesFilterPtr;

//////////////////////////////////////////////////////////////////////////
//Music


//////////////////////////////////////////////////////////////////////////
//File

struct FileBaseInfo
{
  CProxyJSONRPC::FileNode baseInfo;
};
typedef boost::shared_ptr<FileBaseInfo> FileBaseInfoPtr;

struct FileMetaInfo
{
  CProxyJSONRPC::ServerMetaInfo meta;
};
typedef boost::shared_ptr<FileMetaInfo> FileMetaInfoPtr;

struct FilePlaylists
{
  CProxyJSONRPC::ServerPlaylists playlists;
  int nGetThumbnail;
};
typedef boost::shared_ptr<FilePlaylists> FilePlaylistsPtr;

struct FileInfo
{
  FileBaseInfoPtr baseInfo;
  FileMetaInfoPtr details;
  FilePlaylistsPtr playlists;
};
typedef boost::shared_ptr<FileInfo> FileInfoPtr;

struct FilesInfo
{
  std::vector<FileInfoPtr> filesInfo;
};
typedef boost::shared_ptr<FilesInfo> FilesInfoPtr;

}

#endif