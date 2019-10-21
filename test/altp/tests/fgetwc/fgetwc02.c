#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <wchar.h>

#include "test_utils.h"

#define TEST_CHAR L'А'

int do_test() {
    setlocale(LC_ALL, "en_US.utf8");
    int test_result = EXIT_SUCCESS;
    char tmp_file_name[255];    
    int i = 0;
    wint_t c = TEST_CHAR;
    
    CREATE_TMP_DIR();
    
    sprintf(tmp_file_name, "%s/file", tmp_dir_name);

    FILE* pFile = fopen(tmp_file_name, "w");
    
    if(pFile != NULL) {
        for(i = 0; i < 100; i++) {
            if(fputwc(c, pFile) == EOF) {
                test_result = EXIT_FAILURE;
                break;
            }
        }
    
        if(fclose(pFile) == EOF) {
            test_result = EXIT_FAILURE;
        }
    } else {
        test_result = EXIT_FAILURE;
    }

    pFile = fopen(tmp_file_name, "r");
    
    if(pFile != NULL) {
        for(i = 0; i < 100; i++) {
            c = fgetwc(pFile);
            if(c == EOF || c != TEST_CHAR) {
                test_result = EXIT_FAILURE;
                break;
            }
        }
    
        if(fclose(pFile) == EOF) {
            test_result = EXIT_FAILURE;
        }
    } else {
        test_result = EXIT_FAILURE;
    }

    unlink(tmp_file_name);
    
    REMOVE_TMP_DIR();

    return test_result;
}