#include "format.h"

const std::string Format::getExt(const std::string_view filePath)
{
	std::string readPath = filePath.data();
	u32 ext = readPath.find_last_of(".");
	std::string extName = readPath.substr(ext, readPath.size() - ext);

	return extName;
}
