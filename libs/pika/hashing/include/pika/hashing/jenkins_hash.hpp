//  Copyright (c) 2005-2007 Hartmut Kaiser
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <pika/config.hpp>

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <random>
#include <string>
#include <utility>

///////////////////////////////////////////////////////////////////////////////
namespace pika { namespace util {
    namespace detail {
        // ------------------------------------------------------------------------
        // mix -- mix 3 32-bit values reversibly.
        //
        // This is reversible, so any information in (a,b,c) before mix() is
        // still in (a,b,c) after mix().
        //
        // If four pairs of (a,b,c) inputs are run through mix(), or through
        // mix() in reverse, there are at least 32 bits of the output that
        // are sometimes the same for one pair and different for another pair.
        // This was tested for:
        // * pairs that differed by one bit, by two bits, in any combination
        //   of top bits of (a,b,c), or in any combination of bottom bits of
        //   (a,b,c).
        // * "differ" is defined as +, -, ^, or ~^.  For + and -, I transformed
        //   the output delta to a Gray code (a^(a>>1)) so a string of 1's (as
        //   is commonly produced by subtraction) look like a single 1-bit
        //   difference.
        // * the base values were pseudorandom, all zero but one bit set, or
        //   all zero plus a counter that starts at zero.
        //
        // Some k values for my "a-=c; a^=rot(c,k); c+=b;" arrangement that
        // satisfy this are
        //     4  6  8 16 19  4
        //     9 15  3 18 27 15
        //    14  9  3  7 17  3
        // Well, "9 15 3 18 27 15" didn't quite get 32 bits diffing
        // for "differ" defined as + with a one-bit base and a two-bit delta.  I
        // used http://burtleburtle.net/bob/hash/avalanche.html to choose
        // the operations, constants, and arrangements of the variables.
        //
        // This does not achieve avalanche.  There are input bits of (a,b,c)
        // that fail to affect some output bits of (a,b,c), especially of a.  The
        // most thoroughly mixed value is c, but it doesn't really even achieve
        // avalanche in c.
        //
        // This allows some parallelism.  Read-after-writes are good at doubling
        // the number of bits affected, so the goal of mixing pulls in the opposite
        // direction as the goal of parallelism.  I did what I could.  Rotates
        // seem to cost as much as shifts on every machine I could lay my hands
        // on, and rotates are much kinder to the top and bottom bits, so I used
        // rotates.
        // ------------------------------------------------------------------------
        template <typename T>
        inline void mix(T& a, T& b, T& c)
        {
            // clang-format off
            a -= b; a -= c; a ^= (c >> 13);
            b -= c; b -= a; b ^= (a << 8);
            c -= a; c -= b; c ^= (b >> 13);
            a -= b; a -= c; a ^= (c >> 12);
            b -= c; b -= a; b ^= (a << 16);
            c -= a; c -= b; c ^= (b >> 5);
            a -= b; a -= c; a ^= (c >> 3);
            b -= c; b -= a; b ^= (a << 10);
            c -= a; c -= b; c ^= (b >> 15);
            // clang-format on
        }
    }    // namespace detail

    /////////////////////////////////////////////////////////////////////////////
    /// The jenkins_hash class encapsulates a hash calculation function published
    /// by Bob Jenkins here: http://burtleburtle.net/bob/hash
    class jenkins_hash
    {
    public:
        /// this is the type representing the result of this hash
        using size_type = std::uint32_t;

        /// The seedenum is used as a dummy parameter to distinguish the different
        /// constructors
        enum seedenum
        {
            seed = 1
        };

        /// constructors and destructor
        jenkins_hash()
          : seed_(0)
        {
        }

        explicit jenkins_hash(size_type size)
        {
            unsigned int _seed = std::random_device{}();
            std::mt19937 gen{_seed};
            seed_ = std::uniform_int_distribution<>(0, size - 1)(gen);
        }

        explicit jenkins_hash(size_type seedval, seedenum)
          : seed_(seedval)
        {
        }

        ~jenkins_hash() {}

