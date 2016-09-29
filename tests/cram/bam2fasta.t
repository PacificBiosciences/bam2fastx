
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
  $ file1=$(ls -l rsii_1.fasta.gz| awk '{print $5}'); file2=$(ls -l rsii_9.fasta.gz| awk '{print $5}'); if [ "$file1" -gt "$file2" ] ; then echo "true"; else echo "false"; fi
  true