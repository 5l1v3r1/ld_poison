#define _GNU_SOURCE

#include <stdio.h>
#include <dlfcn.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <string.h>

/* Dirty hack */
#define _SYS_IOCTL_H
#include <bits/ioctls.h>

#define DEBUG

char mac[] = "aa:bb:cc:dd:ee:ff";

static void init (void) __attribute__ ((constructor));
static int (*o_ioctl) (int, unsigned long int, ...);
int ioctl (int, unsigned long int, void *);

void init(void)
{
	#ifdef DEBUG
	printf("[-] ld_poison loaded.\n");
	#endif

	o_ioctl = dlsym(RTLD_NEXT, "ioctl");

	#ifdef DEBUG
    printf("[-] old ioctl addr: %p\n", o_ioctl);
    #endif
}

int ioctl(int fd, unsigned long int request, void * arg)
{

    struct ifreq *s;
    unsigned int imac[6];
    int ret;
    int i;

    #ifdef DEBUG
    printf("[-] ioctl hooked\n");
    printf("[-] ioctl requested: 0x%lx\n", request);
    #endif

    if (request == SIOCGIFHWADDR) {
        s = (struct ifreq *) arg;

        #ifdef DEBUG
        printf("[-] this is SIOCGIFHWADDR\n");
        printf("[-] iface: %s\n", s->ifr_name);
        #endif

        ret = o_ioctl(fd, request, s);
        if (ret == 0) {

            #ifdef DEBUG
            printf("[-] mac before: ");
            for (i = 0; i < 6; i++) {
                printf("%02x", (unsigned char) s->ifr_addr.sa_data[i]);
            }
            printf("\n");
            #endif

            sscanf(mac, "%x:%x:%x:%x:%x:%x", &imac[0], &imac[1], &imac[2], &imac[3], &imac[4], &imac[5]);
            for (i = 0; i < 6; i++) {
                s->ifr_addr.sa_data[i] = (unsigned char) imac[i];
            }

            #ifdef DEBUG
            printf("[-] mac after: ");
            for (i = 0; i < 6; i++) {
                printf("%02x", (unsigned char) s->ifr_addr.sa_data[i]);
            }
            printf("\n");
            #endif

            return ret;
        } else {

            #ifdef DEBUG
            printf("[-] SIOCGIFHWADDR ioctl for %s failed\n", s->ifr_name); 
            #endif

            return ret;
        }
    }

    return o_ioctl(fd, request, arg);
}
