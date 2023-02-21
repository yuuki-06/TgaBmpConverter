#include "tga.h"

std::unique_ptr<Image> Tga::loadData(const std::string_view filePath)
{
	FileConvert fileconvert;
	std::unique_ptr<u8[]>data = fileconvert.readFile(filePath);		//�t�@�C���ǂݍ���


	TgaHeader* tgaData = (TgaHeader*)data.get();

	ImageData imageData;
	imageData.height_ = (u32)tgaData->height_;		//�摜�c���擾
	imageData.width_ = (u32)tgaData->width_;		//�摜�����擾

	//�J���[�^�C�v���
	switch (static_cast<TgaColorType>(tgaData->colorDataType_))
	{
	case TgaColorType::TgaNoneImage://�C���[�W�Ȃ�
		imageData.colorType_ = ColorType::NoneImage;
		break;
	case TgaColorType::TgaIndexColor://�C���f�b�N�X�J���[(256�F)
		imageData.colorType_ = ColorType::IndexColor;
		break;
	case TgaColorType::TgaFullColor://�t���J���[
		imageData.colorType_ = ColorType::FullColor;
		break;
	case TgaColorType::TgaGrayScale://���m�N��
		imageData.colorType_ = ColorType::GrayScale;
		break;
	case TgaColorType::TgaRLEIndexColor://�C���f�b�N�X�J���[(RLE���k)
		imageData.colorType_ = ColorType::RLEIndexColor;
		break;
	case TgaColorType::TgaRLEFullColor://�t���J���[(RLE���k)
		imageData.colorType_ = ColorType::RLEFullColor;
		break;
	case TgaColorType::TgaRLEGrayScale://���m�N��(RLE���k)
		imageData.colorType_ = ColorType::RLEGrayScale;
		break;
	default:
		printf(" get color type failed \n ");
		return nullptr;
		break;
	}
	imageData.colorDepth_ = (u16)tgaData->colorDepth_;		//�F�[�x�擾

	//�J���[�}�b�v�ݒ�
	imageData.colorMap_.isColorMap_ = tgaData->isColorMap_;
	imageData.colorMap_.colorMapEntry = tgaData->colorMapEntry_;
	imageData.colorMap_.colorMapLength_ = tgaData->colorMapLength_;
	imageData.colorMap_.colorMapEntryBit_ = tgaData->colorEntryBit_;
	
	//�摜�f�[�^�̔��]�`�F�b�N
	imageData.isInvert_.isHeightInvert_ = isInvert(tgaData->type_).isHeightInvert_;
	imageData.isInvert_.isWidthInvert_ = isInvert(tgaData->type_).isWidthInvert_;

	std::unique_ptr<Image>image = std::make_unique<Image>(imageData);

	if (ColorType::RLEFullColor == static_cast<ColorType>(imageData.colorType_)) {
		if (!image.get()->decompressedRLE(data.get(), sizeof(TgaHeader))) {
			return nullptr;
		}
	}
	else {
		//�摜�f�[�^�i�[
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

//�摜�f�[�^�쐬
std::unique_ptr<u8[]> Tga::createData(Image *image)
{
	ImageData imageData = image->getImageData();
	u32 imageDataSize = image->getImageDataSize();
	if (imageData.colorMap_.isColorMap_ != 0) {
		imageDataSize += imageData.colorMap_.colorMapLength_ * (imageData.colorMap_.colorMapEntryBit_ / byteSize);
	}
	std::unique_ptr<u8[]>data(new u8[sizeof(TgaHeader) + imageDataSize]);

	TgaHeader* tgaData = (TgaHeader*)data.get();

	//�w�b�_��񏉊���
	initHeader(tgaData);

	//tgaHeader���蓖��
	tgaData->fieldSize_ = 0;	//ID�̃t�B�[���h�T�C�Y�iID�Ȃ����0�j
	tgaData->isColorMap_ = imageData.colorMap_.isColorMap_;	//�J���[�}�b�v�̗L���i�����F0 �L��F1�j
	//�J���[�^�C�v�E�F�[�x�̐ݒ�
	switch (imageData.colorType_){
	case ColorType::NoneImage:	//�C���[�W�Ȃ�
		tgaData->colorDataType_ = static_cast<u8>(TgaColorType::TgaNoneImage);
		break;
	case ColorType::IndexColor:	//�C���f�b�N�X�J���[
		tgaData->colorDataType_ = static_cast<u8>(TgaColorType::TgaIndexColor);
		break;
	case ColorType::FullColor:	//�t���J���[
		tgaData->colorDataType_ = static_cast<u8>(TgaColorType::TgaFullColor);
		break;
	case ColorType::GrayScale:	//����
		tgaData->colorDataType_ = static_cast<u8>(TgaColorType::TgaGrayScale);
		break;
	case ColorType::RLEIndexColor:	//�C���f�b�N�X�J���[(RLE���k)
		tgaData->colorDataType_ = static_cast<u8>(TgaColorType::TgaRLEIndexColor);
		break;
	case ColorType::RLEFullColor:	//�t���J���[(RLE���k)
		tgaData->colorDataType_ = static_cast<u8>(TgaColorType::TgaRLEFullColor);
		break;
	case ColorType::RLEGrayScale:	//����(RLE���k)
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
	tgaData->xPosition_ = 0;							//�摜��x���W
	tgaData->yPosition_ = 0;							//�摜��y���W
	tgaData->width_ = imageData.width_;					//�摜�̉��������蓖��
	tgaData->height_ = imageData.height_;				//�摜�̉��c�������蓖��

	
	if (imageData.isInvert_.isHeightInvert_) {	//true�Ȃ�㉺���]
		tgaData->type_ = tgaTypeheightInvert;
	}
	else {
		tgaData->type_ = 0;
	}

	//�摜�f�[�^�ݒ�
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

//�w�b�_��񏉊���
void Tga::initHeader(TgaHeader* tgaData)
{
	tgaData->fieldSize_ = 0;							//ID�̃t�B�[���h�T�C�Y�iID�Ȃ����0�j
	tgaData->isColorMap_ = 0;							//�J���[�}�b�v�̗L���i�����F0 �L��F1�j
	tgaData->colorDataType_ = 0;
	tgaData->colorDepth_ = 0;							//�J���[�^�C�v�E�F�[�x�̐ݒ�
	tgaData->colorMapEntry_ = 0;
	tgaData->colorMapLength_ = 0;
	tgaData->colorEntryBit_ = 0;
	tgaData->xPosition_ = 0;
	tgaData->yPosition_ = 0;
	tgaData->width_ = 0;								//�摜�̉��������蓖��
	tgaData->height_ = 0;								//�摜�̉��c�������蓖��
	tgaData->type_ = 0;
}

//�g���q����(.tga�ł����true��Ԃ�)
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
	u32 fileSize = sizeof(TgaHeader) + image->getImageDataSize();	//�t�@�C���T�C�Y�v�Z
	if (imageData.colorMap_.isColorMap_ != 0) {
		fileSize += imageData.colorMap_.colorMapLength_ * (imageData.colorMap_.colorMapEntryBit_ / byteSize);
	}
	std::unique_ptr<u8[]>data = createData(image);	//�摜�f�[�^�쐬
	bool isWrite = fileconvert.writeFile(data.get(), filePath, fileSize);	//�t�@�C�������o��
	return isWrite;
}

//���]�`�F�b�N
const InvertType Tga::isInvert(const u8 type)
{
	InvertType invertType;
	invertType.isHeightInvert_ = type & tgaHeightInvert; // 5�� 0010 0000 �㉺���]����
	invertType.isWidthInvert_ = type & tgaWidthInvert;	// 4�� 0001 0000 ���E���]����

	return invertType;
}

//�摜�f�[�^�擾(24bit)
const bool Tga::getImage(Image* image, Rgb* color)
{
	ImageData imageData = image->getImageData();
	//�摜�f�[�^�ݒ�
	for (u32 y = 0; y < imageData.height_; ++y) {
		for (u32 x = 0; x < imageData.width_; ++x) {
			u32 offset = y * imageData.width_ + x;
			if (offset >= image->getImageSize()) {
				printf("image size num over \n");
				return false;
			}
			//�摜�f�[�^��data�Ɋi�[
			image->getColorData(color, offset);
		}
	}
	return true;
}

//�摜�f�[�^�擾(32bit)
const bool Tga::getImage(Image* image, Rgba* color)
{
	ImageData imageData = image->getImageData();
	//�摜�f�[�^�ݒ�
	for (u32 y = 0; y < imageData.height_; ++y) {
		for (u32 x = 0; x < imageData.width_; ++x) {
			u32 offset = y * imageData.width_ + x;
			if (offset >= image->getImageSize()) {
				printf("image size num over \n");
				return false;
			}
			//�摜�f�[�^��data�Ɋi�[
			image->getColorData(color, offset);
		}
	}
	return true;
}

//�摜�f�[�^�擾(8bit)
const bool Tga::getImage(Image* image ,Rgb* paretData ,u8* mapData)
{
	ImageData imageData = image->getImageData();

	//�J���[�p���b�g���擾
	for (u32 i = 0; i < imageData.colorMap_.colorMapLength_; ++i) {
		if (!image->getColorData(paretData, i)) {
			return false;
		}
	}

	//�摜�f�[�^�ݒ�
	for (u32 y = 0; y < imageData.height_; ++y) {
		for (u32 x = 0; x < imageData.width_; ++x) {
			u32 offset = y * imageData.width_ + x;
			if (offset >= image->getImageSize()) {
				printf("image size num over \n");
				return false;
			}
			//�摜�f�[�^��data�Ɋi�[
			mapData[offset] = image->getIndexData(offset);		
		}
	}
	return true;
}