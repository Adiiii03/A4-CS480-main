#include "WSClock.h"

WSClockEntry* create_WSClock_entry(unsigned int vpn, int frameNum, int lastAccessTime, bool dirty) {

    // create new WSClock entry and initialize attributes 
    WSClockEntry* WSClock_entry = new WSClockEntry;         // allocating new entry
    WSClock_entry->vpn = vpn;                               // setting vpn
    WSClock_entry->frameNum = frameNum;                     // setting frame number
    WSClock_entry->lastUsedTime = lastAccessTime;           // setting last access time
    WSClock_entry->dirty = dirty;                           // setting dirty
    return WSClock_entry;                                   // returning the new entry  
}


int page_replacement(WSClockEntry** entries, int clock_hand_position, int ageThreshold, int currentTime, unsigned int newVpn, PageTable* PageTable){


    bool victimFound = false;                               // initializing victimFound to false for tracking 
    int numFrames = PageTable->numOfFramesAllocated;        // initializing numFrames with the total number of frames given


    while(!victimFound) {
        WSClockEntry* entry = entries[clock_hand_position];         // getting current frame's WSClock entry

        int ageOfLastAccessConsideredRecent = currentTime - (entry->lastUsedTime);          // getting the age of the page

        //checking if the age of the page is greater than threshold
        if (ageOfLastAccessConsideredRecent > ageThreshold){

            // checking if the page is clean or not
            if (entry->dirty == false){
                
                unsigned int evictedVPN = entry->vpn;               // evictedVPN: setting with vpn of the current evicted page
                Map* victimMap = findVpn2PfnMapping(PageTable, evictedVPN);         // the current page in the page table

                if (victimMap != nullptr) {                                         // invalidating the current vpn 
                    victimMap->valid = false;
                }

                // updating WSClock entry with new vpn information
                int reusedFrame = entry->frameNum;                  // using the same frame
                entry->vpn = newVpn;                                //replacing the entry's vpn info with newVpn
                entry->lastUsedTime = currentTime;                  // updating last used time
                entry->dirty = false;                               // new page is clean

                // inserting the new vpn mapping to pagetable
                insertVpn2PfnMapping(PageTable, newVpn, reusedFrame);
                victimFound = true;
            }
            else{
            // clearing the dirty flag
            entry->dirty = false;
            }
        }
        if (!victimFound){
            clock_hand_position = (clock_hand_position + 1) % numFrames;        // incrementing clock_hand_position in circular manner
        }
    } 
    return clock_hand_position;             // returning new position of the clock hand
}
