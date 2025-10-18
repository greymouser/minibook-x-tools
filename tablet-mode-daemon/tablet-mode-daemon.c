// SPDX-License-Identifier: GPL-2.0
/*
 * Hyprland Tablet Mode Integration Daemon
 * 
 * Monitors SW_TABLET_MODE input events and triggers Hyprland tablet mode behaviors
 * including virtual keyboard, UI scaling, touch gestures, and window management.
 *
 * Copyright (c) 2025 Armando DiCianno <armando@noonshy.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <getopt.h>
#include <limits.h>
#include <stdarg.h>
#include <time.h>
#include <dirent.h>
#include <linux/input.h>

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

/* Bit manipulation macros for input event handling */
#define NLONGS(x) (((x) + 8 * sizeof(long) - 1) / (8 * sizeof(long)))
#define test_bit(bit, array) ((array)[(bit) / (8 * sizeof(long))] & (1UL << ((bit) % (8 * sizeof(long)))))

#define PROGRAM_NAME "tablet-mode-daemon"
#define VERSION "1.0"

/* Configuration */
struct config {
    char tablet_device[PATH_MAX];
    char config_file[PATH_MAX];
    char on_tablet_script[PATH_MAX];
    char on_laptop_script[PATH_MAX];
    int verbose;
    int daemon_mode;
    unsigned int debounce_ms;
};

/* Global state */
static volatile sig_atomic_t running = 1;
static struct config cfg = {
    .tablet_device = "",  /* Empty = auto-detect */
    .config_file = "",
    .on_tablet_script = "",
    .on_laptop_script = "",
    .verbose = 0,
    .daemon_mode = 1,
    .debounce_ms = 500  /* 500ms debounce to avoid rapid switching */
};

/* Current state tracking */
static int last_tablet_state = -1;
static struct timespec last_event_time = {0, 0};

/* Forward declarations */
static int load_config(const char *config_file);
static int auto_detect_tablet_device(char *device_path, size_t path_size);

/* Signal handlers */
static void signal_handler(int sig)
{
    switch (sig) {
        case SIGTERM:
        case SIGINT:
            running = 0;
            break;
        case SIGHUP:
            /* Reload config in the future */
            break;
    }
}

/* Logging functions */
static void log_msg(const char *level, const char *fmt, ...)
{
    va_list args;
    struct timeval tv;
    char timestr[64];
    
    if (!cfg.verbose && strcmp(level, "DEBUG") == 0)
        return;
        
    gettimeofday(&tv, NULL);
    strftime(timestr, sizeof(timestr), "%Y-%m-%d %H:%M:%S", localtime(&tv.tv_sec));
    
    if (cfg.daemon_mode) {
        /* Use syslog format for daemon mode */
        fprintf(stderr, "%s[%d]: [%s] ", PROGRAM_NAME, getpid(), level);
    } else {
        fprintf(stderr, "%s [%s] ", timestr, level);
    }
    
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, "\n");
    fflush(stderr);
}

#define log_error(fmt, ...) log_msg("ERROR", fmt, ##__VA_ARGS__)
#define log_warn(fmt, ...)  log_msg("WARN", fmt, ##__VA_ARGS__)
#define log_info(fmt, ...)  log_msg("INFO", fmt, ##__VA_ARGS__)
#define log_debug(fmt, ...) log_msg("DEBUG", fmt, ##__VA_ARGS__)

/* Execute script/command with proper error handling */
static int execute_command(const char *command, const char *action)
{
    int status;
    pid_t pid;
    
    if (!command || !*command) {
        log_debug("No %s command configured", action);
        return 0;
    }
    
    log_info("Executing %s command: %s", action, command);
    
    pid = fork();
    if (pid == 0) {
        /* Child process */
        execl("/bin/sh", "sh", "-c", command, NULL);
        log_error("Failed to execute command: %s", strerror(errno));
        exit(127);
    } else if (pid > 0) {
        /* Parent process */
        if (waitpid(pid, &status, 0) == -1) {
            log_error("Failed to wait for %s command: %s", action, strerror(errno));
            return -1;
        }
        
        if (WIFEXITED(status)) {
            int exit_code = WEXITSTATUS(status);
            if (exit_code != 0) {
                log_warn("%s command exited with code %d", action, exit_code);
                return exit_code;
            }
            log_debug("%s command completed successfully", action);
            return 0;
        } else if (WIFSIGNALED(status)) {
            log_error("%s command terminated by signal %d", action, WTERMSIG(status));
            return -1;
        }
    } else {
        log_error("Failed to fork for %s command: %s", action, strerror(errno));
        return -1;
    }
    
    return 0;
}

