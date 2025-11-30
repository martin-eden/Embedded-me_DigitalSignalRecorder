// Digital signal recorder implementation

/*
  Author: Martin Eden
  Last mod.: 2025-11-30
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

    On new period we'll update our personal time record. (We can't use
    time record from [me_RunTime] because it runs on different counter
    and there is no guarantee that that two will be in sync.)

    On event we will have partial (little-endian) timestamp from past.
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

// [Internal] Check index
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

// [Internal] Set signal record
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

// Add signal
TBool TDigitalSignalRecorder::AddSignal(
  TSignal Signal
)
{
  if (NumSignals == NumSignals_Max)
    return false;

  NumSignals = NumSignals + 1;

  return SetSignal(Signal, NumSignals);
}

// Get number of stored signals. Used by binary codec
TUint_2 TDigitalSignalRecorder::GetNumSignals()
{
  return this->NumSignals;
}

// Setting "extern" singleton
TDigitalSignalRecorder me_DigitalSignalRecorder::DigitalSignalRecorder;

const TUint_2 TimerFreq_Hz = 1000;
const TUint_4 TrackingPeriod_Us = (TUint_4) 1000000 / TimerFreq_Hz;
static me_Duration::TDuration TrackingPeriod = {};

static TUint_1 CounterDriveSource = 0;
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

// Advance current signal time
static void AdvanceSignalTimestamp()
{
  me_Duration::TDuration Tmp;

  Tmp = me_Duration::GetVolatile(CurrentSignalTimestamp);
  me_Duration::WrappedAdd(&Tmp, TrackingPeriod);
  me_Duration::SetVolatile(CurrentSignalTimestamp, Tmp);
}

static me_Duration::TDuration GetSignalTimestamp()
{
  me_Counters::TCounter2 CaptiveCounter;
  me_Duration::TDuration Result;

  Result = me_Duration::GetVolatile(CurrentSignalTimestamp);
  me_Duration::WrappedAdd(&Result, GetDurationFromMark(*CaptiveCounter.EventMark));

  return Result;
}

static void StartTimer()
{
  me_Counters::TCounter2 CaptiveCounter;

  CaptiveCounter.Control->DriveSource = CounterDriveSource;
}

static void StopTimer()
{
  me_Counters::TCounter2 CaptiveCounter;

  CaptiveCounter.Control->DriveSource =
    (TUint_1) me_Counters::TDriveSource_Counter2::None;
}

/*
  Signal event is on/off flag and timestamp
*/
struct TSignalEvent
{
  TBool IsOn;
  me_Duration::TDuration Timestamp;
};

static TSignalEvent PrevEvent;
static TBool HasPrevEvent;

// Adds signal given _signal event_. Used by event handler
static TBool AddEvent(
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

  Signal.IsOn = PrevEvent.IsOn;

  Signal.Duration = Event.Timestamp;
  me_Duration::CappedSub(&Signal.Duration, PrevEvent.Timestamp);

  PrevEvent = Event;

  return DigitalSignalRecorder.AddSignal(Signal);
}

// [Interrupt handler] Process signal change
static void OnEventCapture_I()
{
  TSignalEvent Event;
  me_Counters::TCounter2 CaptiveCounter;

  Event.IsOn = !CaptiveCounter.Control->EventIsOnUpbeat;
  Event.Timestamp = GetSignalTimestamp();

  AddEvent(Event);

  // Trigger next capture at opposite side of signal edge
  CaptiveCounter.Control->EventIsOnUpbeat =
    !CaptiveCounter.Control->EventIsOnUpbeat;
}

/*
  Prepare recorder

  Stops timer and sets it up to tick with desired speed.
  Sets event capture handler.
*/
void me_DigitalSignalRecorder::PrepareRecorder()
{
  const TUint_1 EventPinNum = 8;

  me_Pins::TInputPin EventPin;
  me_Counters::TCounter2 CaptiveCounter;
  me_HardwareClockScaling::TClockScale ClockScale;

  EventPin.Init(EventPinNum);

  StopTimer();

  CaptiveCounter.
    SetAlgorithm(me_Counters::TAlgorithm_Counter2::Count_ToMarkA);

  me_HardwareClockScaling::CalculateClockScale_Specs(
    &ClockScale,
    TimerFreq_Hz,
    me_HardwareClockScaling::AtMega328::GetSpecs_Counter2()
  );

  *CaptiveCounter.MarkA = ClockScale.CounterLimit;
  MarkToMicrosDivisor = ClockScale.CounterLimit + 1;

  me_Counters::GetPrescaleConst_Counter2(&CounterDriveSource, ClockScale.Prescale_PowOfTwo);

  me_Interrupts::On_Counter2_ReachedMarkA = AdvanceSignalTimestamp;
  CaptiveCounter.Interrupts->OnMarkA = true;

  me_Interrupts::On_Counter2_CapturedEvent = OnEventCapture_I;
  CaptiveCounter.Interrupts->OnEvent = false;

  CaptiveCounter.Control->EventIsOnUpbeat = false;

  me_Duration::MicrosToDuration(&TrackingPeriod, TrackingPeriod_Us);
}

/*
  Start recording

  Enables event handler. Starts timer from zero.
*/
void me_DigitalSignalRecorder::StartRecording()
{
  me_Counters::TCounter2 CaptiveCounter;

  HasPrevEvent = false;

  CaptiveCounter.Status->GotEventMark = true; // cleared by one
  CaptiveCounter.Interrupts->OnEvent = true;

  me_Duration::SetVolatile(CurrentSignalTimestamp, me_Duration::MinValue);
  *CaptiveCounter.Current = 0;
  CaptiveCounter.Status->GotMarkA = true; // cleared by one

  StartTimer();
}

/*
  Stop recording

  Disables event handler. Stops timer.
*/
void me_DigitalSignalRecorder::StopRecording()
{
  me_Counters::TCounter2 CaptiveCounter;

  StopTimer();

  CaptiveCounter.Interrupts->OnEvent = false;
  CaptiveCounter.Status->GotEventMark = true; // cleared by one
}

/*
  2025 # # # # # # # # # # # # # #
  2025-10-12
  2025-10-23
  2025-10-31
  2025-11-18
  2025-11-22
  2025-11-25
*/
