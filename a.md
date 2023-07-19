We set characters to three part:

1. end -> end searching
2. unique/special -> set the flag and then just linear searching the rest characters. 
3. general -> do nothing.

Characters: 0-9 | + | - | . | _ | e | E | T | Z | i | n | 0x20 | a-f | A-F | 0obx | 0OBX

i     -> inf
n     -> nan
+     -> number
-     -> number
_     -> number
.     -> floating or data time
e     -> floating
E     -> floating
T     -> data time
Z     -> data time
0x20  -> data time
0-9   -> unknown

