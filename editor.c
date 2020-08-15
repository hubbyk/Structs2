//
// Created by hubbyk on 31.05.2020.
//

#include "editor.h"

void writeText(PARAGRAPH* paragr, int count) {
    FILE* outputFile = fopen("result.txt", "w");

    for(int i = 0; i < count; i++) {
        fwrite((paragr + i)->buff, sizeof(unsigned char), (paragr + i)->size, outputFile);
    }

    fclose(outputFile);
}

void edit(PARAGRAPH** paragr, CONFIGURATION* configs, int* parCount, int confCount) {
    for(int i = 0; i < confCount; i++) {
        if((configs + i)->type) {
            change(paragr, (configs + i), parCount);
        }else {
            delete(paragr, (configs + i), parCount);
        }
    }
}

void updateParagraphs(PARAGRAPH** paragr, PARAGRAPH* parToConcate, int* parCount) {
    /*
     * Считаем, что один параграф все-равно должен остаться
     * Пусть даже лексически он не будет являться таковым
     */
    if(parToConcate != *paragr) {
        int i = 1;
        unsigned char *newBuf;
        //создаем новый буфер для хранения реультирующещо абзаца
        for (; i < *parCount && (*paragr + i) != parToConcate; i++);
        newBuf = (unsigned char *) calloc((*paragr + i)->size + (*paragr + i - 1)->size, sizeof(unsigned char));

        //Записываем в негосодержимое буферов двух параграфов
        memcpy(newBuf, (*paragr + i - 1)->buff, (*paragr + i - 1)->size);
        memcpy(newBuf + (*paragr + i - 1)->size, (*paragr + i)->buff, (*paragr + i)->size);
        free((*paragr + i - 1)->buff);

        //перезаписываем остальные параграфы
        (*paragr + i - 1)->buff = newBuf;
        (*paragr + i - 1)->size += (*paragr + i)->size;
        for (; i < *parCount - 1; i++) {
            memcpy(*paragr + i, *paragr + i + 1, sizeof(PARAGRAPH));
        }
        //Ну и уменьшаем счетчик
        --*parCount;
    }
}

int isNotWord(const unsigned char* ch) {
    //просто проверяем символ на принадлежность в определенному диапазону
    return (('A' > *ch || *ch > 'Z') && ('a' > *ch || *ch > 'z') && ('0' > *ch || *ch > '9') && *ch < 128);
}

