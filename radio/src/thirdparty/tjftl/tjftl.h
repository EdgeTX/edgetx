#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct tjftl_t tjftl_t;

typedef bool (*flashcb_read_t)(int addr, uint8_t *buf, int len, void *arg);
typedef bool (*flashcb_erase_32k_t)(int addr, void *arg);
typedef bool (*flashcb_program_t)(int addr, const uint8_t *buf, int len, void *arg);

int tjftl_detect(flashcb_read_t rf, void *arg);
tjftl_t *tjftl_init(flashcb_read_t rf, flashcb_erase_32k_t ef, flashcb_program_t pf, void *arg, int size, int sect_cnt, int verbose);
bool tjftl_read(tjftl_t *tj, int lba, uint8_t *buf);
bool tjftl_write(tjftl_t *tj, int lba, const uint8_t *buf);

#ifdef __cplusplus
}
#endif
