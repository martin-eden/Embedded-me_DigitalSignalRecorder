// Freetown's functions for digital signal recorder

/*
  Author: Martin Eden
  Last mod.: 2025-09-19
*/

#include <me_DigitalSignalRecorder.h>

#include <me_BaseTypes.h>
#include <me_BaseInterfaces.h>
#include <me_Console.h>
#include <me_DebugPrints.h>

using namespace me_DigitalSignalRecorder;

/*
  Write signal event to output stream in some convenient format
*/
void Freetown::SerializeEvent(
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
  Console.Write(" )");
  Console.EndLine();

  Console.SetOutputStream(OrigOutputStream);
}

/*
  Write sequence of signals to output stream
*/
void Freetown::SerializeEvents(
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
    Freetown::SerializeEvent(Event, OutputStream);
    Index = Index + 1;
  }

  Console.Unindent();
  Console.Print(")");

  Console.SetOutputStream(OrigOutputStream);
}

/*
  2025-09-13
  2025-09-14
  2025-09-19
*/
