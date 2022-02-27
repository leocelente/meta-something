# Meta something

Started as a dive into C++ Reflection, ended up as a little bit of fun in code generation.

I got curious by the [HubrisOS](https://github.com/oxidecomputer/hubris), it uses a configuration file that describes the system's processes.
The role of the OS is to give its best effort to keep the system on that state. Then a I thought:
what if configuration and behavior could be represented in a single place. That means that 
the compile-time configuration and data need to affect the system's function. So maybe with 
the use of reflection in C++ (that I had briefly heard of) I could construct a class that contains
all that I need, from that the rest of the program can be generated in compile-time. Inspired by 
[crect](https://github.com/korken89/crect), I dived into reflection libraries ([metacpp](https://github.com/RamblingMadMan/metacpp), [reflang](https://github.com/chakaz/reflang/), [refl-cpp](https://github.com/veselink1/refl-cpp)) and clang's experimental branch. But nothing seemed to work easily and while researching how they worked I saw that they generate code containing metadata with the help of [libclang](https://clang.llvm.org/doxygen/index.html). So eventually, after a few days without much success I started 
to build my own library that generates code from code. 

Basically the goal is, given:
```c++
#include "main.gen.cpp"

struct MySystem {
  int resource;
  float resource2;
  void task() {}

  void task2() {}
};

int main() {
  auto system = Generate<MySystem>();
  return system();
}
```

The special library will populate the file `main.gen.cpp` with a scheduler that runs all methods 
as tasks. After that is basically done I'll look into adding special parameters as custom attributes that adapt the generated code. Imagine: 

```c++
#include "main.gen.cpp"

struct MySystem {
  void task0() [[isr, address(0x80)]] {}
  void task1() [[run_once, timeout(100ms)]] {}
  void task2() [[restart_on_failure]] {}
};
```

One of the goals is to be as transparent as possible so that the framework can be easily ported to different systems, with special attention to embedded devices.
