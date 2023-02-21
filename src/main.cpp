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

	std::string inputPath, outputPath;		//ファイルパス格納用
	
	commandAnalysis(argc, argv, inputPath, outputPath);//コマンド引数解析

	std::unique_ptr<Image>data;

	std::unique_ptr<Tga> tga(new Tga);
	std::unique_ptr<Bmp> bmp(new Bmp);
	
	std::array<Format*, 2> formats = {tga.get(),bmp.get()};
	
	for(auto it : formats) {
		if (it->isExt(inputPath)) {
			//ファイル読み込み、データ格納
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
			//データ読み込み、ファイル書き出し
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

//コマンド引数解析
bool commandAnalysis(u32 argc, u8* argv[], std::string& inputPath, std::string& outputPath) {
	std::string Path = {};
	for (u32 i = 0; i < argc; ++i) {
		Path = (char*)argv[i];
		if (Path == "/i") {
			//outputPath取得
			inputPath = (char*)argv[i + 1];	//「/i」の次のコマンドを取得
		}
		if (Path == "/o") {
			//outputPath取得
			outputPath = createOutPath(inputPath, (char*)argv[i + 1]);
		}
	}

	return true;
}

//outputPath取得
std::string createOutPath(std::string_view inputPath , std::string_view outputPath) {

	std::string inPath = inputPath.data();
	s32 path = inPath.find_last_of("/") + 1;
	s32 ext = inPath.find_last_of(".");
	std::string fileName = inPath.substr(path, ext - path);				//ファイル名取得
	std::string extName = inPath.substr(ext, inPath.size() - ext);		//拡張子取得
	std::string outPath = {};

	//outputPath生成
	if (extName.compare(".bmp") == 0) {
		outPath = outputPath.data() + fileName + std::string(".tga");
	}
	else if (extName.compare(".tga") == 0) {
		outPath = outputPath.data() + fileName + std::string(".bmp");
	}	

	return outPath;
}

void printText() {
	printf("<<操作方法>> \n");
	printf("<<コマンドライン入力>>\n");
	printf("/i '入力ファイルパス'　/o '出力ファイルパス' \n");
}