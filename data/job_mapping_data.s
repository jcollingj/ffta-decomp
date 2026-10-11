@ Decoded key/value records read by ReadJobMappingValue/FindJobMappingKey.
@ Reverse search covers exactly these 43 records; forward lookup is unchecked.
.section .text
.global gJobMapping
.type gJobMapping, %object
gJobMapping:
.macro job_mapping key, value
    .if (\key < 0) || (\key > 255) || (\value < 0) || (\value > 255)
        .error "job mapping fields must fit unsigned bytes"
    .endif
    .byte \key, \value
.endm
    job_mapping 0, 0
    job_mapping 1, 0
    job_mapping 2, 1
    job_mapping 3, 2
    job_mapping 4, 3
    job_mapping 5, 4
    job_mapping 6, 5
    job_mapping 7, 6
    job_mapping 8, 7
    job_mapping 9, 8
    job_mapping 10, 9
    job_mapping 11, 10
    job_mapping 12, 11
    job_mapping 13, 12
    job_mapping 14, 13
    job_mapping 15, 14
    job_mapping 16, 15
    job_mapping 17, 16
    job_mapping 18, 17
    job_mapping 19, 18
    job_mapping 20, 6
    job_mapping 21, 7
    job_mapping 22, 19
    job_mapping 23, 8
    job_mapping 24, 20
    job_mapping 25, 21
    job_mapping 26, 22
    job_mapping 27, 23
    job_mapping 28, 24
    job_mapping 29, 25
    job_mapping 30, 26
    job_mapping 31, 6
    job_mapping 32, 27
    job_mapping 33, 10
    job_mapping 34, 28
    job_mapping 35, 29
    job_mapping 36, 30
    job_mapping 37, 31
    job_mapping 38, 32
    job_mapping 39, 4
    job_mapping 40, 33
    job_mapping 41, 34
    job_mapping 42, 7
.if (.-gJobMapping) != 86
    .error "matching job mapping must contain 43 records"
.endif
.size gJobMapping, .-gJobMapping