int readConfigFile(CONFIGURATION** configs,  char* fileName) {
    FILE* inputFile = fopen(fileName, "rb");
    int count = 0, repeats = ALL, position = START,
            parCount = ALL, parState = COUNT, localParState = COUNT, type = DELETE,
            argLen;
    unsigned char c = '\0', buff[11] = {'\0'};

    fread(&c, 1, 1, inputFile);
    //читаем часть до аргумента
    while (!feof(inputFile)) {
        while((c == '^' || c == '/' || c == '#' || c == '@' || ('0' <= c && c <= '9')) && c != '\\' && !feof(inputFile)) {
            //если это циферки - записываем количество повторений
            if ('0' <= c && c <= '9') {
                repeats = 0;
                while('0' <= c && c <= '9') {
                    repeats = repeats * 10 + (c - 48);
                    fread(&c, 1, 1, inputFile);
                }
                continue;
            }
            //определяем тип, начало редактирования
            if(c == '/') {
                type = CHANGE;
                fread(&c, 1, 1, inputFile);
                break;
            }
            if(c == '^') {
                repeats = 1;
                position = START;
                localParState = COUNT;
            }
            if(c == '#')  {
                repeats = 1;
                position = END;
            }
            if(c == '@') localParState = NUMBER;
            fread(&c, 1, 1, inputFile);
        }
        //если это у нас указание насчет параграфов
        if(c == '\\') {
            fread(buff, 1, 10, inputFile);
            //на всякий проверяем, что конфиг написан корректно
            if(!strcmp((char*)buff, "paragraph\n")) {
                parCount = repeats;
                repeats = ALL;
                parState = localParState;
            }
            fread(&c, 1, 1, inputFile);
            continue;
        }
        //выделяем память для новой структуры конфигурвции, заполняем ее поля
        ++count;
        *configs = (CONFIGURATION*)realloc(*configs, count * sizeof(CONFIGURATION));
        (*configs + count - 1)->parCount = parCount;
        (*configs + count - 1)->parState = parState;
        (*configs + count - 1)->repeats = repeats;
        (*configs + count - 1)->position = position;
        (*configs + count - 1)->type = type;

        //читаем первый аргумент
        argLen = 1;
        (*configs + count - 1)->arg1 = (unsigned char*)calloc(1, sizeof(unsigned char));
        while (c != '\n' && c != '/' && !feof(inputFile)) {
            (*configs + count - 1)->arg1 = (unsigned char*)realloc((*configs + count - 1)->arg1, argLen * sizeof(unsigned char));
            (*configs + count - 1)->arg1[argLen - 1] = c;
            ++argLen;
            fread(&c, 1, 1, inputFile);
        }
        (*configs + count - 1)->arg1 = (unsigned char*)realloc((*configs + count - 1)->arg1, argLen * sizeof(unsigned char));
        (*configs + count - 1)->arg1[argLen - 1] = '\0';

        //если это удаление - читаем второй аргумент
        if(c == '/') {
            argLen = 1;
            fread(&c, 1, 1, inputFile);
            (*configs + count - 1)->arg2 = (unsigned char*)calloc(1, sizeof(unsigned char));
            while (c != '\n' && !feof(inputFile)) {
                (*configs + count - 1)->arg2 = (unsigned char*)realloc((*configs + count - 1)->arg2, argLen * sizeof(unsigned char));
                (*configs + count - 1)->arg2[argLen - 1] = c;
                ++argLen;
                fread(&c, 1, 1, inputFile);
            }
            (*configs + count - 1)->arg2 = (unsigned char*)realloc((*configs + count - 1)->arg2, argLen);
            (*configs + count - 1)->arg2[argLen - 1] = '\0';
        }else {
            (*configs + count - 1)->arg2 = NULL;
        }

        //восстанавливаем значения по умолчанию
        repeats = ALL, position = START,
        type = DELETE;
        fread(&c, 1, 1, inputFile);
    }

    fclose(inputFile);
    return count;
    /*
     * P.s. Почему ALL == -1
     * Патамушта в результате работы функции поиска при каждом нахождении
     * вхождения или при достижении конца параграфа именьшаются соответственные счетчики
     * выглядит это так
     * while(counter && ...) {
     *      ...какая-то логика
     *      --counter;
     * }
     * Соответсвенно, если counter == -1 - то при уменьшении счетчтка он не обратится в ноль
     */
}

int readText(PARAGRAPH** paragraphs, char* fileName) {
    FILE* inputFile = fopen(fileName, "rb");
    int count = 0, textSize;
    unsigned char* buff,* cursor;

    //узнаем размер текста
    fseek(inputFile, 0, SEEK_END);
    textSize = (int) ftell(inputFile);
    buff = (unsigned char*)calloc(textSize, sizeof(unsigned char));
    cursor = buff;

    //читаем текст в буфер - закрываем файл
    fseek(inputFile, 0, SEEK_SET);
    fread(buff, sizeof(unsigned char), textSize, inputFile);
    fclose(inputFile);

    //идем по буферу - ищем начало параграфа - записываем
    for(int i = 0; i < textSize; i++) {
        if(*cursor == '\n') {
            if(!strncmp((char*)cursor, "\n    ", 5)) {
                ++count; ++cursor;
                *paragraphs = (PARAGRAPH*)realloc(*paragraphs, count*sizeof(PARAGRAPH));

                (*paragraphs + count - 1)->buff = (unsigned char*)calloc(cursor - buff + 1, sizeof(unsigned char));
                memcpy((*paragraphs + count - 1)->buff, buff, cursor - buff);
                (*paragraphs + count - 1)->size = (int)(cursor - buff);
                buff = cursor;
            }
        }
        ++cursor;
    }
    ++count;
    *paragraphs = (PARAGRAPH*)realloc(*paragraphs, count*sizeof(PARAGRAPH));

    (*paragraphs + count - 1)->buff = (unsigned char*)calloc(cursor - buff + 1, sizeof(unsigned char));
    memcpy((*paragraphs + count - 1)->buff, buff, cursor - buff);
    (*paragraphs + count - 1)->size = (int)(cursor - buff);

    return count;
}

