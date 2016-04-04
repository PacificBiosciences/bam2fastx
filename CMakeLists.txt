########################################################################
# CMake build script for BAM2FASTX library.
########################################################################

cmake_policy(SET CMP0048 NEW)  # lets us set version in project()
project(BAM2FASTX VERSION 1.0.0 LANGUAGES CXX C)
cmake_minimum_required(VERSION 3.2)

set(PacBioBAM_build_docs    OFF CACHE INTERNAL "" FORCE)
set(PacBioBAM_build_tests   OFF CACHE INTERNAL "" FORCE)
set(PacBioBAM_build_tools   OFF CACHE INTERNAL "" FORCE)

# Generate header with versions
configure_file(template/Version.h.in ${CMAKE_CURRENT_BINARY_DIR}/generated/Version.h)

################

# Build type
IF(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build, options are: Debug Release Profile RelWithDebInfo" FORCE)
ENDIF(NOT CMAKE_BUILD_TYPE)

# Build-time options
option(BAM2FASTX_build_bin    "Build bam2fasta and bamfastq." ON)
option(BAM2FASTX_build_tests  "Build unit tests." ON)

# Main project paths
set(BAM2FASTX_RootDir       ${BAM2FASTX_SOURCE_DIR})
set(BAM2FASTX_IncludeDir    ${BAM2FASTX_RootDir}/include)
set(BAM2FASTX_SourceDir     ${BAM2FASTX_RootDir}/src)
set(BAM2FASTX_TestsDir      ${BAM2FASTX_RootDir}/test)
set(BAM2FASTX_ThirdPartyDir ${BAM2FASTX_RootDir}/third-party)

# shared CXX flags for src & tests
set(BAM2FASTX_FLAGS " -std=c++11 -Wall -Wextra -Werror=strict-aliasing -Wno-unused-parameter")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${BAM2FASTX_FLAGS}")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${BAM2FASTX_FLAGS}")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${BAM2FASTX_FLAGS}")
SET(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -pthread")

################

#  OPTIONPARSER
add_library(OPTIONPARSER_LIB STATIC
    ${BAM2FASTX_ThirdPartyDir}/cpp-optparse/OptionParser.h
    ${BAM2FASTX_ThirdPartyDir}/cpp-optparse/OptionParser.cpp
)
target_include_directories(OPTIONPARSER_LIB PUBLIC ${BAM2FASTX_ThirdPartyDir})


#  COMMON INTERFACE
add_library(COMMON_INTERFACE INTERFACE)

################

if (NOT PacBioBAM_LIBRARIES)
    set(PBBAM_MISSING ON)

    if(NOT HTSLIB_INCLUDE_DIRS OR
       NOT HTSLIB_LIBRARIES)
        if(HTSLIB_ROOTDIR)
            find_package(htslib
                         PATHS ${HTSLIB_ROOTDIR}
                         REQUIRED)
        else()
            find_package(htslib
                         PATHS ${BAM2FASTX_ThirdPartyDir}/htslib
                         REQUIRED)
        endif()
    endif()
    add_subdirectory(${BAM2FASTX_ThirdPartyDir}/pbbam external/pbbam/build)
    target_link_libraries(COMMON_INTERFACE INTERFACE pbbam)
else()
    target_link_libraries(COMMON_INTERFACE INTERFACE ${PacBioBAM_LIBRARIES} ${HTSLIB_LIBRARIES})
endif()


################

if(NOT Boost_INCLUDE_DIRS)
    find_package(Boost REQUIRED)
endif()

if (NOT ZLIB_INCLUDE_DIRS OR
    NOT ZLIB_LIBRARIES)
    find_package(ZLIB REQUIRED)
endif()

#####

include_directories(
    ${BAM2FASTX_IncludeDir}
    ${BAM2FASTX_ThirdPartyDir}
    ${CMAKE_CURRENT_BINARY_DIR}/generated
    ${PacBioBAM_INCLUDE_DIRS}
    ${Htslib_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIRS}
    ${ZLIB_INCLUDE_DIRS}
)

#####

if(BAM2FASTX_build_tests)
    if(NOT TARGET gtest)
      add_subdirectory(${BAM2FASTX_ThirdPartyDir}/gtest/googletest external/gtest/build)
    endif()
    # add_subdirectory(test/unit)
endif()

#####

#  bam2fasta
add_executable(bam2fasta
    ${BAM2FASTX_SourceDir}/bam2fasta.cpp
)
target_link_libraries(bam2fasta
  PUBLIC
    COMMON_INTERFACE
    OPTIONPARSER_LIB
    ${ZLIB_LIBRARIES}
)

#  bam2fastq
add_executable(bam2fastq
     ${BAM2FASTX_SourceDir}/bam2fastq.cpp
)
target_link_libraries(bam2fastq
  PUBLIC
    COMMON_INTERFACE
    OPTIONPARSER_LIB
    ${ZLIB_LIBRARIES}
)

if (PBBAM_MISSING)
    add_dependencies(bam2fasta htslib)
    add_dependencies(bam2fastq htslib)
endif()

set(ALL_EXE_TARGETS bam2fasta bam2fastq)

install(TARGETS ${ALL_EXE_TARGETS} RUNTIME DESTINATION bin)