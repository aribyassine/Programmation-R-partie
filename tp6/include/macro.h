#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

#define DESC_SHM(id, arg)                                                                                                                            \
    printf("%s\n", arg);                                                                                                                             \
    if ((id = shm_open(arg, O_RDWR | O_CREAT, 0600)) == -1) {                                                                                        \
        perror("shm_open");                                                                                                                          \
        exit(EXIT_FAILURE);                                                                                                                          \
    }

#define CTRL_DESC(id, n) ftruncate(id, n * sizeof(int))
#define ALLOC_SHM(ptr, id, n) ptr = mmap(NULL, n * sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, id, 0)
#define CTRL_SHM(ptr) ptr == MAP_FAILED
#define SET_SHM(ptr, id, i, rand_val) ptr[i] = rand_val
#define GET_SHM(res, ptr, i, id) res = ptr[i]
#define FREE_SHM(id, ptr, n, arg) munmap(ptr, n * sizeof(int)) || shm_unlink(arg)
