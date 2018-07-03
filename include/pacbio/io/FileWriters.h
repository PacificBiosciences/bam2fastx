// Programmer: Derek Barnett

#pragma once

#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "pbbam/BamRecord.h"
#include "pbbam/DataSet.h"
#include "pbbam/PbiRawData.h"
#include "AbstractWriter.h"
#include "GZFileWriter.h"
#include "PlainFileWriter.h"

namespace PacBio {
namespace Postprimary {

// injectable factory to decouple the choice of Writer from the
// FileWriters class (which is responsible only for maintaining
// the Writers, which can vary per Record)
class AbstractWriterFactory {
public:
    virtual AbstractWriter* newAbstractWriter(std::string const& fn) const
    {return nullptr;}
    virtual ~AbstractWriterFactory()
    {}
};

class GZFileWriterFactory : public AbstractWriterFactory {
public:
    /// \param[in] mode         output mode (e.g. "wb"+compressionLevel)
    explicit GZFileWriterFactory(std::string const& mode)
        : mode_(mode)
    {}
    AbstractWriter* newAbstractWriter(std::string const& fn) const override
    {return new GZFileWriter(fn, mode_);}
private:
    std::string const mode_;
};

class PlainFileWriterFactory : public AbstractWriterFactory {
public:
    AbstractWriter* newAbstractWriter(std::string const& fn) const override
    {return new PlainFileWriter(fn, "wb");}
};

// wrapper around single or multiple AbstractWriters (when splitting by barcode)
class AbstractWriters
{
public:
    /// Constructs a lookup structure of one or more AbstractWriters.
    ///
    /// When not splitting by barcodes, this will create a single AbstractWriter
    /// operating on <outputPrefix>.<outputSuffix>.
    ///
    /// For the barcode splitting mode, the filenames will be something like:
    /// <outputPrefix>3_5.<outputSuffix> where {3,5} are a barcode pair observed
    /// in the data. All records containing this barcode pair will be directed
    /// toward this writer.
    ///
    /// \param[in] filenames    input filenames (BAM, DataSetXML)
    /// \param[in] outputPrefix filename prefix
    /// \param[in] outputSuffix filename suffix, beginning with dot (e.g. ".fastq.gz" or ".fasta.gz")
    ///
    AbstractWriters(
                  AbstractWriterFactory const& fact,
                  const std::vector<std::string>& filenames,
                  const std::string& outputPrefix,
                  const std::string& outputSuffix,
                  const bool isSplittingBarcodes)
        : isSplittingBarcodes_(isSplittingBarcodes)
        , singleWriter_(nullptr)
        , fact_(fact)
    {
        if (isSplittingBarcodes_)
            CreateBarcodeWriters(filenames, outputPrefix, outputSuffix);
        else
        {
            const std::string outFn = outputPrefix + outputSuffix;
            singleWriter_.reset(newAbstractWriter(outFn));
        }
    }

public:
    AbstractWriter* WriterForRecord(const PacBio::BAM::BamRecord& b) const
    {
        // splitting records on barcode values
        if (isSplittingBarcodes_)
        {
            BarcodePair barcodes;
            if (b.HasBarcodes())
                barcodes = b.Barcodes();
            else
                barcodes = std::make_pair(static_cast<uint16_t>(-1),
                                          static_cast<uint16_t>(-1));

            auto iter = barcodeWriterLookup_.find(barcodes);
            if (iter != barcodeWriterLookup_.cend())
                return iter->second.get();
            else
                throw std::runtime_error("Output file not found for record: " + b.FullName());
        }

        // single output
        else
            return singleWriter_.get();
    }

private:
    typedef std::pair<uint16_t, uint16_t>          BarcodePair;
    typedef std::unique_ptr<AbstractWriter>          AbstractWriterPtr;
    typedef std::map<BarcodePair, AbstractWriterPtr> WriterLookup;

private:
    bool isSplittingBarcodes_;
    AbstractWriterPtr singleWriter_;
    WriterLookup barcodeWriterLookup_;
    AbstractWriterFactory const& fact_;

private:
    AbstractWriter* newAbstractWriter(std::string const& fn) const
    {
        return fact_.newAbstractWriter(fn);
    }

    void CreateBarcodeWriters(const std::vector<std::string>& filenames,
                              const std::string& outputPrefix,
                              const std::string& outputSuffix)
    {

        for (const auto& fn : filenames)
        {
           const PacBio::BAM::DataSet ds(fn);
           const auto bamFiles = ds.BamFiles();
           for (const auto& bamFile : bamFiles)
           {
               // fetch barcode data
               PacBio::BAM::PbiRawData idx(bamFile.PacBioIndexFilename());
               if (idx.NumReads() == 0) continue;
               else if (!idx.HasBarcodeData())
                   throw std::runtime_error(bamFile.Filename() + " has no barcoding data");
               const auto& barcodeData = idx.BarcodeData();
               const auto& bcFor = barcodeData.bcForward_;
               const auto& bcRev = barcodeData.bcReverse_;
               const uint32_t numReads = idx.NumReads();
               if (bcFor.size() != numReads || bcRev.size() != numReads)
                   throw std::runtime_error("malformed index: "+idx.Filename());

               // create new output file if unique barcode pair encountered
               for (uint32_t i = 0; i < numReads; ++i)
               {
                   const BarcodePair barcodes = std::make_pair(static_cast<uint16_t>(bcFor.at(i)),
                                                               static_cast<uint16_t>(bcRev.at(i)));
                   const auto iter = barcodeWriterLookup_.find(barcodes);
                   if (iter == barcodeWriterLookup_.end())
                   {
                       const std::string barcodeString =
                       {
                          "." + std::to_string(barcodes.first) + 
                          "_" + std::to_string(barcodes.second)
                       };
                       const std::string outFn = outputPrefix + barcodeString + outputSuffix;
                       barcodeWriterLookup_.emplace(barcodes, AbstractWriterPtr{newAbstractWriter(outFn)});
                   }
               }
           }
        }
    }
};

}} // ::PacBio::Postprimary
