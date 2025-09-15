// [me_SignalRecorder] test

/*
  Author: Martin Eden
  Last mod.: 2025-09-15
*/

#include <me_DigitalSignalRecorder.h>

#include <me_BaseTypes.h>
#include <me_BaseInterfaces.h>
#include <me_Console.h>
#include <me_StreamsCollection.h>
#include <me_DebugPrints.h>

void TestRecorderClass()
{
  const TUint_1 NumSignals_Max = 4;
  me_DigitalSignalRecorder::TSignalEvent Signals[NumSignals_Max];
  TAddressSegment SignalsSpan;
  me_DigitalSignalRecorder::TDigitalSignalRecorder Recorder;

  SignalsSpan = { (TAddress) &Signals, sizeof(Signals) };

  me_DebugPrints::PrintAddrseg("SignalsSpan", SignalsSpan);

  Recorder.Init(SignalsSpan);

  Recorder.Add({true, { 0, 1, 0, 0 } });
  Recorder.Add({false, { 0, 1, 9, 500 } });
  Recorder.Add({true, { 0, 2, 0, 0 } });
  Recorder.Add({false, { 0, 2, 9, 500 } });

  Console.Print("--");
  me_DigitalSignalRecorder::Save(&Recorder, Console.GetOutputStream());
  Console.Print("--");
}

void setup()
{
  Console.Init();

  Console.Print("( [me_DigtialSignalRecorder] test");
  TestRecorderClass();
  Console.Print(") Done");
}

void loop()
{
}

/*
  2025-09-13
  2025-09-14
*/
