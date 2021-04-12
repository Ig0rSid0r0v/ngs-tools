/*===========================================================================
*
*                            PUBLIC DOMAIN NOTICE
*               National Center for Biotechnology Information
*
*  This software/database is a "United States Government Work" under the
*  terms of the United States Copyright Act.  It was written as part of
*  the author's official duties as a United States Government employee and
*  thus cannot be copyrighted.  This software/database is freely available
*  to the public for use. The National Library of Medicine and the U.S.
*  Government have not placed any restriction on its use or reproduction.
*
*  Although all reasonable efforts have been taken to ensure the accuracy
*  and reliability of the software and data, the NLM and the U.S.
*  Government do not and cannot warrant the performance or results that
*  may be obtained by using this software or data. The NLM and the U.S.
*  Government disclaim all warranties, express or implied, including
*  warranties of performance, merchantability or fitness for any particular
*  purpose.
*
*  Please cite the author in any work or product based on this material.
*
* ===========================================================================
*
*/

#ifndef ALIGNS_TO_DBS_JOB_H_INCLUDED
#define ALIGNS_TO_DBS_JOB_H_INCLUDED

#include "aligns_to_job.h"
#include <set>
#include <map>
#include "hash.h"
#include "seq_transform.h"

// incompatible with multiple intput files option todo: fix by moving table creation to constructor and enable
#define LOOKUP_TABLE 0

struct DBSJob : public Job
{
    struct KmerTax : public DBS::KmerTax
    {
        KmerTax(hash_t kmer = 0, int tax_id = 0) : DBS::KmerTax(kmer, tax_id) { } // todo: remove constructor from KmerTax for faster loading ?

        bool operator < (const KmerTax &x) const // for binary search by hash
        {
            return kmer < x.kmer;
        }
    };

    typedef std::vector<KmerTax> HashSortedArray;

    HashSortedArray hash_array;
    typedef unsigned int tax_t;
    size_t kmer_len = 0;

public:

    struct Hits : public std::map<tax_t, int>
    {
        operator bool() const { return size() != 0; }

        void operator += (const Hits &x)
        {
            for (auto &other : x)
                (*this)[other.first] += other.second;
        }
    };

    virtual size_t db_kmers() const { return hash_array.size();}

    struct Matcher
    {
#if LOOKUP_TABLE
        typedef std::vector< std::pair<size_t, size_t> > HashLookupTable;
        HashLookupTable hash_lookup_table;
        int hash_lookup_shift;
#endif

        const HashSortedArray &hash_array;
        int kmer_len;
        Matcher(const HashSortedArray &hash_array, int kmer_len) : hash_array(hash_array), kmer_len(kmer_len)
        {
#if LOOKUP_TABLE
            // determining size of lookup key
            int lookup_key_bits = 1;
            while ((hash_array.size() >> lookup_key_bits) > 5)
            {
                lookup_key_bits += 1;
            }
            hash_lookup_shift = kmer_len * 2 - lookup_key_bits;

            const size_t bucket_count = size_t(1) << lookup_key_bits;
            LOG("creating lookup table with " << bucket_count << " buckets, on average " << (float(hash_array.size()) / bucket_count) << " hashes per bucket");
            hash_lookup_table.resize(bucket_count);

            // figuring out bucket ranges
            size_t hash_idx = 0;
            hash_t last_hash = 0;
            for (size_t bucket_idx = 0; bucket_idx < bucket_count; ++bucket_idx)
            {
                auto &bucket = hash_lookup_table[bucket_idx];
                bucket.first = hash_idx;
                while (true)
                {
                    if (hash_idx >= hash_array.size())
                        break;
                    hash_t hash = hash_array[hash_idx].kmer;
                    assert(hash >= last_hash);
                    const size_t hash_bucket = hash >> hash_lookup_shift;
                    if (hash_bucket != bucket_idx)
                        break;
                    
                    ++hash_idx;
                    last_hash = hash;
                }
                bucket.second = hash_idx;
            }
#endif
        }

        int find_hash(hash_t hash, int default_value) const
        {
#if LOOKUP_TABLE
            hash_t bucket_idx = hash >> hash_lookup_shift;
            auto &bucket = hash_lookup_table[bucket_idx];
            auto first = hash_array.begin() + bucket.first;
            auto last = hash_array.begin() + bucket.second;
#else
            auto first = hash_array.begin();
            auto last = hash_array.end();
#endif
            first = std::lower_bound(first, last, KmerTax(hash, 0));
            return ((first == last) || (hash < first->kmer) ) ? default_value : first->tax_id;
        }

        Hits operator() (const std::string &seq) const 
        {
            Hits hits;
            Hash<hash_t>::for_all_hashes_do(seq, kmer_len, [&](hash_t hash)
                {
                    if (auto tax_id = get_db_tax(hash))
                        hits[tax_id] ++;

                    return true;
                });

            return hits;
        }

        tax_t get_db_tax(hash_t hash) const
        {
            auto tax_id = get_db_tax_0_variations(hash);
#if 0
            if (tax_id)
                return tax_id;
            
            seq_transform<hash_t>::for_all_1_char_variations_do(hash, KMER_LEN, [&](hash_t hash)
                {
                    hash = seq_transform<hash_t>::min_hash_variant(hash, KMER_LEN);
                    tax_id = find_hash(hash, 0);
                    return !tax_id;
                });

            if (tax_id)
                return tax_id;

            seq_transform<hash_t>::for_all_2_char_variations_do(hash, KMER_LEN, [&](hash_t hash)
                {
                    hash = seq_transform<hash_t>::min_hash_variant(hash, KMER_LEN);
                    tax_id = find_hash(hash, 0);
                    return !tax_id;
                });

            if (tax_id)
                return tax_id;

            seq_transform<hash_t>::for_all_3_char_variations_do(hash, KMER_LEN, [&](hash_t hash)
                {
                    hash = seq_transform<hash_t>::min_hash_variant(hash, KMER_LEN);
                    tax_id = find_hash(hash, 0);
                    return !tax_id;
                });
#endif
            return tax_id;
        }

