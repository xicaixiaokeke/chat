ser : *.cpp
	g++ -o $@ $^ -levent -ljson -lmysqlclient -L/usrb/mysql -g
clean :
	rm -rf server

