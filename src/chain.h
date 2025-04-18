// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin Core developers
// Copyright (c) 2016-2023 The Zcash developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or https://www.opensource.org/licenses/mit-license.php .

#ifndef BITCOIN_CHAIN_H
#define BITCOIN_CHAIN_H

#include "arith_uint256.h"
#include "primitives/block.h"
#include "pow.h"
#include "tinyformat.h"
#include "uint256.h"
#include "util/strencodings.h"

#include <optional>
#include <vector>

#include <rust/metrics.h>

static const int SPROUT_VALUE_VERSION = 1001400;
static const int SAPLING_VALUE_VERSION = 1010100;
static const int CHAIN_HISTORY_ROOT_VERSION = 2010200;
static const int NU5_DATA_VERSION = 4050000;
static const int TRANSPARENT_VALUE_VERSION = 5040026;
static const int NU6_DATA_VERSION = 5100025;

/**
 * Maximum amount of time that a block timestamp is allowed to be ahead of the
 * median-time-past of the previous block.
 */
static const int64_t MAX_FUTURE_BLOCK_TIME_MTP = 90 * 60;

/**
 * Maximum amount of time that a block timestamp is allowed to be ahead of the
 * current local time.
 */
static const int64_t MAX_FUTURE_BLOCK_TIME_LOCAL = 2 * 60 * 60;

/**
 * Timestamp window used as a grace period by code that compares external
 * timestamps (such as timestamps passed to RPCs, or wallet key creation times)
 * to block timestamps.
 */
static const int64_t TIMESTAMP_WINDOW = MAX_FUTURE_BLOCK_TIME_LOCAL + 60;

static_assert(MAX_FUTURE_BLOCK_TIME_LOCAL > MAX_FUTURE_BLOCK_TIME_MTP,
              "MAX_FUTURE_BLOCK_TIME_LOCAL must be greater than MAX_FUTURE_BLOCK_TIME_MTP");
static_assert(TIMESTAMP_WINDOW > MAX_FUTURE_BLOCK_TIME_LOCAL,
              "TIMESTAMP_WINDOW must be greater than MAX_FUTURE_BLOCK_TIME_LOCAL");


class CBlockFileInfo
{
public:
    unsigned int nBlocks;      //!< number of blocks stored in file
    unsigned int nSize;        //!< number of used bytes of block file
    unsigned int nUndoSize;    //!< number of used bytes in the undo file
    unsigned int nHeightFirst; //!< lowest height of block in file
    unsigned int nHeightLast;  //!< highest height of block in file
    uint64_t nTimeFirst;       //!< earliest time of block in file
    uint64_t nTimeLast;        //!< latest time of block in file

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action) {
        READWRITE(VARINT(nBlocks));
        READWRITE(VARINT(nSize));
        READWRITE(VARINT(nUndoSize));
        READWRITE(VARINT(nHeightFirst));
        READWRITE(VARINT(nHeightLast));
        READWRITE(VARINT(nTimeFirst));
        READWRITE(VARINT(nTimeLast));
    }

     void SetNull() {
         nBlocks = 0;
         nSize = 0;
         nUndoSize = 0;
         nHeightFirst = 0;
         nHeightLast = 0;
         nTimeFirst = 0;
         nTimeLast = 0;
     }

     CBlockFileInfo() {
         SetNull();
     }

     std::string ToString() const;

     /** update statistics (does not update nSize) */
     void AddBlock(unsigned int nHeightIn, uint64_t nTimeIn) {
         if (nBlocks==0 || nHeightFirst > nHeightIn)
             nHeightFirst = nHeightIn;
         if (nBlocks==0 || nTimeFirst > nTimeIn)
             nTimeFirst = nTimeIn;
         nBlocks++;
         if (nHeightIn > nHeightLast)
             nHeightLast = nHeightIn;
         if (nTimeIn > nTimeLast)
             nTimeLast = nTimeIn;
     }
};

