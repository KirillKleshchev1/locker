#include <unistd.h>
#include <getopt.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

static int locks_count = 0;
static pid_t process_pid;

void handle_signal(int sig) {
    char stat_str[32];
    int stat_str_len = snprintf(stat_str, sizeof(stat_str),
                              "PID %d Success locks: %d\n", process_pid, locks_count);

    int stat_fd = open("stat", O_WRONLY | O_APPEND | O_CREAT, 0777);
    if (stat_fd != -1) {
        write(stat_fd, stat_str, stat_str_len);
        close(stat_fd);
    }
    exit(0);
}

void lock_file(const char* lck_name, const char* write_buf, int buf_size) {
    int lck_fd;
    do {
        lck_fd = open(lck_name, O_CREAT | O_EXCL | O_RDWR, 0777);
    } while (lck_fd == -1);

    write(lck_fd, write_buf, buf_size);
    close(lck_fd);
}

int unlock_file(const char* lck_name, int buf_size) {
    char lck_buf[buf_size];
    int lck_fd = open(lck_name, O_RDONLY);
    if (lck_fd == -1) return -1;

    int read_bytes = read(lck_fd, lck_buf, buf_size);
    close(lck_fd);

    if (atoi(lck_buf) != process_pid) return -1;
    if (remove(lck_name)) return -1;

    return 0;
}

int main(int argc, char *argv[]) {
    signal(SIGINT, handle_signal);
    char lck_name[strlen(argv[1]) + 5];
    snprintf(lck_name, sizeof(lck_name), "%s.lck", argv[1]);

    process_pid = getpid();
    char pid_str[16];
    int pid_str_len = snprintf(pid_str, sizeof(pid_str), "%d", process_pid);

    while (1) {
        lock_file(lck_name, pid_str, pid_str_len);
        sleep(1);

        if (unlock_file(lck_name, pid_str_len)) {
            fputs("Error: Can not unlock file\n", stderr);
            return 1;
        }

        locks_count++;
    }

    return 0;
}
