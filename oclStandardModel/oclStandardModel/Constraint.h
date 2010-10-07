#ifndef __CONSTRAINT_H
#define __CONSTRAINT_H

#include <cstdlib>
#include <iostream>
#include <cstdio>

#include "definitions.h"

enum ConstraintType {
	LE,
	LEQ,
	EQ,
	NOP
};

typedef struct _constraint {
	int op;
	float coefficients[DIMENSION];
	float b;
} constraint;

constraint* newConstraint(const int n = 1);
constraint* copyConstraint(const constraint* const c);
void negativeConstraint(constraint* const c);
void clearConstraint(constraint* const c);
void deleteConstraint(constraint* const c);

void printConstraint(constraint* c);

#endif