/* Handle tablet mode state change */
static int handle_tablet_mode_change(int tablet_mode)
{
    struct timespec now;
    long long time_diff_ms;
    
    /* Get current time for debouncing */
    clock_gettime(CLOCK_MONOTONIC, &now);
    
    /* Check debouncing */
    if (last_event_time.tv_sec != 0) {
        time_diff_ms = (now.tv_sec - last_event_time.tv_sec) * 1000LL +
                       (now.tv_nsec - last_event_time.tv_nsec) / 1000000LL;
        
        if (time_diff_ms < cfg.debounce_ms) {
            log_debug("Ignoring rapid state change (debounce: %lld ms < %u ms)", 
                     time_diff_ms, cfg.debounce_ms);
            return 0;
        }
    }
    
    last_event_time = now;
    
    /* Skip if state hasn't actually changed */
    if (last_tablet_state == tablet_mode) {
        log_debug("State unchanged (%s mode), skipping", tablet_mode ? "tablet" : "laptop");
        return 0;
    }
    
    last_tablet_state = tablet_mode;
    
    log_info("Tablet mode changed: %s", tablet_mode ? "ENABLED" : "DISABLED");
    
    /* Execute custom scripts */
    if (tablet_mode) {
        return execute_command(cfg.on_tablet_script, "tablet");
    } else {
        return execute_command(cfg.on_laptop_script, "laptop");
    }
}

/* Check if device supports SW_TABLET_MODE */
static int check_device_capabilities(int fd)
{
    unsigned long evbit[NLONGS(EV_MAX)] = {0};
    unsigned long swbit[NLONGS(SW_MAX)] = {0};
    
    if (ioctl(fd, EVIOCGBIT(0, EV_MAX), evbit) < 0) {
        log_error("Failed to get device event types: %s", strerror(errno));
        return -1;
    }
    
    if (!test_bit(EV_SW, evbit)) {
        log_error("Device does not support switch events");
        return -1;
    }
    
    if (ioctl(fd, EVIOCGBIT(EV_SW, SW_MAX), swbit) < 0) {
        log_error("Failed to get switch capabilities: %s", strerror(errno));
        return -1;
    }
    
    if (!test_bit(SW_TABLET_MODE, swbit)) {
        log_error("Device does not support SW_TABLET_MODE");
        return -1;
    }
    
    return 0;
}

/* Get initial tablet mode state */
static int get_initial_state(int fd)
{
    unsigned long swstate[NLONGS(SW_MAX)] = {0};
    
    if (ioctl(fd, EVIOCGSW(sizeof(swstate)), swstate) < 0) {
        log_error("Failed to get initial switch state: %s", strerror(errno));
        return -1;
    }
    
    int tablet_mode = test_bit(SW_TABLET_MODE, swstate) ? 1 : 0;
    log_info("Initial tablet mode state: %s", tablet_mode ? "tablet" : "laptop");
    
    return tablet_mode;
}

/* Auto-detect tablet mode device by scanning /dev/input/event* */
static int auto_detect_tablet_device(char *device_path, size_t path_size)
{
    DIR *input_dir;
    struct dirent *entry;
    char test_path[PATH_MAX];
    int fd;
    int found = 0;
    
    log_debug("Auto-detecting tablet mode device...");
    log_debug("Opening /dev/input directory");
    
    input_dir = opendir("/dev/input");
    if (!input_dir) {
        log_error("Failed to open /dev/input: %s", strerror(errno));
        return -1;
    }
    
    log_debug("Scanning /dev/input for event devices");
    while ((entry = readdir(input_dir)) != NULL) {
        /* Only check event devices */
        if (strncmp(entry->d_name, "event", 5) != 0) {
            continue;
        }
        
        snprintf(test_path, sizeof(test_path), "/dev/input/%s", entry->d_name);
        log_debug("Testing device: %s", test_path);
        
        /* Try to open the device */
        fd = open(test_path, O_RDONLY);
        if (fd < 0) {
            log_debug("Cannot open %s: %s", test_path, strerror(errno));
            continue;
        }
        
        /* Check if device supports SW_TABLET_MODE */
        if (check_device_capabilities(fd) == 0) {
            log_info("Found tablet mode device: %s", test_path);
            snprintf(device_path, path_size, "%s", test_path);
            found = 1;
            close(fd);
            break;
        }
        
        close(fd);
    }
    
    closedir(input_dir);
    
    if (!found) {
        log_error("No tablet mode device found in /dev/input/");
        log_info("Try: sudo libinput list-devices | grep -i tablet");
        return -1;
    }
    
    log_debug("Auto-detection completed successfully");
    return 0;
}

