#pragma once
#include"format.h"

constexpr u8 tgaHeightInvert = 0x20;
constexpr u8 tgaWidthInvert = 0x10;

constexpr u32 tgaTypeheightInvert = 32;
constexpr u32 tgaTypeWidthInvert = 64;

enum class TgaColorType : u8{
	TgaNoneImage = 0,
	TgaIndexColor = 1,
	TgaFullColor = 2,
	TgaGrayScale = 3,
	TgaRLEIndexColor = 9,
	TgaRLEFullColor = 10,
	TgaRLEGrayScale = 11,
};

#pragma pack(1)
struct TgaHeader {
	u8 fieldSize_;			//ヘッダーの後に続く、IDの長さ
	u8 isColorMap_;			//カラーマップの有無 0:なし　1:あり
	u8 colorDataType_;		//データ形式
	u16 colorMapEntry_;		//カラーマップの位置
	u16 colorMapLength_;	//カラーマップの長さ
	u8 colorEntryBit_;
	s16 xPosition_;			//画像のＸ座標
	s16 yPosition_;			//画像のＹ座標
	s16 width_;				//画像の横幅
	s16 height_;			//画像の縦幅
	s8 colorDepth_;			//色深度
	u8 type_;				//属性..
};
#pragma pack()

class Tga :public Format{
public:
	//画像データ読み込み
	virtual std::unique_ptr<Image> loadData(const std::string_view filePath);

	//画像データ作成
	virtual std::unique_ptr<u8[]> createData(Image *image);

	//ヘッダ情報初期化
	void initHeader(TgaHeader* tgaData);

	//拡張子判定
	virtual const bool isExt(const std::string_view filePath);

	//ファイル書き出し
	virtual const bool writeFile(Image* image, const std::string_view filePath);

	//画像データの反転チェック
	const InvertType isInvert(const u8 type);

	//画像データ取得
	const bool getImage(Image* image, Rgb* color);
	const bool getImage(Image* image, Rgba* color);
	const bool getImage(Image* image, Rgb* paretData, u8* mapData);
};