bam2fastx_cram_script = find_program('cram', required : false)
if not bam2fastx_cram_script.found()
  warning('Using bundled cram script')
  bam2fastx_cram_script = find_program('scripts/cram.py', required : true)
endif

test(
  'bam2fasta cram test',
  bam2fastx_cram_script,
  args : [
    '--xunit-file=' + join_paths(meson.build_root(), 'bam2fastx-cram-bam2fasta.xml'),
    '--verbose'] +
    files('cram/bam2fasta.t'),
  env : [
    '__PBTEST_BAM2FASTA_EXE=' + bam2fasta_exe.full_path(),
    'TESTDIR=' + join_paths(meson.current_source_dir(), 'cram')],
  timeout : 3600)

test(
  'bam2fastq cram test',
  bam2fastx_cram_script,
  args : [
    '--xunit-file=' + join_paths(meson.build_root(), 'bam2fastx-cram-bam2fastq.xml'),
    '--verbose'] +
    files('cram/bam2fastq.t'),
  env : [
    '__PBTEST_BAM2FASTQ_EXE=' + bam2fastq_exe.full_path(),
    'TESTDIR=' + join_paths(meson.current_source_dir(), 'cram')],
  timeout : 3600)
