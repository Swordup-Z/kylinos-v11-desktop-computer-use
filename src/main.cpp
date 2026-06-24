#include <linux/uinput.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <cerrno>
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <map>
#include <string>
#include <thread>

namespace {

void usage()
{
    std::cerr
        << "Usage:\n"
        << "  kylinos-v11-desktop-computer-use move <dx> <dy>\n"
        << "  kylinos-v11-desktop-computer-use down [button]\n"
        << "  kylinos-v11-desktop-computer-use up [button]\n"
        << "  kylinos-v11-desktop-computer-use click [button]\n"
        << "  kylinos-v11-desktop-computer-use double-click [button]\n"
        << "  kylinos-v11-desktop-computer-use drag <dx> <dy> [button] [duration-ms]\n"
        << "  kylinos-v11-desktop-computer-use scroll <steps>\n"
        << "  kylinos-v11-desktop-computer-use key <key>\n"
        << "  kylinos-v11-desktop-computer-use type <text>\n"
        << "  kylinos-v11-desktop-computer-use script <commands...>\n"
        << "  kylinos-v11-desktop-computer-use sleep <ms>\n";
}

int toInt(const char *value, const char *name)
{
    char *end = nullptr;
    const long parsed = std::strtol(value, &end, 10);
    if (!end || *end != '\0') {
        std::cerr << "Invalid " << name << ": " << value << "\n";
        std::exit(2);
    }
    return static_cast<int>(parsed);
}

int buttonCode(int button)
{
    switch (button) {
    case 1:
        return BTN_LEFT;
    case 2:
        return BTN_MIDDLE;
    case 3:
        return BTN_RIGHT;
    default:
        std::cerr << "Unsupported mouse button: " << button << "\n";
        std::exit(2);
    }
}

struct KeyStroke {
    int code = 0;
    bool shift = false;
};

const std::map<std::string, KeyStroke> &namedKeys()
{
    static const std::map<std::string, KeyStroke> keys = {
        {"BackSpace", {KEY_BACKSPACE, false}},
        {"Delete", {KEY_DELETE, false}},
        {"Down", {KEY_DOWN, false}},
        {"End", {KEY_END, false}},
        {"Enter", {KEY_ENTER, false}},
        {"Escape", {KEY_ESC, false}},
        {"Home", {KEY_HOME, false}},
        {"Left", {KEY_LEFT, false}},
        {"Page_Down", {KEY_PAGEDOWN, false}},
        {"Page_Up", {KEY_PAGEUP, false}},
        {"Return", {KEY_ENTER, false}},
        {"Right", {KEY_RIGHT, false}},
        {"Space", {KEY_SPACE, false}},
        {"Tab", {KEY_TAB, false}},
        {"Up", {KEY_UP, false}},
        {"backspace", {KEY_BACKSPACE, false}},
        {"delete", {KEY_DELETE, false}},
        {"down", {KEY_DOWN, false}},
        {"end", {KEY_END, false}},
        {"enter", {KEY_ENTER, false}},
        {"escape", {KEY_ESC, false}},
        {"home", {KEY_HOME, false}},
        {"left", {KEY_LEFT, false}},
        {"pagedown", {KEY_PAGEDOWN, false}},
        {"pageup", {KEY_PAGEUP, false}},
        {"return", {KEY_ENTER, false}},
        {"right", {KEY_RIGHT, false}},
        {"space", {KEY_SPACE, false}},
        {"tab", {KEY_TAB, false}},
        {"up", {KEY_UP, false}},
    };
    return keys;
}

int letterKeyCode(char lower)
{
    switch (lower) {
    case 'a':
        return KEY_A;
    case 'b':
        return KEY_B;
    case 'c':
        return KEY_C;
    case 'd':
        return KEY_D;
    case 'e':
        return KEY_E;
    case 'f':
        return KEY_F;
    case 'g':
        return KEY_G;
    case 'h':
        return KEY_H;
    case 'i':
        return KEY_I;
    case 'j':
        return KEY_J;
    case 'k':
        return KEY_K;
    case 'l':
        return KEY_L;
    case 'm':
        return KEY_M;
    case 'n':
        return KEY_N;
    case 'o':
        return KEY_O;
    case 'p':
        return KEY_P;
    case 'q':
        return KEY_Q;
    case 'r':
        return KEY_R;
    case 's':
        return KEY_S;
    case 't':
        return KEY_T;
    case 'u':
        return KEY_U;
    case 'v':
        return KEY_V;
    case 'w':
        return KEY_W;
    case 'x':
        return KEY_X;
    case 'y':
        return KEY_Y;
    case 'z':
        return KEY_Z;
    default:
        std::cerr << "Invalid letter: " << lower << "\n";
        std::exit(6);
    }
}

KeyStroke asciiKeyStroke(char c)
{
    if (c >= 'a' && c <= 'z') {
        return {letterKeyCode(c), false};
    }
    if (c >= 'A' && c <= 'Z') {
        return {letterKeyCode(static_cast<char>(c - 'A' + 'a')), true};
    }
    if (c >= '1' && c <= '9') {
        return {KEY_1 + (c - '1'), false};
    }
    if (c == '0') {
        return {KEY_0, false};
    }

    switch (c) {
    case ' ':
        return {KEY_SPACE, false};
    case '\n':
        return {KEY_ENTER, false};
    case '\t':
        return {KEY_TAB, false};
    case '-':
        return {KEY_MINUS, false};
    case '_':
        return {KEY_MINUS, true};
    case '=':
        return {KEY_EQUAL, false};
    case '+':
        return {KEY_EQUAL, true};
    case '[':
        return {KEY_LEFTBRACE, false};
    case '{':
        return {KEY_LEFTBRACE, true};
    case ']':
        return {KEY_RIGHTBRACE, false};
    case '}':
        return {KEY_RIGHTBRACE, true};
    case '\\':
        return {KEY_BACKSLASH, false};
    case '|':
        return {KEY_BACKSLASH, true};
    case ';':
        return {KEY_SEMICOLON, false};
    case ':':
        return {KEY_SEMICOLON, true};
    case '\'':
        return {KEY_APOSTROPHE, false};
    case '"':
        return {KEY_APOSTROPHE, true};
    case '`':
        return {KEY_GRAVE, false};
    case '~':
        return {KEY_GRAVE, true};
    case ',':
        return {KEY_COMMA, false};
    case '<':
        return {KEY_COMMA, true};
    case '.':
        return {KEY_DOT, false};
    case '>':
        return {KEY_DOT, true};
    case '/':
        return {KEY_SLASH, false};
    case '?':
        return {KEY_SLASH, true};
    case '!':
        return {KEY_1, true};
    case '@':
        return {KEY_2, true};
    case '#':
        return {KEY_3, true};
    case '$':
        return {KEY_4, true};
    case '%':
        return {KEY_5, true};
    case '^':
        return {KEY_6, true};
    case '&':
        return {KEY_7, true};
    case '*':
        return {KEY_8, true};
    case '(':
        return {KEY_9, true};
    case ')':
        return {KEY_0, true};
    default:
        std::cerr << "Only common ASCII text input is supported for now.\n";
        std::exit(6);
    }
}

KeyStroke keyStrokeFromName(const std::string &name)
{
    const auto named = namedKeys().find(name);
    if (named != namedKeys().end()) {
        return named->second;
    }
    if (name.size() == 1) {
        return asciiKeyStroke(name[0]);
    }
    std::cerr << "Unknown key: " << name << "\n";
    std::exit(5);
}

class UinputDevice {
public:
    UinputDevice()
    {
        fd_ = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
        if (fd_ < 0) {
            std::cerr << "Cannot open /dev/uinput: " << std::strerror(errno) << "\n";
            std::exit(9);
        }

        enable(EV_KEY);
        enable(EV_REL);
        enableRel(REL_X);
        enableRel(REL_Y);
        enableRel(REL_WHEEL);
        enableKey(BTN_LEFT);
        enableKey(BTN_MIDDLE);
        enableKey(BTN_RIGHT);
        enableKey(KEY_LEFTSHIFT);

        for (int code = KEY_1; code <= KEY_0; ++code) {
            enableKey(code);
        }
        for (int code = KEY_Q; code <= KEY_P; ++code) {
            enableKey(code);
        }
        for (int code = KEY_A; code <= KEY_L; ++code) {
            enableKey(code);
        }
        for (int code = KEY_Z; code <= KEY_M; ++code) {
            enableKey(code);
        }
        for (const int code : {KEY_SPACE,
                               KEY_ENTER,
                               KEY_TAB,
                               KEY_BACKSPACE,
                               KEY_ESC,
                               KEY_DELETE,
                               KEY_LEFT,
                               KEY_RIGHT,
                               KEY_UP,
                               KEY_DOWN,
                               KEY_HOME,
                               KEY_END,
                               KEY_PAGEUP,
                               KEY_PAGEDOWN,
                               KEY_MINUS,
                               KEY_EQUAL,
                               KEY_LEFTBRACE,
                               KEY_RIGHTBRACE,
                               KEY_BACKSLASH,
                               KEY_SEMICOLON,
                               KEY_APOSTROPHE,
                               KEY_GRAVE,
                               KEY_COMMA,
                               KEY_DOT,
                               KEY_SLASH}) {
            enableKey(code);
        }

        uinput_user_dev device {};
        std::snprintf(device.name, UINPUT_MAX_NAME_SIZE, "kylinos computer use");
        device.id.bustype = BUS_USB;
        device.id.vendor = 0x1209;
        device.id.product = 0x2026;
        device.id.version = 1;
        if (write(fd_, &device, sizeof(device)) != sizeof(device)) {
            fail("write uinput device");
        }
        if (ioctl(fd_, UI_DEV_CREATE) < 0) {
            fail("create uinput device");
        }
        created_ = true;
        std::this_thread::sleep_for(std::chrono::milliseconds(120));
    }

