/* A tiny valid C program with extra whitespace. */

int     main(void)
{
    int     first  =  4;       // The first value.
    int     second =  6;       // The second value.

    /* Add the two values.
       This comment uses two lines. */
    int/* Keep the type and name separate. */total = first + second;

    return     total - 10;     // Return zero when the sum is ten.
}
