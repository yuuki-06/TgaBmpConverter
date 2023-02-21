#include "tga.h"

std::unique_ptr<Image> Tga::loadData(const std::string_view filePath)
{
	FileConvert fileconvert;
	std::unique_ptr<u8[]>data = fileconvert.readFile(filePath);		//ファイル読み込み


	TgaHeader* tgaData = (TgaHeader*)data.get();

	ImageData imageData;
	imageData.height_ = (u32)tgaData->height_;		//画像縦幅取得
	imageData.width_ = (u32)tgaData->width_;		//画像横幅取得

	//カラータイプ解析
	switch (static_cast<TgaColorType>(tgaData->colorDataType_))
	{
	case TgaColorType::TgaNoneImage://イメージなし
		imageData.colorType_ = ColorType::NoneImage;
		break;
	case TgaColorType::TgaIndexColor://インデックスカラー(256色)
		imageData.colorType_ = ColorType::IndexColor;
		break;
	case TgaColorType::TgaFullColor://フルカラー
		imageData.colorType_ = ColorType::FullColor;
		break;
	case TgaColorType::TgaGrayScale://モノクロ
		imageData.colorType_ = ColorType::GrayScale;
		break;
	case TgaColorType::TgaRLEIndexColor://インデックスカラー(RLE圧縮)
		imageData.colorType_ = ColorType::RLEIndexColor;
		break;
	case TgaColorType::TgaRLEFullColor://フルカラー(RLE圧縮)
		imageData.colorType_ = ColorType::RLEFullColor;
		break;
	case TgaColorType::TgaRLEGrayScale://モノクロ(RLE圧縮)
		imageData.colorType_ = ColorType::RLEGrayScale;
		break;
	default:
		printf(" get color type failed \n ");
		return nullptr;
		break;
	}
	imageData.colorDepth_ = (u16)tgaData->colorDepth_;		//色深度取得

	//カラーマップ設定
	imageData.colorMap_.isColorMap_ = tgaData->isColorMap_;
	imageData.colorMap_.colorMapEntry = tgaData->colorMapEntry_;
	imageData.colorMap_.colorMapLength_ = tgaData->colorMapLength_;
	imageData.colorMap_.colorMapEntryBit_ = tgaData->colorEntryBit_;
	
	//画像データの反転チェック
	imageData.isInvert_.isHeightInvert_ = isInvert(tgaData->type_).isHeightInvert_;
	imageData.isInvert_.isWidthInvert_ = isInvert(tgaData->type_).isWidthInvert_;

	std::unique_ptr<Image>image = std::make_unique<Image>(imageData);

	if (ColorType::RLEFullColor == static_cast<ColorType>(imageData.colorType_)) {
		if (!image.get()->decompressedRLE(data.get(), sizeof(TgaHeader))) {
			return nullptr;
		}
	}
	else {
		//画像データ格納
		if (static_cast<ColorDepth>(tgaData->colorDepth_) == ColorDepth::Rgba) {
			Rgba* colorData = (Rgba*)(data.get() + sizeof(TgaHeader));
			if (!image.get()->setImage(colorData)) {
				printf("set image failed \n");
				return nullptr;
			}
		}
		else if (static_cast<ColorDepth>(tgaData->colorDepth_) == ColorDepth::Rgb) {
			Rgb* colorData = (Rgb*)(data.get() + sizeof(TgaHeader));
			if (!image.get()->setImage(colorData)) {
				printf("set image failed \n");
				return nullptr;
			}
		}
		else if (static_cast<ColorDepth>(tgaData->colorDepth_) == ColorDepth::Index) {
			Rgb* colorData= (Rgb*)(data.get() + sizeof(TgaHeader) + imageData.colorMap_.colorMapEntry);
			u8* mapData = data.get() + sizeof(TgaHeader) + imageData.colorMap_.colorMapEntry + (imageData.colorMap_.colorMapLength_*imageData.colorMap_.colorMapEntryBit_/byteSize);
			if (!image.get()->setImage(colorData, mapData)) {
				printf("set image failed \n");
				return nullptr;
			}
		}
	}

	return image;
}

