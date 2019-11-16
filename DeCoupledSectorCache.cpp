/*
 * DeCoupledSectorCache.cpp
 *
 *  Created on: 30-Sep-2019
 *      Author: prati
 */

#include "DeCoupledSectorCache.h"
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <bits/stdc++.h>
#include "defination.h"

using namespace std;

DeCoupledSectorCache::DeCoupledSectorCache()
{
    reads = 0;
    readMiss = 0;
    write = 0;
    writeMiss = 0;
    writeBackFromMemory = 0;
    missRate = 0;
    totalMemoryTrafic = 0;
    writeBackAddr = 0;
    cacheBlockMiss = 0;
    sectorMiss = 0;
    tagValReq = 0;
    IndexValReq = 0;
    dataBloackValReq = 0;
    addrValReq = 0;
    isL2 = false;
    kValIndex = 0;
    jValIndex = 0;
}

void DeCoupledSectorCache::setValue(unsigned long int blockSizeIn,
        unsigned long int sizeIn, unsigned long int assocIn,
        unsigned long int dataBlock, unsigned long int addrTag)
{
    block_size = blockSizeIn;
    assoc = assocIn;
    size = sizeIn;
    l2_DataBlock = dataBlock;
    l2_AddressTag = addrTag;
    isL2 = true;
    num_of_sets = round((size / (assoc * block_size * l2_DataBlock * 1.0)));
    //round ceil for round off of value
    bits_offset = ceil(log2(block_size));
    bits_index = ceil(log2(num_of_sets));
    bits_dataBlock_p = ceil(log2(l2_DataBlock));
    bits_addrTag_n = ceil(log2(l2_AddressTag));

    bits_tag = NUM_OF_BITS - bits_index - bits_offset;
    indexMask = (unsigned int) pow(2, bits_index) - 1;
    blockMask = (unsigned int) pow(2, bits_offset) - 1;
    dataBlock_pMask = (unsigned int) pow(2, bits_dataBlock_p) - 1;
    addrTag_nMask = (unsigned int) pow(2, bits_addrTag_n) - 1;
    //Memory allocation
    cache_addr = new cache_addr_details**[num_of_sets];
    for (unsigned int i = 0; i < num_of_sets; i++)
    {
        cache_addr[i] = new cache_addr_details*[assoc];
        for (unsigned int j = 0; j < assoc; j++)
        {
            cache_addr[i][j] = new cache_addr_details[l2_AddressTag];
        }
    }

    cache_data = new cache_data_details**[num_of_sets];
    for (unsigned int i = 0; i < num_of_sets; i++)
    {
        cache_data[i] = new cache_data_details*[assoc];
        for (unsigned int j = 0; j < assoc; j++)
        {
            cache_data[i][j] = new cache_data_details[l2_DataBlock];
        }
    }

    //initializing the dynamic memory
    for (unsigned long int i = 0; i < num_of_sets; i++)
    {
        for (unsigned long int j = 0; j < assoc; j++)
        {
            for (unsigned long int k = 0; k < l2_AddressTag; k++)
            {
                cache_addr[i][j][k].tagVal = 0;
            }
        }
    }

    for (unsigned long int i = 0; i < num_of_sets; i++)
    {
        for (unsigned long int j = 0; j < assoc; j++)
        {
            for (unsigned long int k = 0; k < l2_DataBlock; k++)
            {
                cache_data[i][j][k].valid = 0;
                cache_data[i][j][k].dirty = 0;
                cache_data[i][j][k].selection = 0;
            }
        }
    }
    //Print
    cout << "L2_SIZE:                          " << size << endl;
    cout << "L2_ASSOC:                         " << assoc << endl;
    cout << "L2_DATA_BLOCKS:                   " << l2_DataBlock << endl;
    cout << "L2_ADDRESS_TAGS:                  " << l2_AddressTag << endl;
}

DeCoupledSectorCache::~DeCoupledSectorCache()
{
    //free memory
    if (isL2)
    {
        for (unsigned int i = 0; i < num_of_sets; i++)
        {
            for (unsigned int j = 0; j < assoc; j++)
                delete[] cache_addr[i][j];
            delete[] cache_addr[i];
        }
        delete[] cache_addr;

        for (unsigned int i = 0; i < num_of_sets; i++)
        {
            for (unsigned int j = 0; j < assoc; j++)
                delete[] cache_data[i][j];
            delete[] cache_data[i];
        }
        delete[] cache_data;
    }
}

