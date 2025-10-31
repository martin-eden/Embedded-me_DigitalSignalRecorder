// Text codec for DSR

/*
  Author: Martin Eden
  Last mod.: 2025-10-31
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
  TDigitalSignalRecorder * Dsr,
  IOutputStream * OutputStream
)
{
  IOutputStream * OrigOutputStream;
  TUint_2 Index;
  TSignal Signal;

  OrigOutputStream = Console.GetOutputStream();

  Console.SetOutputStream(OutputStream);

  Console.Print("(");
  Console.Indent();

  Index = 1;
  while (Dsr->GetSignal(&Signal, Index))
  {
    SerializeSignal(Signal, OutputStream);
    Index = Index + 1;
  }

  Console.Unindent();
  Console.Print(")");

  Console.SetOutputStream(OrigOutputStream);
}

/*
  Write signal to output stream in some convenient format
*/
void me_DigitalSignalRecorder::TextEncoder::Freetown::SerializeSignal(
  TSignal Signal,
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
  Console.Print(Signal.IsOn);
  me_DebugPrints::PrintDuration(Signal.Duration);
  Console.Write(")");
  Console.EndLine();

  Console.SetOutputStream(OrigOutputStream);
}

/*
  2025 # # #
  2025-10-12
*/
