/* SPDX-License-Identifier: GPL-3.0-or-later
 * GhostControl Expanded / PoorDS4 evidence collector.
 *
 * Read-only helper: gathers existing PoorDS4 diagnostic files into one
 * bounded text bundle. It does not inspect or modify game/RemotePlay memory.
 */

#include <dirent.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include <ps5/payload.h>

#define DATA_DIR   "/data/poords4"
#define REPORT_DIR DATA_DIR "/reports"
#define OUT_PATH   DATA_DIR "/13x-evidence.txt"
#define MAX_COPY_PER_FILE (256u * 1024u)

static int
copy_file_bounded(int out_fd, const char *label, const char *path)
{
    int fd = open(path, O_RDONLY);
    if (fd < 0)
        return -1;

    char header[512];
    int hn = snprintf(
        header, sizeof(header),
        "\n===== %s =====\npath=%s\n",
        label ? label : "file", path);
    if (hn > 0)
        (void)write(out_fd, header, (size_t)hn);

    char buf[4096];
    size_t copied = 0;
    for (;;) {
        ssize_t n = read(fd, buf, sizeof(buf));
        if (n <= 0)
            break;
        size_t keep = (size_t)n;
        if (copied + keep > MAX_COPY_PER_FILE)
            keep = MAX_COPY_PER_FILE - copied;
        if (keep != 0)
            (void)write(out_fd, buf, keep);
        copied += keep;
        if (copied >= MAX_COPY_PER_FILE)
            break;
    }
    close(fd);

    if (copied >= MAX_COPY_PER_FILE) {
        static const char truncated[] =
            "\n[collector truncated this file at 256 KiB]\n";
        (void)write(out_fd, truncated, sizeof(truncated) - 1u);
    }
    return 0;
}

static int
ends_with_txt(const char *name)
{
    if (!name)
        return 0;
    size_t n = strlen(name);
    return n >= 4u && strcmp(name + n - 4u, ".txt") == 0;
}

static void
collect_reports(int out_fd)
{
    DIR *dir = opendir(REPORT_DIR);
    if (!dir)
        return;

    struct dirent *ent;
    unsigned included = 0;
    while ((ent = readdir(dir)) != NULL && included < 32u) {
        if (ent->d_name[0] == '.' || !ends_with_txt(ent->d_name))
            continue;

        char path[512];
        int n = snprintf(path, sizeof(path), "%s/%s",
                         REPORT_DIR, ent->d_name);
        if (n <= 0 || (size_t)n >= sizeof(path))
            continue;

        if (copy_file_bounded(out_fd, ent->d_name, path) == 0)
            included++;
    }
    closedir(dir);
}

int
main(void)
{
    (void)mkdir(DATA_DIR, 0755);

    int out_fd = open(
        OUT_PATH, O_WRONLY | O_CREAT | O_TRUNC, 0600);
    if (out_fd < 0) {
        payload_exit(1);
        return 1;
    }

    static const char intro[] =
        "GhostControl Expanded / PoorDS4 13.x Evidence Bundle\n"
        "Storage note: the game may be launched from an FFPFSC mount; "
        "the bridge diagnoses the running eboot/libScePad process.\n";
    (void)write(out_fd, intro, sizeof(intro) - 1u);

    static const struct {
        const char *label;
        const char *path;
    } important[] = {
        {"firmware compatibility digest", DATA_DIR "/fw-compat-last.txt"},
        {"game bridge last report", DATA_DIR "/game-pad-bridge-last.txt"},
        {"client table last report", DATA_DIR "/client-table-last.txt"},
        {"supervisor state", DATA_DIR "/game-pad-bridge-supervisor.txt"},
        {"status report", DATA_DIR "/game-pad-bridge-status.txt"},
        {"game bridge log", DATA_DIR "/game-pad-bridge.log"}
    };

    for (unsigned i = 0;
         i < sizeof(important) / sizeof(important[0]); ++i)
        (void)copy_file_bounded(
            out_fd, important[i].label, important[i].path);

    collect_reports(out_fd);

    static const char done[] =
        "\n===== END OF EVIDENCE BUNDLE =====\n";
    (void)write(out_fd, done, sizeof(done) - 1u);
    close(out_fd);

    payload_exit(0);
    return 0;
}
