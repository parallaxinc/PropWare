/**
 * @file    Queue_Demo.cpp
 *
 * @author  David Zemon
 */

#include <PropWare/PropWare.h>
#include <PropWare/queue.h>
#include <PropWare/printer/printer.h>
#include <PropWare/scanner.h>

static const size_t ARRAY_SIZE = 4;

// Main function
int main () {
    int array[ARRAY_SIZE];
    PropWare::Queue<int> buffer(array);

    pwOut.printf("Please enter a number at each of the following four prompts:\n");
    for (int i = 0; i < 6; ++i) {
        int x;
        pwOut.printf(">>> ");
        pwIn >> x;
        buffer.enqueue(x);
    }

    pwOut.printf("I received the following (%d) values in this order:\n", ARRAY_SIZE);
    while (buffer.size())
        pwOut.printf("    %d\n", buffer.dequeue());

    return 0;
}
