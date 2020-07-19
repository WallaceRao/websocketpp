#include "string_helper.h"
#include <assert.h>
#include <dirent.h>
#include <unistd.h>

#include <stdio.h>
#include <sys/stat.h>
#include <time.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;
const int order_length = 10;

void StringToInt(std::string str, int &value) {
  std::stringstream ss;
  ss << str;
  ss >> value;
}

void StringToFloat(std::string str, float &value) {
  std::stringstream ss;
  ss << str;
  ss >> value;
}

void MD5Hash(const std::string &srcStr, std::string &encodedStr,
             std::string &encodedHexStr) {
  // 调用md5哈希
  unsigned char mdStr[33] = {0};
  MD5((const unsigned char *)srcStr.c_str(), srcStr.length(), mdStr);

  // 哈希后的字符串
  encodedStr = std::string((const char *)mdStr);

  // 哈希后的十六进制串 32字节
  char buf[65] = {0};
  char tmp[3] = {0};
  for (int i = 0; i < 32; i++) {
    sprintf(tmp, "%02x", mdStr[i]);
    strcat(buf, tmp);
  }
  buf[32] = '\0';  // 后面都是0，从32字节截断

  encodedHexStr = std::string(buf);
}

bool IsFileExist(const string path) { return access(path.c_str(), F_OK) == 0; }

bool IsDir(const std::string path) {
  if (!IsFileExist(path)) {
    return false;
  }
  struct stat statbuf;
  lstat(path.c_str(), &statbuf);
  return S_ISDIR(statbuf.st_mode);
}

bool ListFiles(const std::string &path, std::vector<std::string> *files) {
  DIR *dp = NULL;
  struct dirent *entry = NULL;
  if ((dp = opendir(path.c_str())) == NULL) {
    printf("Can not open dir[%s]!", path.c_str());
    return false;
  }
  while ((entry = readdir(dp)) != NULL) {
    std::string filename = path;
    filename.append("/").append(entry->d_name);
    if (IsDir(filename)) {
      continue;
    }
    files->push_back(filename);
  }
  closedir(dp);
  return true;
}

bool ListFolders(const std::string path, std::vector<std::string> *folders_path,
                 std::vector<std::string> *folders_name) {
  DIR *dp = NULL;
  struct dirent *entry = NULL;
  if ((dp = opendir(path.c_str())) == NULL) {
    printf("Can not open dir[%s]!", path.c_str());
    return false;
  }
  while ((entry = readdir(dp)) != NULL) {
    std::string folder_path = path;
    std::string folder_name = entry->d_name;
    if (folder_name == "." || folder_name == "..") {
      continue;
    }
    folder_path.append("/").append(entry->d_name);
    if (IsDir(folder_path)) {
      folders_path->push_back(folder_path);
      folders_name->push_back(folder_name);
    }
  }
  closedir(dp);
  return true;
}

void OrderIdToString(int value, std::string &str) {}

void StringToOrderId(std::string str, int &value) {}

string GetTimeString() {
  time_t t = time(0);
  char tmp[64];
  for (int i = 0; i < 64; i++) {
    tmp[i] = '\0';
  }
  strftime(tmp, sizeof(tmp), "%Y/%m/%d %H:%M:%S", localtime(&t));
  string result = string(tmp);
  return result;
}

std::vector<std::string> Split(const std::string text,
                               const std::string delimiters,
                               const bool ignore_empty) {
  std::vector<std::string> result;
  Split(&result, text, delimiters, ignore_empty);
  return result;
}  // Split

void Split(std::vector<std::string> *result, const std::string text,
           const std::string delimiters, const bool ignore_empty) {
  result->clear();
  std::string str(text);
  std::string sep = (delimiters.empty() ? " " : delimiters);
  size_t prev_pos = 0, pos = 0;
  std::string piece;

  while (pos != std::string::npos) {
    pos = str.find_first_of(sep, pos);
    if (pos != std::string::npos) {
      piece = "";
      if (pos != prev_pos) {
        piece = str.substr(prev_pos, pos - prev_pos);
      }
      if (!ignore_empty || !piece.empty()) {
        result->push_back(piece);
      }
      prev_pos = ++pos;
    }
  }

  piece = "";
  if (prev_pos < str.length()) {
    piece = str.substr(prev_pos);
  }
  if (!ignore_empty || !piece.empty()) {
    result->push_back(piece);
  }
}

std::map<string, string> ParseMapString(const std::string text) {
  printf("ParseMapString:%s\n", text.c_str());
  std::map<string, string> result;
  std::vector<std::string> pairs = Split(text, "&", true);
  for (string pair : pairs) {
    std::vector<std::string> key_value = Split(pair, "=", true);
    if (key_value.size() == 2) {
      string key = UrlDecode(key_value[0]);
      string value = UrlDecode(key_value[1]);
      result[key] = value;
    } else {
      printf("invalid pair:%s\n", pair.c_str());
    }
  }
  return result;
}

unsigned char ToHex(unsigned char x) { return x > 9 ? x + 55 : x + 48; }