struct CDiskBlockPos
{
    int nFile;
    unsigned int nPos;

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action) {
        READWRITE(VARINT(nFile));
        READWRITE(VARINT(nPos));
    }

    CDiskBlockPos() {
        SetNull();
    }

    CDiskBlockPos(int nFileIn, unsigned int nPosIn) {
        nFile = nFileIn;
        nPos = nPosIn;
    }

    friend bool operator==(const CDiskBlockPos &a, const CDiskBlockPos &b) {
        return (a.nFile == b.nFile && a.nPos == b.nPos);
    }

    friend bool operator!=(const CDiskBlockPos &a, const CDiskBlockPos &b) {
        return !(a == b);
    }

    void SetNull() { nFile = -1; nPos = 0; }
    bool IsNull() const { return (nFile == -1); }

    std::string ToString() const
    {
        return strprintf("CBlockDiskPos(nFile=%i, nPos=%i)", nFile, nPos);
    }

};

enum BlockStatus: uint32_t {
    //! Unused.
    BLOCK_VALID_UNKNOWN      =    0,

    //! Parsed, version ok, hash satisfies claimed PoW, 1 <= vtx count <= max, timestamp not in future
    BLOCK_VALID_HEADER       =    1,

    //! All parent headers found, difficulty matches, timestamp >= median previous, checkpoint. Implies all parents
    //! are also at least TREE.
    BLOCK_VALID_TREE         =    2,

    /**
     * Only first tx is coinbase, 2 <= coinbase input script length <= 100, transactions valid, no duplicate txids,
     * sigops, size, merkle root. Implies all parents are at least TREE but not necessarily TRANSACTIONS. When all
     * parent blocks also have TRANSACTIONS, CBlockIndex::nChainTx will be set.
     */
    BLOCK_VALID_TRANSACTIONS =    3,

    //! Outputs do not overspend inputs, no double spends, coinbase output ok, no immature coinbase spends, BIP30.
    //! Implies all parents are also at least CHAIN.
    BLOCK_VALID_CHAIN        =    4,

    //! Scripts & signatures ok. Implies all parents are also at least SCRIPTS.
    BLOCK_VALID_SCRIPTS      =    5,

    //! All validity bits.
    BLOCK_VALID_MASK         =   BLOCK_VALID_HEADER | BLOCK_VALID_TREE | BLOCK_VALID_TRANSACTIONS |
                                 BLOCK_VALID_CHAIN | BLOCK_VALID_SCRIPTS,

    BLOCK_HAVE_DATA          =    8, //! full block available in blk*.dat
    BLOCK_HAVE_UNDO          =   16, //! undo data available in rev*.dat
    BLOCK_HAVE_MASK          =   BLOCK_HAVE_DATA | BLOCK_HAVE_UNDO,

    BLOCK_FAILED_VALID       =   32, //! stage after last reached validness failed
    BLOCK_FAILED_CHILD       =   64, //! descends from failed block
    BLOCK_FAILED_MASK        =   BLOCK_FAILED_VALID | BLOCK_FAILED_CHILD,

    BLOCK_ACTIVATES_UPGRADE  =   128, //! block activates a network upgrade
};

//! Short-hand for the highest consensus validity we implement.
//! Blocks with this validity are assumed to satisfy all consensus rules.
static const BlockStatus BLOCK_VALID_CONSENSUS = BLOCK_VALID_SCRIPTS;

/** The block chain is a tree shaped structure starting with the
 * genesis block at the root, with each block potentially having multiple
 * candidates to be the next block. A blockindex may have multiple pprev pointing
 * to it, but at most one of them can be part of the currently active branch.
 */
class CBlockIndex
{
public:
    //! pointer to the hash of the block, if any. Memory is owned by this CBlockIndex
    const uint256* phashBlock;

    //! pointer to the index of the predecessor of this block
    CBlockIndex* pprev;

