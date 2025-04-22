//
// name: Kaylie Pham
// RedID: 828129478
//
// name: Aditya Bhagat
// RedID: 828612974

#include <iostream>
#include <unistd.h>

#include "vaddr_tracereader.h"
#include "page_table.h"
#include "log_helpers.h"
#include "WSClock.h"
#include <vector>


int main(int argc, char* argv[]) {

    int numOfAddresses = 0;         // count of addresses processed
    int frameNum = 0;               // stores physical frame number, which starts at 0 (and will be sequentially incremented)
    p2AddrTr mtrace;                // structure is typedefed in vaddr_tracereader
    unsigned int vAddr;             // unsigned 32-bit integer type

    std::vector<WSClockEntry> WSclock;      // vector list of type WSClockEntryto hold the clocks entries
    int clock_hand_position = 0;            // position of lock hand starting at 0
    
    int maxNumMemAccesses = -1;             // only process first N memory accesses, default is all
    int numPhysicalFrames = 999999;         // number of available physical frames, default is 999999
    int ageRecentLastAccess = 10;           // age of last access considered recent for page replacement, default is 10
    const char* logMode = "summary";        // log mode, specifying what to be printed, default is "summary"


    // command line argument parsing
    int opt;
    while ((opt = getopt(argc, argv, "n:f:a:l:")) != -1) {
        switch (opt) {
            case 'n':
                maxNumMemAccesses = atoi(optarg);
                if (maxNumMemAccesses < 1) {
                  std::cerr << "Number of memory accesses must be a number, greater than 0" << std::endl;
                  return 1;    // exit prgram, error occurred
                  }
            break;
            case 'f':
                numPhysicalFrames = atoi(optarg);
                if (numPhysicalFrames < 1) {
                  std::cerr << "Number of available frames must be a number, greater than 0" << std::endl;
                  return 1;    // exit program, error occurred
                }
            break;
            case 'a':
                ageRecentLastAccess = atoi(optarg);
                if (ageRecentLastAccess < 1) {
                  std::cerr << "Age of last access considered recent must be a number, greater than 0" << std::endl;
                }
            break;
            case 'l':
                 logMode = optarg;
            break;
            default:
                return 1;
        }
    }

    int idx = optind;           // parameter, indexes next element to be processed

    const char* mtrace_file_name = argv[idx];
    idx++;                      // increment index to get next argument -- file
    const char* readswrites_file_name = argv[idx];
    idx++;                      // increment index to get next argument -- num bits for level(s)

    // opening trace and read/writes file
    FILE* mtrace_file = fopen(mtrace_file_name, "r");
    FILE* readswrites_file = fopen(readswrites_file_name, "r");

    // checking if file opens correcrtly
    if (!mtrace_file) {
        std::cerr << "Unable to open <<trace.tr>>" << std::endl;
             return 1;          // exit program, error occurred
    }
    if (!readswrites_file) {
        std::cerr << "Unable to open <<readswrites.tr>>" << std::endl;
        return 1;
    }


    // store number of bits to be used for each level
    int totalNumBitsAllLevels = 0;    // store total number of bits for all levels
    int levelCount = argc - idx;
    int bitsPerLevel[levelCount];    // create array to store bits for each level
    if (idx < argc) {
        for (int i = 0; i < levelCount; i++) {
          bitsPerLevel[i] = atoi(argv[idx + i]);            // converting and storing bits per level
          totalNumBitsAllLevels += bitsPerLevel[i];
        }

        // checking if too many bits used
       if (totalNumBitsAllLevels > 28) {
         std::cerr << "Too many bits used in page tables" << std::endl;
         return 1;          // exit program, error occurred
       }
    }
    
    // creating pagetable and WSClock entry arrays
    PageTable* page_table = create_pagetable(bitsPerLevel, levelCount);
    WSClockEntry** WSClock_entry = new WSClockEntry*[numPhysicalFrames]();      // array of pointers for WSClock entries
    
    // main memory access loop
    // magic number for maxnumMemAccess!!
    while ((maxNumMemAccesses == -1 || numOfAddresses < maxNumMemAccesses) && NextAddress(mtrace_file, &mtrace)) {
        vAddr = mtrace.addr;        // reading & storing virtual address

        // extracting VPNs for each level
        unsigned int vpns[levelCount];
        for (int i = 0; i < levelCount; ++i) {
            vpns[i] = getVPNFromVirtualAddress(vAddr, page_table->bitMaskAry[i], page_table->shiftAry[i]);
        }

        // checking if access is a write or read
        bool isWrite = (fgetc(readswrites_file) == '1');

        // checking if the address has existing mapping already
        Map* map = findVpn2PfnMapping(page_table, vAddr);
        unsigned int fullVPN = vAddr >> page_table->numOfBitsOffset; // the full VPN for logging

        if (map != nullptr) {                // pagetable hit
            page_table->pageTableHits++;
            WSClock_entry[map->frameNum]->lastUsedTime = numOfAddresses;        // updating last used time
            if (isWrite) {
                WSClock_entry[map->frameNum]->dirty = true;            // if the page is written, flag it as dirty
            }
            if (strcmp(logMode, "vpn2pfn_pr") == 0){
                log_mapping(fullVPN, map->frameNum, -1, true);     // logging map hit
            }
                
        }
        else {                               // pagetable miss
            if (page_table->numOfFramesAllocated < numPhysicalFrames) {         // checking if a free frame is available
                insertVpn2PfnMapping(page_table, vAddr, frameNum);              // if yes, add new mapping
                WSClock_entry[frameNum] = create_WSClock_entry(vAddr, frameNum, numOfAddresses, isWrite);
                if (strcmp(logMode, "vpn2pfn_pr") == 0){
                    log_mapping(fullVPN, frameNum, -1, false);     // logging new map
                }
                frameNum++;                 // incrementing frameNum
            }
            else {                          // performing page replacement
                unsigned int victimVpn = WSClock_entry[clock_hand_position]->vpn;               // getting VPN of the victim page
                clock_hand_position = page_replacement(WSClock_entry, clock_hand_position, ageRecentLastAccess, numOfAddresses, vAddr >> page_table->numOfBitsOffset, page_table);
                int replacedFrame = WSClock_entry[clock_hand_position]->frameNum;
                page_table->numOfPageReplaced++;
                if (strcmp(logMode, "vpn2pfn_pr") == 0){
                    log_mapping(fullVPN, replacedFrame, victimVpn, false);
                }
                    
            }
        }

        page_table->numOfAddresses++;       // updating total number of address for pagetable

            // if max number of memory accesses is specified (in command line) > 0
            // and if max number of memory accesses is reached (>= maxNumMemAccesses), then exit while loop
            if (maxNumMemAccesses > 0 && numOfAddresses >= maxNumMemAccesses) {
                break;
            }
    }


    // print logs given the log mode from command line
    if (strcmp(logMode, "bitmasks") == 0){
        log_bitmasks(page_table->levelCount, page_table->bitMaskAry);
    }

    else if (strcmp(logMode, "summary") == 0) {
        log_summary(page_table->pageSize, page_table->numOfPageReplaced, page_table->pageTableHits,
            page_table->numOfAddresses, page_table->numOfFramesAllocated, page_table->pgTableEntries);
    }

    // close files before exiting
    fclose(mtrace_file);            // closing trace file
    fclose(readswrites_file);       // closing read/write file
    delete[] WSClock_entry;         // deallocating memory for WSClock entries

    return 0; // exit program, as it successfully complete
    
}



