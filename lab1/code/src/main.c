#include <stdio.h>
#include <errno.h>
#include <elf.h>
#include "../include/functions.h"
#include "../include/error.h"

#define ARGUMENTS 2
#define ELF_FILE_NAME 1
#define SECTION_NAME 2

//TODO: remove stdio.h
//TODO: delete comments

int main( int argc, char** argv ) {
    char        *elf_file_name;         // переменные для считывания аргументов
    char        *section_name;
    int         fd;                     // файловый дескриптор считанного файла
    Elf64_Ehdr  elf_header;             // считывание заголовка elf-файла
    Elf64_Off   elf_offset;             // смещение заголовка таблицы имен секций
    Elf64_Shdr  section_header;         // хранение заголовка секции, содержащей таблицу имен секций
    Elf64_Off   section_offset;         // хранение найденного заголовка секции по ее имени

    // 1. Проверка аргументов командной строки
    if (argc != ARGUMENTS + 1)
        error_and_exit("Invalid amount of arguments\n", EINVAL);

    // считываем параметры
    elf_file_name = argv[ELF_FILE_NAME];
    section_name = argv[SECTION_NAME];

    // 2. Открываем файл
    fd = open_elf_file(elf_file_name);

    // 3. Считываем ELF-заголовок
    elf_header = read_elf_header(fd);

    // 4. ВАЛИДАЦИЯ: Проверяем магическое число ELF (тип файла - ELF)
    // Проверяем, что это не динамическая библиотека
    // Проверяем, что есть таблица заголовков секций
    if (!is_valid_elf_header(&elf_header))
    {
        close_elf_file(fd);
        error_and_exit("Invalid ELF64 header\n", EINVAL);
    }

    // 5. Находим смещение заголовка таблицы имен сецкций
    // (чтобы знать откуда считывать заголовок таблицы имен секций)
    elf_offset = find_name_section_table_offset(&elf_header);

    // 6. Считываем заголовок таблицы имен секци
    section_header = read_section_header(fd, elf_offset);

    // 7. Ищем заголовок секции по ее имени
    section_offset = find_section_offset(fd, section_name, &elf_header, section_header.sh_offset);

    // 8. Считываем заголовок найденной секции
    section_header = read_section_header(fd, section_offset);

    // 9. Проверяем, что секция является исполняемой.
    if (!(section_header.sh_flags & SHF_EXECINSTR))
    {
        close_elf_file(fd);
        error_and_exit("Error: section is not executable\n", EINVAL);
    }

    // 10. загрузка сегментов программы в виртуальную память
    load_segments(fd, &elf_header);

    // 11. Запустить секцию
    execute_section(&section_header);

    close_elf_file(fd);

    return (0);
}