    //! pointer to the index of some further predecessor of this block
    CBlockIndex* pskip;

    //! height of the entry in the chain. The genesis block has height 0
    int nHeight;

    //! Which # file this block is stored in (blk?????.dat)
    int nFile;

    //! Byte offset within blk?????.dat where this block's data is stored
    unsigned int nDataPos;

    //! Byte offset within rev?????.dat where this block's undo data is stored
    unsigned int nUndoPos;

    //! (memory only) Total amount of work (expected number of hashes) in the chain up to and including this block
    arith_uint256 nChainWork;

    //! Number of transactions in this block.
    //! Note: in a potential headers-first mode, this number cannot be relied upon
    unsigned int nTx;

    //! (memory only) Number of transactions in the chain up to and including this block.
    //! This value will be non-zero only if and only if transactions for this block and all its parents are available.
    //! Change to 64-bit type when necessary; won't happen before 2030
    unsigned int nChainTx;

    //! Verification status of this block. See enum BlockStatus
    unsigned int nStatus;

    //! Branch ID corresponding to the consensus rules used to validate this block.
    //! Only cached if block validity is BLOCK_VALID_CONSENSUS.
    //! Persisted at each activation height, memory-only for intervening blocks.
    std::optional<uint32_t> nCachedBranchId;

    //! Root of the ZIP 244 authorizing data commitment tree for this block.
    //!
    //! - For blocks prior to (not including) the NU5 activation block, this is always
    //!   null.
    //! - For blocks including and after the NU5 activation block, this is only set once
    //!   a block has been connected to the main chain, and will be null otherwise.
    uint256 hashAuthDataRoot;

    //! The anchor for the tree state up to the start of this block
    uint256 hashSproutAnchor;

    //! (memory only) The anchor for the tree state up to the end of this block
    uint256 hashFinalSproutRoot;

    //! The change to the chain supply caused by this block. This is defined as
    //! the value of the coinbase outputs (transparent and shielded) in this block,
    //! minus fees not claimed by the miner.
    //!
    //! Will be std::nullopt under the following conditions:
    //! - if the block has never been connected to a chain tip
    //! - for older blocks until a reindex has taken place
    std::optional<CAmount> nChainSupplyDelta;

    //! (memory only) Total chain supply up to and including this block.
    //!
    //! Will be std::nullopt until a reindex has taken place.
    //! Will be std::nullopt if nChainTx is zero, or if the block has never been
    //! connected to a chain tip.
    std::optional<CAmount> nChainTotalSupply;

    //! Change in value in the transparent pool produced by the action of the
    //! transparent inputs to and outputs from transactions in this block.
    //!
    //! Will be std::nullopt for older blocks until a reindex has taken place.
    std::optional<CAmount> nTransparentValue;

    //! (memory only) Total value of the transparent value pool up to and
    //! including this block.
    //!
    //! Will be std::nullopt until a reindex has taken place.
    //! Will be std::nullopt if nChainTx is zero.
    std::optional<CAmount> nChainTransparentValue;

    //! Change in value held by the Sprout circuit over this block.
    //! Will be std::nullopt for older blocks on old nodes until a reindex has taken place.
    std::optional<CAmount> nSproutValue;

    //! (memory only) Total value held by the Sprout circuit up to and including this block.
    //! Will be std::nullopt for on old nodes until a reindex has taken place.
    //! Will be std::nullopt if nChainTx is zero.
    std::optional<CAmount> nChainSproutValue;

    //! Change in value held by the Sapling circuit over this block.
    //! Not a std::optional because this was added before Sapling activated, so we can
    //! rely on the invariant that every block before this was added had nSaplingValue = 0.
    CAmount nSaplingValue;

    //! (memory only) Total value held by the Sapling circuit up to and including this block.
    //! Will be std::nullopt if nChainTx is zero.
    std::optional<CAmount> nChainSaplingValue;

