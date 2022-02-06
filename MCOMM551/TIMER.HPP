
/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//  TIMER.HPP - header file for C++ programs using MCOMM timer functions.  //
//    Zortech C++ 2.0, Turbo C++ 1.00 version.                             //
//                                                                         //
//  In order to use this class you must call the function 'tickhookset'    //
//  with a non-zero argument.  This hooks interrupt vector 1C (timer       //
//  tick) and enables the master tick variable.                            //
//                                                                         //
//                      * * *  W A R N I N G  * * *                        //
//  BEFORE EXITING YOUR PROGRAM YOU MUST CALL tickhookset AGAIN WITH A     //
//  ZERO ARGUMENT TO UNHOOK THE VECTOR.  Failing to do this will crash     //
//  the operating system.                                                  //
//                                                                         //
//    Mike Dumdei,  6 Holly Lane,  Texarkana TX  75503    (c) 1989,1990    //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#if !defined(TIMER_HPP)
#define TIMER_HPP

//////////////////////////////
//  Timer class definition  //
//////////////////////////////
class Timer
{
  private:
    long TimeOutClock;              // instance variable
  public:
    Timer();                        // constructors
    Timer(int Ticks);
    Timer(unsigned int Ticks);
    Timer(long Ticks);
    Timer(unsigned long Ticks);
    ~Timer();                       // destructor
    void Set(int Ticks);            // set Timer functions
    void Set(unsigned int Ticks);
    void Set(long Ticks);
    void Set(unsigned long Ticks);
    void Delay(int Ticks);          // set delay functions
    void Delay(unsigned int Ticks);
    int Expired();                  // check for timer expired
    unsigned long ReadTicks();      // read master tick count
};

////////////////////////////////////
//  External low level functions  //
////////////////////////////////////
#ifndef TIMER_H
  extern "C" {
int tickhookset(int flag);
void set_timeout(Timer *timer, unsigned int ticks);
void set_longtimeout(Timer *timer, long ticks);
int timed_out(Timer *timer);
void tdelay(unsigned int ticks);
long get_ticker(void);
  }
#define TIMER_H
#endif

//////////////////////////////////
//  Timer class implementation  //
//////////////////////////////////
inline Timer::Timer()
 { }
inline Timer::Timer(int Ticks)
 { set_timeout(this, (unsigned int)Ticks); }
inline Timer::Timer(unsigned int Ticks)
 { set_timeout(this, Ticks); }
inline Timer::Timer(long Ticks)
 { set_longtimeout(this, Ticks); }
inline Timer::Timer(unsigned long Ticks)
 { set_longtimeout(this, (long)Ticks); }
inline Timer::~Timer()
 { }
inline void Timer::Set(int Ticks)
 { set_timeout(this, (unsigned int)Ticks); }
inline void Timer::Set(unsigned int Ticks)
 { set_timeout(this, Ticks); }
inline void Timer::Set(long Ticks)
 { set_longtimeout(this, Ticks); }
inline void Timer::Set(unsigned long Ticks)
 { set_longtimeout(this, (long)Ticks); }
inline void Timer::Delay(int Ticks)
 { tdelay((unsigned int)Ticks); }
inline void Timer::Delay(unsigned int Ticks)
 { tdelay(Ticks); }
inline int Timer::Expired()
 { return timed_out(this); }
inline unsigned long Timer::ReadTicks()
 { return (unsigned long)get_ticker(); }

#endif          // TIMER_HPP

