/*
 * File: KovidToneTokyo.c
 *
 * NTS-1 additive / wave sequencing synthesizer using 2020 Tokyo COVID-19 statistics
 *
 */
#include <KovidToneTokyo.h>

// Returns single covid stat specified by day and harmonics
//  with safeguard to prevent exceed array bounds
uint16_t getCovidStat(uint16_t dayIdx, uint8_t overtoneIdx) {
  uint16_t idx = VOICE.startDay + dayIdx + overtoneIdx;
  if(idx < covidStatsLen)
    return covidStats[idx];
  else
    return 0;
}

// Calculate each harmonic level ratios
void calculateHarmonics(void) {
  // Calculate coeff for normalize current day
  float normCoeffCurrDay = 0.f;
  for (uint8_t overtone_id = 0; overtone_id < NUM_OVERTONES; overtone_id++) {
    normCoeffCurrDay += getCovidStat(VOICE.currDay, overtone_id);
  }
  // Avoid zero div to calculate reciprocal number
  if(normCoeffCurrDay != 0.f){
    normCoeffCurrDay = 1.f / normCoeffCurrDay;
  }

  // Wave sequencing (X-fade day to day)
  uint32_t numXFadeSamples = VOICE.xfadeRatio * VOICE.dayLength;
  uint32_t numNonXFadeSamples = VOICE.dayLength - numXFadeSamples;
  // In the X-fade period
  if (VOICE.currSamplePos > numNonXFadeSamples) {
    float currXFadeRatio = numXFadeSamples == 0.f ? 0.f : ((float) (VOICE.currSamplePos - numNonXFadeSamples) / numXFadeSamples);
    // Calculate next day index
    uint16_t nextDay = VOICE.currDay + 1;
    //If entire period passed or reached to 2020/12/31, return to first day to loop the X-faded waveform
    if (nextDay > VOICE.period || nextDay + VOICE.startDay + NUM_OVERTONES > covidStatsLen) {
      nextDay = 0;
    }

    // Calculate coeff for normalize next day
    float normCoeffNextDay = 0.f;
    for (uint8_t overtone_id = 0; overtone_id < NUM_OVERTONES; overtone_id++) {
      normCoeffNextDay += getCovidStat(nextDay, overtone_id);
    }
    // Avoid zero div to calculate reciprocal number
    if(normCoeffNextDay != 0.f){
      normCoeffNextDay = (1.f / normCoeffNextDay);
    }

    // Calculate harmonics level
    for (uint8_t overtone_id = 0; overtone_id < NUM_OVERTONES; overtone_id++) {
      VOICE.level[overtone_id] = (1.0 - currXFadeRatio) * normCoeffCurrDay * getCovidStat(VOICE.currDay, overtone_id)
          + currXFadeRatio * normCoeffNextDay * getCovidStat(nextDay, overtone_id);
    }
  }
  // Before the X-fade period
  else {
    // Calculate harmonics level
    for (uint8_t overtone_id = 0; overtone_id < NUM_OVERTONES; overtone_id++) {
      VOICE.level[overtone_id] = normCoeffCurrDay * getCovidStat(VOICE.currDay, overtone_id);
    }
  }
}

