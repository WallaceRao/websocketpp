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

static uint8 alphabet_map[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static uint8 reverse_map[] = {
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 62,  255,
    255, 255, 63,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  255, 255,
    255, 255, 255, 255, 255, 0,   1,   2,   3,   4,   5,   6,   7,   8,   9,
    10,  11,  12,  13,  14,  15,  16,  17,  18,  19,  20,  21,  22,  23,  24,
    25,  255, 255, 255, 255, 255, 255, 26,  27,  28,  29,  30,  31,  32,  33,
    34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,  48,
    49,  50,  51,  255, 255, 255, 255, 255};

int base64_encode(const uint8 *text, uint32 text_len, string &result) {
  uint32 i, j;
  uint8 *encode = new uint8[text_len * 2];
  memset(encode, 0, text_len * 2);

  for (i = 0, j = 0; i + 3 <= text_len; i += 3) {
    encode[j++] =
        alphabet_map[text[i] >> 2];  //取出第一个字符的前6位并找出对应的结果字符
    encode[j++] = alphabet_map
        [((text[i] << 4) & 0x30) |
         (text[i + 1] >>
          4)];  //将第一个字符的后2位与第二个字符的前4位进行组合并找到对应的结果字符
    encode[j++] = alphabet_map
        [((text[i + 1] << 2) & 0x3c) |
         (text[i + 2] >>
          6)];  //将第二个字符的后4位与第三个字符的前2位组合并找出对应的结果字符
    encode[j++] =
        alphabet_map[text[i + 2] & 0x3f];  //取出第三个字符的后6位并找出结果字符
  }

  if (i < text_len) {
    uint32 tail = text_len - i;
    if (tail == 1) {
      encode[j++] = alphabet_map[text[i] >> 2];
      encode[j++] = alphabet_map[(text[i] << 4) & 0x30];
      encode[j++] = '=';
      encode[j++] = '=';
    } else  // tail==2
    {
      encode[j++] = alphabet_map[text[i] >> 2];
      encode[j++] = alphabet_map[((text[i] << 4) & 0x30) | (text[i + 1] >> 4)];
      encode[j++] = alphabet_map[(text[i + 1] << 2) & 0x3c];
      encode[j++] = '=';
    }
  }
  result = (char *)encode;
  delete[] encode;
  return j;
}

int base64_decode(const uint8 *code, uint32 code_len, const uint8 *buffer,
                  int &buffer_size) {
  assert((code_len & 0x03) ==
         0);  //如果它的条件返回错误，则终止程序执行。4的倍数。
  uint8 *plain = new uint8[code_len * 2];
  memset(plain, 0, code_len * 2);
  uint32 i, j = 0;
  uint8 quad[4];
  for (i = 0; i < code_len; i += 4) {
    for (uint32 k = 0; k < 4; k++) {
      quad[k] = reverse_map
          [code[i + k]];  //分组，每组四个分别依次转换为base64表内的十进制数
    }

    assert(quad[0] < 64 && quad[1] < 64);

    plain[j++] =
        (quad[0] << 2) |
        (quad[1] >>
         4);  //取出第一个字符对应base64表的十进制数的前6位与第二个字符对应base64表的十进制数的前2位进行组合

    if (quad[2] >= 64)
      break;
    else if (quad[3] >= 64) {
      plain[j++] =
          (quad[1] << 4) |
          (quad[2] >>
           2);  //取出第二个字符对应base64表的十进制数的后4位与第三个字符对应base64表的十进制数的前4位进行组合
      break;
    } else {
      plain[j++] = (quad[1] << 4) | (quad[2] >> 2);
      plain[j++] =
          (quad[2] << 6) |
          quad[3];  //取出第三个字符对应base64表的十进制数的后2位与第4个字符进行组合
    }
  }
  memcpy(buffer, plain, j);
  delete[] plain;
  return j;
}
