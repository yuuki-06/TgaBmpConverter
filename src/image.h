#pragma once

constexpr u32 byteSize = 8;
constexpr u8 loopCountMask = 0x7f;
constexpr u8 isLoopMask = 0x80;

//�F�[�x
enum class ColorDepth {
	Index = 8,
	Rgb = 24,
	Rgba = 32,
};

//�摜�f�[�^�i�[��������p�t���O
struct InvertType {
	bool isHeightInvert_;
	bool isWidthInvert_;
};

//RGBA�̐F����ێ�����\����
struct Rgba {
	u8 r_ ;		// Red
	u8 g_ ;		// Green
	u8 b_ ;		// Blue
	u8 a_ ;		// Alpha
};

//RGB�̐F����ێ�����\����
struct Rgb {
	u8 r_;		// Red
	u8 g_;		// Green
	u8 b_;		// Blue
};

enum class ColorType {
	NoneImage,	//�C���[�W�Ȃ�
	IndexColor,	//�C���f�b�N�X�J���[(256�F)
	FullColor,	//�t���J���[
	GrayScale,	//����
	RLEIndexColor,	//�C���f�b�N�X�J���[(RLE���k)
	RLEFullColor,	//�t���J���[(RLE���k)
	RLEGrayScale,	//����(RLE���k)

	ColorType_MAX
};

struct ColorMap {
	u8 isColorMap_ = 0;			//�J���[�}�b�v�̗L��
	u32 colorMapEntry = 0;		//�J���[�}�b�v�̏����ʒu
	u32 colorMapLength_ = 0;	//�J���[�}�b�v�̒���
	u32 colorMapEntryBit_ = 0;	//�J���[�}�b�v��colorDepth
};

//�摜�f�[�^�ێ��̍\����
struct ImageData {
	u32 width_;				// ��
	u32 height_;			// ����
	ColorType colorType_;	//�f�[�^�^�C�v
	u16 colorDepth_;		// �F�[�x(bit/pixcel)
	InvertType isInvert_;	//���]����
	ColorMap colorMap_;		//�J���[�}�b�v�f�[�^
};

class Image {
private:
	ImageData imageData_;
	std::unique_ptr<Rgba[]>colorDataRGBA_;
	std::unique_ptr<u8[]>colorDataIdx_;	//indexcolor�g�p���摜�f�[�^�ۊ�

	u32 imageSize_;	//�摜�T�C�Y

public:
	Image();
	Image(const ImageData& imageData);

	//�C���[�W�f�[�^�擾
	const ImageData getImageData();

	//�摜�f�[�^�T�C�Y�擾(height*width*RGBA)
	const u32 getImageDataSize();
	
	//�摜�T�C�Y�擾(height*width)
	const u32 getImageSize();

	//�摜�f�[�^�i�[
	const bool setImage(const Rgba* colorData);
	const bool setImage(const Rgb* colorData);
	const bool setImage(const Rgb* paretData, u8* mapData);
	
	//�F���擾
	const bool getColorData(u8* color, const u32 colorOffset, const u32 rgbaOffset);
	const bool getColorData(Rgb* color, const u32 offset);
	const bool getColorData(Rgba* color, const u32 offset);
	const Rgba getColorData(const u32 offset);

	//�摜�f�[�^�擾
	const u8 getIndexData(const u32 offset);
	
private:
	//�摜�f�[�^�i�[
	const bool setColorData(const Rgba* color, const u32 num );
	const bool setColorData(const Rgb* color, const u32 num);
public:

	//RLE���k�𓀏���
	bool decompressedRLE(u8* data ,u32 headerSize);
};