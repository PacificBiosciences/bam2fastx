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

#include "OptionParser.h"
#include "pacbio/gzfile/GZFileWriters.h"

#include "pbbam/BamFile.h"
#include "pbbam/BamRecord.h"
#include "pbbam/PbiFilterQuery.h"

#include "Version.h"

using namespace PacBio::BAM;

static constexpr size_t wrapLength = 60;

// Entry point
int main(int argc, char* argv[])
{
    auto parser = optparse::OptionParser().description(
        "Converts multiple BAM and/or DataSet files into into gzipped FASTA file(s).")
    .usage("-o outputPrefix [options] movieName.(subreads|hqregion|polymerase).bam")
    .epilog("Example: bam2fasta movieName.subreads1.bam movieName2.subreads.bam -o myProject");

    parser.add_option("-v", "--version").dest("version").action("store_true").help("Print the tool version and exit");

    auto groupMand = optparse::OptionGroup(parser, "Mandatory parameters");
    groupMand.add_option("-o").dest("output").metavar("STRING").help("Prefix of output filenames");
    parser.add_option_group(groupMand);

    auto groupOpt = optparse::OptionGroup(parser, "Optional parameters");
    groupOpt.add_option("-c").dest("compression").metavar("INT").help("Gzip compression level [1-9]");
    groupOpt.add_option("--split-barcodes").dest("split_barcodes").action("store_true")
            .help("Split output into multiple FASTQ files, by barcode pairs. ");
    parser.add_option_group(groupOpt);

    optparse::Values options = parser.parse_args(argc, argv);
    std::vector<std::string> args = parser.args();

    // Print version
    if (options.get("version"))
    {
        std::cerr << "bam2fasta version: " << BAM2FASTX_VERSION << std::endl;
        exit(0);
    }

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

    // setup open mode string
    std::string mode = "wb";
    mode += options["compression"].empty() ? "1" : options["compression"];

    // setup output files
    PacBio::Postprimary::GZFileWriters writers(args,
                                               mode,
                                               options["output"],
                                               ".fasta.gz",
                                               options.is_set("split_barcodes"));
    // for each input file
    for (const auto& input : args)
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
            const auto name = record.FullName();
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
}