    //! Change in value held by the Orchard circuit over this block.
    //! Not a std::optional because this was added before Orchard activated, so we can
    //! rely on the invariant that every block before this was added had nOrchardValue = 0.
    CAmount nOrchardValue;

    //! (memory only) Total value held by the Orchard circuit up to and including this block.
    //! Will be std::nullopt if and only if nChainTx is zero.
    std::optional<CAmount> nChainOrchardValue;

    //! Change in value held by the development fund lockbox over this block.
    //!
    //! Not a std::optional because this is added before NU6 activation, so we can
    //! rely on the invariant that every block before this was added had nLockboxValue = 0.
    CAmount nLockboxValue;

    //! (memory only) Total value held by the development fund lockbox up to
    //! and including this block. Will be std::nullopt if and only if nChainTx
    //! is zero.
    std::optional<CAmount> nChainLockboxValue;

    //! Root of the Sapling commitment tree as of the end of this block.
    //!
    //! - For blocks prior to (not including) the Heartwood activation block, this is
    //!   always equal to hashBlockCommitments.
    //! - For blocks including and after the Heartwood activation block, this is only set
    //!   once a block has been connected to the main chain, and will be null otherwise.
    uint256 hashFinalSaplingRoot;

    //! Root of the Orchard commitment tree as of the end of this block.
    //!
    //! - For blocks prior to (not including) the NU5 activation block, this is always
    //!   null.
    //! - For blocks including and after the NU5 activation block, this is only set
    //!   once a block has been connected to the main chain, and will be null otherwise.
    uint256 hashFinalOrchardRoot;

    //! Root of the ZIP 221 history tree as of the end of the previous block.
    //!
    //! - For blocks prior to and including the Heartwood activation block, this is
    //!   always null.
    //! - For blocks after (not including) the Heartwood activation block, and prior to
    //!   (not including) the NU5 activation block, this is always equal to
    //!   hashBlockCommitments.
    //! - For blocks including and after the NU5 activation block, this is only set
    //!   once a block has been connected to the main chain, and will be null otherwise.
    uint256 hashChainHistoryRoot;

    //! block header
    int nVersion;
    uint256 hashMerkleRoot;
    uint256 hashBlockCommitments;
    unsigned int nTime;
    unsigned int nBits;
    uint256 nNonce;
protected:
    // The Equihash solution, if it is stored. Once we know that the block index
    // entry is present in leveldb, this field can be cleared via the TrimSolution
    // method to save memory.
    std::vector<unsigned char> nSolution = DRIVECHAIN_EH_SOLUTION;

public:
    //! (memory only) Sequential id assigned to distinguish order in which blocks are received.
    uint32_t nSequenceId;

    void SetNull()
    {
        phashBlock = NULL;
        pprev = NULL;
        pskip = NULL;
        nHeight = 0;
        nFile = 0;
        nDataPos = 0;
        nUndoPos = 0;
        nChainWork = arith_uint256();
        nTx = 0;
        nChainTx = 0;
        nStatus = 0;
        nCachedBranchId = std::nullopt;
        hashAuthDataRoot = uint256();
        hashSproutAnchor = uint256();
        hashFinalSproutRoot = uint256();
        hashFinalSaplingRoot = uint256();
        hashFinalOrchardRoot = uint256();
        hashChainHistoryRoot = uint256();
        nSequenceId = 0;

        nChainSupplyDelta = std::nullopt;
        nChainTotalSupply = std::nullopt;
        nTransparentValue = std::nullopt;
        nChainTransparentValue = std::nullopt;
        nLockboxValue = 0;
        nChainLockboxValue = std::nullopt;

        nSproutValue = std::nullopt;
        nChainSproutValue = std::nullopt;
        nSaplingValue = 0;
        nChainSaplingValue = std::nullopt;
        nOrchardValue = 0;
        nChainOrchardValue = std::nullopt;

        nVersion       = 0;
        hashMerkleRoot = uint256();
        hashBlockCommitments = uint256();
        nTime          = 0;
        nBits          = 0;
        nNonce         = uint256();
    }

