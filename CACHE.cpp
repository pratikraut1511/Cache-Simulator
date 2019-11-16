/*
 * CACHE.cpp
 *
 *  Created on: 17-Sep-2019
 *      Author: prati
 */

#include "CACHE.h"
#include <cmath>
#include <iomanip>
#include <iostream>
#include <bits/stdc++.h>
#include "defination.h"

using namespace std;



CACHE::CACHE()
{
    reads = 0;
    readMiss = 0;
    write = 0;
    writeMiss = 0;
    writeBackFromMemory = 0;
    missRate = 0;
    totalMemoryTrafic = 0;
    isL2 = false;
    tagValReq = 0;
    IndexValReq = 0;
    jValIndex = 0;
    isFoundRun = 0;
}

CACHE::CACHE(unsigned long int blockSizeIn, unsigned long int l1_sizeIn,
        unsigned long int assocIn)
{
    block_size = blockSizeIn;
    assoc = assocIn;
    size = l1_sizeIn;
    num_of_sets = round((size / (assoc * block_size * 1.0)));
    //round ceil for round off of value
    bits_offset = ceil(log2(block_size));
    bits_index = ceil(log2(num_of_sets));
    bits_tag = NUM_OF_BITS - bits_index - bits_offset;
    indexMask = (unsigned int) pow(2, bits_index) - 1;
    blockMask = (unsigned int) pow(2, bits_offset) - 1;
    isL2 = true;
    cache_memory_details = new cache_info*[num_of_sets];
    for (unsigned int i = 0; i < num_of_sets; i++)
        cache_memory_details[i] = new cache_info[assoc];

    for (unsigned long int i = 0; i < num_of_sets; i++)
    {
        for (unsigned long int j = 0; j < assoc; j++)
        {
            cache_memory_details[i][j].valid = 0;
            cache_memory_details[i][j].tag = 0;
            cache_memory_details[i][j].LRUVal = 0;
            cache_memory_details[i][j].dirty = 0;
        }
    }
    //Initial Values
    reads = 0;
    readMiss = 0;
    write = 0;
    writeMiss = 0;
    writeBackFromMemory = 0;
    missRate = 0;
    totalMemoryTrafic = 0;
    l2_DataBlock = 0;
    l2_AddressTag = 0;
    //Common
    tagValReq = 0;
    IndexValReq = 0;
    jValIndex = 0;
    isFoundRun = false;
    isDirty = false;
    cout << "===== Simulator configuration =====" << endl;
    cout << "BLOCKSIZE:                        " << block_size << endl;
    cout << "L1_SIZE:                          " << size << endl;
    cout << "L1_Assoc:                         " << assoc << endl;
}
void CACHE::setValues(unsigned long int blockSizeIn, unsigned long int sizeIn,
        unsigned long int assocIn, unsigned long int dataBlock,
        unsigned long int addrTag)
{
    block_size = blockSizeIn;
    assoc = assocIn;
    size = sizeIn;
    l2_DataBlock = dataBlock;
    l2_AddressTag = addrTag;
    num_of_sets = round((size / (assoc * block_size * 1.0)));
    //round ceil for round off of value
    bits_offset = ceil(log2(block_size));
    bits_index = ceil(log2(num_of_sets));
    bits_tag = NUM_OF_BITS - bits_index - bits_offset;
    indexMask = (unsigned int) pow(2, bits_index) - 1;
    blockMask = (unsigned int) pow(2, bits_offset) - 1;
#if DEBUG1
    cout << "----- Cache value L2-----" << endl;
    cout << "assoc     " << assoc << "num_of_sets   " << num_of_sets << endl;
    cout << "bits_offset   " << bits_offset << " bits_index    " << bits_index
    << endl;
#endif
    cache_memory_details = new cache_info*[num_of_sets];
    for (unsigned int i = 0; i < num_of_sets; i++)
        cache_memory_details[i] = new cache_info[assoc];

    for (unsigned long int i = 0; i < num_of_sets; i++)
    {
        for (unsigned long int j = 0; j < assoc; j++)
        {
            cache_memory_details[i][j].valid = 0;
            cache_memory_details[i][j].tag = 0;
            cache_memory_details[i][j].LRUVal = 0;
            cache_memory_details[i][j].dirty = 0;
        }
    }
    //Initial Values
    reads = 0;
    readMiss = 0;
    write = 0;
    writeMiss = 0;
    writeBackFromMemory = 0;
    missRate = 0;
    totalMemoryTrafic = 0;
    tagValReq = 0;
    IndexValReq = 0;
    jValIndex = 0;
    isFoundRun = false;
    isDirty = false;
    isL2 = true;
    cout << "L2_SIZE:                          " << size << endl;
    cout << "L2_ASSOC:                         " << assoc << endl;
    cout << "L2_DATA_BLOCKS:                   " << l2_DataBlock << endl;
    cout << "L2_ADDRESS_TAGS:                  " << l2_AddressTag << endl;
}

