#include "constants.h"
#include <algorithm>
#include <array>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <termios.h>
#include <unistd.h>
#include <vector>

// class Game
// {
// };

class Object
{
  private:
    size_t m_x{0};                 // x-coordinate of left-top corner of object
    size_t m_y{0};                 // y-coordinate of left-top corner of object
    const std::string m_shape{""}; // string of shape that contains '\n'
    std::string m_rshape{""};      // reverse of shape
    size_t m_length{1};
    size_t m_height{1};
    bool m_reverse{false};

    void reverse_shape()
    {
        std::vector<char> line{};
        for (char c : m_shape) {
            if (c == '(') {
                c = ')';
            } else if (c == ')') {
                c = '(';
            } else if (c == '/') {
                c = '\\';
            } else if (c == '\\') {
                c = '/';
            } else if (c == '<') {
                c = '>';
            } else if (c == '>') {
                c = '<';
            }
            line.push_back(c);
            if (c == '\n') {
                std::reverse(line.begin(), line.end());
                for (char ch : line) {
                    m_rshape += ch;
                }
                line.clear();
            }
        }
        std::reverse(line.begin(), line.end());
        for (char ch : line) {
            m_rshape += ch;
        }
    }

  public:
    Object(size_t x, size_t y, const std::string shape)
        : m_x{x}, m_y{y}, m_shape{shape}, m_rshape{""}, m_reverse{false}
    {
        size_t len{0};
        size_t height{1};
        for (const char chr : shape) {
            if (chr == '\n') {
                ++height;
                len = 0;
                continue;
            }
            ++len;
        }
        m_length = len;
        m_height = height;
        reverse_shape();
    }

    void add_x(const int n)
    {
        int current_x = (int)m_x;
        if (current_x + n <= 0) {
            current_x = 0;
        } else if (current_x + n >= 100) {
            current_x = 100;
        } else {
            current_x += n;
        }
        m_x = (size_t)current_x;
    }

    void add_y(const int n)
    {
        int current_y = (int)m_y;
        if (current_y + n <= 0) {
            current_y = 0;
        } else if (current_y + n >= 80) {
            current_y = 80;
        } else {
            current_y += n;
        }
        m_y = (size_t)current_y;
    }
    void set_reverse(bool reverse) { m_reverse = reverse; }
    bool is_reverse() { return m_reverse; }
    size_t get_x() const { return m_x; }
    size_t get_y() const { return m_y; }
    size_t get_length() const { return m_length; }
    size_t get_height() const { return m_height; }
    std::string get_shape() const { return m_shape; }
    std::string get_rshape() const { return m_rshape; }
};

class Map
{
  private:
    size_t m_height{constants::screen_height};
    size_t m_width{constants::screen_width};
    std::array<std::string, constants::screen_height> m_map_default{""};
    std::array<std::string, constants::screen_height> m_map{""};
    std::vector<std::shared_ptr<Object>> m_objects{};

    void update()
    {
        m_map = m_map_default;
        for (auto object : m_objects) {
            size_t y{object->get_y()};
            size_t x{object->get_x()};
            size_t height{object->get_height()};
            size_t length{object->get_length()};
            size_t idx{0};
            std::string shape = object->is_reverse() ? object->get_rshape()
                                                     : object->get_shape();
            for (size_t i = y; i < y + height; ++i) {
                for (size_t j = x; j < x + length; ++j) {
                    const char chr =
                        (shape[idx] == '\n') ? shape[++idx] : shape[idx];

                    m_map[i][j] = chr;
                    ++idx;
                }
            }
        }
    }

    void clear_screen() { std::cout << "\033[2J\033[1;1H"; }

  public:
    Map() : m_height{constants::screen_height}, m_width{constants::screen_width}
    {
        for (size_t i = 0; i < m_height; ++i) {
            for (size_t j = 0; j < m_width; ++j) {
                m_map_default[i].append(" ");
            }
        }
    }

    int load_file(const std::string filename)
    {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Error: Could not open the file" << std::endl;
            return -1;
        }
        std::string line{};
        size_t n{10};
        size_t start_idx{0};
        while (std::getline(file, line)) {
            start_idx = (m_width - line.length()) / 2;
            m_map_default[n].replace(start_idx, line.length(), line);
            ++n;
            if (n > m_height) {
                std::cerr << "Error: the map loaded from file is too big"
                          << std::endl;
                return -1;
            }
        }
        file.close();
        return (int)start_idx;
    }

    void draw()
    {
        update();
        clear_screen();
        for (size_t i = 0; i < m_height; ++i) {
            std::cout << m_map[i] << "\n";
        }
    }
    void add(std::shared_ptr<Object> object) { m_objects.push_back(object); }
};

void set_raw_mode(bool enable)
{
    static termios oldt, newt;
    if (enable) {
        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        newt.c_lflag &= ~static_cast<tcflag_t>(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    } else {
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt); // restore
    }
}

int main()
{
    Map stage1 = Map();
    stage1.load_file("maps/stage1.txt");
    auto bird =
        std::make_shared<Object>(0, 0, "  I   \n>(-)//\n  (  )\n   m m");
    auto pig = std::make_shared<Object>(
        20, 0, "  III   \n&(;.;)& \n  (   )~\n   m m  ");
    stage1.add(bird);
    stage1.add(pig);
    stage1.draw();
    set_raw_mode(true);
    char c;
    while (std::cin.get(c)) {
        if (c == '\x1b') { // ESC
            char seq[2]{};
            if (!std::cin.get(seq[0]))
                break;
            if (!std::cin.get(seq[1]))
                break;

            if (seq[0] == '[') {
                switch (seq[1]) {
                case 'A': // when press "UP"
                    pig->add_y(-1);
                    stage1.draw();
                    break;
                case 'B': // when press "DOWN"
                    pig->add_y(1);
                    stage1.draw();
                    break;
                case 'C': // when press "RIGHT"
                    pig->set_reverse(true);
                    pig->add_x(1);
                    stage1.draw();
                    break;
                case 'D': // when press "LEFT"
                    pig->set_reverse(false);
                    pig->add_x(-1);
                    stage1.draw();
                    break;
                default:
                    break; // unrecognized escape sequence
                }
            }
        }
    }
    set_raw_mode(false);
    return 0;
}
