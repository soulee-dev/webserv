TARGET = tiny
OBJS = 	tiny.o csapp.o
%.o: %.c
	gcc -c $<
$(TARGET) : $(OBJS)
	gcc -o $(TARGET) $(OBJS)
fclean:
	rm -f *.o
	rm -f $(TARGET)