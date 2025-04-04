OSX_MIN_VERSION=10.14
XCODE_VERSION=11.3.1
XCODE_BUILD_ID=11C505
LD64_VERSION=530

OSX_SDK=$(SDK_PATH)/Xcode-$(XCODE_VERSION)-$(XCODE_BUILD_ID)-extracted-SDK-with-libcxx-headers

# Flag explanations:
#
#     -mlinker-version
#
#         Ensures that modern linker features are enabled. See here for more
#         details: https://github.com/bitcoin/bitcoin/pull/19407.
#
#     -B$(build_prefix)/bin
#
#         Explicitly point to our binaries (e.g. cctools) so that they are
#         ensured to be found and preferred over other possibilities.
#
#     -nostdinc++ -isystem $(OSX_SDK)/usr/include/c++/v1
#
#         Forces clang to use the libc++ headers from our SDK and completely
#         forget about the libc++ headers from the standard directories
#
#         TODO: Once we start requiring a clang version that has the
#         -stdlib++-isystem<directory> flag first introduced here:
#         https://reviews.llvm.org/D64089, we should use that instead. Read the
#         differential summary there for more details.
#
darwin_CC=clang -target $(host) -mmacosx-version-min=$(OSX_MIN_VERSION) --sysroot $(OSX_SDK) -mlinker-version=$(LD64_VERSION) -B$(build_prefix)/bin
darwin_CXX=clang++ -target $(host) -mmacosx-version-min=$(OSX_MIN_VERSION) --sysroot $(OSX_SDK) -stdlib=libc++ -mlinker-version=$(LD64_VERSION) -B$(build_prefix)/bin -nostdinc++ -isystem $(OSX_SDK)/usr/include/c++/v1

# Building for ARM (M chips) is not supported. However, ARM Macs can 
# emulate x86_64 programs very well. We therefore build for x86_64, which 
# /is/ supported. 
darwin_CFLAGS=-pipe -arch x86_64 
darwin_CXXFLAGS=$(darwin_CFLAGS) 

# Add CMake-specific flags to ensure all packages use x86_64
darwin_cmake_system=Darwin
darwin_cmake_arch_flags=-DCMAKE_OSX_ARCHITECTURES=x86_64 -DCMAKE_C_FLAGS="-arch x86_64" -DCMAKE_CXX_FLAGS="-arch x86_64"

darwin_release_CFLAGS=-O3
darwin_release_CXXFLAGS=$(darwin_release_CFLAGS)

darwin_debug_CFLAGS=-O0
darwin_debug_CXXFLAGS=$(darwin_debug_CFLAGS)

darwin_native_binutils=native_cctools
darwin_native_toolchain=native_cctools
