# Copyright (c) 2016-2019 The Zcash developers
# Copyright (c) 2013-2019 The Bitcoin Core developers
# Copyright (c) 2013-2019 Bitcoin Developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or https://www.opensource.org/licenses/mit-license.php .

DIST_SUBDIRS = secp256k1 univalue

AM_LDFLAGS = $(PTHREAD_CFLAGS) $(LIBTOOL_LDFLAGS) $(HARDENED_LDFLAGS) $(GPROF_LDFLAGS) $(SANITIZER_LDFLAGS)
AM_CXXFLAGS = $(DEBUG_CXXFLAGS) $(HARDENED_CXXFLAGS) $(ERROR_CXXFLAGS) $(GPROF_CXXFLAGS) $(SANITIZER_CXXFLAGS) $(WARNING_CXXFLAGS)
AM_CPPFLAGS = $(DEBUG_CPPFLAGS) $(HARDENED_CPPFLAGS)
EXTRA_LIBRARIES =

BITCOIN_CONFIG_INCLUDES=-I$(builddir)/config
BITCOIN_INCLUDES=-I$(builddir) -I$(builddir)/obj $(BDB_CPPFLAGS) $(BOOST_CPPFLAGS) $(LEVELDB_CPPFLAGS)

BITCOIN_CONFIG_INCLUDES += -I$(srcdir)/rust/include
BITCOIN_CONFIG_INCLUDES += -I$(srcdir)/rust/gen/include
BITCOIN_INCLUDES += -I$(srcdir)/rust/include
BITCOIN_INCLUDES += -I$(srcdir)/rust/gen/include
BITCOIN_INCLUDES += -I$(srcdir)/secp256k1/include
BITCOIN_INCLUDES += -I$(srcdir)/univalue/include

LIBBITCOIN_SERVER=libbitcoin_server.a
LIBBITCOIN_COMMON=libbitcoin_common.a
LIBBITCOIN_CONSENSUS=libbitcoin_consensus.a
LIBBITCOIN_CLI=libbitcoin_cli.a
LIBBITCOIN_SCRIPT=libbitcoin_script.a
LIBBITCOIN_UTIL=libbitcoin_util.a
LIBBITCOIN_CRYPTO=crypto/libbitcoin_crypto.a
LIBBITCOIN_CRYPTO_SSE41=crypto/libbitcoin_crypto_sse41.a
LIBBITCOIN_CRYPTO_AVX2=crypto/libbitcoin_crypto_avx2.a
LIBCXXBRIDGE=libcxxbridge.a
LIBRUSTZCASH=$(top_builddir)/target/$(RUST_TARGET)/release/librustzcash.la
LIBSECP256K1=secp256k1/libsecp256k1.la
LIBUNIVALUE=univalue/libunivalue.la
LIBZCASH=libzcash.a

WALLET_TOOL_BIN=zsided-wallet-tool$(EXEEXT)
WALLET_TOOL_BUILD=$(top_builddir)/target/$(RUST_TARGET)/release/zsided-wallet-tool$(EXEEXT)

if ENABLE_ZMQ
LIBBITCOIN_ZMQ=libbitcoin_zmq.a
endif
if BUILD_BITCOIN_LIBS
LIBZCASH_SCRIPT=libzcash_script.la
endif
if ENABLE_WALLET
LIBBITCOIN_WALLET=libbitcoin_wallet.a
endif

CXXBRIDGE_RS = \
  rust/src/blake2b.rs \
  rust/src/ed25519.rs \
  rust/src/equihash.rs \
  rust/src/history.rs \
  rust/src/init.rs \
  rust/src/random.rs \
  rust/src/sapling/spec.rs \
  rust/src/sapling/zip32.rs \
  rust/src/sprout.rs \
  rust/src/streams.rs \
  rust/src/bridge.rs
CXXBRIDGE_H = \
  rust/gen/include/rust/blake2b.h \
  rust/gen/include/rust/ed25519.h \
  rust/gen/include/rust/equihash.h \
  rust/gen/include/rust/history.h \
  rust/gen/include/rust/init.h \
  rust/gen/include/rust/random.h \
  rust/gen/include/rust/sapling/spec.h \
  rust/gen/include/rust/sapling/zip32.h \
  rust/gen/include/rust/sprout.h \
  rust/gen/include/rust/streams.h \
  rust/gen/include/rust/bridge.h
