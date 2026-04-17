# 0 "juliet_subset/CWE416_Use_After_Free/CWE416_Use_After_Free__malloc_free_char_10.c"
# 0 "<built-in>"
# 0 "<command-line>"
# 1 "juliet_subset/CWE416_Use_After_Free/CWE416_Use_After_Free__malloc_free_char_10.c"
# 18 "juliet_subset/CWE416_Use_After_Free/CWE416_Use_After_Free__malloc_free_char_10.c"
# 1 "./std_testcase.h" 1
void printLine(const char * line);
void printIntLine(int intNumber);
void printHexCharLine(char charHex);
void srand(unsigned int seed);
unsigned int time(void *ptr);
# 19 "juliet_subset/CWE416_Use_After_Free/CWE416_Use_After_Free__malloc_free_char_10.c" 2

# 1 "./wchar.h" 1
typedef int wchar_t;
# 21 "juliet_subset/CWE416_Use_After_Free/CWE416_Use_After_Free__malloc_free_char_10.c" 2



void CWE416_Use_After_Free__malloc_free_char_10_bad()
{
    char * data;

    data = ((void *)0);
    if(globalTrue)
    {
        data = (char *)malloc(100*sizeof(char));
        if (data == ((void *)0)) {exit(-1);}
        memset(data, 'A', 100-1);
        data[100-1] = '\0';

        free(data);
    }
    if(globalTrue)
    {

        printLine(data);

    }
}






static void goodB2G1()
{
    char * data;

    data = ((void *)0);
    if(globalTrue)
    {
        data = (char *)malloc(100*sizeof(char));
        if (data == ((void *)0)) {exit(-1);}
        memset(data, 'A', 100-1);
        data[100-1] = '\0';

        free(data);
    }
    if(globalFalse)
    {

        printLine("Benign, fixed string");
    }
    else
    {



        ;
    }
}


static void goodB2G2()
{
    char * data;

    data = ((void *)0);
    if(globalTrue)
    {
        data = (char *)malloc(100*sizeof(char));
        if (data == ((void *)0)) {exit(-1);}
        memset(data, 'A', 100-1);
        data[100-1] = '\0';

        free(data);
    }
    if(globalTrue)
    {



        ;
    }
}


static void goodG2B1()
{
    char * data;

    data = ((void *)0);
    if(globalFalse)
    {

        printLine("Benign, fixed string");
    }
    else
    {
        data = (char *)malloc(100*sizeof(char));
        if (data == ((void *)0)) {exit(-1);}
        memset(data, 'A', 100-1);
        data[100-1] = '\0';

    }
    if(globalTrue)
    {

        printLine(data);

    }
}


static void goodG2B2()
{
    char * data;

    data = ((void *)0);
    if(globalTrue)
    {
        data = (char *)malloc(100*sizeof(char));
        if (data == ((void *)0)) {exit(-1);}
        memset(data, 'A', 100-1);
        data[100-1] = '\0';

    }
    if(globalTrue)
    {

        printLine(data);

    }
}

void CWE416_Use_After_Free__malloc_free_char_10_good()
{
    goodB2G1();
    goodB2G2();
    goodG2B1();
    goodG2B2();
}
# 169 "juliet_subset/CWE416_Use_After_Free/CWE416_Use_After_Free__malloc_free_char_10.c"
int main(int argc, char * argv[])
{

    srand( (unsigned)time(((void *)0)) );

    printLine("Calling good()...");
    CWE416_Use_After_Free__malloc_free_char_10_good();
    printLine("Finished good()");


    printLine("Calling bad()...");
    CWE416_Use_After_Free__malloc_free_char_10_bad();
    printLine("Finished bad()");

    return 0;
}
