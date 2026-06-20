#include "frame_grab.h"

#include <iostream>
#include <fstream>

#include <stdio.h>

VideoCapture::VideoCapture() {
    // Gör skärmbufferten redo och helt svart från start
    screen_buffer.resize(MAX_HEIGHT, std::vector<Pixel>(MAX_WIDTH, {0, 0, 0}));
}


void VideoCapture::process_step(bool hsync, bool vsync, bool r, bool g, bool b) {

  // Konvertera 1-bit hårdvarusignaler till 8-bit RGB
    uint8_t r_8 = r ? 255 : 0;
    uint8_t g_8 = g ? 255 : 0;
    uint8_t b_8 = b ? 255 : 0;

    // --- Detektera V-SYNC (Ny bildruta påbörjas eller avslutas) ---
    if (vsync && !last_vsync) { 
        if (!frame_started) {
            frame_started = true;
            y = 0;
            x = 0;
        } else {
            // En hel bildruta har passerat! Framkalla och spara den.
            std::string filename = "frame_" + std::to_string(frame_counter++) + ".ppm";
            save_to_ppm(filename);
            
            // Nollställ strålens position inför nästa bildruta
            y = 0;
            x = 0;
        }
    }

    // --- Detektera H-SYNC (Ny rad på skärmen) ---
    if (hsync && !last_hsync) {
        y++;
        x = 0;
    }

    // --- Spara pixeln om vi har börjat rita och strålen är inom marginalerna ---
    if (frame_started && x < MAX_WIDTH && y < MAX_HEIGHT) {
        if (hsync || vsync) {
            // Om någon av synksignalerna är aktiva är vi i blanking (rita svart)
            screen_buffer[y][x] = {0, 0, 0};
        } else {
            screen_buffer[y][x] = {r_8, g_8, b_8};
        }
        x++;
    }

    // Spara tillståndet till nästa klockcykel
    last_hsync = hsync;
    last_vsync = vsync;
}

int VideoCapture::get_frame_count() const {
    return frame_counter;
}

void VideoCapture::save_to_ppm(const std::string& filename) {
    std::ofstream ppm_file(filename);
    if (!ppm_file.is_open()) {
        std::cerr << "[CAPTURE ERROR] Kunde inte skapa filen " << filename << "\n";
        return;
    }

    // PPM Raw Magic header (P3 = textformat, bredd, höjd, max färgstyrka)
    ppm_file << "P3\n" << MAX_WIDTH << " " << MAX_HEIGHT << "\n255\n";
    
    // Pumpa ut pixlarna ur vår "Transmografisator"
    for (int row = 0; row < MAX_HEIGHT; ++row) {
        for (int col = 0; col < MAX_WIDTH; ++col) {
            Pixel p = screen_buffer[row][col];
            ppm_file << (int)p.r << " " << (int)p.g << " " << (int)p.b << " ";
        }
        ppm_file << "\n";
    }
    
    ppm_file.close();
    std::cout << "[TRANSMOGRIFIER] " << filename << " har kläckts på hårddisken!\n";
}
	

bool VideoCapture::test()
{
  int f, h,v;

  // We end the sequence with a vsync so first frame here wont
  // be captured
  //
  for (f=0;f<2;f++)
    {
      for (v=0;v<MAX_HEIGHT;v++)
	{
	  for (h=0;h<MAX_WIDTH;h+=2)
	    {
	      // Doing some moire pattern
	      // hsync vsync r g b..
	      
	      if (v>MAX_HEIGHT-100) // Vertical retrace
		{
		  process_step(0, 1, 0, 0, 0);
		}
	      else if (h>MAX_WIDTH-100) // Horizontal retrace
		{
		  process_step(1, 0, 0, 0, 0);
		}
	      else
		{
		  process_step(0, 0, 1, 0, 1);
		  process_step(0, 0, 0, 1, 0);
		}
	      
	    }
	}
    }

  return true;
  
}
