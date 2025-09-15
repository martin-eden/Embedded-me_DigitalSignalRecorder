// Freetown's functions for digital signal recorder

/*
  Author: Martin Eden
  Last mod.: 2025-09-15
*/

#include <me_DigitalSignalRecorder.h>

#include <me_BaseTypes.h>
#include <me_BaseInterfaces.h>
#include <me_Timestamp.h>
#include <me_Console.h>
#include <me_DebugPrints.h>

using namespace me_DigitalSignalRecorder;

/*
  Convert two events with timestamps to one with duration

  Signal events store timestamp while they should store
  duration since last segment. We're doing this correction
  here.
*/
TBool Freetown::GetDurationSegment(
  TSignalEvent * Result,
  TSignalEvent Prev,
  TSignalEvent Current
)
{
  Result->IsOn = Prev.IsOn;
  Result->Timestamp = Current.Timestamp;

  return me_Timestamp::Subtract(&Result->Timestamp, Prev.Timestamp);
}

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
  2025-09-13
  2025-09-14
*/
