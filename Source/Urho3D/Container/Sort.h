//
// Copyright (c) 2008-2019 the Urho3D project.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#pragma once

#include "../Container/Swap.h"
#include "../Container/VectorBase.h"

namespace Urho3D
{

static const int QUICKSORT_THRESHOLD = 16;

// Based on Comparison of several sorting algorithms by Juha Nieminen
// http://warp.povusers.org/SortComparison/

/// Perform insertion sort on an array.
template <class T> void InsertionSort(RandomAccessIterator<T> begin, RandomAccessIterator<T> end)
{
    for (RandomAccessIterator<T> i = begin + 1; i < end; ++i)
    {
        T temp = *i;
        RandomAccessIterator<T> j = i;
        while (j > begin && temp < *(j - 1))
        {
            *j = *(j - 1);
            --j;
        }
        *j = temp;
    }
}

/// Perform insertion sort on an array using a compare function.
// 冒泡排序：从第二个元素开始遍历，每个元数都与前面的所有元数比较，移动到合适的位置。
template <class T, class U> void InsertionSort(RandomAccessIterator<T> begin, RandomAccessIterator<T> end, U compare)
{
    for (RandomAccessIterator<T> i = begin + 1; i < end; ++i)
    {
        T temp = *i;
        RandomAccessIterator<T> j = i;
        while (j > begin && compare(temp, *(j - 1)))
        {
            *j = *(j - 1);
            --j;
        }
        *j = temp;
    }
}

/// Perform quick sort initial pass on an array. Does not sort fully.
template <class T> void InitialQuickSort(RandomAccessIterator<T> begin, RandomAccessIterator<T> end)
{
    while (end - begin > QUICKSORT_THRESHOLD)
    {
        // Choose the pivot by median
        RandomAccessIterator<T> pivot = begin + ((end - begin) / 2);
        if (*begin < *pivot && *(end - 1) < *begin)
            pivot = begin;
        else if (*(end - 1) < *pivot && *begin < *(end - 1))
            pivot = end - 1;

        // Partition and sort recursively
        RandomAccessIterator<T> i = begin - 1;
        RandomAccessIterator<T> j = end;
        T pivotValue = *pivot;
        for (;;)
        {
            while (pivotValue < *(--j));
            while (*(++i) < pivotValue);
            if (i < j)
                Swap(*i, *j);
            else
                break;
        }

        InitialQuickSort(begin, j + 1);
        begin = j + 1;
    }
}

/// Perform quick sort initial pass on an array using a compare function. Does not sort fully.
// 二分排序（元素个数小于QUICKSORT_THRESHOLD时使用）：选个中值，将数组分两段，比中值小的交换到一边，大的交换到另一边，然后再对两段迭代。
template <class T, class U> void InitialQuickSort(RandomAccessIterator<T> begin, RandomAccessIterator<T> end, U compare)
{
    while (end - begin > QUICKSORT_THRESHOLD)
    {
        // Choose the pivot by median
        RandomAccessIterator<T> pivot = begin + ((end - begin) / 2); // 设为数组中间位置元素
        if (compare(*begin, *pivot) && compare(*(end - 1), *begin)) // 设为数组开始位置元素
            pivot = begin;
        else if (compare(*(end - 1), *pivot) && compare(*begin, *(end - 1))) // 设为数组末尾位置元素
            pivot = end - 1;

        // Partition and sort recursively
        RandomAccessIterator<T> i = begin - 1;
        RandomAccessIterator<T> j = end;
        T pivotValue = *pivot;
        for (;;)
        {
            while (compare(pivotValue, *(--j)));
            while (compare(*(++i), pivotValue));
            if (i < j)
                Swap(*i, *j);
            else
                break;
        }

        InitialQuickSort(begin, j + 1, compare);
        begin = j + 1;
    }
}

/// Sort in ascending order using quicksort for initial passes, then an insertion sort to finalize.
template <class T> void Sort(RandomAccessIterator<T> begin, RandomAccessIterator<T> end)
{
    InitialQuickSort(begin, end);
    InsertionSort(begin, end);
}

// compare(lhs, rhs)，如果返回值为true，则lhs（Left Hand Side）放在rhs（Right Hand Side）的左侧；如果返回值为false，则相反
/// Sort in ascending order using quicksort for initial passes, then an insertion sort to finalize, using a compare function.
// 使用冒泡法排序（元数个数小于QUICKSORT_THRESHOLD时，先进行二分法排序）
template <class T, class U> void Sort(RandomAccessIterator<T> begin, RandomAccessIterator<T> end, U compare)
{
    InitialQuickSort(begin, end, compare);
    InsertionSort(begin, end, compare);
}

// https://blog.csdn.net/weixin_39540045/article/details/80499817

}