CXXBRIDGE_CPP = \
  rust/gen/src/blake2b.cpp \
  rust/gen/src/ed25519.cpp \
  rust/gen/src/equihash.cpp \
  rust/gen/src/history.cpp \
  rust/gen/src/init.cpp \
  rust/gen/src/random.cpp \
  rust/gen/src/sapling/spec.cpp \
  rust/gen/src/sapling/zip32.cpp \
  rust/gen/src/sprout.cpp \
  rust/gen/src/streams.cpp \
  rust/gen/src/bridge.cpp

# We add a rust/cxx.h include to indicate that we provide this (via the rustcxx depends
# package), so that cxxbridge doesn't include it within the generated headers and code.
CXXBRIDGE_OPTS = -i rust/cxx.h

$(CXXBRIDGE_RS): ;
$(CXXBRIDGE_H) $(CXXBRIDGE_CPP): $(CXXBRIDGE_RS)
	@$(MKDIR_P) $(@D)
	$(AM_V_GEN)$(CXXBRIDGE) $(CXXBRIDGE_OPTS) $(basename $(patsubst rust/gen/include/rust/%,rust/src/%,$(patsubst rust/gen/src/%,rust/src/%,$@))).rs -o $@

# We pass through CC etc. flags so they are available to Rust dependencies that internally
# compile C or C++ code with the `cc` crate.
#
# We depend on the secp256k1 crate for some logic on the Rust side of the FFI. This crate
# is a wrapper around libsecp256k1, which we already vendor in our code; the crate vendors
# its own copy with non-colliding symbols. To ensure that we only use a single version of
# libsecp256k1, we disable symbol renaming in the secp256k1-sys crate  so it links to the
# same library as the C++ code.
# - Note that this does not prevent the secp256k1-sys vendored code from being built; this
#   requires https://github.com/rust-bitcoin/rust-secp256k1/issues/380 to be addressed.
RUST_ENV_VARS = \
  RUSTC="$(RUSTC)" \
  RUSTFLAGS="--cfg=rust_secp_no_symbol_renaming -L native=$(abs_top_srcdir)/src/secp256k1/.libs" \
  CC="$(CC)" \
  CFLAGS="$(CFLAGS)" \
  CXX="$(CXX)" \
  CXXFLAGS="$(CXXFLAGS)" \
  TERM=dumb
RUST_BUILD_OPTS = --release --target $(RUST_TARGET) --manifest-path $(top_srcdir)/Cargo.toml

rust_verbose = $(rust_verbose_@AM_V@)
rust_verbose_ = $(rust_verbose_@AM_DEFAULT_V@)
rust_verbose_0 = @echo "  RUST     $@";

cargo_verbose = $(cargo_verbose_@AM_V@)
cargo_verbose_ = $(cargo_verbose_@AM_DEFAULT_V@)
cargo_verbose_1 = --verbose

if ENABLE_ONLINE_RUST
# Ensure that .cargo/config.toml (or the older .cargo/config) does not exist.
CARGO_CONFIGURED = $(top_srcdir)/.cargo/.configured-for-online
$(CARGO_CONFIGURED):
	$(AM_V_at)rm -f $(top_srcdir)/.cargo/.configured-for-offline $(top_srcdir)/.cargo/config.toml
	$(AM_V_at)rm -f $(top_srcdir)/.cargo/config
	$(AM_V_at)touch $@

else
# Enable dependency vendoring
RUST_BUILD_OPTS += --locked --offline

CARGO_CONFIGURED = $(top_srcdir)/.cargo/.configured-for-offline
$(CARGO_CONFIGURED): $(top_srcdir)/.cargo/config.toml.offline
	$(AM_V_at)rm -f $(top_srcdir)/.cargo/.configured-for-online
	$(AM_V_at)rm -f $(top_srcdir)/.cargo/config
	$(AM_V_at)cp $< $(top_srcdir)/.cargo/config.toml
	$(AM_V_at)echo "directory = \"$(RUST_VENDORED_SOURCES)\"" >>$(top_srcdir)/.cargo/config.toml
	$(AM_V_at)touch $@
endif
if ENABLE_SHANI
LIBBITCOIN_CRYPTO_SHANI = crypto/libbitcoin_crypto_shani.a
LIBBITCOIN_CRYPTO += $(LIBBITCOIN_CRYPTO_SHANI)
endif

