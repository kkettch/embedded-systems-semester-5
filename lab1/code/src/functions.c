#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <elf.h>
#include <sys/mman.h>
#include <unistd.h>
#include "../include/error.h"

#ifndef MAP_FIXED_NOREPLACE
#define MAP_FIXED_NOREPLACE 0x100000
#endif

size_t string_length(char *string)
{
    size_t i;

    i = 0;
    while (string[i] != '\0')
    {
        i++;
    }
    return (i);
}

int open_elf_file(char *elf_file_name)
{
    int fd;

    fd = open(elf_file_name, O_RDONLY);
    if (fd <= 0)
        error_and_exit("Error during opening file\n", ENOENT);
    return (fd);
}

void close_elf_file(int fd)
{
    close(fd);
}

Elf64_Ehdr read_elf_header(int fd)
{
    Elf64_Ehdr elf_header;
    ssize_t bytes_read;

    bytes_read = read(fd, &elf_header, sizeof(Elf64_Ehdr));
    if (bytes_read < sizeof(Elf64_Ehdr))
    {
        close_elf_file(fd);
        error_and_exit("Error during reading elf header\n", EIO);
    }
    return (elf_header);
}

int is_valid_elf_type(Elf64_Ehdr *elf_header)
{
    if (elf_header->e_ident[EI_MAG0] != 0x7f ||
        elf_header->e_ident[EI_MAG1] != 'E' ||
        elf_header->e_ident[EI_MAG2] != 'L' ||
        elf_header->e_ident[EI_MAG3] != 'F')
        return (0);
    return (1);
}

int is_dynamic_library(Elf64_Ehdr *elf_header)
{
    if (elf_header->e_type == ET_DYN)
        return (1);
    return (0);
}

int is_section_header_table_contained(Elf64_Ehdr *elf_header)
{
    if (elf_header->e_shnum == 0)
        return (0);
    return (1);
}

int is_valid_elf_header(Elf64_Ehdr *elf_header)
{
    if (is_valid_elf_type(elf_header) &&
        !is_dynamic_library(elf_header) &&
        is_section_header_table_contained(elf_header))
        return (1);
    return (0);
}

Elf64_Off find_name_section_table_offset(Elf64_Ehdr *elf_header)
{
    Elf64_Off elf_offset;

    elf_offset = elf_header->e_shoff + (elf_header->e_shentsize * elf_header->e_shstrndx);
    return (elf_offset);
}

Elf64_Shdr read_section_header(int fd, Elf64_Off elf_offset)
{
    Elf64_Shdr  section_header;
    off_t       result_offset;
    ssize_t     bytes_read;

    result_offset = lseek(fd, (off_t)elf_offset, SEEK_SET);
    if (result_offset == (off_t)-1)
    {
        close_elf_file(fd);
        error_and_exit("Error reading section header during moving to the required offset\n", EIO);
    }
    bytes_read = read(fd, &section_header, sizeof(Elf64_Shdr));
    if (bytes_read < sizeof(Elf64_Shdr))
    {
        close_elf_file(fd);
        error_and_exit("Error during reading section header\n", EIO);
    }
    return (section_header);
}

int string_equal(int fd, char *section_name)
{
    char c;
    ssize_t bytes_read;

    bytes_read = read(fd, &c, 1);
    while (bytes_read == 1 && c == *section_name++)
    {
        if (c == '\0')
            return (1);
        bytes_read = read(fd, &c, 1);
    }
    return (0);
}

Elf64_Off find_section_offset(int fd, char *section_name, Elf64_Ehdr *elf_header,  Elf64_Off offset)
{
    size_t      i;
    Elf64_Off   current_section_offset;
    off_t       lseek_result;
    ssize_t     bytes_read;
    Elf64_Shdr  section_header;

    i = 0;
    while (i < elf_header->e_shnum)
    {
        current_section_offset = elf_header->e_shoff + (elf_header->e_shentsize * i);
        lseek_result = lseek(fd, (off_t)current_section_offset, SEEK_SET);
        if (lseek_result == (off_t)-1)
        {
            close_elf_file(fd);
            error_and_exit("Error finding section offset during moving to the required offset\n", EIO);
        }
        bytes_read = read(fd, &section_header, sizeof(Elf64_Shdr));
        if (bytes_read < sizeof(Elf64_Shdr))
        {
            close_elf_file(fd);
            error_and_exit("Error finding section offset during reading\n", EIO);
        }
        lseek_result = lseek(fd, (off_t)offset + section_header.sh_name, SEEK_SET);
        if (lseek_result == (off_t)-1)
        {
            close_elf_file(fd);
            error_and_exit("Error finding section offset during moving to the required offset\n", EIO);
        }
        if (string_equal(fd, section_name))
            return (current_section_offset);
        i++;
    }
    close_elf_file(fd);
    error_and_exit("Error: section name not found\n", EINVAL);
    return (0);
}

int get_protection_flags(Elf64_Phdr *program_header)
{
    int flags;

    flags = 0;
    if (program_header->p_flags & PF_X)
        flags |= PROT_EXEC;
    if (program_header->p_flags & PF_W)
        flags |= PROT_WRITE;
    if (program_header->p_flags & PF_R)
        flags |= PROT_READ;
    return (flags);
}

void map_segment(int fd, Elf64_Phdr *program_header, size_t pagesz)
{
    Elf64_Addr virtual_address;
    Elf64_Off offset;
    size_t memory_size;

    virtual_address = program_header->p_vaddr;
    offset = program_header->p_offset;
    virtual_address -= virtual_address % pagesz;
    offset -= offset % pagesz;
    memory_size = program_header->p_memsz + (program_header->p_vaddr - virtual_address);
    if (mmap(
            (void *) virtual_address,
            memory_size,
            get_protection_flags(program_header),
            MAP_PRIVATE | MAP_FIXED | MAP_FIXED_NOREPLACE,
            fd,
            (off_t)offset
    ) == MAP_FAILED) {
        close_elf_file(fd);
        error_and_exit("Error: fail during mapping segment\n", EIO);
    }
}

void load_segments(int fd, Elf64_Ehdr *header)
{
    size_t page_size;
    Elf64_Phdr program_header;
    size_t index;

    page_size = sysconf(_SC_PAGE_SIZE);
    index = 0;
    if (lseek(fd, (off_t)header->e_phoff, SEEK_SET) < 0) {
        close_elf_file(fd);
        error_and_exit("Failed to seek to program headers", EIO);
    }
    while (index < header->e_phnum) {
        if (read(fd, &program_header, header->e_phentsize) < header->e_phentsize) {
            close_elf_file(fd);
            error_and_exit("Failed to read program header", EIO);
        }
        if (program_header.p_type == PT_LOAD) {
            map_segment(fd, &program_header, page_size);
        }
        index++;
    }
}

void execute_section(Elf64_Shdr *sh)
{
    typedef void (*section_start_fn)(void);
    section_start_fn start = (section_start_fn)sh->sh_addr;
    start();
}






