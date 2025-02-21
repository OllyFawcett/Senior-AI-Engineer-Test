#pragma once

#include <map>
#include <string>
#include <vector>
#include <filesystem>

class CSVWriter
{
public:

	CSVWriter(const std::string& filename);
	CSVWriter();
	~CSVWriter();

	bool AddColumnHeader(const std::string& columnHeader);
	bool AddRowValue(const std::string& columnHeader, const std::string& value);
	bool WriteRow();

private:
	bool WriteColumnHeaders();
	bool FileExists(const std::filesystem::path& filename);

	std::string m_filename;
	std::vector<std::string> m_columnHeaders;
	std::map<std::string, std::string> m_values;
	bool m_columnHeadersWritten;
};
