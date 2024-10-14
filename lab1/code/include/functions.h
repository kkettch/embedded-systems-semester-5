#ifndef ELF64_LOADER_FUNCTIONS_H
#define ELF64_LOADER_FUNCTIONS_H

#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <elf.h>
#include <unistd.h>
#include "error.h"

size_t string_length(char *string);
int open_elf_file(char *elf_file_name);
void close_elf_file(int fd);
Elf64_Ehdr read_elf_header(int fd);
int is_valid_elf_header(Elf64_Ehdr *elf_header);
Elf64_Off find_name_section_table_offset(Elf64_Ehdr *elf_header);
Elf64_Shdr read_section_header(int fd, Elf64_Off elf_offset);
Elf64_Off find_section_offset(int fd, char *section_name, Elf64_Ehdr *elf_header,  Elf64_Off offset);
void load_segments(int fd, Elf64_Ehdr *header);
void execute_section(Elf64_Shdr *sh);

#endif //ELF64_LOADER_FUNCTIONS_H