    CBlockIndex()
    {
        SetNull();
    }

    CBlockIndex(const CBlockHeader& block)
    {
        SetNull();

        nVersion       = block.nVersion;
        hashMerkleRoot = block.hashMerkleRoot;
        hashBlockCommitments = block.hashBlockCommitments;
        nTime          = block.nTime;
        nBits          = block.nBits;
        nNonce         = block.nNonce;
        nSolution      = block.nSolution;
        MetricsIncrementCounter("zcashd.debug.memory.allocated_equihash_solutions");
    }

    CDiskBlockPos GetBlockPos() const {
        CDiskBlockPos ret;
        if (nStatus & BLOCK_HAVE_DATA) {
            ret.nFile = nFile;
            ret.nPos  = nDataPos;
        }
        return ret;
    }

    CDiskBlockPos GetUndoPos() const {
        CDiskBlockPos ret;
        if (nStatus & BLOCK_HAVE_UNDO) {
            ret.nFile = nFile;
            ret.nPos  = nUndoPos;
        }
        return ret;
    }

    //! Get the block header for this block index. Requires cs_main.
    CBlockHeader GetBlockHeader() const;

    //! Clear the Equihash solution to save memory. Requires cs_main.
    void TrimSolution();

    uint256 GetBlockHash() const
    {
        assert(phashBlock);
        return *phashBlock;
    }

    int64_t GetBlockTime() const
    {
        return (int64_t)nTime;
    }

    enum { nMedianTimeSpan=11 };

    int64_t GetMedianTimePast() const
    {
        int64_t pmedian[nMedianTimeSpan];
        int64_t* pbegin = &pmedian[nMedianTimeSpan];
        int64_t* pend = &pmedian[nMedianTimeSpan];

        const CBlockIndex* pindex = this;
        for (int i = 0; i < nMedianTimeSpan && pindex; i++, pindex = pindex->pprev)
            *(--pbegin) = pindex->GetBlockTime();

        std::sort(pbegin, pend);
        return pbegin[(pend - pbegin)/2];
    }

    std::string ToString() const
    {
        return strprintf("CBlockIndex(pprev=%p, nHeight=%d, merkle=%s, hashBlock=%s, HasSolution=%s)",
            pprev, nHeight,
            hashMerkleRoot.ToString(),
            phashBlock ? GetBlockHash().ToString() : "(nil)",
            HasSolution());
    }

    //! Check whether this block index entry is valid up to the passed validity level.
    bool IsValid(enum BlockStatus nUpTo = BLOCK_VALID_TRANSACTIONS) const
    {
        assert(!(nUpTo & ~BLOCK_VALID_MASK)); // Only validity flags allowed.
        if (nStatus & BLOCK_FAILED_MASK)
            return false;
        return ((nStatus & BLOCK_VALID_MASK) >= nUpTo);
    }

    //! Is the Equihash solution stored?
    bool HasSolution() const
    {
        return true;
    }

    //! Raise the validity level of this block index entry.
    //! Returns true if the validity was changed.
    bool RaiseValidity(enum BlockStatus nUpTo)
    {
        assert(!(nUpTo & ~BLOCK_VALID_MASK)); // Only validity flags allowed.
        if (nStatus & BLOCK_FAILED_MASK)
            return false;
        if ((nStatus & BLOCK_VALID_MASK) < nUpTo) {
            nStatus = (nStatus & ~BLOCK_VALID_MASK) | nUpTo;
            return true;
        }
        return false;
    }

    //! Build the skiplist pointer for this entry.
    void BuildSkip();

    //! Efficiently find an ancestor of this block.
    CBlockIndex* GetAncestor(int height);
    const CBlockIndex* GetAncestor(int height) const;
};

