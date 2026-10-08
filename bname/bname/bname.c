#define _DEFAULT_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/utsname.h>
#include <string.h>
#include <locale.h>
#ifndef __linux__
#include <sys/param.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#endif
//#define COLOR "\033[48;2;R;G;B"
#define BK "\x1B[38;02;02;02;02;48;2;22;22;22m"
#define RD "\x1B[31m"
#define GN "\x1B[32m"
#define YL "\x1B[33m"
#define BL "\x1B[34m"
#define MG "\x1B[35m"
#define CY "\x1B[36m"
#define WH "\x1B[40;37m"
#define NO "\x1B[0m" 
#define DGR "\033[48;2;22;22;22;38;2;36;36;36m"
#define GR "\033[48;2;22;22;22;38;2;63;63;63m"
#define FLAME "\033[48;2;133;0;0;38;2;43;0;0m"
#define CLEAR "\033[2J\033[1;1H"
#define PLACEHOLDER "[[Hyperlink blocked]]"

int main(void);
int pages_size_valid(long p, long s);

int
pages_size_valid(long p, long s) {
    return (p > 0 && s > 0);
}

int
main(void) {
    //# structs
    setlocale(LC_ALL, "");
    struct utsname buffer;
    if (uname(&buffer) != 0) {
        perror("bname: uname failed");
        return 1;
    }
    //# cpu
    #if defined(__linux__)
    int cores = sysconf(_SC_NPROCESSORS_ONLN);
    #else
    int cores = 1;
    int mib_cpu[2] = {CTL_HW, HW_NCPU};
    int num_cpus = 1;
    size_t cpu_len = sizeof(num_cpus);
    if (sysctl(mib_cpu, 2, &num_cpus, &cpu_len, NULL, 0) == 0) cores = num_cpus;
    #endif
    if (cores < 1) cores = 1;
    double max_freq_mhz = 0.0;
    #if defined(__linux__)
    FILE *fp = fopen("/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq", "r");
    if (fp) {
        unsigned int max_freq_khz = 0;
        if (fscanf(fp, "%u", &max_freq_khz) == 1) {
            max_freq_mhz = max_freq_khz / 1000.0;
        }
        fclose(fp);
    }
    #else
    int clockrate = 0;
    size_t clock_len = sizeof(clockrate);
    if (sysctlbyname("hw.clockrate", &clockrate, &clock_len, NULL, 0) == 0) {
        max_freq_mhz = (double)clockrate;
    }
    #endif
    //# ram n swap
    long pages = sysconf(_SC_PHYS_PAGES);
    #if defined(_SC_PAGESIZE)
    long page_size = sysconf(_SC_PAGESIZE);
    #else
    long page_size = sysconf(_SC_PAGE_SIZE); 
    #endif
    long long total_ram_mb = 0;
    if (pages_size_valid(pages, page_size)) {
        total_ram_mb = ((long long)pages * page_size) / (1024 * 1024);
    }
    long long total_swap_mb = 0;
    #if defined(__linux__)
    long swap_pages = sysconf(_SC_AVPHYS_PAGES);
    if (swap_pages > 0 && page_size > 0) {
        total_swap_mb = ((long long)swap_pages * page_size) / (1024 * 1024);
    }
    #elif defined(__FreeBSD__)
    unsigned long total_swap_bytes = 0;
    size_t swap_len = sizeof(total_swap_bytes);
    if (sysctlbyname("vm.swap_total", &total_swap_bytes, &swap_len, NULL, 0) == 0) {
        total_swap_mb = (long long)total_swap_bytes / (1024 * 1024);
    }
    #elif defined(__NetBSD__) || defined(__OpenBSD__) || defined(__DragonFly__)
    int mib[3];
    size_t len;
    mib[0] = CTL_VM;
    mib[1] = VM_SWAPUSAGE;
    struct {
        int64_t total;
        int64_t reserved;
        int64_t allocated;
    } swap_info;
    len = sizeof(swap_info);
    if (sysctl(mib, 2, &swap_info, &len, NULL, 0) == 0) {
        total_swap_mb = swap_info.total / (1024 * 1024);
    }
    #else
    total_swap_mb = 0; 
    #endif
    //# misc
    const char *ver = "26.10.08" ;
    //# Get username
    const char *user = getenv("USER");
    if (!user) user = getenv("LOGNAME");
    if (!user) user = PLACEHOLDER;
    //# Get shell and tty
    char *tty_dev = ttyname(0);
    if (!tty_dev) tty_dev = PLACEHOLDER;
    char *shell = getenv("SHELL");
    if (!shell) shell = PLACEHOLDER;
    size_t num = strlen(user) + 4;
    char *str = (char *)malloc(num + 1);
    if (str != NULL) {
        memset(str, 'x', num); 
        str[num] = '\0';
    } else {
        str = "xx";
    }
    //# show info
    printf("%s┌────┤LiquidUtils├─────┐%s X [%s]\n",FLAME,NO, user);
    printf("%s│ &#Xx   x****x X****x │%s X%s \n",FLAME,NO, str);
    printf("%s│ #...$  #..... #....X │%s X Kernel - [%s]\n",FLAME,NO, buffer.sysname);
    printf("%s│ @#X#@x ^====x #....X │%s X Release - [%s]\n",FLAME,NO, buffer.release);
    printf("%s│ #....# .....# #....X │%s X Machine - [%s]\n",FLAME,NO, buffer.machine);
    printf("%s│ &$$$X^ ^xxxx^ #xxxx^ │%s X Nodename - [%s]\n",FLAME,NO, buffer.nodename);
    printf("%s│xx  x   x   xx_xx xxxx│%s X Version - [%s]\n",FLAME,NO, buffer.version);
    printf("%s│X X X _X_X_ X^X^X X===│%s X Shell at Input - [%s] at [%s]\n",FLAME,NO,shell,tty_dev);
    printf("%s│X  XX X* *X X X X Xxxx│%s X Freq * Cores - [%.2f] MHZ * [%d] CORES\n",FLAME,NO,max_freq_mhz, cores);
    printf("%s└──────────────────────┘%s X Total Ram + Swap Vol - [%lld] + [%lld] MB\n",FLAME,NO, total_ram_mb, total_swap_mb);
    printf("%sxX%sxX%sxX%sxX%sxX%sxX%sxX%sxX%sxX%sxX%sxX%sxX X bname ver - [%s]\n",BK,DGR,GR,WH,RD,YL,GN,CY,BL,MG,FLAME,NO,ver);
    if (str != NULL && strcmp(str, "xx") != 0) free(str);
    return 0;
}