<p align="center">
  <img src="http://www.pacb.com/wp-content/themes/pacific-biosciences/img/pacific-biosciences-logo-mobile.svg" alt="PacBio logo"/>
</p>
<h1 align="center">BAM2fastx tools</b></h1>
The BAM2fastx tools allow conversion of PacBio BAM files into gzipped fasta and
fastq files, including demultiplexing of barcoded data.

## DEPENDENCIES

Please install Boost and zlib.

## INSTALL

- Download source from github

        git clone https://github.com/PacificBiosciences/bam2fastx && cd bam2fastx

- Sync your code with the latest git code base:

        git submodule update --init --remote

- Create build directory

        mkdir build && cd build

- Invoke cmake with Boost and zlib as system libraries

        cmake ..

- Or with user-specific paths

        cmake -DBoost_INCLUDE_DIRS=/path/to/boost/include -DZLIB_INCLUDE_DIRS=/path/to/zlib/include -DZLIB_LIBRARIES=/path/to/zlib/lib ..

- Build

        make

## USAGE

Both tools have an identical interface and take BAM and/or DataSet files as input. Examples:

        bam2fasta -o projectName m54008_160330_053509.subreads.bam
        bam2fastq -o myEcoliRuns m54008_160330_053509.subreads.bam m54008_160331_235636.subreads.bam
        bam2fasta -o myHumanGenome m54012_160401_000001.subreadset.xml

## HELP

Support is only provided for official and stable
[SMRT Analysis builds](http://www.pacb.com/products-and-services/analytical-software/)
provided by PacBio and not for source builds.

DISCLAIMER
----------
THIS WEBSITE AND CONTENT AND ALL SITE-RELATED SERVICES, INCLUDING ANY DATA, ARE PROVIDED "AS IS," WITH ALL FAULTS, WITH NO REPRESENTATIONS OR WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, ANY WARRANTIES OF MERCHANTABILITY, SATISFACTORY QUALITY, NON-INFRINGEMENT OR FITNESS FOR A PARTICULAR PURPOSE. YOU ASSUME TOTAL RESPONSIBILITY AND RISK FOR YOUR USE OF THIS SITE, ALL SITE-RELATED SERVICES, AND ANY THIRD PARTY WEBSITES OR APPLICATIONS. NO ORAL OR WRITTEN INFORMATION OR ADVICE SHALL CREATE A WARRANTY OF ANY KIND. ANY REFERENCES TO SPECIFIC PRODUCTS OR SERVICES ON THE WEBSITES DO NOT CONSTITUTE OR IMPLY A RECOMMENDATION OR ENDORSEMENT BY PACIFIC BIOSCIENCES.
