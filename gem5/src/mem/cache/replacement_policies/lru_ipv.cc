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

#include "mem/cache/replacement_policies/lru_ipv.hh"

#include <cassert>
#include <memory>

#include "params/LRUIPVRP.hh"
//#include "debug/LRUDEBUG.hh"

LRUIPVRP::LRUIPVRP(const Params *p)
    : BaseReplacementPolicy(p),count(0),vectInstance(nullptr)
{
}


LRUIPVRP::LRUReplData::LRUReplData( int index, std::shared_ptr<LruIpvInst> vect): index(index), vect(vect)  {
}

void
LRUIPVRP::invalidate(const std::shared_ptr<ReplacementData>& replacement_data) const
{
    //No doing anything here as getVictim is taking care of it.
    //Invalidate just removes this block which has the value 16 
    return; 
}

void
LRUIPVRP::reset(const std::shared_ptr<ReplacementData>& replacement_data) const
{
    std::shared_ptr<LRUReplData> data = std::static_pointer_cast<LRUReplData>(replacement_data);
    LruIpvInst* vec = data->vect.get();
       
    int ipv_index = 0;
    //just increment the values in vect which are from IPV[16]  to num_way - 1. current (num_way - 1)th block  will be evicted
    while (ipv_index < vec->size()) 
    {   
         if ((vec->at(ipv_index) >= IPV[num_way]) && (vec->at(ipv_index) < num_way)){ 
             vec->at(ipv_index) = vec->at(ipv_index) + 1;
         }
         ipv_index++;
    }
    //Updating the vector value corresponding to the index of replacement block 
    vec->at(data->index) = IPV[num_way];
}

void
LRUIPVRP::touch(const std::shared_ptr<ReplacementData>& replacement_data) const
{
    std::shared_ptr<LRUReplData> data = std::static_pointer_cast<LRUReplData>(replacement_data); 
    LruIpvInst* vec = data->vect.get();

    //refer the table to get the corresponding value for promotion 
    int newVal = IPV[data->index];   
    int oldVal = vec->at(data->index);
    int ipv_index = 0;
    while (ipv_index < vec->size()) 
    {
         if((vec->at(ipv_index) >= newVal) && (vec->at(ipv_index)< oldVal))
         { 
             vec->at(ipv_index) = vec->at(ipv_index) + 1;
         }
         ipv_index++;
     }
     vec->at(data->index) = newVal;
 
}

ReplaceableEntry*
LRUIPVRP::getVictim(const ReplacementCandidates& candidates) const
{

    assert(candidates.size() > 0);

    std::shared_ptr<LRUReplData> data = std::static_pointer_cast<LRUReplData>(candidates[0]->replacementData); 
    LruIpvInst* vec = data->vect.get();
    int ipv_index = 0;
    int max = 0;
    int insertIndex = 0;
    while (ipv_index < vec->size()) 
    {
        if(max < vec->at(ipv_index))
        {
            max = vec->at(ipv_index);
            insertIndex = ipv_index;
        }
        ipv_index++;
    }
    return candidates[insertIndex];

}

std::shared_ptr<ReplacementData>
LRUIPVRP::instantiateEntry()
{
    
    //create a vector for each set. Number of elements in the vector is 
    // equal to the set associativity (num_way)
    if(count % num_way == 0)
    {
        vectInstance = new LruIpvInst(num_way,num_way);
    }

    LRUReplData* replData = new LRUReplData(count % num_way,std::shared_ptr<LruIpvInst>(vectInstance));

    count++;
    return std::shared_ptr<ReplacementData>(replData);
   
}

LRUIPVRP*
LRUIPVRPParams::create()
{
    return new LRUIPVRP(this);
}
