@ Explicit ROM pointers searched by EncodeUnitReferencePointer.
@ Only this consumer-bounded span is recovered; pointees remain opaque.
.section .text
.global gUnitReferenceTable1
.type gUnitReferenceTable1, %object
.macro reference_pointer address
    .if (\address < 0x08000000) || (\address >= 0x09000000)
        .error "reference pointer must address the 16 MiB ROM window"
    .endif
    .4byte \address
.endm
gUnitReferenceTable1:
    reference_pointer 0x0855128C @ 0
    reference_pointer 0x08551296 @ 1
    reference_pointer 0x0855129F @ 2
    reference_pointer 0x085512A7 @ 3
    reference_pointer 0x085512AD @ 4
    reference_pointer 0x085512B3 @ 5
    reference_pointer 0x085512BB @ 6
    reference_pointer 0x085512C0 @ 7
    reference_pointer 0x085512C7 @ 8
    reference_pointer 0x085512D2 @ 9
    reference_pointer 0x085512D9 @ 10
    reference_pointer 0x085512DF @ 11
    reference_pointer 0x085512E5 @ 12
    reference_pointer 0x085512EC @ 13
    reference_pointer 0x085512F5 @ 14
    reference_pointer 0x085512FB @ 15
    reference_pointer 0x08551305 @ 16
    reference_pointer 0x0855130C @ 17
    reference_pointer 0x08551315 @ 18
    reference_pointer 0x0855131D @ 19
    reference_pointer 0x0855132A @ 20
    reference_pointer 0x08551347 @ 21
    reference_pointer 0x08551360 @ 22
    reference_pointer 0x08551369 @ 23
    reference_pointer 0x08551370 @ 24
    reference_pointer 0x08551378 @ 25
    reference_pointer 0x08551385 @ 26
    reference_pointer 0x0855138E @ 27
    reference_pointer 0x0855139A @ 28
    reference_pointer 0x085513A4 @ 29
    reference_pointer 0x085513AE @ 30
    reference_pointer 0x085513B8 @ 31
    reference_pointer 0x085513BF @ 32
    reference_pointer 0x085513C7 @ 33
    reference_pointer 0x085513CE @ 34
    reference_pointer 0x085513D6 @ 35
    reference_pointer 0x085513E0 @ 36
    reference_pointer 0x085513EB @ 37
    reference_pointer 0x085513F3 @ 38
    reference_pointer 0x085513FC @ 39
    reference_pointer 0x08551403 @ 40
    reference_pointer 0x0855140C @ 41
    reference_pointer 0x08551413 @ 42
    reference_pointer 0x0855141A @ 43
    reference_pointer 0x08551421 @ 44
    reference_pointer 0x08551429 @ 45
    reference_pointer 0x08551431 @ 46
    reference_pointer 0x08551437 @ 47
    reference_pointer 0x0855143E @ 48
    reference_pointer 0x0855144A @ 49
    reference_pointer 0x08551453 @ 50
    reference_pointer 0x08551459 @ 51
    reference_pointer 0x08551461 @ 52
    reference_pointer 0x0855146C @ 53
    reference_pointer 0x08551473 @ 54
    reference_pointer 0x0855147B @ 55
    reference_pointer 0x08551492 @ 56
    reference_pointer 0x0855149B @ 57
    reference_pointer 0x085514A2 @ 58
    reference_pointer 0x085514A8 @ 59
    reference_pointer 0x085514BF @ 60
    reference_pointer 0x085514D4 @ 61
    reference_pointer 0x085514ED @ 62
    reference_pointer 0x08551504 @ 63
    reference_pointer 0x0855151B @ 64
    reference_pointer 0x08551526 @ 65
    reference_pointer 0x0855152D @ 66
    reference_pointer 0x08551534 @ 67
    reference_pointer 0x0855153C @ 68
    reference_pointer 0x08551545 @ 69
    reference_pointer 0x0855155A @ 70
    reference_pointer 0x08551562 @ 71
    reference_pointer 0x0855156A @ 72
    reference_pointer 0x08551572 @ 73
    reference_pointer 0x08551589 @ 74
    reference_pointer 0x08551592 @ 75
    reference_pointer 0x0855159B @ 76
    reference_pointer 0x085515A2 @ 77
    reference_pointer 0x085515A8 @ 78
    reference_pointer 0x085515BF @ 79
    reference_pointer 0x085515CC @ 80
    reference_pointer 0x085515D2 @ 81
    reference_pointer 0x085515DA @ 82
    reference_pointer 0x085515EF @ 83
    reference_pointer 0x085515F7 @ 84
    reference_pointer 0x08551604 @ 85
    reference_pointer 0x0855160E @ 86
    reference_pointer 0x08551613 @ 87
    reference_pointer 0x0855161A @ 88
    reference_pointer 0x08551620 @ 89
    reference_pointer 0x08551627 @ 90
    reference_pointer 0x08551630 @ 91
    reference_pointer 0x08551638 @ 92
    reference_pointer 0x0855163F @ 93
    reference_pointer 0x08551647 @ 94
    reference_pointer 0x0855164F @ 95
    reference_pointer 0x08551659 @ 96
    reference_pointer 0x0855165F @ 97
    reference_pointer 0x08551665 @ 98
    reference_pointer 0x0855167A @ 99
    reference_pointer 0x08551684 @ 100
    reference_pointer 0x08551691 @ 101
    reference_pointer 0x0855169E @ 102
    reference_pointer 0x085516B1 @ 103
    reference_pointer 0x085516B9 @ 104
    reference_pointer 0x085516C0 @ 105
    reference_pointer 0x085516CA @ 106
    .if (. - gUnitReferenceTable1) != 428
        .error "reference table count changed"
    .endif
.size gUnitReferenceTable1, . - gUnitReferenceTable1
