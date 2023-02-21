#pragma once
#pragma warning(disable : 4996) 
#include"type.h"

class FileConvert {
public:
	//ファイル読み込み
	const std::unique_ptr<u8[]> readFile(const std::string_view fileName);
	
	//ファイル書き出し
	const bool writeFile(const u8* data, const  std::string_view writeFileName, const u32 fileSize);
};