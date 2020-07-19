
#ifndef _STRING_HELPER_H_
#define _STRING_HELPER_H_

#include <string.h>
#include <iostream>
#include <list>
#include <map>
#include <vector>

#include "openssl/des.h"
#include "openssl/md5.h"
#include "openssl/pem.h"
#include "openssl/rsa.h"
#include "openssl/sha.h"

using namespace std;

typedef unsigned char uint8;
typedef unsigned long uint32;

bool ListFiles(const std::string& path, std::vector<std::string>* files);

bool ListFolders(const std::string path, std::vector<std::string>* folders_path,
                 std::vector<std::string>* folders_name);

void StringToInt(std::string str, int& value);
void StringToFloat(std::string str, float& value);

void MD5Hash(const std::string& srcStr, std::string& encodedStr,
             std::string& encodedHexStr);

string GetTimeString();

void OrderIdToString(int value, std::string& str);

void StringToOrderId(std::string str, int& value);

std::vector<std::string> Split(const std::string text,
                               const std::string delimiters,
                               const bool ignore_empty);

void Split(std::vector<std::string>* result, const std::string text,
           const std::string delimiters, const bool ignore_empty);

std::map<string, string> ParseMapString(const std::string text);

std::string UrlEncode(const std::string& str);

std::string UrlDecode(const std::string& str);

int base64_encode(const uint8* text, uint32 text_len, string& result);
int base64_decode(const uint8* code, uint32 code_len, uint8* buffer,
                  int& buffer_size);

#endif
