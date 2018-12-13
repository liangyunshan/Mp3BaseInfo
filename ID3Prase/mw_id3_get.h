#include <string>

#include "json/json.h"

bool mw_get_pic(const char* mp3file, const char* jpgfile);

bool mw_get_id3(const char* mp3file, std::string& str1, std::string& str2, std::string& str3, std::string& str4);

void readID3InfoOfMp3File(Json::Value &ID3Json,const char *mp3FilePath);
bool extractAlbumImgofMp3File(const char *mp3FilePath,const char *albumImgPath);
bool fileExists(const char *mp3FilePath);
std::string GetPathOrURLShortName(std::string strFullName);
void string_replace(std::string &strBig, const std::string &strsrc, const std::string &strdst);
void analyseAlbumImgType(const char *APIC);
int getDurationOfMp3File(const char *mp3FilePath);
