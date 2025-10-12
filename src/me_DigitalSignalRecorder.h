// Digital signal recorder

/*
  Author: Martin Eden
  Last mod.: 2025-10-12
*/

/*
  Current implementation uses pin 8 for signal input

  That's because we're using counter 2 which can fire interrupt
  on signal change.

  We want to use interrupt which can be attached to any digital pin.
  But looks like it's impossible on ATmega328.
  You can attach pin interrupt only to some pins.
  Or attach interrupt when one of eight pins changes.
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
    Signal recorder

    Contract of this class is provide events with _timestamp_,
    not with _duration_.
  */
  class TDigitalSignalRecorder
  {
    public:
      void Init(TAddressSegment Span);
      TBool Add(TSignalEvent Event);
      TBool GetEvent(TSignalEvent * Event, TUint_2 Index);

    protected:
      TBool CheckIndex(TUint_2 Index);
      TBool SetEvent(TSignalEvent Event, TUint_2 Index);

    private:
      TBool InitDone = false;
      TAddressSegment Span;
      TUint_2 NumEvents_Max;
      TUint_2 NumEvents;
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
      void SerializeEvent(TSignalEvent, IOutputStream *);
    }
  }

  namespace BinaryCodec
  {
    TBool Save(TDigitalSignalRecorder *, IOutputStream *);
    TBool Load(TDigitalSignalRecorder *, IInputStream *);
  }
}

/*
  2025 # # # # # # # # # # # # # #
  2025-10-12
*/
