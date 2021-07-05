#include <stdio.h>
#include <stddef.h>

struct bpf_lpm_trie_key {
    int prefixlen;    /* up to 32 for AF_INET, 128 for AF_INET6 */
    char data[0];    /* Arbitrary size */
};


int main() {
    int a = 1;
    // typeof(a) = int
    typeof(a) b = 9;
    printf("%d %d\n", a, b);
    
    printf("%p\n", &(((struct bpf_lpm_trie_key *) 0)->data));
    printf("offset: %lu\n", offsetof(struct bpf_lpm_trie_key, data));
    return 0;
}