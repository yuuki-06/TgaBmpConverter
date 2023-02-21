#include "image.h"

Image::Image(){}

Image::Image(const ImageData& imageData) {

	imageSize_ = imageData.height_ * imageData.width_;
	imageData_ = imageData;
}

//画像データ格納
const bool Image::setColorData(const Rgba* color, const u32 offset) {
	if (offset >= imageSize_) {
		printf("image size num over \n");
		return false;
	}	
	colorDataRGBA_[offset] = *color;
	return true;
}

//画像データ格納
const bool Image::setColorData(const Rgb* color, const u32 offset) {
	if (offset >= imageSize_) {
		printf("image size num over \n");
		return false;
	}
	colorDataRGBA_[offset].r_ = color->r_;
	colorDataRGBA_[offset].g_ = color->g_;
	colorDataRGBA_[offset].b_ = color->b_;
	colorDataRGBA_[offset].a_ = 255;

	return true;
}

//イメージデータ取得
const ImageData Image::getImageData() {
	return imageData_;
}

//画像データサイズ取得
const u32 Image::getImageDataSize() {
	u32 pixcelSize = imageData_.colorDepth_ / byteSize;
	u32 imageDataSize = pixcelSize * imageSize_;
	return imageDataSize;
}

//画像サイズ取得
const u32 Image::getImageSize(){
	return imageSize_;
}

//画像データ格納
const bool Image::setImage(const Rgba* colorData){
	colorDataRGBA_ = std::make_unique<Rgba[]>(imageSize_);
	u32 offset = 0;
	//画像データ読み込み（Imageクラスに格納）
	//iswidthInvert = trueなら左右反転
	for (u32 y = 0; y < imageData_.height_; ++y) {
		u32 offsetY = imageData_.width_ * y;
		for (u32 x = 0; x < imageData_.width_; ++x) {
			//true : 左右反転
			u32 offsetX = imageData_.isInvert_.isWidthInvert_ ? imageData_.width_ - x : x;	
			if (!setColorData(colorData + (offsetY + offsetX), offset)) {
				return false;
			}
			offset++;
		}
	}
	return true;
}

//画像データ格納
const bool Image::setImage(const Rgb* colorData) {
	colorDataRGBA_ = std::make_unique<Rgba[]>(imageSize_);
	u32 offset = 0;
	//画像データ読み込み（Imageクラスに格納）
	//iswidthInvert = trueなら左右反転
	for (u32 y = 0; y < imageData_.height_; ++y) {
		u32 offsetY = imageData_.width_ * y;
		for (u32 x = 0; x < imageData_.width_; ++x) {
			//true : 左右反転
			u32 offsetX = imageData_.isInvert_.isWidthInvert_ ? imageData_.width_ - x : x;
			if (!setColorData(colorData + (offsetY + offsetX), offset)) {
				return false;
			}
			offset++;
		}
	}
	return true;
}

//画像データ格納
const bool Image::setImage(const Rgb* paretData, u8* mapData) {

	u32 paretsSize = imageData_.colorMap_.colorMapLength_ * imageData_.colorMap_.colorMapEntryBit_ / byteSize;
	colorDataIdx_ = std::make_unique<u8[]>(imageSize_);
	colorDataRGBA_ = std::make_unique<Rgba[]>(imageData_.colorMap_.colorMapLength_);
	for (u32 i = 0; i < imageData_.colorMap_.colorMapLength_; ++i) {
		colorDataRGBA_[i].r_ = (paretData + i)->r_;
		colorDataRGBA_[i].g_ = (paretData + i)->g_;
		colorDataRGBA_[i].b_ = (paretData + i)->b_;
		colorDataRGBA_[i].a_ = 255;
	}

	for (u32 i = 0; i < imageSize_; ++i) {
		colorDataIdx_[i] = mapData[i];
	}
	return true;
}

//色情報取得
const bool Image::getColorData(u8* color , const u32 colorOffset , const u32 rgbaOffset)
{
	if (rgbaOffset>imageSize_) {
		printf("offset over imageSize \n");
		return false;
	}
	//画像データをdataに格納
	color[colorOffset] = colorDataRGBA_[rgbaOffset].r_;
	color[colorOffset + 1] = colorDataRGBA_[rgbaOffset].g_;
	color[colorOffset + 2] = colorDataRGBA_[rgbaOffset].b_;
	return true;
}

