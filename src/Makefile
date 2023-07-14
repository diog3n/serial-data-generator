alarmer-debug:
	gcc alarmer.c utils.c -g -std=c11 -fsanitize=address -lc -o alarmer
alarmer-release:
	gcc alarmer.c utils.c -std=c11 -lc -o alarmer
debug-rd:
	gcc serial_reader.c utils.c -g -std=c11 -fsanitize=address -lc -o serial_reader
release-rd:
	gcc serial_reader.c utils.c -std=c11 -lc -o serial_reader
debug-wr:
	gcc serial_writer.c utils.c -g -std=c11 -fsanitize=address -lc -o serial_writer
release-wr:
	gcc serial_writer.c utils.c -std=c11 -lc -o run serial_writer
