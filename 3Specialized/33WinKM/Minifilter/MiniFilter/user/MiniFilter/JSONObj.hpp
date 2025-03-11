#ifndef JSON_OBJ_HPP
#define JSON_OBJ_HPP

#include <Winerror.h>

#include <format>
#include <iostream>
#include <string>
#include <unordered_map>

class JSONObj {
 private:
  std::unordered_map<std::wstring, std::wstring> umSingleObjs;

 public:
  JSONObj();
  ~JSONObj();

  HRESULT addSingleObj(std::wstring wsKey, std::wstring wsValue);

  std::wstring toString();
};

#endif  // ! JSON_OBJ_HPP