        /// calculate the hash value for the given key
        size_type operator()(std::string const& key) const
        {
            return hash(key.c_str(), static_cast<std::size_t>(key.size()));
        }

        size_type operator()(char const* key) const
        {
            return hash(key, std::strlen(key));
        }

        /// re-seed the hash generator
        bool reset(size_type size)
        {
            seed_ = rand() % size;
            return true;
        }

        /// initialize the hash generator to a specific seed
        void set_seed(size_type seedval)
        {
            seed_ = seedval;
        }

        /// support for std::swap
        void swap(jenkins_hash& rhs)
        {
            std::swap(seed_, rhs.seed_);
        }

    protected:
        // hash() -- hash a variable-length key into a 32-bit value
        // k       : the key (the unaligned variable-length array of bytes)
        // len     : the length of the key, counting by bytes
        // Returns a 32-bit value.  Every bit of the key affects every bit of
        // the return value.  Every 1-bit and 2-bit delta achieves avalanche.
        // About 6*len+35 instructions.
        //
        // The best hash table sizes are powers of 2.  There is no need to do
        // mod a prime (mod is sooo slow!).  If you need less than 32 bits,
        // use a bitmask.  For example, if you need only 10 bits, do
        // h = (h & hashmask(10));
        // In which case, the hash table should have hashsize(10) elements.
        //
        // If you are hashing n strings (cmph_uint8 **)k, do it like this:
        // for (i=0, h=0; i<n; ++i) h = hash( k[i], len[i], h);
        //
        // By Bob Jenkins, 1996.  bob_jenkins@burtleburtle.net.  You may use this
        // code any way you wish, private, educational, or commercial.  It's free.
        //
        // See http://burtleburtle.net/bob/hash/evahash.html
        // Use for hash table lookup, or anything where one collision in 2^^32 is
        // acceptable.  Do NOT use for cryptographic purposes.
        size_type hash(const char* k, std::size_t length) const
        {
            size_type a, b, c;
            std::size_t len = length;

            /* Set up the internal state */
            a = b = 0x9e3779b9; /* the golden ratio; an arbitrary value */
            c = seed_;          /* the previous hash value - seed in our case */

            /*---------------------------------------- handle most of the key */
            while (len >= 12)
            {
                a += (k[0] + ((size_type) k[1] << 8) +
                    ((size_type) k[2] << 16) + ((size_type) k[3] << 24));
                b += (k[4] + ((size_type) k[5] << 8) +
                    ((size_type) k[6] << 16) + ((size_type) k[7] << 24));
                c += (k[8] + ((size_type) k[9] << 8) +
                    ((size_type) k[10] << 16) + ((size_type) k[11] << 24));
                detail::mix(a, b, c);
                k += 12;
                len -= 12;
            }

            /*------------------------------------- handle the last 11 bytes */
            c += (size_type) length;
            switch (len) /* all the case statements fall through */
            {
            case 11:
                c += ((size_type) k[10] << 24);
                PIKA_FALLTHROUGH;
            case 10:
                c += ((size_type) k[9] << 16);
                PIKA_FALLTHROUGH;
            case 9:
                c += ((size_type) k[8] << 8);
                PIKA_FALLTHROUGH;
                /* the first byte of c is reserved for the length */
            case 8:
                b += ((size_type) k[7] << 24);
                PIKA_FALLTHROUGH;
            case 7:
                b += ((size_type) k[6] << 16);
                PIKA_FALLTHROUGH;
            case 6:
                b += ((size_type) k[5] << 8);
                PIKA_FALLTHROUGH;
            case 5:
                b += k[4];
                PIKA_FALLTHROUGH;
            case 4:
                a += ((size_type) k[3] << 24);
                PIKA_FALLTHROUGH;
            case 3:
                a += ((size_type) k[2] << 16);
                PIKA_FALLTHROUGH;
            case 2:
                a += ((size_type) k[1] << 8);
                PIKA_FALLTHROUGH;
            case 1:
                a += k[0];
                /* case 0: nothing left to add */
            }

            detail::mix(a, b, c);
            return c; /* report the result */
        }

    private:
        size_type seed_;
    };
}}    // namespace pika::util
