// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

// STD

// OZKS
#include "oZKS/storage/batch_storage.h"
#include "oZKS/storage/memory_storage_batch_inserter.h"

using namespace std;
using namespace ozks;
using namespace ozks::storage;

MemoryStorageBatchInserter::~MemoryStorageBatchInserter()
{}

bool MemoryStorageBatchInserter::load_ctnode(
    const vector<byte> &trie_id, const partial_label_type &node_id, CTNode &node)
{
    if (nullptr == storage_)
        throw runtime_error("storage is not initialized");

    // First, check unsaved nodes
    StorageNodeKey key(trie_id, node_id);
    auto unsaved_node = unsaved_nodes_.find(key);
    if (unsaved_node != unsaved_nodes_.end()) {
        node = unsaved_node->second;
        return true;
    }

    // Second, check backing storage
    return storage_->load_ctnode(trie_id, node_id, this, node);
}

void MemoryStorageBatchInserter::save_ctnode(const vector<byte> &trie_id, const CTNode &node)
{
    if (nullptr == storage_)
        throw runtime_error("storage is not initialized");

    StorageNodeKey key(trie_id, node.label);
    unsaved_nodes_[key] = node;
}

bool MemoryStorageBatchInserter::load_compressed_trie(
    const vector<byte> &trie_id, CompressedTrie &trie)
{
    if (nullptr == storage_)
        throw runtime_error("storage is not initialized");

    // First, check unsaved tries
    StorageTrieKey key(trie_id);
    auto unsaved_trie = unsaved_tries_.find(key);
    if (unsaved_trie != unsaved_tries_.end()) {
        trie = unsaved_trie->second;
        return true;
    }

    // Second, check backing storage
    return storage_->load_compressed_trie(trie_id, trie);
}

void MemoryStorageBatchInserter::save_compressed_trie(const CompressedTrie &trie)
{
    if (nullptr == storage_)
        throw runtime_error("storage is not initialized");

    StorageTrieKey key(trie.id());
    unsaved_tries_[key] = trie;
}

bool MemoryStorageBatchInserter::load_ozks(const vector<byte> &trie_id, OZKS &ozks)
{
    if (nullptr == storage_)
        throw runtime_error("storage is not initialized");

    // First, check unsaved OZKS instances
    StorageOZKSKey key(trie_id);
    auto unsaved_ozks = unsaved_ozks_.find(key);
    if (unsaved_ozks != unsaved_ozks_.end()) {
        ozks = unsaved_ozks->second;
        return true;
    }

    // Second, check backing storage
    return storage_->load_ozks(trie_id, ozks);
}

void MemoryStorageBatchInserter::save_ozks(const OZKS &ozks)
{
    if (nullptr == storage_)
        throw runtime_error("storage is not initialized");

    StorageOZKSKey key(ozks.id());
    unsaved_ozks_[key] = ozks;
}

bool MemoryStorageBatchInserter::load_store_element(
    const vector<byte> &trie_id, const vector<byte> &key, store_value_type &value)
{
    if (nullptr == storage_)
        throw runtime_error("storage is not initialized");

    // First, check unsaved store elements
    StorageStoreElementKey sekey(trie_id, key);
    auto unsaved_store_element = unsaved_store_elements_.find(sekey);
    if (unsaved_store_element != unsaved_store_elements_.end()) {
        value = unsaved_store_element->second;
        return true;
    }

    // Second, check backing storage
    return storage_->load_store_element(trie_id, key, value);
}

void MemoryStorageBatchInserter::save_store_element(
    const vector<byte> &trie_id, const vector<byte> &key, const store_value_type &value)
{
    if (nullptr == storage_)
        throw runtime_error("storage is not initialized");

    StorageStoreElementKey sekey(trie_id, key);
    unsaved_store_elements_[sekey] = value;
}

