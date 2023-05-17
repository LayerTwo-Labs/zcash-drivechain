// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin Core developers
// Copyright (c) 2017-2023 The Zcash developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or https://www.opensource.org/licenses/mit-license.php .

#include "amount.h"
#include "consensus/consensus.h"

#include "tinyformat.h"

const std::string CURRENCY_UNIT = "ZEC";
const std::string MINOR_CURRENCY_UNIT = "zatoshis";

CFeeRate::CFeeRate(const CAmount& nFeePaid, size_t nSize)
{
    if (nSize > 0) {
        nSatoshisPerK = std::min(nFeePaid*1000/nSize, (uint64_t)INT64_MAX / MAX_BLOCK_SIZE);
    } else {
        nSatoshisPerK = 0;
    }
}

CAmount CFeeRate::GetFeeForRelay(size_t nSize) const
{
    return std::min(GetFee(nSize), LEGACY_DEFAULT_FEE);
}

CAmount CFeeRate::GetFee(size_t nSize) const
{
    CAmount nFee = nSatoshisPerK*nSize / 1000;

    if (nFee == 0 && nSatoshisPerK > 0)
        nFee = nSatoshisPerK;

    return nFee;
}

std::string CFeeRate::ToString() const
{
    return strprintf("%d.%08d %s per 1000 bytes", nSatoshisPerK / COIN, nSatoshisPerK % COIN, CURRENCY_UNIT);
}
