// Copyright (c) 2009-2020 Satoshi Nakamoto
// Copyright (c) 2009-2020 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_PRIMITIVES_TRANSACTION_H
#define BITCOIN_PRIMITIVES_TRANSACTION_H

#include "amount.h"
#include "script/script.h"
#include "serialize.h"
#include "uint256.h"
#include <metaverse/bitcoin/chain/attachment/attachment.hpp>

/** An outpoint - a combination of a transaction hash and an index n into its vout */
class COutPoint
{
public:
    uint256 hash;
    uint32_t n;

    COutPoint() { SetNull(); }
    COutPoint(uint256 hashIn, uint32_t nIn) { hash = hashIn; n = nIn; }

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion) {
        READWRITE(hash);
        READWRITE(n);
    }

    void SetNull() { hash.SetNull(); n = (uint32_t) -1; }
    bool IsNull() const { return (hash.IsNull() && n == (uint32_t) -1); }

    friend bool operator<(const COutPoint& a, const COutPoint& b)
    {
        return (a.hash < b.hash || (a.hash == b.hash && a.n < b.n));
    }

    friend bool operator==(const COutPoint& a, const COutPoint& b)
    {
        return (a.hash == b.hash && a.n == b.n);
    }

    friend bool operator!=(const COutPoint& a, const COutPoint& b)
    {
        return !(a == b);
    }

    std::string ToString() const;
};

/** An input of a transaction.  It contains the location of the previous
 * transaction's output that it claims and a signature that matches the
 * output's public key.
 */
class CTxIn
{
public:
    COutPoint prevout;
    CScript scriptSig;
    uint32_t nSequence;

    /* Setting sequence to this value for every input in a transaction
     * disables nLockTime. */
    static const uint32_t SEQUENCE_FINAL = 0xffffffff;

    CTxIn()
    {
        nSequence = SEQUENCE_FINAL;
    }

    explicit CTxIn(COutPoint prevoutIn, CScript scriptSigIn=CScript(), uint32_t nSequenceIn=SEQUENCE_FINAL);
    CTxIn(uint256 hashPrevTx, uint32_t nOut, CScript scriptSigIn=CScript(), uint32_t nSequenceIn=SEQUENCE_FINAL);

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion) {
        READWRITE(prevout);
        READWRITE(*(CScriptBase*)(&scriptSig));
        READWRITE(nSequence);
    }

    bool IsFinal() const
    {
        return (nSequence == SEQUENCE_FINAL);
    }

    friend bool operator==(const CTxIn& a, const CTxIn& b)
    {
        return (a.prevout   == b.prevout &&
                a.scriptSig == b.scriptSig &&
                a.nSequence == b.nSequence);
    }

    friend bool operator!=(const CTxIn& a, const CTxIn& b)
    {
        return !(a == b);
    }

    std::string ToString() const;
};
class CAssetDetail
{
public:
    std::string symbol;
    uint64_t maximum_supply;
    uint32_t asset_type;
    std::string issuer;
    std::string address;
    std::string description;

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion) {
        READWRITE(symbol);
        READWRITE(maximum_supply);
        READWRITE(asset_type);
        READWRITE(issuer);
        READWRITE(address);
        READWRITE(description);
    }
};
class CAssetTransfer
{
public:
    std::string address;  // symbol  -- in block
    uint64_t quantity;  // -- in block

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion) {
        READWRITE(address);
        READWRITE(quantity);
    }
};

class CAsset
{
public:
    uint32_t status;
    CAssetDetail detail;
    CAssetTransfer trans;

    size_t GetSerializeSize(int nType, int nVersion) const {
        CSizeComputer s(nType, nVersion);
        NCONST_PTR(this)->Serialize(s, nType, nVersion);
        return s.size();
    }
    template<typename Stream>
    void Serialize(Stream& s, int nType, int nVersion) const {
        //NCONST_PTR(this)->SerializationOp(s, CSerActionSerialize(), nType, nVersion);
        (::SerReadWrite(s, (status), nType, nVersion, CSerActionSerialize()));
        switch(status) {
            case 1: // asset detail
                (::SerReadWrite(s, (*(CAssetDetail*)(&detail)), nType, nVersion, CSerActionSerialize()));
                break;
            case 2: // asset transfer
                (::SerReadWrite(s, (*(CAssetTransfer*)(&trans)), nType, nVersion, CSerActionSerialize()));
                break;
        };
    }
    template<typename Stream>
    void Unserialize(Stream& s, int nType, int nVersion) {
        //SerializationOp(s, CSerActionUnserialize(), nType, nVersion);
        (::SerReadWrite(s, (status), nType, nVersion, CSerActionUnserialize()));
        switch(status) {
            case 1: // asset detail
                (::SerReadWrite(s, (*(CAssetDetail*)(&detail)), nType, nVersion, CSerActionUnserialize()));
                break;
            case 2: // asset transfer
                (::SerReadWrite(s, (*(CAssetTransfer*)(&trans)), nType, nVersion, CSerActionUnserialize()));
                break;
        };
    }
};
class CMessage
{
public:
    std::string content;

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion) {
        READWRITE(content);
    }
};

