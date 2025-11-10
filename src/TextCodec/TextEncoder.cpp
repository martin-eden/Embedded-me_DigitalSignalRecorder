// Text codec for DSR

/*
  Author: Martin Eden
  Last mod.: 2025-11-10
*/

#include <me_DigitalSignalRecorder.h>

#include <me_BaseTypes.h>
#include <me_BaseInterfaces.h>
#include <me_BooleansCodec.h>
#include <me_WriteInteger.h>

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

// Emit newline
static void EndLine(
  IOutputStream * OutputStream
)
{
  OutputStream->Write('\n');
}

// Emit space
static void Space(
  IOutputStream * OutputStream
)
{
  OutputStream->Write(' ');
}

// Write duration to output stream
static void SerializeDuration(
  me_Duration::TDuration Duration,
  IOutputStream * OutputStream
)
{
  me_WriteInteger::Write_U2(Duration.KiloS, OutputStream);
  Space(OutputStream);
  me_WriteInteger::Write_U2(Duration.S, OutputStream);
  Space(OutputStream);
  me_WriteInteger::Write_U2(Duration.MilliS, OutputStream);
  Space(OutputStream);
  me_WriteInteger::Write_U2(Duration.MicroS, OutputStream);
}

// Write signal to output stream
static void SerializeSignal(
  TSignal Signal,
  IOutputStream * OutputStream
)
{
  me_BooleansCodec::Write(Signal.IsOn, OutputStream);
  Space(OutputStream);
  SerializeDuration(Signal.Duration, OutputStream);
  EndLine(OutputStream);
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

  me_WriteInteger::Write_U2(NumSignals, OutputStream);
  EndLine(OutputStream);

  for (Index = 1; Index <= NumSignals; ++Index)
  {
    Dsr->GetSignal(&Signal, Index);

    SerializeSignal(Signal, OutputStream);
  }
}

/*
  2025 # # #
  2025-10-12
  2025-11-10
*/
