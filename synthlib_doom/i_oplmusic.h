//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2014-2015 Alexey Khokholov
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// DESCRIPTION:
//   System interface for music.
//

#ifndef I_OPLMUSIC_H
#define I_OPLMUSIC_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../interface.h"
#define MIDI_CHANNELS_PER_TRACK 16

#define MIDI_EVENT_NOTE_OFF 0x80
#define MIDI_EVENT_NOTE_ON 0x90

#define MIDI_CONTROLLER_MAIN_VOLUME 0x7
#define MIDI_CONTROLLER_PAN 0xa
#define MIDI_CONTROLLER_ALL_NOTES_OFF 0x7b
#define MIDI_EVENT_CONTROLLER 0xb0
#define MIDI_EVENT_PROGRAM_CHANGE 0xc0
#define MIDI_EVENT_CHAN_AFTERTOUCH 0xd0
#define MIDI_EVENT_PITCH_BEND 0xe0

#define OPL_NUM_OPERATORS   21
#define OPL_NUM_VOICES      9

#define OPL_REG_WAVEFORM_ENABLE   0x01
#define OPL_REG_TIMER1            0x02
#define OPL_REG_TIMER2            0x03
#define OPL_REG_TIMER_CTRL        0x04
#define OPL_REG_FM_MODE           0x08

#define OPL_REGS_TREMOLO          0x20
#define OPL_REGS_LEVEL            0x40
#define OPL_REGS_ATTACK           0x60
#define OPL_REGS_SUSTAIN          0x80
#define OPL_REGS_WAVEFORM         0xE0

#define OPL_REGS_FREQ_1           0xA0
#define OPL_REGS_FREQ_2           0xB0
#define OPL_REGS_FEEDBACK         0xC0

#define OPL_REG_NEW_MODE          0x105

#define GENMIDI_NUM_INSTRS  128
#define GENMIDI_NUM_PERCUSSION 47

#define GENMIDI_HEADER          "#OPL_II#"
#define GENMIDI_SIZE            11908
#define GENMIDI_FLAG_FIXED      0x0001         /* fixed pitch */
#define GENMIDI_FLAG_2VOICE     0x0004         /* double voice (OPL3) */

#define PERCUSSION_LOG_LEN 16

typedef unsigned char byte;

#pragma pack(1)
typedef struct
{
    byte tremolo;
    byte attack;
    byte sustain;
    byte waveform;
    byte scale;
    byte level;
} genmidi_op_t;
#pragma pack()

#pragma pack(1)
typedef struct
{
    genmidi_op_t modulator;
    byte feedback;
    genmidi_op_t carrier;
    byte unused;
    short base_note_offset;
} genmidi_voice_t;
#pragma pack()

#pragma pack(1)
typedef struct
{
    unsigned short flags;
    byte fine_tuning;
    byte fixed_note;

    genmidi_voice_t voices[2];
} genmidi_instr_t;
#pragma pack()

// Data associated with a channel of a track that is currently playing.

typedef struct
{
    // The instrument currently used for this track.

    genmidi_instr_t *instrument;
    int program;

    // Volume level

    int volume;

    // Pan value

    int pan;

    // Pitch bend value:

    int bend;

} opl_channel_data_t;

typedef struct opl_voice_s opl_voice_t;

struct opl_voice_s
{
    // Index of this voice:
    int index;

    // The operators used by this voice:
    int op1, op2;

    // Array of this voice
    int array;

    // Currently-loaded instrument data
    genmidi_instr_t *current_instr;

    // The voice number in the instrument to use.
    // This is normally set to zero; if this is a double voice
    // instrument, it may be one.
    unsigned int current_instr_voice;

    // The channel currently using this voice.
    opl_channel_data_t *channel;

    // The midi key that this voice is playing.
    unsigned int key;

    // The note being played.  This is normally the same as
    // the key, but if the instrument is a fixed pitch
    // instrument, it is different.
    unsigned int note;

    // The frequency value being used.
    unsigned int freq;

    // The volume of the note being played on this channel.
    unsigned int note_volume;

