//
// name: Kaylie Pham
// RedID: 828129478
//
// name: Aditya Bhagat
// RedID: 828612974

#include "page_table.h"
#include <cmath>

Level* create_level(int depth, int entryCount, PageTable* pageTablePtr) {

  Level* newLevel = new Level;   // create new level

  newLevel->depth = depth;
  newLevel->pageTablePtr = pageTablePtr;

  newLevel->nextLevelPtr = nullptr;
  Map* mapPtr = nullptr;
  // add entry count to page table entries
  pageTablePtr->pgTableEntries += entryCount;

	// insert page entries; new level array for internal nodes, map objects for leaf nodes
  if (depth != pageTablePtr->levelCount - 1) {
    newLevel->nextLevelPtr = new Level*[entryCount];
    for (int i = 0; i < entryCount; i++) {
      newLevel->nextLevelPtr[i] = nullptr;
    }
  }
   else {
     newLevel->mapPtr = new Map[entryCount];
     for (int i = 0; i < entryCount; i++) {
       newLevel->mapPtr[i] = create_map();
     }
   }

  return newLevel;
}

Map create_map() {
    // create new map object to return
   Map newMap = *new Map;
   // set default values of map object
   newMap.frameNum = -1;
   newMap.valid = false;


   return newMap;
}

PageTable* create_pagetable(int bitsPerLevel[], int levelCount) {

  PageTable* newPageTable = new PageTable;  // creating new page table
  newPageTable->levelCount = levelCount;    // storing number of levels


  // store count of bits for offset; offset = 32 - (number of bits for each level)
  newPageTable->numOfBitsOffset = 32;
  for (int i = 0; i < levelCount; i++) {
    newPageTable->numOfBitsOffset -= bitsPerLevel[i];
  }

  // create array of bits to shift for getting level i page index
  newPageTable->shiftAry = new int[newPageTable->levelCount];
  // create array of bits for bit mask for level i
  newPageTable->bitMaskAry = new unsigned int[newPageTable->levelCount];

  int shiftAryAmt = 32;               // assuming the address has 32 bits, initialize to 32 and decrement from there
  unsigned int bit_mask = 0x00000000; // initialize hex value to 0
  for (int i = 0; i < newPageTable->levelCount; i++) {
    shiftAryAmt -= bitsPerLevel[i];   // for each level, decrement by # of bits per level to get shift amount

    // get hex value to add to bit mask array
    bit_mask = (1U << bitsPerLevel[i]) - 1;  // ex: if bitsPerLevel[i] = 5, we need 111111(binary) or 31(decimal)
    bit_mask <<= shiftAryAmt;                // shift by shift amount given the level i
    newPageTable->bitMaskAry[i] = bit_mask;  // store hex value into bits mask array

    newPageTable->shiftAry[i] = shiftAryAmt; // store shift amount for each level i
  }

  // create array of # of next level entries for level i
  // for each level, the entry count is 2^(number of bits for level i)
  newPageTable->entryCount = new int[newPageTable->levelCount];
  int base = 2;                 // base is 2 because two binary values: 0 and 1
  int exponent = 0;
  for (int i = 0; i < newPageTable->levelCount; i++) {
    exponent = bitsPerLevel[i];  // exponent = number of bits for level i
    newPageTable->entryCount[i] = std::pow(base, exponent);
  }

  int root_depth = 0;
  // create root level with depth 0, entry count for the first level (so index 0), page table pointer
  newPageTable->rootLevel = create_level(root_depth, newPageTable->entryCount[0], newPageTable);

  // additional variable for summary
  exponent = newPageTable->numOfBitsOffset;
  newPageTable->pageSize = std::pow(base, exponent);           // bytes per page = 2^(offset)

  // all initialized to 0 because no addresses have been processed, replaced, mapped, allocated
  newPageTable->numOfPageReplaced = 0;      // number of page replacement
  newPageTable->pageTableHits = 0;          // number of times a virtual page was mapped
  newPageTable->numOfAddresses = 0;         // number of addresses processed
  newPageTable->numOfFramesAllocated = 0;   // number of frames allocated
  newPageTable->pgTableEntries = 0;         // total number of page table entries across all levels

  return newPageTable;
}

unsigned int getVPNFromVirtualAddress(unsigned int virtualAdd, unsigned int mask, unsigned int shift) {

  int page;   // stores the VPN of any level or full VPN, depending on given mask or shift amount

  page = virtualAdd & mask;  // extract the relevant bits with bitwise and
  page >>= shift;            // shift right by given shift amount

  return page;
}

// Inserting VPN -> PFN mapping to pagetable
void insertVpn2PfnMapping(PageTable* PageTable, unsigned int VPN, int frameNum) {

    // starting from root level
    Level* currLevel = PageTable->rootLevel;

    // traversing through each level in pagetable
    for (int i=0; i < PageTable->levelCount; ++i) {

        int vpn_index = getVPNFromVirtualAddress(VPN, PageTable->bitMaskAry[i], PageTable->shiftAry[i]);
        if (vpn_index < 0 || vpn_index >= PageTable->entryCount[i]) {
          return;
        }

        // when leaf level reached and map PFN does not exist, create map object at given index
        if (i == PageTable->levelCount - 1) {
        	if (currLevel->mapPtr[vpn_index].frameNum == -1) {
            	currLevel->mapPtr[vpn_index].frameNum = frameNum;           // assigning PFN
            	currLevel->mapPtr[vpn_index].valid = (frameNum >= 0);       // setting validity
            	PageTable->numOfFramesAllocated++;
           		return;
                }
         }
         // creating next level nodes for non-leaf level if it doesn't exist
        else {
          if (currLevel->nextLevelPtr[vpn_index] == nullptr) {
            currLevel->nextLevelPtr[vpn_index] = create_level(i+1, PageTable->entryCount[i+1], PageTable);
          }
          // moving to next level
          currLevel = currLevel->nextLevelPtr[vpn_index];
        }
    }
}

// looks up the VPN in the pagetable and returns the Map only if valid
Map* findVpn2PfnMapping(PageTable* PageTable, unsigned int VPN) {
    // starting at the root node
    Level* currLevel = PageTable->rootLevel;

    // traversing through levels in pagetable
    for (int i=0; i < PageTable->levelCount; ++i){

      	// extracting the index into that level's array
        int index = getVPNFromVirtualAddress(VPN, PageTable->bitMaskAry[i], PageTable->shiftAry[i]);
        if (index < 0 || index >= PageTable->entryCount[i]) return nullptr;
        

        // if at leaf level and the map object has a valid flag, return the map
        if (i == PageTable->levelCount - 1){
            if (!currLevel->mapPtr[index].valid){        // check if the map entry is valid
            return nullptr;                             // if not return nullptr
            }
            return &currLevel->mapPtr[index];           // if valid, return the pointer to Map object with PFN
        }

        // moving to next level if exists, else return null
        if (currLevel->nextLevelPtr[index] == nullptr){         //check if valid mapping found
            return nullptr;                                     // returning nullptr if not
        }
        // if next level exists, go to next level
        currLevel = currLevel->nextLevelPtr[index];
    }
    return nullptr;
}
