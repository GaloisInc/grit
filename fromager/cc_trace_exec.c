#include <stdint.h>
#include <stdio.h>

void __cc_trace_exec(
    const char* name,
    uintptr_t arg0,
    uintptr_t arg1,
    uintptr_t arg2,
    uintptr_t arg3,
    uintptr_t arg4,
    uintptr_t arg5,
    uintptr_t arg6,
    uintptr_t arg7) {
  int count = 8;
  if (count == 8 && arg7 == 0) { --count; }
  if (count == 7 && arg6 == 0) { --count; }
  if (count == 6 && arg5 == 0) { --count; }
  if (count == 5 && arg4 == 0) { --count; }
  if (count == 4 && arg3 == 0) { --count; }
  if (count == 3 && arg2 == 0) { --count; }
  if (count == 2 && arg1 == 0) { --count; }
  if (count == 1 && arg0 == 0) { --count; }

  printf("[FUNC] %s(", name);
  if (count >= 1) { printf("%lx", arg0); }
  if (count >= 2) { printf(", %lx", arg1); }
  if (count >= 3) { printf(", %lx", arg2); }
  if (count >= 4) { printf(", %lx", arg3); }
  if (count >= 5) { printf(", %lx", arg4); }
  if (count >= 6) { printf(", %lx", arg5); }
  if (count >= 7) { printf(", %lx", arg6); }
  if (count >= 8) { printf(", %lx", arg7); }
  printf(")\n");
}
