#pragma once

constexpr u32 byteSize = 8;
constexpr u8 loopCountMask = 0x7f;
constexpr u8 isLoopMask = 0x80;

//色深度
enum class ColorDepth {
	Index = 8,
	Rgb = 24,
	Rgba = 32,
};

//画像データ格納方向判定用フラグ
struct InvertType {
	bool isHeightInvert_;
	bool isWidthInvert_;
};

//RGBAの色情報を保持する構造体
struct Rgba {
	u8 r_ ;		// Red
	u8 g_ ;		// Green
	u8 b_ ;		// Blue
	u8 a_ ;		// Alpha
};

//RGBの色情報を保持する構造体
struct Rgb {
	u8 r_;		// Red
	u8 g_;		// Green
	u8 b_;		// Blue
};

enum class ColorType {
	NoneImage,	//イメージなし
	IndexColor,	//インデックスカラー(256色)
	FullColor,	//フルカラー
	GrayScale,	//白黒
	RLEIndexColor,	//インデックスカラー(RLE圧縮)
	RLEFullColor,	//フルカラー(RLE圧縮)
	RLEGrayScale,	//白黒(RLE圧縮)

	ColorType_MAX
};

struct ColorMap {
	u8 isColorMap_ = 0;			//カラーマップの有無
	u32 colorMapEntry = 0;		//カラーマップの初期位置
	u32 colorMapLength_ = 0;	//カラーマップの長さ
	u32 colorMapEntryBit_ = 0;	//カラーマップのcolorDepth
};

//画像データ保持の構造体
struct ImageData {
	u32 width_;				// 幅
	u32 height_;			// 高さ
	ColorType colorType_;	//データタイプ
	u16 colorDepth_;		// 色深度(bit/pixcel)
	InvertType isInvert_;	//反転方向
	ColorMap colorMap_;		//カラーマップデータ
};

class Image {
private:
	ImageData imageData_;
	std::unique_ptr<Rgba[]>colorDataRGBA_;
	std::unique_ptr<u8[]>colorDataIdx_;	//indexcolor使用時画像データ保管

	u32 imageSize_;	//画像サイズ

public:
	Image();
	Image(const ImageData& imageData);

	//イメージデータ取得
	const ImageData getImageData();

	//画像データサイズ取得(height*width*RGBA)
	const u32 getImageDataSize();
	
	//画像サイズ取得(height*width)
	const u32 getImageSize();

	//画像データ格納
	const bool setImage(const Rgba* colorData);
	const bool setImage(const Rgb* colorData);
	const bool setImage(const Rgb* paretData, u8* mapData);
	
	//色情報取得
	const bool getColorData(u8* color, const u32 colorOffset, const u32 rgbaOffset);
	const bool getColorData(Rgb* color, const u32 offset);
	const bool getColorData(Rgba* color, const u32 offset);
	const Rgba getColorData(const u32 offset);

	//画像データ取得
	const u8 getIndexData(const u32 offset);
	
private:
	//画像データ格納
	const bool setColorData(const Rgba* color, const u32 num );
	const bool setColorData(const Rgb* color, const u32 num);
public:

	//RLE圧縮解凍処理
	bool decompressedRLE(u8* data ,u32 headerSize);
};