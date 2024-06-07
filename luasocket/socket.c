#include "socket.h"
//===========================================================================//
//wjftag 实现一套POLLFD_* 操作来替换FD_*
void POLLFD_CLR(t_socket fd, PollFDSet* set)
{
    unsigned int i;
    for (i = 0; i < set->fd_count; ++i)
    {
        if (set->pollfds[i].fd == fd)
        {
            while (i < set->fd_count - 1)
            {
                set->pollfds[i] = set->pollfds[i + 1];
            }
        }
        set->fd_count--;
        break;
    }
}
void POLLFD_SET(t_socket fd, PollFDSet* set, short events)
{
    unsigned int i;
    for (i = 0; i < set->fd_count; ++i)
    {
        if (set->pollfds[i].fd == fd)
        {
            break;
        }
    }

    if (i == set->fd_count && set->fd_count < POLLFD_SETSIZE)
    {
        set->pollfds[i].fd = fd;
        set->pollfds[i].events = events;
        set->fd_count++;
    }
}
int  POLLFD_ISSET(t_socket fd, PollFDSet* set)
{
    unsigned int i;
    for (i = 0; i < set->fd_count; ++i)
    {
        if (set->pollfds[i].fd == fd)
        {
            return 1;
        }
    }
    return 0;
}
void POLLFD_ZERO(PollFDSet* set)
{
    set->fd_count = 0;
}