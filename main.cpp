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
