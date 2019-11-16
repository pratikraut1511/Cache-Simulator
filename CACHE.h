/*
 * CACHE.h
 *
 *  Created on: 17-Sep-2019
 *      Author: prati
 */

#ifndef CACHE_H_
#define CACHE_H_

#include <stdlib.h>
#include <stdio.h>
#include <string>

using namespace std;

#define NUM_OF_BITS 32

//structure to hold tag, valid and LRU
typedef struct
{
    unsigned int tag;
    unsigned int valid;
    unsigned int LRUVal;
    unsigned int dirty;
} cache_info;

class CACHE
{
public:
    CACHE();
    CACHE(unsigned long int blockSizeIn, unsigned long int l1_sizeIn,
            unsigned long int l1_assocIn);
    void setValues(unsigned long int blockSizeIn, unsigned long int sizeIn,
            unsigned long int assocIn, unsigned long int dataBlock,
            unsigned long int addrTag);
    virtual ~CACHE();

    bool readFromAddress(unsigned int addr);
    bool writeToAddress(unsigned int add);
    void updateLRU(unsigned int jIndex);
    void printL1Contet();
    void printSimulationResultL1();
    void printL2Contet();
    void printSimlationResultL2();
    void swap(cache_info *val1, cache_info *val2);
    bool sortVal(cache_info *val);

    bool get_isFound()
    {
        return isFoundRun;
    }
    bool get_isInvalid()
    {
        return L1inValid;
    }
    bool get_isDirty()
    {
        return isDirty;
    }
    unsigned int get_readMiss()
    {
        return readMiss;
    }
    unsigned int get_writeMiss()
    {
        return writeMiss;
    }
    unsigned int get_writeBackFromMemory()
    {
        return writeBackFromMemory;
    }
    unsigned int get_writeBackAddr()
    {
        return writeBackAddr;
    }

    unsigned int get_write()
    {
        return write;
    }
    void set_write(unsigned int writeVal)
    {
        write = writeVal;
    }

private:
    //Common params
    unsigned long int size;
    unsigned long int assoc;
    unsigned long int block_size;
    unsigned long int num_of_sets;

    //L2 cache params
    unsigned long int l2_DataBlock;
    unsigned long int l2_AddressTag;

    //Bits related to params
    unsigned int bits_index;
    unsigned int bits_offset;
    unsigned int bits_tag;
    unsigned int indexMask;
    unsigned int blockMask;
    //2d dynamic array
    cache_info **cache_memory_details;

    //hit-miss related variables
    unsigned int reads;
    unsigned int readMiss;
    unsigned int write;
    unsigned int writeMiss;
    unsigned int writeBackFromMemory;
    unsigned int missRate;
    unsigned int totalMemoryTrafic;
    //Common
    unsigned int tagValReq;
    unsigned int writeBackAddr;
    unsigned int IndexValReq;
    unsigned int jValIndex;
    bool isFoundRun;
    bool isDirty;
    bool L1inValid;
    bool isL2;
};

#endif /* CACHE_H_ */
