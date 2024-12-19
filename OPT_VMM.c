#include <stdio.h>
#include <stdlib.h>

#define SIZE_OF_TLB 16
#define SIZE_OF_PAGE_TABLE 256
#define NUMBER_OF_FRAME 128
#define FRAME_SIZE 256
#define NUM_ADDRESS 40000

int addressCount = 0;
int numberOfPageFault = 0;
int TLBhit;
int TLBsum;
int hit;
int TLB[SIZE_OF_TLB][2];
int pageTable[SIZE_OF_PAGE_TABLE];
int freeFrame[NUMBER_OF_FRAME];
int8_t physicalMemory[NUMBER_OF_FRAME][FRAME_SIZE];
int freeIndex = 0;
int freeIndexTLB = 0;
int arrLogicalAddress[NUM_ADDRESS], cntLogicalAddress;

void initialize()
{
    for (int i = 0; i < SIZE_OF_PAGE_TABLE; i++) 
        pageTable[i] = -1; // -1 represents as a invalid bit
    for (int i = 0; i < NUMBER_OF_FRAME; i++)
        freeFrame[i] = 1;
    for (int i = 0; i < SIZE_OF_TLB; i++) {
        TLB[i][0] = -1;
    }
}

int handlePageFault(FILE* backingStoreFile, int pageNumber, int offset, int start)
{
    for (int i = 0; i < NUMBER_OF_FRAME; i++)
    {
        if (freeFrame[i] == 1) 
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

    // OPT
    if (fseek(backingStoreFile, pageNumber * 256, SEEK_SET) != 0)
    {
        printf("Error while seeking backing store (OPT)");
        exit(1);
    }

    // Get the OPT page in pageTable, use the frame it maps to store our new demand
    int victimIndex = 0, opt = 0;
    printf("Start: %d\n", start);
    for (int i = 0; i < SIZE_OF_PAGE_TABLE; i++)
    {
        if (pageTable[i] != -1)
        {
            int found = 0;
            for (int j = start + 1; j < cntLogicalAddress; j++) {
                if (arrLogicalAddress[j] == i) {
                    if (opt < j - start) {
                        opt = j - start;
                        victimIndex = i;
                    }
                    found = 1;
                    break;
                } 
            }
            if (!found) { 
                victimIndex = i; 
                break;
            }
        }
    }
    // printf("victimIndex: %d\n", victimIndex);
    int frameIndex = pageTable[victimIndex];
    pageTable[victimIndex] = -1;
    pageTable[pageNumber] = frameIndex;
    
    // Add data
    for (int j = 0; j < FRAME_SIZE; j++)
        fread(&physicalMemory[frameIndex][j], 1, 1, backingStoreFile);

    return frameIndex;
}

int checkTLB(int pageNumber, int offset, int logicalAddress, int data)
{
    TLBsum++;
    for(int i = 0; i < SIZE_OF_TLB ; i++)
        if( TLB[i][0] == pageNumber ) {
            int physicalAddress = TLB[i][1] * FRAME_SIZE + offset;
            printf("Virtual address: %d Physical address: %d Value: %d\n", logicalAddress, physicalAddress, data);
            return 1;
        }
    return 0;
}

void updateTLB(int pageNumber,int frameNumber)
{
    TLBhit++;
    TLB[freeIndexTLB][0] = pageNumber;
    TLB[freeIndexTLB][1] = frameNumber;
    freeIndexTLB = (freeIndexTLB + 1) % SIZE_OF_TLB;
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

    cntLogicalAddress = 0;
    while (fscanf(addressFile, "%d", &logicalAddress) == 1) {
        arrLogicalAddress[cntLogicalAddress++] = logicalAddress;
    }

    for (int i = 0; i < cntLogicalAddress; i++) {
        logicalAddress = arrLogicalAddress[i];
        hit = 0;
        int data, physicalAddress;
        // Extract offset and page number from virtual addresses
        int offset = logicalAddress & 0xff;
        int pageNumber = (logicalAddress & 0xffff) >> 8; 

        // Check TLB
        hit = checkTLB(pageNumber, offset, logicalAddress, data);

        // If not found, check pageTable
        if (!hit)
        {
            if (pageTable[pageNumber] != -1)
            {
                updateTLB(pageNumber,pageTable[pageNumber]);
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
            int frameNumber = handlePageFault(backingStoreFile, pageNumber, offset, i);
            updateTLB(pageNumber,pageTable[pageNumber]);
            data = physicalMemory[frameNumber][offset];
            physicalAddress = frameNumber * FRAME_SIZE + offset;
            // printf("Huhu: %d Huhu: %d Huhu: %d\n", frameNumber, FRAME_SIZE, offset);
            printf("Virtual address: %d Physical address: %d Value: %d\n", logicalAddress, physicalAddress, data);
        }
        addressCount += 1;
    }

    printf("Number of Translated Addresses = %d\n", addressCount);
    printf("Page Faults = %d\n", numberOfPageFault);
    printf("Page Fault Rate = %.3f\n", (float) numberOfPageFault / addressCount);
    printf("TLB Hits = %d\n",( TLBsum - TLBhit ));
    printf("TLB Hit Rate = %.3f\n",1.0*( TLBsum-TLBhit ) / TLBsum);
    fclose(addressFile);
    fclose(backingStoreFile);
}
