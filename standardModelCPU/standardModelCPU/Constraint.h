#ifndef _CONSTRAINT_H_
#define _CONSTRAINT_H_

#include <stdio.h>
#include <stdlib.h>

#include "definitions.h"

enum ConstraintType {
	LE,
	LEQ,
	EQ,
	NOP
};

typedef struct _constraint {
	int op;
	float coefficients[N];
	float b;
} constraint;

constraint* newConstraint(const int n = 1);
constraint* copyConstraint(const constraint* const c);
void negativeConstraint(constraint* const c);
void clearConstraint(constraint* const c);
void deleteConstraint(constraint* const c);

void printConstraint(constraint* c);

#endif