// Set start day from the combination of month and day of month
void setStartDay(void){
  uint16_t startDay = 0;
  for(uint8_t month = 0; month < VOICE.startMonth; month++){
    startDay += daysOfMonth[month];
  }
  VOICE.startDay = startDay + VOICE.startDayOfMonth;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

// Initialize function
//  called once on instantiation of the oscillator
void OSC_INIT(uint32_t platform, uint32_t api)
{
  // Initialize voice parameters
  // Initialize
  for(uint8_t i = 0; i < NUM_OVERTONES; i++){
    VOICE.phase[i] = 0.f;
    VOICE.level[i] = 0.f;
  }
  VOICE.currSamplePos = 0;
  VOICE.currDay = 0;
  VOICE.startMonth = INIT_MONTH_PARAM;
  VOICE.startDayOfMonth = INIT_DOM_PARAM;
  setStartDay();
  VOICE.period = NUM_OVERTONES;
  VOICE.dayLength = MIN_SAMPLE_LEN;
  VOICE.xfadeRatio = 0;
}

// Wave Generation function
//  callbacked for each sample (or frames)
void OSC_CYCLE(const user_osc_param_t * const params,
               int32_t *yn,
               const uint32_t frames)
{
  // Pointer to output buffer
  q31_t * __restrict y = (q31_t *) yn;

  // Last address of output buffer
  const q31_t * y_e = y + frames;

  // MIDI note# of current process
  VOICE.note = params->pitch >> 8;

  // Corresponding frequency of the MIDI note#
  // Not only notenumber but also pitchbend and built-in LFO pitch modulation is taken into account
  for(uint8_t i = 0; i < NUM_OVERTONES; i++){
    VOICE.frequency[i] = osc_notehzf(VOICE.note) * (i + 1);
  }

  // Working memory to store current sample value
  // Effective range is -1.0 <= sample < 1.0 to convert into Q31 format later
  float sample;

  // Process one sample by sample in frames
  while( y != y_e ) {
    sample = 0.f;
    calculateHarmonics();

    // Additive synthesis
    for(uint8_t i = 0; i < NUM_OVERTONES; i++){
      sample +=  VOICE.level[i] * osc_sinf(VOICE.phase[i]);
      // Step a phase
      VOICE.phase[i] += VOICE.frequency[i] / k_samplerate;
      // Keep the phase within 0 <= phase < 1
      VOICE.phase[i] -= (uint32_t) VOICE.phase[i];
    }

    // Convert a sample into Q31 format, and write to output
    *(y++) = f32_to_q31(sample);

    // Step a sample position
    VOICE.currSamplePos++;
    if (VOICE.currSamplePos >= VOICE.dayLength){ // Logically the condition is "==", but too quick knob tweak may cause ">=" state
      VOICE.currSamplePos = 0;
      // Step current day
      VOICE.currDay++;
      // If entire period passed or reached to 2020/12/31, return to first day to loop the waveform
      if (VOICE.currDay > VOICE.period || VOICE.currDay + VOICE.startDay + NUM_OVERTONES > covidStatsLen) {
        VOICE.currDay = 0;
      }
    }
  }
}

// MIDI note-on event process function
//  * This function is not hooked if active note is already exist
void OSC_NOTEON(const user_osc_param_t * const params)
{
  VOICE.currSamplePos = 0;
  VOICE.currDay = 0;
}

// MIDI note-off event process function
//  * This function is not hooked if active note remains
void OSC_NOTEOFF(const user_osc_param_t * const params)
{
  // Nothing to do mandatory in NTS-1
}

// Parameter change event process funnction
void OSC_PARAM(uint16_t index, uint16_t value)
{
  // 0-200 for bipolar percent parameters. 0% at 100, -100% at 0.
  // 0-100 for unipolar percent and typeless parameters.
  // 10 bit resolution for shape/shift-shape.
  switch (index) {
    case k_user_osc_param_id1: // Start month for wave sequence (0-11, 1-12 in display)
      // Choose this param by pressing OSC and tweaking TYPE knob, then input by B knob
      VOICE.startMonth = (uint8_t) value;
      setStartDay();
      break;
    case k_user_osc_param_id2: // Start day of month for wave sequence (0-30, 1-31 in display)
      // Choose this param by pressing OSC and tweaking TYPE knob, then input by B knob
      VOICE.startDayOfMonth = (uint8_t) value;
      setStartDay();
      break;
    case k_user_osc_param_id3: // Period (duration) of wave sequence target days (0-99, 1-100 in display)
      // Choose this param by pressing OSC and tweaking TYPE knob, then input by B knob
      VOICE.period = (uint8_t) value;
      break;
    case k_user_osc_param_id4: // Not used
      // Choose this param by pressing OSC and tweaking TYPE knob, then input by B knob
      break;
    case k_user_osc_param_id5: // Not used
      // Choose this param by pressing OSC and tweaking TYPE knob, then input by B knob
      break;
    case k_user_osc_param_id6: // Not used
      // Choose this param by pressing OSC and tweaking TYPE knob, then input by B knob
      break;
    case k_user_osc_param_shape: // Speed of wave sequence
      // 10bit parameter, 0 <= value <= 1023
      // Mapped to OSC mode A knob(shape) and MIDI CC#54
      // Store the value as each day's waveform length.
      // The wave sequence speed is inverse proportional to the length of each day's waveform.
      VOICE.dayLength = MIN_SAMPLE_LEN + (1.f - param_val_to_f32(value)) * (k_samplerate - MIN_SAMPLE_LEN);
      break;
    case k_user_osc_param_shiftshape: // X-fade ratio among day N and N+1
      // Similar to k_user_osc_param_shape, but mapped to OSC mode B knob(alt) and MIDI CC#55
      VOICE.xfadeRatio = param_val_to_f32(value);
      break;
    default:
      break;
  }
}
#pragma GCC diagnostic pop