void DeCoupledSectorCache::evictData()
{
    for (unsigned int k = 0; k < l2_DataBlock; k++)
    {
        if (cache_data[IndexValReq][0][k].selection == addrValReq)
        {
            if (cache_data[IndexValReq][0][k].dirty == 1)
            {
                writeBackFromMemory++;
                cache_data[IndexValReq][0][k].dirty = 0;
            }
            cache_data[IndexValReq][0][k].selection = 0;
            cache_data[IndexValReq][0][k].valid = 0;
            cache_data[IndexValReq][0][k].dirty = 0;
        }
    }
}

bool DeCoupledSectorCache::readFromAddress(unsigned int addr)
{
    unsigned int blockValReq;
    tagValReq = 0;
    IndexValReq = 0;
    bool isFound = false;
    bool isMiss = false;
    tagValReq = (unsigned int) addr
            >> (bits_index + bits_offset + bits_addrTag_n + bits_dataBlock_p);
    blockValReq = (unsigned int) addr >> (bits_offset + bits_dataBlock_p);
    IndexValReq = (unsigned int) (blockValReq & indexMask);
    dataBloackValReq =
            (unsigned int) ((addr >> (bits_offset)) & dataBlock_pMask);
    addrValReq = (unsigned int) ((addr
            >> (bits_offset + bits_dataBlock_p + bits_index)) & addrTag_nMask);
#if DEBUG
    cout << "L2 read: " << hex << addr << " (C0 " << hex << dataBloackValReq
    << ", C1 " << hex << IndexValReq << ", C2 " << addrValReq << ", C3 "
    << hex << tagValReq << ")" << endl;
#endif
    //increment L1 Reads value
    reads++;
    //Find valid for a index in cache

    if (cache_data[IndexValReq][0][dataBloackValReq].valid == 1
            && cache_addr[IndexValReq][0][addrValReq].tagVal == tagValReq
            && cache_data[IndexValReq][0][dataBloackValReq].selection
                    == addrValReq)
    {
        isFound = true;
#if DEBUG
        cout << "L2 hit" << endl;
        cout << "L2 update LRU" << endl;
#endif
    }
    if (!isFound)
    {
        readMiss++;
#if DEBUG
        cout << "L2 miss" << endl;
#endif
        for (unsigned int i = 0; i < l2_DataBlock; i++)
        {
            if (cache_data[IndexValReq][0][i].valid == 1)
            {
                cacheBlockMiss++;
                isMiss = true;
                break;
            }
        }
        if (!isMiss)
        {
            sectorMiss++;
        }
        if (cache_data[IndexValReq][0][dataBloackValReq].valid == 0
                && cache_addr[IndexValReq][0][addrValReq].tagVal == tagValReq)
        {
            //update tag value
            cache_addr[IndexValReq][0][addrValReq].tagVal = tagValReq;
            cache_data[IndexValReq][0][dataBloackValReq].valid = 1;
            cache_data[IndexValReq][0][dataBloackValReq].selection = addrValReq;
            //cache_data[IndexValReq][0][dataBloackValReq].dirty = 0;
            isFound = true;
#if DEBUG
            cout << "L2 update LRU" << endl;
#endif
        }

        if (!isFound)
        {
            if (cache_addr[IndexValReq][0][addrValReq].tagVal == tagValReq)
            {
#if DEBUG
                cout << "L2 update LRU" << endl;
#endif
            }
            else
            {
                //eviction
                evictData();
            }
            if (cache_data[IndexValReq][0][dataBloackValReq].dirty == 1)
            {
                writeBackFromMemory++;
                cache_data[IndexValReq][0][dataBloackValReq].dirty = 0;
            }
            cache_addr[IndexValReq][0][addrValReq].tagVal = tagValReq;
            cache_data[IndexValReq][0][dataBloackValReq].valid = 1;
            cache_data[IndexValReq][0][dataBloackValReq].selection = addrValReq;
#if DEBUG
            cout << "L2 update LRU" << endl;
#endif
        }
    }
    return true;
}