void delete(PARAGRAPH** paragr, CONFIGURATION* config, int* parCount) {
    unsigned char** positions; int argLen = (int)strlen((char*)config->arg1), posCount = 0;
    unsigned char* result; PARAGRAPH* parCopy = *paragr;
    int parCountCopy = *parCount;

    //ищем все нужные вхождения
    positions = (config->position)?
                searchBack(paragr, config, *parCount, &posCount):
                searchForward(paragr, config, *parCount, &posCount);

    //редактируем
    while(parCountCopy && posCount) {
        //ищем параграф, в котором есть вхождение
        while (parCountCopy && !((parCopy)->buff <= *positions && *positions <= ((parCopy)->buff + (parCopy)->size))) {
            ++parCopy; --parCountCopy;
        }
        if(parCountCopy) {
            //считаем новый размер для буфера параграфа
            unsigned char** posCursor = positions;
            int posCountCopy = posCount;
            int bufSizeCopy = (parCopy)->size;
            while(posCountCopy && (parCopy)->buff <= *posCursor && *posCursor <= ((parCopy)->buff + bufSizeCopy)) {
                (parCopy)->size -= argLen;
                ++posCursor; --posCountCopy;
            }
            //перезаписываем
            result = (unsigned char*)calloc((parCopy)->size + 1, sizeof(int16_t));
            unsigned char* newCursor = result;
            unsigned char* cursor = (parCopy)->buff;
            while(newCursor < result + (parCopy)->size) {
                if(cursor == *positions && posCount) {
                    cursor += argLen;
                    ++positions; --posCount;
                }

                *newCursor = *cursor;
                ++newCursor, ++cursor;
            }
            //смотрим - сохранил ли параграф право таковым называться
            if(result[0] == '\t' || result[0] == ' ' && result[1] == ' ' && result[2] == ' ' && result[3] == ' ') {
                free((parCopy)->buff);
                (parCopy)->buff = result;
            }else {
                (parCopy)->buff = result;
                updateParagraphs(paragr, parCopy, parCount);
            }
        }
    }
}

void change(PARAGRAPH** paragr, CONFIGURATION* config, int* parCount) { //аналогично удалению
    unsigned char** positions; int argLen = (int)strlen((char*)config->arg1), posCount = 0;
    unsigned char* result; PARAGRAPH* parCopy = *paragr;

    positions = (config->position)?
            searchBack(paragr, config, *parCount, &posCount):
            searchForward(paragr, config, *parCount, &posCount);
    int parCountCopy = *parCount;
    while(parCountCopy && posCount) {
        //skip paragraphs
        while (parCountCopy && !((parCopy)->buff <= *positions && *positions <= ((parCopy)->buff + (parCopy)->size))) {
            ++parCopy; --parCountCopy;
        }
        if(parCountCopy) {
            //count new size
            unsigned char** posCursor = positions;
            int posCountCopy = posCount;
            int bufSizeCopy = (parCopy)->size;
            while(posCountCopy && (parCopy)->buff <= *posCursor && *posCursor <= ((parCopy)->buff + bufSizeCopy)) {
                (parCopy)->size += (int)strlen((char*)config->arg2) - argLen;
                ++posCursor; --posCountCopy;
            }
            //copy
            result = (unsigned char*)calloc((parCopy)->size + 1, sizeof(unsigned char));
            unsigned char* newCursor = result;
            unsigned char* cursor = (parCopy)->buff;
            unsigned char* arg2Cursor = config->arg2;
            while(newCursor < result + (parCopy)->size) {
                if(cursor == *positions && posCount) {
                    for(int i = 0; i < strlen((char*)config->arg2); i++) {
                        *newCursor = *arg2Cursor;
                        ++newCursor; ++arg2Cursor;
                    }
                    ++positions; --posCount;
                    cursor += argLen;
                    arg2Cursor = config->arg2;
                }
                *newCursor = *cursor;
                ++newCursor, ++cursor;
            }
           if(result[0] == '\t' || result[0] == ' ' && result[1] == ' ' && result[2] == ' ' && result[3] == ' ') {
                free((parCopy)->buff);
                (parCopy)->buff = result;
            }else {
               (parCopy)->buff = result;
                updateParagraphs(paragr, parCopy, parCount);
            }
        }
    }
}

//Используется так называемфй Улучшенный поиск Бойера-Мура

