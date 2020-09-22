// Programmer: Armin Töpfer

#include <string>

#include <pacbio/io/FileWriters.h>

#include <pbbam/BamFile.h>
#include <pbbam/BamRecord.h>
#include <pbbam/PbiFilterQuery.h>

#include <pbcopper/cli2/CLI.h>
#include <pbcopper/cli2/Interface.h>

#include "Version.h"

using namespace PacBio::BAM;
using namespace PacBio::CLI_v2;

namespace Options {

const PacBio::CLI_v2::PositionalArgument Input {
R"({
    "name" : "input",
    "description" : "Input file(s)."
})"};

const PacBio::CLI_v2::Option OutputPrefix {
R"({
    "names" : ["o", "output"],
    "description" : [
        "Prefix of output filenames, '-' implies streaming. Streaming not supported ",
        "with compression nor with split_barcodes"
    ],
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
    "description" : "Split output into multiple FASTA files, by barcode pairs."
})"};

const PacBio::CLI_v2::Option SeqIdPrefix {
R"({
    "names" : ["p", "seqid-prefix"],
    "description" : "Prefix for sequence IDs in headers",
    "type" : "string"
})"};


} // namespace Options

static constexpr size_t wrapLength = 60;

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
        return EXIT_FAILURE;
    }
    else if (outputPrefix.empty())
    {
        std::cerr << "ERROR: OUTPUT EMPTY." << std::endl;
        return EXIT_FAILURE;
    }

    std::unique_ptr<PacBio::Postprimary::AbstractWriterFactory> fact;
    std::string suffix;
    std::string namePrefix = seqIdPrefix;

    if (uncompressed) {
        fact.reset(new PacBio::Postprimary::PlainFileWriterFactory);
        suffix = ".fasta";
    } else {
        // setup open mode string
        std::string mode = "wb";
        mode += std::to_string(compressionLevel);
        fact.reset(new PacBio::Postprimary::GZFileWriterFactory(mode));
        suffix = ".fasta.gz";
    }
    bool isStreamed;
    if ("-" == outputPrefix) {
        if (splitBarcodes) {
            const auto msg = "Streamed mode cannot be used with barcodes.";
            throw std::runtime_error(msg);
        }
        // Ignore 'suffix'.
        isStreamed = true;
    } else {
        isStreamed = false;
    }
    // setup output files
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

static PacBio::CLI_v2::Interface CreateCLI()
{
    PacBio::CLI_v2::Interface i{
        "bam2fasta",
        "Converts multiple BAM and/or DataSet files into into gzipped FASTA file(s).",
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
