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

static TBool ParseDuration(
  me_Duration::TDuration * Duration
)
{
  TUint_2 KiloS, S, MilliS, MicroS;

  if (!Console.Read(&KiloS))
    return false;
  if (!Console.Read(&S))
    return false;
  if (!Console.Read(&MilliS))
    return false;
  if (!Console.Read(&MicroS))
    return false;

  Duration->KiloS = KiloS;
  Duration->S = S;
  Duration->MilliS = MilliS;
  Duration->MicroS = MicroS;

  return true;
}

static TBool ParseSignal(
  TSignal * Signal
)
{
  if (!Console.Read(&Signal->IsOn))
    return false;

  if (!ParseDuration(&Signal->Duration))
    return false;

  return true;
}

/*
  Load saved signals
*/
TBool me_DigitalSignalRecorder::TextCodec::Load(
  TDigitalSignalRecorder * Dsr
)
{
  /*
    Data sample:

      00002
      Y 00000 00000 00001 00328
      N 00000 00000 00000 00344
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
  2025-11-09
  2025-11-10
  2025-11-12
*/
