void initPIT0Timer();
void restoreDefaultPIT0Timer();
__int64 readPIT0Ticks();
inline float getPIT0Frequency()  { return (float)1193181.6666; }
inline float getPIT0TickTime()  { return (float)(1./1193181.6666); }