CACHE::~CACHE()
{
    if (isL2)
    {
        //free memory
        for (unsigned int i = 0; i < num_of_sets; i++)
        {
            delete[] cache_memory_details[i];
        }
        delete[] cache_memory_details;
    }
}

void CACHE::updateLRU(unsigned int jIndex)
{
    //Update LRU value
    for (unsigned int j = 0; j < assoc; j++)
    {
        if (jIndex != j)
        {
            cache_memory_details[IndexValReq][j].LRUVal++;
        }
    }
}

bool CACHE::readFromAddress(unsigned int addr)
{
    unsigned int blockValReq;
    unsigned int oldLURVal = 0;
    unsigned int refLURVal = 0;
    unsigned int LRUIndex = 0;
    bool isInvalid;
    tagValReq = 0;
    IndexValReq = 0;
    jValIndex = 0;
    isInvalid = false;
    bool isFound = false;
    isFoundRun = false;
    isDirty = false;
    L1inValid = false;
    tagValReq = (unsigned int) addr >> (bits_index + bits_offset);
    blockValReq = (unsigned int) addr >> (bits_offset);
    IndexValReq = (unsigned int) (blockValReq & indexMask);
#if DEBUG
    cout << "read " << hex << addr << endl;
    cout << "L1 read: " << hex << addr << " (tag " << hex << tagValReq << dec
    << ", index " << IndexValReq << ")" << endl;
#endif
    //increment L1 Reads value
    reads++;
    //Find index in cache
    for (unsigned int j = 0; j < assoc; j++)
    {
        if (cache_memory_details[IndexValReq][j].valid == 1)
        {
            if (cache_memory_details[IndexValReq][j].tag == tagValReq)
            {
                isFound = true;
                isFoundRun = true;
                jValIndex = j;
                break;
            }
        }
    }
    //Update LRU if isFound == true
    if (isFound)
    {
        oldLURVal = cache_memory_details[IndexValReq][jValIndex].LRUVal;
        cache_memory_details[IndexValReq][jValIndex].LRUVal = 0;

#if DEBUG
        cout << "L1 hit" << endl;
        // cout << "L1 update LRU" << endl;
#endif
        for (unsigned int j = 0; j < assoc; j++)
        {
            if (jValIndex != j
                    && cache_memory_details[IndexValReq][j].LRUVal < oldLURVal)
            {
                cache_memory_details[IndexValReq][j].LRUVal++;
            }
        }
    }
    else
    {
        readMiss++;
#if DEBUG
        cout << "L1 miss" << endl;
#endif
        //check for any invalid field
        for (unsigned int j = 0; j < assoc; j++)
        {
            if (cache_memory_details[IndexValReq][j].valid == 0)
            {
                isInvalid = true;
                jValIndex = j;
                L1inValid = true;
                break;
            }
        }
        if (isInvalid)
        {
            //update tag value
            cache_memory_details[IndexValReq][jValIndex].tag = tagValReq;
            cache_memory_details[IndexValReq][jValIndex].valid = 1;
            cache_memory_details[IndexValReq][jValIndex].LRUVal = 0;
            cache_memory_details[IndexValReq][jValIndex].dirty = 0;
            //Update LRU value
            updateLRU(jValIndex);
#if DEBUG
            //   cout << "L1 update LRU" << endl;
#endif
        }
        else
        {
            //Remove the Least freq used tag value and replace it with new tag value
            for (unsigned int j = 0; j < assoc; j++)
            {
                if (cache_memory_details[IndexValReq][j].LRUVal > refLURVal)
                {
                    refLURVal = cache_memory_details[IndexValReq][j].LRUVal;
                    LRUIndex = j;
                }
            }
            //update tag value
            cache_memory_details[IndexValReq][LRUIndex].valid = 1;
            cache_memory_details[IndexValReq][LRUIndex].LRUVal = 0;
            if (cache_memory_details[IndexValReq][LRUIndex].dirty == 1)
            {
                writeBackFromMemory++;
                writeBackAddr = cache_memory_details[IndexValReq][LRUIndex].tag;
                writeBackAddr = ((writeBackAddr << (bits_index + bits_offset))
                        + (IndexValReq << bits_offset));
                isDirty = true;
                cache_memory_details[IndexValReq][LRUIndex].dirty = 0;
            }
            cache_memory_details[IndexValReq][LRUIndex].tag = tagValReq;
            //Update LRU
            updateLRU(LRUIndex);
#if DEBUG
            //  cout << "L1 update LRU" << endl;
#endif
        }
    }
    return true;
}

