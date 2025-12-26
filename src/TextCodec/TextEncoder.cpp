// Text codec for DSR

/*
  Author: Martin Eden
  Last mod.: 2025-12-26
*/

#include <me_DigitalSignalRecorder.h>

#include <me_BaseTypes.h>
#include <me_BaseInterfaces.h>
#include <me_BaseTypesIo.h>
#include <me_Duration.h>

using namespace me_DigitalSignalRecorder;

/*
  Data format

  We would prefer parenthesis format like

    ( ( Y ( 1 320 ) ) ( N ( 340 ) ) )

  But it requires restorable input stream to parse.

  So we're using length-prefixed fixed format:

    2
    Y 0 0 1 320
    N 0 0 0 340
*/

// Write signal to stream
static void SerializeSignal(
  TSignal Signal,
  IOutputStream * OutputStream
)
{
  me_Duration::TDuration Duration;

  me_BaseTypesIo::Write_Bool(Signal.IsOn, OutputStream);
  OutputStream->Write(' ');
  if (!me_Duration::DurationFromMicros(&Duration, Signal.Duration_Us))
    Duration = {};
  me_Duration::Write(Duration, OutputStream);
}

/*
  Save signals to some loadable format
*/
void me_DigitalSignalRecorder::TextCodec::Save(
  TDigitalSignalRecorder * Dsr,
  IOutputStream * OutputStream
)
{
  TUint_2 NumSignals;
  TUint_2 Index;
  TSignal Signal;

  NumSignals = Dsr->GetNumSignals();

  me_BaseTypesIo::Write_Uint_2(NumSignals, OutputStream);
  OutputStream->Write('\n');

  for (Index = 1; Index <= NumSignals; ++Index)
  {
    Dsr->GetSignal(&Signal, Index);

    SerializeSignal(Signal, OutputStream);

    OutputStream->Write('\n');
  }
}

/*
  2025 # # #
  2025-10-12
  2025-11-10
  2025-11-12
  2025-12-26
*/