class CAssetCert
{
public:
    std::string symbol;
    std::string owner;
    std::string address;
    uint32_t type;
    uint8_t status;
    std::string content;

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion) {
        READWRITE(symbol);
        READWRITE(owner);
        READWRITE(address);
        READWRITE(type);
        READWRITE(status);

        bc::chain::asset_cert_type type_enum(type);
        if (type_enum.has_content()) {
            READWRITE(content);
        }
    }
};

class CAssetMit
{
public:
    uint8_t status;
    std::string symbol;
    std::string address;
    std::string content;

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion) {
        READWRITE(status);
        READWRITE(symbol);
        READWRITE(address);
        if (status == MIT_STATUS_REGISTER) {
            READWRITE(content);
        }
    }
};

class CDidDetail
{
public:
    std::string symbol;
    std::string address;


    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion) {
        READWRITE(symbol);
        READWRITE(address);
    }
};

class CDidTransfer
{
public:
    std::string symbol;
    std::string address;


    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion) {
        READWRITE(symbol);
        READWRITE(address);
    }
};

class CDid
{
public:
    uint32_t status;
    CDidDetail detail;
    CDidTransfer trans;

    size_t GetSerializeSize(int nType, int nVersion) const {
        CSizeComputer s(nType, nVersion);
        NCONST_PTR(this)->Serialize(s, nType, nVersion);
        return s.size();
    }
    template<typename Stream>
    void Serialize(Stream& s, int nType, int nVersion) const {
        //NCONST_PTR(this)->SerializationOp(s, CSerActionSerialize(), nType, nVersion);
        (::SerReadWrite(s, (status), nType, nVersion, CSerActionSerialize()));
        switch(status) {
            case 1: // did detail
                (::SerReadWrite(s, (*(CDidDetail*)(&detail)), nType, nVersion, CSerActionSerialize()));
                break;
            case 2: // did transfer
                (::SerReadWrite(s, (*(CDidTransfer*)(&trans)), nType, nVersion, CSerActionSerialize()));
                break;
        };
    }
    template<typename Stream>
    void Unserialize(Stream& s, int nType, int nVersion) {
        //SerializationOp(s, CSerActionUnserialize(), nType, nVersion);
        (::SerReadWrite(s, (status), nType, nVersion, CSerActionUnserialize()));
        switch(status) {
            case 1: // did detail
                (::SerReadWrite(s, (*(CDidDetail*)(&detail)), nType, nVersion, CSerActionUnserialize()));
                break;
            case 2: // did transfer
                (::SerReadWrite(s, (*(CDidTransfer*)(&trans)), nType, nVersion, CSerActionUnserialize()));
                break;
        };
    }
};
/** An output of a transaction.  It contains the public key that the next input
 * must be able to sign with to claim it.
 */
class CTxOut
{
public:
    CAmount nValue;
    CScript scriptPubKey;
    // add for attachment
    uint32_t version;
    uint32_t type;
    std::string fromdid;
    std::string todid;

    CAsset asset;
    CAssetCert assetcert;
    CAssetMit mit;
    CDid did;
    CMessage message;

    CTxOut()
    {
        SetNull();
    }

    CTxOut(const CAmount& nValueIn, CScript scriptPubKeyIn);

