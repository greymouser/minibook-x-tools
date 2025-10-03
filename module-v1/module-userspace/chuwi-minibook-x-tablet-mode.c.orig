#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* read raw accel axis from IIO sysfs */
static int read_iio(const char *dev, const char *axis, int *out)
{
    char path[256], buf[64];
    FILE *f;
    snprintf(path, sizeof(path), "/sys/bus/iio/devices/%s/in_accel_%s_raw", dev, axis);
    f = fopen(path, "r");
    if (!f) return -1;
    if (!fgets(buf, sizeof(buf), f)) { fclose(f); return -1; }
    fclose(f);
    *out = (int)strtol(buf, NULL, 10);
    return 0;
}

/* read accel scale if available */
static double read_scale(const char *dev)
{
    char path[256];
    double scale = 0.0;
    FILE *f;

    snprintf(path, sizeof(path), "/sys/bus/iio/devices/%s/in_accel_scale", dev);
    f = fopen(path, "r");
    if (f) {
        if (fscanf(f, "%lf", &scale) != 1)
            scale = 0.0;
        fclose(f);
    }
    return scale;
}

int main(int argc, char **argv)
{
    const char *base = "iio:device0";
    const char *lid  = "iio:device1";
    unsigned poll_ms = 100;

    if (argc >= 3) { base = argv[1]; lid = argv[2]; }
    if (argc >= 4) { poll_ms = (unsigned)atoi(argv[3]); }

    for (;;) {
        int bx,by,bz, lx,ly,lz;
        double bscale, lscale;
        int bxs,bys,bzs, lxs,lys,lzs;

        if (read_iio(base,"x",&bx) || read_iio(base,"y",&by) || read_iio(base,"z",&bz) ||
            read_iio(lid ,"x",&lx) || read_iio(lid ,"y",&ly) || read_iio(lid ,"z",&lz)) {
            usleep(200000);
            continue;
        }

        bscale = read_scale(base);
        lscale = read_scale(lid);

        if (bscale > 0.0) {
            bxs = (int)(bx * bscale * 1e6);
            bys = (int)(by * bscale * 1e6);
            bzs = (int)(bz * bscale * 1e6);
        } else {
            bxs = bx * 1000;
            bys = by * 1000;
            bzs = bz * 1000;
        }

        if (lscale > 0.0) {
            lxs = (int)(lx * lscale * 1e6);
            lys = (int)(ly * lscale * 1e6);
            lzs = (int)(lz * lscale * 1e6);
        } else {
            lxs = lx * 1000;
            lys = ly * 1000;
            lzs = lz * 1000;
        }

        /* write to kernel sysfs */
        FILE *wb = fopen("/sys/kernel/chuwi-minibook-x-tablet-mode/base_vec","w");
        if (!wb) {
            perror("open base_vec");
        } else {
            fprintf(wb,"%d %d %d\n", bxs,bys,bzs);
            fclose(wb);
        }

        FILE *wl = fopen("/sys/kernel/chuwi-minibook-x-tablet-mode/lid_vec","w");
        if (!wl) {
            perror("open lid_vec");
        } else {
            fprintf(wl,"%d %d %d\n", lxs,lys,lzs);
            fclose(wl);
        }

        usleep(poll_ms*1000);
    }
}
