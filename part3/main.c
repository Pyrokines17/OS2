#include "proxy/proxy.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    proxy_t *proxy = proxy_create();

    if (proxy == NULL) {
        fprintf(stderr, "Failed to create proxy\n");
        return EXIT_FAILURE;
    }

    proxy_start(proxy);

    return EXIT_SUCCESS;
}