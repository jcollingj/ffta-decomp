@ Two prerequisite job/count pairs per record; job zero ends the list.
@ Indices 0..19 are used by the 116 original job records (selector 39).
.section .text
.global gJobRequirements
.type gJobRequirements, %object
gJobRequirements:
.macro job_requirements job1, count1, job2, count2
    .if (\job1 < 0) || (\job1 > 115) || (\job2 < 0) || (\job2 > 115)
        .error "prerequisite job must be within the original job table"
    .endif
    .if (\count1 < 0) || (\count1 > 255) || (\count2 < 0) || (\count2 > 255)
        .error "prerequisite counts must fit unsigned bytes"
    .endif
    .byte \job1, \count1, \job2, \count2
.endm
    job_requirements 0, 0, 0, 0 @ 0
    job_requirements 5, 2, 0, 0 @ 1
    job_requirements 39, 2, 0, 0 @ 2
    job_requirements 2, 2, 0, 0 @ 3
    job_requirements 13, 2, 0, 0 @ 4
    job_requirements 28, 1, 31, 1 @ 5
    job_requirements 28, 1, 0, 0 @ 6
    job_requirements 17, 2, 0, 0 @ 7
    job_requirements 11, 2, 0, 0 @ 8
    job_requirements 35, 1, 29, 2 @ 9
    job_requirements 33, 2, 0, 0 @ 10
    job_requirements 42, 5, 0, 0 @ 11
    job_requirements 36, 1, 0, 0 @ 12
    job_requirements 7, 3, 8, 5 @ 13
    job_requirements 7, 1, 8, 1 @ 14
    job_requirements 21, 5, 0, 0 @ 15
    job_requirements 20, 3, 21, 5 @ 16
    job_requirements 31, 2, 29, 2 @ 17
    job_requirements 25, 5, 0, 0 @ 18
    job_requirements 25, 2, 20, 3 @ 19
.if (.-gJobRequirements) != 80
    .error "matching prerequisite table must contain 20 records"
.endif
.size gJobRequirements, .-gJobRequirements
