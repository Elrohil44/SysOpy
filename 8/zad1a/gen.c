//
// Created by marek on 13.05.17.
//

#include <stdio.h>
#include <time.h>
#include <stdlib.h>

int main() {
    srand(time(NULL));
    char buffer[1020];
    int id;
    FILE *data = fopen("data", "w+");
    for(int i = 1; i < 100000; ++i){
        id = i;
        for(int j = 0; j < 1020; ++j){
            buffer[j] = rand()%('z' - 'a' + 1) + 'a';
        }
        fwrite(&id, sizeof(int), 1, data);
        fwrite(buffer, sizeof(char), 1020, data);
    }
    fclose(data);
    return 0;
}
