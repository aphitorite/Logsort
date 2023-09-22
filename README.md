## Introduction

Logsort is a novel in-place stable O(n log n) quicksort.  The algorithm uses O(log n) space, hence the name, which many consider to be in-place despite not being the optimal O(1).  Unlike well-known in-place stable quicksorts which are O(n log² n), Logsort is asymptotically optimal.

## Motivation

O(n log n) in-place stable sorting is a hard task to achieve for sorting algorithms.  Bubble Sort and Insertion Sort are stable and in-place but suboptimal.  Efficient sorts, such as Quicksort and Heapsort, are in-place and O(n log n) but unstable.  

One class of sorting algorithms that achieve both in-place, stability, and O(n log n) time is Block Sort (a.k.a. Block Merge Sorts), such as [Wikisort](https://github.com/BonzaiThePenguin/WikiSort) and [Grailsort](https://github.com/Mrrl/GrailSort), which are in-place merge sorts.  However, they are incredibly complicated and hard to implement.  In addition, in-place stable partitioning is a rather obscure problem in sorting.  The papers that do provide solutions describe partitioning algorithms that are only of theoretical interest.

Logsort is a new sorting algorithm that aims to provide a simple and practical O(n log n) in-place stable sort implementation like alternatives such as Block Sort.  The algorithm uses a novel in-place stable partitioning algorithm that borrows ideas from [Munro, et. al. Stable in situ sorting and minimum data movement](https://doi.org/10.1007/BF02017344) and [Aeos Quicksort](https://www.youtube.com/watch?v=_YTl2VJnQ4s) (stable quicksort with O(sqrt n) size blocking).

## Algorithm

Partitioning is analogous to sorting an array of 0's and 1's, where elements smaller than the pivot are 0 and elements larger are 1.  Logsort sorts 0's and 1's stably in O(n) time and O(log n) space via its partition.

The partitioning algorithm is divided into four phases:
1. Grouping elements into blocks
2. Bit encoding the blocks
3. Swapping the blocks
4. Sorting the blocks (+ cleanup)

### Grouping phase

Given an unordered list of 0's and 1's, we group them into blocks of a fixed size where each block contains either only 0's or 1's.  Given two buckets of B extra space, one can easily group blocks of size B:

```
B = 2
        ↓ move to ones bucket
array: [1, 0, 1, 1, 0, 0, 1, 1]
zeros: [ ,  ]
 ones: [ ,  ]
 
           ↓ move to zeros bucket
array: [ , 0, 1, 1, 0, 0, 1, 1]
zeros: [ ,  ]
 ones: [1,  ]
 
              ↓ move to ones bucket
array: [ ,  , 1, 1, 0, 0, 1, 1]
zeros: [0,  ]
 ones: [1,  ]
 
array: [ ,  ,  , 1, 0, 0, 1, 1]      
zeros: [0,  ]
 ones: [1, 1] ← ones bucket full: output back into array

        ↓  ↓ first block created: continue looping
array: [1, 1,  , 1, 0, 0, 1, 1]
zeros: [0,  ]
 ones: [ ,  ] 
 ```
It's likely we end up with partially filled buckets at the end of this phase.  In that case, we output the 0 elements followed by the 1's at the end of the array.

```
                          ↓ output zeros
array: [1, 1, 0, 0, 1, 1,  ,  ]
zeros: [0,  ]
 ones: [1,  ]

                             ↓ output ones
array: [1, 1, 0, 0, 1, 1, 0,  ]
zeros: [ ,  ]
 ones: [1,  ]


array: [1, 1, 0, 0, 1, 1, 0, 1]

       |---- blocks ----| ↓ leftover zeros
       [1, 1][0, 0][1, 1][0][1]
```

Logsort's O(log n) space usage comes from grouping blocks of size O(log n), we'll see why this is important later.

In the actual implementation, Logsort uses a space optimization from Aeos Quicksort which only needs one bucket instead of two.  Like Wikisort and Grailsort, Logsort's external buffer size can be configured, given it's at least Ω(log n).

> Since each element is moved twice, once to the bucket and once back to the array, the grouping phase is O(n) regardless of block size but requires O(block size) extra space.

### Bit encoding

0's and 1's can also be concatenated to make binary numbers.  Since Logsort's blocks are O(log n) in size, we can encode numbers, with a range of O(n), in blocks by swapping elements between 0 blocks and 1 blocks.  Decoding a number in a block requires a scan of the block which costs O(log n) comparisons.

```
Encode 11 = 0b1101:

[0, 0, 0, 0, 0] [1, 1, 1, 1, 1]
 └───────────────┘
    └───────────────┘          ← swap the following
          └───────────────┘
          
|--- 11 ---|    |-- ~11 ---|
[1, 1, 0, 1, 0] [0, 0, 1, 0, 1]
             ↑               ↑ 
             reserved last bit determines 0 or 1 block
```

Using this, the encoding phase encodes every pair of 0 and 1 block with a unique index.  We maintain two iterators: one for 0 blocks and 1 blocks respectively:

```
Encode blocks:

 (0)  (0)
[ 0 ][ 1 ][ 1 ][ 1 ][ 0 ][ 1 ][ 0 ][ 1 ]
└───┘└───┘ encode 0

 (0)  (0)  (1)       (1)
[ 0 ][ 1 ][ 1 ][ 1 ][ 0 ][ 1 ][ 0 ][ 1 ]
          └───┘     └───┘ encode 1

 (0)  (0)  (1)  (2)  (1)       (2)
[ 0 ][ 1 ][ 1 ][ 1 ][ 0 ][ 1 ][ 0 ][ 1 ]
               └───┘          └───┘ encode 2 and finish
```

In this example, there were fewer 0 blocks, so all 0 blocks get encoded leaving some 1 blocks untouched.  These leftover blocks will be dealt with in the swapping phase.

> Encoding an index costs O(log n) swaps.  Since each block is size O(log n), there are O(n/log n) blocks, so this phase is n/log n \* log n = O(n).

### Sorting the blocks

```
for each index from 0 to blocks.count - 1:
	current = blocks[index].decode()
	while current != index:
		block swap blocks[current] and blocks[index]
		current = blocks[index].decode()
```

## Conclusion

## Acknowledgements

Special thanks to Sam Walko (Anonymous0726) for providing Aeos Quicksort as a reference, [Distray](https://github.com/distay0xGit) for revising the block encoding algorithm, and [Scandum](https://github.com/scandum) for the pivot selection logic from [Fluxsort](https://github.com/scandum/fluxsort).