cargo-build: $(CARGO_CONFIGURED) $(LIBSECP256K1)
	$(rust_verbose)$(RUST_ENV_VARS) $(CARGO) build $(RUST_BUILD_OPTS) $(cargo_verbose)

cargo-build-lib: cargo-build

cargo-build-bins: cargo-build

$(WALLET_TOOL_BIN): cargo-build-bins
	$(AM_V_at)cp $(WALLET_TOOL_BUILD) $@

$(LIBRUSTZCASH): cargo-build-lib
	$(AM_V_at)$(MKDIR_P) $(@D)/.libs
	$(AM_V_at)echo '# $(@F) - a libtool library file' >$@
	$(AM_V_at)echo "# Generated by src/Makefile.am for libtool" >>$@
	$(AM_V_at)echo "dlname=''" >>$@
	$(AM_V_at)echo "library_names=''" >>$@
	$(AM_V_at)echo "old_library='../$(basename $(@F)).a'" >>$@
	$(AM_V_at)echo "inherited_linker_flags=''" >>$@
	$(AM_V_at)echo "installed=no" >>$@
	$(AM_V_at)echo "shouldnotlink=no" >>$@

$(LIBSECP256K1): $(wildcard secp256k1/src/*) $(wildcard secp256k1/include/*)
	$(AM_V_at)$(MAKE) $(AM_MAKEFLAGS) -C $(@D) $(@F)

$(LIBUNIVALUE): $(wildcard univalue/lib/*) $(wildcard univalue/include/*)
	$(AM_V_at)$(MAKE) $(AM_MAKEFLAGS) -C $(@D) $(@F)

# Make is not made aware of per-object dependencies to avoid limiting building parallelization
# But to build the less dependent modules first, we manually select their order here:
EXTRA_LIBRARIES += \
  $(LIBBITCOIN_CRYPTO) \
  $(LIBBITCOIN_UTIL) \
  $(LIBBITCOIN_COMMON) \
  $(LIBBITCOIN_SCRIPT) \
  $(LIBBITCOIN_CONSENSUS) \
  $(LIBBITCOIN_SERVER) \
  $(LIBBITCOIN_CLI) \
  $(LIBBITCOIN_WALLET) \
  $(LIBBITCOIN_ZMQ) \
  $(LIBCXXBRIDGE) \
  $(LIBZCASH)

lib_LTLIBRARIES = $(LIBZCASH_SCRIPT)

bin_PROGRAMS =
noinst_PROGRAMS =
bin_SCRIPTS =
TESTS =
BENCHMARKS =

if BUILD_BITCOIND
  bin_PROGRAMS += zsided
endif

if BUILD_BITCOIN_UTILS
  bin_PROGRAMS += zside-cli zside-tx
  bin_SCRIPTS += $(WALLET_TOOL_BIN)
endif

LIBZCASH_H = \
  zcash/IncrementalMerkleTree.hpp \
  zcash/NoteEncryption.hpp \
  zcash/Address.hpp \
  zcash/address/transparent.h \
  zcash/address/mnemonic.h \
  zcash/address/orchard.hpp \
  zcash/address/sapling.hpp \
  zcash/address/sprout.hpp \
  zcash/address/unified.h \
  zcash/address/zip32.h \
  zcash/History.hpp \
  zcash/JoinSplit.hpp \
  zcash/memo.h \
  zcash/Note.hpp \
  zcash/prf.h \
  zcash/Proof.hpp \
  zcash/util.h \
  zcash/Zcash.h

.PHONY: FORCE cargo-build-lib cargo-build-bins check-symbols check-security
# bitcoin core #
BITCOIN_CORE_H = \
  addrdb.h \
  addressindex.h \
  addrman.h \
  alert.h \
  asyncrpcoperation.h \
  asyncrpcqueue.h \
  base58.h \
  bech32.h \
  bloom.h \
  chain.h \
  chainparams.h \
  chainparamsbase.h \
  chainparamsseeds.h \
  checkpoints.h \
  checkqueue.h \
  clientversion.h \
  coincontrol.h \
  coins.h \
  compat.h \
  compat/byteswap.h \
  compat/endian.h \
  compat/sanity.h \
  compressor.h \
  consensus/consensus.h \
  core_io.h \
  core_memusage.h \
  cuckoocache.h \
  deprecation.h \
  experimental_features.h \
  fs.h \
  httprpc.h \
  httpserver.h \
  init.h \
  int128.h \
  key.h \
  key_constants.h \
  key_io.h \
  keystore.h \
  dbwrapper.h \
  limitedmap.h \
  logging.h \
  drivechain.h \
  main.h \
  memusage.h \
  merkleblock.h \
  metrics.h \
  miner.h \
  net.h \
  netbase.h \
  noui.h \
  policy/policy.h \
  pow.h \
  proof_verifier.h \
  protocol.h \
  random.h \
  reverse_iterator.h \
  reverselock.h \
  rpc/client.h \
  rpc/common.h \
  rpc/protocol.h \
  rpc/server.h \
  rpc/register.h \
  scheduler.h \
  script/sigcache.h \
  script/sign.h \
  script/standard.h \
  script/ismine.h \
  spentindex.h \
  streams.h \
  support/allocators/secure.h \
  support/allocators/zeroafterfree.h \
  support/cleanse.h \
  support/events.h \
  support/lockedpool.h \
  sync.h \
  threadsafety.h \
  timedata.h \
  timestampindex.h \
  torcontrol.h \
  transaction_builder.h \
  txdb.h \
  mempool_limit.h \
  txmempool.h \
  ui_interface.h \
  uint256.h \
  uint252.h \
  undo.h \
  util/system.h \
  util/match.h \
  util/moneystr.h \
  util/string.h \
  util/test.h \
  util/time.h \
  util/vector.h \
  validationinterface.h \
  wallet/asyncrpcoperation_common.h \
  wallet/asyncrpcoperation_mergetoaddress.h \
  wallet/asyncrpcoperation_saplingmigration.h \
  wallet/asyncrpcoperation_sendmany.h \
  wallet/asyncrpcoperation_shieldcoinbase.h \
  wallet/wallet_tx_builder.h \
  wallet/crypter.h \
  wallet/db.h \
  wallet/orchard.h \
  wallet/paymentdisclosure.h \
  wallet/paymentdisclosuredb.h \
  wallet/rpcwallet.h \
  wallet/wallet.h \
  wallet/walletdb.h \
  wallet/wallet_tx_builder.h \
  warnings.h \
  weighted_map.h \
  zip317.h \
  zmq/zmqabstractnotifier.h \
  zmq/zmqconfig.h\
  zmq/zmqnotificationinterface.h \
  zmq/zmqpublishnotifier.h


obj/build.h: FORCE
	@$(MKDIR_P) $(builddir)/obj
	@$(top_srcdir)/share/genbuild.sh $(abs_top_builddir)/src/obj/build.h \
	  $(abs_top_srcdir)
libbitcoin_util_a-clientversion.$(OBJEXT): obj/build.h

# server: zcashd
libbitcoin_server_a_CPPFLAGS = $(AM_CPPFLAGS) $(BITCOIN_INCLUDES) $(EVENT_CFLAGS) $(EVENT_PTHREADS_CFLAGS)
libbitcoin_server_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
libbitcoin_server_a_SOURCES = \
  sendalert.cpp \
  addrman.cpp \
  addrdb.cpp \
  alert.cpp \
  alertkeys.h \
  asyncrpcoperation.cpp \
  asyncrpcqueue.cpp \
  bloom.cpp \
  chain.cpp \
  checkpoints.cpp \
  deprecation.cpp \
  experimental_features.cpp \
  httprpc.cpp \
  httpserver.cpp \
  init.cpp \
  dbwrapper.cpp \
  drivechain.cpp \
  main.cpp \
  merkleblock.cpp \
  metrics.cpp \
  miner.cpp \
  net.cpp \
  noui.cpp \
  policy/policy.cpp \
  pow.cpp \
  rest.cpp \
  rpc/blockchain.cpp \
  rpc/mining.cpp \
  rpc/misc.cpp \
  rpc/net.cpp \
  rpc/rawtransaction.cpp \
  rpc/server.cpp \
  script/sigcache.cpp \
  script/ismine.cpp \
  timedata.cpp \
  torcontrol.cpp \
  txdb.cpp \
  mempool_limit.cpp \
  txmempool.cpp \
  validationinterface.cpp \
  $(BITCOIN_CORE_H) \
  $(LIBZCASH_H)

libcxxbridge_a_CPPFLAGS = $(AM_CPPFLAGS) $(BITCOIN_INCLUDES) $(EVENT_CFLAGS) $(EVENT_PTHREADS_CFLAGS)
libcxxbridge_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
libcxxbridge_a_SOURCES = \
  $(CXXBRIDGE_CPP) \
  $(CXXBRIDGE_H) \
  zcash/cache.cpp \
  zcash/cache.h

if ENABLE_ZMQ
libbitcoin_zmq_a_CPPFLAGS = $(BITCOIN_INCLUDES) $(ZMQ_CFLAGS)
libbitcoin_zmq_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
libbitcoin_zmq_a_SOURCES = \
  zmq/zmqabstractnotifier.cpp \
  zmq/zmqnotificationinterface.cpp \
  zmq/zmqpublishnotifier.cpp
endif

# wallet: zcashd, but only linked when wallet enabled
libbitcoin_wallet_a_CPPFLAGS = $(AM_CPPFLAGS) $(BITCOIN_INCLUDES)
libbitcoin_wallet_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
libbitcoin_wallet_a_SOURCES = \
  zcbenchmarks.cpp \
  zcbenchmarks.h \
  wallet/asyncrpcoperation_common.cpp \
  wallet/asyncrpcoperation_mergetoaddress.cpp \
  wallet/asyncrpcoperation_saplingmigration.cpp \
  wallet/asyncrpcoperation_sendmany.cpp \
  wallet/asyncrpcoperation_shieldcoinbase.cpp \
  wallet/wallet_tx_builder.cpp \
  wallet/crypter.cpp \
  wallet/db.cpp \
  wallet/orchard.cpp \
  wallet/paymentdisclosure.cpp \
  wallet/paymentdisclosuredb.cpp \
  wallet/rpcdisclosure.cpp \
  wallet/rpcdump.cpp \
  wallet/rpcwallet.cpp \
  wallet/wallet.cpp \
  wallet/walletdb.cpp \
  $(BITCOIN_CORE_H) \
  $(LIBZCASH_H)

# crypto primitives library
crypto_libbitcoin_crypto_a_CPPFLAGS = $(AM_CPPFLAGS) $(BITCOIN_CONFIG_INCLUDES)
crypto_libbitcoin_crypto_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
crypto_libbitcoin_crypto_a_SOURCES = \
  crypto/aes.cpp \
  crypto/aes.h \
  crypto/chacha20.h \
  crypto/chacha20.cpp \
  crypto/common.h \
  crypto/equihash.cpp \
  crypto/equihash.h \
  crypto/equihash.tcc \
  crypto/hmac_sha256.cpp \
  crypto/hmac_sha256.h \
  crypto/hmac_sha512.cpp \
  crypto/hmac_sha512.h \
  crypto/ripemd160.cpp \
  crypto/ripemd160.h \
  crypto/sha1.cpp \
  crypto/sha1.h \
  crypto/sha256.cpp \
  crypto/sha256.h \
  crypto/sha512.cpp \
  crypto/sha512.h

if USE_ASM
crypto_libbitcoin_crypto_a_SOURCES += crypto/sha256_sse4.cpp
endif

if ENABLE_MINING
EQUIHASH_TROMP_SOURCES = \
  pow/tromp/equi_miner.h \
  pow/tromp/equi.h \
  pow/tromp/osx_barrier.h

crypto_libbitcoin_crypto_a_CPPFLAGS += \
  -DEQUIHASH_TROMP_ATOMIC
crypto_libbitcoin_crypto_a_SOURCES += \
  ${EQUIHASH_TROMP_SOURCES}
endif

crypto_libbitcoin_crypto_sse41_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
crypto_libbitcoin_crypto_sse41_a_CPPFLAGS = $(AM_CPPFLAGS)
if ENABLE_SSE41
crypto_libbitcoin_crypto_sse41_a_CXXFLAGS += $(SSE41_CXXFLAGS)
crypto_libbitcoin_crypto_sse41_a_CPPFLAGS += -DENABLE_SSE41
LIBBITCOIN_CRYPTO += $(LIBBITCOIN_CRYPTO_SSE41)
endif
crypto_libbitcoin_crypto_sse41_a_SOURCES = crypto/sha256_sse41.cpp

crypto_libbitcoin_crypto_avx2_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
crypto_libbitcoin_crypto_avx2_a_CPPFLAGS = $(AM_CPPFLAGS)
if ENABLE_AVX2
crypto_libbitcoin_crypto_avx2_a_CXXFLAGS += $(AVX2_CXXFLAGS)
crypto_libbitcoin_crypto_avx2_a_CPPFLAGS += -DENABLE_AVX2
LIBBITCOIN_CRYPTO += $(LIBBITCOIN_CRYPTO_AVX2)
endif
crypto_libbitcoin_crypto_avx2_a_SOURCES = crypto/sha256_avx2.cpp

crypto_libbitcoin_crypto_shani_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
crypto_libbitcoin_crypto_shani_a_CPPFLAGS = $(AM_CPPFLAGS)
crypto_libbitcoin_crypto_shani_a_CXXFLAGS += $(SHANI_CXXFLAGS)
crypto_libbitcoin_crypto_shani_a_CPPFLAGS += -DENABLE_SHANI
crypto_libbitcoin_crypto_shani_a_SOURCES = crypto/sha256_shani.cpp

# script: shared between all executables that validate any Bitcoin-style scripts.
libbitcoin_script_a_CPPFLAGS = $(AM_CPPFLAGS) $(BITCOIN_INCLUDES)
libbitcoin_script_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
libbitcoin_script_a_SOURCES = \
  amount.cpp \
  amount.h \
  hash.cpp \
  hash.h \
  prevector.h \
  primitives/orchard.h \
  primitives/sapling.h \
  primitives/transaction.cpp \
  primitives/transaction.h \
  pubkey.cpp \
  pubkey.h \
  script/interpreter.cpp \
  script/interpreter.h \
  script/script.cpp \
  script/script.h \
  script/script_error.cpp \
  script/script_error.h \
  script/zcash_script.cpp \
  serialize.h \
  streams_rust.cpp \
  streams_rust.h \
  tinyformat.h \
  util/strencodings.cpp \
  util/strencodings.h \
  version.h \
  zip317.cpp

# consensus: shared between all executables that validate any consensus rules.
libbitcoin_consensus_a_CPPFLAGS = $(AM_CPPFLAGS) $(BITCOIN_INCLUDES)
libbitcoin_consensus_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
libbitcoin_consensus_a_SOURCES = \
  arith_uint256.cpp \
  arith_uint256.h \
  consensus/funding.cpp \
  consensus/funding.h \
  consensus/merkle.cpp \
  consensus/merkle.h \
  consensus/params.cpp \
  consensus/params.h \
  consensus/upgrades.cpp \
  consensus/upgrades.h \
  consensus/validation.h \
  primitives/block.cpp \
  primitives/block.h

# common: shared between zcashd and non-server tools
libbitcoin_common_a_CPPFLAGS = $(AM_CPPFLAGS) $(BITCOIN_INCLUDES)
libbitcoin_common_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
libbitcoin_common_a_SOURCES = \
  base58.cpp \
  bech32.cpp \
  chainparams.cpp \
  coins.cpp \
  compressor.cpp \
  core_read.cpp \
  core_write.cpp \
  key.cpp \
  key_io.cpp \
  keystore.cpp \
  netbase.cpp \
  primitives/tx_version_info.cpp \
  proof_verifier.cpp \
  protocol.cpp \
  scheduler.cpp \
  script/sign.cpp \
  script/standard.cpp \
  transaction_builder.cpp \
  util/test.cpp \
  warnings.cpp \
  $(BITCOIN_CORE_H) \
  $(LIBZCASH_H)

# util: shared between all executables.
# This library *must* be included to make sure that the glibc
# backward-compatibility objects and their sanity checks are linked.
libbitcoin_util_a_CPPFLAGS = $(AM_CPPFLAGS) $(BITCOIN_INCLUDES)
libbitcoin_util_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
libbitcoin_util_a_SOURCES = \
  support/lockedpool.cpp \
  chainparamsbase.cpp \
  clientversion.cpp \
  compat/glibc_sanity.cpp \
  compat/glibcxx_sanity.cpp \
  compat/strnlen.cpp \
  fs.cpp \
  logging.cpp \
  random.cpp \
  rpc/protocol.cpp \
  support/cleanse.cpp \
  sync.cpp \
  uint256.cpp \
  util/system.cpp \
  util/moneystr.cpp \
  util/strencodings.cpp \
  util/time.cpp \
  $(BITCOIN_CORE_H) \
  $(LIBZCASH_H)

if GLIBC_BACK_COMPAT
libbitcoin_util_a_SOURCES += compat/glibc_compat.cpp
endif

# cli: zcash-cli
libbitcoin_cli_a_CPPFLAGS = $(AM_CPPFLAGS) $(BITCOIN_INCLUDES)
libbitcoin_cli_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
libbitcoin_cli_a_SOURCES = \
  rpc/client.cpp \
  $(BITCOIN_CORE_H) \
  $(LIBZCASH_H)

nodist_libbitcoin_util_a_SOURCES = $(srcdir)/obj/build.h
#

# bitcoind binary #
zsided_SOURCES = bitcoind.cpp
zsided_CPPFLAGS = $(AM_CPPFLAGS) $(BITCOIN_INCLUDES)
zsided_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
zsided_LDFLAGS = $(RELDFLAGS) $(AM_LDFLAGS) $(LIBTOOL_APP_LDFLAGS)

if TARGET_WINDOWS
zsided_SOURCES += bitcoind-res.rc
endif

zsided_LDADD = \
  $(LIBBITCOIN_SERVER) \
  $(LIBBITCOIN_WALLET) \
  $(LIBBITCOIN_COMMON) \
  $(LIBCXXBRIDGE) \
  $(LIBUNIVALUE) \
  $(LIBBITCOIN_UTIL) \
  $(LIBBITCOIN_ZMQ) \
  $(LIBBITCOIN_CONSENSUS) \
  $(LIBBITCOIN_SCRIPT) \
  $(LIBBITCOIN_CRYPTO) \
  $(LIBRUSTZCASH) \
  $(LIBZCASH) \
  $(LIBLEVELDB) \
  $(LIBLEVELDB_SSE42) \
  $(LIBMEMENV) \
  $(LIBSECP256K1)

zsided_LDADD += \
  $(BOOST_LIBS) \
  $(BDB_LIBS) \
  $(EVENT_PTHREADS_LIBS) \
  $(EVENT_LIBS) \
  $(ZMQ_LIBS) \
  $(LIBZCASH_LIBS)

# bitcoin-cli binary #
zside_cli_SOURCES = bitcoin-cli.cpp
zside_cli_CPPFLAGS = $(AM_CPPFLAGS) $(BITCOIN_INCLUDES) $(EVENT_CFLAGS)
zside_cli_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
zside_cli_LDFLAGS = $(RELDFLAGS) $(AM_LDFLAGS) $(LIBTOOL_APP_LDFLAGS)

if TARGET_WINDOWS
zside_cli_SOURCES += bitcoin-cli-res.rc
endif

zside_cli_LDADD = \
  $(LIBBITCOIN_CLI) \
  $(LIBUNIVALUE) \
  $(LIBBITCOIN_UTIL) \
  $(LIBCXXBRIDGE) \
  $(BOOST_LIBS) \
  $(EVENT_LIBS) \
  $(LIBRUSTZCASH) \
  $(LIBZCASH) \
  $(LIBBITCOIN_CRYPTO) \
  $(LIBSECP256K1) \
  $(LIBZCASH_LIBS)
#

# zside-tx binary #
zside_tx_SOURCES = bitcoin-tx.cpp
zside_tx_CPPFLAGS = $(AM_CPPFLAGS) $(BITCOIN_INCLUDES)
zside_tx_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
zside_tx_LDFLAGS = $(RELDFLAGS) $(AM_LDFLAGS) $(LIBTOOL_APP_LDFLAGS)

if TARGET_WINDOWS
zside_tx_SOURCES += bitcoin-tx-res.rc
endif

# FIXME: Is libzcash needed for zcash_tx?
zside_tx_LDADD = \
  $(LIBUNIVALUE) \
  $(LIBBITCOIN_COMMON) \
  $(LIBBITCOIN_UTIL) \
  $(LIBBITCOIN_CONSENSUS) \
  $(LIBBITCOIN_SCRIPT) \
  $(LIBCXXBRIDGE) \
  $(LIBSECP256K1) \
  $(LIBRUSTZCASH) \
  $(LIBZCASH) \
  $(LIBBITCOIN_CRYPTO) \
  $(LIBZCASH_LIBS)

zside_tx_LDADD += $(BOOST_LIBS)
#

# zcash protocol primitives #
libzcash_a_SOURCES = \
  zcash/IncrementalMerkleTree.cpp \
  zcash/NoteEncryption.cpp \
  zcash/Address.cpp \
  zcash/address/transparent.cpp \
  zcash/address/mnemonic.cpp \
  zcash/address/orchard.cpp \
  zcash/address/sapling.cpp \
  zcash/address/sprout.cpp \
  zcash/address/unified.cpp \
  zcash/address/zip32.cpp \
  zcash/History.cpp \
  zcash/JoinSplit.cpp \
  zcash/memo.cpp \
  zcash/Note.cpp \
  zcash/prf.cpp \
  zcash/util.cpp

libzcash_a_CPPFLAGS = $(AM_CPPFLAGS) $(PIC_FLAGS) $(BITCOIN_INCLUDES)
libzcash_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)

# zcash_script library #
if BUILD_BITCOIN_LIBS
include_HEADERS = script/zcash_script.h
libzcash_script_la_SOURCES = \
  $(crypto_libbitcoin_crypto_a_SOURCES) \
  $(libbitcoin_script_a_SOURCES) \
  $(libcxxbridge_a_SOURCES) \
  support/cleanse.cpp \
  uint256.cpp

if GLIBC_BACK_COMPAT
  libzcash_script_la_SOURCES += compat/glibc_compat.cpp
endif

libzcash_script_la_LDFLAGS = $(AM_LDFLAGS) -no-undefined $(RELDFLAGS)
libzcash_script_la_LIBADD = $(LIBRUSTZCASH) $(LIBSECP256K1)
libzcash_script_la_CPPFLAGS = $(AM_CPPFLAGS) -I$(builddir)/obj -I$(srcdir)/rust/include -I$(srcdir)/rust/gen/include -I$(srcdir)/secp256k1/include -DBUILD_BITCOIN_INTERNAL
libzcash_script_la_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)

endif
#

CTAES_DIST =  crypto/ctaes/bench.c
CTAES_DIST += crypto/ctaes/ctaes.c
CTAES_DIST += crypto/ctaes/ctaes.h
CTAES_DIST += crypto/ctaes/README.md
CTAES_DIST += crypto/ctaes/test.c

BUILT_SOURCES = $(CXXBRIDGE_H)

CLEANFILES = *.gcda *.gcno */*.gcno wallet/*/*.gcno $(bin_SCRIPTS)

DISTCLEANFILES = obj/build.h

EXTRA_DIST = $(CTAES_DIST) rust

clean-local:
	rm -f $(top_srcdir)/.cargo/config.toml $(top_srcdir)/.cargo/config $(top_srcdir)/.cargo/.configured-for-*
	-$(MAKE) -C secp256k1 clean
	-$(MAKE) -C univalue clean
	rm -f leveldb/*/*.gcno leveldb/helpers/memenv/*.gcno
	rm -rf rust/gen
	rm -f fuzz.cpp
	rm -rf fuzzing/*/output
	-rm -f config.h
	-rm -f *.lib bench/*.lib test/*.lib

.rc.o:
	@test -f $(WINDRES)
	$(AM_V_GEN) $(WINDRES) -DWINDRES_PREPROC -i $< -o $@

check-symbols: $(bin_PROGRAMS) $(bin_SCRIPTS)
if GLIBC_BACK_COMPAT
	@echo "Checking glibc back compat of [$(bin_PROGRAMS) $(bin_SCRIPTS)]..."
	$(AM_V_at) READELF=$(READELF) CPPFILT=$(CPPFILT) $(top_srcdir)/contrib/devtools/symbol-check.py $(bin_PROGRAMS) $(bin_SCRIPTS)
endif

check-security: $(bin_PROGRAMS) $(bin_SCRIPTS)
if HARDEN
	@echo "Checking binary security of [$(bin_PROGRAMS) $(bin_SCRIPTS)]..."
	$(AM_V_at) READELF=$(READELF) OBJDUMP=$(OBJDUMP) $(top_srcdir)/contrib/devtools/security-check.py $(bin_PROGRAMS)
	$(AM_V_at) READELF=$(READELF) OBJDUMP=$(OBJDUMP) $(top_srcdir)/contrib/devtools/security-check.py --allow-no-canary $(bin_SCRIPTS)
endif

%.pb.cc %.pb.h: %.proto
	@test -f $(PROTOC)
	$(AM_V_GEN) $(PROTOC) --cpp_out=$(@D) --proto_path=$(abspath $(<D) $<)

if EMBEDDED_LEVELDB
include Makefile.crc32c.include
include Makefile.leveldb.include
endif

if ENABLE_TESTS
include Makefile.test.include
include Makefile.gtest.include
endif

if ENABLE_BENCH
include Makefile.bench.include
endif
