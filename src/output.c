
#include <stdio.h>
#include <stdarg.h>

#include "common.h"
#include "output.h"

int csv_row = 0;
int csv_header_printed = 0;

void print_csv_data(FILE * out, int columns, char *header, ...) {
    va_list args;
    int column;
    
    va_start(args, header);
    
    if (!csv_header_printed) {
        fprintf(out, "\"Array Index\",%s\n", header);
        csv_header_printed = 1;
    }
    
    printf("%i,", csv_row++);
    for(column = 0; column < columns; column++) {
        double arg = va_arg(args, double);
        fprintf(out, "%f", arg);
        if (column != columns - 1) {
            fprintf(out, ",");
        }
    }
    fprintf(out, "\n");
    
    va_end(args);
}
