// Digital signal recorder implementation

/*
  Author: Martin Eden
  Last mod.: 2025-10-31
*/

/*
  Capturing pin is pin 8

  That's counter 2 event capturing pin. It's fixed.

  In capturing event we're inverting signal from receiver.
  That means we assume pin is PULLUP and LOW means that
  we're receiving signal.
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
#include <me_HardwareClockScaling.h>

using namespace me_DigitalSignalRecorder;

static const TUint_4 Freq_Hz = 1000;

// Setup margins according to provided memory segment
void TDigitalSignalRecorder::Init(
  TAddressSegment Span
)
{
  this->Span = Span;
  NumSignals_Max = Span.Size / sizeof(TSignal);

  Clear();
}

// Nullify number of stored events
void TDigitalSignalRecorder::Clear()
{
  NumSignals = 0;
  HasPrevEvent = false;
}

// [Internal] Get address of element in our array
static TSignal * GetSlotAddr(
  TAddressSegment Span,
  TUint_2 Index
)
{
  // Fucking love majestic C! Fake arrays
  TAddress SlotAddr;

  // We want 1-based indexes but we can't fail on index 0
  if (Index == 0)
    Index = 1;

  SlotAddr = (Index - 1) * sizeof(TSignal) + Span.Addr;

  return (TSignal *) SlotAddr;
}

// Check index
TBool TDigitalSignalRecorder::CheckIndex(
  TUint_2 Index
)
{
  return (Index >= 1) && (Index <= NumSignals);
}

// Return a copy of signal record
TBool TDigitalSignalRecorder::GetSignal(
  TSignal * Signal,
  TUint_2 Index
)
{
  if (!CheckIndex(Index))
    return false;

  *Signal = *GetSlotAddr(Span, Index);

  return true;
}

// Set signal record
TBool TDigitalSignalRecorder::SetSignal(
  TSignal Signal,
  TUint_2 Index
)
{
  TSignal * SignalSlotPtr;

  if (!CheckIndex(Index))
    return false;

  SignalSlotPtr = GetSlotAddr(Span, Index);

  *SignalSlotPtr = Signal;

  return true;
}

// Adds signal. Used by binary loader
TBool TDigitalSignalRecorder::AddSignal(
  TSignal Signal
)
{
  if (NumSignals == NumSignals_Max)
    return false;

  NumSignals = NumSignals + 1;

  return SetSignal(Signal, NumSignals);
}

// Adds signal given _signal event_
TBool TDigitalSignalRecorder::AddEvent(
  TSignalEvent Event
)
{
  TSignal Signal;

  if (!HasPrevEvent)
  {
    PrevEvent = Event;
    HasPrevEvent = true;

    return true;
  }

  Signal.Duration = Event.Timestamp;
  me_Duration::Subtract(&Signal.Duration, PrevEvent.Timestamp);

  Signal.IsOn = PrevEvent.IsOn;

  PrevEvent = Event;

  return AddSignal(Signal);
}

// Get number of stored signals. Used by binary codec
TUint_2 TDigitalSignalRecorder::GetNumSignals()
{
  return this->NumSignals;
}

// Setting "extern" singleton
TDigitalSignalRecorder me_DigitalSignalRecorder::DigitalSignalRecorder;

// [Interrupt handler] Process signal change
void OnEventCapture_I()
{
  TSignalEvent Event;
  me_Counters::TCounter2 CaptiveCounter;

  Event.IsOn = !CaptiveCounter.Control->EventIsOnUpbeat;
  Event.Timestamp = me_RunTime::GetTime_Precise();

  DigitalSignalRecorder.AddEvent(Event);

  // Trigger next capture at opposite side of signal edge
  CaptiveCounter.Control->EventIsOnUpbeat =
    !CaptiveCounter.Control->EventIsOnUpbeat;
}

// Start recording
void me_DigitalSignalRecorder::StartRecording()
{
  const TUint_1 EventPinNum = 8;

  me_Pins::TInputPin EventPin;
  me_Counters::TCounter2 CaptiveCounter;

  EventPin.Init(EventPinNum);

  CaptiveCounter.Control->EventIsOnUpbeat = false;

  me_Interrupts::On_Counter2_CapturedEvent = OnEventCapture_I;
  CaptiveCounter.Status->GotEventMark = true; // cleared by one
  CaptiveCounter.Interrupts->OnEvent = true;

  me_RunTime::Init();
  me_RunTime::SetTime({ 0, 0, 0, 0 });
  me_RunTime::Start();
}

// Stop recording
void me_DigitalSignalRecorder::StopRecording()
{
  me_Counters::TCounter2 CaptiveCounter;

  CaptiveCounter.Interrupts->OnEvent = false;

  me_RunTime::Stop();
}

/*
  2025 # # # # # # # # # # # # # #
  2025-10-12
  2025-10-23
  2025-10-31
*/
