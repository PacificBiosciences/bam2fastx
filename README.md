<h1 align="center">BAM2fastx tools</h1>
<p align="center">Conversion of PacBio BAM files into gzipped fasta and fastq files, including splitting of barcoded data</p>

***

## Availability
Latest version can be installed via bioconda package `bam2fastx`.

Please refer to our [official pbbioconda page](https://github.com/PacificBiosciences/pbbioconda)
for information on Installation, Support, License, Copyright, and Disclaimer.

## Usage

Both tools have an identical interface and take BAM and/or DataSet files as input. Examples:

        bam2fasta -o projectName m54008_160330_053509.subreads.bam
        bam2fastq -o myEcoliRuns m54008_160330_053509.subreads.bam m54008_160331_235636.subreads.bam
        bam2fasta -o myHumanGenome m54012_160401_000001.subreadset.xml

DISCLAIMER
----------
THIS WEBSITE AND CONTENT AND ALL SITE-RELATED SERVICES, INCLUDING ANY DATA, ARE PROVIDED "AS IS," WITH ALL FAULTS, WITH NO REPRESENTATIONS OR WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, ANY WARRANTIES OF MERCHANTABILITY, SATISFACTORY QUALITY, NON-INFRINGEMENT OR FITNESS FOR A PARTICULAR PURPOSE. YOU ASSUME TOTAL RESPONSIBILITY AND RISK FOR YOUR USE OF THIS SITE, ALL SITE-RELATED SERVICES, AND ANY THIRD PARTY WEBSITES OR APPLICATIONS. NO ORAL OR WRITTEN INFORMATION OR ADVICE SHALL CREATE A WARRANTY OF ANY KIND. ANY REFERENCES TO SPECIFIC PRODUCTS OR SERVICES ON THE WEBSITES DO NOT CONSTITUTE OR IMPLY A RECOMMENDATION OR ENDORSEMENT BY PACIFIC BIOSCIENCES.