    ~UinputDevice()
    {
        if (created_) {
            ioctl(fd_, UI_DEV_DESTROY);
        }
        if (fd_ >= 0) {
            close(fd_);
        }
    }

    void move(int dx, int dy)
    {
        if (dx != 0) {
            emit(EV_REL, REL_X, dx);
        }
        if (dy != 0) {
            emit(EV_REL, REL_Y, dy);
        }
        sync();
    }

    void button(int button, bool pressed)
    {
        emit(EV_KEY, buttonCode(button), pressed ? 1 : 0);
        sync();
    }

    void click(int button)
    {
        this->button(button, true);
        this->button(button, false);
    }

    void scroll(int steps)
    {
        const int direction = steps > 0 ? -1 : 1;
        for (int i = 0; i < std::abs(steps); ++i) {
            emit(EV_REL, REL_WHEEL, direction);
            sync();
            std::this_thread::sleep_for(std::chrono::milliseconds(28));
        }
    }

    void key(const KeyStroke &stroke)
    {
        if (stroke.shift) {
            emit(EV_KEY, KEY_LEFTSHIFT, 1);
        }
        emit(EV_KEY, stroke.code, 1);
        emit(EV_KEY, stroke.code, 0);
        if (stroke.shift) {
            emit(EV_KEY, KEY_LEFTSHIFT, 0);
        }
        sync();
    }

private:
    void enable(int type)
    {
        if (ioctl(fd_, UI_SET_EVBIT, type) < 0) {
            fail("enable uinput event type");
        }
    }

