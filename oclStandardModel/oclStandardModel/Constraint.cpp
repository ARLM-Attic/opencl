#include "Constraint.h"

constraint* newConstraint(const int n) {
	constraint* c = (constraint*) malloc(sizeof(constraint)*n);
	for (int i = 0; i < n; i++)
		clearConstraint(&c[i]);
	return c;
}

void clearConstraint(constraint* const c) {
	for (int i = 0; i < DIMENSION; i++) {
		c->coefficients[i] = 0;
		c->b = 0;
		c->op = NOP;
	}
}

constraint* copyConstraint(const constraint* const c) {
	constraint* c2 = newConstraint();
	c2->b = c->b;
	c2->op = c->op;
	for (int i = 0; i < DIMENSION; i++)
		c2->coefficients[i] = c->coefficients[i];
	return c2;
}

void negativeConstraint(constraint* const c) {
	for (int i = 0; i < DIMENSION; i++)
		c->coefficients[i] = -1 * c->coefficients[i];
	c->b = -1 * c->b;
}

void deleteConstraint(constraint* const c) {
	free (c);
}

void printConstraint(constraint* c) {
	for (int i = 0; i < DIMENSION; i++) {
		printf("%f\t", c->coefficients[i]);
	}
	printf("op\t%f", c->b);
}