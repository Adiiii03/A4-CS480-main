//
// name: Kaylie Pham
// RedID: 828129478
//
// name: Aditya Bhagat
// RedID: 828612974

#ifndef PAGE_TABLE_H
#define PAGE_TABLE_H

#include <iostream>
#include <cstring>

struct PageTable;

struct Map{

    int frameNum;               // the frame number
    bool valid;                 // whether the mapping is valid or not

};

struct Level{

    int depth;                  // current depth level (root=0)
    PageTable* pageTablePtr;    // pointer back to the parent table
    Level** nextLevelPtr;        // stores the array of next Level* pointers, for non-leaf nodes
    Map* mapPtr;               // stores the array of map objects, for leaf nodes

};

struct PageTable{

    int levelCount;             // to track number of levels
    unsigned int* bitMaskAry;            // bits used for each level
    int* shiftAry;              // shifts for each level
    int* entryCount;            // total entries for each level
    Level* rootLevel;           // root level for the pagetable tree
    int numOfBitsOffset;        // stores the number of bits in the offset

    // additional variable for logging purposes
    int pageSize;               // bytes per page
    int numOfPageReplaced;      // number of page replacement
    int pageTableHits;          // number of times a virtual page was mapped
    int numOfAddresses;           // number of addresses processed
    int numOfFramesAllocated;   // number of frames allocated
    int pgTableEntries;         // total number of page table entries acrosss all levels

};

Map create_map();    // constructor for map object

Level* create_level(int depth, int entryCount, PageTable* pageTablePtr);   // constructor for Level

PageTable* create_pagetable(int bitsPerLevel[], int levelCount);     // constructor for page table

/**
 * @brief Applies bit mask and shift right to given virtual address. Used to retrieve full VPN or VPN of any level.
 * @param virtualAdd - virtual address
 * @param mask - bit mask
 * @param shift - amount to shift right
 * Returns the virtual page number (vpn)
 */
unsigned int getVPNFromVirtualAddress(unsigned int virtualAdd, unsigned int mask, unsigned int shift);

/**
 * @brief Adds new entries to page table for VPN -> PFN mapping
 * @param VPN - virtual page number
 * @param frameNum - physical frame number
 */
void insertVpn2PfnMapping(PageTable *PageTable, unsigned int VPN, int frameNum);

/**
 * @brief Returns the mapping of the given VPN to PFN mapping. Returns null if virtual page is not found or the...
 *         ...mapping is not with a valid flag
 * @param PageTable - page table
 * @param VPN - virtual page number
 */
Map* findVpn2PfnMapping(PageTable *PageTable, unsigned int VPN);


#endif //PAGE_TABLE_H
