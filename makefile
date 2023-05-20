ifdef CAPE_GREEN
MACRO = -DCAPE_GREEN
endif

SOURCES = main.c alarm_interface.c camera.c keyboard.c joystick.c stopwatch.c motion.c udp.c volKnob.c helpers.c 14segCountdown.c ext_8x8led.c led_status.c terminal.c audioBuffer.c audioControl.c audioOutput.c

WAVEDIR = wave-files
SERVERDIR = security-server-copy

OUTFILE = security_system
OUTDIR = $(HOME)/cmpt433/public/myApps

CROSS_COMPILE = arm-linux-gnueabihf-
CC_C = $(CROSS_COMPILE)gcc
CFLAGS = -Wall -g -std=c99 -D _POSIX_C_SOURCE=200809L -Werror -Wshadow $(MACRO)

LFLAGS = -L$(HOME)/cmpt433/public/asound_lib_BBB

all: wav node password app success
		
app:
	$(CC_C) $(CFLAGS) $(SOURCES) -o $(OUTDIR)/$(OUTFILE) $(LFLAGS) -lpthread -lasound

wav:
	mkdir -p $(OUTDIR)/$(WAVEDIR)/
	cp -R $(WAVEDIR)/* $(OUTDIR)/$(WAVEDIR)/

password:
	cp password.txt $(OUTDIR)/password.txt

node:
	mkdir -p $(OUTDIR)/$(SERVERDIR)/ 
	cp -R server-code/* $(OUTDIR)/$(SERVERDIR) 
	cd $(OUTDIR)/$(SERVERDIR)/ && npm install

success:
	@echo "Sucessfully Compiled!"

clean: 
		rm $(OUTDIR)/$(OUTFILE)
		rm -r $(OUTDIR)/$(SERVERDIR)/
		rm -r $(OUTDIR)/$(WAVEDIR)/
