#pragma once

namespace PacBio {
namespace Postprimary {

// simple RAII wrapper for writing All the Things
class AbstractWriter
{
public:
    virtual ~AbstractWriter()
    {}
    virtual int Write(const char* buffer, unsigned len)
    {return 0;}
};

}} // ::PacBio::Postprimary
