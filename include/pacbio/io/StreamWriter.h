#pragma once

#include <stdexcept>
#include <string>
#include <ostream>

#include "AbstractWriter.h"

namespace PacBio {
namespace Postprimary {

// simple RAII wrapper for writing to a stream
class StreamWriter : public AbstractWriter
{
public:
    StreamWriter(std::ostream& fout)
        : fout_(fout)
    {
        if (!fout_.good())
            throw std::runtime_error("Problem with output stream.");
    }

    ~StreamWriter(void) override
    {}

    int Write(const char* buffer, unsigned len) override
    {
        fout_.write(buffer, int(len));
        if (!fout_.good())
            return 0;
        else
            return int(len);
    }

private:
    std::ostream& fout_;
};

}} // ::PacBio::Postprimary