    // The current volume (register value) that has been set for this channel.
    unsigned int car_volume;
    unsigned int mod_volume;

    // The current pan.
    unsigned int reg_pan;

    // Priority.
    unsigned int priority;
};

typedef enum {
    opl_doom1_1_666,    // Doom 1 v1.666
    opl_doom2_1_666,    // Doom 2 v1.666, Hexen, Heretic
    opl_doom_1_9        // Doom v1.9, Strife
} opl_driver_ver_t;

// Operators used by the different voices.

const int voice_operators[2][OPL_NUM_VOICES] = {
    { 0x00, 0x01, 0x02, 0x08, 0x09, 0x0a, 0x10, 0x11, 0x12 },
    { 0x03, 0x04, 0x05, 0x0b, 0x0c, 0x0d, 0x13, 0x14, 0x15 }
};

// Frequency values to use for each note.

const unsigned short frequency_curve[] = {

    0x133, 0x133, 0x134, 0x134, 0x135, 0x136, 0x136, 0x137,   // -1
    0x137, 0x138, 0x138, 0x139, 0x139, 0x13a, 0x13b, 0x13b,
    0x13c, 0x13c, 0x13d, 0x13d, 0x13e, 0x13f, 0x13f, 0x140,
    0x140, 0x141, 0x142, 0x142, 0x143, 0x143, 0x144, 0x144,

    0x145, 0x146, 0x146, 0x147, 0x147, 0x148, 0x149, 0x149,   // -2
    0x14a, 0x14a, 0x14b, 0x14c, 0x14c, 0x14d, 0x14d, 0x14e,
    0x14f, 0x14f, 0x150, 0x150, 0x151, 0x152, 0x152, 0x153,
    0x153, 0x154, 0x155, 0x155, 0x156, 0x157, 0x157, 0x158,

    // These are used for the first seven MIDI note values:

    0x158, 0x159, 0x15a, 0x15a, 0x15b, 0x15b, 0x15c, 0x15d,   // 0
    0x15d, 0x15e, 0x15f, 0x15f, 0x160, 0x161, 0x161, 0x162,
    0x162, 0x163, 0x164, 0x164, 0x165, 0x166, 0x166, 0x167,
    0x168, 0x168, 0x169, 0x16a, 0x16a, 0x16b, 0x16c, 0x16c,

    0x16d, 0x16e, 0x16e, 0x16f, 0x170, 0x170, 0x171, 0x172,   // 1
    0x172, 0x173, 0x174, 0x174, 0x175, 0x176, 0x176, 0x177,
    0x178, 0x178, 0x179, 0x17a, 0x17a, 0x17b, 0x17c, 0x17c,
    0x17d, 0x17e, 0x17e, 0x17f, 0x180, 0x181, 0x181, 0x182,

    0x183, 0x183, 0x184, 0x185, 0x185, 0x186, 0x187, 0x188,   // 2
    0x188, 0x189, 0x18a, 0x18a, 0x18b, 0x18c, 0x18d, 0x18d,
    0x18e, 0x18f, 0x18f, 0x190, 0x191, 0x192, 0x192, 0x193,
    0x194, 0x194, 0x195, 0x196, 0x197, 0x197, 0x198, 0x199,

    0x19a, 0x19a, 0x19b, 0x19c, 0x19d, 0x19d, 0x19e, 0x19f,   // 3
    0x1a0, 0x1a0, 0x1a1, 0x1a2, 0x1a3, 0x1a3, 0x1a4, 0x1a5,
    0x1a6, 0x1a6, 0x1a7, 0x1a8, 0x1a9, 0x1a9, 0x1aa, 0x1ab,
    0x1ac, 0x1ad, 0x1ad, 0x1ae, 0x1af, 0x1b0, 0x1b0, 0x1b1,

    0x1b2, 0x1b3, 0x1b4, 0x1b4, 0x1b5, 0x1b6, 0x1b7, 0x1b8,   // 4
    0x1b8, 0x1b9, 0x1ba, 0x1bb, 0x1bc, 0x1bc, 0x1bd, 0x1be,
    0x1bf, 0x1c0, 0x1c0, 0x1c1, 0x1c2, 0x1c3, 0x1c4, 0x1c4,
    0x1c5, 0x1c6, 0x1c7, 0x1c8, 0x1c9, 0x1c9, 0x1ca, 0x1cb,

    0x1cc, 0x1cd, 0x1ce, 0x1ce, 0x1cf, 0x1d0, 0x1d1, 0x1d2,   // 5
    0x1d3, 0x1d3, 0x1d4, 0x1d5, 0x1d6, 0x1d7, 0x1d8, 0x1d8,
    0x1d9, 0x1da, 0x1db, 0x1dc, 0x1dd, 0x1de, 0x1de, 0x1df,
    0x1e0, 0x1e1, 0x1e2, 0x1e3, 0x1e4, 0x1e5, 0x1e5, 0x1e6,

    0x1e7, 0x1e8, 0x1e9, 0x1ea, 0x1eb, 0x1ec, 0x1ed, 0x1ed,   // 6
    0x1ee, 0x1ef, 0x1f0, 0x1f1, 0x1f2, 0x1f3, 0x1f4, 0x1f5,
    0x1f6, 0x1f6, 0x1f7, 0x1f8, 0x1f9, 0x1fa, 0x1fb, 0x1fc,
    0x1fd, 0x1fe, 0x1ff, 0x200, 0x201, 0x201, 0x202, 0x203,

    // First note of looped range used for all octaves:

    0x204, 0x205, 0x206, 0x207, 0x208, 0x209, 0x20a, 0x20b,   // 7
    0x20c, 0x20d, 0x20e, 0x20f, 0x210, 0x210, 0x211, 0x212,
    0x213, 0x214, 0x215, 0x216, 0x217, 0x218, 0x219, 0x21a,
    0x21b, 0x21c, 0x21d, 0x21e, 0x21f, 0x220, 0x221, 0x222,

    0x223, 0x224, 0x225, 0x226, 0x227, 0x228, 0x229, 0x22a,   // 8
    0x22b, 0x22c, 0x22d, 0x22e, 0x22f, 0x230, 0x231, 0x232,
    0x233, 0x234, 0x235, 0x236, 0x237, 0x238, 0x239, 0x23a,
    0x23b, 0x23c, 0x23d, 0x23e, 0x23f, 0x240, 0x241, 0x242,

    0x244, 0x245, 0x246, 0x247, 0x248, 0x249, 0x24a, 0x24b,   // 9
    0x24c, 0x24d, 0x24e, 0x24f, 0x250, 0x251, 0x252, 0x253,
    0x254, 0x256, 0x257, 0x258, 0x259, 0x25a, 0x25b, 0x25c,
    0x25d, 0x25e, 0x25f, 0x260, 0x262, 0x263, 0x264, 0x265,

    0x266, 0x267, 0x268, 0x269, 0x26a, 0x26c, 0x26d, 0x26e,   // 10
    0x26f, 0x270, 0x271, 0x272, 0x273, 0x275, 0x276, 0x277,
    0x278, 0x279, 0x27a, 0x27b, 0x27d, 0x27e, 0x27f, 0x280,
    0x281, 0x282, 0x284, 0x285, 0x286, 0x287, 0x288, 0x289,

    0x28b, 0x28c, 0x28d, 0x28e, 0x28f, 0x290, 0x292, 0x293,   // 11
    0x294, 0x295, 0x296, 0x298, 0x299, 0x29a, 0x29b, 0x29c,
    0x29e, 0x29f, 0x2a0, 0x2a1, 0x2a2, 0x2a4, 0x2a5, 0x2a6,
    0x2a7, 0x2a9, 0x2aa, 0x2ab, 0x2ac, 0x2ae, 0x2af, 0x2b0,

    0x2b1, 0x2b2, 0x2b4, 0x2b5, 0x2b6, 0x2b7, 0x2b9, 0x2ba,   // 12
    0x2bb, 0x2bd, 0x2be, 0x2bf, 0x2c0, 0x2c2, 0x2c3, 0x2c4,
    0x2c5, 0x2c7, 0x2c8, 0x2c9, 0x2cb, 0x2cc, 0x2cd, 0x2ce,
    0x2d0, 0x2d1, 0x2d2, 0x2d4, 0x2d5, 0x2d6, 0x2d8, 0x2d9,

    0x2da, 0x2dc, 0x2dd, 0x2de, 0x2e0, 0x2e1, 0x2e2, 0x2e4,   // 13
    0x2e5, 0x2e6, 0x2e8, 0x2e9, 0x2ea, 0x2ec, 0x2ed, 0x2ee,
    0x2f0, 0x2f1, 0x2f2, 0x2f4, 0x2f5, 0x2f6, 0x2f8, 0x2f9,
    0x2fb, 0x2fc, 0x2fd, 0x2ff, 0x300, 0x302, 0x303, 0x304,

    0x306, 0x307, 0x309, 0x30a, 0x30b, 0x30d, 0x30e, 0x310,   // 14
    0x311, 0x312, 0x314, 0x315, 0x317, 0x318, 0x31a, 0x31b,
    0x31c, 0x31e, 0x31f, 0x321, 0x322, 0x324, 0x325, 0x327,
    0x328, 0x329, 0x32b, 0x32c, 0x32e, 0x32f, 0x331, 0x332,

    0x334, 0x335, 0x337, 0x338, 0x33a, 0x33b, 0x33d, 0x33e,   // 15
    0x340, 0x341, 0x343, 0x344, 0x346, 0x347, 0x349, 0x34a,
    0x34c, 0x34d, 0x34f, 0x350, 0x352, 0x353, 0x355, 0x357,
    0x358, 0x35a, 0x35b, 0x35d, 0x35e, 0x360, 0x361, 0x363,

    0x365, 0x366, 0x368, 0x369, 0x36b, 0x36c, 0x36e, 0x370,   // 16
    0x371, 0x373, 0x374, 0x376, 0x378, 0x379, 0x37b, 0x37c,
    0x37e, 0x380, 0x381, 0x383, 0x384, 0x386, 0x388, 0x389,
    0x38b, 0x38d, 0x38e, 0x390, 0x392, 0x393, 0x395, 0x397,

    0x398, 0x39a, 0x39c, 0x39d, 0x39f, 0x3a1, 0x3a2, 0x3a4,   // 17
    0x3a6, 0x3a7, 0x3a9, 0x3ab, 0x3ac, 0x3ae, 0x3b0, 0x3b1,
    0x3b3, 0x3b5, 0x3b7, 0x3b8, 0x3ba, 0x3bc, 0x3bd, 0x3bf,
    0x3c1, 0x3c3, 0x3c4, 0x3c6, 0x3c8, 0x3ca, 0x3cb, 0x3cd,

    // The last note has an incomplete range, and loops round back to
    // the start.  Note that the last value is actually a buffer overrun
    // and does not fit with the other values.

    0x3cf, 0x3d1, 0x3d2, 0x3d4, 0x3d6, 0x3d8, 0x3da, 0x3db,   // 18
    0x3dd, 0x3df, 0x3e1, 0x3e3, 0x3e4, 0x3e6, 0x3e8, 0x3ea,
    0x3ec, 0x3ed, 0x3ef, 0x3f1, 0x3f3, 0x3f5, 0x3f6, 0x3f8,
    0x3fa, 0x3fc, 0x3fe, 0x36c,
};