void MemoryStorageBatchInserter::flush(const vector<byte> &trie_id)
{
    if (nullptr == storage_)
        throw runtime_error("storage is not initialized");

    vector<CTNode> nodes;
    vector<CompressedTrie> tries;
    vector<OZKS> ozkss;
    vector<pair<vector<byte>, store_value_type>> store_elements;

    nodes.reserve(unsaved_nodes_.size());
    tries.reserve(unsaved_tries_.size());
    ozkss.reserve(unsaved_ozks_.size());
    store_elements.reserve(unsaved_store_elements_.size());

    for (auto &node_pair : unsaved_nodes_) {
        nodes.emplace_back(move(node_pair.second));
    }
    unsaved_nodes_.clear();

    for (auto &trie_pair : unsaved_tries_) {
        tries.emplace_back(move(trie_pair.second));
    }
    unsaved_tries_.clear();

    for (auto &ozks_pair : unsaved_ozks_) {
        ozkss.emplace_back(move(ozks_pair.second));
    }
    unsaved_ozks_.clear();

    for (auto store_element_pair : unsaved_store_elements_) {
        pair<vector<byte>, store_value_type> se(
            store_element_pair.first.key(), store_element_pair.second);
        store_elements.emplace_back(move(se));
    }
    unsaved_store_elements_.clear();

    storage_->flush(trie_id, nodes, tries, ozkss, store_elements);
}

void MemoryStorageBatchInserter::add_ctnode(const vector<byte> &, const CTNode &)
{
    throw runtime_error("Does not make sense for this Storage implementation");
}

void MemoryStorageBatchInserter::add_compressed_trie(const CompressedTrie &)
{
    throw runtime_error("Does not make sense for this Storage implementation");
}

void MemoryStorageBatchInserter::add_store_element(
    const vector<byte> &, const vector<byte> &, const store_value_type &)
{
    throw runtime_error("Does not make sense for this Storage implementation");
}

size_t MemoryStorageBatchInserter::get_compressed_trie_epoch(const vector<byte> &trie_id)
{
    CompressedTrie trie;

    // Look first in backing storage
    if (!storage_->load_compressed_trie(trie_id, trie)) {
        // Not in backing storage, try in unsaved tries
        StorageTrieKey key(trie_id);
        auto unsaved_trie = unsaved_tries_.find(key);
        if (unsaved_trie != unsaved_tries_.end()) {
            trie = unsaved_trie->second;
        }
    }

    return trie.epoch();
}

void MemoryStorageBatchInserter::load_updated_elements(
    size_t epoch, const vector<byte> &trie_id, Storage *storage)
{
    // Important: The Batch inserter usually sits between a cache and real storage. As such, it
    // makes sense for the callback to be directed to the caller of this method, instead to this
    // storage itself.
    if (storage == this) {
        // If this is the top storage, however, we don't want the callback.
        storage = nullptr;
    }
    storage_->load_updated_elements(epoch, trie_id, storage);
}

void MemoryStorageBatchInserter::delete_ozks(const vector<byte> &trie_id)
{
    {
        // Find nodes to delete
        vector<StorageNodeKey> nodes_to_delete;
        for (const auto &node : unsaved_nodes_) {
            if (node.first.trie_id() == trie_id) {
                nodes_to_delete.push_back(node.first);
            }
        }

        // Do the deletion
        for (const auto &node : nodes_to_delete) {
            unsaved_nodes_.erase(node);
        }
    }

    // There should be a single trie with the trie_id
    StorageTrieKey trie_key(trie_id);
    unsaved_tries_.erase(trie_key);

    // There should be a single OZKS instance with the trie_id
    StorageOZKSKey ozks_key(trie_id);
    unsaved_ozks_.erase(ozks_key);

    {
        // Find storage elements to delete
        vector<StorageStoreElementKey> store_elems_to_delete;
        for (const auto &se : unsaved_store_elements_) {
            if (se.first.trie_id() == trie_id) {
                store_elems_to_delete.push_back(se.first);
            }
        }

        // Do the deletion
        for (const auto &se : store_elems_to_delete) {
            unsaved_store_elements_.erase(se);
        }
    }

    // Perform same operation on backing storage
    storage_->delete_ozks(trie_id);
}
