#define _CRT_SECURE_NO_WARNINGS
#include "save.h"
#include <stdio.h>

bool LoadCompletion()
{
    FILE* f = fopen("save.txt", "r");

    if (!f) return false;

    int v = 0;
    fscanf(f, "%d", &v);
    fclose(f);

    return v == 1;
}

void SaveCompletion()
{
    FILE* f = fopen("save.txt", "w");

    if (!f) return;

    fprintf(f, "1");
    fclose(f);
}