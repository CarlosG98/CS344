#include <stdlib.h>
#include <assert.h>

#include "dynarray.h"

#define DYNARRAY_INIT_CAPACITY 1

struct dynarray{
    int* data;
    int length;
    int capacity;
};


struct dynarray* create_array(){
    struct dynarray* da = malloc(sizeof(struct dynarray));
    assert(da);

    da->data = malloc(DYNARRAY_INIT_CAPACITY * sizeof(int));
    assert(da->data);
    da->length = 0;
    da->capacity = DYNARRAY_INIT_CAPACITY;

    return da;
}

void free_array(struct dynarray* da){
    assert(da);
    free(da->data);
    free(da);
}

int array_length(struct dynarray* da){
    assert(da);
    return da->length;
}

void _dynarray_resize(struct dynarray* da, int new_capacity){
    assert(new_capacity > da->length);

    int* new_data = malloc(new_capacity * sizeof(int));
    assert(new_data);

    for(int i = 0; i < da->length; i++){
        new_data[i] = da->data[i];
    }

    free(da->data);
    da->data = new_data;
    da->capacity = new_capacity;
}

void dynarray_insert(struct dynarray* da, int idx, int val){
    assert(da);
    assert((idx <= da->length && idx >=0) || idx == -1);

    if(idx == -1){
        idx = da->length;
    }

    if(da->length == da->capacity){
        _dynarray_resize(da, 2 * da->capacity);
    }

    for(int i = da->length; i > idx; i--){
        da->data[i] = da->data[i-1];
    }

    da->data[idx] = val;
    da->length++;
}

void dynarray_remove(struct dynarray* da, int idx){
    assert(da);
    assert((idx < da->length && idx >= 0) || idx == -1);

    if(idx==-1){
        idx = da->length - 1;
    }

    for(int i = idx; i < da->length - 1; i++){
        da->data[i] = da->data[i+1];
    }

    da->length--;
}


int array_get(struct dynarray* da, int idx){
    assert(da);
    assert((idx < da->length && idx >=0) || idx==-1);
    
    if(idx == -1){
        idx = da->length - 1;
    }

    return da->data[idx];
}


void set_array(struct dynarray* da, int idx, int val){
    assert(da);
    assert((idx < da->length && idx >= 0) || idx == -1);
    
    if(idx == -1){
        idx = da->length - 1;
    }

    da->data[idx] = val;
}