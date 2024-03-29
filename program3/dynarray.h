/*
 * This file contains the definition of an interface for a dynamic array.
 * You should not modify anything in this file, but you can find descriptions
 * of the dynamic array functions, including their parameters and their return
 * values, below.
 */

#ifndef __DYNARRAY_H
#define __DYNARRAY_H

/*
 * Structure used to represent a dynamic array.
 */
struct dynarray;

/*
 * Creates a new, empty dynamic array and returns a pointer to it.
 */
struct dynarray* dynarray_create();

/*
 * Free the memory associated with a dynamic array.  Note that, while this
 * function cleans up all memory used in the array itself, it does not free
 * any memory allocated to the pointer values stored in the array.  This is
 * the responsibility of the caller.
 *
 * Params:
 *   da - the dynamic array to be destroyed.  May not be NULL.
 */
void dynarray_free(struct dynarray* da);

/*
 * Returns the length (i.e. the number of elements) of a given dynamic array.
 */
int dynarray_length(struct dynarray* da);

/*
 * Inserts a new element to a dynamic array at a specified index.  All existing
 * elements following the specified index are moved back to make room for the
 * new element.
 *
 * Params:
 *   da - the dynamic array into which to insert an element.  May not be NULL.
 *   idx - the index in the array at which to insert the new element.  The
 *     special value -1 may be passed to insert at the end of the array.
 *   val - the value to be inserted.  Note that this parameter has type void*,
 *     which means that a pointer of any type can be passed.
 */
void dynarray_insert(struct dynarray* da, int idx, int val);

/*
 * Removes an element at a specified index from a dynamic array.  All existing
 * elements following the specified index are moved forward to fill in the
 * gap left by the removed element.
 *
 * Params:
 *   da - the dynamic array from which to remove an element.  May not be NULL.
 *   idx - the index of the element to be removed.  The special value -1 may
 *     be passed to remove the element at the end of the array.
 */
void dynarray_remove(struct dynarray* da, int idx);

/*
 * Returns the value of an existing element a dynamic array array.  Note that
 * this value is returned as type void*, so it will need to be cast back to
 * its original type to be used.
 *
 * Params:
 *   da - the dynamic array from which to get a value.  May not be NULL.
 *   idx - the index of the element whose value should be returned.  Must
 *     be between 0 and the length of the array.The special value -1 may also
 *     be passed to return the element at the end of the array.
 */
int dynarray_get(struct dynarray* da, int idx);

/*
 * Sets an existing element in a dynamic array array to a new value.
 *
 * Params:
 *   da - the dynamic array in which to set a value.  May not be NULL.
 *   idx - the index of the element whose value is to be set.  Must be
 *     between 0 and the length of the array. The special value -1 may also be
 *     passed to set the element at the end of the array.
 *   val - the new value to be set.  Note that this parameter has type void*,
 *     which means that a pointer of any type can be passed.
 */
void dynarray_set(struct dynarray* da, int idx, int val);

#endif