    void enableRel(int code)
    {
        if (ioctl(fd_, UI_SET_RELBIT, code) < 0) {
            fail("enable uinput relative axis");
        }
    }

    void enableKey(int code)
    {
        if (ioctl(fd_, UI_SET_KEYBIT, code) < 0) {
            fail("enable uinput key");
        }
    }

    void emit(std::uint16_t type, std::uint16_t code, std::int32_t value)
    {
        input_event event {};
        event.type = type;
        event.code = code;
        event.value = value;
        if (write(fd_, &event, sizeof(event)) != sizeof(event)) {
            fail("write uinput event");
        }
    }

    void sync()
    {
        emit(EV_SYN, SYN_REPORT, 0);
    }

    [[noreturn]] void fail(const char *operation) const
    {
        std::cerr << "Failed to " << operation << ": " << std::strerror(errno) << "\n";
        std::exit(9);
    }

    int fd_ = -1;
    bool created_ = false;
};

void drag(UinputDevice &device, int dx, int dy, int button, int durationMs)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(40));
    device.button(button, true);
    std::this_thread::sleep_for(std::chrono::milliseconds(80));

    const int steps = std::max(16, durationMs / 16);
    const int stepDelay = std::max(8, durationMs / steps);
    int previousX = 0;
    int previousY = 0;
    for (int i = 1; i <= steps; ++i) {
        const int x = (dx * i) / steps;
        const int y = (dy * i) / steps;
        device.move(x - previousX, y - previousY);
        previousX = x;
        previousY = y;
        std::this_thread::sleep_for(std::chrono::milliseconds(stepDelay));
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(40));
    device.button(button, false);
}

void typeText(UinputDevice &device, const std::string &text)
{
    for (const char c : text) {
        device.key(asciiKeyStroke(c));
        std::this_thread::sleep_for(std::chrono::milliseconds(18));
    }
}

