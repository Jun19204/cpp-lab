#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

class Screen {
public:
  typedef std::string::size_type pos;

  Screen() = default;
  Screen(pos ht, pos wd, char c)
      : height(ht), width(wd), contents(ht * wd, c) {}

  char get() const {
    ++access_ctr;
    return contents[cursor];
  }
  inline char get(pos ht, pos wd) const;

  Screen &set(char);
  Screen &set(pos, pos, char);

  Screen &move(pos r, pos c);

  Screen &display(std::ostream &os) {
    do_display(os);
    return *this;
  }
  const Screen &display(std::ostream &os) const {
    do_display(os);
    return *this;
  }

private:
  mutable size_t access_ctr;
  pos cursor{0};
  pos height{0}, width{0};
  std::string contents;

  void do_display(std::ostream &os) const {
    ++access_ctr;
    os << contents;
  }
};
inline Screen &Screen::move(pos r, pos c) {
  ++access_ctr;
  pos row = r * width;
  cursor = row + c;
  return *this;
}
char Screen::get(pos r, pos c) const {
  ++access_ctr;
  pos row = r * width;
  return contents[row + c];
}
inline Screen &Screen::set(char c) {
  contents[cursor] = c;
  return *this;
}
inline Screen &Screen::set(pos r, pos col, char ch) {
  contents[r * width + col] = ch;
  return *this;
}

class Window_mgr {
private:
  std::vector<Screen> screens{Screen(24, 80, ' ')};
};


int main() {
  Screen myScreen(5, 5, 'X');
  myScreen.move(0, 0).set('#').display(std::cout);
  std::cout << "\n";
  myScreen.move(1, 0).set('@').display(std::cout);
  std::cout << "\n";
  return EXIT_SUCCESS;
}