// Mapping from MIDI volume level to OPL level value.

const unsigned int volume_mapping_table[] = {
    0, 1, 3, 5, 6, 8, 10, 11,
    13, 14, 16, 17, 19, 20, 22, 23,
    25, 26, 27, 29, 30, 32, 33, 34,
    36, 37, 39, 41, 43, 45, 47, 49,
    50, 52, 54, 55, 57, 59, 60, 61,
    63, 64, 66, 67, 68, 69, 71, 72,
    73, 74, 75, 76, 77, 79, 80, 81,
    82, 83, 84, 84, 85, 86, 87, 88,
    89, 90, 91, 92, 92, 93, 94, 95,
    96, 96, 97, 98, 99, 99, 100, 101,
    101, 102, 103, 103, 104, 105, 105, 106,
    107, 107, 108, 109, 109, 110, 110, 111,
    112, 112, 113, 113, 114, 114, 115, 115,
    116, 117, 117, 118, 118, 119, 119, 120,
    120, 121, 121, 122, 122, 123, 123, 123,
    124, 124, 125, 125, 126, 126, 127, 127
};

const unsigned int channel_map_table[MIDI_CHANNELS_PER_TRACK] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 15, 9, 10, 11, 12, 13, 14
};

class DoomOPL : public midisynth {
private:
    fm_chip *opl;
    opl_channel_data_t channels[MIDI_CHANNELS_PER_TRACK];
    opl_driver_ver_t opl_drv_ver;

