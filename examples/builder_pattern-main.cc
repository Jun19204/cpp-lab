#include <cstdlib>
#include <iostream>
#include <string>

class Window {
public:
  // 1. 설정 데이터를 담을 구조체나 Window 내부 멤버
  struct Config {
    std::string title = "Untitled";
    int width = 800;
    int height = 600;
    bool fullscreen = false;
    bool resizable = true;
  };


  // 2. Builder 클래스 정의
  class Builder {
  private:
    Config config_;
  public:
    Builder() = default;
    // 각 설정 메서드는 자기 자신의 참조를 반환 (Method Chaining)
    Builder &setTitle(std::string title) {
      config_.title = std::move(title);
      return *this;
    }
    Builder &setSize(int width, int height) {
      config_.width = width;
      config_.height = height;
      return *this;
    }
    Builder &setFullscreen(bool enabled) {
      config_.fullscreen = enabled;
      return *this;
    }
    Builder &setResizable(bool enabled) {
      config_.resizable = enabled;
      return *this;
    }
    // 최종 객체 생성
    [[nodiscard]]
    Window build() const { return Window(config_); }
  };


  void print_info() const {
    std::cout << title_ << " (" << width_ << "x" << height_
              << ") - Fullscreen: " << fullscreen_
              << ", Resizable: " << resizable_ << "\n";
  }

private:
  // Builder 통해서만 생성하도록 private 생성자 활용 가능
  explicit Window(Config config)
      : title_(std::move(config.title)), width_(config.width),
        height_(config.height), fullscreen_(config.fullscreen),
        resizable_(config.resizable) {}

  std::string title_;
  int width_;
  int height_;
  bool fullscreen_;
  bool resizable_;
};




int main() {
  // 순서에 의존하지 않고, 중간 옵션(setResizable)만 쏙 골라서 설정 가능!
  Window win =
      Window::Builder().setTitle("Custom Window").setResizable(false).build();

  win.print_info();

  return EXIT_SUCCESS;
}

