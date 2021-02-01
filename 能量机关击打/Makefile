
objs = main.o kalman.o random.o tool.o predictor.o sys_time.o

prog: $(objs)
	g++ $(objs) -o prog

$(filter-out predictor.o, $(objs)):%.o:%.cpp
	g++ -c $< -o $@

predictor.o:SinePredictor.cpp
	g++ -c SinePredictor.cpp -o predictor.o

.PHNOY: run clean

run:
	@make
	@./prog.exe

clean:
	rm prog.exe
	rm $(objs)
