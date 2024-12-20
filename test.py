import random

def generate_addresses(num_addresses, page_count, working_set_size, physical_pages):
    """
    Generates addresses to favor LRU over FIFO.
    
    Parameters:
    - num_addresses: Total number of virtual addresses to generate.
    - page_count: Total number of unique virtual pages.
    - working_set_size: Number of pages in the working set at any time.
    - physical_pages: Number of physical pages available.
    
    Returns:
    - List of logical addresses.
    """
    addresses = []
    working_set = list(range(working_set_size))  # Start with a working set
    remaining_pages = list(range(working_set_size, page_count))
    
    for i in range(num_addresses):
        # Simulate locality of reference: 70% from working set
        if random.random() < 0.7:
            page = random.choice(working_set)
        else:
            # Access a new or less-used page (favor FIFO eviction)
            if remaining_pages:
                page = remaining_pages.pop(0)
                working_set.append(page)
                if len(working_set) > physical_pages:
                    working_set.pop(0)  # FIFO eviction in the working set
            else:
                page = random.randint(0, page_count - 1)
        
        # Generate a full logical address by appending a random offset
        offset = random.randint(0, 255)  # 8-bit offset
        logical_address = (page << 8) | offset
        addresses.append(logical_address)
    
    return addresses

# Parameters
num_addresses = 20000
page_count = 256  # Virtual pages
working_set_size = 16  # Simulated working set size
physical_pages = 128  # Number of physical pages

# Generate addresses
addresses = generate_addresses(num_addresses, page_count, working_set_size, physical_pages)

# Write to file
with open("test_addresses.txt", "w") as f:
    for addr in addresses:
        f.write(f"{addr}\n")
