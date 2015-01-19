// Copyright (c) 2013, Jack.
// All rights reserved.
//
// Author: Jack <lapsangx@gmail.com>
// Created: 05/08/13
// Description: eventfd test

#include "eventfd.h"
#include <stdio.h>

static void Producer(int evfd, int loop_count)
{
    for (int i = 0; i < loop_count; ++i)
    {
        eventfd_t l = i;
        eventfd_write(evfd, l);
    }
}

int main(int argc, char** argv)
{
    unsigned int loop_count = 100;

    int evfd = eventfd(0, 0);
    if (evfd < 0)
    {
        perror("eventfd");
        return 1;
    }

    Producer(evfd, loop_count);
    close(evfd);
}
