#pragma once
#include"format.h"

constexpr u16 bmpFileType = 0x4D42;  // "BM"�����g���G���f�B�A���ŉ��߂����l

enum class BmpColorType {
	BmpNoneImage = 0,	//�C���[�W�Ȃ�
	BmpFullColor = 24,	//�t���J���[
	BmpGrayScale = 8,	//����
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
	u16 fileType_;      // �t�@�C���^�C�v�A�K��"BM"
	u32 fileFullSize_;  // �S�̃t�@�C���T�C�Y
	s16 reserved1_;		// �|�C���^�̃z�b�g�X�|�b�g��x���W
	s16 reserved2_;		// �|�C���^�̃z�b�g�X�|�b�g��y���W
	u32 offBits_;		// �t�@�C���w�b�_�̐擪�A�h���X����r�b�g�}�b�v�f�[�^�̐擪�A�h���X�܂ł̃I�t�Z�b�g�B�P�ʂ̓o�C�g�B
};

#pragma pack(1)
struct BmpInfoHeader {
	u32 size_;				// ���̍\���̂̃T�C�Y
	s32 width_;				// �摜�̕�
	s32 height_;			// �摜�̍���(���Ȃ�ォ�牺�A���Ȃ牺�����)
	u16 planes_;			// �摜�̖����A�ʏ�1
	u16 bitCount_;			// ��F�̃r�b�g��
	u32 compression_;		// ���k�`��
	u32 sizeImage_;			// �摜�f�[�^�̃T�C�Y�i�o�C�g���j
	u32 xPelsPerMeter_;		// �摜�̉������𑜓x���
	u32 yPelsPerMeter_;		// �摜�̏c�����𑜓x���
	u32 colorUsed_;			// �J���[�e�[�u���̐F��
	u32 colorImportant_;	// �\���ɕK�v�ȃJ���[�e�[�u���̐F��
	u32 redMask_;			//�J���[�}�X�N_R
	u32 greenMask_;			//�J���[�}�X�N_G
	u32 blueMask_;			//�J���[�}�X�N_B
	u32 alphaMask_;			//�J���[�}�X�N_A
	u32 colorSpaceType_;	//�F���
	CieRGB cieRGB_;			
	u32 gammaRed_;			//R�����K���}�l
	u32 gammaGreen_;		//G�����K���}�l
	u32 gammaBlue_;			//B�����K���}�l
};
#pragma pack()

struct BmpHeader {
	BmpFileHeader bmpFile_;		//bmp�t�@�C���w�b�_��
	BmpInfoHeader bmpInfo_;		//bmp���w�b�_��
};

//�J���[�p���b�g
struct ColorParet {
	u8 red_;
	u8 green_;
	u8 blue_;
	u8 reserved_;	//�\��̈�
};

class Bmp :public Format {
public:
	//�摜�f�[�^�ǂݍ���
	virtual std::unique_ptr<Image> loadData(const std::string_view filePath);

	//�摜�f�[�^�쐬
	virtual std::unique_ptr<u8[]> createData(Image* image);

	//�w�b�_��񏉊���
	void initHeader(BmpHeader* bmpData);

	//�g���q����
	virtual const bool isExt(const std::string_view filePath);

	//�t�@�C�������o��
	virtual const bool writeFile(Image* image, const std::string_view filePath);

	//�摜�f�[�^�̔��]�`�F�b�N
	const bool isInvert(const s32 bmpHeight);

	//�摜�f�[�^�擾
	const bool getImage(Image* image ,u8* color);
	const bool getImage(Image* image, Rgba* color);
	const bool getImage(Image* image, ColorParet* paretData, u8* mapData);
};