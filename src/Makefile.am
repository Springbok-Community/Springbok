# Copyright (c) 2013-2016 The Bitcoin Core developers
# Copyright (c) 2014-2018 The Dash Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.

DIST_SUBDIRS = secp256k1 univalue

AM_LDFLAGS = $(LIBTOOL_LDFLAGS) $(HARDENED_LDFLAGS) $(GPROF_LDFLAGS) $(SANITIZER_LDFLAGS)
AM_CXXFLAGS = $(DEBUG_CXXFLAGS) $(HARDENED_CXXFLAGS) $(WARN_CXXFLAGS) $(NOWARN_CXXFLAGS) $(ERROR_CXXFLAGS) $(GPROF_CXXFLAGS) $(SANITIZER_CXXFLAGS)
AM_CPPFLAGS = $(DEBUG_CPPFLAGS) $(HARDENED_CPPFLAGS)
AM_LIBTOOLFLAGS = --preserve-dup-deps
PTHREAD_FLAGS = $(PTHREAD_CFLAGS) $(PTHREAD_LIBS)
EXTRA_LIBRARIES =

if ENABLE_STACKTRACES
if ENABLE_CRASH_HOOKS
if CRASH_HOOKS_WRAPPED_CXX_ABI
# Wrap internal C++ ABI's so that we can attach stacktraces to exceptions
LDFLAGS_WRAP_EXCEPTIONS = -Wl,-wrap,__cxa_allocate_exception -Wl,-wrap,__cxa_free_exception
if TARGET_WINDOWS
LDFLAGS_WRAP_EXCEPTIONS += -Wl,-wrap,_assert -Wl,-wrap,_wassert
else
LDFLAGS_WRAP_EXCEPTIONS += -Wl,-wrap,__assert_fail
endif
endif
endif

if TARGET_WINDOWS
BACKTRACE_LIB = -ldbghelp -lbacktrace
else
BACKTRACE_LIB = -lbacktrace
endif
endif #ENABLE_STACKTRACES

if EMBEDDED_UNIVALUE
LIBUNIVALUE = univalue/libunivalue.la

