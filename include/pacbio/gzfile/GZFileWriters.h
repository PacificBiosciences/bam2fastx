// Copyright (c) 2014-2016, Pacific Biosciences of California, Inc.
//
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted (subject to the limitations in the
// disclaimer below) provided that the following conditions are met:
//
//  * Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//
//  * Redistributions in binary form must reproduce the above
//    copyright notice, this list of conditions and the following
//    disclaimer in the documentation and/or other materials provided
//    with the distribution.
//
//  * Neither the name of Pacific Biosciences nor the names of its
//    contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
// NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE
// GRANTED BY THIS LICENSE. THIS SOFTWARE IS PROVIDED BY PACIFIC
// BIOSCIENCES AND ITS CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
// WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
// OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL PACIFIC BIOSCIENCES OR ITS
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
// USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
// OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
// SUCH DAMAGE.

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
#include "pacbio/gzfile/GZFileWriter.h"

namespace PacBio {
namespace Postprimary {

// wrapper around single or multiple GZFileWriters (when splitting by barcode)
class GZFileWriters
{
public:
    /// Constructs a lookup structure of one or more GZFileWriters.
    ///
    /// When not splitting by barcodes, this will create a single GZFileWriter
    /// operating on <outputPrefix>.<outputSuffix>.
    ///
    /// For the barcode splitting mode, the filenames will be something like:
    /// <outputPrefix>3_5.<outputSuffix> where {3,5} are a barcode pair observed
    /// in the data. All records containing this barcode pair will be directed
    /// toward this writer.
    ///
    /// \param[in] filenames    input filenames (BAM, DataSetXML)
    /// \param[in] mode         output mode (e.g. "wb"+compressionLevel)
    /// \param[in] outputPrefix filename prefix
    /// \param[in] outputSuffix filename suffix, beginning with dot (e.g. ".fastq.gz" or ".fasta.gz")
    ///
    GZFileWriters(const std::vector<std::string>& filenames,
                  const std::string& mode,
                  const std::string& outputPrefix,
                  const std::string& outputSuffix,
                  const bool isSplittingBarcodes)
        : isSplittingBarcodes_(isSplittingBarcodes)
        , singleWriter_(nullptr)
    {
        if (isSplittingBarcodes_)
            CreateBarcodeWriters(filenames, mode, outputPrefix, outputSuffix);
        else
        {
            const std::string outFn = outputPrefix + outputSuffix;
            singleWriter_.reset(new GZFileWriter(outFn, mode));
        }
    }

public:
    GZFileWriter* WriterForRecord(const PacBio::BAM::BamRecord& b) const
    {
        // splitting records on barcode values
        if (isSplittingBarcodes_)
        {
            BarcodePair barcodes;
            if (b.HasBarcodes)
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
    typedef std::unique_ptr<GZFileWriter>          GZFileWriterPtr;
    typedef std::map<BarcodePair, GZFileWriterPtr> WriterLookup;

private:
    bool isSplittingBarcodes_;
    GZFileWriterPtr singleWriter_;
    WriterLookup barcodeWriterLookup_;

private:
    void CreateBarcodeWriters(const std::vector<std::string>& filenames,
                              const std::string& mode,
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
               if (!idx.HasBarcodeData())
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
                          std::to_string(barcodes.first) + "_" +
                          std::to_string(barcodes.second)
                       };
                       const std::string outFn = outputPrefix + barcodeString + outputSuffix;
                       barcodeWriterLookup_.emplace(barcodes, GZFileWriterPtr{new GZFileWriter(outFn, mode)});
                   }
               }
           }
        }
    }
};

}} // ::PacBio::Postprimary
