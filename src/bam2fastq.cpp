// Programmer: Armin Töpfer

#include <cassert>
#include <string>

#include <pacbio/io/FileWriters.h>

#include <pbbam/BamFile.h>
#include <pbbam/BamRecord.h>
#include <pbbam/PbiFilterQuery.h>

#include <pbcopper/cli2/CLI.h>
#include <pbcopper/cli2/Interface.h>

#include "Version.h"

using namespace PacBio::BAM;

namespace Options {

const PacBio::CLI_v2::PositionalArgument Input {
R"({
    "name" : "input",
    "description" : "Input file(s)."
})"};

const PacBio::CLI_v2::Option OutputPrefix {
R"({
    "names" : ["o", "output"],
    "description" : "Prefix of output filenames",
    "type" : "string"
})"};

const PacBio::CLI_v2::Option CompressionLevel {
R"({
    "names" : ["c"],
    "description" : "Gzip compression level [1-9]",
    "type" : "int",
    "default" : 1
})"};

const PacBio::CLI_v2::Option Uncompressed {
R"({
    "names" : ["u"],
    "description" : "Do not compress. In this case, we will not add .gz, and we ignore any -c setting."
})"};

const PacBio::CLI_v2::Option SplitBarcodes {
R"({
    "names" : ["split-barcodes"],
    "description" : "Split output into multiple FASTQ files, by barcode pairs."
})"};

const PacBio::CLI_v2::Option SeqIdPrefix {
R"({
    "names" : ["p", "seqid-prefix"],
    "description" : "Prefix for sequence IDs in headers",
    "type" : "string"
})"};


} // namespace Options

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

static int Runner(const PacBio::CLI_v2::Results& options)
{
    // Get source args
    const std::vector<std::string> files = options.PositionalArguments();
    const std::string outputPrefix = options[Options::OutputPrefix];
    const std::string seqIdPrefix = options[Options::SeqIdPrefix];
    const int compressionLevel = options[Options::CompressionLevel];
    const bool uncompressed = options[Options::Uncompressed];
    const bool splitBarcodes = options[Options::SplitBarcodes];

    if (files.size() == 0)
    {
        std::cerr << "ERROR: INPUT EMPTY." << std::endl;
        return 0;
    }
    else if (outputPrefix.empty())
    {
        std::cerr << "ERROR: OUTPUT EMPTY." << std::endl;
        return 0;
    }

    std::unique_ptr<PacBio::Postprimary::AbstractWriterFactory> fact;
    std::string suffix;
    std::string namePrefix = seqIdPrefix;

    if (uncompressed) {
        fact.reset(new PacBio::Postprimary::PlainFileWriterFactory);
        suffix = ".fastq";
    } else {
        // setup open mode string
        std::string mode = "wb";
        mode += std::to_string(compressionLevel);
        fact.reset(new PacBio::Postprimary::GZFileWriterFactory(mode));
        suffix = ".fastq.gz";
    }
    // setup output files
    const bool isStreamed = false;
    PacBio::Postprimary::AbstractWriters writers(*fact,
                                               files,
                                               outputPrefix,
                                               suffix,
                                               isStreamed,
                                               splitBarcodes);
    // for each input file
    for (const auto& input : files)
    {
        // setup query (respecting dataset filter, if present)
        const PbiFilter filter = PbiFilter::FromDataSet(input);
        const PbiFilterQuery bamQuery(filter, input);

        // Iterate over all records and convert online
        bool mayHaveInsertQV = true;
        bool firstRecord = true;
        for (const auto& record : bamQuery)
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
            const auto name  = namePrefix + record.FullName();
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

    return EXIT_SUCCESS;
}

static PacBio::CLI_v2::Interface CreateCLI()
{
    PacBio::CLI_v2::Interface i{
        "bam2fastq",
        "Converts multiple BAM and/or DataSet files into into gzipped FASTQ file(s).",
        BAM2FASTX_VERSION};

    i.DisableLogLevelOption()
     .DisableLogFileOption()
     .DisableNumThreadsOption();

    i.AddPositionalArgument(Options::Input);
    i.AddOptionGroup("Options",
    {
        Options::OutputPrefix,
        Options::CompressionLevel,
        Options::Uncompressed,
        Options::SplitBarcodes,
        Options::SeqIdPrefix
    });

    return i;
}

// Entry point
int main(int argc, char* argv[])
{
    return PacBio::CLI_v2::Run(argc, argv, CreateCLI(), &Runner);
}
