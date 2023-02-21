#include "bmp.h"

std::unique_ptr<Image> Bmp::loadData(const std::string_view filePath)
{

	FileConvert fileconvert;
	std::unique_ptr<u8[]>data = fileconvert.readFile(filePath);		//ファイル読み込み

	BmpHeader* bmpData = (BmpHeader*)data.get();

	//bmpデータ解析、保管
	ImageData imageData;
	imageData.height_ = bmpData->bmpInfo_.height_;				//画像縦幅取得、保管
	imageData.width_ = bmpData->bmpInfo_.width_;				//画像横幅取得、保管
	imageData.colorDepth_ = bmpData->bmpInfo_.bitCount_;		//色深度取得
	//カラータイプ解析
	switch (static_cast<BmpColorType>(bmpData->bmpInfo_.bitCount_))
	{
	case BmpColorType::BmpNoneImage ://イメージなし
		imageData.colorType_ = ColorType::NoneImage;
		break;
	case BmpColorType::BmpFullColor ://フルカラー
		imageData.colorType_ = ColorType::FullColor;
		break;
	case BmpColorType::BmpGrayScale ://モノクロ
		imageData.colorType_ = ColorType::GrayScale;
		break;
	default:
		printf(" get color type failed \n ");
		return nullptr;
		break;
	}

	//画像データの反転チェック
	imageData.isInvert_.isHeightInvert_ = isInvert(bmpData->bmpInfo_.height_);
	imageData.isInvert_.isWidthInvert_ = false;	//左右反転無し

	std::unique_ptr<Image>image_ = std::make_unique<Image>(imageData);	//イメージデータ保管用

	//画像データを格納
	if (ColorDepth::Rgb == static_cast<ColorDepth>(imageData.colorDepth_)) {
		Rgb* colorData = (Rgb*)(data.get() + bmpData->bmpFile_.offBits_);
		if (!image_.get()->setImage(colorData)) {
			printf("set image failed \n");
			return nullptr;
		}
	}
	else if (ColorDepth::Rgba == static_cast<ColorDepth>(imageData.colorDepth_)) {
		Rgba* colorData = (Rgba*)(data.get() + bmpData->bmpFile_.offBits_);
		if (!image_.get()->setImage(colorData)) {
			printf("set image failed \n");
			return nullptr;
		}
	}

	return image_;
}

std::unique_ptr<u8[]> Bmp::createData(Image* image){
	
	ImageData imageData = image->getImageData();
	u32 imageDataSize = image->getImageDataSize();
	
	//画像データサイズ取得
	if (ColorDepth::Rgb == static_cast<ColorDepth>(imageData.colorDepth_)) {
		u32 padding = imageData.width_ % 4;	//パディング処理判定
		imageDataSize = imageData.height_ * (imageData.width_ * (imageData.colorDepth_ / byteSize) + padding);//画像データサイズ取得
	}
	
	//カラーパレット使用時サイズ変更
	if (imageData.colorMap_.isColorMap_ != 0) {
		imageDataSize += sizeof(ColorParet) * imageData.colorMap_.colorMapLength_;
	}

	std::unique_ptr<u8[]>data(new u8[sizeof(BmpHeader) + imageDataSize]);

	BmpHeader* bmpData = (BmpHeader*)data.get();

	initHeader(bmpData);	//ヘッダの一部を初期化

	//bmpHeader割り当て
	bmpData->bmpFile_.fileType_ = bmpFileType;
	bmpData->bmpFile_.fileFullSize_ = sizeof(BmpHeader) + imageDataSize;		//ファイル全体のサイズ取得
	bmpData->bmpFile_.offBits_ = sizeof(BmpHeader);								//画像データ部の開始位置
	bmpData->bmpInfo_.size_ = sizeof(BmpInfoHeader);							//infoヘッダのサイズ
	bmpData->bmpInfo_.width_ = imageData.width_;								//画像の横幅を割り当て

	if (!imageData.isInvert_.isHeightInvert_) {
		bmpData->bmpInfo_.height_ = imageData.height_;							//画像の縦幅を割り当て
	}
	else {//trueなら反転
		bmpData->bmpInfo_.height_ = -imageData.height_;							//画像の縦幅を割り当て(配列反転)
	}

	bmpData->bmpInfo_.bitCount_ = imageData.colorDepth_;						//カラータイプの設定
	bmpData->bmpInfo_.compression_ = 0;											//圧縮形式の設定(非圧縮：0)
	bmpData->bmpInfo_.sizeImage_ = imageDataSize;								//画像データ分のサイズ取得
	bmpData->bmpInfo_.xPelsPerMeter_ = 0;										//0でいい	
	bmpData->bmpInfo_.yPelsPerMeter_ = 0;										//0でいい	
	u32 headerSize = bmpData->bmpFile_.offBits_;
	bmpData->bmpInfo_.colorUsed_ = imageData.colorMap_.colorMapLength_;
	bmpData->bmpInfo_.colorImportant_ = imageData.colorMap_.colorMapEntryBit_;

	//画像データ設定
	if (ColorDepth::Rgb == static_cast<ColorDepth>(imageData.colorDepth_)) {
		//24bit
		u8* color = data.get() + headerSize;	//RGBの場合パディングを考慮
		if (!getImage(image, color)) {
			return nullptr;
		}
	}
	else if (ColorDepth::Rgba == static_cast<ColorDepth>(imageData.colorDepth_)) {
		//32bit
		Rgba* color = (Rgba*)(data.get() + headerSize);
		if (!getImage(image, color)) {
			return nullptr;
		}
	}
	else if (ColorDepth::Index == static_cast<ColorDepth>(imageData.colorDepth_)) {
		//8bitIndex
		ColorParet* colorData = (ColorParet*)(data.get() + headerSize);
		u8* mapData = data.get() + headerSize + (sizeof(ColorParet) * imageData.colorMap_.colorMapLength_);
		if (!getImage(image, colorData, mapData)) {
			return nullptr;
		}
	}
	return data;
}

