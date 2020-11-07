#ifndef DYNARRAY_H
#define DYNARRAY_H

struct dynarray;

struct dynarray* create_array();
void free_array();
int array_length(struct dynarray* da);
void dynarray_insert(struct dynarray*da, int idx, int val);
void dynarray_remove(struct dynarray* da, int idx);
int array_get(struct dynarray* da, int idx);
void set_array(struct dynarray* da, int idx, int val);




#endif