    //ADD_SERIALIZE_METHODS;
    size_t GetSerializeSize(int nType, int nVersion) const {
        CSizeComputer s(nType, nVersion);
        NCONST_PTR(this)->Serialize(s, nType, nVersion);
        return s.size();
    }
    template<typename Stream>
    void Serialize(Stream& s, int nType, int nVersion) const {
        //NCONST_PTR(this)->SerializationOp(s, CSerActionSerialize(), nType, nVersion);
        (::SerReadWrite(s, (nValue), nType, nVersion, CSerActionSerialize()));
        (::SerReadWrite(s, (*(CScriptBase*)(&scriptPubKey)), nType, nVersion, CSerActionSerialize()));
        (::SerReadWrite(s, (version), nType, nVersion, CSerActionSerialize()));
        (::SerReadWrite(s, (type), nType, nVersion, CSerActionSerialize()));
        if (version == DID_ATTACH_VERIFY_VERSION) {
            (::SerReadWrite(s, (fromdid), nType, nVersion, CSerActionSerialize()));
            (::SerReadWrite(s, (todid), nType, nVersion, CSerActionSerialize()));
        }

        switch(type) {
            case 0: // etp
            case 1: // etp award
                // not data left
                break;
            case 2: // asset
                (::SerReadWrite(s, (*(CAsset*)(&asset)), nType, nVersion, CSerActionSerialize()));
                break;
            case 3: // message
                (::SerReadWrite(s, (*(CMessage*)(&message)), nType, nVersion, CSerActionSerialize()));
                break;
            case 4: //did
                (::SerReadWrite(s, (*(CDid*)(&did)), nType, nVersion, CSerActionSerialize()));
                break;
            case 5: // asset cert
                (::SerReadWrite(s, (*(CAssetCert*)(&assetcert)), nType, nVersion, CSerActionSerialize()));
                break;
            case 6: // mit
                (::SerReadWrite(s, (*(CAssetMit*)(&mit)), nType, nVersion, CSerActionSerialize()));
                break;
        };
    }
    template<typename Stream>
    void Unserialize(Stream& s, int nType, int nVersion) {
        //SerializationOp(s, CSerActionUnserialize(), nType, nVersion);
        (::SerReadWrite(s, (nValue), nType, nVersion, CSerActionUnserialize()));
        (::SerReadWrite(s, (*(CScriptBase*)(&scriptPubKey)), nType, nVersion, CSerActionUnserialize()));
        (::SerReadWrite(s, (version), nType, nVersion, CSerActionUnserialize()));
        (::SerReadWrite(s, (type), nType, nVersion, CSerActionUnserialize()));
        if (version == DID_ATTACH_VERIFY_VERSION) {
            (::SerReadWrite(s, (fromdid), nType, nVersion, CSerActionUnserialize()));
            (::SerReadWrite(s, (todid), nType, nVersion, CSerActionUnserialize()));
        }
        switch(type) {
            case 0: // etp
            case 1: // etp award
                // not data left
                break;
            case 2: // asset
                (::SerReadWrite(s, (*(CAsset*)(&asset)), nType, nVersion, CSerActionUnserialize()));
                break;
            case 3: // message
                (::SerReadWrite(s, (*(CMessage*)(&message)), nType, nVersion, CSerActionUnserialize()));
                break;
            case 4: // did
                (::SerReadWrite(s, (*(CDid*)(&did)), nType, nVersion, CSerActionUnserialize()));
                break;
            case 5: // asset cert
                (::SerReadWrite(s, (*(CAssetCert*)(&assetcert)), nType, nVersion, CSerActionUnserialize()));
                break;
            case 6: // mit
                (::SerReadWrite(s, (*(CAssetMit*)(&mit)), nType, nVersion, CSerActionUnserialize()));
                break;
        };
    }

    void SetNull()
    {
        nValue = -1;
        scriptPubKey.clear();
    }

    bool IsNull() const
    {
        return (nValue == -1);
    }

    uint256 GetHash() const;

    CAmount GetDustThreshold(const CFeeRate &minRelayTxFee) const
    {
        // "Dust" is defined in terms of CTransaction::minRelayTxFee,
        // which has units satoshis-per-kilobyte.
        // If you'd pay more than 1/3 in fees
        // to spend something, then we consider it dust.
        // A typical spendable txout is 34 bytes big, and will
        // need a CTxIn of at least 148 bytes to spend:
        // so dust is a spendable txout less than
        // 546*minRelayTxFee/1000 (in satoshis)
        if (scriptPubKey.IsUnspendable())
            return 0;

        size_t nSize = GetSerializeSize(SER_DISK,0)+148u;
        return 3*minRelayTxFee.GetFee(nSize);
    }

    bool IsDust(const CFeeRate &minRelayTxFee) const
    {
        return (nValue < GetDustThreshold(minRelayTxFee));
    }

    friend bool operator==(const CTxOut& a, const CTxOut& b)
    {
        return (a.nValue       == b.nValue &&
                a.scriptPubKey == b.scriptPubKey);
    }

    friend bool operator!=(const CTxOut& a, const CTxOut& b)
    {
        return !(a == b);
    }

    std::string ToString() const;
};
#if 0 // old CTxOut
class CTxOut
{
public:
    CAmount nValue;
    CScript scriptPubKey;

    CTxOut()
    {
        SetNull();
    }

