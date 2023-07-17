#include <iostream>
#include "core/types/vstring.h"
#include "core/types/vector.h"
#include "core/types/stack.h"
#include "core/types/queue.h"

int main() {
    Queue<int> st{};
    st.enqueue(1);
    st.enqueue(2);
    st.enqueue(3);

    auto st2 = st;

    while (!st.empty())
        std::cout << st.dequeue() << std::endl;
    while (!st2.empty())
        std::cout << st2.dequeue() << std::endl;
    return 0;
}
