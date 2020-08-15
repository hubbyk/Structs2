//
// Created by hubbyk on 31.05.2020.
//

#ifndef STRUCTS2_EDITOR_H
#define STRUCTS2_EDITOR_H
#define ALL -1
//набор имен файлов с текстом для тестов
#define en_text_file_name "en_text.txt"
#define ru_text_file_name "ru_text.txt"
#define ru_en_text_file_name "ru-en_text.txt"
//набор имен файлов конфигурации для тестов
#define en_config_file_name "en_config.txt"
#define ru_config_file_name "ru_config.txt"
#define ru_en_config_file_name "ru-en_config.txt"

#include <malloc.h>
#include <string.h>
#include <stdint.h>

enum position {START, END};
enum parState {NUMBER, COUNT};
enum type {DELETE, CHANGE};

/*
 * Структура, описывающая конфигурацию для редактирования
 * position - говорит, с конца или с начала заданного промежутка начинать редактирование
 * repeats - количество повторений
 * parCount - номер или количество параграфов
 * psrState - указывает, как использовать parCount - как счетчик, или как номер
 * type - тип (удаление, замена)
 * arg1 - певый аргумент
 * arg2 - второй аргумент (в случае удаления принимает значение NULL)
 */

typedef struct CONFIGURATION{
    int position;
    int repeats;
    int parCount;
    int parState;

    int type;

    unsigned char* arg1;
    unsigned char* arg2;
}CONFIGURATION;

/*
 * Структура, описывающая хранение параграфа в памяти
 * size - размер, количество символов (не всегда, точнее будет -
 * сколько раз надо выполнить инкремент buf, чтобы дойти до конца буфера)
 * buff - буфер с текстом параграфа
 */

typedef struct PARAGRAPH{
    int size;
    unsigned char* buff;
}PARAGRAPH;

//записывает результат
void writeText(PARAGRAPH* paragr, int count);

//вызывает попрядку все действия редактирования
void edit(PARAGRAPH** paragr, CONFIGURATION* configs, int* parCount, int confCount);

/*
 * На случай, если будет удалено/заменено начало параграфа
 * Считается, что начало параграфа - это \t или "    ", перед которыми стоит \n
 * (за исключением 1 параграфа)
 */
void updateParagraphs(PARAGRAPH** paragr, PARAGRAPH* parToConcate, int* parCount);

/*
 * Поскольку надо удалять конкретные лиексические единицы -
 *  - необходимо проверять, что мы не удаляем часть какого-то слова
 *  Проверяет символы из буфера справа и слева от аргумента на принадлежность их к слову,
 *  как лексической единице
 *  С условиями работы все очень странно и не понятно - но очень интересно
 */
int isNotWord(const unsigned char*);

//удаляет подстроку из строки
void delete(PARAGRAPH** paragr, CONFIGURATION* config, int* parCount);

//заменяет подстроку на другую
void change(PARAGRAPH** paragr, CONFIGURATION* config, int* parCount);

//обрабатывает конфигурационный файл
int readConfigFile(CONFIGURATION**, char* fileName);

//обрабатывает файл с текстом
int readText(PARAGRAPH**, char* fileName);

//ищет нужное количество вхождений построки, начиная с начала заданного промежутка
unsigned char** searchForward(PARAGRAPH** paragr, CONFIGURATION* config, int paragrCount, int* count);

//ищет нужное количество вхождений построки, начиная с конца заданного промежутка
unsigned char** searchBack(PARAGRAPH** paragr, CONFIGURATION* config, int paragrCount, int* count);

/* Какова логика
 * Логита такова, какова некакова и больше некакова, а именно
 * Текст храним отдельными параграфами в памяти
 * Плюсы данного подхода:
 *  + просто определять параграфы, которые надо редактировать
 *  + в результате редактирования надо переписывать не весь текст - а только нужные параграфы
 *  + простая реализация функций удаления и замены
 * Минусы данного подхода:
 *  - немного усложняет функции поиска
 *  - может возникнуть необходимость перезаписать параграфы (когда их стало меньше)
 * Как говорится - сойдет
 *
 * Функции поиска занимаются разбором структуры конфигурации
 * и ищут в тексте все нужные вхождения для редактирования
 * Функциям редактирования остается лишь определить, в каких параграфах
 * находятся эти вхождения - перезаписать их и в случае необходимости вызвать
 * функцию перезаписи паграфов
 */

#endif //STRUCTS2_EDITOR_H
