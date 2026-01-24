#include <gbdk/platform.h>
#include <stdint.h>


void main(void)
{

    cpu_fast();

    vsync();
    SHOW_BKG;

    // Loop forever
    while(1) {

        // Main loop processing goes here
        // Done processing, yield CPU and wait for start of next frame
        vsync();
    }
}


