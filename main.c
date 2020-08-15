#include <malloc.h>
#include "editor.h"

int main() {
    int CONFIG_COUNT, PARAGRAPH_COUNT;
    CONFIGURATION* configs = (CONFIGURATION*)malloc(sizeof(CONFIGURATION));
    PARAGRAPH* paragraphs = (PARAGRAPH*)malloc(sizeof(PARAGRAPH));

    CONFIG_COUNT = readConfigFile(&configs, ru_config_file_name);
    PARAGRAPH_COUNT = readText(&paragraphs, ru_text_file_name);

    edit(&paragraphs, configs, &PARAGRAPH_COUNT, CONFIG_COUNT);
    writeText(paragraphs, PARAGRAPH_COUNT);

    //вывод в консоль результата, пусть побудет здесь
//    for(int i = 0; i < PARAGRAPH_COUNT; i++) {
//        printf("%s", (paragraphs + i)->buff);
//    }
    return 0;
}
