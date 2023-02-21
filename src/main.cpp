#include <windows.h>
#include <stdio.h>
#include<array>
#include "tga.h"
#include "bmp.h"
#include "format.h"

bool commandAnalysis(u32 argc, u8* argv[], std::string & inputPath,std::string & outputPath);
void printText();

std::string createOutPath(std::string_view inputPath, std::string_view outputPath);

int main(u32 argc, u8* argv[]) {

	std::string inputPath, outputPath;		//�t�@�C���p�X�i�[�p
	
	commandAnalysis(argc, argv, inputPath, outputPath);//�R�}���h�������

	std::unique_ptr<Image>data;

	std::unique_ptr<Tga> tga(new Tga);
	std::unique_ptr<Bmp> bmp(new Bmp);
	
	std::array<Format*, 2> formats = {tga.get(),bmp.get()};
	
	for(auto it : formats) {
		if (it->isExt(inputPath)) {
			//�t�@�C���ǂݍ��݁A�f�[�^�i�[
			data = it->loadData(inputPath);
			if (data == nullptr) {
				printf("file input failed \n");
				printText();
				return 1;
			}
		}
	}

	for (auto it : formats){
		if (it->isExt(outputPath)) {
			//�f�[�^�ǂݍ��݁A�t�@�C�������o��
			bool isWrite = it->writeFile(data.get(), outputPath);
			if (!isWrite) {
				printf("file output failed \n");
				printText();
				return 1;
			}
		}
	}
	return 0;
}

//�R�}���h�������
bool commandAnalysis(u32 argc, u8* argv[], std::string& inputPath, std::string& outputPath) {
	std::string Path = {};
	for (u32 i = 0; i < argc; ++i) {
		Path = (char*)argv[i];
		if (Path == "/i") {
			//outputPath�擾
			inputPath = (char*)argv[i + 1];	//�u/i�v�̎��̃R�}���h���擾
		}
		if (Path == "/o") {
			//outputPath�擾
			outputPath = createOutPath(inputPath, (char*)argv[i + 1]);
		}
	}

	return true;
}

//outputPath�擾
std::string createOutPath(std::string_view inputPath , std::string_view outputPath) {

	std::string inPath = inputPath.data();
	s32 path = inPath.find_last_of("/") + 1;
	s32 ext = inPath.find_last_of(".");
	std::string fileName = inPath.substr(path, ext - path);				//�t�@�C�����擾
	std::string extName = inPath.substr(ext, inPath.size() - ext);		//�g���q�擾
	std::string outPath = {};

	//outputPath����
	if (extName.compare(".bmp") == 0) {
		outPath = outputPath.data() + fileName + std::string(".tga");
	}
	else if (extName.compare(".tga") == 0) {
		outPath = outputPath.data() + fileName + std::string(".bmp");
	}	

	return outPath;
}

void printText() {
	printf("<<������@>> \n");
	printf("<<�R�}���h���C������>>\n");
	printf("/i '���̓t�@�C���p�X'�@/o '�o�̓t�@�C���p�X' \n");
}