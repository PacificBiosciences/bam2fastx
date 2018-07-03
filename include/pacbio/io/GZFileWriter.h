// Programmer: Derek Barnett

#pragma once

#include <stdexcept>
#include <string>

#include <zlib.h>
#include "AbstractWriter.h"

namespace PacBio {
namespace Postprimary {

// simple RAII wrapper for writing gzFiles
class GZFileWriter : public AbstractWriter
{
public:
    GZFileWriter(const std::string& filename,
                 const std::string& mode)
        : f_(gzopen(filename.c_str(), mode.c_str()))
    {
        if (!f_)
            throw std::runtime_error("Error opening file: " + filename + " for writing");
    }

    ~GZFileWriter(void) override
    { gzclose(f_); }

    int Write(const char* buffer, unsigned len) override
    { return gzwrite(f_, buffer, len); }

private:
    gzFile f_;
};

}} // ::PacBio::Postprimary
