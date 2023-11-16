target: all atob btoa
all: 
	ipcrm -a
atob: progress_atob.cpp mailbox.h
	gcc -o atob progress_atob.cpp
btoa: progress_btoa.cpp mailbox.h
	gcc -o btoa progress_btoa.cpp
clean:
	rm -rf atob
	rm -rf btoa
