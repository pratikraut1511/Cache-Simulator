/*
 * main.cpp
 *
 *  Created on: 17-Sep-2019
 *      Author: prati
 */
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "CACHE.h"
#include "defination.h"
#include <iomanip>
#include "DeCoupledSectorCache.h"

using namespace std;

int main(int argc, char *argv[])
{
    //Local variable declaration
    FILE *pFileHandler;                 // File handler for input trace file
    char *trace_file;                   // to store file name from command line
    char operation[2];                     // r or w operation
    unsigned long int address;          // memory location
    char oper;
    bool retVal = false;
    int operationCount = 0;
    unsigned int L2_blockSize;
    unsigned int L2_size;
    unsigned int L2_assoc;
    unsigned int L2_dataBlock;
    unsigned int L2_addrTag;
    bool deCoupled = false;
    bool isL2 = true;
    //Expected input pattern
    //sim_cache <BLOCKSIZE> <L1_SIZE> <L1_ASSOC> <L2_SIZE> <L2_ASSOC>
    //<L2_DATA_BLOCKS> <L2_ADDR_TAGS> <trace_file>
    if (argc != 9) // Checks if correct number of inputs have been given.
                   // Throw error and exit if wrong
    {
        cerr << "Error: Expected inputs:9 Given inputs:" << argc - 1 << endl;
        return 1;
    }
    //Create cache object
    CACHE cacheL1(atoi(argv[1]), atoi(argv[2]), atoi(argv[3]));
    CACHE cacheL2;
    DeCoupledSectorCache cacheL2De;
    L2_blockSize = atoi(argv[1]);
    L2_size = atoi(argv[4]);
    L2_assoc = atoi(argv[5]);
    L2_dataBlock = atoi(argv[6]);
    L2_addrTag = atoi(argv[7]);

    if (L2_size && L2_assoc && L2_dataBlock == 1 && L2_addrTag == 1)
    {
        deCoupled = false;
        cacheL2.setValues(L2_blockSize, L2_size, L2_assoc, L2_dataBlock,
                L2_addrTag);
    }
    else if (L2_size && L2_assoc && L2_dataBlock >= 1 && L2_addrTag >= 1)
    {
        deCoupled = true;
        cacheL2De.setValue(L2_blockSize, L2_size, L2_assoc, L2_dataBlock,
                L2_addrTag);
    }
    else
    {
        isL2 = false;
    }

    trace_file = argv[8];
    if (!isL2)
    {
        cout << "L2_SIZE:                          0" << endl;
        cout << "L2_ASSOC:                         0" << endl;
        cout << "L2_DATA_BLOCKS:                   0" << endl;
        cout << "L2_ADDRESS_TAGS:                  0" << endl;
    }
    cout << "trace_file:                       " << trace_file << endl;
    //open trace fine
    pFileHandler = fopen(trace_file, "r");

    //If FP == NULL then there is issue will opening trace_file
    if (pFileHandler == NULL)
    {
        // Throw error and exit if fopen() failed
        cout << "Error: Unable to open file " << trace_file << endl;
        return 1;
    }

#if DEBUG1
    cout << "file " << trace_file << " opened" << endl;
#endif

    while (fscanf(pFileHandler, "%s %lx", operation, &address) != EOF)
    {
        oper = operation[0];
        operationCount++;
#if DEBUG
        cout << "------------------------------------------------" << endl;
        cout << "# " << dec << operationCount << ": ";
#endif
        if (oper == 'r')
        {
            cacheL1.readFromAddress(address);
            if (!cacheL1.get_isFound())
            {
                if (!cacheL1.get_isInvalid())
                {
                    if (cacheL1.get_isDirty())
                    {
                        if (isL2)
                        {
                            deCoupled == false ?
                                    cacheL2.writeToAddress(
                                            cacheL1.get_writeBackAddr()) :
                                    cacheL2De.writeToAddress(
                                            cacheL1.get_writeBackAddr());
                        }
                    }
                }
                if (isL2)
                {
                    deCoupled == false ?
                            cacheL2.readFromAddress(address) :
                            cacheL2De.readFromAddress(address);
                }
            }
        }
        else
        {
            cacheL1.writeToAddress(address);
            if (!cacheL1.get_isFound())
            {
                if (!cacheL1.get_isInvalid())
                {
                    if (cacheL1.get_isDirty())
                    {
                        if (isL2)
                        {
                            deCoupled == false ?
                                    cacheL2.writeToAddress(
                                            cacheL1.get_writeBackAddr()) :
                                    cacheL2De.writeToAddress(
                                            cacheL1.get_writeBackAddr());
                        }
                    }
                }
                if (isL2)
                {
                    deCoupled == false ?
                            cacheL2.readFromAddress(address) :
                            cacheL2De.readFromAddress(address);
                }
            }
        }
    }

    cacheL1.printL1Contet();
    if (isL2)
        deCoupled == false ?
                cacheL2.printL2Contet() : cacheL2De.printL2Content();
    if (isL2)
    {
        if(!deCoupled)
        {
            cacheL2.set_write(cacheL1.get_writeBackFromMemory());
        }
    }
    cacheL1.printSimulationResultL1();
    if (isL2)
        deCoupled == false ?
                cacheL2.printSimlationResultL2() :
                cacheL2De.printSimlationResultL2();
    else
    {
        cout << "g. total memory traffic:                  " << dec
                << (cacheL1.get_readMiss() + cacheL1.get_writeMiss()
                        + cacheL1.get_writeBackFromMemory()) << endl;
    }
    return retVal;
}
