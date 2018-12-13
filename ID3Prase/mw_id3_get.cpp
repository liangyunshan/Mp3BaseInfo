#include "mw_id3_get.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include "tag.h"
#include "misc_support.h"
#include "utils.h"
#include "misc_support.h"
#include "readers.h"
#include "io_helpers.h"
#include <iostream>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "json/json.h"

#include "demo_info_options.h"
#include "id3lib_streams.h"
#include "getmp3duration.h"

using namespace dami;
using std::cout;
using std::endl;

bool mw_get_pic(const char* mp3file, const char* jpgfile)
{
	ID3_Tag tag(mp3file);
	const ID3_Frame* frame = tag.Find(ID3FID_PICTURE);
	if (frame && frame->Contains(ID3FN_DATA))
	{
		std::cout << "*** extracting picture to file \"" << jpgfile << "\"...";
		frame->Field(ID3FN_DATA).ToFile(jpgfile);
		std::cout << " done!" << std::endl;
		return true;
	}
	else
	{
		std::cout << "*** no picture frame found in \"" << mp3file << "\"" << std::endl;
		return false;
	}
}

static void PrintInformation(const ID3_Tag &myTag)
{
  ID3_Tag::ConstIterator* iter = myTag.CreateIterator();
  const ID3_Frame* frame = NULL;
  while (NULL != (frame = iter->GetNext()))
  {
    const char* desc = frame->GetDescription();
    if (!desc) desc = "";
    cout << "=== " << frame->GetTextID() << " (" << desc << "): ";
    ID3_FrameID eFrameID = frame->GetID();
    switch (eFrameID)
    {
      case ID3FID_ALBUM:
      case ID3FID_BPM:
      case ID3FID_COMPOSER:
      case ID3FID_CONTENTTYPE:
      case ID3FID_COPYRIGHT:
      case ID3FID_DATE:
      case ID3FID_PLAYLISTDELAY:
      case ID3FID_ENCODEDBY:
      case ID3FID_LYRICIST:
      case ID3FID_FILETYPE:
      case ID3FID_TIME:
      case ID3FID_CONTENTGROUP:
      case ID3FID_TITLE:
      case ID3FID_SUBTITLE:
      case ID3FID_INITIALKEY:
      case ID3FID_LANGUAGE:
      case ID3FID_SONGLEN:
      case ID3FID_MEDIATYPE:
      case ID3FID_ORIGALBUM:
      case ID3FID_ORIGFILENAME:
      case ID3FID_ORIGLYRICIST:
      case ID3FID_ORIGARTIST:
      case ID3FID_ORIGYEAR:
      case ID3FID_FILEOWNER:
      case ID3FID_LEADARTIST:
      case ID3FID_BAND:
      case ID3FID_CONDUCTOR:
      case ID3FID_MIXARTIST:
      case ID3FID_PARTINSET:
      case ID3FID_PUBLISHER:
      case ID3FID_TRACKNUM:
      case ID3FID_RECORDINGDATES:
      case ID3FID_NETRADIOSTATION:
      case ID3FID_NETRADIOOWNER:
      case ID3FID_SIZE:
      case ID3FID_ISRC:
      case ID3FID_ENCODERSETTINGS:
      case ID3FID_YEAR:
      {
        char *sText = ID3_GetString(frame, ID3FN_TEXT);
        cout << sText << endl;
        delete [] sText;
        break;
      }
      case ID3FID_USERTEXT:
      {
        char 
        *sText = ID3_GetString(frame, ID3FN_TEXT), 
        *sDesc = ID3_GetString(frame, ID3FN_DESCRIPTION);
        cout << "(" << sDesc << "): " << sText << endl;
        delete [] sText;
        delete [] sDesc;
        break;
      }
      case ID3FID_COMMENT:
      case ID3FID_UNSYNCEDLYRICS:
      {
        char 
        *sText = ID3_GetString(frame, ID3FN_TEXT), 
        *sDesc = ID3_GetString(frame, ID3FN_DESCRIPTION), 
        *sLang = ID3_GetString(frame, ID3FN_LANGUAGE);
        cout << "(" << sDesc << ")[" << sLang << "]: "
             << sText << endl;
        delete [] sText;
        delete [] sDesc;
        delete [] sLang;
        break;
      }
      case ID3FID_WWWAUDIOFILE:
      case ID3FID_WWWARTIST:
      case ID3FID_WWWAUDIOSOURCE:
      case ID3FID_WWWCOMMERCIALINFO:
      case ID3FID_WWWCOPYRIGHT:
      case ID3FID_WWWPUBLISHER:
      case ID3FID_WWWPAYMENT:
      case ID3FID_WWWRADIOPAGE:
      {
        char *sURL = ID3_GetString(frame, ID3FN_URL);
        cout << sURL << endl;
        delete [] sURL;
        break;
      }
      case ID3FID_WWWUSER:
      {
        char 
        *sURL = ID3_GetString(frame, ID3FN_URL),
        *sDesc = ID3_GetString(frame, ID3FN_DESCRIPTION);
        cout << "(" << sDesc << "): " << sURL << endl;
        delete [] sURL;
        delete [] sDesc;
        break;
      }
      case ID3FID_INVOLVEDPEOPLE:
      {
        size_t nItems = frame->GetField(ID3FN_TEXT)->GetNumTextItems();
        for (size_t nIndex = 0; nIndex < nItems; nIndex++)
        {
          char *sPeople = ID3_GetString(frame, ID3FN_TEXT, nIndex);
          cout << sPeople;
          delete [] sPeople;
          if (nIndex + 1 < nItems)
          {
            cout << ", ";
          }
        }
        cout << endl;
        break;
      }
      case ID3FID_PICTURE:
      {
        char
        *sMimeType = ID3_GetString(frame, ID3FN_MIMETYPE),
        *sDesc     = ID3_GetString(frame, ID3FN_DESCRIPTION),
        *sFormat   = ID3_GetString(frame, ID3FN_IMAGEFORMAT);
        size_t
        nPicType   = frame->GetField(ID3FN_PICTURETYPE)->Get(),
        nDataSize  = frame->GetField(ID3FN_DATA)->Size();
        cout << "(" << sDesc << ")[" << sFormat << ", "
             << nPicType << "]: " << sMimeType << ", " << nDataSize
             << " bytes" << endl;
        delete [] sMimeType;
        delete [] sDesc;
        delete [] sFormat;
        break;
      }
      case ID3FID_GENERALOBJECT:
      {
        char 
        *sMimeType = ID3_GetString(frame, ID3FN_MIMETYPE), 
        *sDesc = ID3_GetString(frame, ID3FN_DESCRIPTION), 
        *sFileName = ID3_GetString(frame, ID3FN_FILENAME);
        size_t 
        nDataSize = frame->GetField(ID3FN_DATA)->Size();
        cout << "(" << sDesc << ")[" 
             << sFileName << "]: " << sMimeType << ", " << nDataSize
             << " bytes" << endl;
        delete [] sMimeType;
        delete [] sDesc;
        delete [] sFileName;
        break;
      }
      case ID3FID_UNIQUEFILEID:
      {
        char *sOwner = ID3_GetString(frame, ID3FN_OWNER);
        size_t nDataSize = frame->GetField(ID3FN_DATA)->Size();
        cout << sOwner << ", " << nDataSize
             << " bytes" << endl;
        delete [] sOwner;
        break;
      }
      case ID3FID_PLAYCOUNTER:
      {
        size_t nCounter = frame->GetField(ID3FN_COUNTER)->Get();
        cout << nCounter << endl;
        break;
      }
      case ID3FID_POPULARIMETER:
      {
        char *sEmail = ID3_GetString(frame, ID3FN_EMAIL);
        size_t
        nCounter = frame->GetField(ID3FN_COUNTER)->Get(),
        nRating = frame->GetField(ID3FN_RATING)->Get();
        cout << sEmail << ", counter=" 
             << nCounter << " rating=" << nRating << endl;
        delete [] sEmail;
        break;
      }
      case ID3FID_CRYPTOREG:
      case ID3FID_GROUPINGREG:
      {
        char *sOwner = ID3_GetString(frame, ID3FN_OWNER);
        size_t 
        nSymbol = frame->GetField(ID3FN_ID)->Get(),
        nDataSize = frame->GetField(ID3FN_DATA)->Size();
        cout << "(" << nSymbol << "): " << sOwner
             << ", " << nDataSize << " bytes" << endl;
        break;
      }
      case ID3FID_SYNCEDLYRICS:
      {
        char 
        *sDesc = ID3_GetString(frame, ID3FN_DESCRIPTION), 
        *sLang = ID3_GetString(frame, ID3FN_LANGUAGE);
        size_t
        nTimestamp = frame->GetField(ID3FN_TIMESTAMPFORMAT)->Get(),
        nRating = frame->GetField(ID3FN_CONTENTTYPE)->Get();
        const char* format = (2 == nTimestamp) ? "ms" : "frames";
        cout << "(" << sDesc << ")[" << sLang << "]: ";
        switch (nRating)
        {
          case ID3CT_OTHER:    cout << "Other"; break;
          case ID3CT_LYRICS:   cout << "Lyrics"; break;
          case ID3CT_TEXTTRANSCRIPTION:     cout << "Text transcription"; break;
          case ID3CT_MOVEMENT: cout << "Movement/part name"; break;
          case ID3CT_EVENTS:   cout << "Events"; break;
          case ID3CT_CHORD:    cout << "Chord"; break;
          case ID3CT_TRIVIA:   cout << "Trivia/'pop up' information"; break;
        }
        cout << endl;
        ID3_Field* fld = frame->GetField(ID3FN_DATA);
        if (fld)
        {
          ID3_MemoryReader mr(fld->GetRawBinary(), fld->BinSize());
          while (!mr.atEnd())
          {
            cout << io::readString(mr).c_str();
            cout << " [" << io::readBENumber(mr, sizeof(uint32)) << " " 
                 << format << "] ";
          }
        }
        cout << endl;
        delete [] sDesc;
        delete [] sLang;
        break;
      }
      case ID3FID_AUDIOCRYPTO:
      case ID3FID_EQUALIZATION:
      case ID3FID_EVENTTIMING:
      case ID3FID_CDID:
      case ID3FID_MPEGLOOKUP:
      case ID3FID_OWNERSHIP:
      case ID3FID_PRIVATE:
      case ID3FID_POSITIONSYNC:
      case ID3FID_BUFFERSIZE:
      case ID3FID_VOLUMEADJ:
      case ID3FID_REVERB:
      case ID3FID_SYNCEDTEMPO:
      case ID3FID_METACRYPTO:
      {
        cout << " (unimplemented)" << endl;
        break;
      }
      default:
      {
        cout << " frame" << endl;
        break;
      }
    }
  }
  delete iter;
}

