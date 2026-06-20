#ifndef VIDEO_CAPTURE_H
#define VIDEO_CAPTURE_H

#include <cstdint>
#include <string>
#include <vector>

// Struktur för en enskild pixel
struct Pixel {
    uint8_t r, g, b;
};

class VideoCapture {
private:
    // Oric PAL timing-dimensioner (inklusive overscan/blanking)
    const int MAX_WIDTH = 384;  
    const int MAX_HEIGHT = 312; 
    
    std::vector<std::vector<Pixel>> screen_buffer;
    
    int x = 0;
    int y = 0;
    bool last_hsync = false;
    bool last_vsync = false;
    bool frame_started = false;
    int frame_counter = 0;

public:
    // Konstruktor: sätter upp den tomma "skärmen"
    VideoCapture();

    // Denna anropas efter varje top_design.step() + debug_eval()
    void process_step(bool hsync, bool vsync, bool r, bool g, bool b);

    // Hämtar antalet hittills sparade bilder (frames)
    int get_frame_count() const;

    // "Transmografiserar" skärmbufferten till en äkta PPM-fil på disken
    void save_to_ppm(const std::string& filename);

    bool test();

};

#endif // VIDEO_CAPTURE_H
