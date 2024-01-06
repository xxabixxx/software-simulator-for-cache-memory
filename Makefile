all:
		g++ cache_sim.cpp -lreadline

test: 
			./a.out inp.txt

clean:
		rm -rf a.out

report:
			evince Report.pdf