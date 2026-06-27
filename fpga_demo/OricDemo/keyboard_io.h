#pragma once
#include <termios.h>
#include <unistd.h>
#include <sys/select.h>
#include <fcntl.h>

class Keyboard {
private:
    struct termios original_settings;

public:
    // Ställ om terminalen till unbuffered/non-blocking läge
    void init() {
      int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
      fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
      tcgetattr(STDIN_FILENO, &original_settings);
      struct termios new_settings = original_settings;
      new_settings.c_lflag &= ~(ICANON | ECHO); // Stäng av enter-krav och eko
      tcsetattr(STDIN_FILENO, TCSANOW, &new_settings);
    }

    // Återställ terminalen när vi stänger av simulatorn
    void restore() {
        tcsetattr(STDIN_FILENO, TCSANOW, &original_settings);
    }
    // Give back the keyboard if possible

    ~Keyboard() {
        restore();
    }

    // Kollar blixtsnabbt om en tangent har tryckts ner (Icke-blockerande)
    bool kbhit() {
        struct timeval tv = {0, 0}; // 0 sekunder, 0 mikrosekunder fördröjning
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(STDIN_FILENO, &fds);
        return select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv) > 0;
    }

    // Läser byten från tangentbordet
    char get_key() {
        char ch=-1;
        read(STDIN_FILENO, &ch, 1);
        return ch;
    }
};
