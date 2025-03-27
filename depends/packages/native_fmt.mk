package=native_fmt
$(package)_version=11.1.1
$(package)_download_path=https://github.com/fmtlib/fmt/archive/refs/tags
$(package)_download_file=$($(package)_version).tar.gz
$(package)_file_name=fmt-$($(package)_version).tar.gz
$(package)_sha256_hash=482eed9efbc98388dbaee5cb5f368be5eca4893456bb358c18b7ff71f835ae43
$(package)_build_subdir=build
$(package)_dependencies=native_cmake

define $(package)_set_vars
$(package)_config_opts += -DCMAKE_BUILD_TYPE=Release
$(package)_config_opts += -DCMAKE_POSITION_INDEPENDENT_CODE=TRUE
$(package)_config_opts += -DFMT_TEST=FALSE
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
