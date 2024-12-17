#include <stdio.h>
#include <stdlib.h>

#define SIZE_OF_TLB 16
#define SIZE_OF_PAGE_TABLE 256
#define NUMBER_OF_FRAME 128
#define FRAME_SIZE 256

int addressCount = 0;
int numberOfPageFault = 0;
int hit;
int TLB[SIZE_OF_TLB][2];
int pageTable[SIZE_OF_PAGE_TABLE];
int uptime[SIZE_OF_PAGE_TABLE];
int freeFrame[NUMBER_OF_FRAME];
int8_t physicalMemory[NUMBER_OF_FRAME][FRAME_SIZE];

void initialize()
{
    for (int i = 0; i < SIZE_OF_PAGE_TABLE; i++) 
        pageTable[i] = -1; // -1 represents as a invalid bit
    for (int i = 0; i < NUMBER_OF_FRAME; i++)
        freeFrame[i] = 1;   
    for (int i = 0; i < SIZE_OF_PAGE_TABLE; i++) 
        uptime[i] = 0;
}

int handlePageFault(FILE* backingStoreFile, int pageNumber, int offset)
{
    numberOfPageFault += 1;
    for (int i = 0; i < NUMBER_OF_FRAME; i++)
    {
        if (freeFrame[i]) 
        {
            // Add data
            if (fseek(backingStoreFile, pageNumber * 256, SEEK_SET) != 0)
            {
                printf("Error while seeking backing store (free frame)");
                exit(1);
            }
            for (int j = 0; j < FRAME_SIZE; j++)
                fread(&physicalMemory[i][j], 1, 1, backingStoreFile);
            
            freeFrame[i] = 0;
            pageTable[pageNumber] = i;

            return i; // return frame number
        }
    }

    // LRU
    if (fseek(backingStoreFile, pageNumber * 256, SEEK_SET) != 0)
    {
        printf("Error while seeking backing store (LRU)");
        exit(1);
    }

    int victimIndex = 0, lru = 0;
    for (int i = 0; i < SIZE_OF_PAGE_TABLE; i++)
    {
        if (uptime[i] > lru)
        {
            lru = uptime[i];
            victimIndex = i;
        }
    }
    // Get the LRU page in pageTable, use the frame it maps to store our new demand
    int frameIndex = pageTable[victimIndex];
    pageTable[victimIndex] = -1;
    pageTable[pageNumber] = frameIndex;
    
    // Add data
    for (int j = 0; j < FRAME_SIZE; j++)
        fread(&physicalMemory[frameIndex][j], 1, 1, backingStoreFile);

    // Reset uptime
    uptime[victimIndex] = 0;

    // Update uptime
    for (int i = 0; i < SIZE_OF_PAGE_TABLE; i++)
        if (pageTable[i] != -1) uptime[i] += 1;   
    
    return frameIndex;
}

int main(int argc, char* argv[])
{
    initialize();
    int logicalAddress;

    // Open addresses.txt
    FILE* addressFile;
    addressFile = fopen(argv[1], "r");
    if (addressFile == NULL)
    {
        printf("Error while opening addresses file");
        exit(1);
    }

    // Open BACKING_STORE.bin
    FILE* backingStoreFile;
    backingStoreFile = fopen("BACKING_STORE.bin", "rb");
    if (backingStoreFile == NULL)
    {
        printf("Error while opening backing store");
        exit(1);
    }

    while (fscanf(addressFile, "%d", &logicalAddress) == 1)
    {
        hit = 0;
        int data, physicalAddress;
        // Extract offset and page number from virtual addresses
        int offset = logicalAddress & 0xff;
        int pageNumber = (logicalAddress & 0xffff) >> 8;
        // Check TLB

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
            int frameNumber = handlePageFault(backingStoreFile, pageNumber, offset);
            data = physicalMemory[frameNumber][offset];
            physicalAddress = frameNumber * FRAME_SIZE + offset;
            printf("Virtual address: %d Physical address: %d Value: %d\n", logicalAddress, physicalAddress, data);
        }
        addressCount += 1;
    }
    printf("Number of Translated Addresses = %d\n", addressCount);
    printf("Page Faults = %d\n", numberOfPageFault);
    printf("Page Fault Rate = %f\n", numberOfPageFault / addressCount);
    printf("TLB Hits = \n");
    printf("TLB Hit Rate = \n");
    fclose(addressFile);
    fclose(backingStoreFile);
}