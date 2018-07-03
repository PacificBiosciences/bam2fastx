// Programmer: Armin Töpfer

#include <cassert>
#include <string>

#include <pacbio/io/FileWriters.h>

#include <pbbam/BamFile.h>
#include <pbbam/BamRecord.h>
#include <pbbam/PbiFilterQuery.h>

#include <pbcopper/cli/CLI.h>

#include "Version.h"

using namespace PacBio::BAM;

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

static int Runner(const PacBio::CLI::Results& options)
{
    // Get source args
    const std::vector<std::string> files = options.PositionalArguments();

    if (files.size() == 0)
    {
        std::cerr << "ERROR: INPUT EMPTY." << std::endl;
        return 0;
    }
    else if (options["output"].empty())
    {
        std::cerr << "ERROR: OUTPUT EMPTY." << std::endl;
        return 0;
    }

    std::unique_ptr<PacBio::Postprimary::AbstractWriterFactory> fact;
    std::string suffix;
    std::string namePrefix = options["seqid_prefix"];

    if (options["uncompressed"]) {
        fact.reset(new PacBio::Postprimary::PlainFileWriterFactory);
        suffix = ".fastq";
    } else {
        // setup open mode string
        std::string mode = "wb";
        mode += std::to_string(static_cast<int>(options["compression"]));
        fact.reset(new PacBio::Postprimary::GZFileWriterFactory(mode));
        suffix = ".fastq.gz";
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

static PacBio::CLI::Interface CreateCLI()
{
    using Option = PacBio::CLI::Option;
    PacBio::CLI::Interface i{"bam2fastq",
                             "Converts multiple BAM and/or DataSet files into into gzipped FASTQ file(s).",
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
        {"split_barcodes", {"split-barcodes"}, "Split output into multiple FASTQ files, by barcode pairs.", Option::BoolType()},
        {"seqid_prefix", {"p", "seqid-prefix"}, "Prefix for sequence IDs in headers", Option::StringType("")}
    });

    return i;
}

// Entry point
int main(int argc, char* argv[])
{
    return PacBio::CLI::Run(argc, argv, CreateCLI(), &Runner);
}
