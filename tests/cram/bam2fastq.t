
# bam2fastq tests

Test if the output is identical RSII
  $ $__PBTEST_BAM2FASTQ_EXE $TESTDIR/../data/RSII.bam -o rsii -u
  $ wc -c rsii.fastq | awk '{ print $1 }'
  44900
  $ md5sum rsii.fastq | awk '{ print $1 }'
  7e9715a5dd2d676e0c06cdd5b6a8605e
  $ rm rsii.fastq

Test if the output is identical SEQUEL CCS
  $ $__PBTEST_BAM2FASTQ_EXE $TESTDIR/../data/sequel.ccs.bam -o sequel.ccs -u
  $ wc -c sequel.ccs.fastq | awk '{ print $1 }'
  202
  $ md5sum sequel.ccs.fastq | awk '{ print $1 }'
  922a16ec5672069c83d9c471a5f722a3
  $ rm sequel.ccs.fastq

Test if compression works
  $ $__PBTEST_BAM2FASTQ_EXE $TESTDIR/../data/RSII.bam -o rsii_1 -c 1
  $ $__PBTEST_BAM2FASTQ_EXE $TESTDIR/../data/RSII.bam -o rsii_9 -c 9
  $ file1=$(wc -c < rsii_1.fastq.gz); file2=$(wc -c < rsii_9.fastq.gz); if [ "$file1" -gt "$file2" ] ; then echo "true"; else echo "false"; fi
  true
