## Introduction

Logsort is a novel in-place stable O(n log n) quicksort.  The algorithm uses O(log n) space, hence the name, which many consider to be in-place despite not being the optimal O(1).  Unlike well-known in-place stable quicksorts which are O(n log² n), Logsort is asymptotically optimal.

## Motivation

O(n log n) in-place stable sorting is a hard task to achieve for sorting algorithms.  Bubble Sort and Insertion Sort are stable and in-place but are suboptimal.  Efficient sorts, such as Quicksort and Heapsort, are in-place but are unstable.  

One class of sorting algorithms that achieve both in-place, stability, and efficiency are Block Sort (a.k.a. Block Merge Sorts), such as [Wikisort](https://github.com/BonzaiThePenguin/WikiSort) and [Grailsort](https://github.com/Mrrl/GrailSort), which are in-place merge sorts.  However, they are incredibly complicated and hard to implement.  In-place stable partitioning is also a rather obscure problem in sorting.  The papers that do provide solutions describe partitioning algorithms that are only of theoretical interest.

Logsort is a new sorting algorithm that aims to provide a simple and practical O(n log n) in-place stable sort implementation like alternatives such as Block Sort.  The algorithm also uses an in-place stable partition algorithm and borrows ideas from [Munro, et. al. Stable in situ sorting and minimum data movement](https://doi.org/10.1007/BF02017344) and [stable quicksort with O(sqrt n) size blocking](https://www.youtube.com/watch?v=_YTl2VJnQ4s).

## Algorithm

Like Wikisort and Grailsort, Logsort's external buffer size can be configured, given it's at least 32 or Ω(log n).

## Acknowledgements

Special thanks to Sam Walko (Anonymous0726) for the block grouping algorithm, [Distray](https://github.com/distay0xGit) for revising the block encoding algorithm, and [Scandum](https://github.com/scandum) for the pivot selection algorithms.