bool DeCoupledSectorCache::writeToAddress(unsigned int addr)
{
    unsigned int blockValReq;
    bool isFound = false;
    bool isMiss = false;
    tagValReq = 0;
    IndexValReq = 0;

    tagValReq = (unsigned int) addr
            >> (bits_index + bits_offset + bits_addrTag_n + bits_dataBlock_p);
    blockValReq = (unsigned int) addr >> (bits_offset + bits_dataBlock_p);
    IndexValReq = (unsigned int) (blockValReq & indexMask);
    dataBloackValReq =
            (unsigned int) ((addr >> (bits_offset)) & dataBlock_pMask);
    addrValReq = (unsigned int) ((addr
            >> (bits_offset + bits_dataBlock_p + bits_index)) & addrTag_nMask);

#if DEBUG
    cout << "L2 write: " << " (C0 " << hex << dataBloackValReq << ", C1 " << hex
    << IndexValReq << ", C2 " << addrValReq << ", C3 " << hex
    << tagValReq << ")" << endl;
#endif
    //update write counter
    write++;

    //Find index in cache

    if (cache_data[IndexValReq][0][dataBloackValReq].valid == 1
            && (cache_addr[IndexValReq][0][addrValReq].tagVal == tagValReq
                    && cache_data[IndexValReq][0][dataBloackValReq].selection
                            == addrValReq))
    {
        //update tag value
        cache_data[IndexValReq][0][dataBloackValReq].dirty = 1;
        isFound = true;
#if DEBUG
        cout << "L2 hit" << endl;
        cout << "L2 update LRU" << endl;
#endif
    }
    if (!isFound)
    {
        writeMiss++;
        for (unsigned int i = 0; i < l2_DataBlock; i++)
        {
            if (cache_data[IndexValReq][0][i].valid == 1)
            {
                cacheBlockMiss++;
                isMiss = true;
                break;
            }
        }
        if (!isMiss)
        {
            sectorMiss++;
        }
#if DEBUG
        cout << "L2 miss" << endl;
#endif

        if (cache_data[IndexValReq][0][dataBloackValReq].valid == 0
                && cache_addr[IndexValReq][0][addrValReq].tagVal == tagValReq)
        {
            //update tag value
            cache_addr[IndexValReq][0][addrValReq].tagVal = tagValReq;
            cache_data[IndexValReq][0][dataBloackValReq].valid = 1;
            cache_data[IndexValReq][0][dataBloackValReq].selection = addrValReq;
            cache_data[IndexValReq][0][dataBloackValReq].dirty = 1;
            isFound = true;
#if DEBUG
            cout << "L2 update LRU" << endl;
#endif
        }

        if (!isFound)
        {
            if (cache_addr[IndexValReq][0][addrValReq].tagVal == tagValReq)
            {

#if DEBUG
                cout << "L2 update LRU" << endl;
#endif
            }
            else
            {
                //eviction
                evictData();
            }
            if (cache_data[IndexValReq][0][dataBloackValReq].dirty == 1)
            {
                writeBackFromMemory++;
                cache_data[IndexValReq][0][dataBloackValReq].dirty = 0;
            }
            cache_addr[IndexValReq][0][addrValReq].tagVal = tagValReq;
            cache_data[IndexValReq][0][dataBloackValReq].valid = 1;
            cache_data[IndexValReq][0][dataBloackValReq].selection = addrValReq;
            cache_data[IndexValReq][0][dataBloackValReq].dirty = 1;
#if DEBUG
            cout << "L2 update LRU" << endl;
#endif
        }
    }
    return true;
}

void DeCoupledSectorCache::printSimlationResultL2()
{
    float rate;
    float miss = readMiss;
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
    cout << "k. number of L2 sector misses:            " << dec << sectorMiss
            << endl;
    cout << "l. number of L2 cache block misses:       " << dec
            << cacheBlockMiss << endl;
    cout << "m. L2 miss rate:                          ";
    rate = readMiss / (reads * 1.0);
    printf("%.4f\n", rate);
    cout << "n. number of writebacks from L2 memory:   " << dec
            << writeBackFromMemory << endl;
    cout << "o. total memory traffic:                  " << dec
            << (readMiss + writeMiss + writeBackFromMemory) << endl;
}

void DeCoupledSectorCache::printL2Content()
{
    cout << "\n=====L2 Address Array contents=====\n" << endl;
    for (unsigned int i = 0; i < num_of_sets; i++)
    {
        cout << "set   " << dec << i << " :\t";

        for (unsigned int k = 0; k < l2_AddressTag; k++)
        {
            cout << hex << cache_addr[i][0][k].tagVal << "\t";
            cout << "\t";
        }
        cout << "||";

        cout << endl;
    }

    cout << "\n=====L2 Data Array contents=====\n" << endl;
    for (unsigned int i = 0; i < num_of_sets; i++)
    {
        cout << "set   " << dec << i << " :\t";

        for (unsigned int k = 0; k < l2_DataBlock; k++)
        {
            cout << hex << cache_data[i][0][k].selection << ",";
            cache_data[i][0][k].valid == 1 ? cout << "V" : cout << "I";
            //cout << cache_data[i][j][k].valid;
            cout << ",";
            cache_data[i][0][k].dirty == 1 ? cout << "D" : cout << "N";
            //cout << cache_data[i][j][k].dirty;
            cout << "\t";
            cout << "\t";
        }
        cout << "||";

        cout << endl;
    }
}
