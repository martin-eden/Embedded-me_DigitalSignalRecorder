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

// [Internal] Get address of element in our array
TSignalSegment * GetSlotAddr(
  TAddressSegment Span,
  TUint_2 Index
)
{
  // Fucking love majestic C! Fake arrays
  TAddress SlotAddr;

  SlotAddr = Index * sizeof(TSignalSegment) + Span.Addr;

  return (TSignalSegment *) SlotAddr;
}

// Add signal segment
TBool TDigitalSignalRecorder::Add(
  TSignalSegment SigSeg
)
{
  TSignalSegment * SigSegPtr;

  if (!InitDone)
    return false;

  if (NumEvents == NumEvents_Max)
    return false;

  SigSegPtr = GetSlotAddr(Span, NumEvents);
  *SigSegPtr = SigSeg;

  NumEvents = NumEvents + 1;

  return true;
}

// Save signals to some loadable format
void TDigitalSignalRecorder::Save(
  IOutputStream * OutputStream
)
{
  if (!InitDone)
    return;

  if (NumEvents < 2)
    return;

  for (TUint_2 Index = 1; Index < NumEvents; ++Index)
  {
    TSignalSegment * PrevSigsegPtr;
    TSignalSegment * CurSigsegPtr;
    TSignalSegment CorrectedSigseg;
    TBool IsOk;

    PrevSigsegPtr = GetSlotAddr(Span, Index - 1);
    CurSigsegPtr = GetSlotAddr(Span, Index);

    IsOk =
      Freetown::GetDurationSegment(
        &CorrectedSigseg, *PrevSigsegPtr, *CurSigsegPtr
      );

    if (!IsOk)
      break;

    Freetown::SerializeSegment(CorrectedSigseg, OutputStream);
  }
}

// Start recording
void StartRecording()
{
  me_Counters::TCounter2 CaptiveCounter;

  me_RunTime::Start();

  CaptiveCounter.Control->EventIsOnUpbeat = false;
  CaptiveCounter.Interrupts->OnEvent = true;
}

// Stop recording
void StopRecording()
{
  me_Counters::TCounter2 CaptiveCounter;

  CaptiveCounter.Interrupts->OnEvent = false;
  CaptiveCounter.Status->GotEventMark = true; // yep, cleared by one

  me_RunTime::Stop();
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
