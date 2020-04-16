
#ifndef mp3_h
#define mp3_h


class sclass
{
  public:
    sclass();
    void setup();
    void loop();
    void play_welcome(void (*)(void));
    void play_random(void (*)(void));
    void play_welcome();
    void play_random();
    void set_volume(u_int);
};

extern sclass mp3;

#endif
