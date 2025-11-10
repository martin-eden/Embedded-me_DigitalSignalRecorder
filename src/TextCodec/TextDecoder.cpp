// Text codec for DSR

/*
  Author: Martin Eden
  Last mod.: 2025-11-10
*/

#include <me_DigitalSignalRecorder.h>

#include <me_BaseTypes.h>
#include <me_BaseInterfaces.h>
#include <me_BooleansCodec.h>
#include <me_ReadInteger.h>

using namespace me_DigitalSignalRecorder;

static TBool ParseDuration(
  me_Duration::TDuration * Duration,
  IInputStream * InputStream
)
{
  TUint_2 KiloS, S, MilliS, MicroS;

  if (!me_ReadInteger::Read_TUint_2(&KiloS, InputStream))
    return false;
  if (!me_ReadInteger::Read_TUint_2(&S, InputStream))
    return false;
  if (!me_ReadInteger::Read_TUint_2(&MilliS, InputStream))
    return false;
  if (!me_ReadInteger::Read_TUint_2(&MicroS, InputStream))
    return false;

  Duration->KiloS = KiloS;
  Duration->S = S;
  Duration->MilliS = MilliS;
  Duration->MicroS = MicroS;

  return true;
}

static TBool ParseSignal(
  TSignal * Signal,
  IInputStream * InputStream
)
{
  if (!me_BooleansCodec::Read(&Signal->IsOn, InputStream))
    return false;

  if (!ParseDuration(&Signal->Duration, InputStream))
    return false;

  return true;
}

/*
  Load saved signals
*/
TBool me_DigitalSignalRecorder::TextCodec::Load(
  TDigitalSignalRecorder * Dsr,
  IInputStream * InputStream
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

  if (!me_ReadInteger::Read_TUint_2(&NumSignals, InputStream))
    return false;

  for (Index = 1; Index <= NumSignals; ++Index)
  {
    if (!ParseSignal(&Signal, InputStream))
      return false;

    Dsr->AddSignal(Signal);
  }

  return true;
}

/*
  2025-11-09
  2025-11-10
*/