    CTxOut(const CAmount& nValueIn, CScript scriptPubKeyIn);

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion) {
        READWRITE(nValue);
        READWRITE(*(CScriptBase*)(&scriptPubKey));
    }

    void SetNull()
    {
        nValue = -1;
        scriptPubKey.clear();
    }

    bool IsNull() const
    {
        return (nValue == -1);
    }

    uint256 GetHash() const;

    CAmount GetDustThreshold(const CFeeRate &minRelayTxFee) const
    {
        // "Dust" is defined in terms of CTransaction::minRelayTxFee,
        // which has units satoshis-per-kilobyte.
        // If you'd pay more than 1/3 in fees
        // to spend something, then we consider it dust.
        // A typical spendable txout is 34 bytes big, and will
        // need a CTxIn of at least 148 bytes to spend:
        // so dust is a spendable txout less than
        // 546*minRelayTxFee/1000 (in satoshis)
        if (scriptPubKey.IsUnspendable())
            return 0;

        size_t nSize = GetSerializeSize(SER_DISK,0)+148u;
        return 3*minRelayTxFee.GetFee(nSize);
    }

    bool IsDust(const CFeeRate &minRelayTxFee) const
    {
        return (nValue < GetDustThreshold(minRelayTxFee));
    }

    friend bool operator==(const CTxOut& a, const CTxOut& b)
    {
        return (a.nValue       == b.nValue &&
                a.scriptPubKey == b.scriptPubKey);
    }

    friend bool operator!=(const CTxOut& a, const CTxOut& b)
    {
        return !(a == b);
    }

    std::string ToString() const;
};
#endif
struct CMutableTransaction;

/** The basic transaction that is broadcasted on the network and contained in
 * blocks.  A transaction can contain multiple inputs and outputs.
 */
class CTransaction
{
private:
    /** Memory only. */
    const uint256 hash;
    void UpdateHash() const;

public:
    static const int32_t CURRENT_VERSION=1;

    // The local variables are made const to prevent unintended modification
    // without updating the cached hash value. However, CTransaction is not
    // actually immutable; deserialization and assignment are implemented,
    // and bypass the constness. This is safe, as they update the entire
    // structure, including the hash.
    const int32_t nVersion;
    const std::vector<CTxIn> vin;
    const std::vector<CTxOut> vout;
    const uint32_t nLockTime;

    /** Construct a CTransaction that qualifies as IsNull() */
    CTransaction();

    /** Convert a CMutableTransaction into a CTransaction. */
    CTransaction(const CMutableTransaction &tx);

    CTransaction& operator=(const CTransaction& tx);

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion) {
        READWRITE(*const_cast<int32_t*>(&this->nVersion));
        nVersion = this->nVersion;
        READWRITE(*const_cast<std::vector<CTxIn>*>(&vin));
        READWRITE(*const_cast<std::vector<CTxOut>*>(&vout));
        READWRITE(*const_cast<uint32_t*>(&nLockTime));
        if (ser_action.ForRead())
            UpdateHash();
    }

    bool IsNull() const {
        return vin.empty() && vout.empty();
    }

    const uint256& GetHash() const {
        return hash;
    }

    // Return sum of txouts.
    CAmount GetValueOut() const;
    // GetValueIn() is a method on CCoinsViewCache, because
    // inputs must be known to compute value in.

    // Compute priority, given priority of inputs and (optionally) tx size
    double ComputePriority(double dPriorityInputs, unsigned int nTxSize=0) const;

    // Compute modified tx size for priority calculation (optionally given tx size)
    unsigned int CalculateModifiedSize(unsigned int nTxSize=0) const;

    bool IsCoinBase() const
    {
        return (vin.size() == 1 && vin[0].prevout.IsNull());
    }

    friend bool operator==(const CTransaction& a, const CTransaction& b)
    {
        return a.hash == b.hash;
    }

    friend bool operator!=(const CTransaction& a, const CTransaction& b)
    {
        return a.hash != b.hash;
    }

    std::string ToString() const;
};

/** A mutable version of CTransaction. */
struct CMutableTransaction
{
    int32_t nVersion;
    std::vector<CTxIn> vin;
    std::vector<CTxOut> vout;
    uint32_t nLockTime;

    CMutableTransaction();
    CMutableTransaction(const CTransaction& tx);

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion) {
        READWRITE(this->nVersion);
        nVersion = this->nVersion;
        READWRITE(vin);
        READWRITE(vout);
        READWRITE(nLockTime);
    }

    /** Compute the hash of this CMutableTransaction. This is computed on the
     * fly, as opposed to GetHash() in CTransaction, which uses a cached result.
     */
    uint256 GetHash() const;
};

#endif // BITCOIN_PRIMITIVES_TRANSACTION_H
