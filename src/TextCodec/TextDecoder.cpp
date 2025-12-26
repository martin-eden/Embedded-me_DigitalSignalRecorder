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

// [Internal] Load signal from stream
static TBool ParseSignal(
  TSignal * Signal,
  IInputStream * InputStream
)
{
  me_Duration::TDuration Duration;

  if (!me_BaseTypesIo::Read_Bool(&Signal->IsOn, InputStream))
    return false;

  if (!me_Duration::Read(&Duration, InputStream))
    return false;

  if (!me_Duration::MicrosFromDuration(&Signal->Duration_Us, Duration))
    return false;

  return true;
}

/*
  Load signals from stream
*/
TBool me_DigitalSignalRecorder::TextCodec::Load(
  TDigitalSignalRecorder * Dsr,
  IInputStream * InputStream
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

  if (!me_BaseTypesIo::Read_Uint_2(&NumSignals, InputStream))
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
  2025-11-12
  2025-12-26
*/
