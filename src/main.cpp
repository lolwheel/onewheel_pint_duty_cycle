#include <stdint.h>

struct BSS_T {
};

/**
 * Static and non-const seems to be the magic combination to keep thes variables unstripped
 * while still allocated in flash.
 */
static uint32_t BSS_REQUESTED_SIZE
    __attribute__((section(".bss_requested_len"))) __attribute__((__used__)) =
        sizeof(BSS_T);
static BSS_T* BSS __attribute__((section(".bss_ptr")))
__attribute__((__used__)) = (BSS_T*)(0x200002B0 + 0x1998);

/**
 * This function will be located 8 bytes into the stripped patch. 
 */
int16_t __attribute__((section(".entry"))) __attribute__((__used__)) patchFunc() {
  int16_t v1 = *(int16_t*)(0x20001bc0);
  int16_t v2 = *(int16_t*)(0x20001bc2);
  if (v1 < 0) {
    v1 = - v1;
  }
  if (v2 < 0) {
    v2 = -v2;
  }
  return v1;
}