//色情報取得
const bool Image::getColorData(Rgb* color, const u32 offset)
{
	if (offset > imageSize_) {
		printf("offset over imageSize \n");
		return false;
	}
	//画像データをdataに格納
	color[offset].r_ = colorDataRGBA_[offset].r_;
	color[offset].g_ = colorDataRGBA_[offset].g_;
	color[offset].b_ = colorDataRGBA_[offset].b_;
	return true;
}

//色情報取得
const bool Image::getColorData(Rgba* color, const u32 offset)
{
	if (offset > imageSize_) {
		printf("offset over imageSize \n");
		return false;
	}
	//画像データをdataに格納
	color[offset].r_ = colorDataRGBA_[offset].r_;
	color[offset].g_ = colorDataRGBA_[offset].g_;
	color[offset].b_ = colorDataRGBA_[offset].b_;
	color[offset].a_ = colorDataRGBA_[offset].a_;
	return true;
}

//色情報取得
const Rgba Image::getColorData(const u32 offset) {
	return colorDataRGBA_[offset];
}

//画像データ取得
const u8 Image::getIndexData(const u32 offset) {
	return colorDataIdx_[offset];
}

//RLE圧縮解凍処理
bool Image::decompressedRLE(u8* data, u32 headerSize)
{
	u32 imageDataSize = getImageDataSize();
	std::unique_ptr<u8[]>decompList(new u8[imageDataSize]);
	u32 listOffset = 0;
	u32 dataStart = headerSize;

	u32 loopSize;
	if (static_cast<ColorDepth>(imageData_.colorDepth_) == ColorDepth::Rgb) {
		loopSize = 3;
	}
	else if (static_cast<ColorDepth>(imageData_.colorDepth_) == ColorDepth::Rgba) {
		loopSize = 4;
	}

	while (listOffset < imageDataSize) {
		u32 loopCount = 0;
		bool isLoop = data[dataStart] & isLoopMask;	//loop or noloop
		loopCount = (data[dataStart] & loopCountMask) + 1;	//ループ回数を取得
		if (!isLoop) {	//noloop
			for (u32 n = 0; n < loopCount * loopSize; ++n) {
				dataStart += 1;
				decompList[listOffset+n] = data[dataStart];
			}
			dataStart += 1;
		}
		else if (isLoop) {	//loop
			if (static_cast<ColorDepth>(imageData_.colorDepth_) == ColorDepth::Rgb) {
				for (u32 n = 0; n < loopCount; ++n) {
					decompList[listOffset + (n * 3)] = data[dataStart + 1];
					decompList[listOffset + ((n * 3) + 1)] = data[dataStart + 2];
					decompList[listOffset + ((n * 3) + 2)] = data[dataStart + 3];
				}

			}
			else if (static_cast<ColorDepth>(imageData_.colorDepth_) == ColorDepth::Rgba) {
				for (u32 n = 0; n < loopCount; ++n) {
					decompList[listOffset + (n * 4)] = data[dataStart + 1];
					decompList[listOffset + ((n * 4) + 1)] = data[dataStart + 2];
					decompList[listOffset + ((n * 4) + 2)] = data[dataStart + 3];
					decompList[listOffset + ((n * 4) + 3)] = data[dataStart + 4];
				}
			}
			dataStart += loopSize + 1;
		}

		listOffset += loopCount * loopSize;
		if (listOffset > imageDataSize) {
			printf("over Data decompList size \n");
			return false;
		}
	}

	//画像データ格納
	if (static_cast<ColorDepth>(imageData_.colorDepth_) == ColorDepth::Rgb) {
		Rgb* colorData = (Rgb*)decompList.get();
		if (!setImage(colorData)) {
			printf("set image failed \n");
			return false;
		}
	}
 	else if (static_cast<ColorDepth>(imageData_.colorDepth_) == ColorDepth::Rgba) {
		Rgba* colorData = (Rgba*)decompList.get();
		if (!setImage(colorData)) {
			printf("set image failed \n");
			return false;
		}
	}
	
	return true;
}