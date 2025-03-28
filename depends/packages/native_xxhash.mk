package=native_xxhash
$(package)_version=0.8.3
$(package)_download_path=https://github.com/Cyan4973/xxHash/archive/refs/tags
$(package)_download_file=v$($(package)_version).tar.gz
$(package)_file_name=xxhash-$($(package)_version).tar.gz
$(package)_sha256_hash=aae608dfe8213dfd05d909a57718ef82f30722c392344583d3f39050c7f29a80

ifeq ($(build_os),darwin)
$(package)_CFLAGS=-arch x86_64
$(package)_CXXFLAGS=-arch x86_64
endif

define $(package)_build_cmds
  $(MAKE) CFLAGS="$($(package)_CFLAGS)" CXXFLAGS="$($(package)_CXXFLAGS)" libxxhash.a
endef

define $(package)_stage_cmds
  mkdir -p $($(package)_staging_prefix_dir)/include && \
  mkdir -p $($(package)_staging_prefix_dir)/lib && \
  cp xxhash.h $($(package)_staging_prefix_dir)/include && \
  cp libxxhash.a $($(package)_staging_prefix_dir)/lib
endef
