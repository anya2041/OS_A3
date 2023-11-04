#include "mems.h"

// Define PAGE_SIZE
#define PAGE_SIZE 4096

// Define data structures
struct SubChainNode {
    size_t size;  // Size of the segment
    int type;     // 0 for HOLE, 1 for PROCESS
    void* memsVirtualAddress;  // MeMS virtual address corresponding to this segment
    struct SubChainNode* next;
    struct SubChainNode* prev;
};

struct MainChainNode {
    struct SubChainNode* subChain;
    struct MainChainNode* next;
    struct MainChainNode* prev;
};

// Head of the free list
struct MainChainNode* freeListHead = NULL;

// Starting MeMS virtual address
void* memsVirtualAddress = NULL;

// Function to find a suitable HOLE segment in the free list
struct SubChainNode* findSuitableHole(size_t size) {
    struct MainChainNode* currentNode = freeListHead;
    while (currentNode != NULL) {
        struct SubChainNode* currentSubChainNode = currentNode->subChain;
        while (currentSubChainNode != NULL) {
            if (currentSubChainNode->type == 0 && currentSubChainNode->size >= size) {
                return currentSubChainNode;  // Found a suitable HOLE segment
            }
            currentSubChainNode = currentSubChainNode->next;
        }
        currentNode = currentNode->next;
    }
    return NULL;  // No suitable HOLE segment found
}

// Function to split a HOLE segment into two parts: one for allocation and the other for creating a new HOLE
void splitHole(struct SubChainNode* holeSegment, size_t allocationSize) {
    if (holeSegment->size > allocationSize) {
        // Create a new HOLE segment with the remaining size
        struct SubChainNode* newHoleSegment = (struct SubChainNode*)malloc(sizeof(struct SubChainNode));
        newHoleSegment->size = holeSegment->size - allocationSize;
        newHoleSegment->type = 0;
        newHoleSegment->next = holeSegment->next;
        newHoleSegment->prev = holeSegment;

        if (holeSegment->next != NULL) {
            holeSegment->next->prev = newHoleSegment;
        }

        holeSegment->next = newHoleSegment;
        holeSegment->size = allocationSize;
    }
}

// Function to allocate more memory using mmap and update the free list
void allocateMoreMemory(size_t size) {
    // Use mmap to allocate more memory for MeMS
    void* newMemsVirtualAddress = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    // Update the free list with a new MainChainNode and SubChainNode
    struct MainChainNode* newNode = (struct MainChainNode*)malloc(sizeof(struct MainChainNode));
    newNode->subChain = (struct SubChainNode*)malloc(sizeof(struct SubChainNode));
    newNode->subChain->size = size;
    newNode->subChain->type = 0;  // New segment is a HOLE
    newNode->subChain->memsVirtualAddress = newMemsVirtualAddress;
    newNode->subChain->next = NULL;
    newNode->subChain->prev = NULL;

    newNode->next = freeListHead;
    newNode->prev = NULL;

    if (freeListHead != NULL) {
        freeListHead->prev = newNode;
    }

    freeListHead = newNode;
}

void mems_init() {
    // Use mmap to allocate initial memory for MeMS
    memsVirtualAddress = mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    // Initialize the head of the free list
    freeListHead = (struct MainChainNode*)malloc(sizeof(struct MainChainNode));
    freeListHead->subChain = (struct SubChainNode*)malloc(sizeof(struct SubChainNode));
    freeListHead->subChain->size = PAGE_SIZE;
    freeListHead->subChain->type = 0;  // Initial segment is a HOLE
    freeListHead->subChain->memsVirtualAddress = memsVirtualAddress;
    freeListHead->subChain->next = NULL;
    freeListHead->subChain->prev = NULL;

    freeListHead->next = NULL;
    freeListHead->prev = NULL;
}

void mems_finish() {
    // Unmap the allocated memory using munmap
    if (memsVirtualAddress != NULL) {
        munmap(memsVirtualAddress, PAGE_SIZE);
    }

    // Free any dynamically allocated memory for the free list
    struct MainChainNode* currentNode = freeListHead;
    while (currentNode != NULL) {
        struct MainChainNode* nextNode = currentNode->next;
        free(currentNode->subChain);
        free(currentNode);
        currentNode = nextNode;
    }

    // Additional cleanup if needed
}

void* mems_malloc(size_t size) {
    // Find a suitable HOLE segment in the free list
    struct SubChainNode* holeSegment = findSuitableHole(size);

    if (holeSegment != NULL) {
        // Allocate memory from the existing HOLE segment
        holeSegment->type = 1;  // Update the segment's type to PROCESS
        splitHole(holeSegment, size);  // Split the HOLE segment if needed

        // Return MeMS virtual address
        return holeSegment->memsVirtualAddress;
    } else {
        // Allocate more memory using mmap
        size_t allocationSize = size > PAGE_SIZE ? size : PAGE_SIZE;
        allocateMoreMemory(allocationSize);

        // Call mems_malloc recursively to allocate from the newly added HOLE
        return mems_malloc(size);
    }
}

void mems_free(void* ptr) {
    // Find the corresponding sub-chain node in the free list based on the MeMS virtual address
    struct MainChainNode* currentNode = freeListHead;
    while (currentNode != NULL) {
        struct SubChainNode* currentSubChainNode = currentNode->subChain;
        while (currentSubChainNode != NULL) {
            if (currentSubChainNode->memsVirtualAddress == ptr) {
                // Mark the corresponding sub-chain node as HOLE
                currentSubChainNode->type = 0;

                // Merge adjacent HOLEs if needed
                // ...

                return;
            }
            currentSubChainNode = currentSubChainNode->next;
        }
        currentNode = currentNode->next;
    }
}

void mems_print_stats() {
    // Print total number of mapped pages, unused memory, details about each node in the main chain, and each segment in the sub-chain
    // ...

    // Print additional information if needed
}

void* mems_get(void* v_ptr) {
    // Return the MeMS physical address mapped to ptr (ptr is MeMS virtual address)
    // ...

    // Placeholder, replace with your implementation
    return v_ptr;
}
