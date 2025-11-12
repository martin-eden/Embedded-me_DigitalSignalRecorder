// Text codec for DSR

/*
  Author: Martin Eden
  Last mod.: 2025-11-12
*/

#include <me_DigitalSignalRecorder.h>

#include <me_BaseTypes.h>
#include <me_BaseInterfaces.h>
#include <me_Console.h>

using namespace me_DigitalSignalRecorder;

/*
  Data format

  We would prefer parenthesis format like

    ( ( YES ( 1 320 ) ) ( NO ( 340 ) ) )

  But it requires restorable input stream to parse.

  So we're using length-prefixed format:

    2
    YES 0 0 1 320
    NO 0 0 0 340
*/

// Write duration to console
static void SerializeDuration(
  me_Duration::TDuration Duration
)
{
  Console.Print(Duration.KiloS);
  Console.Print(Duration.S);
  Console.Print(Duration.MilliS);
  Console.Print(Duration.MicroS);
}

// Write signal to console
static void SerializeSignal(
  TSignal Signal
)
{
  Console.Print(Signal.IsOn);
  SerializeDuration(Signal.Duration);
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
  2025 # # #
  2025-10-12
  2025-11-10
  2025-11-12
*/
