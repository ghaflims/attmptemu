CC65 = cc65
CA65 = ca65
LD65 = ld65
NAME = lesson11

All: $(NAME).nes $(NAME)b.nes
# if linux uncomment the next line
	rm *.o
# windows	
#	python3 fceux_symbols4.py lesson11
# or python3

$(NAME).nes: $(NAME).o reset.o asm4c.o nes.cfg
	$(LD65) -C nes.cfg -o $(NAME).nes reset.o $(NAME).o asm4c.o nes.lib -Ln l.txt -m m.map --dbgfile l.dbg
	@echo $(NAME).nes created

#
$(NAME)b.nes: $(NAME)b.o reset.o asm4c.o nes.cfg
	$(LD65) -C nes.cfg -o $(NAME)b.nes reset.o $(NAME)b.o asm4c.o nes.lib -Ln lb.txt -m mb.map --dbgfile lb.dbg
	@echo $(NAME)b.nes created

reset.o: reset.s
	$(CA65) reset.s

asm4c.o: asm4c.s
	$(CA65) asm4c.s

$(NAME).o: $(NAME).s
	$(CA65) $(NAME).s

$(NAME).s: $(NAME).c
	$(CC65) -Oi $(NAME).c --add-source --debug-info

$(NAME)b.o: $(NAME)b.s
	$(CA65) $(NAME)b.s

$(NAME)b.s: $(NAME)b.c
	$(CC65) -Oi $(NAME)b.c --add-source --debug-info

clean:
# if linux uncomment the next line
	rm *.nes