unsigned char** searchForward(PARAGRAPH** paragr, CONFIGURATION* config, int paragrCount, int* count) {
    int argLen = (int)strlen((char*)config->arg1);
    int delta[256] = {argLen};
    unsigned char* argCursor = config->arg1;
    /*
     * Строим таблицу смещений
     * по умолчанию для всех букв, не входящих в подстроку
     * и для последней буквыы подстроки
     * смещение равно длине подстроки
     */
    for(int i = 0; i < 256; i++) {
        delta[i] = argLen;
    }
    /*
     * Для остальных считаем
     */
    for(int i = 0; i < argLen - 1; i++) {
        delta[*argCursor] = argLen - i - 1;
        ++argCursor;
    }
    //Очень удобный подход, при котором, используя букву как индекс массива -
    // - можем сразу же получить нужное смещение

    unsigned char** result = NULL;

    unsigned char* checkCursor;
    //определяем, в каких параграфах искать
    PARAGRAPH* parCursor = *paragr;
    if(!config->parState)  {
        parCursor += config->parCount - 1;
        paragrCount = 1;
    }
    //пока не пройден весь текст и нужное кол-во параграфов и не найдено нужное кол-во вхождений
    while(paragrCount && config->repeats && config->parCount) {
        unsigned char* bufCursor = (parCursor)->buff + (argLen - 1);
        unsigned char* bufEnd = (parCursor)->buff + (parCursor)->size;
        int check;
        //Пока не дошли до конца параграфа и не найдено нужное кол-во вхождений
        while (bufCursor < bufEnd && config->repeats) {
            check = argLen;
            while (check && bufCursor < bufEnd) {
                argCursor = config->arg1 + argLen - 1;
                checkCursor = bufCursor;
                check = argLen;

                //сравниваем подстроку
                while (check && *checkCursor == *argCursor) --checkCursor, --argCursor, --check;

                //сдвигаем
                bufCursor += delta[*bufCursor];
            }
            //проверяем на корректность с лексической точки зрения
            if (!check && !isNotWord(checkCursor)) {
                ++*count; --config->repeats;
                result = (unsigned char **) realloc(result, (*count) * sizeof(char *));
                *(result + *count - 1) = checkCursor + 1;
            }else if(!check && isNotWord(checkCursor) && isNotWord(checkCursor + argLen + 1)) {
                ++*count; --config->repeats;
                result = (unsigned char **) realloc(result, (*count) * sizeof(char *));
                *(result + *count - 1) = checkCursor + 1;
            }
        }
        --paragrCount; --config->parCount;
        ++parCursor;
    }

    return result;
}

//аналогично поиску вперед
unsigned char** searchBack(PARAGRAPH** paragr, CONFIGURATION* config, int paragrCount, int* count) {
    int argLen = (int)strlen((char*)config->arg1);
    int delta[256] = {argLen};
    unsigned char* argCursor = config->arg1 + argLen - 1;

    for(int i = 0; i < 256; i++) {
        delta[i] = argLen;
    }

    for(int i = 0; i < argLen - 1; i++) {
        delta[*argCursor] = argLen - i - 1;
        --argCursor;
    }

    unsigned char** result = NULL;

    unsigned char* checkCursor;
    PARAGRAPH* parCursor = *paragr;
    if(config->parCount < 0) {
        parCursor += paragrCount - 1;
    }else if(!config->parState)  {
        parCursor += config->parCount - 1;
        paragrCount = 1;
    }else if(config->parCount >= 0){
        parCursor += config->parCount - 1;
    }

    while(paragrCount && config->repeats && config->parCount) {
        unsigned char* bufStart = (parCursor)->buff;
        unsigned char* bufCursor = (parCursor)->buff + ((parCursor)->size - 1) - (argLen - 1);
        int check;

        while (bufCursor > bufStart && config->repeats) {
            check = argLen;
            while (check && bufCursor > bufStart) {
                argCursor = config->arg1;
                checkCursor = bufCursor;
                check = argLen;

                while (check && *checkCursor == *argCursor) ++checkCursor, ++argCursor, --check;

                bufCursor -= delta[*bufCursor];
            }
            if (!check && !isNotWord(checkCursor - argLen)) {
                ++*count; --config->repeats;
                result = (unsigned char **) realloc(result, (*count) * sizeof(char *));
                *(result + *count - 1) = checkCursor - argLen;
            }else if(!check && isNotWord(checkCursor) && isNotWord(checkCursor - argLen - 1)) {
                ++*count; --config->repeats;
                result = (unsigned char **) realloc(result, (*count) * sizeof(char *));
                *(result + *count - 1) = checkCursor - argLen;
            }
        }
        --paragrCount; --config->parCount;
        --parCursor;
    }
    unsigned char** inverseRes = (unsigned char**)calloc((*count), sizeof(char *));
    for(int i = 0, j = *count - 1; i < *count; i++, j--) {
        inverseRes[i] = result[j];
    }

    return inverseRes;
}