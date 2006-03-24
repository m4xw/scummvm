/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

// Code is based on:

/* 
 * Copyright (c) 1998-2003 Massachusetts Institute of Technology. 
 * This code was developed as part of the Haystack research project 
 * (http://haystack.lcs.mit.edu/). Permission is hereby granted, 
 * free of charge, to any person obtaining a copy of this software 
 * and associated documentation files (the "Software"), to deal in 
 * the Software without restriction, including without limitation 
 * the rights to use, copy, modify, merge, publish, distribute, 
 * sublicense, and/or sell copies of the Software, and to permit 
 * persons to whom the Software is furnished to do so, subject to 
 * the following conditions: 
 * 
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software. 
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES 
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT 
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR 
 * OTHER DEALINGS IN THE SOFTWARE. 
 */

/*************************************************

  assocarray.h - Associative arrays

  Andrew Y. Ng, 1996

**************************************************/

#include "common/stdafx.h"
#include "common/str.h"
#include "common/util.h"

#ifndef COMMON_ASSOCARRAY_H
#define COMMON_ASSOCARRAY_H

namespace Common {

#define INIT_SIZE 11

typedef Common::String String;

// If aa is an AssocArray<Key,Val>, then space is allocated each
// time aa[key] is referenced, for a new key. To "see" the value
// of aa[key] but without allocating new memory, use aa.queryVal(key)
// instead.
// 
// An AssocArray<Key,Val> Maps type Key to type Val. For each 
// Key, we need a int hashit(Key,int) that hashes Key and returns 
// an integer from 0 to hashsize-1, and a int data_eq(Key,Key) 
// that returns true if the 2 arguments it is passed are to
// be considered equal. Also, we assume that "=" works
// on Val's for assignment.

int hashit(int x, int hashsize);
int data_eq(int x, int y);
int hashit(double x, int hashsize);
int data_eq(double x, double y);
int hashit(const char *str, int hashsize);
int data_eq(const char *str1, const char *str2);
int hashit(const String &str, int hashsize);
int data_eq(const String &str1, const String &str2);

template <class Key, class Val>
class AssocArray {
private:
	// data structure used by AssocArray internally to keep
	// track of what's mapped to what.
	struct aa_ref_t {
		Key key;
		Val dat;
		aa_ref_t(const Key& k) : key(k) {}
	};
	
	aa_ref_t **_arr;	// hashtable of size arrsize.
	int _arrsize, _nele;

	int lookup(const Key &key) const;
	void expand_array(void);

public:

	AssocArray();
	~AssocArray();

	bool contains(const Key &key) const;

	Val &operator [](const Key &key);
	const Val &operator [](const Key &key) const;
	const Val &queryVal(const Key &key) const;

	void clear(bool shrinkArray = 0);

