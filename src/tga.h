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
	u8 fieldSize_;			//�w�b�_�[�̌�ɑ����AID�̒���
	u8 isColorMap_;			//�J���[�}�b�v�̗L�� 0:�Ȃ��@1:����
	u8 colorDataType_;		//�f�[�^�`��
	u16 colorMapEntry_;		//�J���[�}�b�v�̈ʒu
	u16 colorMapLength_;	//�J���[�}�b�v�̒���
	u8 colorEntryBit_;
	s16 xPosition_;			//�摜�̂w���W
	s16 yPosition_;			//�摜�̂x���W
	s16 width_;				//�摜�̉���
	s16 height_;			//�摜�̏c��
	s8 colorDepth_;			//�F�[�x
	u8 type_;				//����..
};
#pragma pack()

class Tga :public Format{
public:
	//�摜�f�[�^�ǂݍ���
	virtual std::unique_ptr<Image> loadData(const std::string_view filePath);

	//�摜�f�[�^�쐬
	virtual std::unique_ptr<u8[]> createData(Image *image);

	//�w�b�_��񏉊���
	void initHeader(TgaHeader* tgaData);

	//�g���q����
	virtual const bool isExt(const std::string_view filePath);

	//�t�@�C�������o��
	virtual const bool writeFile(Image* image, const std::string_view filePath);

	//�摜�f�[�^�̔��]�`�F�b�N
	const InvertType isInvert(const u8 type);

	//�摜�f�[�^�擾
	const bool getImage(Image* image, Rgb* color);
	const bool getImage(Image* image, Rgba* color);
	const bool getImage(Image* image, Rgb* paretData, u8* mapData);
};