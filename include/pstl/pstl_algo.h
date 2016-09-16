// -*- C++ -*-
/*
 *
 * Copyright (c) 1994
 * Hewlett-Packard Company
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.  Hewlett-Packard Company makes no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 *
 * Copyright (c) 1996,1997
 * Silicon Graphics Computer Systems, Inc.
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.  Silicon Graphics makes no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 * Copyright (c) 2005
 * Takashi Tsunakawa, Kenta Oouchida and Takashi Ninomiya
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.  Takashi Tsunakawa, Kenta Oouchida and
 * Takashi Ninomiya make no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 */

#ifndef __SGI_PSTL_ALGO_H
#define __SGI_PSTL_ALGO_H

#include "pstl_algobase.h"
#include "pstl_tempbuf.h"
#include "_pstl_algo.h"
#include "_pstl_numeric.h"

#ifdef __PSTL_USE_NAMESPACES

// Names from "_pstl_algo.h"
using pstl::for_each; 
using pstl::find; 
using pstl::find_if; 
using pstl::adjacent_find; 
using pstl::count; 
using pstl::count_if; 
using pstl::search; 
using pstl::search_n; 
using pstl::swap_ranges; 
using pstl::transform; 
using pstl::replace; 
using pstl::replace_if; 
using pstl::replace_copy; 
using pstl::replace_copy_if; 
using pstl::generate; 
using pstl::generate_n; 
using pstl::remove; 
using pstl::remove_if; 
using pstl::remove_copy; 
using pstl::remove_copy_if; 
using pstl::unique; 
using pstl::unique_copy; 
using pstl::reverse; 
using pstl::reverse_copy; 
using pstl::rotate; 
using pstl::rotate_copy; 
using pstl::random_shuffle; 
using pstl::random_sample; 
using pstl::random_sample_n; 
using pstl::partition; 
using pstl::stable_partition; 
using pstl::sort; 
using pstl::stable_sort; 
using pstl::partial_sort; 
using pstl::partial_sort_copy; 
using pstl::nth_element; 
using pstl::lower_bound; 
using pstl::upper_bound; 
using pstl::equal_range; 
using pstl::binary_search; 
using pstl::merge; 
using pstl::inplace_merge; 
using pstl::includes; 
using pstl::set_union; 
using pstl::set_intersection; 
using pstl::set_difference; 
using pstl::set_symmetric_difference; 
using pstl::min_element; 
using pstl::max_element; 
using pstl::next_permutation; 
using pstl::prev_permutation; 
using pstl::find_first_of; 
using pstl::find_end; 
using pstl::is_sorted; 
using pstl::is_heap; 

// Names from pstl_heap.h
using pstl::push_heap;
using pstl::pop_heap;
using pstl::make_heap;
using pstl::sort_heap;

// Names from "_pstl_numeric.h"
using pstl::accumulate; 
using pstl::inner_product; 
using pstl::partial_sum; 
using pstl::adjacent_difference; 
using pstl::power; 
using pstl::iota; 

#endif /* __PSTL_USE_NAMESPACES */

#endif /* __SGI_PSTL_ALGO_H */

// Local Variables:
// mode:C++
// End:
