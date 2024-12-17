#include <stdio.h>
        

        // If not found, check pageTable
        if (!hit)
        {
            if (pageTable[pageNumber] != -1)
            {
                data = physicalMemory[pageTable[pageNumber]][offset];
                physicalAddress = pageTable[pageNumber] * FRAME_SIZE + offset;
                printf("Virtual address: %d Physical address: %d Value: %d\n", logicalAddress, physicalAddress, data);
                hit = 1;
            }
        }
        // If not found, handle page fault
        if (!hit) 
        {
            numberOfPageFault += 1;
            printf("Page Fault: %d\n", numberOfPageFault);
            int frameNumber = handlePageFault(backingStoreFile, pageNumber, offset);
            data = physicalMemory[frameNumber][offset];
            physicalAddress = frameNumber * FRAME_SIZE + offset;
            // printf("Huhu: %d Huhu: %d Huhu: %d\n", frameNumber, FRAME_SIZE, offset);
            printf("Virtual address: %d Physical address: %d Value: %d\n", logicalAddress, physicalAddress, data);
        }
        addressCount += 1;
    }
    printf("Number of Translated Addresses = %d\n", addressCount);
    printf("Page Faults = %d\n", numberOfPageFault);
    printf("Page Fault Rate = %f\n", (float)numberOfPageFault / addressCount);
    printf("TLB Hits = \n");
    printf("TLB Hit Rate = \n");
    fclose(addressFile);
    fclose(backingStoreFile);
}