    // GENMIDI lump instrument data:

    byte lump[GENMIDI_SIZE];
    genmidi_instr_t *main_instrs;
    genmidi_instr_t *percussion_instrs;

    // Voices:

    opl_voice_t voices[OPL_NUM_VOICES * 2];
    opl_voice_t *voice_free_list[OPL_NUM_VOICES * 2];
    opl_voice_t *voice_alloced_list[OPL_NUM_VOICES * 2];
    unsigned int voice_free_num;
    unsigned int voice_alloced_num;

    bool opl_new;
    unsigned int opl_voices;

    void OPL_WriteRegister(unsigned int reg, unsigned char data);
    void OPL_InitRegisters(bool opl_new);
    bool LoadInstrumentTable(const char *filename);
    void ReleaseVoice(unsigned int id);
    void LoadOperatorData(int slot, genmidi_op_t *data, bool max_level, unsigned int *volume);
    void SetVoiceInstrument(opl_voice_t *voice, genmidi_instr_t *instr, unsigned int instr_voice);
    void SetVoiceVolume(opl_voice_t *voice, unsigned int volume);
    void SetVoicePan(opl_voice_t *voice, unsigned int pan);
    void InitVoices(void);
    void VoiceKeyOff(opl_voice_t *voice);
    opl_channel_data_t *TrackChannelForEvent(unsigned char channel_num);
    void KeyOffEvent(unsigned char channel_num, unsigned char key);
    void ReplaceExistingVoice();
    void ReplaceExistingVoiceDoom1();
    void ReplaceExistingVoiceDoom2(opl_channel_data_t *channel);
    unsigned int FrequencyForVoice(opl_voice_t *voice);
    void UpdateVoiceFrequency(opl_voice_t *voice);
    void VoiceKeyOn(opl_channel_data_t *channel, genmidi_instr_t *instrument, unsigned int instrument_voice,
                    unsigned int note, unsigned int key, unsigned int volume);
    void KeyOnEvent(unsigned char channel_num, unsigned char key, unsigned char volume);
    void ProgramChangeEvent(unsigned char channel_num, unsigned char instrument);
    void SetChannelVolume(opl_channel_data_t *channel, unsigned int volume);
    void SetChannelPan(opl_channel_data_t *channel, unsigned int pan);
    void AllNotesOff(opl_channel_data_t *channel);
    void ControllerEvent(unsigned char channel_num, unsigned char controller, unsigned char param);
    void PitchBendEvent(unsigned char channel_num, unsigned char bend);
    void InitChannel(opl_channel_data_t *channel);
    int InitSynth();
public:
    int midi_init(unsigned int rate);
    void midi_changerate(unsigned int rate);
    void midi_write(unsigned int data);
    void midi_write_sysex(unsigned char *buffer, unsigned int length);
    void midi_panic();
    void midi_reset();
    void midi_generate(signed short *buffer, unsigned int length);
    void midi_generate_dosbox(signed short *buffer, unsigned int length);
    void midi_close();
    const char *midi_synthname(void);
    int midi_getprogram(unsigned int channel);
    bool midi_loadbank(char *filename);
    void midi_resetbank();
    int midi_getvoicecount();
};

#endif