//bmpヘッダ情報初期化
void Bmp::initHeader(BmpHeader* bmpData) {

	bmpData->bmpFile_.fileType_ = 0;
	bmpData->bmpFile_.fileFullSize_ = 0;			//ファイル全体のサイズ取得
	bmpData->bmpFile_.reserved1_ = 0;				//基本的に0
	bmpData->bmpFile_.reserved2_ = 0;				//基本的に0
	bmpData->bmpFile_.offBits_ = 0;					//画像データ部の開始位置
	bmpData->bmpInfo_.size_ = 0;					//infoヘッダのサイズ
	bmpData->bmpInfo_.width_ = 0;					//画像の横幅を割り当て
	bmpData->bmpInfo_.height_ = 0;					//画像の縦幅を割り当て
	bmpData->bmpInfo_.bitCount_ = 0;				//カラータイプの設定
	bmpData->bmpInfo_.planes_ = 1;					//基本的に１
	bmpData->bmpInfo_.compression_ = 0;				//圧縮形式の設定(非圧縮：0)
	bmpData->bmpInfo_.sizeImage_ = 0;				//画像データ分のサイズ取得
	bmpData->bmpInfo_.xPelsPerMeter_ = 0;			//0でいい	
	bmpData->bmpInfo_.yPelsPerMeter_ = 0;			//0でいい	
	bmpData->bmpInfo_.colorUsed_ = 0;				//使用しているカラーパレットの数
	bmpData->bmpInfo_.colorImportant_ = 0;			//カラーパレットの中の重要色の数

	bmpData->bmpInfo_.redMask_ = 0;					//カラーマスク Red
	bmpData->bmpInfo_.greenMask_ = 0;				//カラーマスク Green
	bmpData->bmpInfo_.blueMask_ = 0;				//カラーマスク Blue
	bmpData->bmpInfo_.alphaMask_ = 0;				//カラーマスク Alpha
	bmpData->bmpInfo_.colorSpaceType_ = 0;			//色空間

	bmpData->bmpInfo_.cieRGB_.cieR_.x_ = 0;			//0でいい
	bmpData->bmpInfo_.cieRGB_.cieR_.y_ = 0;			//0でいい
	bmpData->bmpInfo_.cieRGB_.cieR_.z_ = 0;			//0でいい
	bmpData->bmpInfo_.cieRGB_.cieG_.x_ = 0;			//0でいい
	bmpData->bmpInfo_.cieRGB_.cieG_.y_ = 0;			//0でいい
	bmpData->bmpInfo_.cieRGB_.cieG_.z_ = 0;			//0でいい
	bmpData->bmpInfo_.cieRGB_.cieB_.x_ = 0;			//0でいい
	bmpData->bmpInfo_.cieRGB_.cieB_.y_ = 0;			//0でいい
	bmpData->bmpInfo_.cieRGB_.cieB_.z_ = 0;			//0でいい

	bmpData->bmpInfo_.gammaRed_ = 0;				//Rのガンマ値
	bmpData->bmpInfo_.gammaGreen_ = 0;				//Gのガンマ値
	bmpData->bmpInfo_.gammaBlue_ = 0;				//Bのガンマ値
}

