#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/utsname.h>

#include "utils/common.h"
#include "utils/includes.h"
#include "utils/kernel_module_utils.h"

#define BUFFER 100

#define init_module(mod, len, opt) syscall(__NR_init_module, mod, len, opt)

int check_macsec_module();

/*
 *  Check if MACsec kernel module is currently loaded into the
 *  system
 */
int check_macsec_module() 
{
    FILE* fd_proc_modules;
    char buff[BUFFER];
    char proc_entry[BUFFER];
    int found = 0;

    wpa_printf(MSG_DEBUG, "Checking /proc/modules for MACsec module");
    /*
     *  Search for 'macsec' in /proc/modules
     */
    fd_proc_modules = fopen("/proc/modules", "r");
    if (fd_proc_modules == NULL) {
        wpa_printf(MSG_ERROR, "Error while loading /proc/modules");
        return -1;
    }


    while(fgets(buff, BUFFER, fd_proc_modules) != NULL) {
        sscanf(buff, "%s", proc_entry);
        if (!strcmp("macsec", proc_entry)) {
            found = 1;
            break;
        }
    }

    fclose(fd_proc_modules);

    return found;
}

/*
 *  Load MACsec kernel module if it has not been previously
 *  loaded
 */
int load_macsec_module() 
{    
    struct utsname* release;
    char* module_path;
    struct stat st;
    int fd, ret = 0, n;
    void* mod_image;
    size_t mod_image_size;
    
    wpa_printf(MSG_DEBUG, "Checking MACsec kernel module");

    if (check_macsec_module()) {
        wpa_printf(MSG_DEBUG, "MACsec kernel module is loaded");
    } else {
        wpa_printf(MSG_DEBUG, "MACsec kernel module is not loaded");
       
        /*
         *  Retrieve the release of the running kernel
         */
        release = os_zalloc(sizeof(*release));
        uname(release);

        module_path = os_zalloc(BUFFER*sizeof(char));
        
        strcat(module_path, "/lib/modules/");
        strcat(module_path, release->release);
        strcat(module_path, "/kernel/drivers/net/macsec.ko");
        
        os_free(release);

        /*
         *  Open MACsec kernel module image
         *
         *  Path -> /lib/modules/$(uname -r)/kernel/drivers/net/macsec.ko
         */
        fd = open(module_path, O_RDONLY);
        
        if (fd > 0) { 
            fstat(fd, &st);

            mod_image_size = st.st_size;
            mod_image = os_zalloc(mod_image_size);
            n = read(fd, mod_image, mod_image_size);
            if (n < 0) {
                ret = -1;
            }
            close(fd);

            /*  
             *  Call init_module system call
             */
            if (init_module(mod_image, mod_image_size, "")) {
                wpa_printf(MSG_ERROR, "MACsec kernel module loading failed");
                ret = -1;
            } else {
                wpa_printf(MSG_DEBUG, "MACsec kernel module correctly loaded");
            }

            os_free(mod_image);
            os_free(module_path);
        } else {
            wpa_printf(MSG_ERROR, "Can not open %s", module_path);
            ret = -1;
        }
    }

    return ret;
}
