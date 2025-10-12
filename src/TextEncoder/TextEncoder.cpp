// Text codec for DSR

/*
  Author: Martin Eden
  Last mod.: 2025-10-12
*/

#include <me_DigitalSignalRecorder.h>

#include <me_BaseTypes.h>
#include <me_BaseInterfaces.h>
#include <me_Console.h>
#include <me_DebugPrints.h>

using namespace me_DigitalSignalRecorder::TextEncoder::Freetown;

/*
  Save signals to some loadable format
*/
void me_DigitalSignalRecorder::TextEncoder::Save(
  TDigitalSignalRecorder * Recorder,
  IOutputStream * OutputStream
)
{
  SerializeEvents(Recorder, OutputStream);
}

/*
  Write sequence of signals to output stream
*/
void me_DigitalSignalRecorder::TextEncoder::Freetown::SerializeEvents(
  TDigitalSignalRecorder * Dsr,
  IOutputStream * OutputStream
)
{
  IOutputStream * OrigOutputStream;
  TUint_2 Index;
  TSignalEvent Event;

  OrigOutputStream = Console.GetOutputStream();

  Console.SetOutputStream(OutputStream);

  Console.Print("(");
  Console.Indent();

  Index = 1;
  while (Dsr->GetEvent(&Event, Index))
  {
    SerializeEvent(Event, OutputStream);
    Index = Index + 1;
  }

  Console.Unindent();
  Console.Print(")");

  Console.SetOutputStream(OrigOutputStream);
}

/*
  Write signal event to output stream in some convenient format
*/
void me_DigitalSignalRecorder::TextEncoder::Freetown::SerializeEvent(
  TSignalEvent SigSeg,
  IOutputStream * OutputStream
)
{
  /*
    We're lazy and smart here

    We'll use Console to write to our stream.
  */
  IOutputStream * OrigOutputStream;

  OrigOutputStream = Console.GetOutputStream();

  Console.SetOutputStream(OutputStream);

  Console.Write("(");
  Console.Print(SigSeg.IsOn);
  me_DebugPrints::PrintDuration(SigSeg.Timestamp);
  Console.Write(")");
  Console.EndLine();

  Console.SetOutputStream(OrigOutputStream);
}

/*
  2025 # # #
  2025-10-12
*/
