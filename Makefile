#Copyright Octavian Armasu 315CAa 2022-2023
build:
	gcc editor.c -o image_editor -lm -Wall -Wextra
clean:
	rm -f editor
pack:
	zip -FSr 315CA_ArmasuOctavian_Tema3.zip README Makefile *.c 

.PHONY: pack clean