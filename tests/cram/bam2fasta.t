
# bam2fasta tests

Test if the output is identical
  $ $__PBTEST_BAM2FASTA_EXE $TESTDIR/../data/RSII.bam -o rsii -u
  $ wc -c rsii.fasta | awk '{ print $1 }'
  23191
  $ md5sum rsii.fasta | awk '{ print $1 }'
  83cb99fda4554d21be54901146f44b03
  $ rm rsii.fasta

Test if compression works
  $ $__PBTEST_BAM2FASTA_EXE $TESTDIR/../data/RSII.bam -o rsii_1 -c 1
  $ $__PBTEST_BAM2FASTA_EXE $TESTDIR/../data/RSII.bam -o rsii_9 -c 9
  $ file1=$(wc -c < rsii_1.fasta.gz); file2=$(wc -c < rsii_9.fasta.gz); if [ "$file1" -gt "$file2" ] ; then echo "true"; else echo "false"; fi
  true

Test header seqid prefix
  $ $__PBTEST_BAM2FASTA_EXE $TESTDIR/../data/RSII.bam -o rsii_prefixed -u -p testprefix
  $ grep -c ">testprefix" rsii_prefixed.fasta
  10
