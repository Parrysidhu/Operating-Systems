Execution Steps - 

1. Compile - make

2. Insert module - sudo insmod linepipe.ko buff_size=<size>

3. Run producer	- sudo ./producer /dev/linepipe

4. Run consumer	- sudo ./consumer /dev/linepipe