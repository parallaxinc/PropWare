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

/**
 * @example     Queue_Demo.cpp
 *
 * Insert items from the user onto a Queue and then read them back at the terminal
 */
int main () {
    int array[ARRAY_SIZE];
    PropWare::Queue<int> buffer(array);

    pwOut.printf("Please enter a number at each of the following six prompts:\n");
    for (int i = 0; i < 6; ++i) {
        int x;
        pwOut << ">>> ";
        pwIn >> x;
        buffer.enqueue(x);
    }

    pwOut << "I received the following (" << ARRAY_SIZE << ") values in this order:\n";
    while (buffer.size())
        pwOut << "    " << buffer.dequeue() << "\n";
    pwOut << "The Queue instance only had space for four objects, so you'll notice that\n"
        << "the first two numbers you entered are no longer in the Queue.\n";

    return 0;
}
