//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// lru_k_replacer.h
//
// Identification: src/include/buffer/lru_k_replacer.h
//
// Copyright (c) 2015-2022, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#pragma once

#include <limits>
#include <list>
#include <mutex>  // NOLINT
#include <unordered_map>
#include <vector>
#include<memory>

#include "common/config.h"
#include "common/macros.h"

namespace bustub {

enum class AccessType { Unknown = 0, Lookup, Scan, Index };

class LRUKNode {
 private:
  /** History of last seen K timestamps of this page. Least recent timestamp stored in front. */
  // Remove maybe_unused if you start using them. Feel free to change the member variables as you want.

  // Storing timestamp in sec till 1st epoch, so minimum will be oldest.
  // Adding new access at front and oldest will be at last
  std::list<size_t> history_;
  size_t k_;
  frame_id_t fid_;
  bool is_evictable_{false};                  // false means it will not present in min heap
  size_t index_in_heap_arr_;                  // keep the index of minheap arr, at which index it is present

  public:

  // Constructor
  explicit LRUKNode(size_t k,frame_id_t fid);

  // Return history_
  std::list<size_t> getHistory();

  // Return fid_
  frame_id_t getFrameId();

  // Return index_in_heap_arr_
  size_t getIndexInHeapArr();

  // Set index_in_heap_arr_
  void setIndexInHeapArr(size_t ind);

  // get is_evicatble value
  bool getIsEvictable();

  // set is_evicatble value
  void setIsEvictable(bool setVal);

  // ENter Current timestamp in history
  void enterCurrentTimeStamp();

  // ~LRUKNode();

};

class MinHeap{
  private:
    // Array which stores LRUKNode pointers
    std::shared_ptr<LRUKNode>* arr_ele_;
    size_t heap_size_;
    size_t heap_capacity_;

    void heapifyUp(size_t indOfArr);

    void heapifyDown(size_t indOfArr);

    bool comparator(std::shared_ptr<LRUKNode>& a,std::shared_ptr<LRUKNode>& b);
  
  public:

    explicit MinHeap(size_t cap);

    ~MinHeap();

    size_t getHeapSize();

    void push(std::shared_ptr<LRUKNode>);

    void pop();

    bool isEmpty();

    std::shared_ptr<LRUKNode> top(); 

    void removeEle(size_t indOfArr);

    void heapifyUpAndDown(size_t indOfArr);
};

/**
 * LRUKReplacer implements the LRU-k replacement policy.
 *
 * The LRU-k algorithm evicts a frame whose backward k-distance is maximum
 * of all frames. Backward k-distance is computed as the difference in time between
 * current timestamp and the timestamp of kth previous access.
 *
 * A frame with less than k historical references is given
 * +inf as its backward k-distance. When multiple frames have +inf backward k-distance,
 * classical LRU algorithm is used to choose victim.
 */
class LRUKReplacer {
 public:
  /**
   *
   * TODO(P1): Add implementation
   *
   * @brief a new LRUKReplacer.
   * @param num_frames the maximum number of frames the LRUReplacer will be required to store
   */
  explicit LRUKReplacer(size_t num_frames, size_t k);

  DISALLOW_COPY_AND_MOVE(LRUKReplacer);

  /**
   * TODO(P1): Add implementation
   *
   * @brief Destroys the LRUReplacer.
   */
  ~LRUKReplacer();

  /**
   * TODO(P1): Add implementation
   *
   * @brief Find the frame with largest backward k-distance and evict that frame. Only frames
   * that are marked as 'evictable' are candidates for eviction.
   *
   * A frame with less than k historical references is given +inf as its backward k-distance.
   * If multiple frames have inf backward k-distance, then evict frame with earliest timestamp
   * based on LRU.
   *
   * Successful eviction of a frame should decrement the size of replacer and remove the frame's
   * access history.
   *
   * @param[out] frame_id id of frame that is evicted.
   * @return true if a frame is evicted successfully, false if no frames can be evicted.
   */
  auto Evict(frame_id_t *frame_id) -> bool;

  /**
   * TODO(P1): Add implementation
   *
   * @brief Record the event that the given frame id is accessed at current timestamp.
   * Create a new entry for access history if frame id has not been seen before.
   *
   * If frame id is invalid (ie. larger than replacer_size_), throw an exception. You can
   * also use BUSTUB_ASSERT to abort the process if frame id is invalid.
   *
   * @param frame_id id of frame that received a new access.
   * @param access_type type of access that was received. This parameter is only needed for
   * leaderboard tests.
   */
  void RecordAccess(frame_id_t frame_id, AccessType access_type = AccessType::Unknown);

  /**
   * TODO(P1): Add implementation
   *
   * @brief Toggle whether a frame is evictable or non-evictable. This function also
   * controls replacer's size. Note that size is equal to number of evictable entries.
   *
   * If a frame was previously evictable and is to be set to non-evictable, then size should
   * decrement. If a frame was previously non-evictable and is to be set to evictable,
   * then size should increment.
   *
   * If frame id is invalid, throw an exception or abort the process.
   *
   * For other scenarios, this function should terminate without modifying anything.
   *
   * @param frame_id id of frame whose 'evictable' status will be modified
   * @param set_evictable whether the given frame is evictable or not
   * 
   */
  void SetEvictable(frame_id_t frame_id, bool set_evictable);

  /**
   * TODO(P1): Add implementation
   *
   * @brief Remove an evictable frame from replacer, along with its access history.
   * This function should also decrement replacer's size if removal is successful.
   *
   * Note that this is different from evicting a frame, which always remove the frame
   * with largest backward k-distance. This function removes specified frame id,
   * no matter what its backward k-distance is.
   *
   * If Remove is called on a non-evictable frame, throw an exception or abort the
   * process.
   *
   * If specified frame is not found, directly return from this function.
   *
   * @param frame_id id of frame to be removed
   */
  void Remove(frame_id_t frame_id);

  void RemoveWithoutLock(frame_id_t frame_id);

  /**
   * TODO(P1): Add implementation
   *
   * @brief Return replacer's size, which tracks the number of evictable frames.
   *
   * @return size_t
   */
  auto Size() -> size_t;


  // Check if frame no is valid or not
  bool validityOfFrame(frame_id_t frame_id);

 private:
  // TODO(student): implement me! You can replace these member variables as you like.
  // Remove maybe_unused if you start using them.
  std::unordered_map<frame_id_t, std::shared_ptr<LRUKNode>> node_store_;
  [[maybe_unused]] size_t current_timestamp_{0};
  [[maybe_unused]] size_t curr_size_{0};
  size_t replacer_size_;                                 // which tracks the number of evictable frames.
  size_t k_;
  std::mutex latch_;
  // storing LRUKNode pointers in it.
  MinHeap min_heap_obj_;                                    // Keep track of what to evict next
  size_t total_num_frames;
};

}  // namespace bustub
