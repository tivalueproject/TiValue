// Most of this file has been ported from EncryptionUtils.cpp from BitcoinArmory:
////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//  Copyright(C) 2011-2013, Armory Technologies, Inc.                         //
//  Distributed under the GNU Affero General Public License (AGPL v3)         //
//  See LICENSE or http://www.gnu.org/licenses/agpl.html                      //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
#include <fc/crypto/sha256.hpp>
#include <fc/crypto/sha512.hpp>
#include <fc/crypto/romix.hpp>
#include <string.h>

namespace fc
{
    romix::romix( uint32_t memReqts, uint32_t numIter, std::string salt ) :
       hashOutputBytes_( 64 ),
       kdfOutputBytes_( 32 )
    {
       memoryReqtBytes_ = memReqts;
       sequenceCount_   = memoryReqtBytes_ / hashOutputBytes_;
       numIterations_   = numIter;
       salt_            = salt;
    }

    std::string romix::deriveKey_OneIter( std::string const & password )
    {
        static fc::sha512 sha512;

        // Concatenate the salt/IV to the password
        std::string saltedPassword = password + salt_;

        // Prepare the lookup table
        char *lookupTable_ = new char[memoryReqtBytes_];
        uint32_t const HSZ = hashOutputBytes_;

        // First hash to seed the lookup table, input is variable length anyway
        fc::sha512 hash = sha512.hash(saltedPassword);
        memcpy(lookupTable_, &hash, HSZ);

        // Compute <sequenceCount_> consecutive hashes of the passphrase
        // Every iteration is stored in the next 64-bytes in the Lookup table
        for( uint32_t nByte = 0; nByte < memoryReqtBytes_ - HSZ; nByte += HSZ )
        {
            // Compute hash of slot i, put result in slot i+1
            fc::sha512 hash = sha512.hash(lookupTable_ + nByte, HSZ);
            memcpy(lookupTable_ + nByte + HSZ, &hash, HSZ);
        }

        // LookupTable should be complete, now start lookup sequence.
        // Start with the last hash from the previous step
        std::string X(lookupTable_ + memoryReqtBytes_ - HSZ, HSZ);
        std::string Y(HSZ, '0');

        // We "integerize" a hash value by taking the last 4 bytes of
        // as a u_int32_t, and take modulo sequenceCount
        uint64_t* X64ptr = (uint64_t*)(X.data());
        uint64_t* Y64ptr = (uint64_t*)(Y.data());
        uint64_t* V64ptr = NULL;
        uint32_t newIndex;
        uint32_t const nXorOps = HSZ / sizeof(uint64_t);

        // Pure ROMix would use sequenceCount_ for the number of lookups.
        // We divide by 2 to reduce computation time RELATIVE to the memory usage
        // This still provides suffient LUT operations, but allows us to use more
        // memory in the same amount of time (and this is the justification for
        // the scrypt algorithm -- it is basically ROMix, modified for more
        // flexibility in controlling compute-time vs memory-usage).
        uint32_t const nLookups = sequenceCount_ / 2;
        for(uint32_t nSeq=0; nSeq<nLookups; nSeq++)
        {
            // Interpret last 4 bytes of last result (mod seqCt) as next LUT index
            newIndex = *(uint32_t*)(X.data()+HSZ-4) % sequenceCount_;

            // V represents the hash result at <newIndex>
            V64ptr = (uint64_t*)(lookupTable_ + HSZ * newIndex);

            // xor X with V, and store the result in X
            for(uint32_t i = 0; i < nXorOps; i++)
                *(Y64ptr + i) = *(X64ptr + i) ^ *(V64ptr + i);

            // Hash the xor'd data to get the next index for lookup
            fc::sha512 hash = sha512.hash(Y.data(), HSZ);
            X.assign(hash.data(), HSZ);
        }
        // Truncate the final result to get the final key
        delete lookupTable_;
        return X.substr(0, kdfOutputBytes_);
    }

    std::string romix::deriveKey( std::string const & password )
    {
        std::string masterKey(password);
        for(uint32_t i=0; i<numIterations_; i++)
            masterKey = deriveKey_OneIter(masterKey);

        return masterKey;
    }
}  // namespace fc
