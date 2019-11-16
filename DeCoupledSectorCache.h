/*
 * DeCoupledSectorCache.h
 *
 *  Created on: 30-Sep-2019
 *      Author: prati
 */

#ifndef DECOUPLEDSECTORCACHE_H_
#define DECOUPLEDSECTORCACHE_H_

#include <stdlib.h>
#include <stdio.h>
#include <string>

using namespace std;

#define NUM_OF_BITS 32

typedef struct
{
    unsigned int tagVal;
    //unsigned int LRUVal;
} cache_addr_details;

typedef struct
{
    unsigned int valid;
    unsigned int dirty;
    unsigned int selection;
} cache_data_details;

class DeCoupledSectorCache
{
public:
    DeCoupledSectorCache();
    virtual ~DeCoupledSectorCache();
    //Functions
    void setValue(unsigned long int blockSizeIn,
            unsigned long int sizeIn, unsigned long int assocIn,
            unsigned long int dataBlock, unsigned long int addrTag);
    void printL2Content();
    void printSimlationResultL2();
    void evictData();
    bool readFromAddress(unsigned int addr);
    bool writeToAddress(unsigned int addr);
private:
    //Common params
    unsigned long int size;
    unsigned long int assoc;
    unsigned long int block_size;
    unsigned long int num_of_sets;

    //L2 cache params
    unsigned long int l2_DataBlock;   //
    unsigned long int l2_AddressTag;  //

    //cache structure
    cache_addr_details ***cache_addr;
    cache_data_details ***cache_data;

    //Bits related to params
    unsigned int bits_index;
    unsigned int bits_offset;
    unsigned int bits_tag;
    unsigned int bits_dataBlock_p;
    unsigned int bits_addrTag_n;
    unsigned int indexMask;
    unsigned int dataBlock_pMask;
    unsigned int addrTag_nMask;
    unsigned int blockMask;
    //hit-miss related variables
    unsigned int reads;
    unsigned int readMiss;
    unsigned int write;
    unsigned int writeMiss;
    unsigned int writeBackFromMemory;
    unsigned int missRate;
    unsigned int totalMemoryTrafic;

    unsigned int tagValReq;
    unsigned int IndexValReq;
    unsigned int addrValReq;
    unsigned int dataBloackValReq;

    //for wrtie back address
    unsigned int writeBackAddr;
    unsigned int cacheBlockMiss;
    unsigned int sectorMiss;
    unsigned int kValIndex;
    unsigned int jValIndex;
    bool isL2;
};

#endif /* DECOUPLEDSECTORCACHE_H_ */