//画像データ作成
std::unique_ptr<u8[]> Tga::createData(Image *image)
{
	ImageData imageData = image->getImageData();
	u32 imageDataSize = image->getImageDataSize();
	if (imageData.colorMap_.isColorMap_ != 0) {
		imageDataSize += imageData.colorMap_.colorMapLength_ * (imageData.colorMap_.colorMapEntryBit_ / byteSize);
	}
	std::unique_ptr<u8[]>data(new u8[sizeof(TgaHeader) + imageDataSize]);

	TgaHeader* tgaData = (TgaHeader*)data.get();

	//ヘッダ情報初期化
	initHeader(tgaData);

	//tgaHeader割り当て
	tgaData->fieldSize_ = 0;	//IDのフィールドサイズ（IDなければ0）
	tgaData->isColorMap_ = imageData.colorMap_.isColorMap_;	//カラーマップの有無（無し：0 有り：1）
	//カラータイプ・色深度の設定
	switch (imageData.colorType_){
	case ColorType::NoneImage:	//イメージなし
		tgaData->colorDataType_ = static_cast<u8>(TgaColorType::TgaNoneImage);
		break;
	case ColorType::IndexColor:	//インデックスカラー
		tgaData->colorDataType_ = static_cast<u8>(TgaColorType::TgaIndexColor);
		break;
	case ColorType::FullColor:	//フルカラー
		tgaData->colorDataType_ = static_cast<u8>(TgaColorType::TgaFullColor);
		break;
	case ColorType::GrayScale:	//白黒
		tgaData->colorDataType_ = static_cast<u8>(TgaColorType::TgaGrayScale);
		break;
	case ColorType::RLEIndexColor:	//インデックスカラー(RLE圧縮)
		tgaData->colorDataType_ = static_cast<u8>(TgaColorType::TgaRLEIndexColor);
		break;
	case ColorType::RLEFullColor:	//フルカラー(RLE圧縮)
		tgaData->colorDataType_ = static_cast<u8>(TgaColorType::TgaRLEFullColor);
		break;
	case ColorType::RLEGrayScale:	//白黒(RLE圧縮)
		tgaData->colorDataType_ = static_cast<u8>(TgaColorType::TgaRLEGrayScale);
		break;
	default:
		printf(" set color type failed \n ");
		return nullptr;
		break;
	}
	tgaData->colorDepth_ = imageData.colorDepth_;
	tgaData->colorMapEntry_ = imageData.colorMap_.colorMapEntry;
	tgaData->colorEntryBit_ = imageData.colorMap_.colorMapEntryBit_;
	tgaData->colorMapLength_ = imageData.colorMap_.colorMapLength_;
	tgaData->xPosition_ = 0;							//画像のx座標
	tgaData->yPosition_ = 0;							//画像のy座標
	tgaData->width_ = imageData.width_;					//画像の横幅を割り当て
	tgaData->height_ = imageData.height_;				//画像の横縦幅を割り当て

	
	if (imageData.isInvert_.isHeightInvert_) {	//trueなら上下反転
		tgaData->type_ = tgaTypeheightInvert;
	}
	else {
		tgaData->type_ = 0;
	}

	//画像データ設定
	if (ColorDepth::Rgb == static_cast<ColorDepth>(imageData.colorDepth_)) {
		//24bit
		Rgb* color = (Rgb*)(data.get() + sizeof(TgaHeader));
		if (!getImage(image, color)) {
			return nullptr;
		}
	}
	else if (ColorDepth::Rgba == static_cast<ColorDepth>(imageData.colorDepth_)) {
		//32bit
		Rgba* color = (Rgba*)(data.get() + sizeof(TgaHeader));
		if (!getImage(image,color)) {
			return nullptr;
		}
	}
	else if (ColorDepth::Index == static_cast<ColorDepth>(imageData.colorDepth_)) {
		//8bit
		Rgb* colorData = (Rgb*)(data.get() + sizeof(TgaHeader) + imageData.colorMap_.colorMapEntry);
		u8* mapData = data.get() + sizeof(TgaHeader) + imageData.colorMap_.colorMapEntry + (imageData.colorMap_.colorMapLength_ * imageData.colorMap_.colorMapEntryBit_ / byteSize);
		if (!getImage(image, colorData,mapData)) {
			return nullptr;
		}
	}
	
	return data;
}

