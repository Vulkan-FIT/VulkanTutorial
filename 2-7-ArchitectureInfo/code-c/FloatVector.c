#include "FloatVector.h"
#include <stdio.h>
#include <stdlib.h>


void vector_destroy(struct FloatVector* v)
{
	if(v->capacity != 0) {
		free(v->data);
		v->size = 0;
		v->data = 0;
		v->capacity = 0;
	}
}


void vector_push_back(struct FloatVector* v, float value)
{
	if(v->size < v->capacity) {
		v->data[v->size] = value;
		v->size++;
	}
	else if(v->capacity == 0) {
		v->data = malloc(4 * sizeof(float));
		v->capacity = 4;
		v->data[0] = value;
		v->size = 1;
	}
	else {
		float* tmp;
		v->capacity *= 2;
		tmp = realloc(v->data, v->capacity * sizeof(float));
		if(tmp == 0) {
			printf("Out of memory.");
			exit(1);
		}
		v->data = tmp;
		v->data[v->size] = value;
		v->size++;
	}
}


void vector_sort(struct FloatVector* v)
{
	unsigned long i,c;
	float tmp;
	c = vector_size(v);
	if(c <= 1)
		return;
	c -= 1;
again:
	for(i=0; i<c; i++) {
		if(v->data[i] > v->data[i+1]) {
			tmp = v->data[i];
			v->data[i] = v->data[i+1];
			v->data[i+1] = tmp;
			i++;
			for(; i<c; i++) {
				if(v->data[i] > v->data[i+1]) {
					tmp = v->data[i];
					v->data[i] = v->data[i+1];
					v->data[i+1] = tmp;
				}
			}
			c--;
			goto again;
		}
	}
}