        tax_t get_db_tax_0_variations(hash_t hash) const
        {
            hash = seq_transform<hash_t>::min_hash_variant(hash, kmer_len);
            return find_hash(hash, 0);
        }
    };

    struct TaxMatchId
    {
        int seq_id;
        Hits hits;
        TaxMatchId(int seq_id, const Hits &hits) : seq_id(seq_id), hits(hits)   {}

        bool operator < (const TaxMatchId &b) const { return seq_id < b.seq_id; }
    };

    struct TaxPrinter
    {
        IO::Writer &writer;
        const bool print_counts, compact;
        TaxPrinter(bool print_counts, bool compact, IO::Writer &writer) : print_counts(print_counts), compact(compact), writer(writer) {}

        void operator() (const std::vector<Reader::Fragment> &processing_sequences, const std::vector<TaxMatchId> &ids)
        {
            if (compact)
                print_compact(processing_sequences, ids);
            else
                print(processing_sequences, ids);

            writer.check();
        }

    private:
        void print(const std::vector<Reader::Fragment> &processing_sequences, const std::vector<TaxMatchId> &ids)
        {
            for (auto seq_id : ids)
                print_id_info(processing_sequences, seq_id);
        }

        void print_spotid(const Reader::Fragment &fragment)
        {
            for (auto c : fragment.spotid) 
            {
                if (c == '\t' || c == '\n')
                    c = ' ';
                writer.f() << c;
            }
        }

        void print_id_info(const std::vector<Reader::Fragment> &processing_sequences, const TaxMatchId &seq_id)
        {
            print_spotid(processing_sequences[seq_id.seq_id]);

            for (auto &hit : seq_id.hits) 
            {
                writer.f() << '\t' << hit.first;
                if (print_counts && hit.second > 1)
                    writer.f() << 'x' << hit.second;
            }

            writer.f() << std::endl;
        }

        void print_compact(const std::vector<Reader::Fragment> &processing_sequences, const std::vector<TaxMatchId> &ids)
        {
            int from = 0, to = 0;
            for (from = 0; from < ids.size() && processing_sequences.begin()->spotid == processing_sequences[ids[from].seq_id].spotid; from++);
            for (to = ids.size() - 1; to >= 0 && processing_sequences.rbegin()->spotid == processing_sequences[ids[to].seq_id].spotid; to--);

            print_first_and_last(processing_sequences, ids, from, to); // to merge them later correctly

            auto spots = merge_to_spots(processing_sequences, ids, from, to);
            auto reverse_index = spot_tax_reverse_index(spots);

            for (auto &x: reverse_index)
            {
                writer.f() << x.second;
                for (auto &tax : x.first)
                    writer.f() << '\t' << tax;

                writer.f() << std::endl;
            }
        }

        void print_first_and_last(const std::vector<Reader::Fragment> &processing_sequences, const std::vector<TaxMatchId> &ids, int &from, int &to)
        {
            for (int i = 0; i < from; i++)
            {
                writer.f() << '\t';
                print_id_info(processing_sequences, ids[i]);
            }

            for (int i = to + 1; i < ids.size(); i++)
            {
                writer.f() << '\t';
                print_id_info(processing_sequences, ids[i]);
            }
        }

        typedef long long spot_id_t;
        typedef int tax_id_t;
        typedef std::map<spot_id_t, std::set<tax_id_t> > SpotTaxIds; // todo: unordered map ?

        SpotTaxIds merge_to_spots(const std::vector<Reader::Fragment> &processing_sequences, const std::vector<TaxMatchId> &ids, int &from, int &to)
        {
            SpotTaxIds spots;
            for (int i = from; i <= to; i++)
            {
                auto &spot = spots[std::stoll(processing_sequences[ids[i].seq_id].spotid)];
                for (auto &hit : ids[i].hits)
                    spot.insert(hit.first);
            }

            return spots;
        }
        
        typedef std::map< std::set<tax_id_t>, spot_id_t> TaxSpotIds;

        TaxSpotIds spot_tax_reverse_index(const SpotTaxIds &spots)
        {
            TaxSpotIds tax_spots;
            for (auto &spot : spots)
                tax_spots[spot.second]++;

            return tax_spots;
        }

    };

    bool hide_counts = false;
    bool compact = false;

    virtual void run(const std::string &filename, IO::Writer &writer, const Config &config) override
    {
        hide_counts = config.hide_counts;
        compact = config.compact;
        Matcher m(hash_array, kmer_len); // todo: move to constructor
        Job::run_for_matcher(filename, config.spot_filter_file, config.unaligned_only, [&](const std::vector<Reader::Fragment> &chunk){ match_and_print_chunk(chunk, writer, m); } );
    }

    virtual void match_and_print_chunk(const std::vector<Reader::Fragment> &chunk, IO::Writer &writer, Matcher &m) // override
    {
//      Matcher m(hash_array, kmer_len);
        TaxPrinter print(!hide_counts, compact, writer);
        Job::match_and_print<Matcher, TaxPrinter, TaxMatchId>(chunk, print, m);
    }
};

struct DBSBasicJob : public DBSJob
{
    DBSBasicJob(const std::string &dbs)
    {
        kmer_len = DBSIO::load_dbs(dbs, hash_array);
    }
};

#endif
