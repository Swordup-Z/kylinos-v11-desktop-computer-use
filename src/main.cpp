#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/XTest.h>

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

namespace {

struct DisplayCloser {
    void operator()(Display *display) const
    {
        if (display) {
            XCloseDisplay(display);
        }
    }
};

using DisplayPtr = std::unique_ptr<Display, DisplayCloser>;

void usage()
{
    std::cerr
        << "Usage:\n"
        << "  kylinos-v11-desktop-computer-use screen\n"
        << "  kylinos-v11-desktop-computer-use position\n"
        << "  kylinos-v11-desktop-computer-use move <x> <y>\n"
        << "  kylinos-v11-desktop-computer-use click <x> <y> [button]\n"
        << "  kylinos-v11-desktop-computer-use double-click <x> <y> [button]\n"
        << "  kylinos-v11-desktop-computer-use key <keysym>\n"
        << "  kylinos-v11-desktop-computer-use type <text>\n"
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

DisplayPtr openDisplay()
{
    Display *display = XOpenDisplay(nullptr);
    if (!display) {
        std::cerr << "Cannot open X display. Check DISPLAY and use an X11/XCB session.\n";
        std::exit(3);
    }
    int eventBase = 0;
    int errorBase = 0;
    int major = 0;
    int minor = 0;
    if (!XTestQueryExtension(display, &eventBase, &errorBase, &major, &minor)) {
        std::cerr << "XTest extension is not available on this display.\n";
        std::exit(4);
    }
    return DisplayPtr(display);
}

void flush(Display *display)
{
    XFlush(display);
    XSync(display, False);
}

void movePointer(Display *display, int x, int y)
{
    XTestFakeMotionEvent(display, DefaultScreen(display), x, y, CurrentTime);
    flush(display);
}

void clickButton(Display *display, int button)
{
    XTestFakeButtonEvent(display, static_cast<unsigned int>(button), True, CurrentTime);
    XTestFakeButtonEvent(display, static_cast<unsigned int>(button), False, CurrentTime);
    flush(display);
}

void typeKey(Display *display, const std::string &keysymName)
{
    KeySym keysym = XStringToKeysym(keysymName.c_str());
    if (keysym == NoSymbol && keysymName.size() == 1) {
        keysym = XStringToKeysym(std::string(1, keysymName[0]).c_str());
    }
    if (keysym == NoSymbol) {
        std::cerr << "Unknown keysym: " << keysymName << "\n";
        std::exit(5);
    }
    const KeyCode code = XKeysymToKeycode(display, keysym);
    if (code == 0) {
        std::cerr << "No keycode for keysym: " << keysymName << "\n";
        std::exit(5);
    }

    const bool upper = keysymName.size() == 1 && keysymName[0] >= 'A' && keysymName[0] <= 'Z';
    const KeyCode shift = XKeysymToKeycode(display, XK_Shift_L);
    if (upper && shift != 0) {
        XTestFakeKeyEvent(display, shift, True, CurrentTime);
    }
    XTestFakeKeyEvent(display, code, True, CurrentTime);
    XTestFakeKeyEvent(display, code, False, CurrentTime);
    if (upper && shift != 0) {
        XTestFakeKeyEvent(display, shift, False, CurrentTime);
    }
    flush(display);
}

void typeText(Display *display, const std::string &text)
{
    for (const unsigned char c : text) {
        if (c == '\n') {
            typeKey(display, "Return");
        } else if (c == '\t') {
            typeKey(display, "Tab");
        } else if (c >= 32 && c <= 126) {
            typeKey(display, std::string(1, static_cast<char>(c)));
        } else {
            std::cerr << "Only ASCII text input is supported for now.\n";
            std::exit(6);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(18));
    }
}

void printPosition(Display *display)
{
    Window root = DefaultRootWindow(display);
    Window child = 0;
    int rootX = 0;
    int rootY = 0;
    int winX = 0;
    int winY = 0;
    unsigned int mask = 0;
    XQueryPointer(display, root, &root, &child, &rootX, &rootY, &winX, &winY, &mask);
    std::cout << rootX << " " << rootY << "\n";
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

    auto display = openDisplay();
    if (command == "screen") {
        const int screen = DefaultScreen(display.get());
        std::cout << DisplayWidth(display.get(), screen) << " " << DisplayHeight(display.get(), screen) << "\n";
        return 0;
    }
    if (command == "position") {
        printPosition(display.get());
        return 0;
    }
    if (command == "move") {
        if (argc != 4) {
            usage();
            return 2;
        }
        movePointer(display.get(), toInt(argv[2], "x"), toInt(argv[3], "y"));
        return 0;
    }
    if (command == "click" || command == "double-click") {
        if (argc != 4 && argc != 5) {
            usage();
            return 2;
        }
        const int button = argc == 5 ? toInt(argv[4], "button") : 1;
        movePointer(display.get(), toInt(argv[2], "x"), toInt(argv[3], "y"));
        clickButton(display.get(), button);
        if (command == "double-click") {
            std::this_thread::sleep_for(std::chrono::milliseconds(80));
            clickButton(display.get(), button);
        }
        return 0;
    }
    if (command == "key") {
        if (argc != 3) {
            usage();
            return 2;
        }
        typeKey(display.get(), argv[2]);
        return 0;
    }
    if (command == "type") {
        if (argc != 3) {
            usage();
            return 2;
        }
        typeText(display.get(), argv[2]);
        return 0;
    }

    usage();
    return 2;
}
