/*
 * $Id: pvector.h,v 1.3 2007-09-02 09:34:39 ninomi Exp $
 *  Copyright (c) 2007, Tsujii Laboratory, The University of Tokyo.
 *  All rights reserved.
 *
 */

#ifndef __pvector_h
#define __pvector_h

#include <vector>

namespace up {

    template <class _X> class pvector_const_iterator;
    
    template <class _X> class pvector_iterator {
    public:
        typedef _X value_type;
        typedef _X& reference;
        typedef _X* pointer;
        typedef typename std::vector<value_type*>::difference_type difference_type;
        typedef typename std::vector<value_type*>::iterator::iterator_category iterator_category;
        typedef pvector_iterator iterator;
        typedef size_t size_type;

    protected:
        typename std::vector<value_type*>::iterator it;
        
    public:
        pvector_iterator(typename std::vector<value_type*>::iterator __it) : it(__it) {}
        pvector_iterator(const pvector_iterator& pit) : it(pit.it) {}
        reference operator*() const { return **it; }
        pointer operator->() const { return *it; }
        iterator& operator++() { ++it; return *this; }
        iterator operator++(int) { return iterator(it++); }
        iterator& operator--() { --it;  return *this; }
        iterator operator--(int) { return iterator(it--); }
        reference operator[](const difference_type& __n) const { return *(it[__n]); }
        iterator& operator+=(const difference_type& __n) { it += __n; return *this; }
        iterator operator+(const difference_type& __n) const { return iterator(it + __n); }
        iterator& operator-=(const difference_type& __n) { it -= __n; return *this; }
        iterator operator-(const difference_type& __n) const { return iterator(it - __n); }
        typename std::vector<value_type*>::iterator base() const { return it; }
        friend class pvector_const_iterator<_X>;
    };

    template <class _X>
    inline bool operator==(const pvector_iterator<_X>& a, const pvector_iterator<_X>& b) {
        return a.base() == b.base();
    }
    template <class _X>
    inline bool operator!=(const pvector_iterator<_X>& a, const pvector_iterator<_X>& b) {
        return a.base() != b.base();
    }
        
    template <class _X> class pvector_const_iterator {
    public:
        typedef _X value_type;
        typedef _X& reference;
        typedef const _X & const_reference;
        typedef _X* pointer;
        typedef typename std::vector<value_type*>::difference_type difference_type;
        typedef typename std::vector<value_type*>::iterator::iterator_category iterator_category;
        typedef pvector_const_iterator<_X> const_iterator;
        typedef size_t size_type;

    protected:
        typename std::vector<value_type*>::const_iterator it;
        
    public:
        pvector_const_iterator(typename std::vector<value_type*>::const_iterator __it) : it(__it) {}
        pvector_const_iterator(const pvector_const_iterator<_X>& pit) : it(pit.it) {}
        pvector_const_iterator(const pvector_iterator<_X>& pit) : it(pit.it) {}
        reference operator*() const { return **it; }
        pointer operator->() const { return *it; }
        const_iterator& operator++() { ++it; return *this; }
        const_iterator operator++(int) { return const_iterator(it++); }
        const_iterator& operator--() { --it;  return *this; }
        const_iterator operator--(int) { return const_iterator(it--); }
        const_reference operator[](const difference_type& __n) const { return *(it[__n]); }
        const_iterator& operator+=(const difference_type& __n) { it += __n; return *this; }
        const_iterator operator+(const difference_type& __n) const { return const_iterator(it + __n); }
        const_iterator& operator-=(const difference_type& __n) { it -= __n; return *this; }
        const_iterator operator-(const difference_type& __n) const { return const_iterator(it - __n); }
        typename std::vector<value_type*>::const_iterator base() const { return it; }
    };

    template <class _X>
    inline bool operator==(const pvector_const_iterator<_X>& a, const pvector_const_iterator<_X>& b) {
        return a.base() == b.base();
    }
    template <class _X>
    inline bool operator!=(const pvector_const_iterator<_X>& a, const pvector_const_iterator<_X>& b) {
        return a.base() != b.base();
    }
        
