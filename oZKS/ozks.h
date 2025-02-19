// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

// STD
#include <iostream>
#include <memory>
#include <unordered_map>

// oZKS
#include "oZKS/commitment.h"
#include "oZKS/defines.h"
#include "oZKS/insert_result.h"
#include "oZKS/ozks_config.h"
#include "oZKS/query_result.h"
#include "oZKS/serialization_helpers.h"
#include "oZKS/storage/storage.h"
#include "oZKS/utilities.h"
#include "oZKS/vrf.h"

namespace ozks {
    namespace storage {
        class Storage;
    }

    class OZKS {
    public:
        /**
        Contructor for OZKS class
        */
        OZKS();

        /**
        Contructor for OZKS class
        */
        OZKS(std::shared_ptr<ozks::storage::Storage> storage);

        /**
        Contructor for OZKS class
        */
        OZKS(std::shared_ptr<ozks::storage::Storage> storage, const OZKSConfig &config);

        /**
        ID of the OZKS instance
        */
        const std::vector<std::byte> &id() const
        {
            return trie_id_;
        }

        /**
        Insert a key and payload into this instance
        */
        std::shared_ptr<InsertResult> insert(const key_type &key, const payload_type &payload);

        /**
        Insert a batch of keys and payloads into this instance
        */
        InsertResultBatch insert(const key_payload_batch_type &input);

        /**
        Perform a query for a given key
        */
        QueryResult query(const key_type &key) const;

        /**
        Flush any pending insertions
        */
        void flush();

        /**
        Check for an updated trie
        */
        void check_for_update();

        /**
        Get the VRF public key that can be used to verify VRF proofs
        */
        VRFPublicKey get_public_key() const;

        /**
        Get the current epoch
        */
        std::size_t get_epoch() const;

        /**
        Get the current commitment
        */
        Commitment get_commitment() const;

        /**
        Get the current configuration
        */
        const OZKSConfig &get_configuration() const;

        /**
        Save the current oZKS instance to a stream
        */
        std::size_t save(std::ostream &stream) const;

        /**
        Save the current oZKS instance to a byte vector
        */
        template <class T>
        std::size_t save(std::vector<T> &vec) const;

        /**
        Load an oZKS instance from a stream
        */
        static std::size_t Load(OZKS &ozks, std::istream &stream);

        /**
        Load an oZKS instance from a byte vector
        */
        template <class T>
        static std::size_t Load(OZKS &ozks, const std::vector<T> &vec, std::size_t position = 0);

        /**
        Save the current oZKS instance to storage
        */
        void save() const;

        /**
        Load the current oZKS instance from storage
        */
        static bool Load(
            const std::vector<std::byte> &trie_id,
            std::shared_ptr<ozks::storage::Storage> storage,
            OZKS &ozks);

        /**
        Clear the contents of this instance. Secret/Public keys are not cleared.
        */
        void clear();

    private:
        using pending_insertion = std::pair<key_type, payload_type>;
        using pending_result = std::shared_ptr<InsertResult>;

        VRFSecretKey vrf_sk_;
        VRFPublicKey vrf_pk_;

        std::shared_ptr<ozks::storage::Storage> storage_;
        std::vector<std::byte> trie_id_;

        std::vector<pending_insertion> pending_insertions_;
        std::vector<pending_result> pending_results_;

        std::pair<payload_type, randomness_type> commit(const payload_type &payload);

        OZKSConfig config_;

        std::size_t save(SerializationWriter &writer) const;

        static std::size_t Load(SerializationReader &reader, OZKS &ozks);

        void load_trie(CompressedTrie &trie) const;

        void initialize_vrf();

        hash_type get_key_hash(const key_type &key) const;

        void do_pending_insertions();

        void initialize();
    };
} // namespace ozks
