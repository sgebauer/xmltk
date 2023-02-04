#ifndef _SYSTIMER_H
#define _SYSTIMER_H

// SysTimer.h
//
// July 3, 1996
// David Levine, Chris Nevison
//
// This file specifies a timer class.  The purpose of this class is to 
// permit the programmer to measure the amount of system time which is 
// spent during particular phases of a program.  The general model for 
// this timer class was taken from _A Computer Science Tapestry_ by 
// Owen Astrachan, McGraw-Hill, ISBN 0-07-005997-7.
//
// The basic model of the class is that of a stop watch which has two 
// buttons.  One button is used to toggle the stop watch between the 
// "recording" and "idle" states and the other button is used to reset 
// the stop watch.
//
// Concerning units:  All times are given in units of seconds returned
// as a double.
//
// The class is based in the C library function clock(), which returns
// the number of ticks of the system clock since the program started, 
// where the system constant CLOCKS_PER_SEC specifies the number of 
// ticks per second.  This constant determines the smallest time 
// differential which can be measured, ranging from a few microseconds
// to more than a hundredth of a second.  This smallest time difference
// is returned by the function granularity.
//
// On some systems the constant CLK_TCK may be used instead of
// instead of CLOCKS_PER_SEC, in which case the line after the 
// includes should be uncommented.


//#include "bool.h"
#include <time.h> 
// const CLOCKS_PER_SEC CLK_TCK;


class SysTimer{

 public:

  SysTimer();					// constructor

  // methods to modify the state of the stopwatch

  void start();				// begin timing
  void stop();				// stop timing
  void reset();				// reset timer to 0;
                                        // stops watch if running
  
  // methods to query the stopwatch
  
  bool isRunning() const;	 // returns true if and only if the stopwatch
                                 // is currently running
  
  double lapTime() const;	 // returns number of microseconds since the 
                                 // stopwatch was started; if stopwatch is 
                                 // not running, returns 0
  
  double elapsedTime() const;	// returns number of microseconds between 
                                // last start and stop of stopwatch; 
                                // if stopwatch is running, returns 0
  
  double cumulativeTime() const;  // returns total of all times since
                                  // stopwatch was last reset; if stopwatch is
                                  // running, this includes the current
                                  // lap time
  double granularity() const;  // returns the smallest time difference
                               // which can be measured by the stopwatch.
  
 private:
  
  bool	   amRunning;	 // true iff stopwatch is currently running
  clock_t myStartTime;   // holds the last time the watch was started
  clock_t myElapsed;     // holds the time between the last start
                         // and the last stop
  clock_t myCumulative;  // holds the total amount of time that the
                         // stopwatch has been on since the last reset
                         // (except for the current "lap" time)
};

#endif
