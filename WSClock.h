
#ifndef WSCLOCK_H
#define WSCLOCK_H

#include <vector>
#include <cstdint>
#include "page_table.h"

struct WSClockEntry{
    unsigned int vpn;       // virtual page number (full)
    int frameNum;           // physical frame number
    int lastUsedTime;       // time this page was last accessed
    bool dirty;             // true if page has been written to
};

WSClockEntry* create_WSClock_entry(unsigned int vpn, int frameNum, int lastAccessTime, bool dirty);


int page_replacement(WSClockEntry** entries, int clock_hand_position, int ageThreshold, int currentTime, unsigned int newVpn, PageTable* PageTable);

#endif