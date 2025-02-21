#include <algorithm>
#include <sstream>
#include <fstream>
#include "CSVWriter.h"

CSVWriter::CSVWriter(const std::string& filename) : m_filename(filename), m_columnHeadersWritten(false)
{
}

CSVWriter::CSVWriter() : m_filename("")
{
}

CSVWriter::~CSVWriter()
{
}

bool CSVWriter::AddColumnHeader(const std::string& columnHeader)
{
    const std::lock_guard lg(m_mutex);
    m_columnHeaders.push_back(columnHeader);
    return true;
}

bool CSVWriter::AddRowValue(const std::string& columnHeader, const std::string& value)
{
    const std::lock_guard lg(m_mutex);
    bool success = false;
    if (std::find(m_columnHeaders.begin(), m_columnHeaders.end(), columnHeader) != m_columnHeaders.end())
    {
        m_values[columnHeader] = value;
        success = true;
    }
    else if (!m_columnHeadersWritten)
    {
        AddColumnHeader(columnHeader);
        m_values[columnHeader] = value;
        success = true;
    }
    return success;
}

bool CSVWriter::WriteRow()
{
    const std::lock_guard lg(m_mutex);
    bool success = false;
    if (!m_columnHeadersWritten || !FileExists(m_filename))
    {
        WriteColumnHeaders();
    }

    if (!m_filename.empty())
    {
        std::stringstream sstream;
        for (const std::string& columnHeader : m_columnHeaders)
        {
            std::map<std::string, std::string>::iterator itr = m_values.find(columnHeader);
            if (itr != m_values.end())
            {
                sstream << m_values[columnHeader];
            }
            sstream << ",";
        }
        sstream << std::endl;
        success = SaveTextData(m_filename, sstream, true);
        m_values.clear();
    }
    return success;
}

bool CSVWriter::WriteColumnHeaders()
{
    bool success = false;
    if (!m_filename.empty())
    {
        std::stringstream sstream;
        for (const std::string& columnHeader : m_columnHeaders)
        {
            sstream << columnHeader << ",";
        }
        sstream << std::endl;
        success = SaveTextData(m_filename, sstream, false);
    }
    m_columnHeadersWritten = true;
    return success;
}

bool CSVWriter::FileExists(const std::filesystem::path& filename)
{
    bool exists = false;
    std::filesystem::path file(filename);
    exists = std::filesystem::exists(file);
    return exists;
}

bool CSVWriter::SaveTextData(const std::string& filename, const std::stringstream& sstream, const bool append)
{
    bool success = false;

    std::ios_base::openmode mode = append ? std::ios::app : std::ios::out;
    std::ofstream out(filename, mode);
    if (out)
    {
        out << sstream.str();
        out.close();
        success = true;
    }

    return success;
}