$(LIBUNIVALUE): $(wildcard univalue/lib/*) $(wildcard univalue/include/*)
	$(AM_V_at)$(MAKE) $(AM_MAKEFLAGS) -C $(@D) $(@F)
else
LIBUNIVALUE = $(UNIVALUE_LIBS)
endif

BITCOIN_INCLUDES=-I$(builddir) $(BDB_CPPFLAGS) $(BOOST_CPPFLAGS) $(LEVELDB_CPPFLAGS) $(CRYPTO_CFLAGS) $(SSL_CFLAGS)

BITCOIN_INCLUDES += -I$(srcdir)/secp256k1/include
BITCOIN_INCLUDES += $(UNIVALUE_CFLAGS)

BLS_LIBS=-lbls-dash -lrelic_s

LIBBITCOIN_SERVER=libdash_server.a
LIBBITCOIN_COMMON=libdash_common.a
LIBBITCOIN_CONSENSUS=libdash_consensus.a
LIBBITCOIN_CLI=libdash_cli.a
LIBBITCOIN_UTIL=libdash_util.a
LIBBITCOIN_CRYPTO_BASE=crypto/libdash_crypto_base.a
LIBBITCOINQT=qt/libdashqt.a
LIBSECP256K1=secp256k1/libsecp256k1.la

if ENABLE_ZMQ
LIBBITCOIN_ZMQ=libdash_zmq.a
endif
if BUILD_BITCOIN_LIBS
LIBBITCOINCONSENSUS=libdashconsensus.la
endif
if ENABLE_WALLET
LIBBITCOIN_WALLET=libdash_wallet.a
LIBBITCOIN_WALLET_TOOL=libdash_wallet_tool.a
endif

LIBBITCOIN_CRYPTO= $(LIBBITCOIN_CRYPTO_BASE)
if ENABLE_SSE41
LIBBITCOIN_CRYPTO_SSE41 = crypto/libdash_crypto_sse41.a
LIBBITCOIN_CRYPTO += $(LIBBITCOIN_CRYPTO_SSE41)
endif
if ENABLE_AVX2
LIBBITCOIN_CRYPTO_AVX2 = crypto/libdash_crypto_avx2.a
LIBBITCOIN_CRYPTO += $(LIBBITCOIN_CRYPTO_AVX2)
endif
if ENABLE_X86_SHANI
LIBBITCOIN_CRYPTO_X86_SHANI = crypto/libdash_crypto_x86_shani.a
LIBBITCOIN_CRYPTO += $(LIBBITCOIN_CRYPTO_X86_SHANI)
endif
if ENABLE_ARM_SHANI
LIBBITCOIN_CRYPTO_ARM_SHANI = crypto/libdash_crypto_arm_shani.a
LIBBITCOIN_CRYPTO += $(LIBBITCOIN_CRYPTO_ARM_SHANI)
endif

$(LIBSECP256K1): $(wildcard secp256k1/src/*.h) $(wildcard secp256k1/src/*.c) $(wildcard secp256k1/include/*)
	$(AM_V_at)$(MAKE) $(AM_MAKEFLAGS) -C $(@D) $(@F)

# Make is not made aware of per-object dependencies to avoid limiting building parallelization
# But to build the less dependent modules first, we manually select their order here:
EXTRA_LIBRARIES += \
  $(LIBBITCOIN_CRYPTO) \
  $(LIBBITCOIN_UTIL) \
  $(LIBBITCOIN_COMMON) \
  $(LIBBITCOIN_CONSENSUS) \
  $(LIBBITCOIN_SERVER) \
  $(LIBBITCOIN_CLI) \
  $(LIBBITCOIN_WALLET) \
  $(LIBBITCOIN_WALLET_TOOL) \
  $(LIBBITCOIN_ZMQ)

lib_LTLIBRARIES = $(LIBBITCOINCONSENSUS)

bin_PROGRAMS =
noinst_PROGRAMS =
TESTS =
BENCHMARKS =

if BUILD_BITCOIND
  bin_PROGRAMS += springbokd
endif

if BUILD_BITCOIN_CLI
  bin_PROGRAMS += springbok-cli
endif
if BUILD_BITCOIN_TX
  bin_PROGRAMS += springbok-tx
endif
if ENABLE_WALLET
if BUILD_BITCOIN_WALLET
  bin_PROGRAMS += springbok-wallet
endif
endif

.PHONY: FORCE check-symbols check-security
# springbok core #
BITCOIN_CORE_H = \
  addrdb.h \
  addressindex.h \
  spentindex.h \
  addrman.h \
  attributes.h \
  banman.h \
  base58.h \
  batchedlogger.h \
  bech32.h \
  bip39.h \
  bip39_english.h \
  blockencodings.h \
  bloom.h \
  cachemap.h \
  cachemultimap.h \
  blockfilter.h \
  chain.h \
  chainparams.h \
  chainparamsbase.h \
  chainparamsseeds.h \
  checkqueue.h \
  clientversion.h \
  coinjoin/coinjoin.h \
  coinjoin/client.h \
  coinjoin/options.h \
  coinjoin/server.h \
  coinjoin/util.h \
  coins.h \
  compat.h \
  compat/assumptions.h \
  compat/byteswap.h \
  compat/endian.h \
  compat/sanity.h \
  compressor.h \
  consensus/consensus.h \
  consensus/tx_check.h \
  consensus/tx_verify.h \
  core_io.h \
  core_memusage.h \
  cuckoocache.h \
  ctpl_stl.h \
  cxxtimer.hpp \
  evo/cbtx.h \
  evo/deterministicmns.h \
  evo/dmnstate.h \
  evo/evodb.h \
  evo/mnauth.h \
  evo/mnhftx.h \
  evo/providertx.h \
  evo/simplifiedmns.h \
  evo/specialtx.h \
  evo/specialtxman.h \
  dsnotificationinterface.h \
  governance/governance.h \
  governance/classes.h \
  governance/exceptions.h \
  governance/object.h \
  governance/validators.h \
  governance/vote.h \
  governance/votedb.h \
  flat-database.h \
  hdchain.h \
  flatfile.h \
  fs.h \
  httprpc.h \
  httpserver.h \
  index/base.h \
  index/blockfilterindex.h \
  index/disktxpos.h \
  index/txindex.h \
  indirectmap.h \
  init.h \
  interfaces/chain.h \
  interfaces/handler.h \
  interfaces/node.h \
  interfaces/wallet.h \
  key.h \
  key_io.h \
  keystore.h \
  dbwrapper.h \
  limitedmap.h \
  llmq/quorums.h \
  llmq/blockprocessor.h \
  llmq/commitment.h \
  llmq/chainlocks.h \
  llmq/clsig.h \
  llmq/debug.h \
  llmq/dkgsessionhandler.h \
  llmq/dkgsessionmgr.h \
  llmq/dkgsession.h \
  llmq/init.h \
  llmq/instantsend.h \
  llmq/snapshot.h \
  llmq/signing.h \
  llmq/signing_shares.h \
  llmq/utils.h \
  llmq/params.h \
  logging.h \
  logging/timer.h \
  mapport.h \
  masternode/node.h \
  masternode/meta.h \
  masternode/payments.h \
  masternode/sync.h \
  masternode/utils.h \
  memusage.h \
  merkleblock.h \
  messagesigner.h \
  miner.h \
  net.h \
  net_permissions.h \
  net_processing.h \
  net_types.h \
  netaddress.h \
  netbase.h \
  netfulfilledman.h \
  netmessagemaker.h \
  node/coin.h \
  node/coinstats.h \
  node/transaction.h \
  noui.h \
  optional.h \
  policy/feerate.h \
  policy/fees.h \
  policy/policy.h \
  policy/settings.h \
  pow.h \
  protocol.h \
  psbt.h \
  random.h \
  reverse_iterator.h \
  rpc/blockchain.h \
  rpc/client.h \
  rpc/mining.h \
  rpc/protocol.h \
  rpc/rawtransaction_util.h \
  rpc/register.h \
  rpc/request.h \
  rpc/server.h \
  rpc/util.h \
  saltedhasher.h \
  scheduler.h \
  script/descriptor.h \
  script/sigcache.h \
  script/sign.h \
  script/standard.h \
  shutdown.h \
  spork.h \
  stacktraces.h \
  streams.h \
  statsd_client.h \
  support/allocators/mt_pooled_secure.h \
  support/allocators/pooled_secure.h \
  support/allocators/secure.h \
  support/allocators/zeroafterfree.h \
  support/cleanse.h \
  support/events.h \
  support/lockedpool.h \
  sync.h \
  threadsafety.h \
  threadinterrupt.h \
  timedata.h \
  torcontrol.h \
  txdb.h \
  txmempool.h \
  ui_interface.h \
  undo.h \
  unordered_lru_cache.h \
  util/bip32.h \
  util/bytevectorhash.h \
  util/check.h \
  util/error.h \
  util/fees.h \
  util/spanparsing.h \
  util/system.h \
  util/asmap.h \
  util/getuniquepath.h \
  util/macros.h \
  util/memory.h \
  util/moneystr.h \
  util/ranges.h \
  util/serfloat.h \
  util/string.h \
  util/time.h \
  util/threadnames.h \
  util/translation.h \
  util/vector.h \
  util/url.h \
  util/validation.h \
  validation.h \
  validationinterface.h \
  versionbits.h \
  versionbitsinfo.h \
  walletinitinterface.h \
  wallet/bdb.h \
  wallet/coincontrol.h \
  wallet/crypter.h \
  wallet/db.h \
  wallet/fees.h \
  wallet/ismine.h \
  wallet/load.h \
  wallet/psbtwallet.h \
  wallet/rpcwallet.h \
  wallet/salvage.h \
  wallet/wallet.h \
  wallet/walletdb.h \
  wallet/wallettool.h \
  wallet/walletutil.h \
  wallet/coinselection.h \
  warnings.h \
  zmq/zmqabstractnotifier.h \
  zmq/zmqnotificationinterface.h \
  zmq/zmqpublishnotifier.h \
  zmq/zmqrpc.h \
  zmq/zmqutil.h


obj/build.h: FORCE
	@$(MKDIR_P) $(builddir)/obj
	@$(top_srcdir)/share/genbuild.sh "$(abs_top_builddir)/src/obj/build.h" \
	  "$(abs_top_srcdir)"
libdash_util_a-clientversion.$(OBJEXT): obj/build.h

# server: shared between springbokd and springbok-qt
# Contains code accessing mempool and chain state that is meant to be separated
# from wallet and gui code (see node/README.md). Shared code should go in
# libdash_common or libdash_util libraries, instead.
libdash_server_a_CPPFLAGS = $(AM_CPPFLAGS) $(BITCOIN_INCLUDES) $(MINIUPNPC_CPPFLAGS) $(NATPMP_CPPFLAGS) $(EVENT_CFLAGS) $(EVENT_PTHREADS_CFLAGS)
libdash_server_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
libdash_server_a_SOURCES = \
  addrdb.cpp \
  addrman.cpp \
  banman.cpp \
  batchedlogger.cpp \
  blockencodings.cpp \
  blockfilter.cpp \
  chain.cpp \
  coinjoin/coinjoin.cpp \
  coinjoin/options.cpp \
  coinjoin/server.cpp \
  consensus/tx_verify.cpp \
  dbwrapper.cpp \
  dsnotificationinterface.cpp \
  evo/cbtx.cpp \
  evo/deterministicmns.cpp \
  evo/dmnstate.cpp \
  evo/evodb.cpp \
  evo/mnauth.cpp \
  evo/mnhftx.cpp \
  evo/providertx.cpp \
  evo/simplifiedmns.cpp \
  evo/specialtx.cpp \
  evo/specialtxman.cpp \
  flatfile.cpp \
  httprpc.cpp \
  httpserver.cpp \
  index/base.cpp \
  index/blockfilterindex.cpp \
  index/txindex.cpp \
  interfaces/chain.cpp \
  interfaces/node.cpp \
  init.cpp \
  governance/governance.cpp \
  governance/classes.cpp \
  governance/object.cpp \
  governance/validators.cpp \
  governance/vote.cpp \
  governance/votedb.cpp \
  llmq/quorums.cpp \
  llmq/blockprocessor.cpp \
  llmq/commitment.cpp \
  llmq/chainlocks.cpp \
  llmq/clsig.cpp \
  llmq/debug.cpp \
  llmq/dkgsessionhandler.cpp \
  llmq/dkgsessionmgr.cpp \
  llmq/dkgsession.cpp \
  llmq/init.cpp \
  llmq/instantsend.cpp \
  llmq/snapshot.cpp \
  llmq/signing.cpp \
  llmq/signing_shares.cpp \
  llmq/utils.cpp \
  mapport.cpp \
  masternode/node.cpp \
  masternode/meta.cpp \
  masternode/payments.cpp \
  masternode/sync.cpp \
  masternode/utils.cpp \
  messagesigner.cpp \
  miner.cpp \
  net.cpp \
  netfulfilledman.cpp \
  net_processing.cpp \
  node/coin.cpp \
  node/coinstats.cpp \
  node/transaction.cpp \
  noui.cpp \
  policy/fees.cpp \
  policy/policy.cpp \
  policy/settings.cpp \
  pow.cpp \
  rest.cpp \
  rpc/blockchain.cpp \
  rpc/masternode.cpp \
  rpc/governance.cpp \
  rpc/mining.cpp \
  rpc/misc.cpp \
  rpc/net.cpp \
  rpc/rawtransaction.cpp \
  rpc/rpcevo.cpp \
  rpc/rpcquorums.cpp \
  rpc/server.cpp \
  rpc/coinjoin.cpp \
  script/sigcache.cpp \
  shutdown.cpp \
  spork.cpp \
  statsd_client.cpp \
  timedata.cpp \
  torcontrol.cpp \
  txdb.cpp \
  txmempool.cpp \
  ui_interface.cpp \
  validation.cpp \
  validationinterface.cpp \
  versionbits.cpp \
  $(BITCOIN_CORE_H)

if ENABLE_WALLET
libdash_server_a_SOURCES += wallet/init.cpp
endif
if !ENABLE_WALLET
libdash_server_a_SOURCES += dummywallet.cpp
endif

if ENABLE_ZMQ
libdash_zmq_a_CPPFLAGS = $(AM_CPPFLAGS) $(BITCOIN_INCLUDES) $(ZMQ_CFLAGS)
libdash_zmq_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
libdash_zmq_a_SOURCES = \
  zmq/zmqabstractnotifier.cpp \
  zmq/zmqnotificationinterface.cpp \
  zmq/zmqpublishnotifier.cpp \
  zmq/zmqrpc.cpp \
  zmq/zmqutil.cpp
endif


# wallet: shared between springbokd and springbok-qt, but only linked
# when wallet enabled
libdash_wallet_a_CPPFLAGS = $(AM_CPPFLAGS) $(BITCOIN_INCLUDES)
libdash_wallet_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
libdash_wallet_a_SOURCES = \
  coinjoin/client.cpp \
  coinjoin/options.cpp \
  coinjoin/util.cpp \
  interfaces/wallet.cpp \
  wallet/bdb.cpp \
  wallet/coincontrol.cpp \
  wallet/crypter.cpp \
  wallet/db.cpp \
  wallet/fees.cpp \
  wallet/ismine.cpp \
  wallet/load.cpp \
  wallet/psbtwallet.cpp \
  wallet/rpcdump.cpp \
  wallet/rpcwallet.cpp \
  wallet/salvage.cpp \
  wallet/wallet.cpp \
  wallet/walletdb.cpp \
  wallet/walletutil.cpp \
  wallet/coinselection.cpp \
  $(BITCOIN_CORE_H)

libdash_wallet_tool_a_CPPFLAGS = $(AM_CPPFLAGS) $(BITCOIN_INCLUDES)
libdash_wallet_tool_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
libdash_wallet_tool_a_SOURCES = \
  wallet/wallettool.cpp \
  $(BITCOIN_CORE_H)

# crypto primitives library
crypto_libdash_crypto_base_a_CPPFLAGS = $(AM_CPPFLAGS) $(PIC_FLAGS)
crypto_libdash_crypto_base_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS) $(PIC_FLAGS)
crypto_libdash_crypto_base_a_SOURCES = \
  crypto/aes.cpp \
  crypto/aes.h \
  crypto/chacha_poly_aead.h \
  crypto/chacha_poly_aead.cpp \
  crypto/chacha20.h \
  crypto/chacha20.cpp \
  crypto/common.h \
  crypto/hkdf_sha256_32.cpp \
  crypto/hkdf_sha256_32.h \
  crypto/hmac_sha256.cpp \
  crypto/hmac_sha256.h \
  crypto/hmac_sha512.cpp \
  crypto/hmac_sha512.h \
  crypto/poly1305.h \
  crypto/poly1305.cpp \
  crypto/pkcs5_pbkdf2_hmac_sha512.cpp \
  crypto/pkcs5_pbkdf2_hmac_sha512.h \
  crypto/ripemd160.cpp \
  crypto/aes_helper.c \
  crypto/ripemd160.h \
  crypto/sha1.cpp \
  crypto/sha1.h \
  crypto/sha256.cpp \
  crypto/sha256.h \
  crypto/sha3.cpp \
  crypto/sha3.h \
  crypto/sha512.cpp \
  crypto/sha512.h \
  crypto/siphash.cpp \
  crypto/siphash.h

if USE_ASM
crypto_libdash_crypto_base_a_SOURCES += crypto/sha256_sse4.cpp
endif

crypto_libdash_crypto_sse41_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
crypto_libdash_crypto_sse41_a_CPPFLAGS = $(AM_CPPFLAGS)
crypto_libdash_crypto_sse41_a_CXXFLAGS += $(SSE41_CXXFLAGS)
crypto_libdash_crypto_sse41_a_CPPFLAGS += -DENABLE_SSE41
crypto_libdash_crypto_sse41_a_SOURCES = crypto/sha256_sse41.cpp

crypto_libdash_crypto_avx2_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
crypto_libdash_crypto_avx2_a_CPPFLAGS = $(AM_CPPFLAGS)
crypto_libdash_crypto_avx2_a_CXXFLAGS += $(AVX2_CXXFLAGS)
crypto_libdash_crypto_avx2_a_CPPFLAGS += -DENABLE_AVX2
crypto_libdash_crypto_avx2_a_SOURCES = crypto/sha256_avx2.cpp

# x11
crypto_libdash_crypto_base_a_SOURCES += \
  crypto/blake.c \
  crypto/bmw.c \
  crypto/cubehash.c \
  crypto/echo.c \
  crypto/groestl.c \
  crypto/jh.c \
  crypto/keccak.c \
  crypto/luffa.c \
  crypto/shavite.c \
  crypto/simd.c \
  crypto/skein.c \
  crypto/sph_sha2.c \
  crypto/sph_sha512.c \
  crypto/sph_blake.h \
  crypto/sph_bmw.h \
  crypto/sph_cubehash.h \
  crypto/sph_echo.h \
  crypto/sph_groestl.h \
  crypto/sph_jh.h \
  crypto/sph_keccak.h \
  crypto/sph_luffa.h \
  crypto/sph_shavite.h \
  crypto/sph_simd.h \
  crypto/sph_skein.h \
  crypto/sph_sha2.h \
  cryptonote/aesb.c \
  cryptonote/c_blake256.c \
  cryptonote/c_groestl.c \
  cryptonote/c_jh.c \
  cryptonote/c_keccak.c \
  cryptonote/c_skein.c \
  cryptonote/slow-hash.c \
  cryptonote/hash-ops.c \
  cryptonote/oaes_lib.c \
  cryptonote/wild_keccak.cpp \
  cryptonote/c_blake256.h \
  cryptonote/c_groestl.h \
  cryptonote/c_jh.h \
  cryptonote/c_keccak.h \
  cryptonote/crypto.h \
  cryptonote/c_skein.h \
  cryptonote/groestl_tables.h \
  cryptonote/slow-hash.h \
  cryptonote/int-util.h \
  cryptonote/oaes_config.h \
  cryptonote/hash-ops.h \
  cryptonote/oaes_lib.h \
  cryptonote/skein_port.h \
  cryptonote/variant2_int_sqrt.h \
  cryptonote/wild_keccak.h \
  cryptonote/warnings.h \
  crypto/sph_types.h

crypto_libdash_crypto_x86_shani_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
crypto_libdash_crypto_x86_shani_a_CPPFLAGS = $(AM_CPPFLAGS)
crypto_libdash_crypto_x86_shani_a_CXXFLAGS += $(X86_SHANI_CXXFLAGS)
crypto_libdash_crypto_x86_shani_a_CPPFLAGS += -DENABLE_X86_SHANI
crypto_libdash_crypto_x86_shani_a_SOURCES = crypto/sha256_x86_shani.cpp

crypto_libdash_crypto_arm_shani_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
crypto_libdash_crypto_arm_shani_a_CPPFLAGS = $(AM_CPPFLAGS)
crypto_libdash_crypto_arm_shani_a_CXXFLAGS += $(ARM_SHANI_CXXFLAGS)
crypto_libdash_crypto_arm_shani_a_CPPFLAGS += -DENABLE_ARM_SHANI
crypto_libdash_crypto_arm_shani_a_SOURCES = crypto/sha256_arm_shani.cpp

# consensus: shared between all executables that validate any consensus rules.
libdash_consensus_a_CPPFLAGS = $(AM_CPPFLAGS) $(BITCOIN_INCLUDES)
libdash_consensus_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
libdash_consensus_a_SOURCES = \
  amount.h \
  arith_uint256.cpp \
  arith_uint256.h \
  bls/bls.cpp \
  bls/bls.h \
  consensus/merkle.cpp \
  consensus/merkle.h \
  consensus/params.h \
  consensus/tx_check.cpp \
  consensus/validation.h \
  hash_selection.h \
  hash_selection.cpp \
  hash.cpp \
  hash.h \
  prevector.h \
  primitives/block.cpp \
  primitives/block.h \
  primitives/transaction.cpp \
  primitives/transaction.h \
  pubkey.cpp \
  pubkey.h \
  script/dashconsensus.cpp \
  script/interpreter.cpp \
  script/interpreter.h \
  script/script.cpp \
  script/script.h \
  script/script_error.cpp \
  script/script_error.h \
  serialize.h \
  span.h \
  streams.h \
  tinyformat.h \
  uint256.cpp \
  uint256.h \
  util/strencodings.cpp \
  util/strencodings.h \
  version.h

# common: shared between springbokd, and springbok-qt and non-server tools
libdash_common_a_CPPFLAGS = $(AM_CPPFLAGS) $(BITCOIN_INCLUDES)
libdash_common_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
libdash_common_a_SOURCES = \
  base58.cpp \
  bech32.cpp \
  bip39.cpp \
  bloom.cpp \
  chainparams.cpp \
  coins.cpp \
  compressor.cpp \
  core_read.cpp \
  core_write.cpp \
  hdchain.cpp \
  key.cpp \
  key_io.cpp \
  keystore.cpp \
  merkleblock.cpp \
  netaddress.cpp \
  netbase.cpp \
  net_permissions.cpp \
  policy/feerate.cpp \
  policy/policy.cpp \
  psbt.cpp \
  protocol.cpp \
  rpc/rawtransaction_util.cpp \
  rpc/util.cpp \
  saltedhasher.cpp \
  scheduler.cpp \
  script/descriptor.cpp \
  script/sign.cpp \
  script/standard.cpp \
  versionbitsinfo.cpp \
  warnings.cpp \
  $(BITCOIN_CORE_H)

# util: shared between all executables.
# This library *must* be included to make sure that the glibc
# backward-compatibility objects and their sanity checks are linked.
libdash_util_a_CPPFLAGS = $(AM_CPPFLAGS) $(BITCOIN_INCLUDES)
libdash_util_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
libdash_util_a_SOURCES = \
  bls/bls_batchverifier.h \
  bls/bls_ies.cpp \
  bls/bls_ies.h \
  bls/bls_worker.cpp \
  bls/bls_worker.h \
  support/lockedpool.cpp \
  chainparamsbase.cpp \
  clientversion.cpp \
  compat/glibc_sanity_fdelt.cpp \
  compat/glibc_sanity.cpp \
  compat/glibcxx_sanity.cpp \
  compat/strnlen.cpp \
  fs.cpp \
  interfaces/handler.cpp \
  logging.cpp \
  random.cpp \
  rpc/request.cpp \
  stacktraces.cpp \
  support/cleanse.cpp \
  sync.cpp \
  threadinterrupt.cpp \
  util/bip32.cpp \
  util/bytevectorhash.cpp \
  util/error.cpp \
  util/fees.cpp \
  util/getuniquepath.cpp \
  util/system.cpp \
  util/asmap.cpp \
  util/moneystr.cpp \
  util/spanparsing.cpp \
  util/strencodings.cpp \
  util/time.cpp \
  util/serfloat.cpp \
  util/string.cpp \
  util/threadnames.cpp \
  util/url.cpp \
  util/validation.cpp \
  $(BITCOIN_CORE_H)

if GLIBC_BACK_COMPAT
libdash_util_a_SOURCES += compat/glibc_compat.cpp
AM_LDFLAGS += -Wl,--wrap=log2f -Wl,--wrap=__divmoddi4
AM_LDFLAGS += -Wl,--wrap=exp -Wl,--wrap=exp2 -Wl,--wrap=fcntl
AM_LDFLAGS += -Wl,--wrap=log -Wl,--wrap=log2 -Wl,--wrap=pow
endif

# cli: shared between springbok-cli and springbok-qt
libdash_cli_a_CPPFLAGS = $(AM_CPPFLAGS) $(BITCOIN_INCLUDES)
libdash_cli_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
libdash_cli_a_SOURCES = \
  compat/stdin.h \
  compat/stdin.cpp \
  rpc/client.cpp \
  $(BITCOIN_CORE_H)

nodist_libdash_util_a_SOURCES = $(srcdir)/obj/build.h
#

# springbokd binary #
springbokd_SOURCES = dashd.cpp
springbokd_CPPFLAGS = $(AM_CPPFLAGS) $(BITCOIN_INCLUDES)
springbokd_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
springbokd_LDFLAGS = $(LDFLAGS_WRAP_EXCEPTIONS) $(RELDFLAGS) $(AM_LDFLAGS) $(LIBTOOL_APP_LDFLAGS) $(PTHREAD_FLAGS)

if TARGET_WINDOWS
springbokd_SOURCES += dashd-res.rc
endif

# Libraries below may be listed more than once to resolve circular dependencies (see
# https://eli.thegreenplace.net/2013/07/09/library-order-in-static-linking#circular-dependency)
springbokd_LDADD = \
  $(LIBBITCOIN_SERVER) \
  $(LIBBITCOIN_WALLET) \
  $(LIBBITCOIN_SERVER) \
  $(LIBBITCOIN_COMMON) \
  $(LIBUNIVALUE) \
  $(LIBBITCOIN_UTIL) \
  $(LIBBITCOIN_ZMQ) \
  $(LIBBITCOIN_CONSENSUS) \
  $(LIBBITCOIN_CRYPTO) \
  $(LIBLEVELDB) \
  $(LIBLEVELDB_SSE42) \
  $(LIBMEMENV) \
  $(LIBSECP256K1)

springbokd_LDADD += $(BACKTRACE_LIB) $(BOOST_LIBS) $(BDB_LIBS) $(SSL_LIBS) $(CRYPTO_LIBS) $(MINIUPNPC_LIBS) $(NATPMP_LIBS) $(EVENT_PTHREADS_LIBS) $(EVENT_LIBS) $(ZMQ_LIBS) $(BLS_LIBS) $(GMP_LIBS)

# springbok-cli binary #
springbok_cli_SOURCES = dash-cli.cpp
springbok_cli_CPPFLAGS = $(AM_CPPFLAGS) $(BITCOIN_INCLUDES) $(EVENT_CFLAGS)
springbok_cli_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
springbok_cli_LDFLAGS = $(LDFLAGS_WRAP_EXCEPTIONS) $(RELDFLAGS) $(AM_LDFLAGS) $(LIBTOOL_APP_LDFLAGS) $(PTHREAD_FLAGS)

if TARGET_WINDOWS
springbok_cli_SOURCES += dash-cli-res.rc
endif

springbok_cli_LDADD = \
  $(LIBBITCOIN_CLI) \
  $(LIBUNIVALUE) \
  $(LIBBITCOIN_UTIL) \
  $(LIBBITCOIN_CRYPTO)
springbok_cli_LDADD += $(BACKTRACE_LIB) $(BOOST_LIBS) $(SSL_LIBS) $(CRYPTO_LIBS) $(EVENT_LIBS) $(BLS_LIBS) $(GMP_LIBS)
#

# springbok-tx binary #
springbok_tx_SOURCES = dash-tx.cpp
springbok_tx_CPPFLAGS = $(AM_CPPFLAGS) $(BITCOIN_INCLUDES)
springbok_tx_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
springbok_tx_LDFLAGS = $(LDFLAGS_WRAP_EXCEPTIONS) $(RELDFLAGS) $(AM_LDFLAGS) $(LIBTOOL_APP_LDFLAGS) $(PTHREAD_FLAGS)

if TARGET_WINDOWS
springbok_tx_SOURCES += dash-tx-res.rc
endif

springbok_tx_LDADD = \
  $(LIBUNIVALUE) \
  $(LIBBITCOIN_COMMON) \
  $(LIBBITCOIN_UTIL) \
  $(LIBBITCOIN_CONSENSUS) \
  $(LIBBITCOIN_CRYPTO) \
  $(LIBSECP256K1)

springbok_tx_LDADD += $(BACKTRACE_LIB) $(BOOST_LIBS) $(CRYPTO_LIBS) $(BLS_LIBS) $(GMP_LIBS)
#

# springbok-wallet binary #
springbok_wallet_SOURCES = dash-wallet.cpp
springbok_wallet_CPPFLAGS = $(AM_CPPFLAGS) $(BITCOIN_INCLUDES)
springbok_wallet_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
springbok_wallet_LDFLAGS = $(LDFLAGS_WRAP_EXCEPTIONS) $(RELDFLAGS) $(AM_LDFLAGS) $(LIBTOOL_APP_LDFLAGS)

if TARGET_WINDOWS
springbok_wallet_SOURCES += dash-wallet-res.rc
endif

# Libraries below may be listed more than once to resolve circular dependencies (see
# https://eli.thegreenplace.net/2013/07/09/library-order-in-static-linking#circular-dependency)
springbok_wallet_LDADD = \
  $(LIBBITCOIN_WALLET_TOOL) \
  $(LIBBITCOIN_SERVER) \
  $(LIBBITCOIN_WALLET) \
  $(LIBBITCOIN_SERVER) \
  $(LIBBITCOIN_COMMON) \
  $(LIBBITCOIN_CONSENSUS) \
  $(LIBBITCOIN_UTIL) \
  $(LIBBITCOIN_CRYPTO) \
  $(LIBBITCOIN_ZMQ) \
  $(LIBLEVELDB) \
  $(LIBLEVELDB_SSE42) \
  $(LIBMEMENV) \
  $(LIBSECP256K1) \
  $(LIBUNIVALUE)

springbok_wallet_LDADD += $(BACKTRACE_LIB) $(BOOST_LIBS) $(BDB_LIBS) $(CRYPTO_LIBS) $(EVENT_PTHREADS_LIBS) $(EVENT_LIBS) $(MINIUPNPC_LIBS) $(NATPMP_LIBS) $(ZMQ_LIBS) $(BLS_LIBS) $(GMP_LIBS)
#

# dashconsensus library #
if BUILD_BITCOIN_LIBS
include_HEADERS = script/dashconsensus.h
libdashconsensus_la_SOURCES = support/cleanse.cpp $(crypto_libdash_crypto_base_a_SOURCES) $(libdash_consensus_a_SOURCES)

if GLIBC_BACK_COMPAT
  libdashconsensus_la_SOURCES += compat/glibc_compat.cpp
endif

libdashconsensus_la_LDFLAGS = $(AM_LDFLAGS) -no-undefined $(RELDFLAGS) $(LIBTOOL_APP_LDFLAGS)
libdashconsensus_la_LIBADD = $(LIBSECP256K1) $(BLS_LIBS) $(GMP_LIBS)
libdashconsensus_la_CPPFLAGS = $(AM_CPPFLAGS) -I$(builddir)/obj -I$(srcdir)/secp256k1/include -DBUILD_BITCOIN_INTERNAL
libdashconsensus_la_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)

endif
#

CTAES_DIST =  crypto/ctaes/bench.c
CTAES_DIST += crypto/ctaes/ctaes.c
CTAES_DIST += crypto/ctaes/ctaes.h
CTAES_DIST += crypto/ctaes/README.md
CTAES_DIST += crypto/ctaes/test.c

CLEANFILES = $(EXTRA_LIBRARIES)

CLEANFILES += *.gcda *.gcno
CLEANFILES += compat/*.gcda compat/*.gcno
CLEANFILES += consensus/*.gcda consensus/*.gcno
CLEANFILES += crc32c/src/*.gcda crc32c/src/*.gcno
CLEANFILES += crypto/*.gcda crypto/*.gcno
CLEANFILES += index/*.gcda index/*.gcno
CLEANFILES += interfaces/*.gcda interfaces/*.gcno
CLEANFILES += node/*.gcda node/*.gcno
CLEANFILES += policy/*.gcda policy/*.gcno
CLEANFILES += primitives/*.gcda primitives/*.gcno
CLEANFILES += rpc/*.gcda rpc/*.gcno
CLEANFILES += script/*.gcda script/*.gcno
CLEANFILES += support/*.gcda support/*.gcno
CLEANFILES += univalue/*.gcda univalue/*.gcno
CLEANFILES += util/*.gcda util/*.gcno
CLEANFILES += wallet/*.gcda wallet/*.gcno
CLEANFILES += wallet/test/*.gcda wallet/test/*.gcno
CLEANFILES += zmq/*.gcda zmq/*.gcno
CLEANFILES += obj/build.h

EXTRA_DIST = $(CTAES_DIST)

config/dash-config.h: config/stamp-h1
	@$(MAKE) -C $(top_builddir) $(subdir)/$(@)
config/stamp-h1: $(top_srcdir)/$(subdir)/config/dash-config.h.in $(top_builddir)/config.status
	$(AM_V_at)$(MAKE) -C $(top_builddir) $(subdir)/$(@)
$(top_srcdir)/$(subdir)/config/dash-config.h.in:  $(am__configure_deps)
	$(AM_V_at)$(MAKE) -C $(top_srcdir) $(subdir)/config/dash-config.h.in


clean-local:
	-$(MAKE) -C secp256k1 clean
	-$(MAKE) -C univalue clean
	-rm -f leveldb/*/*.gcda leveldb/*/*.gcno leveldb/helpers/memenv/*.gcda leveldb/helpers/memenv/*.gcno
	-rm -f config.h
	-rm -rf test/__pycache__
	-rm -rf *.dSYM test/*.dSYM bench/*.dSYM qt/*.dSYM qt/test/*.dSYM

.rc.o:
	@test -f $(WINDRES)
	## FIXME: How to get the appropriate modulename_CPPFLAGS in here?
	$(AM_V_GEN) $(WINDRES) $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(CPPFLAGS) -DWINDRES_PREPROC -i $< -o $@

check-symbols: $(bin_PROGRAMS)
if GLIBC_BACK_COMPAT
	@echo "Checking glibc back compat..."
	$(AM_V_at) READELF=$(READELF) CPPFILT=$(CPPFILT) $(PYTHON) $(top_srcdir)/contrib/devtools/symbol-check.py < $(bin_PROGRAMS)
endif

check-security: $(bin_PROGRAMS)
if HARDEN
	@echo "Checking binary security..."
	$(AM_V_at) READELF=$(READELF) OBJDUMP=$(OBJDUMP) $(PYTHON) $(top_srcdir)/contrib/devtools/security-check.py < $(bin_PROGRAMS)
endif


osx_debug: $(bin_PROGRAMS)
	for i in $(bin_PROGRAMS); do mkdir -p $$i.dSYM/Contents/Resources/DWARF && $(DSYMUTIL_FLAT) -o $$i.dSYM/Contents/Resources/DWARF/$$(basename $$i) $$i &> /dev/null ; done

if ENABLE_BIP70
%.pb.cc %.pb.h: %.proto
	@test -f $(PROTOC)
	$(AM_V_GEN) $(PROTOC) --cpp_out=$(@D) --proto_path=$(<D) $<
endif

include Makefile.crc32c.include
include Makefile.leveldb.include
include Makefile.test_util.include

if ENABLE_TESTS
include Makefile.test.include
endif

if ENABLE_BENCH
include Makefile.bench.include
endif

if ENABLE_QT
include Makefile.qt.include
endif

if ENABLE_QT_TESTS
include Makefile.qttest.include
endif
