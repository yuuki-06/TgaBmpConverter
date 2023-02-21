#pragma once

class Format {
public:
	virtual std::unique_ptr<Image> loadData(std::string_view filePath) = 0;

	virtual std::unique_ptr<u8[]> createData(Image* image) = 0;

	virtual const bool isExt(const std::string_view filePath) = 0;

	virtual const bool writeFile(Image* image, const std::string_view filePath) = 0;

	//Šg’£Žq‚ðŽæ“¾
	const std::string getExt(const std::string_view filePath);
};