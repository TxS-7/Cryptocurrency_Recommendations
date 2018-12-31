LSH_DIR   = LSH
LSH_OBJS  = $(LSH_DIR)/LSH.o $(LSH_DIR)/hash_table.o $(LSH_DIR)/cosine_hash_table.o
OBJS      = main.o tweet.o recommendation.o cosine_lsh_recommendation.o clustering.o data_point.o file_io.o util.o metrics.o
CC        = g++
FLAGS     = -Wall -g3 -std=c++11

all: recommendation

recommendation: $(LSH_OBJS) $(OBJS)
	$(CC) -o recommendation $(LSH_OBJS) $(OBJS)


main.o: main.cpp tweet.h cosine_lsh_recommendation.h file_io.h util.h
	$(CC) $(FLAGS) -c main.cpp


cosine_lsh_recommendation.o: cosine_lsh_recommendation.cpp cosine_lsh_recommendation.h recommendation.h tweet.h $(LSH_DIR)/LSH.h data_point.h metrics.h util.h
	$(CC) $(FLAGS) -c cosine_lsh_recommendation.cpp

recommendation.o: recommendation.cpp recommendation.h tweet.h clustering.h data_point.h metrics.h
	$(CC) $(FLAGS) -c recommendation.cpp



clustering.o: clustering.cpp clustering.h data_point.h metrics.h
	$(CC) $(FLAGS) -c clustering.cpp


$(LSH_DIR)/LSH.o: $(LSH_DIR)/LSH.cpp $(LSH_DIR)/LSH.h $(LSH_DIR)/hash_table.h data_point.h  $(LSH_DIR)/cosine_hash_table.h metrics.h
	$(CC) $(FLAGS) -c $(LSH_DIR)/LSH.cpp -o $(LSH_DIR)/LSH.o

$(LSH_DIR)/cosine_hash_table.o: $(LSH_DIR)/cosine_hash_table.cpp $(LSH_DIR)/cosine_hash_table.h $(LSH_DIR)/hash_table.h data_point.h metrics.h util.h
	$(CC) $(FLAGS) -c $(LSH_DIR)/cosine_hash_table.cpp -o $(LSH_DIR)/cosine_hash_table.o

$(LSH_DIR)/hash_table.o: $(LSH_DIR)/hash_table.cpp $(LSH_DIR)/hash_table.h data_point.h metrics.h
	$(CC) $(FLAGS) -c $(LSH_DIR)/hash_table.cpp -o $(LSH_DIR)/hash_table.o



file_io.o: file_io.cpp file_io.h tweet.h util.h
	$(CC) $(FLAGS) -c file_io.cpp

metrics.o: metrics.cpp metrics.h data_point.h
	$(CC) $(FLAGS) -c metrics.cpp

data_point.o: data_point.cpp data_point.h util.h metrics.h
	$(CC) $(FLAGS) -c data_point.cpp

tweet.o: tweet.cpp tweet.h util.h
	$(CC) $(FLAGS) -c tweet.cpp

util.o: util.cpp util.h
	$(CC) $(FLAGS) -c util.cpp



clean:
	rm -f $(OBJS) $(LSH_OBJS) recommendation