    template <class _X> class pvector {
    public:
        typedef _X value_type;
        typedef _X& reference;
        typedef const _X& const_reference;
        typedef _X* pointer;
        typedef typename std::vector<value_type*>::difference_type difference_type;
        typedef pvector_iterator<value_type> iterator;
        typedef pvector_const_iterator<value_type> const_iterator;
        typedef std::reverse_iterator<iterator> reverse_iterator;
        typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
        typedef size_t size_type;

    protected:
        std::vector<value_type*> vec;
        
    public:
        pvector() : vec() {}
        pvector(size_type __n) {
            for(size_type i = 0 ; i < __n ; i++)
                vec.push_back(new value_type());
        }
        pvector(size_type __n, const value_type& __value) {
            for(size_type i = 0 ; i < __n ; i++)
                vec.push_back(new value_type(__value));
        }
        pvector(const pvector& __x) {
            const_iterator i = __x.begin(), last = __x.end();
            for(; i != last ; i++)
                vec.push_back(new value_type(*i));
        }
        ~pvector() {
            typename std::vector<value_type*>::iterator i = vec.begin(), last = vec.end();
            for(; i != last ; i++)
                delete *i;
        }
        pvector& operator=(const pvector& __x) {
            if(&__x != this) {
                clear();
                const_iterator i = __x.begin(), last = __x.end();
                for(; i != last ; i++)
                    vec.push_back(new value_type(*i));
            }
            return *this;
        }
        iterator begin() { return iterator(vec.begin()); }
        const_iterator begin() const { return const_iterator(vec.begin()); }
        iterator end() { return iterator(vec.end()); }
        const_iterator end() const { return const_iterator(vec.end()); }
        reverse_iterator rbegin() { return reverse_iterator(vec.rbegin()); }
        const_reverse_iterator rbegin() const { return const_reverse_iterator(vec.rbegin()); }
        reverse_iterator rend() { return reverse_iterator(vec.rend()); }
        const_reverse_iterator rend() const { return const_reverse_iterator(vec.rend()); }
        size_type size() const { return vec.size(); }
        size_type max_size() const { return vec.max_size(); }
        void resize(size_type __new_size, const value_type& __x) {
            if (__new_size < size())
                erase(begin() + __new_size, end());
            else
                insert(end(), __new_size - size(), __x);
        }
        void resize(size_type __new_size) { resize(__new_size, value_type()); }
        size_type capacity() const { return vec.capacity(); }
        bool empty() const { return vec.empty(); }
        void reserve(size_type __n) { vec.reserve(__n); }
        reference operator[](size_type __n) { return *(vec[__n]); }
        const_reference operator[](size_type __n) const { return *(vec[__n]); }
        reference at(size_type __n) { return *(vec.at(__n)); }
        const_reference at(size_type __n) const { return *(vec.at(__n)); }
        reference front() { return *(vec.front()); }
        const_reference front() const { return *(vec.front()); }
        reference back() { return *(vec.back()); }
        const_reference back() const { return *(vec.back()); }
        void push_back(const value_type& __x) { vec.push_back(new value_type(__x)); }
        void pop_back() { delete *(--vec.end()); vec.pop_back(); }
        iterator insert(iterator __position, const value_type& __x) {
            return  vec.insert(__position, new value_type(__x));
        }
        void insert(iterator __position, size_type __n, const value_type& __x) {
            for(size_type i = 0 ; i < __n ; i++) {
                insert(__position + i, __x);
            }
        }
        iterator erase(iterator __position) { delete *(__position.base()); return vec.erase(__position.base()); }
        iterator erase(iterator __first, iterator __last) {
            iterator i = __first;
            for(; i != __last ; i++)
                delete *(i.base());
            return vec.erase(__first.base(), __last.base());
        }
        void swap(pvector& __x) { vec.swap(__x); }
        void clear() { erase(begin(), end()); }
    };
}
#endif // __pvector_h

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.2.2.1  2007/08/21 07:49:46  ninomi
 * UP 2.0 released!
 *
 * Revision 1.2  2007/08/21 06:17:44  ninomi
 * UP release 2.0
 *
 * Revision 1.1.2.2  2007/08/17 00:53:59  ninomi
 * run the first version of up 2.0.  The edge number for more than 100 sentences went beyond the edge limit.
 *
 * Revision 1.1.2.1  2007/08/08 02:18:50  ninomi
 * added pvector.h (pointered containers' vector)
 *
 */


