// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

// STD
#include <vector>

// OZKS
#include "oZKS/compressed_trie.h"
#include "oZKS/ct_node.h"
#include "oZKS/ozks.h"
#include "oZKS/storage/storage.h"

namespace ozks {
    namespace storage {
        class StorageNodeKey {
        public:
            StorageNodeKey(const std::vector<std::byte> &trie_id, const partial_label_type &node_id)
                : trie_id_(trie_id), node_id_(node_id)
            {}

            const std::vector<std::byte> &trie_id() const
            {
                return trie_id_;
            }

            const partial_label_type &node_id() const
            {
                return node_id_;
            }

            bool operator==(const StorageNodeKey &other) const
            {
                return (trie_id_ == other.trie_id_ && node_id_ == other.node_id_);
            }

            bool operator<(const StorageNodeKey &other) const
            {
                if (trie_id_ == other.trie_id_)
                    return node_id_ < other.node_id_;

                return trie_id_ < other.trie_id_;
            }

        private:
            std::vector<std::byte> trie_id_;
            partial_label_type node_id_;
        };

        class StorageTrieKey {
        public:
            StorageTrieKey(const std::vector<std::byte> &trie_id) : trie_id_(trie_id)
            {}

            const std::vector<std::byte> &trie_id() const
            {
                return trie_id_;
            }

            bool operator==(const StorageTrieKey &other) const
            {
                return trie_id_ == other.trie_id_;
            }

            bool operator<(const StorageTrieKey &other) const
            {
                return trie_id_ < other.trie_id_;
            }

        private:
            std::vector<std::byte> trie_id_;
        };

        class StorageOZKSKey {
        public:
            StorageOZKSKey(const std::vector<std::byte> &trie_id) : trie_id_(trie_id)
            {}

            const std::vector<std::byte> &trie_id() const
            {
                return trie_id_;
            }

            bool operator==(const StorageOZKSKey &other) const
            {
                return trie_id_ == other.trie_id_;
            }

            bool operator<(const StorageOZKSKey &other) const
            {
                return trie_id_ < other.trie_id_;
            }

        private:
            std::vector<std::byte> trie_id_;
        };

        class StorageStoreElementKey {
        public:
            StorageStoreElementKey(
                const std::vector<std::byte> &trie_id, const std::vector<std::byte> &key)
                : trie_id_(trie_id), key_(key)
            {}

            const std::vector<std::byte> &trie_id() const
            {
                return trie_id_;
            }

            const std::vector<std::byte> &key() const
            {
                return key_;
            }

            bool operator==(const StorageStoreElementKey &other) const
            {
                return (trie_id_ == other.trie_id_ && key_ == other.key_);
            }

            bool operator<(const StorageStoreElementKey &other) const
            {
                if (trie_id_ == other.trie_id_)
                    return key_ < other.key_;

                return trie_id_ < other.trie_id_;
            }

        private:
            std::vector<std::byte> trie_id_;
            std::vector<std::byte> key_;
        };

        struct StorageNodeKeyHasher {
            std::size_t operator()(const StorageNodeKey &key) const
            {
                return std::hash<std::vector<bool>>()(key.node_id());
            }
        };

        struct StorageTrieKeyHasher {
            std::size_t operator()(const StorageTrieKey &key) const
            {
                utils::byte_vector_hash hasher;
                return hasher(key.trie_id());
            }
        };

        struct StorageOZKSKeyHasher {
            std::size_t operator()(const StorageOZKSKey &key) const
            {
                utils::byte_vector_hash hasher;
                return hasher(key.trie_id());
            }
        };

        struct StorageStoreElementKeyHasher {
            std::size_t operator()(const StorageStoreElementKey &key) const
            {
                utils::byte_vector_hash hasher;
                return hasher(key.key());
            }
        };
    } // namespace storage
} // namespace ozks