/* Main event monitoring loop */
static int monitor_tablet_events(void)
{
    int fd;
    struct input_event ev;
    ssize_t n;
    char final_device[PATH_MAX];
    
    log_debug("Starting monitor_tablet_events function");
    
    /* Auto-detect device if not specified */
    if (!cfg.tablet_device[0]) {
        log_debug("No device specified, starting auto-detection");
        if (auto_detect_tablet_device(final_device, sizeof(final_device)) < 0) {
            log_error("Auto-detection failed, exiting");
            return -1;
        }
        log_debug("Auto-detection succeeded: %s", final_device);
    } else {
        log_debug("Using configured device: %s", cfg.tablet_device);
        strncpy(final_device, cfg.tablet_device, sizeof(final_device) - 1);
        final_device[sizeof(final_device) - 1] = '\0';
    }
    
    log_debug("Opening device: %s", final_device);
    /* Open tablet mode device */
    fd = open(final_device, O_RDONLY);
    if (fd < 0) {
        log_error("Failed to open device %s: %s", final_device, strerror(errno));
        return -1;
    }
    log_debug("Device opened successfully, fd=%d", fd);
    
    /* Check device capabilities */
    log_debug("Checking device capabilities");
    if (check_device_capabilities(fd) < 0) {
        log_error("Device capability check failed");
        close(fd);
        return -1;
    }
    log_debug("Device capabilities verified");
    
    /* Get and handle initial state */
    log_debug("Getting initial tablet mode state");
    int initial_state = get_initial_state(fd);
    if (initial_state >= 0) {
        /* Set the initial state but don't execute scripts on startup */
        last_tablet_state = initial_state;
        log_info("Initial state set to %s mode (no script execution)", 
                 initial_state ? "tablet" : "laptop");
    }
    
    log_info("Monitoring tablet mode events on %s", final_device);
    log_debug("Entering main event loop");
    
    /* Event monitoring loop */
    while (running) {
        n = read(fd, &ev, sizeof(ev));
        
        if (n < 0) {
            if (errno == EINTR) {
                /* Interrupted by signal - check if we should exit */
                log_debug("Read interrupted by signal, running=%d", running);
                if (!running) {
                    log_debug("Received signal, exiting gracefully");
                    break;
                }
                continue;
            }
            log_error("Error reading from device: %s", strerror(errno));
            break;
        }
        
        if (n != sizeof(ev)) {
            log_warn("Incomplete event read: %zd bytes", n);
            continue;
        }
        
        /* Process tablet mode switch events */
        if (ev.type == EV_SW && ev.code == SW_TABLET_MODE) {
            log_debug("SW_TABLET_MODE event: value=%d", ev.value);
            handle_tablet_mode_change(ev.value);
        }
    }
    
    log_debug("Exiting main event loop");
    close(fd);
    return 0;
}

/* Load configuration with precedence: command line -> user config -> system config */
static int load_config_with_precedence(const char *config_file)
{
    char user_config[PATH_MAX];
    char system_config[] = "/etc/tablet-mode/daemon.conf";
    const char *home;
    int loaded = 0;
    
    /* If specific config file provided via command line, use it exclusively */
    if (config_file && *config_file) {
        log_debug("Using command-line specified config: %s", config_file);
        return load_config(config_file);
    }
    
    /* Try user config first: ~/.config/tablet-mode/daemon.conf */
    home = getenv("HOME");
    if (home) {
        snprintf(user_config, sizeof(user_config), "%s/.config/tablet-mode/daemon.conf", home);
        if (access(user_config, R_OK) == 0) {
            log_debug("Loading user config: %s", user_config);
            if (load_config(user_config) == 0) {
                loaded = 1;
            }
        }
    }
    
    /* If no user config found, try system config: /etc/tablet-mode/daemon.conf */
    if (!loaded && access(system_config, R_OK) == 0) {
        log_debug("Loading system config: %s", system_config);
        if (load_config(system_config) == 0) {
            loaded = 1;
        }
    }
    
    if (!loaded) {
        log_info("No configuration file found, using defaults");
        log_debug("Checked: %s, %s", user_config, system_config);
    }
    
    return 0;
}

