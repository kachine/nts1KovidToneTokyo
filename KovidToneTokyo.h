/*
 * File: KovidToneTokyo.h
 *
 * NTS-1 additive / wave sequencing synthesizer using 2020 Tokyo COVID-19 statistics
 *
 */
#include "userosc.h"
#include <limits.h>

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

#define NUM_OVERTONES     7
#define MAX_PARAM         100
#define MIN_SAMPLE_LEN    2400  // equivalent to 0.05 seconds (k_samplerate / 20)
#define INIT_MONTH_PARAM  3     // April
#define INIT_DOM_PARAM    1

// Tokyo Covid Statistics from 2020/01/01 to 2020/12/31 plus 6 days (= 2021/01/06) to generate 7 days overtones
//  Datasource: https://stopcovid19.metro.tokyo.lg.jp/data/130001_tokyo_covid19_patients.csv
//  Continuous 0 for more than 7 days results silence, so after Mid-March is suitable to generate sound
static const uint16_t covidStats[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 8, 5, 0, 3, 3, 0, 3, 1, 0, 3, 0, 3, 1, 0, 1, 2, 0, 1, 4, 8, 6, 6, 0, 0, 3, 6, 2, 2, 10, 3, 0, 12, 9, 7, 11, 7, 3, 16, 18, 41, 46, 40, 64, 72, 12, 78, 67, 98, 92, 118, 141, 85, 87, 156, 183, 199, 198, 174, 100, 159, 127, 151, 206, 186, 109, 101, 123, 123, 134, 170, 119, 82, 41, 113, 47, 59, 165, 154, 93, 87, 57, 37, 23, 39, 36, 22, 15, 27, 10, 30, 9, 14, 5, 10, 5, 5, 11, 3, 2, 14, 8, 10, 11, 15, 21, 14, 5, 13, 34, 12, 28, 20, 26, 14, 13, 12, 18, 22, 25, 24, 47, 48, 27, 16, 41, 35, 39, 34, 29, 31, 55, 48, 54, 57, 60, 58, 54, 67, 107, 124, 131, 111, 102, 106, 75, 224, 243, 206, 206, 118, 143, 165, 286, 293, 290, 188, 168, 237, 238, 366, 260, 295, 239, 131, 266, 250, 367, 462, 472, 292, 258, 309, 263, 360, 461, 429, 331, 197, 188, 222, 206, 389, 385, 260, 161, 207, 186, 339, 258, 256, 212, 95, 182, 236, 250, 226, 247, 148, 100, 170, 141, 211, 136, 181, 116, 76, 170, 149, 276, 187, 226, 146, 80, 191, 163, 171, 220, 218, 162, 97, 88, 59, 193, 195, 269, 144, 78, 211, 194, 234, 196, 205, 107, 65, 176, 140, 248, 203, 248, 146, 78, 166, 177, 284, 183, 235, 132, 78, 139, 145, 185, 186, 201, 124, 102, 158, 171, 220, 203, 215, 116, 87, 209, 122, 269, 242, 294, 189, 156, 293, 316, 392, 374, 352, 255, 180, 298, 485, 531, 522, 538, 390, 314, 186, 400, 481, 569, 561, 418, 311, 370, 500, 532, 449, 583, 327, 299, 351, 572, 600, 595, 620, 480, 305, 457, 678, 821, 664, 736, 556, 392, 563, 748, 888, 884, 949, 708, 481, 856, 944, 1337, 783, 814, 816, 884, 1278, 1591};
static const uint16_t covidStatsLen = sizeof(covidStats) / sizeof(covidStats[0]);
// Days of each month in 2020        Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec
static const uint8_t daysOfMonth[] = {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

// Voice status structure
struct _voice {
  uint8_t note;                   // MIDI note number
  uint8_t mod;                    // Pitch modifier (0-255)
  float phase[NUM_OVERTONES];     // Phase ratio of each overtone 
  float level[NUM_OVERTONES];     // Level of each harmonics
  uint32_t currSamplePos;         // Current sample position (from NOTE ON) MAX:4.84*10^9 days
  uint16_t currDay;               // Current day index
  uint8_t startMonth;             // start month (0-11)
  uint8_t startDayOfMonth;        // start day of month (0-30)
  uint16_t startDay;              // start day, calculated from startMonth and startDayOfMonth (0-365)
  uint8_t period;                 // Wave sequencing target days (0-99)
  uint32_t dayLength;             // Each day length [Samples] equivalent to speed of wave sequence
  float xfadeRatio;               // Crossfade ratio (0-1)
};
struct _voice VOICE;    // Global variable to keep voice parameters

uint16_t getCovidStat(uint16_t dayIdx, uint8_t overtoneIdx);
void calculateHarmonics(void);
void setStartDay(void);
void OSC_INIT(uint32_t platform, uint32_t api);
void OSC_CYCLE(const user_osc_param_t * const params,
               int32_t *yn,
               const uint32_t frames);
void OSC_NOTEON(const user_osc_param_t * const params);
void OSC_NOTEOFF(const user_osc_param_t * const params);
void OSC_PARAM(uint16_t index, uint16_t value);
