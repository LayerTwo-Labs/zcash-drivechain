package=native_ccache
$(package)_version=4.10.2
$(package)_download_path=https://github.com/ccache/ccache/releases/download/v$($(package)_version)
$(package)_file_name=ccache-$($(package)_version).tar.gz
$(package)_sha256_hash=108100960bb7e64573ea925af2ee7611701241abb36ce0aae3354528403a7d87
$(package)_build_subdir=build
$(package)_dependencies=native_cmake native_fmt native_xxhash native_zstd

define $(package)_set_vars
$(package)_config_opts += -DCMAKE_BUILD_TYPE=Release
$(package)_config_opts += -DDEPS=LOCAL
$(package)_config_opts += -DCMAKE_PREFIX_PATH=$(build_prefix)
$(package)_config_opts += -DFMT_LIBRARY=$(build_prefix)/lib/libfmt.a
$(package)_config_opts += -DXXHASH_LIBRARY=$(build_prefix)/lib/libxxhash.a
$(package)_config_opts += -DZSTD_LIBRARY=$(build_prefix)/lib/libzstd.a
$(package)_config_opts += -DREDIS_STORAGE_BACKEND=OFF
$(package)_config_opts += -DENABLE_TESTING=OFF
ifeq ($(build_os),darwin)
$(package)_config_opts += -DCMAKE_OSX_ARCHITECTURES=x86_64
$(package)_config_opts += -DCMAKE_C_FLAGS="-arch x86_64"
$(package)_config_opts += -DCMAKE_CXX_FLAGS="-arch x86_64"
endif
endef

define $(package)_preprocess_cmds
  mkdir $($(package)_build_subdir)
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

define $(package)_postprocess_cmds
  rm -rf lib include
endef
