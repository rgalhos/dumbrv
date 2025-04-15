#include "vm.h"

#define VM_MEM_SIZE (size_t)(2 << 20)

int main(int argc, char **argv) {
  (void)argc;
  (void)argv;

  struct vm_data vm;

  vm_init(&vm, VM_MEM_SIZE);
  vm_start(&vm);
  vm_shutdown(&vm);
}
