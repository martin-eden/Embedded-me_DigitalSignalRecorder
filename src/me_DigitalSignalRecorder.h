// Digital signal recorder

/*
  Author: Martin Eden
  Last mod.: 2025-10-31
*/

/*
  Current implementation uses pin 8 for signal input

  That's because we're using counter 2 which can fire interrupt
  on signal change.
*/

#pragma once

#include <me_BaseTypes.h>
#include <me_BaseInterfaces.h>
#include <me_Duration.h>

namespace me_DigitalSignalRecorder
{
  /*
    Signal event is on/off flag and timestamp
  */
  struct TSignalEvent
  {
    TBool IsOn;
    me_Duration::TDuration Timestamp;
  };

  /*
    Signal is on/off flag and duration
  */
  struct TSignal
  {
    TBool IsOn;
    me_Duration::TDuration Duration;
  };

  /*
    Signal recorder

    You add signal events. It returns signal records.

    Flushable container with signal records.
    Requires external memory for storage.
  */
  class TDigitalSignalRecorder
  {
    public:
      void Init(TAddressSegment Span);
      void Clear();
      TBool AddSignal(TSignal Signal);
      TBool AddEvent(TSignalEvent Event);
      TBool GetSignal(TSignal * Signal, TUint_2 Index);
      TUint_2 GetNumSignals();

    protected:
      TBool CheckIndex(TUint_2 Index);
      TBool SetSignal(TSignal Signal, TUint_2 Index);

    private:
      TAddressSegment Span;
      TUint_2 NumSignals_Max;
      TUint_2 NumSignals;
      TBool HasPrevEvent;
      TSignalEvent PrevEvent;
  };

  // Singleton instance
  extern TDigitalSignalRecorder DigitalSignalRecorder;

  // ( Start/stop adding events to singleton
  void StartRecording();
  void StopRecording();
  // )

  namespace TextEncoder
  {
    // Serialize signal recorder's data
    void Save(TDigitalSignalRecorder *, IOutputStream *);

    namespace Freetown
    {
      void SerializeSignal(TSignal, IOutputStream *);
    }
  }

  namespace BinaryCodec
  {
    TBool Save(TDigitalSignalRecorder *, IOutputStream *);
    TBool Load(IInputStream *, TDigitalSignalRecorder *);
  }
}

/*
  2025 # # # # # # # # # # # # # #
  2025-10-12
  2025-10-31
*/
