default_build_CC = gcc
default_build_CXX = g++
default_build_AR = ar
default_build_RANLIB = ranlib
default_build_STRIP = strip
default_build_NM = nm
default_build_OTOOL = otool
default_build_INSTALL_NAME_TOOL = install_name_tool

# Force x86_64 architecture for native packages on Darwin
ifeq ($(build_os),darwin)
  native_CFLAGS=-arch x86_64
  native_CXXFLAGS=-arch x86_64
  native_LDFLAGS=-arch x86_64
  native_CMAKE_FLAGS=-DCMAKE_OSX_ARCHITECTURES=x86_64 -DCMAKE_C_FLAGS="-arch x86_64" -DCMAKE_CXX_FLAGS="-arch x86_64"
endif

define add_build_tool_func
build_$(build_os)_$1 ?= $$(default_build_$1)
build_$(build_arch)_$(build_os)_$1 ?= $$(build_$(build_os)_$1)
build_$1=$$(build_$(build_arch)_$(build_os)_$1)
endef
$(foreach var,CC CXX AR RANLIB NM STRIP SHA256SUM DOWNLOAD OTOOL INSTALL_NAME_TOOL,$(eval $(call add_build_tool_func,$(var))))
define add_build_flags_func
build_$(build_arch)_$(build_os)_$1 += $(build_$(build_os)_$1)
build_$1=$$(build_$(build_arch)_$(build_os)_$1)
endef
$(foreach flags, CFLAGS CXXFLAGS LDFLAGS CMAKE_FLAGS, $(eval $(call add_build_flags_func,$(flags))))
