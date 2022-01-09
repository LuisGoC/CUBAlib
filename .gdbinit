#---gdb settings
set pagination off
set confirm off
set verbose off
set height 0
set width 0

#---connect and load program
target remote localhost:2331
#mon arm semihosting enable
load
mon reset
break main
continue
