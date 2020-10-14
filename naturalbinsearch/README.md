

# Natural Restriction of search range
Natural Restriction of search range in strictly order integer sequences

Author: Fabio Sassi

## Abstract

Binary search algorithm works on sorted arrays as a whole.

It is possible to demonstrate that for a strictly sorted
integer sequence, under some conditions, the search extension 
can be limited to a smaller range.



## Concept
Let us consider a book with page numbers printed on all pages.
First page is marked as '1' while the last one is '100'.
Printed page numbers are a strictly sorted integer sequence. If
we remove some pages at random, the page numbers are still a strictly
sorted integer sequence.

If we want to know if page marked as '10' still exists we can limit
search at the first 10 pages (it cannot be over
the 10th page).

In the same way if we want to know the position of page marked as
'90' we have to search only in the last 10 pages.

This idea can be used to limit the search extension in
algorithms like binary search.


## Natural Restriction 

Let us consider the problem to find an integer number `q` in a sequence of
**strictly** (without duplicate) sorted integer:


    x0, x1, x2, ... xk       with  xj > xi for each j > i


If exists an `xf` that satisfy:


    xf = q


then `f` is the position of `q` inside the sequence and also the search result.


From the strictly-sorted ipotesis `xj > xi for each j > i` and 
the integer nature of index `i` and `j` follow that:


    xj >= xi + 1              for each j > i


This inequality is also true for `j = i + 1` because `j = i+1 > i`, so:


    x(i+1) >= x(i) + 1


iterating this approach:


    x(i+2) >= x(i+1) + 1 >= x(i) + 2

    x(i+3) >= x(i+2) + 1 >= x(i+1) + 2 >= x(i) + 3

    ...

It is simple to demonstrate for induction that:


    x(i+m) >= xi + m          where m is positive integer      (0)


This inequality can be used to limit the search extension. 


### Lower limit

If the first value `x0` is know, from (0) follow that:


    xj >= x0 + j


if exists an index `f` that satisfy `xf = q` then:


    q >= x0 + f


or focusing on `f`:


    f <= q - x0                                                      (1)


This inequality say that if `q` exists in the sequence, its index `f` 
cannot be over `q - x0`.

### Upper limit

If last value of the sequence `xk` is know, from inequality (0)
with `(i+m) = k` follow that:


    xk >= xi + (k - i) 


rewritten for `i`:


    i >= xi - xk + k


if exists an index `f` that satisfy `xf = q` then:


    f >= q - xk + k                                                  (2)


So if `q` exists in sequence its index `f` must be equal or greater than
`q - xk + k`


### Limit conditions

These two inequalities (1)(2) can limit the search extension only
under some conditions. For example if `q` is equal to `x0 + k`
then from (1):


    f  <= q - x0 = x0 + k - x0   =>    f <= k


This index bound is meaningless because `f <= k` is always true.
In the same way if `q` is equal to `xk  - k` then from (2):


    q - xk = -k   =>    f >= 0


Also this is meaningless because `f` is a positive index.


#### Lower limit condition 


Inequality (1) can limit the search extensions only if:


    f <= q - x0 < k


So (1) can be used to bound search index when:


    q < x0 + k                                                       (C1)


#### Upper limit condition 

Inequality (2) can limit the search extensions only if:


    f >= q - xk + k > 0 

So (2) can be used to bound search index when:

    q > xk - k                                                       (C2)



#### Upper and Lower conditions

The two conditions (C1) and (C2) can coexists? That's to say, can exists a
number `q` that satisfy:


    xk - k < q < x0 + k                                              (C12)


This is possible only if:


    xk - k  <  x0 + k


or simplified as:


    xk - x0 < 2k                                                     (H12)



Under this prerequisite exists a region where (C1) and (C2)
are both true.

If exists an index `f` that satisfy `xf = q` where `q` fullfill (C12) 
then (1) and (2) can be used together:


    q - xk + k <= f <= q - x0                                        (12)


Under (H12) prerequisite (C1), (C2) and (C12) can be represented as:

         C1        C12       C2
    |\\\\\\\\\\\|XXXXXXX|///////////|
    x0         xk-k   x0+k          xk


instead if (H12) is not true:

         C1                  C2
    |\\\\\\\\\\\|.......|///////////|
    x0         x0+k   xk-k          xk


Where:

    '\'    bound by (1)
    '/'    bound by (2)
    'X'    bound by (1) and (2)



#### Compactness factor

It is interesting to note that (H12) talks about the compactness of the
sequence. Defining a compactness factor as:


    cf = k / (xk - x0)


the prerequisite (H12) can be written as:


    cf > 1/2


If the sequence is complete 


    xi = i


then


    cf = k / (xk - x0) = k / (k - 0) = 1



The compacteness factor `cf` can assume any value less or equal than 1

    
    cf <= 1


The greater `cf` is, the greater can be the bound effect
and so the efficency. 



### Conclusion

Under (C1) and/or (C2) is possible to restrict search index by a lower
and/or upper bound.

These restrictions can be summarized as:


- lower bound: if  `q < x0 + k`  then  `0 <= f <= q - x0`


- upper bound: if  `q > xk - k`  then  `q - xk + k <= f <= k`


- lower and upper bound: if `xk - k < q < x0 + k`  then 
  `q - xk + k <= f <= q - x0` (this only goes when `xk - x0 < 2k`)