void runScript(UinputDevice &device, int argc, char **argv)
{
    int index = 0;
    while (index < argc) {
        const std::string command = argv[index++];
        if (command == "move") {
            if (index + 2 > argc) {
                usage();
                std::exit(2);
            }
            device.move(toInt(argv[index], "dx"), toInt(argv[index + 1], "dy"));
            index += 2;
        } else if (command == "down" || command == "up") {
            if (index + 1 > argc) {
                usage();
                std::exit(2);
            }
            const int button = toInt(argv[index], "button");
            ++index;
            device.button(button, command == "down");
        } else if (command == "click" || command == "double-click") {
            if (index + 1 > argc) {
                usage();
                std::exit(2);
            }
            const int button = toInt(argv[index], "button");
            ++index;
            device.click(button);
            if (command == "double-click") {
                std::this_thread::sleep_for(std::chrono::milliseconds(80));
                device.click(button);
            }
        } else if (command == "drag") {
            if (index + 3 > argc) {
                usage();
                std::exit(2);
            }
            const int dx = toInt(argv[index], "dx");
            const int dy = toInt(argv[index + 1], "dy");
            const int button = toInt(argv[index + 2], "button");
            int durationMs = 700;
            index += 3;
            if (index < argc) {
                const std::string maybeDuration = argv[index];
                if (!maybeDuration.empty() && maybeDuration.find_first_not_of("0123456789") == std::string::npos) {
                    durationMs = toInt(argv[index], "duration");
                    ++index;
                }
            }
            drag(device, dx, dy, button, durationMs);
        } else if (command == "scroll") {
            if (index + 1 > argc) {
                usage();
                std::exit(2);
            }
            device.scroll(toInt(argv[index], "steps"));
            ++index;
        } else if (command == "key") {
            if (index + 1 > argc) {
                usage();
                std::exit(2);
            }
            device.key(keyStrokeFromName(argv[index]));
            ++index;
        } else if (command == "type") {
            if (index + 1 > argc) {
                usage();
                std::exit(2);
            }
            typeText(device, argv[index]);
            ++index;
        } else if (command == "sleep") {
            if (index + 1 > argc) {
                usage();
                std::exit(2);
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(toInt(argv[index], "milliseconds")));
            ++index;
        } else {
            std::cerr << "Unknown script command: " << command << "\n";
            std::exit(2);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(30));
    }
}

} // namespace

int main(int argc, char **argv)
{
    if (argc < 2) {
        usage();
        return 2;
    }

    const std::string command = argv[1];
    if (command == "sleep") {
        if (argc != 3) {
            usage();
            return 2;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(toInt(argv[2], "milliseconds")));
        return 0;
    }

    UinputDevice device;
    if (command == "move") {
        if (argc != 4) {
            usage();
            return 2;
        }
        device.move(toInt(argv[2], "dx"), toInt(argv[3], "dy"));
        return 0;
    }
    if (command == "down" || command == "up") {
        if (argc != 2 && argc != 3) {
            usage();
            return 2;
        }
        const int button = argc == 3 ? toInt(argv[2], "button") : 1;
        device.button(button, command == "down");
        return 0;
    }
    if (command == "click" || command == "double-click") {
        if (argc != 2 && argc != 3) {
            usage();
            return 2;
        }
        const int button = argc == 3 ? toInt(argv[2], "button") : 1;
        device.click(button);
        if (command == "double-click") {
            std::this_thread::sleep_for(std::chrono::milliseconds(80));
            device.click(button);
        }
        return 0;
    }
    if (command == "drag") {
        if (argc != 4 && argc != 5 && argc != 6) {
            usage();
            return 2;
        }
        const int button = argc == 5 ? toInt(argv[4], "button") : 1;
        const int durationMs = argc == 6 ? toInt(argv[5], "duration") : 700;
        drag(device, toInt(argv[2], "dx"), toInt(argv[3], "dy"), button, durationMs);
        return 0;
    }
    if (command == "scroll") {
        if (argc != 3) {
            usage();
            return 2;
        }
        device.scroll(toInt(argv[2], "steps"));
        return 0;
    }
    if (command == "key") {
        if (argc != 3) {
            usage();
            return 2;
        }
        device.key(keyStrokeFromName(argv[2]));
        return 0;
    }
    if (command == "type") {
        if (argc != 3) {
            usage();
            return 2;
        }
        typeText(device, argv[2]);
        return 0;
    }
    if (command == "script") {
        if (argc < 3) {
            usage();
            return 2;
        }
        runScript(device, argc - 2, argv + 2);
        return 0;
    }

    usage();
    return 2;
}
