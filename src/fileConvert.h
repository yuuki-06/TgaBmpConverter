#pragma once
#pragma warning(disable : 4996) 
#include"type.h"

class FileConvert {
public:
	//�t�@�C���ǂݍ���
	const std::unique_ptr<u8[]> readFile(const std::string_view fileName);
	
	//�t�@�C�������o��
	const bool writeFile(const u8* data, const  std::string_view writeFileName, const u32 fileSize);
};