/* Load configuration from file */
static int load_config(const char *config_file)
{
    FILE *fp;
    char line[512];
    char key[256], value[256];
    
    if (!config_file || !*config_file) {
        return 0;  /* No config file specified */
    }
    
    fp = fopen(config_file, "r");
    if (!fp) {
        if (errno != ENOENT) {
            log_error("Failed to open config file %s: %s", config_file, strerror(errno));
            return -1;
        }
        return 0;  /* Config file doesn't exist, use defaults */
    }
    
    log_info("Loading configuration from %s", config_file);
    
    while (fgets(line, sizeof(line), fp)) {
        /* Skip comments and empty lines */
        char *stripped = line;
        while (*stripped == ' ' || *stripped == '\t') stripped++;
        if (*stripped == '#' || *stripped == '\n' || *stripped == '\0') {
            continue;
        }
        
        /* Parse key=value pairs */
        if (sscanf(line, "%255[^=]=%255s", key, value) == 2) {
            /* Trim whitespace */
            char *k = key, *v = value;
            while (*k == ' ' || *k == '\t') k++;
            while (*v == ' ' || *v == '\t') v++;
            
            /* Process configuration options */
            if (strcmp(k, "tablet_device") == 0) {
                strncpy(cfg.tablet_device, v, sizeof(cfg.tablet_device) - 1);
            } else if (strcmp(k, "on_tablet_script") == 0) {
                strncpy(cfg.on_tablet_script, v, sizeof(cfg.on_tablet_script) - 1);
            } else if (strcmp(k, "on_laptop_script") == 0) {
                strncpy(cfg.on_laptop_script, v, sizeof(cfg.on_laptop_script) - 1);
            } else if (strcmp(k, "debounce_ms") == 0) {
                cfg.debounce_ms = atoi(v);
            } else if (strcmp(k, "verbose") == 0) {
                cfg.verbose = atoi(v);
            } else {
                log_warn("Unknown config option: %s", k);
            }
        }
    }
    
    fclose(fp);
    return 0;
}

/* Setup signal handlers */
static int setup_signals(void)
{
    struct sigaction sa;
    
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    /* Don't use SA_RESTART - we want read() to be interrupted by signals */
    sa.sa_flags = 0;
    
    if (sigaction(SIGTERM, &sa, NULL) < 0 ||
        sigaction(SIGINT, &sa, NULL) < 0 ||
        sigaction(SIGHUP, &sa, NULL) < 0) {
        log_error("Failed to setup signal handlers: %s", strerror(errno));
        return -1;
    }
    
    return 0;
}

/* Usage help */
static void usage(const char *program)
{
    printf("Usage: %s [OPTIONS]\n", program);
    printf("\nHyprland Tablet Mode Integration Daemon\n");
    printf("Monitors SW_TABLET_MODE events and triggers Hyprland tablet mode behaviors.\n\n");
    printf("OPTIONS:\n");
    printf("  -d, --device DEVICE     Tablet mode input device (default: auto-detect)\n");
    printf("  -c, --config FILE       Configuration file path (overrides auto-detection)\n");
    printf("  -t, --on-tablet CMD     Command to run when entering tablet mode\n");
    printf("  -l, --on-laptop CMD     Command to run when entering laptop mode\n");
    printf("  -b, --debounce MS       Debounce time in milliseconds (default: %u)\n", cfg.debounce_ms);
    printf("  -f, --foreground        Run in foreground (don't daemonize)\n");
    printf("  -v, --verbose           Enable verbose logging\n");
    printf("  -h, --help              Show this help\n");
    printf("  -V, --version           Show version\n");
    printf("\nCONFIGURATION FILES:\n");
    printf("  Configuration files are checked in this order:\n");
    printf("  1. Command line specified file (-c option)\n");
    printf("  2. User config: ~/.config/tablet-mode/daemon.conf\n");
    printf("  3. Default example scripts (if no config found)\n");
    printf("\nEXAMPLES:\n");
    printf("  %s                                           # Use auto-detected device\n", program);
    printf("  %s -v -f                                     # Verbose, foreground\n", program);
    printf("  %s -c ~/.config/tablet-mode/daemon.conf      # Specific config\n", program);
    printf("  %s -t 'onboard' -l 'pkill onboard'          # Virtual keyboard\n", program);
    printf("\nSee tablet-mode-daemon(8) for more information.\n");
}

