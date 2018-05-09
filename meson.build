project(
  'bam2fastx',
  'cpp',
  version : '1.1.1',
  default_options : [
    'buildtype=release',
    'warning_level=3',
    'cpp_std=c++11',
    'b_ndebug=if-release'],
  license : 'BSD-3',
  meson_version : '>= 0.46.0')

############
# CXXFLAGS #
############

bam2fastx_warning_flags = []
cpp = meson.get_compiler('cpp')
foreach cflag: [
  '-Werror=strict-aliasing',
  '-Wno-unused-parameter']
    if cpp.has_argument(cflag)
      bam2fastx_warning_flags += cflag
    endif
endforeach

bam2fastx_incdir = include_directories('include')

################
# dependencies #
################

# boost
bam2fastx_boost_dep = dependency('boost', required : true)

# pbbam
bam2fastx_pbbam_dep = dependency('pbbam', fallback : ['pbbam', 'pbbam_dep'])

# pbcopper
bam2fastx_pbcopper_dep = dependency('pbcopper', fallback : ['pbcopper', 'pbcopper_dep'])

# zlib
bam2fastx_zlib_dep = dependency('zlib', required : true)

bam2fastx_deps = [bam2fastx_boost_dep, bam2fastx_pbbam_dep, bam2fastx_pbcopper_dep, bam2fastx_zlib_dep]

########################
# sources + executable #
########################

bam2fastx_Version_h_config = configuration_data()
bam2fastx_Version_h_config.set('PROJECT_VERSION', meson.project_version())

bam2fastx_Version_h = configure_file(
  input : files('template/Version.h.in'),
  output : 'Version.h',
  configuration : bam2fastx_Version_h_config)

bam2fasta_exe = executable(
  'bam2fasta', [
    bam2fastx_Version_h,
    files(['src/bam2fasta.cpp'])],
  install : true,
  include_directories : bam2fastx_incdir,
  dependencies : bam2fastx_deps,
  cpp_args : bam2fastx_warning_flags)

bam2fastq_exe = executable(
  'bam2fastq', [
    bam2fastx_Version_h,
    files(['src/bam2fastq.cpp'])],
  install : true,
  include_directories : bam2fastx_incdir,
  dependencies : bam2fastx_deps,
  cpp_args : bam2fastx_warning_flags)

#########
# tests #
#########

if get_option('tests')
  subdir('tests')
endif
