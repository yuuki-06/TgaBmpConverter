#include "bmp.h"

std::unique_ptr<Image> Bmp::loadData(const std::string_view filePath)
{

	FileConvert fileconvert;
	std::unique_ptr<u8[]>data = fileconvert.readFile(filePath);		//�t�@�C���ǂݍ���

	BmpHeader* bmpData = (BmpHeader*)data.get();

	//bmp�f�[�^��́A�ۊ�
	ImageData imageData;
	imageData.height_ = bmpData->bmpInfo_.height_;				//�摜�c���擾�A�ۊ�
	imageData.width_ = bmpData->bmpInfo_.width_;				//�摜�����擾�A�ۊ�
	imageData.colorDepth_ = bmpData->bmpInfo_.bitCount_;		//�F�[�x�擾
	//�J���[�^�C�v���
	switch (static_cast<BmpColorType>(bmpData->bmpInfo_.bitCount_))
	{
	case BmpColorType::BmpNoneImage ://�C���[�W�Ȃ�
		imageData.colorType_ = ColorType::NoneImage;
		break;
	case BmpColorType::BmpFullColor ://�t���J���[
		imageData.colorType_ = ColorType::FullColor;
		break;
	case BmpColorType::BmpGrayScale ://���m�N��
		imageData.colorType_ = ColorType::GrayScale;
		break;
	default:
		printf(" get color type failed \n ");
		return nullptr;
		break;
	}

	//�摜�f�[�^�̔��]�`�F�b�N
	imageData.isInvert_.isHeightInvert_ = isInvert(bmpData->bmpInfo_.height_);
	imageData.isInvert_.isWidthInvert_ = false;	//���E���]����

	std::unique_ptr<Image>image_ = std::make_unique<Image>(imageData);	//�C���[�W�f�[�^�ۊǗp

	//�摜�f�[�^���i�[
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
	
	//�摜�f�[�^�T�C�Y�擾
	if (ColorDepth::Rgb == static_cast<ColorDepth>(imageData.colorDepth_)) {
		u32 padding = imageData.width_ % 4;	//�p�f�B���O��������
		imageDataSize = imageData.height_ * (imageData.width_ * (imageData.colorDepth_ / byteSize) + padding);//�摜�f�[�^�T�C�Y�擾
	}
	
	//�J���[�p���b�g�g�p���T�C�Y�ύX
	if (imageData.colorMap_.isColorMap_ != 0) {
		imageDataSize += sizeof(ColorParet) * imageData.colorMap_.colorMapLength_;
	}

	std::unique_ptr<u8[]>data(new u8[sizeof(BmpHeader) + imageDataSize]);

	BmpHeader* bmpData = (BmpHeader*)data.get();

	initHeader(bmpData);	//�w�b�_�̈ꕔ��������

	//bmpHeader���蓖��
	bmpData->bmpFile_.fileType_ = bmpFileType;
	bmpData->bmpFile_.fileFullSize_ = sizeof(BmpHeader) + imageDataSize;		//�t�@�C���S�̂̃T�C�Y�擾
	bmpData->bmpFile_.offBits_ = sizeof(BmpHeader);								//�摜�f�[�^���̊J�n�ʒu
	bmpData->bmpInfo_.size_ = sizeof(BmpInfoHeader);							//info�w�b�_�̃T�C�Y
	bmpData->bmpInfo_.width_ = imageData.width_;								//�摜�̉��������蓖��

	if (!imageData.isInvert_.isHeightInvert_) {
		bmpData->bmpInfo_.height_ = imageData.height_;							//�摜�̏c�������蓖��
	}
	else {//true�Ȃ甽�]
		bmpData->bmpInfo_.height_ = -imageData.height_;							//�摜�̏c�������蓖��(�z�񔽓])
	}

	bmpData->bmpInfo_.bitCount_ = imageData.colorDepth_;						//�J���[�^�C�v�̐ݒ�
	bmpData->bmpInfo_.compression_ = 0;											//���k�`���̐ݒ�(�񈳏k�F0)
	bmpData->bmpInfo_.sizeImage_ = imageDataSize;								//�摜�f�[�^���̃T�C�Y�擾
	bmpData->bmpInfo_.xPelsPerMeter_ = 0;										//0�ł���	
	bmpData->bmpInfo_.yPelsPerMeter_ = 0;										//0�ł���	
	u32 headerSize = bmpData->bmpFile_.offBits_;
	bmpData->bmpInfo_.colorUsed_ = imageData.colorMap_.colorMapLength_;
	bmpData->bmpInfo_.colorImportant_ = imageData.colorMap_.colorMapEntryBit_;

	//�摜�f�[�^�ݒ�
	if (ColorDepth::Rgb == static_cast<ColorDepth>(imageData.colorDepth_)) {
		//24bit
		u8* color = data.get() + headerSize;	//RGB�̏ꍇ�p�f�B���O���l��
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

//bmp�w�b�_��񏉊���
void Bmp::initHeader(BmpHeader* bmpData) {

	bmpData->bmpFile_.fileType_ = 0;
	bmpData->bmpFile_.fileFullSize_ = 0;			//�t�@�C���S�̂̃T�C�Y�擾
	bmpData->bmpFile_.reserved1_ = 0;				//��{�I��0
	bmpData->bmpFile_.reserved2_ = 0;				//��{�I��0
	bmpData->bmpFile_.offBits_ = 0;					//�摜�f�[�^���̊J�n�ʒu
	bmpData->bmpInfo_.size_ = 0;					//info�w�b�_�̃T�C�Y
	bmpData->bmpInfo_.width_ = 0;					//�摜�̉��������蓖��
	bmpData->bmpInfo_.height_ = 0;					//�摜�̏c�������蓖��
	bmpData->bmpInfo_.bitCount_ = 0;				//�J���[�^�C�v�̐ݒ�
	bmpData->bmpInfo_.planes_ = 1;					//��{�I�ɂP
	bmpData->bmpInfo_.compression_ = 0;				//���k�`���̐ݒ�(�񈳏k�F0)
	bmpData->bmpInfo_.sizeImage_ = 0;				//�摜�f�[�^���̃T�C�Y�擾
	bmpData->bmpInfo_.xPelsPerMeter_ = 0;			//0�ł���	
	bmpData->bmpInfo_.yPelsPerMeter_ = 0;			//0�ł���	
	bmpData->bmpInfo_.colorUsed_ = 0;				//�g�p���Ă���J���[�p���b�g�̐�
	bmpData->bmpInfo_.colorImportant_ = 0;			//�J���[�p���b�g�̒��̏d�v�F�̐�

	bmpData->bmpInfo_.redMask_ = 0;					//�J���[�}�X�N Red
	bmpData->bmpInfo_.greenMask_ = 0;				//�J���[�}�X�N Green
	bmpData->bmpInfo_.blueMask_ = 0;				//�J���[�}�X�N Blue
	bmpData->bmpInfo_.alphaMask_ = 0;				//�J���[�}�X�N Alpha
	bmpData->bmpInfo_.colorSpaceType_ = 0;			//�F���

	bmpData->bmpInfo_.cieRGB_.cieR_.x_ = 0;			//0�ł���
	bmpData->bmpInfo_.cieRGB_.cieR_.y_ = 0;			//0�ł���
	bmpData->bmpInfo_.cieRGB_.cieR_.z_ = 0;			//0�ł���
	bmpData->bmpInfo_.cieRGB_.cieG_.x_ = 0;			//0�ł���
	bmpData->bmpInfo_.cieRGB_.cieG_.y_ = 0;			//0�ł���
	bmpData->bmpInfo_.cieRGB_.cieG_.z_ = 0;			//0�ł���
	bmpData->bmpInfo_.cieRGB_.cieB_.x_ = 0;			//0�ł���
	bmpData->bmpInfo_.cieRGB_.cieB_.y_ = 0;			//0�ł���
	bmpData->bmpInfo_.cieRGB_.cieB_.z_ = 0;			//0�ł���

	bmpData->bmpInfo_.gammaRed_ = 0;				//R�̃K���}�l
	bmpData->bmpInfo_.gammaGreen_ = 0;				//G�̃K���}�l
	bmpData->bmpInfo_.gammaBlue_ = 0;				//B�̃K���}�l
}

//�g���q����(.bmp�ł����true��Ԃ�)
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

	//�摜�f�[�^�T�C�Y�擾
	if (ColorDepth::Rgb == static_cast<ColorDepth>(imageData.colorDepth_)) {
		u32 padding = imageData.width_ % 4;	//�p�f�B���O��������
		imageDataSize = imageData.height_ * (imageData.width_ * (imageData.colorDepth_ / byteSize) + padding);//�摜�f�[�^�T�C�Y�擾
	}

	std::unique_ptr<u8[]>data = createData(image);	//�摜�f�[�^�쐬

	u32 fileSize = sizeof(BmpHeader) + imageDataSize;	//�t�@�C���T�C�Y�v�Z
	if (imageData.colorMap_.isColorMap_ != 0) {			//�J���[�p���b�g�g�p���T�C�Y�ύX
		fileSize += sizeof(ColorParet) * imageData.colorMap_.colorMapLength_;
	}

	bool isWrite = fileconvert.writeFile(data.get(), filePath, fileSize);	//�t�@�C�������o��(�摜�f�[�^�쐬)
	return isWrite;
}

//���]�`�F�b�N
const bool Bmp::isInvert(const s32 bmpHeight) {
	//�i�[���𔻒�
	if (bmpHeight < 0) {
		return true;
	}
	return false;
}

//�摜�f�[�^�擾(24bit)
const bool Bmp::getImage(Image* image, u8* color)
{
	ImageData imageData = image->getImageData();
	u32 pd = imageData.width_ % 4;	//�p�f�B���O����
	u32 colorOffset = 0;
	u32 rgbaOffset = 0;
	u32 imageDataSize = image->getImageDataSize();
	//�摜�f�[�^�ݒ�
	for (u32 y = 0; y < imageData.height_; ++y) {
		for (u32 x = 0; x < imageData.width_; ++x) {
			if (colorOffset >= imageDataSize + (imageData.height_ * pd)) {
				printf("image size num over \n");
				return false;
			}
			//�摜�f�[�^��color�Ɋi�[
			if (!image->getColorData(color, colorOffset, rgbaOffset)) {
				return false;
			}
			colorOffset += 3;
			rgbaOffset++;
		}
		//�p�f�B���O����
		if (pd != 0) {
			for (u32 i = 0; i < pd; ++i) {
				color[colorOffset] = 0;
				colorOffset += 1;
			}
		}
	}
	return true;
}

//�摜�f�[�^�擾(32bit)
const bool Bmp::getImage(Image* image ,Rgba* color) {
	ImageData imageData = image->getImageData();
	//�摜�f�[�^�ݒ�
	for (u32 y = 0; y < imageData.height_; ++y) {
		for (u32 x = 0; x < imageData.width_; ++x) {
			u32 offset = y * imageData.width_ + x;
			if (offset >= image->getImageDataSize()) {
				printf("image size num over \n");
				return false;
			}
			//�摜�f�[�^��data�Ɋi�[
			if (!image->getColorData(color, offset)) {
				return false;
			}
		}
	}
	return true;
}

//�摜�f�[�^�擾(8bit)
const bool Bmp::getImage(Image* image, ColorParet* paretData, u8* mapData) 
{
	ImageData imageData = image->getImageData();

	//�J���[�p���b�g���擾
	for (u32 i = 0; i < imageData.colorMap_.colorMapLength_; ++i) {
		Rgba color = image->getColorData(i);
		(paretData + i)->red_ = color.r_;
		(paretData + i)->green_ = color.g_;
		(paretData + i)->blue_ = color.b_;
		(paretData + i)->reserved_ = 0;
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