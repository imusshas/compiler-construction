/* Teaching example: remove comments and collapse whitespace to one space.
   Scope: C input without string/character literals or preprocessor directives.
   Run: ./lexical_analyzer source.c
   This demonstrates character scanning, not complete token classification. */
#include <stdio.h>                  // Provides file reading and output functions.
#include <ctype.h>                  // Provides isspace() to recognize whitespace.

/* main opens the input file, scans it character by character, and prints clean text. */
int main(int argc, char **argv)     // Receive the command-line argument count and values.
{                                  // Begin the main function.
    if (argc != 2) {               // Require exactly one input filename.
        fprintf(stderr, "Usage: %s source.c\n", argv[0]); // Explain correct usage.
        return 1;                  // Stop because the filename is missing or extra arguments exist.
    }                              // Finish checking the arguments.
    FILE *file = fopen(argv[1], "r"); // Open the specified source file for reading.
    if (file == NULL) {             // Check whether opening the file failed.
        perror("Cannot open input"); // Print the reason for the file-open failure.
        return 1;                  // Stop with an error status.
    }                              // Finish checking the file.
    int c, next, previous;         // Store characters as int so EOF remains distinguishable.
    int gap = 0, printed = 0;      // Track a pending space and whether output has started.
    while ((c = fgetc(file)) != EOF) { // Read one character until the file ends.
        if (c == '/') {            // A slash may start a comment or represent division.
            next = fgetc(file);    // Read one more character to decide which case applies.
            if (next == '/') {     // Two slashes begin a single-line comment.
                while ((c = fgetc(file)) != EOF && c != '\n') {} // Skip through the line.
                gap = 1;           // Keep a separator where the comment was removed.
                continue;          // Read the next source character without printing this comment.
            }                      // Finish handling a single-line comment.
            if (next == '*') {     // Slash followed by star begins a block comment.
                previous = 0;      // Start without a previous comment character.
                while ((c = fgetc(file)) != EOF) { // Read characters inside the comment.
                    if (previous == '*' && c == '/') break; // Stop at the closing star-slash.
                    previous = c;  // Remember this character for the next closing-marker check.
                }                  // Finish skipping the block comment.
                if (c == EOF) {    // Reaching EOF here means the block comment never closed.
                    fprintf(stderr, "Unclosed block comment\n"); // Report malformed input.
                    fclose(file);  // Close the input before stopping.
                    return 1;      // Stop with an error status.
                }                  // Finish checking for an unclosed comment.
                gap = 1;           // Prevent words or operators from joining across the comment.
                continue;          // Resume scanning after the block comment.
            }                      // Finish handling a block comment.
            if (next != EOF) ungetc(next, file); // Put back look-ahead when slash is not a comment.
        }                          // Finish examining the slash; ordinary division still gets printed.
        if (isspace((unsigned char)c)) { // Check for a space, tab, newline, or other whitespace.
            gap = 1;               // Remember that a separator is needed, regardless of gap length.
            continue;              // Skip the original whitespace character.
        }                          // Finish handling whitespace.
        if (gap && printed) putchar(' '); // Print one separator, but never a leading space.
        putchar(c);                // Print the current non-comment, non-whitespace character.
        gap = 0;                   // Clear the pending separator after printing a character.
        printed = 1;               // Remember that output has started.
    }                              // Finish scanning; a pending trailing space is never printed.
    int failed = ferror(file);     // Check whether reading stopped because of a file error.
    fclose(file);                  // Release the input file.
    if (printed) putchar('\n');    // End nonempty cleaned output with one newline.
    return failed ? 1 : 0;         // Return failure for a read error, otherwise report success.
}                                  // End the main function.
