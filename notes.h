#define NOTE_C4 29886
#define NOTE_Cs4 28294
#define NOTE_C5 14929
#define NOTE_C6 7465
#define NOTE_D4 26795
#define NOTE_D5 13298
#define NOTE_DS4 25386
#define NOTE_E4 24063
#define NOTE_F4 22823
#define NOTE_FS5 9561
#define NOTE_Fs4 21663
#define NOTE_G4 20579
#define NOTE_G5 8966
#define NOTE_GS5 9409
#define NOTE_GS4 19569
#define NOTE_A4 18630
#define NOTE_AS4 177758
#define NOTE_AS5 8380
#define NOTE_AS3 33498
#define NOTE_B4 16952

#define SONG_B_LENGTH 129

int melody[] = {
    NOTE_G4, NOTE_C4, NOTE_DS4, NOTE_F4, NOTE_G4, NOTE_C4, NOTE_DS4, NOTE_F4,
    NOTE_G4, NOTE_C4, NOTE_DS4, NOTE_F4, NOTE_G4, NOTE_C4, NOTE_DS4, NOTE_F4,
    NOTE_G4, NOTE_C4, NOTE_E4, NOTE_F4, NOTE_G4, NOTE_C4, NOTE_E4, NOTE_F4,
    NOTE_G4, NOTE_C4, NOTE_E4, NOTE_F4, NOTE_G4, NOTE_C4, NOTE_E4, NOTE_F4,
    NOTE_G4, NOTE_C4,

    NOTE_DS4, NOTE_F4, NOTE_G4, NOTE_C4, NOTE_DS4, NOTE_F4,
    NOTE_D4,
    NOTE_F4, NOTE_AS3,
    NOTE_DS4, NOTE_D4, NOTE_F4, NOTE_AS3,
    NOTE_DS4, NOTE_D4, NOTE_C4,

    NOTE_G4, NOTE_C4,

    NOTE_DS4, NOTE_F4, NOTE_G4, NOTE_C4, NOTE_DS4, NOTE_F4,
    NOTE_D4,
    NOTE_F4, NOTE_AS3,
    NOTE_DS4, NOTE_D4, NOTE_F4, NOTE_AS3,
    NOTE_DS4, NOTE_D4, NOTE_C4,
    NOTE_G4, NOTE_C4,
    NOTE_DS4, NOTE_F4, NOTE_G4, NOTE_C4, NOTE_DS4, NOTE_F4,

    NOTE_D4,
    NOTE_F4, NOTE_AS3,
    NOTE_D4, NOTE_DS4, NOTE_D4, NOTE_AS3,
    NOTE_C4,
    NOTE_C5,
    NOTE_AS4,
    NOTE_C4,
    NOTE_G4,
    NOTE_DS4,
    NOTE_DS4, NOTE_F4,
    NOTE_G4,

    NOTE_C5,
    NOTE_AS4,
    NOTE_C4,
    NOTE_G4,
    NOTE_DS4,
    NOTE_DS4, NOTE_D4,
    NOTE_C5, NOTE_G4, NOTE_GS4, NOTE_AS4, NOTE_C5, NOTE_G4, NOTE_GS4, NOTE_AS4,
    NOTE_C5, NOTE_G4, NOTE_GS4, NOTE_AS4, NOTE_C5, NOTE_G4, NOTE_GS4, NOTE_AS4,

    NOTE_GS5, NOTE_AS5, NOTE_C6, NOTE_G5, NOTE_GS5, NOTE_AS5,
    NOTE_C6, NOTE_G5, NOTE_GS5, NOTE_AS5, NOTE_C6, NOTE_G5, NOTE_GS5, NOTE_AS5};

int durations[] = {
    8, 8, 16, 16, 8, 8, 16, 16,
    8, 8, 16, 16, 8, 8, 16, 16,
    8, 8, 16, 16, 8, 8, 16, 16,
    8, 8, 16, 16, 8, 8, 16, 16,
    4, 4,

    16, 16, 4, 4, 16, 16,
    1,
    4, 4,
    16, 16, 4, 4,
    16, 16, 1,

    4, 4,

    16, 16, 4, 4, 16, 16,
    1,
    4, 4,
    16, 16, 4, 4,
    16, 16, 1,
    4, 4,
    16, 16, 4, 4, 16, 16,

    2,
    4, 4,
    8, 8, 8, 8,
    1,
    2,
    2,
    2,
    2,
    2,
    4, 4,
    1,

    2,
    2,
    2,
    2,
    2,
    4, 4,
    8, 8, 16, 16, 8, 8, 16, 16,
    8, 8, 16, 16, 8, 8, 16, 16,

    16, 16, 8, 8, 16, 16,
    8, 16, 16, 16, 8, 8, 16, 16};
