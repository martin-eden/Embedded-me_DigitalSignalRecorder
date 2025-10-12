// Digital signal recorder implementation

/*
  Author: Martin Eden
  Last mod.: 2025-10-12
*/

/*
  We're dependent on hardware signal

  It means there is global class instance that will be called
  from signal handler.
*/

#include <me_DigitalSignalRecorder.h>

#include <me_BaseTypes.h>
#include <me_BaseInterfaces.h>

#include <me_Duration.h>
#include <me_Counters.h>
#include <me_RunTime.h>
#include <me_Pins.h>
#include <me_Interrupts.h>

using namespace me_DigitalSignalRecorder;

// Setup margins according to provided memory segment
void TDigitalSignalRecorder::Init(
  TAddressSegment Span
)
{
  this->Span = Span;

  NumEvents = 0;

  NumEvents_Max = Span.Size / sizeof(TSignalEvent);

  InitDone = true;
}

// [Internal] Get address of element in our array
TSignalEvent * GetSlotAddr(
  TAddressSegment Span,
  TUint_2 Index
)
{
  // Fucking love majestic C! Fake arrays
  TAddress SlotAddr;

  // We want 1-based indexes but we can't fail on index 0
  if (Index == 0)
    Index = 1;

  SlotAddr = (Index - 1) * sizeof(TSignalEvent) + Span.Addr;

  return (TSignalEvent *) SlotAddr;
}

// Check index
TBool TDigitalSignalRecorder::CheckIndex(
  TUint_2 Index
)
{
  return (Index >= 1) && (Index <= NumEvents);
}

// Return a copy of event data
TBool TDigitalSignalRecorder::GetEvent(
  TSignalEvent * Event,
  TUint_2 Index
)
{
  if (!CheckIndex(Index))
    return false;

  *Event = *GetSlotAddr(Span, Index);

  return true;
}

// Set event data
TBool TDigitalSignalRecorder::SetEvent(
  TSignalEvent Event,
  TUint_2 Index
)
{
  TSignalEvent * EventSlotPtr;

  if (!CheckIndex(Index))
    return false;

  EventSlotPtr = GetSlotAddr(Span, Index);

  *EventSlotPtr = Event;

  return true;
}

// Add signal segment
TBool TDigitalSignalRecorder::Add(
  TSignalEvent SigSeg
)
{
  if (!InitDone)
    return false;

  if (NumEvents == NumEvents_Max)
    return false;

  NumEvents = NumEvents + 1;

  return SetEvent(SigSeg, NumEvents);
}

// Setting "extern" singleton
TDigitalSignalRecorder me_DigitalSignalRecorder::DigitalSignalRecorder;

/*
  [Interrupt handler] Process signal change

  Bound to pin 8 (counter 2 capture event, interrupt 10).
*/
void OnEventCapture_I()
{
  /*
    We want to spend minimum time here because we cant handle
    new signal while we're not done.

    So we're storing timestamp, not duration from last segment.
  */

  TSignalEvent SigSeg;
  me_Counters::TCounter2 CaptiveCounter;

  SigSeg.IsOn = CaptiveCounter.Control->EventIsOnUpbeat;
  SigSeg.Timestamp = me_RunTime::GetTime();

  DigitalSignalRecorder.Add(SigSeg);

  // Trigger next capture at opposite side of signal edge
  CaptiveCounter.Control->EventIsOnUpbeat =
    !CaptiveCounter.Control->EventIsOnUpbeat;
}

// Start recording
void me_DigitalSignalRecorder::StartRecording()
{
  me_Pins::TInputPin InputPin8;
  me_Counters::TCounter2 CaptiveCounter;

  InputPin8.Init(8);

  me_RunTime::Init();
  me_RunTime::SetTime({ 0, 0, 0, 0 });
  me_RunTime::Start();

  CaptiveCounter.Control->EventIsOnUpbeat = false;

  me_Interrupts::On_Counter2_CapturedEvent = OnEventCapture_I;

  CaptiveCounter.Status->GotEventMark = true;
  CaptiveCounter.Interrupts->OnEvent = true;
}

// Stop recording
void me_DigitalSignalRecorder::StopRecording()
{
  me_Counters::TCounter2 CaptiveCounter;

  CaptiveCounter.Interrupts->OnEvent = false;
  CaptiveCounter.Status->GotEventMark = true; // yep, cleared by one

  me_RunTime::Stop();
}

// Save signals to some loadable format
void me_DigitalSignalRecorder::Save(
  TDigitalSignalRecorder * Recorder,
  IOutputStream * OutputStream
)
{
  Freetown::SerializeEvents(Recorder, OutputStream);
}

/*
  2025 # # # # # # # # # # # # # #
  2025-10-12
*/
