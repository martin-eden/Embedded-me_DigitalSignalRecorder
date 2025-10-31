// Binary coder/decoder for DSR's data using binary format

/*
  Author: Martin Eden
  Last mod.: 2025-10-31
*/

/*
  Used to store captured signal in EEPROM
*/

/*
  Data structure

  We're using binary representations.

    <NumEvents: TUint_2>
    (<TDuration>) * <NumEvents>
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
  TAddressSegment DataAddrseg;
  me_StreamsCollection::TWorkmemInputStream SourceDataStream;
  TUint_2 SignalIndex;
  TSignal Signal;

  if (!Dsr->GetNumSignals(&NumSignals))
    return false;

  DataAddrseg = { (TAddress) &NumSignals, sizeof(NumSignals) };
  SourceDataStream.Init(DataAddrseg);

  if (!me_StreamTools::SaveStreamTo(&SourceDataStream, OutputStream))
    return false;

  for (SignalIndex = 1; SignalIndex <= NumSignals; ++SignalIndex)
  {
    if (!Dsr->GetSignal(&Signal, SignalIndex))
      return false;

    DataAddrseg = { (TAddress) &Signal, sizeof(Signal) };
    SourceDataStream.Init(DataAddrseg);

    if (!me_StreamTools::SaveStreamTo(&SourceDataStream, OutputStream))
      return false;
  }

  return true;
}

// Load signals in binary format from stream
TBool me_DigitalSignalRecorder::BinaryCodec::Load(
  IInputStream * RawInputStream,
  TDigitalSignalRecorder * Dsr
)
{
  me_StreamTools::TRereadableInputStream InputStream;
  TUint_2 NumEvents;
  TAddressSegment DataAddrseg;
  me_StreamsCollection::TWorkmemOutputStream DestDataStream;
  TUint_2 EventIndex;
  TSignalEvent Event;

  InputStream.Init(RawInputStream);

  Dsr->Clear();

  DataAddrseg = { (TAddress) &NumEvents, sizeof(NumEvents) };
  DestDataStream.Init(DataAddrseg);

  if (!me_StreamTools::LoadStreamFrom(&DestDataStream, &InputStream))
    return false;

  for (EventIndex = 1; EventIndex <= NumEvents; ++EventIndex)
  {
    DataAddrseg = { (TAddress) &Event, sizeof(Event) };
    DestDataStream.Init(DataAddrseg);

    if (!me_StreamTools::LoadStreamFrom(&DestDataStream, &InputStream))
      return false;

    if (!Dsr->Add(Event))
      return false;
  }

  return true;
}

/*
  2025-10-12
*/