/* Main function */
int main(int argc, char *argv[])
{
    int opt;
    const char *short_opts = "d:c:t:l:b:fvhV";
    static struct option long_opts[] = {
        {"device",     required_argument, 0, 'd'},
        {"config",     required_argument, 0, 'c'},
        {"on-tablet",  required_argument, 0, 't'},
        {"on-laptop",  required_argument, 0, 'l'},
        {"debounce",   required_argument, 0, 'b'},
        {"foreground", no_argument,       0, 'f'},
        {"verbose",    no_argument,       0, 'v'},
        {"help",       no_argument,       0, 'h'},
        {"version",    no_argument,       0, 'V'},
        {0, 0, 0, 0}
    };
    
    /* Parse command line arguments */
    while ((opt = getopt_long(argc, argv, short_opts, long_opts, NULL)) != -1) {
        switch (opt) {
            case 'd':
                strncpy(cfg.tablet_device, optarg, sizeof(cfg.tablet_device) - 1);
                break;
            case 'c':
                strncpy(cfg.config_file, optarg, sizeof(cfg.config_file) - 1);
                break;
            case 't':
                strncpy(cfg.on_tablet_script, optarg, sizeof(cfg.on_tablet_script) - 1);
                break;
            case 'l':
                strncpy(cfg.on_laptop_script, optarg, sizeof(cfg.on_laptop_script) - 1);
                break;
            case 'b':
                cfg.debounce_ms = atoi(optarg);
                break;
            case 'f':
                cfg.daemon_mode = 0;
                break;
            case 'v':
                cfg.verbose = 1;
                break;
            case 'h':
                usage(argv[0]);
                exit(0);
            case 'V':
                printf("%s %s\n", PROGRAM_NAME, VERSION);
                exit(0);
            default:
                usage(argv[0]);
                exit(1);
        }
    }
    
    /* Load configuration file */
    log_debug("Loading configuration file...");
    if (load_config_with_precedence(cfg.config_file) < 0) {
        log_error("Failed to load configuration file");
        exit(1);
    }
    log_debug("Configuration loaded successfully");
    
    /* Setup signal handlers */
    log_debug("Setting up signal handlers...");
    if (setup_signals() < 0) {
        log_error("Failed to setup signal handlers");
        exit(1);
    }
    log_debug("Signal handlers setup successfully");
    
    /* Daemonize if requested */
    if (cfg.daemon_mode) {
        log_debug("Daemonizing process...");
        if (daemon(0, 0) < 0) {
            log_error("Failed to daemonize: %s", strerror(errno));
            exit(1);
        }
        log_debug("Daemonization successful");
    }
    
    log_info("Starting %s %s", PROGRAM_NAME, VERSION);
    log_debug("Process PID: %d", getpid());
    log_debug("User ID: %d, Group ID: %d", getuid(), getgid());
    log_debug("Daemon mode: %s", cfg.daemon_mode ? "enabled" : "disabled");
    log_debug("Verbose logging: %s", cfg.verbose ? "enabled" : "disabled");
    
    if (cfg.tablet_device[0]) {
        log_info("Device: %s (configured), Debounce: %ums", cfg.tablet_device, cfg.debounce_ms);
    } else {
        log_info("Device: auto-detect, Debounce: %ums", cfg.debounce_ms);
    }
    
    log_debug("Configuration summary:");
    log_debug("  Config file: %s", cfg.config_file[0] ? cfg.config_file : "(none specified)");
    log_debug("  On tablet script: %s", cfg.on_tablet_script[0] ? cfg.on_tablet_script : "(none)");
    log_debug("  On laptop script: %s", cfg.on_laptop_script[0] ? cfg.on_laptop_script : "(none)");
    
    log_debug("Starting main monitoring loop");
    
    /* Main monitoring loop */
    int ret = monitor_tablet_events();
    
    log_info("Exiting with code %d", ret == 0 ? 0 : 1);
    return ret == 0 ? 0 : 1;
}