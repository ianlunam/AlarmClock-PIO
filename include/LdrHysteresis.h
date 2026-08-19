#ifndef LdrHysteresis_h
#define LdrHysteresis_h

enum LdrDecision
{
    LDR_NO_CHANGE,
    LDR_GO_BRIGHT, // dark enough, for long enough - raise the backlight
    LDR_GO_DIM     // light enough, for long enough - lower the backlight
};

// Pure hysteresis/debounce decision, no hardware I/O - takes the raw sensor
// reading and current state, returns what (if anything) should change.
// darkCount/lightCount are the caller's running counters, updated in place.
// See test/test_ldr_hysteresis.
LdrDecision ldrUpdate(int sensorValue, bool backlightDown, int &darkCount, int &lightCount,
                      int darkThreshold, int lightThreshold, int debounceCount);

#endif