bool mw_get_id3(const char* mp3file, std::string& str1, std::string& str2, std::string& str3, std::string& str4)
{
	const char* filename = mp3file;
	ID3_Tag myTag;

	myTag.Link(filename, ID3TT_ALL);
	const Mp3_Headerinfo* mp3info;
	mp3info = myTag.GetMp3HeaderInfo();

	cout << endl << "*** Tag information for " << filename << endl;
	PrintInformation(myTag);
	
}

void readID3InfoOfMp3File(Json::Value &ID3Json,const char *mp3FilePath)
{
    ID3_Tag myTag;
    myTag.Link(mp3FilePath, ID3TT_ALL);

    const ID3_Tag &tag = myTag;
    ID3_Tag::ConstIterator* iter = tag.CreateIterator();
    const ID3_Frame* frame = NULL;
    while (NULL != (frame = iter->GetNext()))
    {
        const char* desc = frame->GetDescription();
        const char* textID = frame->GetTextID();
        if (!desc) desc = "";
        ID3_FrameID eFrameID = frame->GetID();
        switch (eFrameID)
        {
            case ID3FID_TITLE:  //mp3标题
            {
                char *titleText = ID3_GetString(frame, ID3FN_TEXT);
                ID3Json[textID] = titleText;
                delete [] titleText;
                break;
            }
            case ID3FID_PICTURE:    //专辑封面
            {
                analyseAlbumImgType(ID3_GetString(frame, ID3FN_MIMETYPE));
                char *sMimeType = ID3_GetString(frame, ID3FN_MIMETYPE);
                ID3Json[textID] = sMimeType;
                delete [] sMimeType;
                break;
            }
            case ID3FID_LEADARTIST:     //歌手
            {
                char *artistText = ID3_GetString(frame, ID3FN_TEXT);
                ID3Json[textID] = artistText;
                delete [] artistText;
                break;
            }
            case ID3FID_ALBUM:      //专辑名称
            {
                char *albumText = ID3_GetString(frame, ID3FN_TEXT);
                ID3Json[textID] = albumText;
                delete [] albumText;
                break;
            }

        }
    }
}