bool CACHE::writeToAddress(unsigned int addr)
{
    unsigned int blockValReq;
    unsigned int oldLURVal = 0;
    unsigned int refLURVal = 0;
    unsigned int LRUIndex = 0;
    bool isFound = false;
    bool isInvalid = false;

    tagValReq = 0;
    IndexValReq = 0;
    jValIndex = 0;
    isFoundRun = false;
    isDirty = false;
    tagValReq = (unsigned int) addr >> (bits_index + bits_offset);
    blockValReq = (unsigned int) addr >> (bits_offset);
    IndexValReq = (unsigned int) (blockValReq & indexMask);
#if DEBUG
    cout << "write " << hex << addr << endl;
    cout << "L1 write: " << addr << " (tag " << tagValReq << dec << ", index "
    << IndexValReq << ")" << endl;
#endif
    write++;

    //Find index in cache
    for (unsigned int j = 0; j < assoc; j++)
    {
        if (cache_memory_details[IndexValReq][j].valid == 1)
        {
            if (cache_memory_details[IndexValReq][j].tag == tagValReq)
            {
                isFound = true;
                isFoundRun = true;
                jValIndex = j;
                break;
            }
        }
    }
    //Update LRU if isFound == true
    if (isFound)
    {
        oldLURVal = cache_memory_details[IndexValReq][jValIndex].LRUVal;
        cache_memory_details[IndexValReq][jValIndex].LRUVal = 0;
        cache_memory_details[IndexValReq][jValIndex].dirty = 1;
        for (unsigned int j = 0; j < assoc; j++)
        {
            if (jValIndex != j
                    && cache_memory_details[IndexValReq][j].LRUVal < oldLURVal)
            {
                cache_memory_details[IndexValReq][j].LRUVal++;
            }
        }
#if DEBUG
        cout << "L1 hit" << endl;
        //  cout << "L1 update LRU" << endl;
#endif
    }
    else
    {
        writeMiss++;
#if DEBUG
        cout << "L1 miss" << endl;
#endif
        //check for any invalid field
        for (unsigned int j = 0; j < assoc; j++)
        {
            if (cache_memory_details[IndexValReq][j].valid == 0)
            {
                isInvalid = true;
                jValIndex = j;
                break;
            }
        }
        if (isInvalid)
        {
            //update tag value
            cache_memory_details[IndexValReq][jValIndex].tag = tagValReq;
            cache_memory_details[IndexValReq][jValIndex].valid = 1;
            cache_memory_details[IndexValReq][jValIndex].LRUVal = 0;
            cache_memory_details[IndexValReq][jValIndex].dirty = 1;
            //Update LRU value
            updateLRU(jValIndex);
#if DEBUG
            //   cout << "L1 update LRU   " << endl;
#endif
        }
        else
        {
            //Remove the Least freq used tag value and replace it with new tag value
            for (unsigned int j = 0; j < assoc; j++)
            {
                if (cache_memory_details[IndexValReq][j].LRUVal > refLURVal)
                {
                    refLURVal = cache_memory_details[IndexValReq][j].LRUVal;
                    LRUIndex = j;
                }
            }
            //update tag value

            cache_memory_details[IndexValReq][LRUIndex].valid = 1;
            cache_memory_details[IndexValReq][LRUIndex].LRUVal = 0;
            if (cache_memory_details[IndexValReq][LRUIndex].dirty == 1)
            {
                writeBackAddr = cache_memory_details[IndexValReq][LRUIndex].tag;
                writeBackAddr = ((writeBackAddr << (bits_index + bits_offset))
                        + (IndexValReq << bits_offset));
                isDirty = true;
                writeBackFromMemory++;
            }
            cache_memory_details[IndexValReq][LRUIndex].tag = tagValReq;
            cache_memory_details[IndexValReq][LRUIndex].dirty = 1;
            //Update LRU
            updateLRU(LRUIndex);
#if DEBUG
            //     cout << "L1 update LRU   " << endl;
#endif
        }
    }
    return true;
}