unsigned char FromHex(unsigned char x) {
  unsigned char y;
  if (x >= 'A' && x <= 'Z')
    y = x - 'A' + 10;
  else if (x >= 'a' && x <= 'z')
    y = x - 'a' + 10;
  else if (x >= '0' && x <= '9')
    y = x - '0';
  else
    assert(0);
  return y;
}

std::string UrlEncode(const std::string &str) {
  std::string strTemp = "";
  size_t length = str.length();
  for (size_t i = 0; i < length; i++) {
    if (isalnum((unsigned char)str[i]) || (str[i] == '-') || (str[i] == '_') ||
        (str[i] == '.') || (str[i] == '~'))
      strTemp += str[i];
    else if (str[i] == ' ')
      strTemp += "+";
    else {
      strTemp += '%';
      strTemp += ToHex((unsigned char)str[i] >> 4);
      strTemp += ToHex((unsigned char)str[i] % 16);
    }
  }
  return strTemp;
}

std::string UrlDecode(const std::string &str) {
  std::string strTemp = "";
  size_t length = str.length();
  for (size_t i = 0; i < length; i++) {
    if (str[i] == '+')
      strTemp += ' ';
    else if (str[i] == '%') {
      assert(i + 2 < length);
      unsigned char high = FromHex((unsigned char)str[++i]);
      unsigned char low = FromHex((unsigned char)str[++i]);
      strTemp += high * 16 + low;
    } else
      strTemp += str[i];
  }
  return strTemp;
}

string Base64Encode(const unsigned char *Data, int DataByte) {
  //编码表
  const char EncodeTable[] =
      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  //返回值
  string strEncode;
  unsigned char Tmp[4] = {0};
  int LineLength = 0;
  for (int i = 0; i < (int)(DataByte / 3); i++) {
    Tmp[1] = *Data++;
    Tmp[2] = *Data++;
    Tmp[3] = *Data++;
    strEncode += EncodeTable[Tmp[1] >> 2];
    strEncode += EncodeTable[((Tmp[1] << 4) | (Tmp[2] >> 4)) & 0x3F];
    strEncode += EncodeTable[((Tmp[2] << 2) | (Tmp[3] >> 6)) & 0x3F];
    strEncode += EncodeTable[Tmp[3] & 0x3F];
    if (LineLength += 4, LineLength == 76) {
      strEncode += "\r\n";
      LineLength = 0;
    }
  }
  //对剩余数据进行编码
  int Mod = DataByte % 3;
  if (Mod == 1) {
    Tmp[1] = *Data++;
    strEncode += EncodeTable[(Tmp[1] & 0xFC) >> 2];
    strEncode += EncodeTable[((Tmp[1] & 0x03) << 4)];
    strEncode += "==";
  } else if (Mod == 2) {
    Tmp[1] = *Data++;
    Tmp[2] = *Data++;
    strEncode += EncodeTable[(Tmp[1] & 0xFC) >> 2];
    strEncode += EncodeTable[((Tmp[1] & 0x03) << 4) | ((Tmp[2] & 0xF0) >> 4)];
    strEncode += EncodeTable[((Tmp[2] & 0x0F) << 2)];
    strEncode += "=";
  }

  return strEncode;
}

string Base64Decode(const char *Data, int DataByte, int &OutByte) {
  //解码表
  const char
      DecodeTable[] =
          {
              ,   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
              0,  0,  0,  0,  0,  0,  0,  0,  0,  ,   0,  0,  0,  0,  0,
              0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
              ,  // '+'
              ,   0,  0,
              ,                                        // '/'
              ,   53, 54, 55, 56, 57, 58, 59, 60, 61,  // '0'-'9'
              ,   0,  0,  0,  0,  0,  0,  ,   1,  2,  3,  4,  5,  6,  7,
              8,  9,  10, 11, 12, ,   14, 15, 16, 17, 18, 19, 20, 21, 22,
              23, 24, 25,  // 'A'-'Z'
              ,   0,  0,  0,  0,  0,  ,   27, 28, 29, 30, 31, 32, 33, 34,
              35, 36, 37, 38, ,   40, 41, 42, 43, 44, 45, 46, 47, 48, 49,
              50, 51,  // 'a'-'z'
          };
  //返回值
  string strDecode;
  int nValue;
  int i = 0;
  while (i < DataByte) {
    if (*Data != '\r' && *Data != '\n') {
      nValue = DecodeTable[*Data++] << 18;
      nValue += DecodeTable[*Data++] << 12;
      strDecode += (nValue & 0x00FF0000) >> 16;
      OutByte++;
      if (*Data != '=') {
        nValue += DecodeTable[*Data++] << 6;
        strDecode += (nValue & 0x0000FF00) >> 8;
        OutByte++;
        if (*Data != '=') {
          nValue += DecodeTable[*Data++];
          strDecode += nValue & 0x000000FF;
          OutByte++;
        }
      }
      i += 4;
    } else  // 回车换行,跳过
    {
      Data++;
      i++;
    }
  }
  return strDecode;
}