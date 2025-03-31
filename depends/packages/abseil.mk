package=abseil
$(package)_version=20250127.1
$(package)_download_path=https://github.com/abseil/abseil-cpp/releases/download/$($(package)_version)
$(package)_file_name=abseil-cpp-$($(package)_version).tar.gz
$(package)_sha256_hash=b396401fd29e2e679cace77867481d388c807671dc2acc602a0259eeb79b7811
$(package)_build_subdir=build
$(package)_dependencies=native_cmake

define $(package)_set_vars
$(package)_config_opts = -DCMAKE_BUILD_TYPE=Release
$(package)_config_opts += -DABSL_ENABLE_INSTALL=ON
$(package)_config_opts += -DABSL_BUILD_TESTING=OFF
$(package)_config_opts += -DABSL_USE_GOOGLETEST_HEAD=OFF
$(package)_config_opts += -DCMAKE_POSITION_INDEPENDENT_CODE=TRUE
$(package)_config_opts += -DCMAKE_CXX_STANDARD=17
$(package)_config_opts += -DCMAKE_CXX_STANDARD_REQUIRED=ON
ifeq ($(build_os),darwin)
$(package)_config_opts += -DCMAKE_OSX_ARCHITECTURES=x86_64
$(package)_config_opts += -DCMAKE_C_FLAGS="-arch x86_64"
$(package)_config_opts += -DCMAKE_CXX_FLAGS="-arch x86_64"
endif
endef

define $(package)_preprocess_cmds
  mkdir -p $($(package)_build_subdir)
endef

define $(package)_config_cmds
  $($(package)_cmake) .. $($(package)_config_opts)
endef

define $(package)_build_cmds
  $(MAKE)
endef

define $(package)_stage_cmds
  $(MAKE) DESTDIR=$($(package)_staging_dir) install
endef