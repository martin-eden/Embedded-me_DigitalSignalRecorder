// Digital signal recorder implementation

/*
  Author: Martin Eden
  Last mod.: 2025-09-14
*/

/*
  We're dependent on hardware signal

  Although there can be set of independent signal recorder instances
  only special one will be called.
*/

#include <me_DigitalSignalRecorder.h>

#include <me_BaseTypes.h>
#include <me_BaseInterfaces.h>

#include <me_Timestamp.h>
#include <me_Counters.h>
#include <me_RunTime.h>

using namespace me_DigitalSignalRecorder;

// Setup margins according to provided memory segment
void TDigitalSignalRecorder::Init(
  TAddressSegment Span
)
{
  this->Span = Span;

  NumEvents = 0;

  NumEvents_Max = Span.Size / sizeof(TSignalSegment);

  InitDone = true;
}

// Start recording
void TDigitalSignalRecorder::StartRecording()
{
  me_Counters::TCounter2 CaptiveCounter;

  me_RunTime::Start();

  CaptiveCounter.Control->EventIsOnUpbeat = false;
  CaptiveCounter.Interrupts->OnEvent = true;
}

// Stop recording
void TDigitalSignalRecorder::StopRecording()
{
  me_Counters::TCounter2 CaptiveCounter;

  CaptiveCounter.Interrupts->OnEvent = false;
  CaptiveCounter.Status->GotEventMark = true; // yep, cleared by one

  me_RunTime::Stop();
}

// [Internal] Get address of element in our array
TSignalSegment * GetSlotAddr(
  TAddressSegment Span,
  TUint_2 Index
)
{
  // Fucking love majestic C! Fake arrays
  TAddress SlotAddr;

  // We want 1-based indexes but we can't fail on index 0
  if (Index == 0)
    Index = 1;

  SlotAddr = (Index - 1) * sizeof(TSignalSegment) + Span.Addr;

  return (TSignalSegment *) SlotAddr;
}

// Return a copy of event data
TBool TDigitalSignalRecorder::GetEvent(
  TSignalSegment * Event,
  TUint_2 Index
)
{
  if (Index > NumEvents)
    return false;

  *Event = *GetSlotAddr(Span, Index);

  return true;
}

// Set event data
TBool TDigitalSignalRecorder::SetEvent(
  TSignalSegment Event,
  TUint_2 Index
)
{
  TSignalSegment * EventSlotPtr;

  if (Index > NumEvents)
    return false;

  EventSlotPtr = GetSlotAddr(Span, Index);

  *EventSlotPtr = Event;

  return true;
}

// Add signal segment
TBool TDigitalSignalRecorder::Add(
  TSignalSegment SigSeg
)
{
  if (!InitDone)
    return false;

  if (NumEvents == NumEvents_Max)
    return false;

  NumEvents = NumEvents + 1;

  return SetEvent(SigSeg, NumEvents);
}

// Replace time value with time difference from prev value
void TDigitalSignalRecorder::Differentiate()
{
  TUint_2 Index;
  TSignalSegment PrevEvent;
  TSignalSegment CurEvent;
  TSignalSegment CorrectedEvent;
  TBool IsOk;

  if (!InitDone)
    return;

  if (NumEvents < 2)
    return;

  Index = 2;

  while (true)
  {
    GetEvent(&PrevEvent, Index - 1);
    GetEvent(&CurEvent, Index);

    IsOk =
      Freetown::GetDurationSegment(
        &CorrectedEvent, PrevEvent, CurEvent
      );

    if (!IsOk)
      break;

    SetEvent(CorrectedEvent, Index - 1);

    if (Index == NumEvents)
      break;

    Index = Index + 1;
  }

  NumEvents = Index - 1;
}

// Save signals to some loadable format
void TDigitalSignalRecorder::Save(
  IOutputStream * OutputStream
)
{
  if (!InitDone)
    return;

  for (TUint_2 Index = 1; Index <= NumEvents; ++Index)
  {
    TSignalSegment CurEvent;

    GetEvent(&CurEvent, Index);

    Freetown::SerializeSegment(CurEvent, OutputStream);
  }
}

// Setting "extern" singleton
TDigitalSignalRecorder me_DigitalSignalRecorder::DigitalSignalRecorder;

/*
  Timestamp saver (interrupt 10 - counter 2 capture event, pin 8)
*/
void __vector_10()
{
  /*
    We want to spend minimum time here because we cant handle
    new signal while we're not done.

    So in signal segment we're storing timestamp,
    not duration from last segment.
  */
  TSignalSegment SigSeg;
  me_Counters::TCounter2 CaptiveCounter;

  SigSeg.IsOn = CaptiveCounter.Control->EventIsOnUpbeat;

  // Trigger next capture at opposite side of signal edge
  CaptiveCounter.Control->EventIsOnUpbeat =
    !CaptiveCounter.Control->EventIsOnUpbeat;

  SigSeg.Duration = me_RunTime::GetTime();

  DigitalSignalRecorder.Add(SigSeg);
}

/*
  2025 # # # # # # # #
  2025-09-12
  2025-09-13
  2025-09-14
*/
