## xv6 Memory Management

### GWU CSCI 3411 - Fall 2020 - Lab 9

---

## Topics

1.  Introduction

2.  Analysis

3.  Exercise

# 1. Introduction

This week we study XV6 memory management. We will focus on how `malloc` and `free` make and maintain memory allocations in user space.

You have been provided a modified version of `umalloc.c`. This modified `umalloc.c` has the same implementation as the default XV6 distribution; however, the variables and structures have been changed in an attempt to make the logic more clear.

# 2. Analysis

Analyze the modified `umalloc.c` and answer the following questions:

## Questions

1.  How is the size of an allocation computed in bytes?

2.  How does the freelist work?

3.  What policy is it using?

4.  If the freelist does not have enough space currently available for a requested allocation, what is the minimum size of memory allocated by the system in bytes?

5.  What operations are performed on `free`?

6.  What operations are performed on `malloc`?

7.  What is the main loop doing in `malloc`?

# 3. Exercise - Best Fit Memory Allocation

Modify `umalloc.c` to use a “Best Fit” algorithm rather than the default XV6 memory allocation policy. This implementation should scan through the freelist and locate the empty memory region that minimizes the difference between the region size and the allocation request size. If there is no available region in the freelist suitable for the allocation request, the algorithm requests a new page.

The `lab9test.c` program has been provided to test your implementation. `lab9test` carries out two tests: verification of best fit into a fragmented page of memory, and verification of multiple pages allocated by a single call to `malloc`. The test file includes the define `VERBOSE` to control whether detailed information is printed to the console during the tests. Set `VERBOSE` equal to `1` in order to visualize the test data.