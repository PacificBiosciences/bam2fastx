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

#include <string>

#include <pacbio/io/FileWriters.h>

#include <pbbam/BamFile.h>
#include <pbbam/BamRecord.h>
#include <pbbam/PbiFilterQuery.h>

#include <pbcopper/cli/CLI.h>

#include "Version.h"

using namespace PacBio::BAM;

static constexpr size_t wrapLength = 60;

static int Runner(const PacBio::CLI::Results& options)
{
    // Get source args
    const std::vector<std::string> files = options.PositionalArguments();

    if (files.size() == 0)
    {
        std::cerr << "ERROR: INPUT EMPTY." << std::endl;
        return EXIT_FAILURE;
    }
    else if (options["output"].empty())
    {
        std::cerr << "ERROR: OUTPUT EMPTY." << std::endl;
        return EXIT_FAILURE;
    }

    std::unique_ptr<PacBio::Postprimary::AbstractWriterFactory> fact;
    std::string suffix;
    std::string namePrefix = options["seqid_prefix"];

    if (options["uncompressed"]) {
        fact.reset(new PacBio::Postprimary::PlainFileWriterFactory);
        suffix = ".fasta";
    } else {
        // setup open mode string
        std::string mode = "wb";
        mode += std::to_string(static_cast<int>(options["compression"]));
        fact.reset(new PacBio::Postprimary::GZFileWriterFactory(mode));
        suffix = ".fasta.gz";
    }
    // setup output files
    PacBio::Postprimary::AbstractWriters writers(*fact,
                                               files,
                                               options["output"],
                                               suffix,
                                               options["split_barcodes"]);
    // for each input file
    for (const auto& input : files)
    {
        // setup query (respecting dataset filter, if present)
        const PbiFilter filter = PbiFilter::FromDataSet(input);
        const PbiFilterQuery bamQuery(filter, input);

        // Iterate over all records and convert online
        for (const auto record : bamQuery)
        {
            // get appropriate writer for record
            auto fastaStream = writers.WriterForRecord(record);
            assert(fastaStream);

            // write header
            const auto name = namePrefix + record.FullName();
            fastaStream->Write(">", 1);
            fastaStream->Write(name.c_str(), name.size());
            fastaStream->Write("\n", 1);

            // write seq
            const auto seq  = record.Sequence();
            size_t seqSize = seq.size();
            const auto& sstr = seq.c_str();
            for (size_t i = 0; i < seqSize; i+=wrapLength)
            {
                size_t end = i + wrapLength > seqSize ? seqSize - i : wrapLength;
                fastaStream->Write(sstr + i, end);
                fastaStream->Write("\n", 1);
            }
        }
    }

    return EXIT_SUCCESS;
}

static PacBio::CLI::Interface CreateCLI()
{
    using Option = PacBio::CLI::Option;
    PacBio::CLI::Interface i{"bam2fasta",
                             "Converts multiple BAM and/or DataSet files into into gzipped FASTA file(s).",
                             BAM2FASTX_VERSION};

    i.AddHelpOption();      // use built-in help output
    i.AddVersionOption();   // use built-in version output

    i.AddPositionalArguments({
        {"input",  "Input file.",  "INPUT"}
    });

    i.AddOptions(
    {
        {"output", {"o", "output"}, "Prefix of output filenames", Option::StringType("")},
        {"compression", {"c"}, "Gzip compression level [1-9]", Option::IntType(1)},
        {"uncompressed", {"u"}, "Do not compress. In this case, we will not add .gz, and we ignore any -c setting.", Option::BoolType()},
        {"split_barcodes", {"split-barcodes"}, "Split output into multiple FASTA files, by barcode pairs.", Option::BoolType()},
        {"seqid_prefix", {"p", "seqid-prefix"}, "Prefix for sequence IDs in headers", Option::StringType("")}
    });

    return i;
}

// Entry point
int main(int argc, char* argv[])
{
    return PacBio::CLI::Run(argc, argv, CreateCLI(), &Runner);
}