bool extractAlbumImgofMp3File(const char *mp3FilePath,const char *albumImgPath)
{
    //    std::string tmpFilePath = mp3FilePath;
    //    std::string fileName = GetPathOrURLShortName(tmpFilePath);
    //    fileName = fileName.replace(fileName.length()-3,3,"jpeg");
    //    std::string imgPath = std::string(albumPath) + "/" + fileName;
    ID3_Tag tag(mp3FilePath);
    const ID3_Frame* frame = tag.Find(ID3FID_PICTURE);
    if (frame && frame->Contains(ID3FN_DATA))
    {
        frame->Field(ID3FN_DATA).ToFile(albumImgPath);
    }
    return fileExists(albumImgPath);
}

bool fileExists(const char *mp3FilePath)
{
    fstream file;
    file.open(mp3FilePath);//file.open(filename, ios::in);
    if(!file)
    {
        file.close();
        return false;
    }
    else
    {
        file.close();
        return true;
    }
}

std::string GetPathOrURLShortName(std::string strFullName)
{
    if (strFullName.empty())
    {
        return "";
    }

    string_replace(strFullName, "/", "\\");

    std::string::size_type iPos = strFullName.find_last_of('\\') + 1;

    return strFullName.substr(iPos, strFullName.length() - iPos);
}