//ヘッダ情報初期化
void Tga::initHeader(TgaHeader* tgaData)
{
	tgaData->fieldSize_ = 0;							//IDのフィールドサイズ（IDなければ0）
	tgaData->isColorMap_ = 0;							//カラーマップの有無（無し：0 有り：1）
	tgaData->colorDataType_ = 0;
	tgaData->colorDepth_ = 0;							//カラータイプ・色深度の設定
	tgaData->colorMapEntry_ = 0;
	tgaData->colorMapLength_ = 0;
	tgaData->colorEntryBit_ = 0;
	tgaData->xPosition_ = 0;
	tgaData->yPosition_ = 0;
	tgaData->width_ = 0;								//画像の横幅を割り当て
	tgaData->height_ = 0;								//画像の横縦幅を割り当て
	tgaData->type_ = 0;
}

//拡張子判定(.tgaであればtrueを返す)
const bool Tga::isExt(const std::string_view filePath)
{
	if (getExt(filePath).compare(".tga") == 0) {
		return true;
	}
	return false;
}

bool const Tga::writeFile(Image* image, const std::string_view filePath)
{
	ImageData imageData = image->getImageData();
	FileConvert fileconvert;
	u32 fileSize = sizeof(TgaHeader) + image->getImageDataSize();	//ファイルサイズ計算
	if (imageData.colorMap_.isColorMap_ != 0) {
		fileSize += imageData.colorMap_.colorMapLength_ * (imageData.colorMap_.colorMapEntryBit_ / byteSize);
	}
	std::unique_ptr<u8[]>data = createData(image);	//画像データ作成
	bool isWrite = fileconvert.writeFile(data.get(), filePath, fileSize);	//ファイル書き出し
	return isWrite;
}

//反転チェック
const InvertType Tga::isInvert(const u8 type)
{
	InvertType invertType;
	invertType.isHeightInvert_ = type & tgaHeightInvert; // 5つ目 0010 0000 上下反転判定
	invertType.isWidthInvert_ = type & tgaWidthInvert;	// 4つ目 0001 0000 左右反転判定

	return invertType;
}

//画像データ取得(24bit)
const bool Tga::getImage(Image* image, Rgb* color)
{
	ImageData imageData = image->getImageData();
	//画像データ設定
	for (u32 y = 0; y < imageData.height_; ++y) {
		for (u32 x = 0; x < imageData.width_; ++x) {
			u32 offset = y * imageData.width_ + x;
			if (offset >= image->getImageSize()) {
				printf("image size num over \n");
				return false;
			}
			//画像データをdataに格納
			image->getColorData(color, offset);
		}
	}
	return true;
}

//画像データ取得(32bit)
const bool Tga::getImage(Image* image, Rgba* color)
{
	ImageData imageData = image->getImageData();
	//画像データ設定
	for (u32 y = 0; y < imageData.height_; ++y) {
		for (u32 x = 0; x < imageData.width_; ++x) {
			u32 offset = y * imageData.width_ + x;
			if (offset >= image->getImageSize()) {
				printf("image size num over \n");
				return false;
			}
			//画像データをdataに格納
			image->getColorData(color, offset);
		}
	}
	return true;
}

//画像データ取得(8bit)
const bool Tga::getImage(Image* image ,Rgb* paretData ,u8* mapData)
{
	ImageData imageData = image->getImageData();

	//カラーパレットを取得
	for (u32 i = 0; i < imageData.colorMap_.colorMapLength_; ++i) {
		if (!image->getColorData(paretData, i)) {
			return false;
		}
	}

	//画像データ設定
	for (u32 y = 0; y < imageData.height_; ++y) {
		for (u32 x = 0; x < imageData.width_; ++x) {
			u32 offset = y * imageData.width_ + x;
			if (offset >= image->getImageSize()) {
				printf("image size num over \n");
				return false;
			}
			//画像データをdataに格納
			mapData[offset] = image->getIndexData(offset);		
		}
	}
	return true;
}