package=protobuf
$(package)_version=30.1
$(package)_download_path=https://github.com/protocolbuffers/protobuf/releases/download/v$($(package)_version)
$(package)_file_name=protobuf-$($(package)_version).tar.gz
$(package)_sha256_hash=1451b03faec83aed17cdc71671d1bbdfd72e54086b827f5f6fd02bf7a4041b68
$(package)_build_subdir=build
$(package)_dependencies=native_cmake abseil

define $(package)_set_vars
$(package)_config_opts = -DCMAKE_BUILD_TYPE=Release
$(package)_config_opts += -Dprotobuf_INSTALL=ON
$(package)_config_opts += -Dprotobuf_BUILD_PROTOBUF_BINARIES=ON
$(package)_config_opts += -Dprotobuf_BUILD_SHARED_LIBS=OFF
$(package)_config_opts += -Dprotobuf_BUILD_TESTS=OFF
$(package)_config_opts += -DCMAKE_PREFIX_PATH=$(build_prefix)/../lib/cmake/absl
$(package)_config_opts += -DCMAKE_CXX_STANDARD=17
$(package)_config_opts += -DCMAKE_CXX_STANDARD_REQUIRED=ON
$(package)_config_opts += -Dprotobuf_ABSL_PROVIDER=package
ifeq ($(build_os),darwin)
$(package)_config_opts += -DCMAKE_OSX_ARCHITECTURES=x86_64
$(package)_config_opts += -DCMAKE_C_FLAGS="-arch x86_64"
$(package)_config_opts += -DCMAKE_CXX_FLAGS="-arch x86_64 -std=c++17"
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
