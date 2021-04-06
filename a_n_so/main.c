#include <stdio.h>
#include <memory.h>

typedef struct person_s person_t;
struct person_s {
	int age;
	char *name;
	void (*run)(person_t *this, int k);
};


void run(person_t *this, int k) {
	printf("%s age: %d can run %d kilometers\n", this->name, this->age, k);
};


int main() {
	char name[] = "dawson";
	person_t dawson = {18, name, run};
	dawson.run(&dawson, 8);
	return 0;
}