#include <stdio.h>
#include <stdlib.h>
#include <mntent.h>
#include <sys/mount.h>
#include <string.h>

int main() {
    struct mntent *ent;
    FILE *f = setmntent("/proc/mounts", "r");
    if (f == NULL) {
        perror("setmntent");
        return 1;
    }

    while ((ent = getmntent(f)) != NULL) {
       if(strcmp(ent->mnt_type, "bpf") == 0) {
           printf("%s on %s\n", ent->mnt_type, ent->mnt_dir);
       }
   }

   endmntent(f);
   return 0;
}