void string_replace(std::string &strBig, const std::string &strsrc, const std::string &strdst)
{
    std::string::size_type pos = 0;
    std::string::size_type srclen = strsrc.size();
    std::string::size_type dstlen = strdst.size();

    while( (pos=strBig.find(strsrc, pos)) != std::string::npos )
    {
        strBig.replace( pos, srclen, strdst );
        pos += dstlen;
    }
}
void analyseAlbumImgType(const char *APIC)
{
    std::string t_APICStr = APIC;
    //根据APIC字符分析封面图片格式
}

int getDurationOfMp3File(const char *mp3FilePath)
{
    struct stat stp;

    char dir[512];
    MPEG_HEAD finfo;
    int fd = -1;

    strcpy (dir, mp3FilePath);

    lstat (dir, &stp);
    finfo.fmode = stp.st_mode;
    finfo.valid = 0;

    if (!S_ISDIR (stp.st_mode)) //Is Not Dir/Path
    {
        if (strstr (dir, ".mp3"))
        {
            //mp3
            if (!S_ISREG (stp.st_mode)) //不是一个常规文件
            {
                finfo.valid = 0;
            }
            else
            {
                fd = open (dir, O_RDONLY);  //是一个常规文件
                if (fd < 0)
                {
                    finfo.valid = 0;
                }
                else if (S_ISREG (stp.st_mode))
                {
                    get_mp3header (&finfo, stp.st_size, fd);
                    close (fd);
                }
            }
              
        }
        else
        {
            return 0;
        }
    }
    else
    {
        return 0;
    }

    if(finfo.valid)
    {
        return finfo.mins*60+finfo.secs;
    }
    else
    {
        return 0;
    }
}
