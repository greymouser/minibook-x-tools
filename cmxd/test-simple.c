#include <stdio.h>
#include "cmxd-modes.h"

void test_log(const char *fmt, ...) {}

int main() {
  cmxd_modes_set_log_debug(test_log);
  printf("Testing 275.0 degrees (laptop): %s\n", cmxd_get_device_mode(275.0, NULL));
  printf("Testing 325.0 degrees (tent): %s\n", cmxd_get_device_mode(325.0, NULL));  
  printf("Testing 355.0 degrees (tablet): %s\n", cmxd_get_device_mode(355.0, NULL));
  printf("Testing 180.0 degrees (flat): %s\n", cmxd_get_device_mode(180.0, NULL));
  printf("Testing 30.0 degrees (closing): %s\n", cmxd_get_device_mode(30.0, NULL));
  return 0;
}
