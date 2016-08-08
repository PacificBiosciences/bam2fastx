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

// Programmer: Armin Töpfer

#include <cassert>
#include <string>

#include "OptionParser.h"
#include "pacbio/io/FileWriters.h"

#include "pbbam/BamFile.h"
#include "pbbam/BamRecord.h"
#include "pbbam/PbiFilterQuery.h"

#include "Version.h"

using namespace PacBio::BAM;

std::string DetermineReadType(const BamHeader&);
//static constexpr size_t wrapLength = 60;

// Entry point
int main(int argc, char* argv[])
{
    auto parser = optparse::OptionParser().description(
        "Converts multiple BAM and/or DataSet files into gzipped FASTQ file(s).")
    .usage("-o outputPrefix [options] movieName.(subreads|hqregion|polymerase).bam")
    .epilog("Example: bam2fastq movieName.subreads1.bam movieName2.subreads.bam -o myProject");

    parser.add_option("-v", "--version").dest("version").action("store_true").help("Print the tool version and exit");

    auto groupMand = optparse::OptionGroup(parser, "Mandatory parameters");
    groupMand.add_option("-o").dest("output").metavar("STRING").help("Prefix of output filenames");
    parser.add_option_group(groupMand);

    auto groupOpt = optparse::OptionGroup(parser, "Optional parameters");
    groupOpt.add_option("-c").dest("compression").metavar("INT").help("Gzip compression level [1-9]");
    groupOpt.add_option("-u").dest("uncompressed").action("store_true").help("Do not compress. In this case, we will not add .gz, and we ignore any -c setting.");
    // groupOpt.add_option("-q").dest("qual").action("store_true").help("Use QUAL field as qualities (Default: IQV)");
    groupOpt.add_option("--split-barcodes").dest("split_barcodes").action("store_true")
            .help("Split output into multiple FASTQ files, by barcode pairs. ");
    parser.add_option_group(groupOpt);

    optparse::Values options = parser.parse_args(argc, argv);
    std::vector<std::string> args = parser.args();

    // Print version
    if (options.get("version"))
    {
        std::cerr << "bam2fastq version: " << BAM2FASTX_VERSION << std::endl;
        exit(0);
    }

    // ensure proper args
    if (args.size() == 0)
    {
        std::cerr << "ERROR: INPUT EMPTY." << std::endl;
        return 0;
    }
    else if (options["output"].empty())
    {
        std::cerr << "ERROR: OUTPUT EMPTY." << std::endl;
        return 0;
    }

    // bool insertQV = !options.get("qual");

    std::unique_ptr<PacBio::Postprimary::AbstractWriterFactory> fact;
    std::string suffix;

    if (options.get("uncompressed")) {
        fact.reset(new PacBio::Postprimary::PlainFileWriterFactory);
        suffix = ".fastq";
    } else {
        // setup open mode string
        std::string mode = "wb";
        mode += options["compression"].empty() ? "1" : options["compression"];
        fact.reset(new PacBio::Postprimary::GZFileWriterFactory(mode));
        suffix = ".fastq.gz";
    }
    // setup output files
    PacBio::Postprimary::AbstractWriters writers(*fact,
                                               args,
                                               options["output"],
                                               suffix,
                                               options.is_set("split_barcodes"));
    // for each input file
    for (const auto& input : args)
    {
        // setup query (respecting dataset filter, if present)
        const PbiFilter filter = PbiFilter::FromDataSet(input);
        const PbiFilterQuery bamQuery(filter, input);

        // Iterate over all records and convert online
        bool mayHaveInsertQV = true;
        bool firstRecord = true;
        for (const auto record : bamQuery)
        {
            // grab data for this file from first  record
            if (firstRecord)
            {
                firstRecord = false;
                const auto readType = DetermineReadType(record.Header());
                if (readType.compare("CCS") == 0)
                    mayHaveInsertQV = false;
                else
                    mayHaveInsertQV = true;
            }

            // get record data
            const auto name  = record.FullName();
            const auto seq   = record.Sequence();
            std::string qual = record.Qualities().Fastq();

            // If empty QUAL field, try falling back to InsertionQV.
            // If that failed, error.
            if (qual.empty())
            {
                if (mayHaveInsertQV && record.HasInsertionQV())
                    qual = record.InsertionQV().Fastq();
                if (qual.empty())
                {
                    std::string msg = "Error retrieving qualities for record: " +
                                      record.FullName() +
                                       " - no data in QUAL field or iq tag.";
                    throw std::runtime_error(msg);
                }
            }

            // get appropriate writer for record
            auto fastqStream = writers.WriterForRecord(record);
            assert(fastqStream);

            // write record as FASTQ
            fastqStream->Write("@", 1);
            fastqStream->Write(name.c_str(), name.size());
            fastqStream->Write("\n", 1);
            fastqStream->Write(seq.c_str(), seq.size());
            fastqStream->Write("\n+\n", 3);
            fastqStream->Write(qual.c_str(), qual.size());
            fastqStream->Write("\n", 1);
        }
    }
}

std::string DetermineReadType(const BamHeader& header)
{
    std::string globalReadType;
    for (const auto& rg : header.ReadGroups())
    {
        auto readType = rg.ReadType();
        std::transform(readType.begin(), readType.end(), readType.begin(), ::toupper);
        if (globalReadType.empty())
            globalReadType = readType;
        else if (readType.compare(globalReadType) != 0)
            throw std::runtime_error("Multiple read groups. Could not identify bam type.");
    }
    return globalReadType;
}
