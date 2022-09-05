#pragma once

#include <climits>
#include <string>
#include <map>
#include <functional>
#include <utility>
#include <vector>
#include <deque>

#if __cplusplus >= 201402L
#	include <filesystem>
namespace fs = std::filesystem;
#elif __cplusplus > 199711L
#	include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#else
#	error "Unsupported C++ version! Set C++11 or newer!"
#endif

#	define uint64 uint64_t
#	define uint32 uint32_t
#	define int64 int64_t
#	define int32 int32_t
#	define int16 int16_t
#	define uint8 uint8_t

typedef const std::string	CString;
typedef std::string			SString;
typedef std::deque<SString> SStringList;

template<typename T>
class SStringMap : public std::map<SString, T>{};
