// [me_SignalRecorder] test

/*
  Author: Martin Eden
  Last mod.: 2025-10-31
*/

/*
  This is internal test, not demo

  For demo see [me_InfraredCloner] project.
*/

#include <me_DigitalSignalRecorder.h>

#include <me_BaseTypes.h>
#include <me_BaseInterfaces.h>
#include <me_Console.h>
#include <me_DebugPrints.h>

void TestRecorderClass()
{
  const TUint_1 NumSignals_Max = 3;
  me_DigitalSignalRecorder::TSignalEvent Signals[NumSignals_Max];
  TAddressSegment SignalsSpan;
  me_DigitalSignalRecorder::TDigitalSignalRecorder Recorder;

  SignalsSpan = { (TAddress) &Signals, sizeof(Signals) };

  me_DebugPrints::PrintAddrseg("SignalsSpan", SignalsSpan);

  Recorder.Init(SignalsSpan);

  Recorder.Add( { true, { 0, 1, 0, 0 } } );
  Recorder.Add( { false, { 0, 1, 9, 500 } } );
  Recorder.Add( { true, { 0, 1, 10, 0 } } );
  Recorder.Add( { false, { 0, 1, 11, 500 } } );

  Console.Print("--");
  me_DigitalSignalRecorder::TextEncoder::
    Save(&Recorder, Console.GetOutputStream());
  Console.Print("--");
}

void setup()
{
  Console.Init();

  Console.Print("( [me_DigtialSignalRecorder] test");
  Console.Indent();
  TestRecorderClass();
  Console.Unindent();
  Console.Print(") Done");
}

void loop()
{
}

/*
  2025-09-13
  2025-09-14
*/
