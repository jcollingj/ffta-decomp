@ Item-linked ability sets: count, unknown byte, nine job/index pairs.
@ Fixed original index span 0..224. Unused pairs are explicitly zero-filled.
.section .text
.global gItemAbilitySets
.type gItemAbilitySets, %object
.macro item_ability_set count, unknown=0, job0=0, ability0=0, job1=0, ability1=0, job2=0, ability2=0, job3=0, ability3=0, job4=0, ability4=0, job5=0, ability5=0, job6=0, ability6=0, job7=0, ability7=0, job8=0, ability8=0
    .if (\count < 0) || (\count > 9)
        .error "ability set count must be within nine pairs"
    .endif
    .irp value,\unknown,\job0,\ability0,\job1,\ability1,\job2,\ability2,\job3,\ability3,\job4,\ability4,\job5,\ability5,\job6,\ability6,\job7,\ability7,\job8,\ability8
        .if (\value < 0) || (\value > 255)
            .error "ability set fields must fit unsigned bytes"
        .endif
    .endr
    .byte \count, \unknown, \job0, \ability0, \job1, \ability1, \job2, \ability2, \job3, \ability3, \job4, \ability4, \job5, \ability5, \job6, \ability6, \job7, \ability7, \job8, \ability8
.endm
gItemAbilitySets:
    item_ability_set 0, 0 @ 0
    item_ability_set 2, 0, 2, 1, 13, 1 @ 1
    item_ability_set 2, 0, 2, 5, 13, 5 @ 2
    item_ability_set 3, 0, 2, 3, 13, 3, 14, 14 @ 3
    item_ability_set 1, 0, 2, 8 @ 4
    item_ability_set 1, 0, 13, 7 @ 5
    item_ability_set 2, 0, 2, 7, 14, 18 @ 6
    item_ability_set 2, 0, 13, 8, 14, 13 @ 7
    item_ability_set 1, 0, 14, 20 @ 8
    item_ability_set 2, 0, 2, 11, 13, 11 @ 9
    item_ability_set 2, 0, 4, 23, 16, 33 @ 10
    item_ability_set 3, 0, 4, 25, 16, 35, 37, 14 @ 11
    item_ability_set 2, 0, 4, 26, 16, 36 @ 12
    item_ability_set 2, 0, 4, 27, 37, 13 @ 13
    item_ability_set 3, 0, 4, 30, 16, 37, 37, 11 @ 14
    item_ability_set 2, 0, 4, 29, 16, 38 @ 15
    item_ability_set 2, 0, 16, 39, 37, 17 @ 16
    item_ability_set 2, 0, 4, 28, 37, 12 @ 17
    item_ability_set 2, 0, 4, 24, 16, 34 @ 18
    item_ability_set 1, 0, 37, 15 @ 19
    item_ability_set 1, 0, 37, 16 @ 20
    item_ability_set 2, 0, 4, 33, 16, 42 @ 21
    item_ability_set 2, 0, 16, 40, 37, 18 @ 22
    item_ability_set 3, 0, 4, 34, 16, 43, 37, 21 @ 23
    item_ability_set 1, 0, 10, 116 @ 24
    item_ability_set 1, 0, 10, 117 @ 25
    item_ability_set 2, 0, 3, 12, 15, 26 @ 26
    item_ability_set 1, 0, 19, 68 @ 27
    item_ability_set 2, 0, 3, 13, 15, 24 @ 28
    item_ability_set 3, 0, 3, 16, 15, 25, 19, 70 @ 29
    item_ability_set 2, 0, 15, 28, 19, 72 @ 30
    item_ability_set 3, 0, 3, 14, 15, 29, 19, 66 @ 31
    item_ability_set 2, 0, 3, 18, 19, 71 @ 32
    item_ability_set 1, 0, 3, 19 @ 33
    item_ability_set 3, 0, 3, 22, 15, 32, 19, 76 @ 34
    item_ability_set 2, 0, 2, 2, 3, 17 @ 35
    item_ability_set 2, 0, 2, 4, 3, 15 @ 36
    item_ability_set 1, 0, 2, 6 @ 37
    item_ability_set 1, 0, 2, 9 @ 38
    item_ability_set 1, 0, 13, 2 @ 39
    item_ability_set 1, 0, 13, 4 @ 40
    item_ability_set 1, 0, 15, 30 @ 41
    item_ability_set 2, 0, 13, 6, 15, 27 @ 42
    item_ability_set 1, 0, 15, 22 @ 43
    item_ability_set 1, 0, 13, 9 @ 44
    item_ability_set 1, 0, 15, 23 @ 45
    item_ability_set 3, 0, 5, 40, 39, 37, 40, 50 @ 46
    item_ability_set 1, 0, 40, 44 @ 47
    item_ability_set 3, 0, 5, 41, 39, 38, 40, 46 @ 48
    item_ability_set 2, 0, 5, 38, 39, 35 @ 49
    item_ability_set 3, 0, 5, 36, 39, 33, 40, 47 @ 50
    item_ability_set 3, 0, 5, 35, 39, 32, 40, 45 @ 51
    item_ability_set 3, 0, 5, 37, 39, 34, 40, 48 @ 52
    item_ability_set 2, 0, 5, 39, 39, 36 @ 53
    item_ability_set 3, 0, 5, 42, 39, 39, 40, 49 @ 54
    item_ability_set 2, 0, 5, 43, 39, 40 @ 55
    item_ability_set 3, 0, 5, 46, 39, 43, 40, 54 @ 56
    item_ability_set 2, 0, 28, 1, 30, 24 @ 57
    item_ability_set 3, 0, 28, 4, 30, 27, 29, 17 @ 58
    item_ability_set 2, 0, 30, 26, 29, 13 @ 59
    item_ability_set 2, 0, 30, 23, 29, 12 @ 60
    item_ability_set 3, 0, 28, 7, 30, 25, 29, 14 @ 61
    item_ability_set 3, 0, 28, 2, 29, 18, 30, 28 @ 62
    item_ability_set 2, 0, 28, 5, 29, 15 @ 63
    item_ability_set 2, 0, 28, 8, 29, 16 @ 64
    item_ability_set 2, 0, 28, 6, 30, 22 @ 65
    item_ability_set 1, 0, 30, 31 @ 66
    item_ability_set 1, 0, 28, 3 @ 67
    item_ability_set 1, 0, 30, 29 @ 68
    item_ability_set 1, 0, 29, 19 @ 69
    item_ability_set 3, 0, 28, 11, 29, 21, 30, 32 @ 70
    item_ability_set 1, 0, 6, 47 @ 71
    item_ability_set 2, 0, 6, 52, 34, 67 @ 72
    item_ability_set 1, 0, 6, 49 @ 73
    item_ability_set 1, 0, 6, 50 @ 74
    item_ability_set 1, 0, 34, 66 @ 75
    item_ability_set 2, 0, 6, 48, 34, 70 @ 76
    item_ability_set 2, 0, 6, 51, 34, 68 @ 77
    item_ability_set 1, 0, 6, 53 @ 78
    item_ability_set 1, 0, 6, 56 @ 79
    item_ability_set 2, 0, 6, 54, 34, 71 @ 80
    item_ability_set 1, 0, 34, 72 @ 81
    item_ability_set 2, 0, 6, 57, 34, 74 @ 82
    item_ability_set 3, 0, 7, 58, 20, 1, 31, 33 @ 83
    item_ability_set 4, 0, 7, 66, 20, 9, 31, 41, 32, 45 @ 84
    item_ability_set 5, 0, 7, 65, 20, 8, 31, 40, 18, 61, 32, 46 @ 85
    item_ability_set 5, 0, 7, 59, 20, 2, 31, 34, 18, 55, 32, 48 @ 86
    item_ability_set 4, 0, 7, 61, 20, 4, 31, 36, 32, 44 @ 87
    item_ability_set 4, 0, 7, 62, 20, 5, 31, 37, 18, 56 @ 88
    item_ability_set 2, 0, 18, 62, 32, 47 @ 89
    item_ability_set 4, 0, 7, 60, 20, 3, 31, 35, 18, 60 @ 90
    item_ability_set 2, 0, 18, 58, 32, 49 @ 91
    item_ability_set 5, 0, 7, 64, 20, 7, 31, 39, 18, 59, 32, 51 @ 92
    item_ability_set 5, 0, 7, 63, 20, 6, 31, 38, 18, 57, 32, 50 @ 93
    item_ability_set 5, 0, 7, 68, 20, 11, 31, 43, 18, 65, 32, 53 @ 94
    item_ability_set 9, 0, 8, 69, 21, 12, 42, 66, 8, 72, 21, 15, 42, 69, 8, 75, 21, 18, 42, 72 @ 95
    item_ability_set 7, 0, 8, 70, 21, 13, 42, 67, 22, 24, 43, 78, 9, 81, 23, 34 @ 96
    item_ability_set 7, 0, 8, 73, 21, 16, 42, 70, 22, 27, 43, 81, 9, 82, 23, 35 @ 97
    item_ability_set 7, 0, 8, 76, 21, 19, 42, 73, 22, 29, 43, 83, 9, 83, 23, 36 @ 98
    item_ability_set 4, 0, 22, 26, 43, 80, 9, 87, 23, 40 @ 99
    item_ability_set 2, 0, 22, 30, 43, 84 @ 100
    item_ability_set 3, 0, 8, 71, 21, 14, 42, 68 @ 101
    item_ability_set 7, 0, 8, 74, 21, 17, 42, 71, 22, 25, 43, 79, 9, 88, 23, 41 @ 102
    item_ability_set 7, 0, 8, 77, 21, 20, 42, 74, 22, 28, 43, 82, 9, 86, 23, 39 @ 103
    item_ability_set 4, 0, 22, 31, 43, 85, 9, 85, 23, 38 @ 104
    item_ability_set 2, 0, 9, 84, 23, 37 @ 105
    item_ability_set 7, 0, 8, 80, 21, 23, 42, 77, 22, 33, 43, 87, 9, 91, 23, 44 @ 106
    item_ability_set 2, 0, 27, 85, 27, 86 @ 107
    item_ability_set 2, 0, 27, 83, 24, 48 @ 108
    item_ability_set 2, 0, 27, 84, 24, 50 @ 109
    item_ability_set 1, 0, 24, 45 @ 110
    item_ability_set 1, 0, 24, 53 @ 111
    item_ability_set 2, 0, 27, 89, 24, 51 @ 112
    item_ability_set 2, 0, 27, 87, 24, 46 @ 113
    item_ability_set 2, 0, 27, 88, 24, 49 @ 114
    item_ability_set 1, 0, 24, 47 @ 115
    item_ability_set 1, 0, 27, 90 @ 116
    item_ability_set 2, 0, 24, 55, 27, 94 @ 117
    item_ability_set 2, 0, 11, 118, 33, 54 @ 118
    item_ability_set 2, 0, 11, 125, 33, 61 @ 119
    item_ability_set 2, 0, 11, 119, 33, 55 @ 120
    item_ability_set 2, 0, 11, 120, 33, 56 @ 121
    item_ability_set 2, 0, 11, 122, 33, 58 @ 122
    item_ability_set 2, 0, 11, 121, 33, 57 @ 123
    item_ability_set 2, 0, 11, 123, 33, 59 @ 124
    item_ability_set 2, 0, 11, 127, 33, 63 @ 125
    item_ability_set 2, 0, 11, 124, 33, 60 @ 126
    item_ability_set 2, 0, 11, 128, 33, 64 @ 127
    item_ability_set 2, 0, 12, 130, 35, 78 @ 128
    item_ability_set 2, 0, 12, 137, 34, 69 @ 129
    item_ability_set 2, 0, 12, 129, 35, 76 @ 130
    item_ability_set 2, 0, 12, 131, 35, 75 @ 131
    item_ability_set 2, 0, 12, 133, 35, 82 @ 132
    item_ability_set 3, 0, 12, 132, 35, 80, 34, 65 @ 133
    item_ability_set 2, 0, 12, 136, 35, 77 @ 134
    item_ability_set 1, 0, 12, 139 @ 135
    item_ability_set 2, 0, 12, 134, 35, 81 @ 136
    item_ability_set 1, 0, 35, 79 @ 137
    item_ability_set 1, 0, 12, 135 @ 138
    item_ability_set 2, 0, 12, 140, 35, 84 @ 139
    item_ability_set 2, 0, 19, 67, 14, 12 @ 140
    item_ability_set 1, 0, 14, 15 @ 141
    item_ability_set 1, 0, 14, 16 @ 142
    item_ability_set 1, 0, 14, 17 @ 143
    item_ability_set 1, 0, 19, 69 @ 144
    item_ability_set 1, 0, 19, 73 @ 145
    item_ability_set 1, 0, 19, 75 @ 146
    item_ability_set 1, 0, 14, 19 @ 147
    item_ability_set 1, 0, 14, 21 @ 148
    item_ability_set 3, 0, 25, 56, 25, 57, 25, 58 @ 149
    item_ability_set 3, 0, 25, 60, 25, 57, 36, 1 @ 150
    item_ability_set 3, 0, 25, 63, 25, 56, 36, 8 @ 151
    item_ability_set 1, 0, 25, 68 @ 152
    item_ability_set 2, 0, 25, 59, 36, 2 @ 153
    item_ability_set 2, 0, 25, 65, 25, 66 @ 154
    item_ability_set 1, 0, 36, 5 @ 155
    item_ability_set 3, 0, 25, 67, 25, 61, 36, 4 @ 156
    item_ability_set 3, 0, 25, 64, 25, 58, 36, 6 @ 157
    item_ability_set 1, 0, 36, 3 @ 158
    item_ability_set 2, 0, 25, 62, 36, 7 @ 159
    item_ability_set 2, 0, 25, 71, 36, 10 @ 160
    item_ability_set 2, 0, 17, 44, 41, 58 @ 161
    item_ability_set 2, 0, 17, 50, 41, 55 @ 162
    item_ability_set 1, 0, 41, 57 @ 163
    item_ability_set 1, 0, 41, 61 @ 164
    item_ability_set 1, 0, 17, 45 @ 165
    item_ability_set 2, 0, 17, 48, 41, 59 @ 166
    item_ability_set 2, 0, 17, 49, 41, 62 @ 167
    item_ability_set 2, 0, 17, 51, 41, 56 @ 168
    item_ability_set 1, 0, 17, 47 @ 169
    item_ability_set 1, 0, 17, 46 @ 170
    item_ability_set 1, 0, 41, 60 @ 171
    item_ability_set 2, 0, 17, 54, 41, 65 @ 172
    item_ability_set 1, 0, 26, 72 @ 173
    item_ability_set 1, 0, 26, 73 @ 174
    item_ability_set 1, 0, 26, 74 @ 175
    item_ability_set 1, 0, 26, 75 @ 176
    item_ability_set 1, 0, 26, 76 @ 177
    item_ability_set 1, 0, 26, 77 @ 178
    item_ability_set 1, 0, 26, 78 @ 179
    item_ability_set 1, 0, 26, 79 @ 180
    item_ability_set 1, 0, 26, 80 @ 181
    item_ability_set 1, 0, 26, 82 @ 182
    item_ability_set 1, 0, 38, 22 @ 183
    item_ability_set 1, 0, 38, 27 @ 184
    item_ability_set 1, 0, 38, 23 @ 185
    item_ability_set 1, 0, 38, 25 @ 186
    item_ability_set 1, 0, 38, 28 @ 187
    item_ability_set 1, 0, 38, 26 @ 188
    item_ability_set 1, 0, 38, 24 @ 189
    item_ability_set 1, 0, 38, 30 @ 190
    item_ability_set 1, 0, 38, 29 @ 191
    item_ability_set 1, 0, 38, 31 @ 192
    item_ability_set 3, 0, 2, 10, 13, 10, 28, 10 @ 193
    item_ability_set 1, 0, 37, 19 @ 194
    item_ability_set 2, 0, 3, 21, 15, 31 @ 195
    item_ability_set 1, 0, 19, 74 @ 196
    item_ability_set 1, 0, 3, 20 @ 197
    item_ability_set 2, 0, 30, 30, 40, 52 @ 198
    item_ability_set 3, 0, 5, 45, 39, 42, 24, 54 @ 199
    item_ability_set 2, 0, 10, 115, 25, 70 @ 200
    item_ability_set 3, 0, 5, 44, 39, 41, 17, 53 @ 201
    item_ability_set 3, 0, 10, 114, 26, 81, 41, 64 @ 202
    item_ability_set 2, 0, 34, 73, 40, 53 @ 203
    item_ability_set 3, 0, 12, 138, 35, 83, 41, 63 @ 204
    item_ability_set 3, 0, 6, 55, 25, 69, 4, 31 @ 205
    item_ability_set 1, 0, 27, 92 @ 206
    item_ability_set 2, 0, 4, 32, 16, 41 @ 207
    item_ability_set 3, 0, 17, 52, 27, 91, 28, 9 @ 208
    item_ability_set 3, 0, 8, 79, 21, 22, 42, 76 @ 209
    item_ability_set 3, 0, 7, 67, 20, 10, 31, 42 @ 210
    item_ability_set 4, 0, 8, 78, 21, 21, 42, 75, 18, 63 @ 211
    item_ability_set 4, 0, 18, 64, 32, 52, 9, 90, 23, 43 @ 212
    item_ability_set 5, 0, 29, 20, 9, 89, 23, 42, 22, 32, 43, 86 @ 213
    item_ability_set 1, 0, 255, 1 @ 214
    item_ability_set 1, 0, 255, 2 @ 215
    item_ability_set 1, 0, 255, 8 @ 216
    item_ability_set 1, 0, 255, 7 @ 217
    item_ability_set 1, 0, 255, 5 @ 218
    item_ability_set 1, 0, 255, 3 @ 219
    item_ability_set 1, 0, 255, 4 @ 220
    item_ability_set 1, 0, 255, 6 @ 221
    item_ability_set 3, 0, 27, 93, 28, 10, 37, 20 @ 222
    item_ability_set 3, 0, 2, 10, 13, 10, 37, 20 @ 223
    item_ability_set 3, 0, 11, 126, 33, 62, 36, 9 @ 224
.if (. - gItemAbilitySets) != 225 * 20
    .error "item ability table must contain 225 records"
.endif
.size gItemAbilitySets, . - gItemAbilitySets
