# KovidTone Tokyo
NTS-1 additive / wave sequencing oscillator using 2020 Tokyo COVID-19 statistics

*NOTE* This software is a musical synthesizer oscillator. It does not synthesize or generate any viruses. And this software is not for medical purpose.


## What's this
This is an oscillator which employs additive synthesis with wave sequencing feature using 2020 Tokyo COVID-19 statistics.
The initial "K" is taken from respective great predecessor, additive synthesizer Kawai K5000 or wave sequencing synthesizer Korg WAVESTATION.


- Additive synthesis engine

Existing additive synthesizer like KAWAI K5000, they can manually program the level of each overtone whereas KovidTone is not. It takes values from the statistics on COVID-19 cases in 2020 Tokyo, instead of user entry.

Consider dayN, 7 days statistics from dayN to dayN+6 is used for generate a tone. And the daily number of COVID-19 cases is used as each overtone level.

For example:
|day|cases|
|---|---|
|N|100|
|N+1|87|
|N+2|53|
|N+3|71|
|N+4|69|
|N+5|57|
|N+6|31|

In this example, the sum of 7 days cases is 468 (=100+87+53+71+69+57+31).
To generate A=440Hz tone, additive synthesis engine use this statistics as level of each overtones like below:

|frequency[Hz]|level ratio|
|---|---|
|440*1=440|100/468|
|440*2=880|87/468|
|440*3=1320|53/468|
|440*4=1760|71/468|
|440*5=2200|69/468|
|440*6=2640|57/468|
|440*7=3080|31/468|

Shift the dayN to generate various sound.


- Wave sequencing engine

Existing wave sequencing synthesizer like KORG WAVESTATION, they have many short PCM waveforms to sequence whereas KovidTone is not. It uses realtime additive synthesis instead of PCM waveform.
It switches the dayN's tone generated by additive synthesis engine to dayN+1's tone (and repeatedly dayN+2, N+3 ...).

Simply switching the tone might cause "steppy" tone, and not suitable for musical instrument tone. To avoid this, morphing (crossfading) is available.

In WAVESTATION, you can select some PCM waveforms and sequence them in any order, but KovidTone is not. Instead, you can specify the start day and how many days to use in wave sequence. If it reaches to the final day, automatically back to start day and loop the sound.


The tone generated by the combination of additive synthesis and wave sequencing engine (i.e. data "audiblize") makes some insights.
The increasing higher overtone level shows COVID-19 spreading worser. It might be the "effective reproduction number" is getting larger.


## Prerequisite
[logue-sdk](https://github.com/korginc/logue-sdk)

If you use pre-built binary, logue-sdk is not required. Simply transfer [KovidToneTokyo.ntkdigunit](https://github.com/kachine/nts1KovidToneTokyo/raw/main/KovidToneTokyo.ntkdigunit) to your NTS-1 using NTS-1 digital Librarian software.


## How to build
You have to change PLATFORMDIR variable in Makefile, it should correspond to where you installed logue-sdk (something like below).
```Makefile
PLATFORMDIR = ${PATHTO}/logue-sdk/platform/nutekt-digital
```

Then, simply type "make" to build this project.
```sh
$ make
```


## How to use
There are 5 paramters for this oscillator:
- Start month

This parameter specifies the wave sequence start month.
After Mid-March is suitable to generate sound, because continuous 0 cases for more than 7 days results silence (i.e. silence means great status of suppressing COVID-19).
Actual wave sequence start day is combined value of start month and day of month.
Choose "MONTH" by pressing OSC and tweaking TYPE knob, then input by B knob.

- Start day of month

This parameter specifies the wave sequence start day of month.
If you specify like Feb 30, it will be treated as Mar 1 internally (*note 2020 was a leap year).
Choose "DAY" by pressing OSC and tweaking TYPE knob, then input by B knob.

- Period

This parameter specifies the wave sequence period (i.e. how many days to sequence).
Choose "PERIOD" by pressing OSC and tweaking TYPE knob, then input by B knob.

- Speed

This parameter controls the wave sequence speed.
The greater value results faster speed.
Mapped to OSC mode A knob(SHPE) and MIDI CC#54

- X-fade ratio

This parameter controls the crossfade ratio between day to the next day.
The minimum value makes no crossfade (results steppy tone change), maximum value makes full crossfade (smooth tone change).
Mapped to OSC mode B knob(ALT) and MIDI CC#55

If you use NTS-1's built-in LFO, only for the P(pitch) is effective and S(shape) will be ignored.


---
The names of products or companies are the trademarks or registered trademarks of their respective companies. These companies have no relation to this project.
