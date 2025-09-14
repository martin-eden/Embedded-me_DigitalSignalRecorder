// Digital signal recorder

/*
  Author: Martin Eden
  Last mod.: 2025-09-14
*/

/*
  Current implementation uses pin 8 for signal input

  That's because we're using counter 2 which can fire interrupt
  on signal change.

  We don't like this, we want to use interrupt which can
  be attached to any digital pin. So this detail probably
  will change later.
*/

#pragma once

#include <me_BaseTypes.h>
#include <me_BaseInterfaces.h>
#include <me_Timestamp.h>

namespace me_DigitalSignalRecorder
{
  struct TSignalSegment
  {
    TBool IsOn;
    me_Timestamp::TTimestamp Duration;
  };

  class TDigitalSignalRecorder
  {
    public:
      void Init(TAddressSegment);
      TBool Add(TSignalSegment);
      void Save(IOutputStream *);
    private:
      TBool InitDone = false;
      TAddressSegment Span;
      TUint_2 NumEvents_Max;
      TUint_2 NumEvents;
  };

  // Singleton instance
  extern TDigitalSignalRecorder DigitalSignalRecorder;

  void StartRecording();
  void StopRecording();

  namespace Freetown
  {
    TBool GetDurationSegment(
      TSignalSegment * Result,
      TSignalSegment Prev,
      TSignalSegment Current
    );
    void SerializeSegment(TSignalSegment, IOutputStream *);
  }
}

// Timestamp saver on signal change (vector 10 - counter 2 capture event)
extern "C" void __vector_10() __attribute__((interrupt, used));

/*
  2025 # # # # # # # #
  2025-09-12
  2025-09-13
  2025-09-14
*/
