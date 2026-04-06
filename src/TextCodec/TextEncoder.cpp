// Text codec for DSR

/*
  Author: Martin Eden
  Last mod.: 2026-04-06
*/

#include <me_DigitalSignalRecorder.h>

#include <me_BaseTypes.h>
#include <me_Duration.h>
#include <me_Console.h>

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

// Write signal to Console
static void SerializeSignal(
  TSignal Signal
)
{
  me_Duration::TDuration Duration;

  Console.Print(Signal.IsOn);
  if (!me_Duration::DurationFromMicros(&Duration, Signal.Duration_Us))
    Duration = {};
  me_Duration::Print(Duration);

  Console.EndLine();
}

/*
  Save signals to some loadable format
*/
void me_DigitalSignalRecorder::TextCodec::Save(
  TDigitalSignalRecorder * Dsr
)
{
  TUint_2 NumSignals;
  TUint_2 Index;
  TSignal Signal;

  NumSignals = Dsr->GetNumSignals();

  Console.Print(NumSignals);
  Console.EndLine();

  for (Index = 1; Index <= NumSignals; ++Index)
  {
    Dsr->GetSignal(&Signal, Index);
    SerializeSignal(Signal);
  }
}

/*
  2025 # # # # # # #
  2026-04-06
*/