//拡張子判定(.bmpであればtrueを返す)
const bool Bmp::isExt(const std::string_view filePath)
{
	if (getExt(filePath).compare(".bmp") == 0) {
		return true;
	}
	return false;
}

const bool Bmp::writeFile(Image* image, const std::string_view filePath)
{
	FileConvert fileconvert;

	ImageData imageData = image->getImageData();
	u32 imageDataSize = image->getImageDataSize();

	//画像データサイズ取得
	if (ColorDepth::Rgb == static_cast<ColorDepth>(imageData.colorDepth_)) {
		u32 padding = imageData.width_ % 4;	//パディング処理判定
		imageDataSize = imageData.height_ * (imageData.width_ * (imageData.colorDepth_ / byteSize) + padding);//画像データサイズ取得
	}

	std::unique_ptr<u8[]>data = createData(image);	//画像データ作成

	u32 fileSize = sizeof(BmpHeader) + imageDataSize;	//ファイルサイズ計算
	if (imageData.colorMap_.isColorMap_ != 0) {			//カラーパレット使用時サイズ変更
		fileSize += sizeof(ColorParet) * imageData.colorMap_.colorMapLength_;
	}

	bool isWrite = fileconvert.writeFile(data.get(), filePath, fileSize);	//ファイル書き出し(画像データ作成)
	return isWrite;
}

//反転チェック
const bool Bmp::isInvert(const s32 bmpHeight) {
	//格納順を判定
	if (bmpHeight < 0) {
		return true;
	}
	return false;
}

//画像データ取得(24bit)
const bool Bmp::getImage(Image* image, u8* color)
{
	ImageData imageData = image->getImageData();
	u32 pd = imageData.width_ % 4;	//パディング判定
	u32 colorOffset = 0;
	u32 rgbaOffset = 0;
	u32 imageDataSize = image->getImageDataSize();
	//画像データ設定
	for (u32 y = 0; y < imageData.height_; ++y) {
		for (u32 x = 0; x < imageData.width_; ++x) {
			if (colorOffset >= imageDataSize + (imageData.height_ * pd)) {
				printf("image size num over \n");
				return false;
			}
			//画像データをcolorに格納
			if (!image->getColorData(color, colorOffset, rgbaOffset)) {
				return false;
			}
			colorOffset += 3;
			rgbaOffset++;
		}
		//パディング処理
		if (pd != 0) {
			for (u32 i = 0; i < pd; ++i) {
				color[colorOffset] = 0;
				colorOffset += 1;
			}
		}
	}
	return true;
}

//画像データ取得(32bit)
const bool Bmp::getImage(Image* image ,Rgba* color) {
	ImageData imageData = image->getImageData();
	//画像データ設定
	for (u32 y = 0; y < imageData.height_; ++y) {
		for (u32 x = 0; x < imageData.width_; ++x) {
			u32 offset = y * imageData.width_ + x;
			if (offset >= image->getImageDataSize()) {
				printf("image size num over \n");
				return false;
			}
			//画像データをdataに格納
			if (!image->getColorData(color, offset)) {
				return false;
			}
		}
	}
	return true;
}

//画像データ取得(8bit)
const bool Bmp::getImage(Image* image, ColorParet* paretData, u8* mapData) 
{
	ImageData imageData = image->getImageData();

	//カラーパレットを取得
	for (u32 i = 0; i < imageData.colorMap_.colorMapLength_; ++i) {
		Rgba color = image->getColorData(i);
		(paretData + i)->red_ = color.r_;
		(paretData + i)->green_ = color.g_;
		(paretData + i)->blue_ = color.b_;
		(paretData + i)->reserved_ = 0;
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