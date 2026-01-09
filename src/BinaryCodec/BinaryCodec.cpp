// Binary coder/decoder for DSR's data using binary format

/*
  Author: Martin Eden
  Last mod.: 2026-01-10
*/

/*
  Used to store captured signal in EEPROM
*/

/*
  Data structure

    <NumEvents>
    <TDuration>... * NumEvents

  We're using binary representations.
*/

#include <me_DigitalSignalRecorder.h>

#include <me_BaseTypes.h>
#include <me_BaseInterfaces.h>
#include <me_StreamTools.h>
#include <me_StreamsCollection.h>

using namespace me_DigitalSignalRecorder::BinaryCodec;

// Save signals to stream in binary format
TBool me_DigitalSignalRecorder::BinaryCodec::Save(
  TDigitalSignalRecorder * Dsr,
  IOutputStream * OutputStream
)
{
  TUint_2 NumSignals;
  TUint_2 SignalIndex;
  TSignal Signal;
  me_StreamsCollection::TWorkmemInputStream SourceDataStream;

  NumSignals = Dsr->GetNumSignals();

  SourceDataStream.Init(M_AsAddrSeg(NumSignals));
  if (!me_StreamTools::SaveStreamTo(&SourceDataStream, OutputStream))
    return false;

  for (SignalIndex = 1; SignalIndex <= NumSignals; ++SignalIndex)
  {
    if (!Dsr->GetSignal(&Signal, SignalIndex))
      return false;

    SourceDataStream.Init(M_AsAddrSeg(Signal));
    if (!me_StreamTools::SaveStreamTo(&SourceDataStream, OutputStream))
      return false;
  }

  return true;
}

// Load signals in binary format from stream
TBool me_DigitalSignalRecorder::BinaryCodec::Load(
  TDigitalSignalRecorder * Dsr,
  IInputStream * RawInputStream
)
{
  TUint_2 NumSignals;
  TUint_2 SignalIndex;
  TSignal Signal;
  me_StreamTools::TRereadableInputStream InputStream;
  me_StreamsCollection::TWorkmemOutputStream DestDataStream;

  InputStream.Init(RawInputStream);

  DestDataStream.Init(M_AsAddrSeg(NumSignals));
  if (!me_StreamTools::LoadStreamFrom(&DestDataStream, &InputStream))
    return false;

  for (SignalIndex = 1; SignalIndex <= NumSignals; ++SignalIndex)
  {
    DestDataStream.Init(M_AsAddrSeg(Signal));
    if (!me_StreamTools::LoadStreamFrom(&DestDataStream, &InputStream))
      return false;

    if (!Dsr->AddSignal(Signal))
      return false;
  }

  return true;
}

/*
  2025-10-12
*/
