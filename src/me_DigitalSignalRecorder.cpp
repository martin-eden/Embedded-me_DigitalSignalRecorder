// Digital signal recorder implementation

/*
  Author: Martin Eden
  Last mod.: 2025-11-22
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

/*
  Current behavior of event handler:

    It timestamps event on receiving. Note that interrupt handler
    code can be executed after quite long time after event occurred.

    This explains strangely short signals in recordings with relatively
    constant sum of timings:

      550 1950 450 2050 550 1950
                ^
                This event handler ran too late, so timestamp is late

  Better way

    Counter 2 provides counter value when event occurred.

    Now I understand how to use it.

    We'll create our personal timer on that counter.

    On overflow we'll update our personal time record. (We can't use
    time record from [me_RunTime] because it runs on different counter
    and there is no guarantee that two will be in sync.)

    On event we will have partial (little-endian) timestamp from past.
    Event handler runs before counter overflow handler.
    So in event handler we don't check for counter overflow.
    We're dealing with past there, not with present.
    We combine big- and little-endian parts to get complete timestamp
    from past.
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

const TUint_2 TimerFreq_Hz = 1000;
static const me_Duration::TDuration TrackingPeriod = { 0, 0, 1, 0 };

static TUint_2 MarkToMicrosDivisor = 1;

static volatile me_Duration::TDuration CurrentSignalTimestamp;

static me_Duration::TDuration GetDurationFromMark(
  TUint_2 CounterMarkValue
)
{
  TUint_2 NumMicros;

  NumMicros =
    (TUint_4)
    TimerFreq_Hz * CounterMarkValue / MarkToMicrosDivisor;

  return { 0, 0, 0, NumMicros};
}

// [Interrupt handler] Advance current signal time
static void OnPeriod_I()
{
  me_Duration::TDuration Tmp;
  Tmp = me_Duration::GetVolatile(CurrentSignalTimestamp);
  me_Duration::Add(&Tmp, TrackingPeriod);
  me_Duration::SetVolatile(CurrentSignalTimestamp, Tmp);
}

static me_Duration::TDuration GetSignalTimestamp()
{
  me_Counters::TCounter2 CaptiveCounter;
  me_Duration::TDuration Result;

  Result = me_Duration::GetVolatile(CurrentSignalTimestamp);
  me_Duration::Add(&Result, GetDurationFromMark(*CaptiveCounter.EventMark));

  return Result;
}

static void StopTimer()
{
  me_Counters::TCounter2 CaptiveCounter;

  CaptiveCounter.Control->DriveSource =
    (TUint_1) me_Counters::TDriveSource_Counter2::None;
}

static void StartTimer()
{
  me_Counters::TCounter2 CaptiveCounter;

  CaptiveCounter.Control->DriveSource =
    (TUint_1) me_Counters::TDriveSource_Counter2::Internal_SlowBy2Pow6;
}

// [Interrupt handler] Process signal change
static void OnEventCapture_I()
{
  TSignalEvent Event;
  me_Counters::TCounter2 CaptiveCounter;

  Event.IsOn = !CaptiveCounter.Control->EventIsOnUpbeat;
  Event.Timestamp = GetSignalTimestamp();

  DigitalSignalRecorder.AddEvent(Event);

  // Trigger next capture at opposite side of signal edge
  CaptiveCounter.Control->EventIsOnUpbeat =
    !CaptiveCounter.Control->EventIsOnUpbeat;
}

// Start recording
void me_DigitalSignalRecorder::StartRecording()
{
  const TUint_1 EventPinNum = 8;

  const me_HardwareClockScaling::TClockScaleSetting
    Spec = { .Prescale_PowOfTwo = 6, .CounterNumBits = 16 };

  me_Pins::TInputPin EventPin;
  me_Counters::TCounter2 CaptiveCounter;
  me_HardwareClockScaling::TClockScale ClockScale;

  EventPin.Init(EventPinNum);

  StopTimer();

  CaptiveCounter.
    SetAlgorithm(me_Counters::TAlgorithm_Counter2::Count_ToMarkA);

  me_HardwareClockScaling::
    CalculateClockScale_Spec(&ClockScale, TimerFreq_Hz, Spec);

  *CaptiveCounter.MarkA = ClockScale.CounterLimit;
  MarkToMicrosDivisor = ClockScale.CounterLimit + 1;

  me_Interrupts::On_Counter2_CapturedEvent = OnEventCapture_I;
  CaptiveCounter.Interrupts->OnEvent = true;

  me_Interrupts::On_Counter2_ReachedMarkA = OnPeriod_I;
  CaptiveCounter.Interrupts->OnMarkA = true;

  CaptiveCounter.Control->EventIsOnUpbeat = false;
  CaptiveCounter.Status->GotEventMark = true; // cleared by one
  CaptiveCounter.Status->GotMarkA = true; // cleared by one
  me_Duration::SetVolatile(CurrentSignalTimestamp, me_Duration::Zero);
  *CaptiveCounter.Current = 0;

  StartTimer();
}

// Stop recording
void me_DigitalSignalRecorder::StopRecording()
{
  me_Counters::TCounter2 CaptiveCounter;

  StopTimer();
  CaptiveCounter.Interrupts->OnEvent = false;
}

/*
  2025 # # # # # # # # # # # # # #
  2025-10-12
  2025-10-23
  2025-10-31
  2025-11-18
  2025-11-22
*/
