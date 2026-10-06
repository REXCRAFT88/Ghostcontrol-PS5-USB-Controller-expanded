/* SPDX-License-Identifier: GPL-3.0-or-later
 * GhostControl Expanded - cooperative stop payload.
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

#ifdef __PROSPERO__
#include <ps5/kernel.h>
#endif

#define PID_PATH "/data/ghostpad/gc_main.pid"

static void elevate_credentials(void) {
#ifdef __PROSPERO__
    pid_t p = getpid();
    uint8_t caps[16];
    for (unsigned i = 0; i < sizeof(caps); ++i)
        caps[i] = 0xff;
    kernel_set_ucred_authid(p, 0x3800000000010003l);
    kernel_set_ucred_caps(p, caps);
#endif
}

int main(void) {
    elevate_credentials();

    int fd = open(PID_PATH, O_RDONLY);
    if (fd < 0)
        return 0;

    char buf[24] = {0};
    ssize_t n = read(fd, buf, sizeof(buf) - 1u);
    close(fd);
    if (n <= 0)
        return 0;

    pid_t pid = (pid_t)atoi(buf);
    if (pid > 1 && pid != getpid())
        (void)kill(pid, SIGTERM);

    (void)unlink(PID_PATH);
    return 0;
}
