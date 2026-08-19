#ifndef AlarmEntry_h
#define AlarmEntry_h

// Plain alarm data, stored byte-for-byte in NVS (Preferences). Kept in its
// own header with zero hardware dependencies so alarm-matching logic
// (AlarmLogic.h) can be unit tested on a regular PC via `pio test -e native`.
struct AlarmEntry
{
  char name[20];
  int hour;
  int minute;
  bool sunday;
  bool monday;
  bool tuesday;
  bool wednesday;
  bool thursday;
  bool friday;
  bool saturday;
  bool skip_phols;
  bool once;
  bool enabled;
};

#endif
