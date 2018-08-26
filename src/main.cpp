#include <iostream>
#include "lib/ring_buffer/ring_buffer.h"

int main(int, char**)
{
   std::cout << "sz: " << sizeof(keryx::Slot) << "\n";
   return 0;
}
