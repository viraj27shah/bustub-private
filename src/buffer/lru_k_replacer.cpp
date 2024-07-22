//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// lru_k_replacer.cpp
//
// Identification: src/buffer/lru_k_replacer.cpp
//
// Copyright (c) 2015-2022, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include "buffer/lru_k_replacer.h"
#include "common/exception.h"
#include <chrono>
#include <ctime>
#include<memory>

namespace bustub {

// ################################################# LRUKNode Functions Start ################################################# //

// Constructor
LRUKNode::LRUKNode(size_t k,frame_id_t fid) : k_(k),fid_(fid) 
{
    for(size_t i=0;i<k;i++)
        history_.push_front(0);
    
    index_in_heap_arr_ = -1;
}

std::list<size_t> LRUKNode::getHistory()
{
    return history_;
}

frame_id_t LRUKNode::getFrameId()
{
    return fid_;
}

size_t LRUKNode::getIndexInHeapArr()
{
    return index_in_heap_arr_;
}

void LRUKNode::setIndexInHeapArr(size_t ind)
{
    index_in_heap_arr_ = ind;
    return;
}

bool LRUKNode::getIsEvictable()
{
    return is_evictable_;
}

void LRUKNode::setIsEvictable(bool setVal)
{
    is_evictable_ = setVal;
    return;
}

void LRUKNode::enterCurrentTimeStamp()
{
    // std::time_t now = std::time(nullptr);
    // size_t now_size_t = static_cast<size_t>(now);
    
    auto now = std::chrono::system_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
    size_t now_size_t = static_cast<size_t>(duration.count());

    history_.pop_back();
    history_.push_front(now_size_t);
    return;
}

// LRUKNode::~LRUKNode()
// {

// }

// ################################################# LRUKNode Functions End ################################################# //

// ################################################# MinHeap Functions Start ################################################# //

// Custom Comparator for MinHeap
// if(a<b) return true else false
bool MinHeap::comparator(std::shared_ptr<LRUKNode>& ele1,std::shared_ptr<LRUKNode>& ele2)
{

    std::list<size_t> l1 = ele1->getHistory();
    std::list<size_t> l2 = ele2->getHistory();

    auto it1 = l1.rbegin();
    auto it2 = l2.rbegin();

    while(it1 != l1.rend())
    {
        if(*it1 == 0 && *it2 == 0)
        {
            it1++;
            it2++;
            continue;
        }
        else if((*it1) == (*it2))
            return (ele1->getFrameId()) < (ele2->getFrameId());
        return ((*it1) < (*it2));
    }

    return true;
}

// Min Heap Constructor
MinHeap::MinHeap(size_t cap) : heap_capacity_(cap)
{
    arr_ele_ = new std::shared_ptr<LRUKNode>[heap_capacity_];
    heap_size_ = 0; 
}

// Destructor
MinHeap::~MinHeap()
{
    delete[] arr_ele_;
}

// Return the no of elements present in heap
size_t MinHeap::getHeapSize()
{
    return heap_size_;
}

// Push the element into heap
void MinHeap::push(std::shared_ptr<LRUKNode> node)
{
    BUSTUB_ASSERT(heap_capacity_ >= heap_size_, "Min Heap size can not be greater than capacity");
    arr_ele_[heap_size_] = node;
    node->setIndexInHeapArr(heap_size_);
    heapifyUp(heap_size_);
    heap_size_++;
    return;
}

// pop the element from heap
void MinHeap::pop()
{
    BUSTUB_ASSERT(!isEmpty(), "Min Heap is empty so can't do pop");
    heap_size_--;
    std::swap(arr_ele_[0],arr_ele_[heap_size_]);
    arr_ele_[0]->setIndexInHeapArr(0);
    arr_ele_[heap_size_]->setIndexInHeapArr(heap_size_);
    arr_ele_[heap_size_] = nullptr;
    heapifyDown(0);
    return;
}

void MinHeap::heapifyUp(size_t indOfArr)
{
    size_t parentIndex = (indOfArr == 0) ? 0 : (indOfArr-1)/2;
    size_t childIndex = indOfArr; 
    while(parentIndex != childIndex)
    {
        if(!comparator(arr_ele_[parentIndex],arr_ele_[childIndex]))
        {
            std::swap(arr_ele_[parentIndex],arr_ele_[childIndex]);
            arr_ele_[parentIndex]->setIndexInHeapArr(parentIndex);
            arr_ele_[childIndex]->setIndexInHeapArr(childIndex);
            childIndex = parentIndex;
            parentIndex = (childIndex == 0) ? 0 : (childIndex-1)/2;
        }
        else
        {
            break;
        }
    }
}

void MinHeap::heapifyDown(size_t indOfArr)
{
    size_t parentIndex = indOfArr;
    size_t childIndexLeft = (parentIndex*2)+1;
    size_t childIndexRight = (parentIndex*2)+2;
    // int maxEle; 
    while(parentIndex < heap_size_)
    {
       bool left = true;
       bool right = true;

       left = (childIndexLeft < heap_size_) ? comparator(arr_ele_[parentIndex],arr_ele_[childIndexLeft]) : true;
       right = (childIndexRight < heap_size_) ? comparator(arr_ele_[parentIndex],arr_ele_[childIndexRight]) : true;

       if(left == false && right == false)
       {
            if(comparator(arr_ele_[childIndexLeft],arr_ele_[childIndexRight]))
            {
                std::swap(arr_ele_[parentIndex],arr_ele_[childIndexLeft]);
                arr_ele_[parentIndex]->setIndexInHeapArr(parentIndex);
                arr_ele_[childIndexLeft]->setIndexInHeapArr(childIndexLeft);
                parentIndex = childIndexLeft;
                childIndexLeft = (parentIndex*2)+1;
                childIndexRight = (parentIndex*2)+2;
            }
            else
            {
                std::swap(arr_ele_[parentIndex],arr_ele_[childIndexRight]);
                arr_ele_[parentIndex]->setIndexInHeapArr(parentIndex);
                arr_ele_[childIndexRight]->setIndexInHeapArr(childIndexRight);
                parentIndex = childIndexRight;
                childIndexLeft = (parentIndex*2)+1;
                childIndexRight = (parentIndex*2)+2;
            }
       }
       else if(left == false)
       {
            std::swap(arr_ele_[parentIndex],arr_ele_[childIndexLeft]);
            arr_ele_[parentIndex]->setIndexInHeapArr(parentIndex);
            arr_ele_[childIndexLeft]->setIndexInHeapArr(childIndexLeft);
            parentIndex = childIndexLeft;
            childIndexLeft = (parentIndex*2)+1;
            childIndexRight = (parentIndex*2)+2;
       }
       else if(right == false)
       {
            std::swap(arr_ele_[parentIndex],arr_ele_[childIndexRight]);
            arr_ele_[parentIndex]->setIndexInHeapArr(parentIndex);
            arr_ele_[childIndexRight]->setIndexInHeapArr(childIndexRight);
            parentIndex = childIndexRight;
            childIndexLeft = (parentIndex*2)+1;
            childIndexRight = (parentIndex*2)+2;
       }
       else if(left == true && right == true)
       {
            break;
       }
    }
}

bool MinHeap::isEmpty()
{
    return (heap_size_ == 0);
}

std::shared_ptr<LRUKNode> MinHeap::top()
{
    try
    {
        if(isEmpty())
        {
            return nullptr;
        }
        return arr_ele_[0];
    }
    catch(const std::exception& e)
    {
        return nullptr;
    }
}

void MinHeap::removeEle(size_t indOfArr)
{
    BUSTUB_ASSERT(!isEmpty(), "Min Heap is empty so can't do pop");
    heap_size_--;
    std::swap(arr_ele_[indOfArr],arr_ele_[heap_size_]);
    arr_ele_[indOfArr]->setIndexInHeapArr(indOfArr);
    arr_ele_[heap_size_]->setIndexInHeapArr(heap_size_);
    arr_ele_[heap_size_]->setIndexInHeapArr(-1);
    arr_ele_[heap_size_] = nullptr;
    heapifyUpAndDown(indOfArr);
    return;
}

void MinHeap::heapifyUpAndDown(size_t indOfArr)
{
    heapifyUp(indOfArr);
    heapifyDown(indOfArr);
}


// ################################################# MinHeap Functions End ################################################# //

// ################################################# LRUKReplacer Functions Start ################################################# //

// Constructor
LRUKReplacer::LRUKReplacer(size_t num_frames, size_t k) : replacer_size_(0), k_(k),min_heap_obj_(num_frames), total_num_frames(num_frames) {}

// Evict
auto LRUKReplacer::Evict(frame_id_t *frame_id) -> bool 
{ 
    BUSTUB_ASSERT(min_heap_obj_.getHeapSize() == replacer_size_, "Min Heap size and replacer size does not match.");

    if(replacer_size_ == 0)
        return false; 
    
    std::shared_ptr<LRUKNode> topNode = min_heap_obj_.top();
    BUSTUB_ASSERT(topNode->getIsEvictable() != false, "Can not evict is evictable false frame");
    *frame_id = topNode->getFrameId();

    // // Remove node from minheap
    // min_heap_obj_.pop();

    // // Reduce replacer size
    // replacer_size_--;

    // // Remove entry from umap of LRUReplacer
    // node_store_.erase(*frame_id);

    // // Remove LRUKNODE
    // delete topNode;

    Remove(*frame_id);

    return true;
}

void LRUKReplacer::RecordAccess(frame_id_t frame_id, [[maybe_unused]] AccessType access_type) 
{
    // check validity of frame id
    BUSTUB_ASSERT(validityOfFrame(frame_id) == true, "Invalid Frame id");
    BUSTUB_ASSERT(min_heap_obj_.getHeapSize() == replacer_size_, "Min Heap size and replacer size does not match.");

    if(node_store_.find(frame_id) == node_store_.end())
    {
        // Create new node
        std::shared_ptr<LRUKNode> node = std::make_shared<LRUKNode>(k_,frame_id);
        node->enterCurrentTimeStamp();

        // Do entry in umap
        node_store_[frame_id] = node;

        // push the object in heap
        // min_heap_obj_.push(node);
    }
    else
    {
        std::shared_ptr<LRUKNode> node(node_store_[frame_id]);
        node->enterCurrentTimeStamp();

        // Heapify
        if(node->getIsEvictable())
            min_heap_obj_.heapifyUpAndDown(node->getIndexInHeapArr());
    }

}

void LRUKReplacer::SetEvictable(frame_id_t frame_id, bool set_evictable) {

    // check validity of frame id
    BUSTUB_ASSERT(validityOfFrame(frame_id) == true, "Invalid Frame id");
    BUSTUB_ASSERT(min_heap_obj_.getHeapSize() == replacer_size_, "Min Heap size and replacer size does not match.");

    if(node_store_.find(frame_id) != node_store_.end())
    {
        std::shared_ptr<LRUKNode> node (node_store_[frame_id]);
        if(set_evictable == false)
        {
            if(node->getIsEvictable() == true)
            {
                replacer_size_--;
                node->setIsEvictable(false);
                min_heap_obj_.removeEle(node->getIndexInHeapArr());
            }
        }
        else
        {
            if(node->getIsEvictable() == false)
            {
                replacer_size_++;
                node->setIsEvictable(true);
                min_heap_obj_.push(node);
            }
        }
    }
}

void LRUKReplacer::Remove(frame_id_t frame_id) 
{
    BUSTUB_ASSERT(min_heap_obj_.getHeapSize() == replacer_size_, "Min Heap size and replacer size does not match.");
    // check validity of frame id
    BUSTUB_ASSERT(validityOfFrame(frame_id) == true, "Invalid Frame id");

    BUSTUB_ASSERT(replacer_size_ > 0, "No removable frame is present");

    if(node_store_.find(frame_id) != node_store_.end())
    {
        std::shared_ptr<LRUKNode> node (node_store_[frame_id]);
        BUSTUB_ASSERT(node->getIsEvictable() == true,"Frame can not be remove as it is non evictable");

        // Remove node from minheap
        min_heap_obj_.removeEle(node->getIndexInHeapArr());

        // Reduce replacer size
        replacer_size_--;

        // Remove entry from umap of LRUReplacer
        node_store_.erase(frame_id);

        // Remove LRUKNODE
        // delete node;

    } 
    else
    {
        BUSTUB_ASSERT(false,"No frame found");
    }

    return;
}

auto LRUKReplacer::Size() -> size_t 
{ 
    return replacer_size_; 
}

bool LRUKReplacer::validityOfFrame(frame_id_t frame_id)
{ 
    if(frame_id <0 || frame_id >= static_cast<frame_id_t>(total_num_frames))
        return false; 
    else    
        return true;
}

LRUKReplacer::~LRUKReplacer()
{
    node_store_.clear();
}

// ################################################# LRUKReplacer Functions End ################################################# //

}  // namespace bustub
