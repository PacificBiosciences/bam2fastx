#pragma once

#include <stdexcept>
#include <string>
#include <cstdio>

#include "AbstractWriter.h"

namespace PacBio {
namespace Postprimary {

// simple RAII wrapper for writing plain files
class PlainFileWriter : public AbstractWriter
{
public:
    PlainFileWriter(const std::string& filename,
                    const std::string& mode)
        : f_(fopen(filename.c_str(), mode.c_str()))
    {
        if (!f_)
            throw std::runtime_error("Error opening plain file: '" + filename + "' for writing");
    }

    ~PlainFileWriter(void) override
    { fclose(f_); }

    int Write(const char* buffer, unsigned len) override
    { return std::fwrite(buffer, 1U, len, f_); }

private:
    FILE* f_;
};

}} // ::PacBio::Postprimary
