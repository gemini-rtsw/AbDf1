[schematic2]
uniq 11
[tools]
[detail]
w 3 -173 100 0 n#1 hwin.hwin#5.in 0 -176 0 -176 ebis.bi.INP
w 659 -253 100 0 n#2 hwout.hwout#7.outp 656 -256 656 -256 ebos.bo.OUT
w 659 -525 100 0 n#3 hwout.hwout#14.outp 656 -528 656 -528 embbos.mbbo.OUT
w 3 -477 100 0 n#4 hwin.hwin#16.in 0 -480 0 -480 embbis.mbbi.INP
w -693 483 100 0 n#5 hwin.hwin#2.in -696 480 -696 480 eais.ai.INP
w -316 435 100 0 n#6 eais.ai.VAL -440 432 -192 432 -192 408 -80 408 ecalcs.calc.INPA
w -404 467 100 0 n#7 eais.ai.FLNK -440 464 -368 464 -368 24 -80 24 ecalcs.calc.SLNK
w 658 162 -100 0 n#8 hwout.hwout#12.outp 648 152 648 152 eaos.ao.OUT
w 218 218 -100 0 n#9 ecalcs.calc.VAL 208 216 392 216 eaos.ao.DOL
w 218 250 -100 0 n#10 ecalcs.calc.FLNK 208 248 264 248 264 184 392 184 eaos.ao.SLNK
[cell use]
use eais -567 371 100 0 ai
xform 0 -568 448
p -623 371 100 0 -1 PV:plc:
p -635 349 100 0 1 DTYP:AB DF1 serial
p -636 299 100 0 1 SCAN:.5 second
p -644 328 100 0 1 LINR:NO CONVERSION
use hwin -888 464 100 0 hwin#2
xform 0 -792 480
p -1005 472 100 0 -1 val(in):@/dev/ttyS1 41 101
use ebis 124 -280 100 0 bi
xform 0 128 -208
p 68 -280 100 0 -1 PV:plc:
p 61 -304 100 0 1 DTYP:AB DF1 serial
p 68 -146 100 0 1 SCAN:I/O Intr
p 59 -354 100 0 1 ONAM:One
p 63 -326 100 0 1 ZNAM:Zero
use hwin -192 -192 100 0 hwin#5
xform 0 -96 -176
p -309 -184 100 0 -1 val(in):@/dev/ttyS1 41 100 15
use ebos 529 -312 100 0 bo
xform 0 528 -224
p 473 -312 100 0 -1 PV:plc:
p 450 -337 100 0 1 DTYP:AB DF1 serial
use hwout 656 -272 100 0 hwout#7
xform 0 752 -256
p 752 -265 100 0 -1 val(outp):@/dev/ttyS1 41 100 15
use eaos 531 91 100 0 ao
xform 0 520 184
p 475 91 100 0 -1 PV:plc:
p 446 66 100 0 1 DTYP:AB DF1 serial
p 449 45 100 0 1 OMSL:closed_loop
use embbis 128 -594 100 0 mbbi
xform 0 128 -512
p 72 -594 100 0 -1 PV:plc:
p 59 -447 100 0 1 SCAN:I/O Intr
p -64 -594 100 0 0 FRVL:4
p 128 -626 100 0 0 FVST:Five
p -64 -626 100 0 0 FVVL:5
p 128 -754 100 0 0 NIST:
p 91 -543 100 0 1 NOBT:3
p 128 -498 100 0 0 ONST:One
p -64 -498 100 0 0 ONVL:1
p -288 -594 100 0 0 PINI:YES
p 128 -690 100 0 0 SVST:Seven
p -64 -690 100 0 0 SVVL:7
p 128 -658 100 0 0 SXST:Six
p -64 -658 100 0 0 SXVL:6
p 128 -562 100 0 0 THST:Three
p -64 -562 100 0 0 THVL:3
p 128 -530 100 0 0 TWST:Two
p -64 -530 100 0 0 TWVL:2
p 128 -466 100 0 0 ZRST:Zero
p 43 -620 100 0 1 DTYP:AB DF1 serial
p 128 -594 100 0 0 FRST:Four
use embbos 528 -622 100 0 mbbo
xform 0 528 -528
p 472 -622 100 0 -1 PV:plc:
p 449 -646 100 0 1 DTYP:AB DF1 serial
p 490 -560 100 0 1 NOBT:3
use hwout 648 136 100 0 hwout#12
xform 0 744 152
p 744 143 100 0 -1 val(outp):@/dev/ttyS1 41 101
use hwout 656 -544 100 0 hwout#14
xform 0 752 -528
p 752 -537 100 0 -1 val(outp):@/dev/ttyS1 41 100 2
use hwin -192 -496 100 0 hwin#16
xform 0 -96 -480
p -309 -488 100 0 -1 val(in):@/dev/ttyS1 41 100 2
use ecalcs -80 -40 100 0 calc
xform 0 64 200
p -136 -40 100 0 -1 PV:plc:
p 115 356 100 0 1 CALC:A+1
[comments]
