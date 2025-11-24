#pragma once
#include <zephyr/kernel.h>
#include "lorawan.h"

/* Thread entry */
void pir_handler(void *, void *, void *);