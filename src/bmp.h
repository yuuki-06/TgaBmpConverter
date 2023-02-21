#pragma once
#include"format.h"

constexpr u16 bmpFileType = 0x4D42;  // "BM"をリトルエンディアンで解釈した値

enum class BmpColorType {
	BmpNoneImage = 0,	//イメージなし
	BmpFullColor = 24,	//フルカラー
	BmpGrayScale = 8,	//白黒
};

struct Cie {
	u32 x_;
	u32 y_;
	u32 z_;
};

struct CieRGB {
	Cie cieR_;
	Cie cieG_;
	Cie cieB_;
};//36byte

#pragma pack(2)
struct BmpFileHeader {
	u16 fileType_;      // ファイルタイプ、必ず"BM"
	u32 fileFullSize_;  // 全体ファイルサイズ
	s16 reserved1_;		// ポインタのホットスポットのx座標
	s16 reserved2_;		// ポインタのホットスポットのy座標
	u32 offBits_;		// ファイルヘッダの先頭アドレスからビットマップデータの先頭アドレスまでのオフセット。単位はバイト。
};

#pragma pack(1)
struct BmpInfoHeader {
	u32 size_;				// この構造体のサイズ
	s32 width_;				// 画像の幅
	s32 height_;			// 画像の高さ(正なら上から下、負なら下から上)
	u16 planes_;			// 画像の枚数、通常1
	u16 bitCount_;			// 一色のビット数
	u32 compression_;		// 圧縮形式
	u32 sizeImage_;			// 画像データのサイズ（バイト数）
	u32 xPelsPerMeter_;		// 画像の横方向解像度情報
	u32 yPelsPerMeter_;		// 画像の縦方向解像度情報
	u32 colorUsed_;			// カラーテーブルの色数
	u32 colorImportant_;	// 表示に必要なカラーテーブルの色数
	u32 redMask_;			//カラーマスク_R
	u32 greenMask_;			//カラーマスク_G
	u32 blueMask_;			//カラーマスク_B
	u32 alphaMask_;			//カラーマスク_A
	u32 colorSpaceType_;	//色空間
	CieRGB cieRGB_;			
	u32 gammaRed_;			//R成分ガンマ値
	u32 gammaGreen_;		//G成分ガンマ値
	u32 gammaBlue_;			//B成分ガンマ値
};
#pragma pack()

struct BmpHeader {
	BmpFileHeader bmpFile_;		//bmpファイルヘッダ部
	BmpInfoHeader bmpInfo_;		//bmp情報ヘッダ部
};

//カラーパレット
struct ColorParet {
	u8 red_;
	u8 green_;
	u8 blue_;
	u8 reserved_;	//予約領域
};

class Bmp :public Format {
public:
	//画像データ読み込み
	virtual std::unique_ptr<Image> loadData(const std::string_view filePath);

	//画像データ作成
	virtual std::unique_ptr<u8[]> createData(Image* image);

	//ヘッダ情報初期化
	void initHeader(BmpHeader* bmpData);

	//拡張子判定
	virtual const bool isExt(const std::string_view filePath);

	//ファイル書き出し
	virtual const bool writeFile(Image* image, const std::string_view filePath);

	//画像データの反転チェック
	const bool isInvert(const s32 bmpHeight);

	//画像データ取得
	const bool getImage(Image* image ,u8* color);
	const bool getImage(Image* image, Rgba* color);
	const bool getImage(Image* image, ColorParet* paretData, u8* mapData);
};