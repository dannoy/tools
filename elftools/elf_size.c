
/* gcc -o -g elf_size elf_size.c */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <elf.h>

#define _DEBUG_

#ifdef _DEBUG_
#   define DBG(fmt, args...) fprintf(stderr, "[DBG] "fmt, ##args)
#else
#   define DBG(fmt, args...)
#endif

enum {
	ELF_OK,
	ELF_ERR
};

struct ctx {
};

void fatal(struct ctx *u, int errnum, char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    fprintf(stderr, "[FWU] ");
    vfprintf(stderr, fmt, ap);
    va_end(ap);

    exit(errnum);//exit positive number
}


int elf_get_file_size(char *start_addr, size_t size)
{
    int fsize = 0, fsize2 = 0;
    Elf32_Ehdr *eh = (Elf32_Ehdr *)start_addr;
    Elf32_Shdr *sh= NULL;
    Elf32_Shdr *sh_start = NULL ,*sh_end = NULL;
    Elf32_Shdr *sh_shstrtab = NULL;
    char *shstrtab = NULL;
    int is_debug_elf = 0;
	//int is_no_debug_elf = 0;

    assert(size > sizeof(Elf32_Ehdr));

    //DBG("hdr size %d\n", eh->e_ehsize);
    //DBG("eshdr offset %d\n", eh->e_shoff);
    //DBG("eshdr entsize %d\n", eh->e_shentsize);
    //DBG("eshdr shnum %d\n", eh->e_shnum);
    sh_start = (Elf32_Shdr *)(start_addr + eh->e_shoff);
    sh_end = sh_start + eh->e_shnum;
    sh_shstrtab = sh_start + eh->e_shstrndx;
    shstrtab = start_addr + sh_shstrtab->sh_offset;
    //DBG("shstrtab addr %p offset %x\n", shstrtab, sh_shstrtab->sh_offset);

    for(sh = sh_start; sh < sh_end; ++sh) {
        //DBG("shdr name %s\n", &shstrtab[sh->sh_name]);
        if(0 == strncmp(&shstrtab[sh->sh_name], ".debug", strlen(".debug"))) {
            is_debug_elf = 1;
        }
		//else if(sh != sh_start &&
		//		0 == sh->sh_addr) {
        //    is_no_debug_elf = 1;
		//}
    }
	sh = sh_end - 1;
	fsize = sh->sh_offset + sh->sh_size;
    if(is_debug_elf) {
        //sh = sh_end - 1;
        //fsize = sh->sh_offset + sh->sh_size;
		return fsize;
    }
    else {
        fsize2 = eh->e_shoff + eh->e_shnum * eh->e_shentsize;
		fsize = fsize > fsize2 ? fsize : fsize2;
    }


    //DBG("file size %d\n", fsize);
    return fsize;
}

int elf_read_elf_info(char *file)
{
    int ret = 0;
    int fsize = 0;
    int fd = -1;
    void *faddr = NULL;
    struct stat st;
    char *elf_info = NULL;

    if((fd = open(file, O_RDONLY)) < 0) {
        fatal(NULL, ELF_ERR, "Cannot open %s\n", file);
    }
    else if((ret = fstat(fd, &st)) < 0) {
        fatal(NULL, ELF_ERR, "Cannot stat %s\n", file);
    }
    else if((faddr = mmap(NULL, st.st_size,
                PROT_READ, MAP_PRIVATE,
                fd, 0)) == MAP_FAILED) {
        fatal(NULL, ELF_ERR, "Cannot mmap %s\n", file);
    }
    
    fsize = elf_get_file_size((char *)faddr, st.st_size);
//    assert(st.st_size > fsize);

    munmap(faddr, st.st_size);

	DBG("elf file size %d\n", fsize);
    
    return ret;
}

int main(int argc, char *argv[])
{
	int ret = 0;

	elf_read_elf_info(argv[0]);

	return ret;	
}