void CACHE::swap(cache_info *val1, cache_info *val2)
{
    cache_info temp;
    temp.LRUVal = val1->LRUVal;
    temp.tag = val1->tag;
    temp.dirty = val1->dirty;

    val1->LRUVal = val2->LRUVal;
    val1->tag = val2->tag;
    val1->dirty = val2->dirty;

    val2->LRUVal = temp.LRUVal;
    val2->tag = temp.tag;
    val2->dirty = temp.dirty;
}

bool CACHE::sortVal(cache_info *val)
{
    cache_info swappedVals[assoc];
    for (unsigned int i = 0; i < assoc; i++)
    {
        swappedVals[i].LRUVal = val[i].LRUVal;
        swappedVals[i].tag = val[i].tag;
        swappedVals[i].dirty = val[i].dirty;
    }
    for (unsigned int i = 0; i < assoc - 1; i++)
    {
        for (unsigned int j = 0; j < assoc - i - 1; j++)
        {
            if (swappedVals[j].LRUVal > swappedVals[j + 1].LRUVal)
            {
                swap(&swappedVals[j], &swappedVals[j + 1]);
            }
        }
    }

    for (unsigned int i = 0; i < assoc; i++)
    {
        cout << hex << swappedVals[i].tag;
        swappedVals[i].dirty == 1 ? cout << " D\t ||" : cout << " N\t ||";
    }
    cout << endl;
    return true;
}
void CACHE::printL1Contet()
{

    cout << "\n=====L1 contents=====\n";

//    for (unsigned int i = 0; i < num_of_sets; i++)
//    {
//        cout << "Set  " << dec << i << ": " << "\t\t";
//        for (unsigned int j = 0; j < assoc; j++)
//        {
//            cout << hex << cache_memory_details[i][j].tag;
//            cout << dec << "  " << cache_memory_details[i][j].LRUVal;
//            cout << " " << cache_memory_details[i][j].dirty << "\t";
//        }
//        cout << endl;
//    }
    for (unsigned int i = 0; i < num_of_sets; i++)
    {
        cout << "set\t" << dec << i << ":\t";
        sortVal(cache_memory_details[i]);
    }
}
void CACHE::printSimulationResultL1()
{
    float rate;
    float miss = readMiss + writeMiss;
    float request = (write + reads);
    rate = (miss / request);
    cout << endl;
    cout << "===== Simulation Results =====" << endl;
    cout << "a. number of L1 reads:                    " << dec << reads
            << endl;
    cout << "b. number of L1 read misses:              " << dec << readMiss
            << endl;
    cout << "c. number of L1 writes:                   " << dec << write
            << endl;
    cout << "d. number of L1 write misses:             " << dec << writeMiss
            << endl;
    cout << "e. L1 miss rate:                          ";
    printf("%.4f\n", rate);
    cout << "f. number of writebacks from L1 memory:   " << dec
            << writeBackFromMemory << endl;
}
void CACHE::printL2Contet()
{

    cout << "\n=====L2 contents=====\n";

//    for (unsigned int i = 0; i < num_of_sets; i++)
//    {
//        cout << "Set  " << dec << i << ": " << "\t\t";
//        for (unsigned int j = 0; j < assoc; j++)
//        {
//            cout << hex << cache_memory_details[i][j].tag;
//            cout << dec << "  " << cache_memory_details[i][j].LRUVal;
//            cout << " " << cache_memory_details[i][j].dirty << "\t";
//        }
//        cout << endl;
//    }
    for (unsigned int i = 0; i < num_of_sets; i++)
    {
        cout << "set\t" << dec << i << ":\t";
        sortVal(cache_memory_details[i]);
    }
}

void CACHE::printSimlationResultL2()
{
    float rate;
    float miss = readMiss ;
    float request = ( reads);
    rate = (miss / request);
    cout << "g. number of L2 reads:                    " << dec << reads
            << endl;
    cout << "h. number of L2 read misses:              " << dec << readMiss
            << endl;
    cout << "i. number of L2 writes:                   " << dec << write
            << endl;
    cout << "j. number of L2 write misses:             " << dec << writeMiss
            << endl;
    cout << "k. L2 miss rate:                          ";
    printf("%.4f\n", rate);
    cout << "l. number of writebacks from L2 memory:   " << dec
            << writeBackFromMemory << endl;
    cout << "m. total memory traffic:                  " << dec
            << (readMiss + writeMiss + writeBackFromMemory) << endl;
}
