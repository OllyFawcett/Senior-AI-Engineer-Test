#include <algorithm>
#include <sstream>

#include "CSVWriter.h"
#include "FileUtils.h"
#include "Logger.h"

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
    Logger::Get() << LoggerLevel::Verbose << __FUNCTION__ << " " << __LINE__ << " "
        << "Entry" << std::endl;
    m_columnHeaders.push_back(columnHeader);
    return true;
}

bool CSVWriter::AddRowValue(const std::string& columnHeader, const std::string& value)
{
    Logger::Get() << LoggerLevel::Verbose << __FUNCTION__ << " " << __LINE__ << " "
        << "Entry" << std::endl;
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
    Logger::Get() << LoggerLevel::Verbose << __FUNCTION__ << " " << __LINE__ << " "
        << "Entry" << std::endl;
    bool success = false;
    if (!m_columnHeadersWritten || !FileUtils::FileExists(m_filename))
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
        success = FileUtils::SaveTextData(m_filename, sstream, true);
        m_values.clear();
    }
    return success;
}

bool CSVWriter::WriteColumnHeaders()
{
    Logger::Get() << LoggerLevel::Verbose << __FUNCTION__ << " " << __LINE__ << " "
        << "Entry" << std::endl;
    bool success = false;
    if (!m_filename.empty())
    {
        std::stringstream sstream;
        for (const std::string& columnHeader : m_columnHeaders)
        {
            sstream << columnHeader << ",";
        }
        sstream << std::endl;
        success = FileUtils::SaveTextData(m_filename, sstream, false);
    }
    m_columnHeadersWritten = true;
    return success;
}