	// The following two methods are used to return a list of
	// all the keys / all the values (or rather, duplicates of them).
	// Currently they aren't used, and I think a better appraoch
	// is to add iterators, which allow the same in a more C++-style
	// fashion, do not require making duplicates, and finally
	// even allow in-place modifications of
	Key *new_all_keys(void) const;
	Val *new_all_values(void) const;
	
	
	// TODO: There is no "remove(key)" method yet.
};

//-------------------------------------------------------
// AssocArray functions

template <class Key, class Val>
int AssocArray<Key, Val>::lookup(const Key &key) const {
	int ctr;

	ctr = hashit(key, _arrsize);

	while (_arr[ctr] != NULL && !data_eq(_arr[ctr]->key, key)) {
		ctr++;

		if (ctr == _arrsize)
			ctr = 0;
	}

	return ctr;
}

template <class Key, class Val>
bool AssocArray<Key, Val>::contains(const Key &key) const {
	int ctr = lookup(key);
	return (_arr[ctr] != NULL);
}

template <class Key, class Val>
Key *AssocArray<Key, Val>::new_all_keys(void) const {
	Key *all_keys;
	int ctr, dex;

	if (_nele == 0)
		return NULL;

	all_keys = new Key[_nele];

	assert(all_keys != NULL);

	dex = 0;
	for (ctr = 0; ctr < _arrsize; ctr++) {
		if (_arr[ctr] == NULL)
			continue;
		all_keys[dex++] = _arr[ctr]->key;

		assert(dex <= _nele);
	}

	assert(dex == _nele);

	return all_keys;
}

template <class Key, class Val>
Val *AssocArray<Key, Val>::new_all_values(void) const {
	Val *all_values;
	int ctr, dex;

	if (_nele == 0)
		return NULL;

	all_values = new Val[_nele];

	assert(all_values != NULL);

	dex = 0;
	for (ctr = 0; ctr < _arrsize; ctr++) {
		if (_arr[ctr] == NULL)
			continue;

		all_values[dex++] = _arr[ctr]->dat;

		assert(dex <= _nele);
	}

	assert(dex == _nele);

	return all_values;
}

template <class Key, class Val>
AssocArray<Key, Val>::AssocArray() {
	int ctr;

	_arr = new aa_ref_t *[INIT_SIZE];
	assert(_arr != NULL);

	for (ctr = 0; ctr < INIT_SIZE; ctr++)
		_arr[ctr] = NULL;

	_arrsize = INIT_SIZE;
	_nele = 0;
}

template <class Key, class Val>
AssocArray<Key, Val>::~AssocArray() {
	int ctr;

	for (ctr = 0; ctr < _arrsize; ctr++)
		if (_arr[ctr] != NULL)
			delete _arr[ctr];

	delete[] _arr;
}

template <class Key, class Val>
void AssocArray<Key, Val>::clear(bool shrinkArray) {
	for (int ctr = 0; ctr < _arrsize; ctr++) {
		if (_arr[ctr] != NULL) {
			delete _arr[ctr];
			_arr[ctr] = NULL;
		}
	}

	if (shrinkArray && _arrsize > INIT_SIZE) {
		delete _arr;

		_arr = new aa_ref_t *[INIT_SIZE];
		_arrsize = INIT_SIZE;

		for (int ctr = 0; ctr < _arrsize; ctr++)
			_arr[ctr] = NULL;
	}

	_nele = 0;
}

template <class Key, class Val>
void AssocArray<Key, Val>::expand_array(void) {
	aa_ref_t **old_arr;
	int old_arrsize, old_nele, ctr, dex;

	old_nele = _nele;
	old_arr = _arr;
	old_arrsize = _arrsize;

    // GROWTH_FACTOR 1.531415936535
	// allocate a new array 
	_arrsize = 153 * old_arrsize / 100;

	// Ensure that _arrsize is odd.
	_arrsize |= 1;

	_arr = new aa_ref_t *[_arrsize];

	assert(_arr != NULL);

	for (ctr = 0; ctr < _arrsize; ctr++)
		_arr[ctr] = NULL;

	_nele = 0;

	// rehash all the old elements
	for (ctr = 0; ctr < old_arrsize; ctr++) {
		if (old_arr[ctr] == NULL)
			continue;

//      (*this)[old_arr[ctr]->key] = old_arr[ctr]->dat;
		dex = hashit(old_arr[ctr]->key, _arrsize);

		while (_arr[dex] != NULL)
			if (++dex == _arrsize)
				dex = 0;

		_arr[dex] = old_arr[ctr];
		_nele++;
	}

	assert(_nele == old_nele);

	delete[] old_arr;

	return;
}

template <class Key, class Val>
Val &AssocArray<Key, Val>::operator [](const Key &key) {
	int ctr = lookup(key);

	if (_arr[ctr] == NULL) {
		_arr[ctr] = new aa_ref_t(key);
		_nele++;

		if (_nele > _arrsize / 2) {
			expand_array();
			ctr = lookup(key);
		}
	}

	return _arr[ctr]->dat;
}

template <class Key, class Val>
const Val &AssocArray<Key, Val>::operator [](const Key &key) const {
	return queryVal(key);
}

template <class Key, class Val>
const Val &AssocArray<Key, Val>::queryVal(const Key &key) const {
	int ctr = lookup(key);
	assert(_arr[ctr] != NULL);
	return _arr[ctr]->dat;
}

}	// End of namespace Common

#endif
