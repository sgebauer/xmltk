// SysTimer.cc
//
// May 22, 1996
// David Levine
//
// Modified by Chris Nevison
// July 1, 1996
//
// This file contains the implementation for the class SysTimer which
// is a stopwatch for the system clock as specified in SysTimer.h.
//
// This implementation uses the C library time.h 

#include "SysTimer.h"
#include <iostream.h>
#include <time.h>

SysTimer::SysTimer() 
: amRunning(false), 
  myElapsed(0), 
  myCumulative(0)
{}

void SysTimer::start()
  // precondition: stopwatch is not running
// postcondition: stopwatch is running    
{
  if (amRunning) {
    cerr << "attempt to start an already running stopwatch" << endl;
  }else {
    amRunning = true;
    myStartTime = clock();
  }
}

void SysTimer::stop()
  // precondition: stopwatch is running
  // postcondition: stopwatch is stopped    
{
  if (! amRunning) {
    cerr << "attempt to stop a non-running stopwatch" << endl;
  }else {
    clock_t myEndTime = clock();
    myElapsed = myEndTime - myStartTime;   // set the status variables
    myCumulative += myElapsed;            
    amRunning = false;			 // turn off the stopwatch
  }
}

void SysTimer::reset()
  // postcondition: all history of stopwatch use erased
  //                and the stopwatch is stopped    
{
  amRunning = false;
  myElapsed = 0;
  myCumulative = 0;
}

bool SysTimer::isRunning() const
  // postcondition: returns true iff stopwatch is currently running
{
  return amRunning;
}

double SysTimer::lapTime() const
  // precondition: stopwatch is running
  // postcondition: returns time in microseconds since last start    
{
  return amRunning ? 
    (double)(clock() - myStartTime) / CLOCKS_PER_SEC : 0.0;
}

double SysTimer::elapsedTime() const
  // precondition: stopwatch is not running
  // postcondition: returns time in microseconds between last stop and start    
{
  return amRunning ? 0.0 : (double)myElapsed / CLOCKS_PER_SEC;
}

double SysTimer::cumulativeTime() const
  // postcondition: returns total time stopwatch has been active    
{
  return ((double)myCumulative / CLOCKS_PER_SEC) + lapTime();  
}

double SysTimer::granularity() const
{
  return 1.0 / CLOCKS_PER_SEC;
}



