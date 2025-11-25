// Digital signal recorder

/*
  Author: Martin Eden
  Last mod.: 2025-11-25
*/

/*
  Implementation uses pin 8 for signal input

  Pin 8 on Arduino Uno board is known in datasheet as ICP1 or PB[0].
*/

/*
  Module contents:

    * Signal datatype
    * Signals container
    * Binary data codec
    * Text data codec
*/

#pragma once

#include <me_BaseTypes.h>
#include <me_BaseInterfaces.h>
#include <me_Duration.h>

namespace me_DigitalSignalRecorder
{
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

    It's flushable container with signal records.
    Requires external memory for storage.
  */
  class TDigitalSignalRecorder
  {
    public:
      void Init(TAddressSegment Span);
      void Clear();
      TBool AddSignal(TSignal Signal);
      TBool GetSignal(TSignal * Signal, TUint_2 Index);
      TUint_2 GetNumSignals();

    protected:
      TBool CheckIndex(TUint_2 Index);
      TBool SetSignal(TSignal Signal, TUint_2 Index);

    private:
      TAddressSegment Span;
      TUint_2 NumSignals_Max;
      TUint_2 NumSignals;
  };

  // Singleton instance
  extern TDigitalSignalRecorder DigitalSignalRecorder;

  // ( Start/stop adding events to singleton
  void PrepareRecorder();
  void StartRecording();
  void StopRecording();
  // )

  // Text codec writes to Console
  namespace TextCodec
  {
    void Save(TDigitalSignalRecorder *);
    TBool Load(TDigitalSignalRecorder *);
  }

  // Binary codec writes to stream
  namespace BinaryCodec
  {
    TBool Save(TDigitalSignalRecorder *, IOutputStream *);
    TBool Load(TDigitalSignalRecorder *, IInputStream *);
  }
}

/*
  2025 # # # # # # # # # # # # # # # #
  2025-11-09
  2025-11-12
  2025-11-22
  2025-11-25
*/
