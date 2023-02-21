
//�t�@�C���ǂݍ���
const std::unique_ptr<u8[]> FileConvert::readFile(const std::string_view fileName) {
	u32 fileSize;
	FILE* fp = fopen(fileName.data(), "rb");
	fseek(fp, 0, SEEK_END);
	fileSize = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	if (fp == nullptr) {
		perror(fileName.data());
		printf("read file failed \n");
		return nullptr;
	}

	//�w�b�_�o�b�t�@�i�[�p���������m��
	std::unique_ptr<u8[]>data(new u8[fileSize]);
	fread(data.get(), sizeof(u8), fileSize, fp);
	fclose(fp);
	return data;
}

//�t�@�C�������o��
const bool FileConvert::writeFile(const u8* data, const std::string_view writeFileName , const u32 fileSize) {

	FILE* fp = fopen(writeFileName.data(), "wb");
	if (fp == nullptr) {
		perror(writeFileName.data());
		return false;
	}
	fwrite(data, sizeof(u8), fileSize, fp);
	fclose(fp);

	return true;
}