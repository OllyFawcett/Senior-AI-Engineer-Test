#pragma once

#include <map>
#include <string>
#include <vector>
#include <filesystem>
#include <mutex>


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
	bool SaveTextData(const std::string& filename, const std::stringstream& sstream, const bool append = false);


	std::string m_filename;
	std::vector<std::string> m_columnHeaders;
	std::map<std::string, std::string> m_values;
	bool m_columnHeadersWritten;
	std::recursive_mutex m_mutex;
};
