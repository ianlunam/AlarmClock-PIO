#include "LdrHysteresis.h"

LdrDecision ldrUpdate(int sensorValue, bool backlightDown, int &darkCount, int &lightCount,
                      int darkThreshold, int lightThreshold, int debounceCount)
{
    // Hysteresis: "dark" and "light" use different thresholds with a dead
    // zone between them, and a reading must persist for several polls
    // before it's acted on. Without this, a value sitting right on the
    // boundary flickers the backlight between min and max every poll.
    if (sensorValue <= darkThreshold)
    {
        darkCount++;
        lightCount = 0;
    }
    else if (sensorValue >= lightThreshold)
    {
        lightCount++;
        darkCount = 0;
    }
    else
    {
        darkCount = 0;
        lightCount = 0;
    }

    if (darkCount >= debounceCount && backlightDown)
    {
        return LDR_GO_BRIGHT;
    }
    if (lightCount >= debounceCount && !backlightDown)
    {
        return LDR_GO_DIM;
    }
    return LDR_NO_CHANGE;
}
