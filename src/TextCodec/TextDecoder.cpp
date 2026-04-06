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

// [Internal] Load signal from stream
static TBool ParseSignal(
  TSignal * Signal
)
{
  me_Duration::TDuration Duration;

  if (!Console.Read(&Signal->IsOn))
    return false;

  if (!me_Duration::Read(&Duration))
    return false;

  if (!me_Duration::MicrosFromDuration(&Signal->Duration_Us, Duration))
    return false;

  return true;
}

/*
  Load signals from stream
*/
TBool me_DigitalSignalRecorder::TextCodec::Load(
  TDigitalSignalRecorder * Dsr
)
{
  /*
    Data sample:

      00002
      Y 0 000 001 328
      N 0 000 000 344

    Our contract is to consume given number of records
    even if we can't add them.
  */

  TUint_2 NumSignals;
  TUint_2 Index;
  TSignal Signal;

  if (!Console.Read(&NumSignals))
    return false;

  for (Index = 1; Index <= NumSignals; ++Index)
  {
    if (!ParseSignal(&Signal))
      return false;

    Dsr->AddSignal(Signal);
  }

  return true;
}

/*
  2025 # # # #
  2026-04-06
*/