/** Used to marshal pointers into hashes for db storage. */
class CDiskBlockIndex : public CBlockIndex
{
public:
    uint256 hashPrev;

    // This is the serialized `nVersion` of the block index, which is only set
    // after the (de)serialization routine is called. This should only be used
    // in LoadBlockIndexGuts (which is the only place where we read block index
    // objects from disk anyway).
    int nClientVersion = 0;

    CDiskBlockIndex() {
        hashPrev = uint256();
    }

    explicit CDiskBlockIndex(const CBlockIndex* pindex, std::function<std::vector<unsigned char>()> getSolution) : CBlockIndex(*pindex) {
        hashPrev = (pprev ? pprev->GetBlockHash() : uint256());
        if (!HasSolution()) {
            nSolution = getSolution();
        }
    }

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action) {
        int nVersion = s.GetVersion();
        if (!(s.GetType() & SER_GETHASH))
            READWRITE(VARINT(nVersion));
        nClientVersion = nVersion;

        READWRITE(VARINT(nHeight));
        READWRITE(VARINT(nStatus));
        READWRITE(VARINT(nTx));
        if (nStatus & (BLOCK_HAVE_DATA | BLOCK_HAVE_UNDO))
            READWRITE(VARINT(nFile));
        if (nStatus & BLOCK_HAVE_DATA)
            READWRITE(VARINT(nDataPos));
        if (nStatus & BLOCK_HAVE_UNDO)
            READWRITE(VARINT(nUndoPos));
        if (nStatus & BLOCK_ACTIVATES_UPGRADE) {
            if (ser_action.ForRead()) {
                uint32_t branchId;
                READWRITE(branchId);
                nCachedBranchId = branchId;
            } else {
                // nCachedBranchId must always be set if BLOCK_ACTIVATES_UPGRADE is set.
                assert(nCachedBranchId);
                uint32_t branchId = *nCachedBranchId;
                READWRITE(branchId);
            }
        }
        READWRITE(hashSproutAnchor);

        // block header
        READWRITE(this->nVersion);
        READWRITE(hashPrev);
        READWRITE(hashMerkleRoot);
        READWRITE(hashBlockCommitments);
        READWRITE(nTime);
        READWRITE(nBits);
        READWRITE(nNonce);
        READWRITE(nSolution);

        // Only read/write nTransparentValue if the client version used to create
        // this index was storing them.
        if ((s.GetType() & SER_DISK) && (nVersion >= TRANSPARENT_VALUE_VERSION)) {
            READWRITE(nChainSupplyDelta);
            READWRITE(nTransparentValue);
        }

        // Only read/write nSproutValue if the client version used to create
        // this index was storing them.
        if ((s.GetType() & SER_DISK) && (nVersion >= SPROUT_VALUE_VERSION)) {
            READWRITE(nSproutValue);
        }

        // Only read/write nSaplingValue if the client version used to create
        // this index was storing them.
        if ((s.GetType() & SER_DISK) && (nVersion >= SAPLING_VALUE_VERSION)) {
            READWRITE(nSaplingValue);
        }

        // Only read/write hashFinalSaplingRoot and hashChainHistoryRoot if the
        // client version used to create this index was storing them.
        if ((s.GetType() & SER_DISK) && (nVersion >= CHAIN_HISTORY_ROOT_VERSION)) {
            READWRITE(hashFinalSaplingRoot);
            READWRITE(hashChainHistoryRoot);
        } else if (ser_action.ForRead()) {
            // For block indices written before the client was Heartwood-aware,
            // these are always identical.
            hashFinalSaplingRoot = hashBlockCommitments;
        }

        // Only read/write NU5 data if the client version used to create this
        // index was storing them. For block indices written before the client
        // was NU5-aware, these are always null / zero.
        if ((s.GetType() & SER_DISK) && (nVersion >= NU5_DATA_VERSION)) {
            READWRITE(hashAuthDataRoot);
            READWRITE(hashFinalOrchardRoot);
            READWRITE(nOrchardValue);
        }

        // Only read/write NU6 data if the client version used to create this
        // index was storing them. For block indices written before the client
        // was NU6-aware, these are always null / zero.
        if ((s.GetType() & SER_DISK) && (nVersion >= NU6_DATA_VERSION)) {
            READWRITE(nLockboxValue);
        }

        // If you have just added new serialized fields above, remember to add
        // them to CBlockTreeDB::LoadBlockIndexGuts() in txdb.cpp :)
    }

    //! Get the block header for this block index.
    CBlockHeader GetBlockHeader() const
    {
        CBlockHeader header;
        header.nVersion             = nVersion;
        header.hashPrevBlock        = hashPrev;
        header.hashMerkleRoot       = hashMerkleRoot;
        header.hashBlockCommitments = hashBlockCommitments;
        header.nTime                = nTime;
        header.nBits                = nBits;
        header.nNonce               = nNonce;
        header.nSolution            = nSolution;
        return header;
    }

    uint256 GetBlockHash() const
    {
        return GetBlockHeader().GetHash();
    }

    std::vector<unsigned char> GetSolution() const
    {
        assert(HasSolution());
        return nSolution;
    }

    std::string ToString() const
    {
        std::string str = "CDiskBlockIndex(";
        str += CBlockIndex::ToString();
        str += strprintf("\n                hashBlock=%s, hashPrev=%s)",
            GetBlockHash().ToString(),
            hashPrev.ToString());
        return str;
    }

