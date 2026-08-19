#ifndef FLOAT_VECTOR_H_
#define FLOAT_VECTOR_H_


struct FloatVector {
	unsigned size;
	float* data;
	unsigned capacity;
};


#include <assert.h>
inline void vector_init(struct FloatVector* v)  { v->size = 0; v->data = 0; v->capacity = 0; }
void vector_destroy(struct FloatVector* v);
inline int vector_empty(struct FloatVector* v)  { return v->size == 0; }
inline unsigned long vector_size(struct FloatVector* v)  { return v->size; }
inline float vector_get(struct FloatVector* v, unsigned long index)  { assert(index<v->size && "Index out of bounds."); return v->data[index]; }
void vector_push_back(struct FloatVector* v, float value);
void vector_sort(struct FloatVector* v);


#endif
