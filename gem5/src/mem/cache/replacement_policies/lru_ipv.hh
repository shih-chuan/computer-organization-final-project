/**
 * Copyright (c) 2018 Inria
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met: redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer;
 * redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution;
 * neither the name of the copyright holders nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Authors: Moumita Laskar
 */

/**
 * @file
 * Declaration of a replacement policy based on paper
 * "Insersion and Promotion for Tree-Based PseudoLRU Last-Level Caches"
 * by Daniel A. Jimenez
 * Please Note that implementation here is not tree based but have used the
 * IPVector generated from genertic algorithm provided in the paper.
 * This code is implementation of Figure 3 of paper.
 * Gist of Implementation:
 * -Each blacks in a set is associated with a index, from 0 to (num_way-1)
 * - A vector is created which is shared between all the blocks within the set
 * - Job of this vector is to store the information about the status of each 
 *   block within the set. The status changes based on const IPV vector.
 * - Each set will have its own such vectors. 
 * - This vector's index is mapped to block's index. For example
 * - if block's index is 0, it represents the index 0 of the vect.
 * - All manupulation is done on the vect which actually represents the blocks
 * - within the sets
 */

#ifndef __MEM_CACHE_REPLACEMENT_POLICIES_LRUIPV_RP_HH__
#define __MEM_CACHE_REPLACEMENT_POLICIES_LRUIPV_RP_HH__

#include "mem/cache/replacement_policies/base.hh"

struct LRUIPVRPParams;

class LRUIPVRP : public BaseReplacementPolicy
{
  protected:

    int count;

    //Number of set associativity
    const int num_way = 16;

    typedef std::vector<int> LruIpvInst;

    LruIpvInst *vectInstance;

    /** LRU-specific implementation of replacement data. */
    struct LRUReplData : ReplacementData
    {
        /** Tick on which the entry was last touched. */
        Tick lastTouchTick;

        mutable int index;

        std::shared_ptr<LruIpvInst> vect;
        /**
         * Default constructor. 
         */
        LRUReplData(int assoc, std::shared_ptr<LruIpvInst> vect);
    };

    /**
      * IPV used for insertion and promotion 
    */ 
    const std::vector<int> IPV{0,0,1,0,3,0,1,2,1,0,5,1,0,0,1,11,13};


  public:
    /** Convenience typedef. */
    typedef LRUIPVRPParams Params;

    /**
     * Construct and initiliaze this replacement policy.
     */
    LRUIPVRP(const Params *p);

    /**
     * Destructor.
     */
    ~LRUIPVRP() {}


     //debug
     void printVect(const std::shared_ptr<ReplacementData>& replacement_data) const;

    /**
     * Invalidate replacement data to set it as the next probable victim.
     * Sets its last touch tick as the starting tick.
     *
     * @param replacement_data Replacement data to be invalidated.
     */
    void invalidate(const std::shared_ptr<ReplacementData>& replacement_data) const
                                                              override;

    /**
     * Touch an entry to update its replacement data.
     * Sets its last touch tick as the current tick.
     *
     * @param replacement_data Replacement data to be touched.
     */
    void touch(const std::shared_ptr<ReplacementData>& replacement_data) const
                                                                     override;

    /**
     * Reset replacement data. Used when an entry is inserted.
     * Sets its last touch tick as the current tick.
     *
     * @param replacement_data Replacement data to be reset.
     */
    void reset(const std::shared_ptr<ReplacementData>& replacement_data) const
                                                                     override;

    /**
     * Find replacement victim using LRU timestamps.
     *
     * @param candidates Replacement candidates, selected by indexing policy.
     * @return Replacement entry to be replaced.
     */
    ReplaceableEntry* getVictim(const ReplacementCandidates& candidates) const
                                                                     override;

    /**
     * Instantiate a replacement data entry.
     *
     * @return A shared pointer to the new replacement data.
     */
    std::shared_ptr<ReplacementData> instantiateEntry() override;
};

#endif // __MEM_CACHE_REPLACEMENT_POLICIES_LRUIPV_RP_HH__