private:
    //! This method should not be called on a CDiskBlockIndex.
    void TrimSolution()
    {
        assert(!"called CDiskBlockIndex::TrimSolution");
    }
};

/** An in-memory indexed chain of blocks. */
class CChain {
private:
    std::vector<CBlockIndex*> vChain;

public:
    /** Returns the index entry for the genesis block of this chain, or NULL if none. */
    CBlockIndex *Genesis() const {
        return vChain.size() > 0 ? vChain[0] : NULL;
    }

    /** Returns the index entry for the tip of this chain, or NULL if none. */
    CBlockIndex *Tip() const {
        return vChain.size() > 0 ? vChain[vChain.size() - 1] : NULL;
    }

    /** Returns the index entry at a particular height in this chain, or NULL if no such height exists. */
    CBlockIndex *operator[](int nHeight) const {
        if (nHeight < 0 || nHeight >= (int)vChain.size())
            return NULL;
        return vChain[nHeight];
    }

    /** Compare two chains efficiently. */
    friend bool operator==(const CChain &a, const CChain &b) {
        return a.vChain.size() == b.vChain.size() &&
               a.vChain[a.vChain.size() - 1] == b.vChain[b.vChain.size() - 1];
    }

    /** Efficiently check whether a block is present in this chain. */
    bool Contains(const CBlockIndex *pindex) const {
        return (*this)[pindex->nHeight] == pindex;
    }

    /** Find the successor of a block in this chain, or NULL if the given index is not found or is the tip. */
    CBlockIndex *Next(const CBlockIndex *pindex) const {
        if (Contains(pindex))
            return (*this)[pindex->nHeight + 1];
        else
            return NULL;
    }

    /** Return the maximal height in the chain. Is equal to chain.Tip() ? chain.Tip()->nHeight : -1. */
    int Height() const {
        return int(vChain.size()) - 1;
    }

    /** Set/initialize a chain with a given tip. */
    void SetTip(CBlockIndex *pindex);

    /** Return a CBlockLocator that refers to a block in this chain (by default the tip). */
    CBlockLocator GetLocator(const CBlockIndex *pindex = NULL) const;

    /** Find the last common block between this chain and a block index entry. */
    const CBlockIndex *FindFork(const CBlockIndex *pindex) const;
};

#endif // BITCOIN_CHAIN_H
