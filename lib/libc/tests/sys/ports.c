/*
 * This program demonstrates the problem documented in PR#102 regarding
 * multiple bindings on a port.
 *
 * $Id: ports.c,v 1.1 1998/12/31 22:45:35 gdr-ftp Exp $
 */

#include <sys/ports.h>
#include <stdio.h>

#define PORT_NAME "mytestport"

int
main (int argc, char **argv) {
  int result, port1, port2;

  printf("creating ports\n");
  if ((port1 = pcreate(2)) == -1) {
    perror("first pcreate failed");
    return 1;
  }
  printf("port 1 is %d\n", port1);
  if ((port2 = pcreate(3)) == -1) {
    perror("second pcreate failed");
    return 1;
  }
  printf("port 2 is %d\n", port2);

  printf("binding first port: should succeed\n");
  result = pbind(port1, PORT_NAME);
  if (result == -1) {
    printf("bind of first port failed with code %d\n", result);
  } else {
    printf("bind of first port succeeded with code %d\n", result);
  }

  printf("binding second port: should fail\n");
  result = pbind(port2, PORT_NAME);
  if (result == -1) {
    printf("bind of second port failed with code %d\n", result);
  } else {
    printf("bind of second port succeeded with code %d\n", result);
  }

  printf("pgetport returns %d\n", pgetport(PORT_NAME));
  return 0;
}
