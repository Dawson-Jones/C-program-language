#include <stdio.h>
#include <dlfcn.h>


int main(int argc, char const *argv[])
{
    void *handle = dlopen("./libdynmath.so", RTLD_LAZY);
    if (!handle) {
        fprintf(stderr, "Failed to load library: %s\n", dlerror());
        return 1;
    }

    int (*add) (int, int);
    add = dlsym(handle, "add");
    if (!add) {
        fprintf(stderr, "Failed to get function pointer: %s\n", dlerror());
        dlclose(handle);
        return 1;
    }

    printf("2 + 3 = %d\n", add(2,3));

    dlclose(handle);

    return 0;
}
